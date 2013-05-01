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

#include <QtCore>
#include <QRgb>

class Palette
{
public:
	Palette();
	virtual ~Palette();
	virtual bool notZero(quint8 index) const=0;
	QRgb color(int index) const;
	const QList<QRgb> &colors() const;
protected:
	QList<QRgb> _colors;
//	QList<bool> _masks;
};

class PalettePC : public Palette
{
public:
	PalettePC();
	PalettePC(const char *palette, quint8 transparency);
	bool notZero(quint8 index) const;
private:
	bool _transparency;
};

class PalettePS : public Palette
{
public:
	PalettePS();
	explicit PalettePS(const char *palette);
	bool notZero(quint8 index) const;
private:
	QList<bool> _isZero;
};

#endif
