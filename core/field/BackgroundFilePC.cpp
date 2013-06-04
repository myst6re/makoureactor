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

BackgroundFilePC::BackgroundFilePC(FieldPC *field) :
	BackgroundFile(field), aTex(-1)
{
}

quint16 BackgroundFilePC::textureWidth(const Tile &tile) const
{
	return depth(tile) * 256;
}

quint8 BackgroundFilePC::depth(const Tile &tile) const
{
	/* When tile.depth is used, it can be buggy,
	 * because the PC version doesn't understand
	 * depth = 0. */
	if(textures.hasTex(tile.textureID)) {
		return textures.texDepth(tile.textureID);
	}
	return qMax(quint8(1), tile.depth);
}

quint32 BackgroundFilePC::originInData(const Tile &tile) const
{
	if(textures.hasTex(tile.textureID)) {
		quint32 pos = textures.texPos(tile.textureID);
		return pos + (tile.srcY * 256 + tile.srcX) * depth(tile);
	}
	return 0;
}

QRgb BackgroundFilePC::directColor(quint16 color) const
{
	return qRgb( (color>>11)*COEFF_COLOR, (color>>6 & 31)*COEFF_COLOR, (color & 31)*COEFF_COLOR ); // special PC RGB16 color
}

bool BackgroundFilePC::openPalettes(const QByteArray &data, const QByteArray &palData, QList<Palette *> &palettes)
{
	QBuffer palBuff, buff;
	palBuff.setData(palData);
	buff.setData(data);

	if(!buff.open(QIODevice::ReadOnly) ||
			!buff.seek(12)) {
		return false;
	}

	PaletteIOPC io(&palBuff);
	io.setDeviceAlpha(&buff);
	if(!io.read(palettes)) {
		return false;
	}

	return true;
}

bool BackgroundFilePC::openTiles(const QByteArray &data)
{
	BackgroundTiles tiles;
	QBuffer buff;
	buff.setData(data);

	/*QFile fileIn("tilesIn");
	if(fileIn.open(QIODevice::WriteOnly)) {
		fileIn.write(data.mid(0x28));
		fileIn.close();
	} else {
		qWarning() << "tilesIn can't be written";
	}*/

	BackgroundTilesIOPC io(&buff);
	if(!io.read(tiles)) {
		return false;
	}

	/*QFile file("tilesOut");
	BackgroundTilesIOPC io2(&file);
	if(!io2.write(tiles)) {
		qWarning() << "tilesOut can't be written";
	}*/

	setTiles(tiles);

	aTex = buff.pos() + 7;

	return true;
}

bool BackgroundFilePC::openTextures(const QByteArray &data)
{
	QBuffer buff;
	buff.setData(data);

	if(!buff.open(QIODevice::ReadOnly) ||
			!buff.seek(aTex)) {
		return false;
	}

	BackgroundTexturesIOPC io(&buff);
	if(!io.read(&textures)) {
		return false;
	}

	return true;
}

QPixmap BackgroundFilePC::openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
	QByteArray data = field()->sectionData(Field::Background);
	QList<Palette *> palettes;

	if(!openPalettes(data, field()->sectionData(Field::PalettePC), palettes)) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QPixmap();
	}

	/*i=0;
	foreach(const Palette *palette, palettes) {
		palette->toImage().scaled(128, 128, Qt::KeepAspectRatio)
				.save(QString("palettes/PC/palette_%1_%2_PC.png")
					  .arg(field()->name()).arg(i));
		((PalettePC *)palette)->toPS().toImage().scaled(128, 128, Qt::KeepAspectRatio)
				.save(QString("palettes/PC/palette_%1_%2_PC_to_PS.png")
					  .arg(field()->name()).arg(i));
		++i;
	}
	i=0;*/

	if(aTex < 0) {
		if(!openTiles(data)) {
			foreach(Palette *palette, palettes) {
				delete palette;
			}

			return QPixmap();
		}
	}

	if(!openTextures(data)) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QPixmap();
	}

	return drawBackground(tiles().tiles(paramActifs, z, layers), palettes, data);
}
