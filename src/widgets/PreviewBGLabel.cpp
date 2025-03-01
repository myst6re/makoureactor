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
#include "PreviewBGLabel.h"

PreviewBGLabel::PreviewBGLabel(QWidget *parent)
	: QLabel(parent), _showSave(false)
{
}

void PreviewBGLabel::paintEvent(QPaintEvent *e)
{
	QLabel::paintEvent(e);

	if (isEnabled() && _showSave) {
		QPainter painter(this);
		painter.drawPixmap(0, 0, QIcon::fromTheme(QStringLiteral("document-save")).pixmap(32));
		painter.end();
	}
}

void PreviewBGLabel::enterEvent(QEnterEvent *e)
{
	QLabel::enterEvent(e);

	_showSave = true;
	update(0, 0, 32, 32);
}

void PreviewBGLabel::leaveEvent(QEvent *e)
{
	QLabel::leaveEvent(e);

	_showSave = false;
	update(0, 0, 32, 32);
}

void PreviewBGLabel::mousePressEvent(QMouseEvent *e)
{
	QLabel::mousePressEvent(e);
	QPointF pos = e->position();

	if (e->button() == Qt::LeftButton &&
	        pos.x() >= 0.0 && pos.x() < 32.0 &&
	        pos.y() >= 0.0 && pos.y() < 32.0) {
		emit saveRequested();
	}
}
