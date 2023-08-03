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
#include <ColorDisplay>
#include "3d/FieldModel.h"
#include "ModelColorsLayout.h"

class ModelManager : public QDialog
{
	Q_OBJECT
public:
	ModelManager(QWidget *parent = nullptr);
	virtual void clear();
	void fill(Field *field, bool reload = false);
signals:
	void modified();
protected:
	void showModelInfos() {
		showModelInfos(models->currentItem());
	}
	virtual QList<QStringList> modelNames() const=0;
	virtual QList<QTreeWidgetItem *> animItems(int modelID) const=0;
	virtual void showModelInfos2(int row);
	virtual FieldModelFile *modelData(QTreeWidgetItem *item)=0;
	virtual FieldModelLoader *modelLoader() const;
	virtual Field *field() const;
	virtual const QList<FieldModelColorDir> &lightColors(int modelID) const=0;
	virtual void setLightColor(int modelID, int id, const FieldModelColorDir &color)=0;
	virtual QRgb globalColor(int modelID) const=0;
	virtual void setGlobalColor(int modelID, QRgb color)=0;
	virtual quint16 modelScale(int modelID) const=0;
	virtual void setModelScale(int modelID, quint16 scale)=0;
	int currentModelID(QTreeWidgetItem *item = nullptr) const;
	QList<int> selectedModelIDs() const;
	int currentAnimID(QTreeWidgetItem *item = nullptr) const;

	QTreeWidget *models;
	QFrame *modelFrame;
	QToolBar *toolBar2;
	QSpinBox *modelUnknown;
	QSpinBox *modelScaleWidget;
	ColorDisplay *modelGlobalColorWidget;
	ModelColorsLayout *modelColorsLayout;
	QTreeWidget *modelAnims;
	FieldModel *modelPreview;
	QWidget *modelWidget;
private:
	void fillModelList();

	Field *_field;
	FieldModelLoader *fieldModelLoader;
private slots:
	void showModelInfos(QTreeWidgetItem *item, QTreeWidgetItem *previous = nullptr);
	void setModelUnknown(int unknown);
	void setModelScale(int scale);
	void setModelGlobalColor(int id, QRgb color);
	void setModelColor(int id, const FieldModelColorDir &color);
	void showModel(QTreeWidgetItem *item);
};
