/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "BackgroundFilePC.h"
#include "Palette.h"
#include "PaletteIO.h"
#include "../PsColor.h"
#include "FieldPC.h"
#include <algorithm>

BackgroundFilePC::BackgroundFilePC(FieldPC *field) :
	BackgroundFile(field)
{
}

BackgroundFilePC::BackgroundFilePC(const BackgroundFilePC &other) :
	BackgroundFile(other)
{
	setTextures(new BackgroundTexturesPC(*other.textures()));
	PalettesPC palettes;
	for (Palette *pal : other.palettes()) {
		palettes.append(new PalettePC(*pal));
	}
	setPalettes(palettes);
}

void BackgroundFilePC::initEmpty()
{
	clear();

	BackgroundTexturesPC *textures = new BackgroundTexturesPC();
	BackgroundTexturesPCInfos infos;
	infos.pos = 0;
	infos.size = 0; // 16x16 tiles
	infos.depth = 2; // No palettes
	// Black
	textures->setTex(0, QVector<uint>(256 * 256, qRgb(0, 0, 0)).toList(), infos);
	setTextures(textures);

	quint16 tileID = 0;
	QList<Tile> tiles;

	for (qint16 y = 128; y >= -256; y -= 128) {
		for (qint16 dstY = 0; dstY < 128; dstY += 16) {
			for (qint16 dstX = -256; dstX < 256; dstX += 16) {
				Tile t = Tile();
				t.ID = 4095;
				// Data optimization: always the same source
				t.srcX = 0;
				t.srcY = 0;
				t.dstX = dstX;
				t.dstY = y + dstY;
				t.size = 16;
				t.depth = 2;
				t.tileID = tileID++;
				tiles.append(t);
			}
		}
	}

	setTiles(BackgroundTiles(tiles));

	BackgroundFile::initEmpty();
}

bool BackgroundFilePC::open()
{
	if (isOpen() || isModified()) {
		setOpen(true);
		return true;
	}

	return open(field()->sectionData(Field::Background),
	            field()->sectionData(Field::PalettePC));
}

bool BackgroundFilePC::open(const QByteArray &data, const QByteArray &palData)
{
	QBuffer buff, palBuff;

	buff.setData(data);
	palBuff.setData(palData);

	BackgroundIOPC io(&buff, &palBuff);
	if (!io.read(*this)) {
		return false;
	}

	setOpen(true);

	return true;
}

QByteArray BackgroundFilePC::save() const
{
	QBuffer buff, palBuff;

	BackgroundIOPC io(&buff, &palBuff);
	if (!io.write(*this)) {
		return QByteArray();
	}

	return buff.data();
}

QByteArray BackgroundFilePC::savePal() const
{
	QBuffer buff, palBuff;

	PaletteIOPC io(&palBuff, &buff);
	if (!io.write(palettes())) {
		return QByteArray();
	}

	return palBuff.data();
}

BackgroundFilePS BackgroundFilePC::toPS(FieldPS *field) const
{
	PalettesPS palettesPS = static_cast<const PalettesPC *>(&palettes())->toPS();
	BackgroundTiles tilesPS;
	BackgroundTexturesPS texturesPS = textures()->toPS(tiles(), tilesPS, palettesPS);

	BackgroundFilePS filePS(field);
	filePS.setPalettes(palettesPS);
	filePS.setTextures(new BackgroundTexturesPS(texturesPS));
	filePS.setTiles(tilesPS);
	filePS.setOpen(true);

	return filePS;
}

bool BackgroundFilePC::repair()
{
	// PC field file contains PS tiles format
	// Altough it is unused by the game, we can use it to repair the PC format
	BackgroundTilesFile *psTiles = field()->tiles();
	QMap<quint16, Tile> &tiles = reinterpret_cast<QMap<quint16, Tile> &>(tilesRef());
	QMutableMapIterator<quint16, Tile> it(tiles);
	bool modified = false;

	if (psTiles && psTiles->isOpen()) {
		const QMap<qint32, Tile> &psTilesList = psTiles->tiles().sortedTiles();

		while (it.hasNext()) {
			it.next();
			Tile &tile = it.value();
			const Tile tilePs = psTilesList.value((tile.layerID << 16) | tile.tileID);

			if (tile.depth < 2 && tile.paletteID != tilePs.paletteID) {
				tile.paletteID = tilePs.paletteID;
				tile.typeTrans = tilePs.typeTrans;
				modified = true;
			}
		}
	} else { // By hand
		int paletteCount = palettes().size();
		QSet<quint8> usedPalettes = this->tiles().usedPalettes();
		QList<quint8> unusedPalettes;

		// List unused palettes
		for (int palID = 0; palID < paletteCount; ++palID) {
			if (!usedPalettes.contains(quint8(palID))) {
				unusedPalettes.append(quint8(palID));
			}
		}

		std::sort(unusedPalettes.begin(), unusedPalettes.end(), std::less<quint8>());

		QMap<quint8, quint8> texToPalette;

		while (it.hasNext()) {
			it.next();
			Tile &tile = it.value();

			if (tile.depth < 2 && tile.blending && tile.typeTrans != 2 && tile.paletteID >= paletteCount) {
				tile.typeTrans = 2; // Modification in place
				if (texToPalette.contains(tile.textureID)) {
					tile.paletteID = texToPalette.value(tile.textureID);
					modified = true;
				} else if (!unusedPalettes.isEmpty()) {
					tile.paletteID = unusedPalettes.first();
					unusedPalettes.removeFirst();
					texToPalette.insert(tile.textureID, tile.paletteID);
					modified = true;
				} else {
					qWarning() << "BackgroundFilePC::repair cannot detect palette ID to use";
				}
			}
		}
	}

	if (modified) {
		setModified(true);
		return true;
	}

	return false;
}
