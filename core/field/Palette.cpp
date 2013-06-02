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

Palette::~Palette()
{
}

PalettePC::PalettePC() :
	Palette(), _transparency(false)
{
}

PalettePS::PalettePS() :
  Palette()
{
}

PalettePC::PalettePC(const char *palette, bool transparency) :
	Palette(), _transparency(transparency)
{
	quint16 first, color;
	memcpy(&first, palette, 2);
	addColor(PsColor::fromPsColor(first));
//	_masks.append(first >> 15);
//	_isZero.append(first==0);

	palette += 2;

	for(quint16 i=1 ; i<256 ; ++i) {
		memcpy(&color, palette, 2);
		if(color==0) color = first;

		addColor(PsColor::fromPsColor(color));
//		_masks.append(color >> 15);
//		_isZero.append(color==0);

		palette += 2;
	}
}

PalettePS::PalettePS(const char *palette) :
	Palette()
{
	for(quint16 i=0 ; i<256 ; ++i) {
		quint16 color;
		memcpy(&color, palette, 2);

		addColor(PsColor::fromPsColor(color));
//		_masks.append(color >> 15);
		_isZero.append(color==0);

		palette += 2;
	}
}

QByteArray Palette::toByteArray() const
{
	QByteArray data;

	foreach(const QRgb color, _colors) {
		quint16 psColor = PsColor::toPsColor(color);
		data.append((char *)&psColor, 2);
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

PalettePS PalettePC::toPS() const
{
	PalettePS palPS;

	for(int index=0 ; index<256 ; ++index) {
		palPS.setIsZero(index, isZero(index));
		palPS.addColor(color(index));
	}

	return palPS;
}

bool PalettePS::isZero(quint8 index) const
{
	return _isZero.at(index);
}

void PalettePS::setIsZero(int index, bool transparency)
{
	if(_isZero.size() != 256) {
		for(int i=0 ; i<256 ; ++i) {
			_isZero.append(i == index);
		}
	} else {
		_isZero.replace(index, transparency);
	}
}

PalettePC PalettePS::toPC() const
{
	PalettePC palPC;

	for(quint16 index=0 ; index<256 ; ++index) {
		if(isZero(index)) {
			palPC.setTransparency(true);// TODO: real conversion?
		}
		palPC.addColor(color(index));
	}

	return palPC;
}
