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
#ifndef BGDIALOG_H
#define BGDIALOG_H

#include <QtGui>
#include "ApercuBGLabel.h"
#include "Field.h"
#include "FieldArchive.h"

class BGDialog : public QDialog
{
    Q_OBJECT
public:
	explicit BGDialog(FieldArchive *archive, Field *field, QWidget *parent);
private slots:
	void parameterChanged(int index);
	void layerChanged();
	void enableState(QListWidgetItem *item);
	void enableLayer(QListWidgetItem *item);
	void changeZ(int value);
private:
	void fillWidgets();
	void fill();

	bool isPS;
	QByteArray fieldData, mimData;
	FieldArchive *archive;
	Field *field;
	ApercuBGLabel *image;
	QComboBox *parametersWidget;
	QListWidget *statesWidget, *layersWidget;
	QSpinBox *zWidget;

	QHash<quint8, quint8> allparams;
	QHash<quint8, quint8> params;
	bool layers[4];
	qint16 x[3], y[3], z[3];
};

#endif // BGDIALOG_H
