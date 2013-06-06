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
#include "ApercuBG.h"

ApercuBG::ApercuBG(QWidget *parent) :
	QLabel(parent), field(0), error(false)
{
	setAutoFillBackground(true);
	setAlignment(Qt::AlignCenter);
	QPalette pal = palette();
	pal.setColor(QPalette::Active, QPalette::Window, Qt::black);
	pal.setColor(QPalette::Inactive, QPalette::Window, Qt::black);
	pal.setColor(QPalette::Disabled, QPalette::Window, pal.color(QPalette::Disabled, QPalette::Text));
	setPalette(pal);
}

void ApercuBG::fill(Field *field, bool reload)
{
	if(!reload && this->field == field)		return;

	this->field = field;

	QPixmap background = QPixmap::fromImage(field->background()->openBackground());

	QPixmap newBg;
	int w=width()-2, h=height()-2;

	if(background.isNull()) {
		newBg = errorPixmap(w, h);
		setCursor(Qt::ArrowCursor);
		error = true;
	} else {
		newBg = background;
		setCursor(Qt::PointingHandCursor);
		error = false;
	}

	if(newBg.width()>w || newBg.height()>h)
		setPixmap(newBg.scaled(w, h, Qt::KeepAspectRatio));
	else
		setPixmap(newBg);
}

QPixmap ApercuBG::errorPixmap(int w, int h)
{
	QPixmap errorPix(w, h);
	errorPix.fill(QColor(0,0,0));
	QFont font;
	font.setPixelSize(44);
	QString text = tr("Erreur");
	int textWidth = QFontMetrics(font).width(text);
	int textHeight = QFontMetrics(font).height();

	QPainter p(&errorPix);
	p.setPen(QColor(0xFF,0xFF,0xFF));
	p.setFont(font);
	p.drawStaticText((w-textWidth)/2, (h-textHeight)/2, QStaticText(text));
	p.end();

	return errorPix;
}

void ApercuBG::clear()
{
	QLabel::clear();
	setCursor(Qt::ArrowCursor);
	field = 0;
	error = false;
}

void ApercuBG::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton && !error)
	{
		emit clicked();
	}
}
