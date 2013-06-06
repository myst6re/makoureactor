/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel J�r�me <myst6re@gmail.com>
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

bool BackgroundFilePS::openTextures(const QByteArray &data)
{
	QBuffer buff;
	buff.setData(data);

	BackgroundTexturesIOPS io(&buff);
	if(!io.read(&textures)) {
		return false;
	}

	return true;
}

QImage BackgroundFilePS::openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
	/*--- MIM OPENING ---*/
	QByteArray mimDataDec = ((FieldPS *)field())->io()->mimData(field());
	QList<Palette *> palettes;

	if(!openPalettes(mimDataDec, palettes)) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QImage();
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

	if(!openTextures(mimDataDec)) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QImage();
	}

	/*--- DAT OPENING ---*/

	if(tiles().isEmpty() && !openTiles(field()->sectionData(Field::Background))) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QImage();
	}

	QImage ret = drawBackground(tiles().tiles(paramActifs, z, layers), palettes, &textures);

	textures.clear();

	return ret;
}
