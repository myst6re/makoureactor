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

BackgroundFilePS::BackgroundFilePS(FieldPS *field) :
	BackgroundFile(field)
{
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

bool BackgroundFilePS::openTiles(const QByteArray &data)
{
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

bool BackgroundFilePS::openTextures(const QByteArray &data, BackgroundTexturesPS *textures) const
{
	QBuffer buff;
	buff.setData(data);

	BackgroundTexturesIOPS io(&buff);
	if(!io.read(textures)) {
		return false;
	}

	return true;
}

QImage BackgroundFilePS::openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
	QByteArray mimData = ((FieldPS *)field())->io()->mimData(field());
	QList<Palette *> palettes;
	BackgroundTexturesPS textures;

	if(!openPalettes(mimData, palettes)
			|| (tiles().isEmpty() &&
				!openTiles(field()->sectionData(Field::Background)))
			|| !openTextures(mimData, &textures)) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QImage();
	}

	return drawBackground(tiles().tiles(paramActifs, z, layers),
						  palettes, &textures);
}
