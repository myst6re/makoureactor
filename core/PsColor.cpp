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
#include "PsColor.h"

quint16 PsColor::toPsColor(const QRgb &color)
{
	return (qRound(qRed(color)/COEFF_COLOR) & 31) | ((qRound(qGreen(color)/COEFF_COLOR) & 31) << 5) | ((qRound(qBlue(color)/COEFF_COLOR) & 31) << 10) | ((qAlpha(color)==255) << 15);
}

QRgb PsColor::fromPsColor(quint16 color, bool useAlpha)
{
	return qRgba(qRound((color & 31)*COEFF_COLOR), qRound((color>>5 & 31)*COEFF_COLOR), qRound((color>>10 & 31)*COEFF_COLOR), color == 0 && useAlpha ? 0 : 255);
}
