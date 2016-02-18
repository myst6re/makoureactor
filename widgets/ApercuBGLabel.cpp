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
#include "ApercuBGLabel.h"
#include "core/Config.h"

ApercuBGLabel::ApercuBGLabel(QWidget *parent)
	: QLabel(parent), showSave(false)
{
}

ApercuBGLabel::ApercuBGLabel(const QString &name, QWidget *parent)
	: QLabel(parent), _name(name), showSave(false)
{
}

void ApercuBGLabel::setName(const QString &name)
{
	_name = name;
}

void ApercuBGLabel::paintEvent(QPaintEvent *e)
{
	QLabel::paintEvent(e);

	if(isEnabled() && showSave) {
		QPainter painter(this);
		painter.drawPixmap(0, 0, QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton).pixmap(32));
		painter.end();
	}
}

void ApercuBGLabel::enterEvent(QEvent *)
{
	showSave = true;
	update(0, 0, 32, 32);
}

void ApercuBGLabel::leaveEvent(QEvent *)
{
	showSave = false;
	update(0, 0, 32, 32);
}

void ApercuBGLabel::mousePressEvent(QMouseEvent *event)
{
	if(event->button() != Qt::LeftButton)	return;

	if(event->x()>=0 && event->x()<32 && event->y()>=0 && event->y()<32) {
		savePixmap();
	}
}

void ApercuBGLabel::savePixmap()
{
	QString path = Config::value("saveBGPath").toString();
	if(!path.isEmpty()) {
		path.append("/");
	}
	path = QFileDialog::getSaveFileName(this, tr("Save Background"), path + _name + ".png", tr("PNG image (*.png);;JPG image (*.jpg);;BMP image (*.bmp);;Portable Pixmap (*.ppm)"));
	if(path.isNull())	return;

	pixmap()->save(path);

	int index = path.lastIndexOf('/');
	Config::setValue("saveBGPath", index == -1 ? path : path.left(index));
}
