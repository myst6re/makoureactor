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

bool BackgroundFilePC::openTextures(const QByteArray &data, BackgroundTexturesPC &textures)
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

QImage BackgroundFilePC::openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
	QByteArray data = field()->sectionData(Field::Background);
	QList<Palette *> palettes;
	BackgroundTexturesPC textures;

	if(!openPalettes(data, field()->sectionData(Field::PalettePC), palettes)) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QImage();
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

			return QImage();
		}
	}

	if(!openTextures(data, textures)) {
		foreach(Palette *palette, palettes) {
			delete palette;
		}

		return QImage();
	}

	QImage ret = drawBackground(tiles().tiles(paramActifs, z, layers), palettes, &textures);

	textures.clear();

	return ret;
}
