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

ColorDisplay::ColorDisplay(QWidget *parent)
	: QWidget(parent)
{
	setFixedSize(101, 11);
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

void ColorDisplay::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	int size = colors.size(), x;
	painter.setPen(QColor(0, 0, 0, 0xFF));
	for(int i=0 ; i<size ; ++i) {
		x = i*10;
		painter.drawRect(x, 0, 10, 10);
		painter.fillRect(x+1, 1, 9, 9, colors.at(i));
	}
	painter.setPen(QColor(0xFF, 0, 0, 0xFF));
	QPoint cursor_position = this->mapFromGlobal(this->cursor().pos());
	x = (int)(cursor_position.x()/10)*10;
	if(x==this->width()-1)	x -= 10;
		painter.drawRect(x, 0, 10, 10);
	painter.end();
}

void ColorDisplay::enterEvent(QEvent *)
{
	update();
}

void ColorDisplay::leaveEvent(QEvent *)
{
	update();
}

void ColorDisplay::mouseMoveEvent(QMouseEvent *)
{
	update();
}

void ColorDisplay::mouseReleaseEvent(QMouseEvent *event)
{
	int colorIndex = event->x()/10;
	if(colorIndex >= colors.size())
		colorIndex = colors.size()-1;
	QColor color = QColorDialog::getColor(colors.at(colorIndex), this, tr("Choisir une nouvelle couleur"));
	if(color.isValid())
		colors.replace(colorIndex, color.rgb());
	update();
}
