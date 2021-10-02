/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#include "ApercuBGLabel.h"

ApercuBGLabel::ApercuBGLabel(QWidget *parent)
	: QLabel(parent), _showSave(false)
{
}

void ApercuBGLabel::paintEvent(QPaintEvent *e)
{
	QLabel::paintEvent(e);

	if (isEnabled() && _showSave) {
		QPainter painter(this);
		painter.drawPixmap(0, 0, QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton).pixmap(32));
		painter.end();
	}
}

void ApercuBGLabel::enterEvent(QEvent *e)
{
	QLabel::enterEvent(e);

	_showSave = true;
	update(0, 0, 32, 32);
}

void ApercuBGLabel::leaveEvent(QEvent *e)
{
	QLabel::leaveEvent(e);

	_showSave = false;
	update(0, 0, 32, 32);
}

void ApercuBGLabel::mousePressEvent(QMouseEvent *e)
{
	QLabel::mousePressEvent(e);

	if (e->button() == Qt::LeftButton &&
	        e->x() >= 0 && e->x() < 32 &&
	        e->y() >= 0 && e->y() < 32) {
		emit saveRequested();
	}
}
