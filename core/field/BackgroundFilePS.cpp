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

BackgroundFilePS::BackgroundFilePS(const BackgroundFilePS &other) :
	BackgroundFile(other)
{
	setTextures(new BackgroundTexturesPS(*(BackgroundTexturesPS *)other.textures()));
	PalettesPS palettes;
	foreach(Palette *pal, other.palettes()) {
		palettes.append(new PalettePS(*pal));
	}
	setPalettes(palettes);
}

bool BackgroundFilePS::open()
{
	if(isOpen() || isModified()) {
		setOpen(true);
		return true;
	}

	return open(((FieldPS *)field())->io()->mimData(field()),
				field()->sectionData(Field::Background));
}

bool BackgroundFilePS::open(const QByteArray &mimData, const QByteArray &tilesData)
{
	QBuffer mimBuff, tilesBuff;

	mimBuff.setData(mimData);
	tilesBuff.setData(tilesData);

	BackgroundIOPS io(&mimBuff, &tilesBuff);
	if(!io.read(*this)) {
		return false;
	}

	setOpen(true);

	return true;
}

BackgroundFilePC BackgroundFilePS::toPC(FieldPC *field) const
{
	PalettesPC palettesPC = ((PalettesPS *)&palettes())->toPC();
	BackgroundTiles tilesPC;
	BackgroundTexturesPC texturesPC = ((BackgroundTexturesPS *)textures())->toPC(tiles(), tilesPC, palettesPC);

	BackgroundFilePC filePC(field);
	filePC.setPalettes(palettesPC);
	filePC.setTextures(new BackgroundTexturesPC(texturesPC));
	filePC.setTiles(tilesPC);
	filePC.setOpen(true);

	return filePC;
}
