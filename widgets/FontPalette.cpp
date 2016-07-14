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
#include "FontPalette.h"

FontPalette::FontPalette(QWidget *parent) :
	QWidget(parent), colorCount(16), _currentColor(0),
	_currentPalette(WindowBinFile::White), readOnly(false), _windowBinFile(0)
{
}

FontPalette::~FontPalette()
{
}

QSize FontPalette::sizeHint() const
{
	return QSize(colorCount * (CELL_SIZE + BORDER_SIZE) + BORDER_SIZE, CELL_SIZE + 2 * BORDER_SIZE);
}

QSize FontPalette::minimumSizeHint() const
{
	return sizeHint();
}

void FontPalette::setReadOnly(bool ro)
{
	readOnly = ro;
}

void FontPalette::setColorCount(quint8 colorCount)
{
	this->colorCount = qMin(colorCount, quint8(16));
	setFixedSize(sizeHint());
}

void FontPalette::setWindowBinFile(WindowBinFile *windowBinFile)
{
	setColorCount(16);

	_windowBinFile = windowBinFile;
	setCurrentPalette(_currentPalette);
}

void FontPalette::setCurrentPalette(WindowBinFile::FontColor palette)
{
	_currentPalette = palette;
	copyPalette = _windowBinFile->image(_currentPalette).colorTable();
	this->palette = copyPalette;

	update();
}

void FontPalette::setColor(int id, QRgb color)
{
	palette.replace(id, color);

	update(BORDER_SIZE + id * (CELL_SIZE + BORDER_SIZE), BORDER_SIZE, CELL_SIZE, CELL_SIZE);
}

QRgb FontPalette::currentColor() const
{
	return palette.at(_currentColor);
}

void FontPalette::setCurrentColor(int id)
{
	_currentColor = id;
	emit colorChanged(_currentColor);

	update();
}

void FontPalette::clear()
{
	copyPalette.clear();
	palette.clear();
	update();
}

void FontPalette::reset()
{
	if(copyPalette.isEmpty())		return;
	palette = copyPalette;
	update();
}

void FontPalette::paintEvent(QPaintEvent *)
{
	QPainter p(this);

	if(isEnabled()) {
		p.fillRect(0, 0, width(), height(), Qt::black);
	}

	int id=0;

	foreach(const QRgb &color, palette) {
		if(!readOnly && id == _currentColor) {
			p.fillRect(QRect(id * (CELL_SIZE + BORDER_SIZE), 0, CELL_SIZE + 2 * BORDER_SIZE, CELL_SIZE + 2 * BORDER_SIZE), Qt::red);
		} else {
			p.setPen(Qt::black);
		}

		p.fillRect(BORDER_SIZE + id * (CELL_SIZE + BORDER_SIZE), BORDER_SIZE, CELL_SIZE, CELL_SIZE, QColor(color));

		++id;
	}
}

int FontPalette::getColorId(const QPoint &pos)
{
	return pos.x() / (CELL_SIZE + BORDER_SIZE);
}

void FontPalette::mousePressEvent(QMouseEvent *e)
{
	if(readOnly || palette.isEmpty())	return;

	setCurrentColor(getColorId(e->pos()));
}
