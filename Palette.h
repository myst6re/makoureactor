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
#ifndef DEF_PALETTE
#define DEF_PALETTE

#include <QtGui>
#define COEFF_COLOR	8.2258064516129032258064516129032 // 255/31

class PsColor
{
public:
	static quint16 toPsColor(const QRgb &color);
	static QRgb fromPsColor(quint16 color, bool useAlpha=false);
};

class Palette
{
public:
	Palette();
	explicit Palette(const char *palette);// PS
	Palette(const char *palette, quint8 transparency);// PC
	bool transparency;
	QList<QRgb> couleurs;
//	QList<bool> masks;
	QList<bool> isZero;
};

#endif
