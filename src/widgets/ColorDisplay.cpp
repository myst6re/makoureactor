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
#include "ColorDisplay.h"

ColorDisplay::ColorDisplay(QWidget *parent) :
	QWidget(parent), _ro(false), _hover(false)
{
}

QSize ColorDisplay::sizeHint() const
{
	return minimumSizeHint();
}

QSize ColorDisplay::minimumSizeHint() const
{

	const qreal scale = devicePixelRatio();
	int colorCount = qMax(1, _colors.size());
	return QSize((COLOR_DISPLAY_MIN_CELL_SIZE * scale
	             + COLOR_DISPLAY_BORDER_WIDTH) * colorCount
	             + COLOR_DISPLAY_BORDER_WIDTH,
	             COLOR_DISPLAY_MIN_CELL_SIZE * scale
	             + COLOR_DISPLAY_BORDER_WIDTH * 2);
}

void ColorDisplay::setColors(const QList<QRgb> &colors)
{
	_colors = colors;
	setMouseTracking(_colors.size() > 1);
	update();
	if (!_colors.isEmpty()) {
		setToolTip(QColor(_colors.first()).name());
	}
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

int ColorDisplay::cellWidth(int width) const
{
	if (_colors.isEmpty()) {
		return COLOR_DISPLAY_MIN_CELL_SIZE;
	}
	QSize minSize = minimumSizeHint();
	// Ensure current widget width respect minimum size hint
	int w = qMax(width, minSize.width());

	return (w - (_colors.size() + 1) * COLOR_DISPLAY_BORDER_WIDTH)
	        / _colors.size();
}

int ColorDisplay::cellHeight(int height) const
{
	if (_colors.isEmpty()) {
		return COLOR_DISPLAY_MIN_CELL_SIZE;
	}
	QSize minSize = minimumSizeHint();
	// Ensure current widget width respect minimum size hint
	int h = qMax(height, minSize.height());

	return h - 2 * COLOR_DISPLAY_BORDER_WIDTH;
}

QSize ColorDisplay::cellSize() const
{
	return QSize(cellWidth(width()), cellHeight(height()));
}

void ColorDisplay::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)

	QPainter painter(this);
	painter.setPen(palette().color(QPalette::Shadow));

	// Colors
	const QSize cellS = cellSize();
	const int size = qMax(_colors.size(), 1),
	          cellFullWidth = cellS.width() + COLOR_DISPLAY_BORDER_WIDTH,
	          cellFullHeight = cellS.height() + COLOR_DISPLAY_BORDER_WIDTH;

	for (int i = 0; i < size; ++i) {
		const int x = i * cellFullWidth;
		painter.drawRect(x, 0, cellFullWidth, cellFullHeight);
		QColor color;
		if (_colors.isEmpty()) {
			color = Qt::transparent;
		} else if (isEnabled()) {
			color = QColor(_colors.at(i));
		} else {
			const int gray = qGray(_colors.at(i));
			color = QColor(gray, gray, gray);
		}
		painter.fillRect(x + COLOR_DISPLAY_BORDER_WIDTH,
		                 COLOR_DISPLAY_BORDER_WIDTH,
		                 cellS.width(), cellS.height(), color);
	}

	// Red frame
	if (isEnabled() && !isReadOnly() && _hover) {
		painter.setPen(palette().color(QPalette::Mid));
		const QPoint cursorPos = mapFromGlobal(cursor().pos());
		const int x = colorId(cursorPos) * cellFullWidth;
		painter.drawRect(x, 0, cellFullWidth, cellFullHeight);
	}
}

int ColorDisplay::colorId(const QPoint &pos) const
{
	return qMin(pos.x() / (cellWidth() + COLOR_DISPLAY_BORDER_WIDTH),
	            _colors.size() - 1);
}

void ColorDisplay::enterEvent(QEnterEvent *event)
{
	Q_UNUSED(event)
	_hover = true;
	update();
}

void ColorDisplay::leaveEvent(QEvent *event)
{
	Q_UNUSED(event)
	_hover = false;
	update();
}

void ColorDisplay::mouseMoveEvent(QMouseEvent *event)
{
	update();
	emit colorHovered(colorId(event->pos()));
}

void ColorDisplay::mouseReleaseEvent(QMouseEvent *event)
{
	if (isReadOnly()) {
		return;
	}

	const int colorIndex = colorId(event->pos());
	if (colorIndex >= _colors.size()) {
		return;
	}
	QColor color = QColorDialog::getColor(_colors.at(colorIndex), this,
	                                      tr("Choose a new color"));
	if (color.isValid()) {
		_colors.replace(colorIndex, color.rgb());
		emit colorEdited(colorIndex, color.rgb());
	}
	update();
}
