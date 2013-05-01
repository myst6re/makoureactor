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

bool ColorDisplay::isReadOnly() const
{
	return _ro;
}

void ColorDisplay::setReadOnly(bool ro)
{
	setMouseTracking(!ro);
	_ro = ro;
}

void ColorDisplay::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	int gray;
	int size = colors.size(), x;
	// Colors
	painter.setPen(QColor(0, 0, 0));
	for(int i=0 ; i<size ; ++i) {
		x = i*10;
		painter.drawRect(x, 0, 10, 10);
		gray = qGray(colors.at(i));
		painter.fillRect(x+1, 1, 9, 9, isEnabled() ? QColor(colors.at(i)) : QColor(gray, gray, gray));
	}
	// Red frame
	if(isEnabled() && !isReadOnly()) {
		painter.setPen(QColor(0xFF, 0, 0));
		QPoint cursor_position = this->mapFromGlobal(this->cursor().pos());
		x = (int)(cursor_position.x()/10)*10;
		if(x==this->width()-1)	x -= 10;
			painter.drawRect(x, 0, 10, 10);
	}
	painter.end();
}

void ColorDisplay::enterEvent(QEvent *)
{
	if(isReadOnly())	return;
	update();
}

void ColorDisplay::leaveEvent(QEvent *)
{
	if(isReadOnly())	return;
	update();
}

void ColorDisplay::mouseMoveEvent(QMouseEvent *)
{
	if(isReadOnly())	return;
	update();
}

void ColorDisplay::mouseReleaseEvent(QMouseEvent *event)
{
	if(isReadOnly())	return;
	int colorIndex = event->x()/10;
	if(colorIndex >= colors.size())
		colorIndex = colors.size()-1;
	QColor color = QColorDialog::getColor(colors.at(colorIndex), this, tr("Choisir une nouvelle couleur"));
	if(color.isValid()) {
		colors.replace(colorIndex, color.rgb());
		emit colorEdited(colorIndex, color.rgb());
	}
	update();
}
