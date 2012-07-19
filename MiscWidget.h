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
#ifndef MISCWIDGET_H
#define MISCWIDGET_H

#include <QtGui>
#include "InfFile.h"
#include "Field.h"
#include "Data.h"
#include "VertexWidget.h"

class MiscWidget : public QDialog
{
	Q_OBJECT
public:
	explicit MiscWidget(InfFile *data, Field *field, QWidget *parent=0);
private slots:
	void changeGate(QListWidgetItem *item, QListWidgetItem *previous);
	void setEnabledGate(int index);
private:
	void fill();
	void setEnabledGate(bool enable);
	void saveExit(int row);
	InfFile *data, dataCpy;
	Field *field;
	QLineEdit *mapName, *mapAuthor;
	QSpinBox *mapScale, *mapControl;
	QListWidget *exitList;
	QComboBox *fieldID;
	QCheckBox *arrowDisplay;
	VertexWidget *triggerPoint1, *triggerPoint2, *destination;
	QLabel *unknown2Lbl;
	QLineEdit *unknown0, *unknown1, *unknown2;
protected:
	void accept();
};

#endif // MISCWIDGET_H
