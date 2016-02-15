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
	scale = qApp->desktop()->physicalDpiX()/96;
	setFixedSize((COLOR_DISPLAY_CELL_SIZE*scale + COLOR_DISPLAY_BORDER_WIDTH) * 10 + COLOR_DISPLAY_BORDER_WIDTH,
				 COLOR_DISPLAY_CELL_SIZE*scale + COLOR_DISPLAY_BORDER_WIDTH * 2);
	setMouseTracking(true);
}

void ColorDisplay::setColors(const QList<QRgb> &colors)
{
	this->colors = colors;
	update();
}

const QList<QRgb> &ColorDisplay::getColors() const
{
	return colors;
}

bool ColorDisplay::isReadOnly() const
{
	return _ro;
}

void ColorDisplay::setReadOnly(bool ro)
{
	_ro = ro;
}

void ColorDisplay::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	int gray;
	int size = colors.size(), x;
	// Colors
	painter.setPen(QColor(0, 0, 0));
	const int cellFullWidth = COLOR_DISPLAY_CELL_SIZE*scale + COLOR_DISPLAY_BORDER_WIDTH;
	for(int i=0 ; i<size ; ++i) {
		x = i * cellFullWidth;
		painter.drawRect(x, 0, cellFullWidth, cellFullWidth);
		gray = qGray(colors.at(i));
		painter.fillRect(x+COLOR_DISPLAY_BORDER_WIDTH,
						 COLOR_DISPLAY_BORDER_WIDTH,
						 COLOR_DISPLAY_CELL_SIZE*scale,
						 COLOR_DISPLAY_CELL_SIZE*scale,
						 isEnabled() ? QColor(colors.at(i)) : QColor(gray, gray, gray));
	}
	// Red frame
	if(isEnabled()) {
		painter.setPen(QColor(0xFF, 0, 0));
		QPoint cursor_position = this->mapFromGlobal(this->cursor().pos());
		x = colorId(cursor_position) * cellFullWidth;
		painter.drawRect(x, 0, cellFullWidth, cellFullWidth);
	}
	painter.end();
}

int ColorDisplay::colorId(const QPoint &pos) const
{
	return qMin(pos.x()/(COLOR_DISPLAY_CELL_SIZE *scale + COLOR_DISPLAY_BORDER_WIDTH), colors.size() - 1);
}

void ColorDisplay::enterEvent(QMouseEvent *event)
{
	update();
	emit colorHovered(colorId(event->pos()));
}

void ColorDisplay::leaveEvent(QMouseEvent *)
{
	update();
}

void ColorDisplay::mouseMoveEvent(QMouseEvent *event)
{
	update();
	emit colorHovered(colorId(event->pos()));
}

void ColorDisplay::mouseReleaseEvent(QMouseEvent *event)
{
	if(isReadOnly())	return;
	int colorIndex = colorId(event->pos());
	QColor color = QColorDialog::getColor(colors.at(colorIndex), this, tr("Choose a new color"));
	if(color.isValid()) {
		colors.replace(colorIndex, color.rgb());
		emit colorEdited(colorIndex, color.rgb());
	}
	update();
}
