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
#ifndef DEF_PSCOLOR
#define DEF_PSCOLOR

#include <QtCore>
#include <QRgb>
#define COEFF_COLOR	8.2258064516129032258064516129032 // 255/31
#define psColorAlphaBit(color) \
	((color >> 15) & 1)
#define setPsColorAlphaBit(color, alpha) \
	((color & 0x7FFF) | (alpha << 15))

class PsColor
{
public:
	static inline quint16 toPsColor(const QRgb &color) {
		return (qRound(qRed(color)/COEFF_COLOR) & 31) | ((qRound(qGreen(color)/COEFF_COLOR) & 31) << 5)
		       | ((qRound(qBlue(color)/COEFF_COLOR) & 31) << 10) | ((qAlpha(color)==255) << 15);
	}
	static inline QRgb fromPsColor(quint16 color, bool useAlpha=false) {
		quint8 r = color & 31,
		       g = (color >> 5) & 31,
		       b = (color >> 10) & 31;

		return qRgba((r << 3) + (r >> 2), (g << 3) + (g >> 2), (b << 3) + (b >> 2), color == 0 && useAlpha ? 0 : 255);
	}
};

#endif // DEF_PSCOLOR
