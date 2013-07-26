/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef WINDOWBINFILE_H
#define WINDOWBINFILE_H

#include <QtCore>
#include "TimFile.h"

#define LEFT_PADD(w)	(w >> 5)
#define CHAR_WIDTH(w)	(w & 0x1F)

class WindowBinFile
{
public:
	enum FontColor {
		DarkGrey=0,
		DarkBlue=1,
		Red=2,
		Purple=3,
		Green=4,
		Cyan=5,
		Yellow=6,
		White=7
	};

	WindowBinFile();
	void clear();
	bool open(const QString &path);
	bool open(const QByteArray &data);
	bool isValid() const;
	bool isModified() const;
	bool isJp() const;
	void setModified(bool modified);
	int charCount() const;
	int tableCount() const;
	static int tableSize(quint8 table);
	const QImage &image(FontColor color);
	QImage letter(quint8 table, quint8 id, FontColor color);
	quint8 charWidth(quint8 table, quint8 id) const;
	quint8 charLeftPadding(quint8 table, quint8 id) const;
	void setCharWidth(quint8 table, quint8 id, quint8 width);
	void setCharLeftPadding(quint8 table, quint8 id, quint8 padding);
private:
	QImage letter(int id, FontColor color);
	int palette(FontColor color, quint8 table) const;
	QRect letterRect(int charId) const;
	bool openFont(const QByteArray &data);
	bool openFont2(const QByteArray &data);
	bool openFontSize(const QByteArray &data);
	static int absoluteId(quint8 table, quint8 id);
	inline quint8 charInfo(quint8 table, quint8 id) const {
		return _charWidth.value(absoluteId(table, id));
	}
	inline void setCharInfo(quint8 table, quint8 id, quint8 info) {
		_charWidth.replace(absoluteId(table, id), info);
	}

	QVector<quint8> _charWidth;
	TimFile _font, _font2;
	bool modified;
};

#endif // WINDOWBINFILE_H
