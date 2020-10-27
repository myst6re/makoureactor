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
#ifndef FONTDISPLAY_H
#define FONTDISPLAY_H

#include <QtWidgets>
#include "core/WindowBinFile.h"

class FontDisplay : public QWidget
{
	Q_OBJECT
public:
	explicit FontDisplay(QWidget *parent=0);
	virtual ~FontDisplay();
	WindowBinFile *windowBinFile() const;
	virtual void setWindowBinFile(WindowBinFile *windowBinFile);
	void clear();
	int currentTable() const;
	int currentLetter() const;
public slots:
	void setCurrentTable(int currentTable);
	void setColor(WindowBinFile::FontColor color);
	virtual void setLetter(int letter);
protected:
	static QPoint getCellPos(const QPoint &pos, const QSize &cellSize);
	static int getCell(const QPoint &pos, const QSize &cellSize, int colCount);
	WindowBinFile *_windowBinFile;
	WindowBinFile::FontColor _color;
	int _currentTable, _letter;
private:
	static int getLetter(QPoint pos);
	static QPoint getPos(int letter);
};

#endif // FONTDISPLAY_H
