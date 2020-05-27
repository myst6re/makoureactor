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

WindowBinFile::WindowBinFile() :
	modified(false)
{
}

void WindowBinFile::clear()
{
	_charWidth.clear();
	_font.clear();
	_font2.clear();
	_icons.clear();
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

	if(positions.size() < 3 || positions.size() > 4) {
		return false;
	}

	_icons = TimFile(GZIP::decompress(constData + positions.first() + 6, sizes.first(), 0));

	if(!_icons.isValid()) {
		return false;
	}

	if(!openFont(GZIP::decompress(constData + positions.at(1) + 6, sizes.at(1), 0))) {
		return false;
	}

	// jp version
	if(positions.size() == 4 && !openFont2(GZIP::decompress(constData + positions.at(2) + 6, sizes.at(2), 0))) {
		return false;
	}

	if(!openFontSize(GZIP::decompress(constData + positions.last() + 6, sizes.last(), 0))) {
		return false;
	}

	return true;
}

void WindowBinFile::saveSection(const QByteArray &section, QByteArray &data, quint16 type)
{
	QByteArray compressedData = GZIP::compress(section, 9);
	quint16 size = compressedData.size();
	data.append((char *)&size, 2);
	size = section.size();
	data.append((char *)&size, 2);
	data.append((char *)&type, 2);
	data.append(compressedData);
}

bool WindowBinFile::save(QByteArray &data) const
{
	QByteArray sectionData, compressedData;
	if(!_icons.save(sectionData)) {
		return false;
	}

	saveSection(sectionData, data, 0);

	sectionData.clear();
	if(!_font.save(sectionData)) {
		return false;
	}

	saveSection(sectionData, data, 0);

	if(isJp()) {
		sectionData.clear();
		if(!_font2.save(sectionData)) {
			return false;
		}

		saveSection(sectionData, data, 0);
	}

	saveSection(QByteArray((char *)_charWidth.data(), _charWidth.size()), data, 1);

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

int WindowBinFile::tableSize(quint8 table)
{
	return table == 0 || table == 2
			? 231  /* 21 * 11 */
			: 210; /* 21 * 10 */
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

QPoint WindowBinFile::letterPos(quint8 tableId, quint8 charId) const
{
	int id = charId;
	if(tableId >= 4) {
		if (!isJp()) {
			return QPoint();
		}
	} else {
		id = (tableId % 2) * 231 + charId;
	}

	QSize size = letterSize();
	return QPoint((id % 21) * size.width(), (id / 21) * size.height());
}

QRect WindowBinFile::letterRect(quint8 tableId, quint8 charId) const
{
	QPoint pos = letterPos(tableId, charId);
	if(pos.isNull()) {
		return QRect();
	}
	return QRect(pos, letterSize());
}

QImage WindowBinFile::letter(quint8 tableId, quint8 charId, FontColor color)
{
	QRect rect = letterRect(tableId, charId);
	if (rect.isNull()) {
		return QImage();
	}
	TimFile &f = font(tableId);
	f.setCurrentColorTable(palette(color, tableId));
	return f.image().copy(letterRect(tableId, charId));
}

bool WindowBinFile::setLetter(quint8 tableId, quint8 charId, const QImage &image)
{
	QRect rect = letterRect(tableId, charId);

	if(image.size() != rect.size()) {
		return false;
	}

	int x2, y2 = rect.y();

	for(int y=0 ; y<image.height() ; ++y) {
		x2 = rect.x();

		for(int x=0 ; x<image.width() ; ++x) {
			font(tableId).imagePtr()->setPixel(x2, y2, image.pixelIndex(x, y));
			++x2;
		}

		++y2;
	}

	modified = true;

	return true;
}

uint WindowBinFile::letterPixelIndex(quint8 tableId, quint8 charId, const QPoint &pos) const
{
	return constFont(tableId).image().pixel(letterPos(tableId, charId) + pos);
}

bool WindowBinFile::setLetterPixelIndex(quint8 tableId, quint8 charId, const QPoint &pos, uint pixelIndex)
{
	// pixelIndex must be a number between 0 and 15
	font(tableId).imagePtr()->setPixel(letterPos(tableId, charId) + pos, pixelIndex % 16);
	modified = true;
	return true;
}

int WindowBinFile::palette(FontColor color, quint8 table) const
{
	quint8 pal = table == 2 || table == 3 || table == 5;
	return color * 2 + pal;
}

int WindowBinFile::absoluteId(quint8 table, quint8 id)
{
	int absId = id;
	for(int i=0 ; i<table ; ++i) {
		absId += tableSize(i);
	}
	return absId;
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
