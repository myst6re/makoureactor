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
#include "ColorDisplay.h"

ColorDisplay::ColorDisplay(QWidget *parent) :
	QWidget(parent), _ro(false)
{
	setMouseTracking(true);
}

QSize ColorDisplay::sizeHint() const
{
	return minimumSizeHint();
}

QSize ColorDisplay::minimumSizeHint() const
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
	const int scale = 1;
#else
	const int scale = devicePixelRatio();
#endif
	return QSize((COLOR_DISPLAY_MIN_CELL_SIZE * scale + COLOR_DISPLAY_BORDER_WIDTH) * _colors.size() + COLOR_DISPLAY_BORDER_WIDTH,
	             COLOR_DISPLAY_MIN_CELL_SIZE * scale + COLOR_DISPLAY_BORDER_WIDTH * 2);
}

int ColorDisplay::heightForWidth(int w) const
{
	return cellSize(w) + COLOR_DISPLAY_BORDER_WIDTH * 2;
}

void ColorDisplay::setColors(const QList<QRgb> &colors)
{
	_colors = colors;
	setFixedSize(minimumSizeHint());
	update();
}

const QList<QRgb> &ColorDisplay::colors() const
{
	return _colors;
}

bool ColorDisplay::isReadOnly() const
{
	return _ro;
}

void ColorDisplay::setReadOnly(bool ro)
{
	_ro = ro;
}

int ColorDisplay::cellSize(int width) const
{
	if(_colors.isEmpty()) {
		return COLOR_DISPLAY_MIN_CELL_SIZE;
	}
	QSize minSize = minimumSizeHint();
	int w = qMax(width, minSize.width()); // Ensure current widget width respect minimum size hint

	return (w - (_colors.size() + 1) * COLOR_DISPLAY_BORDER_WIDTH) / _colors.size();
}

void ColorDisplay::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)

	QPainter painter(this);
	painter.setPen(Qt::black);

	// Colors
	const int size = _colors.size(),
	          cellWidth = cellSize(),
	          cellFullWidth = cellWidth + COLOR_DISPLAY_BORDER_WIDTH;

	for(int i = 0 ; i < size ; ++i) {
		const int x = i * cellFullWidth;
		painter.drawRect(x, 0, cellFullWidth, cellFullWidth);
		const int gray = qGray(_colors.at(i));
		painter.fillRect(x + COLOR_DISPLAY_BORDER_WIDTH,
		                 COLOR_DISPLAY_BORDER_WIDTH,
		                 cellWidth, cellWidth,
		                 isEnabled() ? QColor(_colors.at(i))
		                             : QColor(gray, gray, gray));
	}

	// Red frame
	if(isEnabled() && !isReadOnly()) {
		painter.setPen(Qt::red);
		const QPoint cursorPos = mapFromGlobal(cursor().pos());
		const int x = colorId(cursorPos) * cellFullWidth;
		painter.drawRect(x, 0, cellFullWidth, cellFullWidth);
	}
}

int ColorDisplay::colorId(const QPoint &pos) const
{
	return qMin(pos.x() / (cellSize() + COLOR_DISPLAY_BORDER_WIDTH), _colors.size() - 1);
}

void ColorDisplay::enterEvent(QEvent *event)
{
	Q_UNUSED(event);
	update();
}

void ColorDisplay::leaveEvent(QEvent *event)
{
	Q_UNUSED(event);
	update();
}

void ColorDisplay::mouseMoveEvent(QMouseEvent *event)
{
	update();
	emit colorHovered(colorId(event->pos()));
}

void ColorDisplay::mouseReleaseEvent(QMouseEvent *event)
{
	if(isReadOnly()) {
		return;
	}

	const int colorIndex = colorId(event->pos());
	if(colorIndex >= _colors.size()) {
		return;
	}
	QColor color = QColorDialog::getColor(_colors.at(colorIndex), this, tr("Choose a new color"));
	if(color.isValid()) {
		_colors.replace(colorIndex, color.rgb());
		emit colorEdited(colorIndex, color.rgb());
	}
	update();
}
