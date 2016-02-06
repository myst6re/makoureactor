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
	BackgroundFile(field)
{
}

BackgroundFilePC::BackgroundFilePC(const BackgroundFilePC &other) :
	BackgroundFile(other)
{
	setTextures(new BackgroundTexturesPC(*(BackgroundTexturesPC *)other.textures()));
	PalettesPC palettes;
	foreach(Palette *pal, other.palettes()) {
		palettes.append(new PalettePC(*pal));
	}
	setPalettes(palettes);
}

bool BackgroundFilePC::open()
{
	if(isOpen() || isModified()) {
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
	BackgroundTexturesPS texturesPS = ((BackgroundTexturesPC *)textures())->toPS(tiles(), tilesPS, palettesPS);

	BackgroundFilePS filePS(field);
	filePS.setPalettes(palettesPS);
	filePS.setTextures(new BackgroundTexturesPS(texturesPS));
	filePS.setTiles(tilesPS);
	filePS.setOpen(true);

	return filePS;
}
