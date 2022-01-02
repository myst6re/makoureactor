/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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
#include "FontDisplay.h"

FontDisplay::FontDisplay(QWidget *parent) :
    QWidget(parent), _windowBinFile(nullptr), _color(WindowBinFile::White),
    _currentTable(0), _letter(0)
{
}

FontDisplay::~FontDisplay()
{
}

WindowBinFile *FontDisplay::windowBinFile() const
{
	return _windowBinFile;
}

void FontDisplay::setWindowBinFile(WindowBinFile *windowBinFile)
{
	_windowBinFile = windowBinFile;
	update();
}

void FontDisplay::clear()
{
	_windowBinFile = nullptr;
	update();
}

quint8 FontDisplay::currentTable() const
{
	return _currentTable;
}

void FontDisplay::setCurrentTable(quint8 currentTable)
{
	_currentTable = currentTable;
	update();
}

quint8 FontDisplay::currentLetter() const
{
	return _letter;
}

void FontDisplay::setColor(WindowBinFile::FontColor color)
{
	_color = color;
	update();
}

void FontDisplay::setLetter(quint8 letter)
{
	_letter = letter;
	update();
}

QPoint FontDisplay::getCellPos(const QPoint &pos, const QSize &cellSize)
{
	return QPoint(pos.x() / cellSize.width(), pos.y() / cellSize.height());
}

int FontDisplay::getCell(const QPoint &pos, const QSize &cellSize, int colCount)
{
	QPoint cellPos = getCellPos(pos, cellSize);

	return cellPos.y() * colCount + cellPos.x();
}
