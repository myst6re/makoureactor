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
#include <QImage>

class Palette
{
public:
	Palette();
	virtual ~Palette();
	inline bool notZero(quint8 index) const { return !isZero(index); }
	virtual bool isZero(quint8 index) const=0;
	inline QRgb color(int index) const { return _colors.at(index); }
	inline const QList<QRgb> &colors() const { return _colors; }
	inline void addColor(QRgb color) { _colors.append(color); }
	inline void setColor(int index, QRgb color) { _colors.replace(index, color); }
	inline void insertColor(int index, QRgb color) { _colors.insert(index, color); }
	inline void removeColor(int index) { _colors.removeAt(index); }
	inline void setColors(const QList<QRgb> &colors) { _colors = colors; }
	QByteArray toByteArray() const;
	QImage toImage() const;
private:
	QList<QRgb> _colors;
//	QList<bool> _masks;
};

class PalettePS;
class PalettePC;

class PalettePC : public Palette
{
public:
	PalettePC();
	explicit PalettePC(const char *palette, bool transparency=false);
	bool isZero(quint8 index) const;
	bool transparency() const;
	void setTransparency(bool transparency);
	PalettePS toPS() const;
private:
	bool _transparency;
};

class PalettePS : public Palette
{
public:
	PalettePS();
	explicit PalettePS(const char *palette);
	bool isZero(quint8 index) const;
	void setIsZero(int index, bool transparency);
	PalettePC toPC() const;
private:
	QList<bool> _isZero;
};

#endif
