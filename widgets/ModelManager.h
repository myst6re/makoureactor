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
#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <QtWidgets>
#include "core/field/FieldPC.h"
#include "ColorDisplay.h"
#include "FieldModel.h"

class ModelManager : public QDialog
{
	Q_OBJECT
public:
	ModelManager(const QGLWidget *shareWidget=0, QWidget *parent=0);
	virtual void clear();
	void fill(Field *field, bool reload=false);
signals:
	void modified();
protected:
	virtual QList<QStringList> modelNames() const=0;
	virtual QList<QTreeWidgetItem *> animItems(int modelID) const=0;
	virtual void showModelInfos2(int row);
	virtual FieldModelFile *modelData(QTreeWidgetItem *item)=0;
	virtual FieldModelLoader *modelLoader() const;
	virtual Field *field() const;
	virtual QList<QRgb> lightColors(int modelID) const=0;
	virtual quint16 modelScale(int modelID) const=0;
	int currentModelID(QTreeWidgetItem *item=0) const;
	QList<int> selectedModelIDs() const;
	int currentAnimID(QTreeWidgetItem *item=0) const;

	QTreeWidget *models;
	QFrame *modelFrame;
	QSpinBox *modelUnknown;
	QSpinBox *modelScaleWidget;
	ColorDisplay *modelColorDisplay;
	QLabel *modelColorLabel;
	QTreeWidget *modelAnims;
	FieldModel *modelPreview;
	QWidget *modelWidget;
private:
	void fillModelList();

	Field *_field;
	FieldModelLoader *fieldModelLoader;
private slots:
	void showModelInfos(QTreeWidgetItem *, QTreeWidgetItem *);
	void setModelUnknown(int unknown);
	void showModel(QTreeWidgetItem *item);
	void setModelColorLabel(int colorId);
};

#endif // MODELMANAGER_H
