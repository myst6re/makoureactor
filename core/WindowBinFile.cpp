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
#include "WindowBinFile.h"
#include "GZIP.h"

#define LEFT_PADD(w)	(w >> 5)
#define CHAR_WIDTH(w)	(w & 0x1F)

WindowBinFile::WindowBinFile() :
	modified(false)
{
}

void WindowBinFile::clear()
{
	_charWidth.clear();
	_font.clear();
}

bool WindowBinFile::open(const QString &path)
{
	QFile windowFile(path);
	if(windowFile.open(QIODevice::ReadOnly)) {
		QByteArray windowData = windowFile.readAll();
		windowFile.close();
		if(!open(windowData)) {
			return false;
		}
	} else {
		return false;
	}

	return true;
}

bool WindowBinFile::open(const QByteArray &data)
{
	clear();

	const char *constData = data.constData();
	int cur=0;
	quint16 size;

	// Section 1 (icons texture)
	if(data.size() < 2) {
		return false;
	}

	memcpy(&size, constData, 2);

	// Section 2 (font texture)
	cur += 6 + size;

	if(data.size() < cur + 2) {
		return false;
	}

	memcpy(&size, constData + cur, 2);

	if(!openFont(GZIP::decompress(data.mid(cur + 6, size), 0))) {
		return false;
	}

	// Section 3 (font size)
	cur += 6 + size;

	if(data.size() < cur + 2) {
		return false;
	}

	memcpy(&size, constData + cur, 2);

	if(!openFontSize(GZIP::decompress(data.mid(cur + 6, size), 0))) {
		return false;
	}

	return true;
}

bool WindowBinFile::isValid() const
{
	return !_charWidth.isEmpty();
}

bool WindowBinFile::isModified() const
{
	return modified;
}

void WindowBinFile::setModified(bool modified)
{
	this->modified = modified;
}

int WindowBinFile::charCount() const
{
	return _charWidth.size();
}

int WindowBinFile::tableCount() const
{
	return 1;
}

bool WindowBinFile::openFont(const QByteArray &data)
{
	_font = TimFile(data);
	return _font.isValid() && _font.colorTableCount() == 16; //TODO: jp font
}

bool WindowBinFile::openFontSize(const QByteArray &data)
{
	if(data.size() != 1302) {
		return false;
	}

	_charWidth.resize(data.size());

	memcpy(_charWidth.data(), data.constData(), data.size());

	return true;
}

const QImage &WindowBinFile::image(FontColor color)
{
	setFontColor(color);
	return _font.image();
}

const QImage &WindowBinFile::image() const
{
	return _font.image();
}

QImage WindowBinFile::letter(quint8 table, quint8 id) const
{
	if(table != 0) {
		return QImage(); //TODO: jp font
	}

	return _font.image().copy((id % 21) * 12,
							  (id / 21) * 12,
							  12, 12);
}

void WindowBinFile::setFontColor(FontColor color)
{
	_font.setCurrentColorTable(palette(color)); //TODO: jp font
}

int WindowBinFile::palette(FontColor color) const
{
	return color * 2; //TODO: jp font
}

quint8 WindowBinFile::charWidth(quint8 table, quint8 id) const
{
	return CHAR_WIDTH(charInfo(table, id));
}

quint8 WindowBinFile::charLeftPadding(quint8 table, quint8 id) const
{
	return LEFT_PADD(charInfo(table, id));
}

void WindowBinFile::setCharWidth(quint8 table, quint8 id, quint8 width)
{
	setCharInfo(table, id, (charInfo(table, id) & 0xE0) | (width & 0x1F));
}

void WindowBinFile::setCharLeftPadding(quint8 table, quint8 id, quint8 padding)
{
	setCharInfo(table, id, (padding & 0xE0) | (charInfo(table, id) & 0x1F));
}
