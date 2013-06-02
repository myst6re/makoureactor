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

QHash<quint8, quint32> BackgroundFilePC::posTextures;
QByteArray BackgroundFilePC::data;

BackgroundFilePC::BackgroundFilePC(FieldPC *field) :
	BackgroundFile(field)
{
}

quint16 BackgroundFilePC::textureWidth(const Tile &tile) const
{
	return depth(tile) * 256;
}

quint8 BackgroundFilePC::depth(const Tile &tile) const
{
	if(posTextures.contains(tile.textureID)) {
		quint32 pos = posTextures.value(tile.textureID);
		return data.at(pos);
	}
	return quint8(-1);
}

quint32 BackgroundFilePC::originInData(const Tile &tile) const
{
	if(posTextures.contains(tile.textureID)) {
		quint32 pos = posTextures.value(tile.textureID);
		return pos + 2 + (tile.srcY * 256 + tile.srcX) * (quint8)data.at(pos);
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

bool BackgroundFilePC::openTiles(const QByteArray &data, qint64 *pos)
{
	BackgroundTiles tiles;
	QBuffer buff;
	buff.setData(data);

	BackgroundTilesIOPC io(&buff);
	if(!io.read(tiles)) {
		return false;
	}

	setTiles(tiles);

	if(pos) {
		*pos = buff.pos();
	}

	return true;
}

QPixmap BackgroundFilePC::openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
	data = field()->sectionData(Field::Background);
	quint32 dataSize = data.size(), i;
	QList<Palette *> palettes;
	qint64 aTex;

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

	if(!tilesAreCached() && !openTiles(data, &aTex)) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QPixmap();
	}

	aTex += 7;
	posTextures.clear();

	//Textures
	for(i=0 ; i<42 ; ++i) {
		if(dataSize < aTex+2) {
			foreach(Palette *palette, palettes) {
				delete palette;
			}
			return QPixmap();
		}
		if((bool)data.at(aTex)) {
			posTextures.insert(i, aTex+4);
			aTex += (quint8)data.at(aTex+4)*65536 + 4;
			if(dataSize < aTex) {
				foreach(Palette *palette, palettes) {
					delete palette;
				}
				return QPixmap();
			}
		}
		aTex += 2;
	}

	return drawBackground(tiles().tiles(paramActifs, z, layers), palettes, data);
}

Tile BackgroundFilePC::tilePC2Tile(const TilePC &tile)
{
	Tile ret;

	ret.dstX = tile.dstX;
	ret.dstY = tile.dstY;
	ret.srcX = tile.srcX;
	ret.srcY = tile.srcY;
	ret.paletteID = tile.paletteID;
	ret.ID = tile.ID;
	ret.param = tile.param;
	ret.state = tile.state;
	ret.blending = tile.blending;
	ret.typeTrans = tile.typeTrans;
	ret.size = tile.size;
	ret.textureID = tile.textureID;
	ret.textureID2 = tile.textureID2;
	ret.depth = tile.depth;

	return ret;
}
