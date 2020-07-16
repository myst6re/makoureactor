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
	QLabel(parent), _field(nullptr), _error(false)
{
	setAutoFillBackground(true);
	setAlignment(Qt::AlignCenter);
	setBackgroundRole(QPalette::Dark);

	QPalette pal = palette();
	pal.setColor(QPalette::Active, QPalette::Dark, Qt::black);
	pal.setColor(QPalette::Inactive, QPalette::Dark, Qt::black);
	pal.setColor(QPalette::Disabled, QPalette::Dark, pal.color(QPalette::Mid));
	setPalette(pal);
}

void ApercuBG::fill(Field *field, bool reload)
{
	if (!reload && _field == field) {
		return;
	}

	if (!field->isOpen()) {
		clear();
		return;
	}

	_field = field;
	QImage image = _field->background()->openBackground();

	if (image.isNull()) {
		_background = errorPixmap(contentsRect().size());
		setCursor(Qt::ArrowCursor);
		_error = true;
	} else {
		_background = QPixmap::fromImage(image);
		setCursor(Qt::PointingHandCursor);
		_error = false;
	}

	drawBackground();
}

void ApercuBG::drawBackground()
{
	if (!_field) {
		return;
	}

	QSize contentsSize = contentsRect().size();

	if (_background.size() != contentsSize) {
		setPixmap(_background.scaled(contentsSize, Qt::KeepAspectRatio));
	} else {
		setPixmap(_background);
	}
}

QPixmap ApercuBG::errorPixmap(const QSize &size)
{
	QPixmap errorPix(size);
	errorPix.fill(Qt::black);
	QFont font;
	font.setPixelSize(44);
	QString text = tr("Error");
	int textWidth = QFontMetrics(font).width(text),
	    textHeight = QFontMetrics(font).height();

	QPainter p(&errorPix);
	p.setPen(Qt::white);
	p.setFont(font);
	p.drawStaticText((size.width() - textWidth) / 2,
	                 (size.height() - textHeight) / 2,
	                 QStaticText(text));
	p.end();

	return errorPix;
}

void ApercuBG::clear()
{
	QLabel::clear();
	setCursor(Qt::ArrowCursor);
	_field = 0;
	_background = QPixmap();
	_error = false;
}

void ApercuBG::mouseReleaseEvent(QMouseEvent *e)
{
	QLabel::mouseReleaseEvent(e);

	if (e->button() == Qt::LeftButton && !_error) {
		emit clicked();
	}
}

void ApercuBG::resizeEvent(QResizeEvent *e)
{
	Q_UNUSED(e);

	if (!_background.isNull() &&
	        (!pixmap() || contentsRect().size() != pixmap()->size())) {
		setUpdatesEnabled(false);
		drawBackground(); // Call setPixmap() -> update() which must be prevented
		setUpdatesEnabled(true);
	}

	QLabel::resizeEvent(e);
}
