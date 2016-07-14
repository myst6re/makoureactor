/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
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
#include "PCFieldFile.h"
#include <algorithm>

BackgroundFilePC::BackgroundFilePC(FieldPC *field) :
	BackgroundFile(field)
{
}

BackgroundFilePC::BackgroundFilePC(const BackgroundFilePC &other) :
	BackgroundFile(other)
{
	setTextures(new BackgroundTexturesPC(*static_cast<BackgroundTexturesPC *>(other.textures())));
	PalettesPC palettes;
	foreach(Palette *pal, other.palettes()) {
		palettes.append(new PalettePC(*pal));
	}
	setPalettes(palettes);
}

FieldPC *BackgroundFilePC::field() const
{
	return static_cast<FieldPC *>(FieldPart::field());
}

bool BackgroundFilePC::open()
{
	if(isOpen() || isModified()) {
		setOpen(true);
		return true;
	}

	PCFieldFile *pcFieldFile = field()->pcFieldFile();
	return open(pcFieldFile->sectionData(PCFieldFile::Background),
				pcFieldFile->sectionData(PCFieldFile::Palette));
}

bool BackgroundFilePC::open(const QByteArray &data, const QByteArray &palData)
{
	QBuffer buff, palBuff;

	buff.setData(data);
	palBuff.setData(palData);

	BackgroundIOPC io(&buff, &palBuff);
	if(!io.read(*this)) {
		return false;
	}

	setOpen(true);

	return true;
}

QByteArray BackgroundFilePC::save() const
{
	QBuffer buff, palBuff;

	BackgroundIOPC io(&buff, &palBuff);
	if(!io.write(*this)) {
		return QByteArray();
	}

	return buff.data();
}

QByteArray BackgroundFilePC::savePal() const
{
	QBuffer buff, palBuff;

	PaletteIOPC io(&palBuff, &buff);
	if(!io.write(palettes())) {
		return QByteArray();
	}

	return palBuff.data();
}

BackgroundFilePS BackgroundFilePC::toPS(FieldPS *field) const
{
	PalettesPS palettesPS = ((PalettesPC *)&palettes())->toPS();
	BackgroundTiles tilesPS;
	BackgroundTexturesPS texturesPS = (static_cast<BackgroundTexturesPC *>(textures()))->toPS(tiles(), tilesPS, palettesPS);

	BackgroundFilePS filePS(field);
	filePS.setPalettes(palettesPS);
	filePS.setTextures(new BackgroundTexturesPS(texturesPS));
	filePS.setTiles(tilesPS);
	filePS.setOpen(true);

	return filePS;
}

bool BackgroundFilePC::repair()
{
	int paletteCount = palettes().size();
	bool modified = false;

	QSet<quint8> usedPalettes = this->tiles().usedPalettes();
	QList<quint8> unusedPalettes;

	// List unused palettes
	for(int palID = 0; palID < paletteCount; ++palID) {
		if(!usedPalettes.contains(palID)) {
			unusedPalettes.append(palID);
		}
	}

	std::sort(unusedPalettes.begin(), unusedPalettes.end(), qLess<quint8>());

	QMap<quint16, Tile> &tiles = (QMap<quint16, Tile> &)tilesRef();
	QMap<quint8, quint8> texToPalette;
	QMutableMapIterator<quint16, Tile> it(tiles);
	while(it.hasNext()) {
		it.next();
		Tile &tile = it.value();
		if (tile.depth < 2 && tile.blending && tile.typeTrans != 2 && tile.paletteID >= paletteCount) {
			tile.typeTrans = 2; // Modification in place
			if(texToPalette.contains(tile.textureID)) {
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

	if (modified) {
		setModified(true);
		return true;
	}

	return false;
}
