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
#include "Palette.h"
#include "../PsColor.h"

Palette::Palette() {}

PalettePC::PalettePC() : Palette() {}

PalettePS::PalettePS() : Palette() {}

PalettePC::PalettePC(const char *palette, quint8 transparency) :
	Palette(), _transparency(transparency)
{
	quint16 first, color;
	memcpy(&first, palette, 2);
	_colors.append(PsColor::fromPsColor(first));
//	_masks.append(first >> 15);
//	_isZero.append(first==0);

	palette += 2;

	for(quint16 i=1 ; i<256 ; ++i)
	{
		memcpy(&color, palette, 2);
		if(color==0) color = first;

		_colors.append(PsColor::fromPsColor(color));
//		_masks.append(color >> 15);
//		_isZero.append(color==0);

		palette += 2;
	}
}

PalettePS::PalettePS(const char *palette) :
	Palette()
{
	for(quint16 i=0 ; i<256 ; ++i)
	{
		quint16 color;
		memcpy(&color, palette, 2);

		_colors.append(PsColor::fromPsColor(color));
//		_masks.append(color >> 15);
		_isZero.append(color==0);

		palette += 2;
	}
}

QRgb Palette::color(int index) const
{
	return _colors.at(index);
}

const QList<QRgb> &Palette::colors() const
{
	return _colors;
}

bool PalettePC::notZero(quint8 index) const
{
	return index != 0 || !_transparency;
}

bool PalettePS::notZero(quint8 index) const
{
	return !_isZero.at(index);
}
