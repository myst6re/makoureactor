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
#pragma once

#include <QtCore>
#include <QRgb>
#include <QImage>

class Palette
{
public:
	Palette();
	explicit Palette(const char *data);
	virtual ~Palette();
	inline bool notZero(quint8 index) const { return !isZero(index); }
	virtual inline bool isZero(quint8 index) const {
		return _isZero.at(index);
	}
	virtual inline QRgb color(int index) const { return _colors.at(index); }
	inline bool mask(int index) const { return _masks.at(index); }
	inline void setColor(int index, QRgb color) { _colors.replace(index, color); }
	inline const QList<bool> &areZero() const {
		return _isZero;
	}
	void fromData(const char *data);
	QByteArray toByteArray() const;
	QImage toImage() const;
private:
	inline void addColor(QRgb color, bool mask, bool isZero) {
		_colors.append(color);
		_masks.append(mask);
		_isZero.append(isZero);
	}

	QList<QRgb> _colors;
	QList<bool> _masks;
	QList<bool> _isZero;
};

class PalettePC : public Palette
{
public:
	PalettePC();
	PalettePC(const Palette &palette, bool transparency=false);
	explicit PalettePC(const char *data, bool transparency=false);
	QRgb color(int index) const;
	bool isZero(quint8 index) const;
	bool transparency() const;
	void setTransparency(bool transparency);
private:
	bool _transparency;
};

typedef Palette PalettePS;

class PalettesPS;
class PalettesPC;

typedef QList<Palette *> Palettes;

class PalettesPS : public Palettes
{
public:
	PalettesPS();
	PalettesPC toPC() const;
};

class PalettesPC : public Palettes
{
public:
	PalettesPC();
	PalettesPS toPS() const;
};
