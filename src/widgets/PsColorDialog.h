/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2023 Arzel Jérôme <myst6re@gmail.com>
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
#include "QColorPicker.h"
#include <ImageGridWidget.h>

class PsColorDialog : public QDialog
{
	Q_OBJECT
public:
	explicit PsColorDialog(const QColor &color, QWidget *parent = nullptr);
	explicit PsColorDialog(const QList<QRgb> &palette, quint8 index, QWidget *parent = nullptr);
	uint indexOrColor() const;
private slots:
	void setColor(const QColor &color);
	void setHsv(int h, int s, int v);
	void setColorFromPalette(const Cell &cell);
	void setColorFromHtmlCode(const QString &text);
	void setTransparentEnabled(bool enabled);
private:
	virtual void keyPressEvent(QKeyEvent *event) override
	{
		int key = event->key();
		if (key == Qt::Key_Return || key == Qt::Key_Enter)
			event->ignore();
		else
			QDialog::keyPressEvent(event);
		
	}
	void setLayout();

	QLabel *_topLabel;
	QGridLayout *_layout;
	QColorPicker *_colorPicker;
	QColorLuminancePicker *_colorLumniancePicker;
	QColorShowLabel *_colorShowLabel;
	QLineEdit *_colorHtmlCode;
	QLabel *_colorHtmlCodeLabel;
	QCheckBox *_alphaFlag;
	ImageGridWidget *_imageGrid;
	QList<QRgb> _palette;
	QColor _color;
	quint8 _index;
};
