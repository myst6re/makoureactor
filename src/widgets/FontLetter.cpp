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
#include "FontLetter.h"

FontLetter::FontLetter(QWidget *parent) :
	FontDisplay(parent), _pixelIndex(0), readOnly(false), startDrag(false),
	startDrag2(false)
{
	setFixedSize(sizeHint());
	setMouseTracking(true);
}

FontLetter::~FontLetter()
{
}

QSize FontLetter::sizeHint() const
{
	return QSize(16*PIXEL_SIZE, 12*PIXEL_SIZE);
}

QSize FontLetter::minimumSizeHint() const
{
	return sizeHint();
}

void FontLetter::setReadOnly(bool ro)
{
	readOnly = ro;
	setMouseTracking(!readOnly);
}

void FontLetter::setPixelIndex(int index)
{
	_pixelIndex = index;
}

void FontLetter::setWindowBinFile(WindowBinFile *windowBinFile)
{
	if (windowBinFile) {
		copyLetter = windowBinFile->letter(_currentTable, _letter, _color);
	}
	FontDisplay::setWindowBinFile(windowBinFile);
}

void FontLetter::setLetter(quint8 letter)
{
	if (_windowBinFile) {
		copyLetter = _windowBinFile->letter(_currentTable, letter, _color);
	}
	FontDisplay::setLetter(letter);
}

void FontLetter::reset()
{
	if (copyLetter.isNull() || !_windowBinFile)		return;
	_windowBinFile->setLetter(_currentTable, _letter, copyLetter);
	update();
}

void FontLetter::paintEvent(QPaintEvent *)
{
	QPainter p(this);

	if (isEnabled()) {
		p.setBrush(Qt::black);
		p.drawRect(0, 0, width(), height());
	}

	if (_windowBinFile) {
		QImage letter = _windowBinFile->letter(_currentTable, _letter, _color);
		if (!letter.isNull()) {
			p.drawImage(QPoint(0, 0), letter.scaled(QSize(12*PIXEL_SIZE, 12*PIXEL_SIZE), Qt::KeepAspectRatio));
			int linePos = _windowBinFile->charWidth(_currentTable, _letter) * PIXEL_SIZE;
			p.setPen(Qt::red);
			p.drawLine(QPoint(linePos, 0), QPoint(linePos, height()));
		}
	}
}

QPoint FontLetter::getPixel(const QPoint &pos)
{
	return getCellPos(pos, QSize(PIXEL_SIZE, PIXEL_SIZE));
}

bool FontLetter::setPixel(const QPoint &pixel)
{
	if (!_windowBinFile)	return false;

	if (pixel.x() >= 0 && pixel.y() >= 0 && pixel.x() < 12 && pixel.y() < 12
			&& _windowBinFile->setLetterPixelIndex(_currentTable, _letter, pixel, _pixelIndex)) {
		update(QRect(pixel * PIXEL_SIZE, QSize(PIXEL_SIZE, PIXEL_SIZE)));
		emit imageChanged(QRect(pixel, QSize(1, 1)));
		return true;
	}

	return false;
}

void FontLetter::mouseMoveEvent(QMouseEvent *e)
{
	if (readOnly || !_windowBinFile)	return;

	const QPoint &mousePos = e->pos();
	int linePos = _windowBinFile->charWidth(_currentTable, _letter) * PIXEL_SIZE;

	if (startDrag) {
		int newLinePos = mousePos.x() / PIXEL_SIZE;
		if (linePos / PIXEL_SIZE != newLinePos && newLinePos < 16) {
			_windowBinFile->setCharWidth(_currentTable, _letter, quint8(newLinePos));
			update();
			emit widthEdited(newLinePos);
		}
	} else if (startDrag2) {
		setPixel(getPixel(mousePos));
	} else {
		if (mousePos.x() >= linePos - 1 && mousePos.x() <= linePos + 1) {
			if (cursor().shape() != Qt::SplitHCursor) {
				setCursor(Qt::SplitHCursor);
			}
		} else {
			if (mousePos.x() < 12 * PIXEL_SIZE && mousePos.y() < 12 * PIXEL_SIZE) {
				if (cursor().shape() != Qt::PointingHandCursor) {
					setCursor(Qt::PointingHandCursor);
				}
			} else if (cursor().shape() != Qt::ArrowCursor) {
				setCursor(Qt::ArrowCursor);
			}
		}
	}
}

void FontLetter::mousePressEvent(QMouseEvent *e)
{
	if (readOnly || !_windowBinFile)	return;

	QPoint pixel = getPixel(e->pos());

	int linePos = _windowBinFile->charWidth(_currentTable, _letter) * PIXEL_SIZE;

	if (e->pos().x() >= linePos - 1 && e->pos().x() <= linePos + 1) {
		startDrag = true;
	} else if (setPixel(pixel)) {
		startDrag2 = true;
	}
}

void FontLetter::mouseReleaseEvent(QMouseEvent *)
{
	if (readOnly)	return;

	startDrag = startDrag2 = false;
}
