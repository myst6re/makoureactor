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
	QList<quint16> positions, sizes;

	while(cur + 2 < data.size()) {
		positions.append(cur);

		memcpy(&size, constData + cur, 2);

		sizes.append(size);

		cur += 6 + size;
	}

	if(positions.size() == 3) {
		if(!openFont(GZIP::decompress(constData + positions.at(1) + 6, sizes.at(1), 0))) {
			return false;
		}

		if(!openFontSize(GZIP::decompress(constData + positions.last() + 6, sizes.last(), 0))) {
			return false;
		}
	} else if(positions.size() == 4) { // jp version
		if(!openFont(GZIP::decompress(constData + positions.at(1) + 6, sizes.at(1), 0))) {
			return false;
		}

		if(!openFont2(GZIP::decompress(constData + positions.at(2) + 6, sizes.at(2), 0))) {
			return false;
		}

		if(!openFontSize(GZIP::decompress(constData + positions.last() + 6, sizes.last(), 0))) {
			return false;
		}
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

bool WindowBinFile::isJp() const
{
	return _font2.isValid();
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
	return isJp() ? 6 : 4;
}

bool WindowBinFile::openFont(const QByteArray &data)
{
	_font = TimFile(data);
	return _font.isValid() && _font.colorTableCount() == 16;
}

bool WindowBinFile::openFont2(const QByteArray &data)
{
	_font2 = TimFile(data);
	return _font2.isValid() && _font2.colorTableCount() == 16;
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
	_font.setCurrentColorTable(palette(color, 0));//TODO: idk
	return _font.image();
}

QRect WindowBinFile::letterRect(int charId) const
{
	QSize size = QSize(12, 12);
	QPoint point = QPoint((charId%21)*size.width(), (charId/21)*size.height());

	return QRect(point, size);
}

QImage WindowBinFile::letter(quint8 table, quint8 id, FontColor color)
{
	if(table >= 4) {
		if(isJp()) {
			_font2.setCurrentColorTable(palette(color, table));
			return _font2.image().copy(letterRect(id));
		}
		return QImage();
	}
	_font.setCurrentColorTable(palette(color, table));
	return _font.image().copy(letterRect((table % 2) * 231 + id));
}

int WindowBinFile::palette(FontColor color, quint8 table) const
{
	quint8 pal = table == 2 || table == 3 || table == 5;
	return color * 2 + pal;
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
