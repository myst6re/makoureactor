/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel J�r�me <myst6re@gmail.com>
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

ApercuBG::ApercuBG(QWidget *parent)
	: QScrollArea(parent), archive(0), field(0), error(false)
{
	setAlignment(Qt::AlignCenter);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QPalette pal = palette();
	pal.setColor(QPalette::Active, QPalette::Window, Qt::black);
	pal.setColor(QPalette::Inactive, QPalette::Window, Qt::black);
	pal.setColor(QPalette::Disabled, QPalette::Window, pal.color(QPalette::Disabled, QPalette::Text));
	setPalette(pal);
}

void ApercuBG::fill(FieldArchive *archive, Field *field)
{
	this->archive = archive;
	this->field = field;

	QPixmap background;

	if(archive->isLgp()) {
		background = ((FieldPC *)field)->openModelAndBackground(archive->getFieldData(field));
	} else {
		background = ((FieldPS *)field)->openModelAndBackground(archive->getMimData(field), archive->getFieldData(field));
	}

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
	QLabel *label = new QLabel;
	if(newBg.width()>w || newBg.height()>h)
		label->setPixmap(newBg.scaled(w, h, Qt::KeepAspectRatio));
	else
		label->setPixmap(newBg);
	setWidget(label);
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
	setWidget(new QWidget);
	setCursor(Qt::ArrowCursor);
	archive = 0;
	field = 0;
	error = false;
}

void ApercuBG::execDialog()
{
	if(!error && archive && field) {
		BGDialog dialog(archive, field, this);
		dialog.exec();
	}
}

void ApercuBG::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
	{
		execDialog();
	}
}
