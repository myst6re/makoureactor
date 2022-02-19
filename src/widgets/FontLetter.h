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
#pragma once

#include <QtWidgets>
#include "FontDisplay.h"
#include "FontGrid.h"

#define PIXEL_SIZE	21

class FontLetter : public FontDisplay
{
	Q_OBJECT
public:
	explicit FontLetter(QWidget *parent = nullptr);
	virtual ~FontLetter() override;
	void setReadOnly(bool ro);
	virtual void setWindowBinFile(WindowBinFile *windowBinFile) override;
	bool isLetterSizeEditable() const;
public slots:
	virtual void setLetter(quint8 letter) override;
	void setPixelIndex(int index);
	void reset();
signals:
	void imageChanged(const QRect &rect);
	void widthEdited(int width);
protected:
	virtual QSize sizeHint() const override;
	virtual QSize minimumSizeHint() const override;
	virtual void paintEvent(QPaintEvent *e) override;
	virtual void mouseMoveEvent(QMouseEvent *e) override;
	virtual void mousePressEvent(QMouseEvent *e) override;
	virtual void mouseReleaseEvent(QMouseEvent *) override;
private:
	QPoint getPixel(const QPoint &pos);
	bool setPixel(const QPoint &pixel);
	QImage copyLetter;
	int _pixelIndex;
	bool readOnly, startDrag, startDrag2;
};
