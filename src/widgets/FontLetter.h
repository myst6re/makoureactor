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
#ifndef FONTLETTER_H
#define FONTLETTER_H

#include <QtWidgets>
#include "FontDisplay.h"
#include "FontGrid.h"

#define PIXEL_SIZE	21

class FontLetter : public FontDisplay
{
	Q_OBJECT
public:
	explicit FontLetter(QWidget *parent=0);
	virtual ~FontLetter();
	void setReadOnly(bool ro);
	virtual void setWindowBinFile(WindowBinFile *windowBinFile);
public slots:
	virtual void setLetter(int letter);
	void setPixelIndex(int index);
	void reset();
signals:
	void imageChanged(const QRect &rect);
	void widthEdited(int width);
protected:
	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;
	virtual void paintEvent(QPaintEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *);
private:
	QPoint getPixel(const QPoint &pos);
	bool setPixel(const QPoint &pixel);
	QImage copyLetter;
	int _pixelIndex;
	bool readOnly, startDrag, startDrag2;
};

#endif // FONTLETTER_H
