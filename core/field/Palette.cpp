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

Palette::Palette()
{
}

Palette::Palette(const char *data)
{
	fromData(data);
}

Palette::~Palette()
{
}

void Palette::fromData(const char *data)
{
	for(quint16 i=0 ; i<256 ; ++i) {
		quint16 color;
		memcpy(&color, data, 2);

		addColor(PsColor::fromPsColor(color),
				 color >> 15, color == 0);

		data += 2;
	}
}

QByteArray Palette::toByteArray() const
{
	QByteArray data;
	quint8 colorId = 0;

	foreach(const QRgb color, _colors) {
		quint16 psColor = (PsColor::toPsColor(color) & 0x7FFF)
				| (_masks.at(colorId) << 15);
		data.append((char *)&psColor, 2);
		++colorId;
	}

	return data;
}

QImage Palette::toImage() const
{
	const quint8 size = 16;
	QImage image(size, size, QImage::Format_ARGB32);

	for(int y=0 ; y<size ; ++y) {
		for(int x=0 ; x<size ; ++x) {
			image.setPixel(x, y, color(y * size + x));
		}
	}

	return image;
}

PalettePC::PalettePC() :
	Palette(), _transparency(false)
{
}

PalettePC::PalettePC(const Palette &palette, bool transparency) :
	Palette(palette), _transparency(transparency)
{
}

PalettePC::PalettePC(const char *data, bool transparency) :
	Palette(data), _transparency(transparency)
{
}

QRgb PalettePC::color(int index) const
{
	if(Palette::isZero(index)) {
		return Palette::color(0);
	}
	return Palette::color(index);
}

bool PalettePC::isZero(quint8 index) const
{
	return index == 0 && _transparency;
}

bool PalettePC::transparency() const
{
	return _transparency;
}

void PalettePC::setTransparency(bool transparency)
{
	_transparency = transparency;
}

PalettesPC::PalettesPC()
{
}

PalettesPS PalettesPC::toPS() const
{
	PalettesPS palettesPS;

	foreach(Palette *palette, *this) {
		palettesPS.append(new PalettePS(*palette));
	}

	return palettesPS;
}

PalettesPS::PalettesPS()
{
}

PalettesPC PalettesPS::toPC() const
{
	PalettesPC palettesPC;

	foreach(Palette *palette, *this) {
		palettesPC.append(new PalettePC(*palette));
	}

	return palettesPC;
}
