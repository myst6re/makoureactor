/****************************************************************************
 ** Deling Final Fantasy VIII Field Editor
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
#include "ApercuBGLabel.h"
#include "3d/FieldModel.h"

class ArchivePreview : public QStackedWidget
{
	Q_OBJECT
public:
	enum Pages { EmptyPage = 0, ImagePage, TextPage, ModelPage };
	explicit ArchivePreview(QWidget *parent = nullptr);
	void clearPreview();
	void imagePreview(const QPixmap &image, const QString &name = QString(),
	                  int palID = 0, int palCount = 0, int imageID = 0,
	                  int imageCount = 0);
	void textPreview(const QString &text);
	void modelPreview(FieldModelFile *fieldModel);
signals:
	void currentImageChanged(int);
	void currentPaletteChanged(int);
public slots:
	void saveImage();
private:
	QWidget *imageWidget();
	QWidget *textWidget();
	QWidget *modelWidget();
	QScrollArea *scrollArea;
	QComboBox *imageSelect, *palSelect;
	ApercuBGLabel *_lbl;
	QString _name;
};
