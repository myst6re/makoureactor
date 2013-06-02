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
#include "BackgroundFilePS.h"
#include "Palette.h"
#include "PaletteIO.h"
#include "../PsColor.h"
#include "FieldPS.h"

quint32 BackgroundFilePS::headerPalSize;
MIM BackgroundFilePS::headerImg;
MIM BackgroundFilePS::headerEffect;

BackgroundFilePS::BackgroundFilePS(FieldPS *field) :
	BackgroundFile(field)
{
}

quint16 BackgroundFilePS::textureWidth(const Tile &tile) const
{
	return tile.textureID2 ? headerEffect.w : headerImg.w;
}

quint8 BackgroundFilePS::depth(const Tile &tile) const
{
	return tile.depth;
}

quint32 BackgroundFilePS::originInData(const Tile &tile) const
{
	quint16 texID = tile.textureID - (tile.textureID2 ? headerEffect.x : headerImg.x)/64;
	quint32 dataStart = headerPalSize + 12 + (tile.textureID2 ? headerImg.size : 0);
	quint32 textureStart = texID * 128;
	quint32 tileStart = tile.srcY * textureWidth(tile) + tile.srcX * (tile.depth == 0 ? 0.5 : tile.depth);
	return dataStart + textureStart + tileStart;
}

QRgb BackgroundFilePS::directColor(quint16 color) const
{
	return PsColor::fromPsColor(color);
}

bool BackgroundFilePS::openPalettes(const QByteArray &data, QList<Palette *> &palettes)
{
	QBuffer palBuff;
	palBuff.setData(data);

	PaletteIOPS io(&palBuff);
	if(!io.read(palettes)) {
		return false;
	}

	return true;
}

bool BackgroundFilePS::openTiles(const QByteArray &data, qint64 *pos)
{
	Q_UNUSED(pos)

	BackgroundTiles tiles;
	QBuffer buff;
	buff.setData(data);

	BackgroundTilesIOPS io(&buff);
	if(!io.read(tiles)) {
		return false;
	}

	setTiles(tiles);

	return true;
}

QPixmap BackgroundFilePS::openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
	/*--- MIM OPENING ---*/
	QByteArray mimDataDec = ((FieldPS *)field())->io()->mimData(field());
	const char *constMimData = mimDataDec.constData();
	quint32 mimDataSize = mimDataDec.size();
	QList<Palette *> palettes;

	if(!openPalettes(mimDataDec, palettes)) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QPixmap();
	}

	/*i=0;
	foreach(const Palette *palette, palettes) {
		palette->toImage().scaled(128, 128, Qt::KeepAspectRatio)
				.save(QString("palettes/PS/palette_%1_%2_PS.png")
					  .arg(field()->name()).arg(i));
		((PalettePS *)palette)->toPC().toImage().scaled(128, 128, Qt::KeepAspectRatio)
				.save(QString("palettes/PS/palette_%1_%2_PS_to_PC.png")
					  .arg(field()->name()).arg(i));
		++i;
	}
	i=0;*/

	memcpy(&headerPalSize, constMimData, 4);

	if(mimDataSize < headerPalSize + 12) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QPixmap();
	}

	memcpy(&headerImg, constMimData + headerPalSize, 12);

	headerImg.w *= 2;

	if(headerPalSize+headerImg.size+12 <= mimDataSize) {
		memcpy(&headerEffect, constMimData + headerPalSize+headerImg.size, 12);
		headerEffect.w *= 2;
	} else {
		headerEffect.size = 4;
		headerEffect.w = 0;
		headerEffect.x = 0;
	}

	/*--- DAT OPENING ---*/

	if(!tilesAreCached() && !openTiles(field()->sectionData(Field::Background))) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QPixmap();
	}

	return drawBackground(tiles().tiles(paramActifs, z, layers), palettes, mimDataDec);
}
