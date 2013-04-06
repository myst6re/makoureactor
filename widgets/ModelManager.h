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

#include <QtGui>
#include "core/field/Field.h"
#include "ColorDisplay.h"
#include "FieldModel.h"

class ModelManager : public QDialog
{
	Q_OBJECT
public:
	ModelManager(const QGLWidget *shareWidget=0, QWidget *parent=0);
	void clear();
	void fill(FieldPC *field, bool reload=false);
signals:
	void modified();
private:
	void fillModelList();
	int currentModelID(QTreeWidgetItem *item=0) const;
	int currentAnimID(QTreeWidgetItem *item=0) const;

	FieldPC *field;
	FieldModelLoaderPC *fieldModelLoader;
	QSpinBox *globalScale;
	QTreeWidget *models;
	QFrame *modelFrame;
	QLineEdit *modelName;
	QSpinBox *modelUnknown;
	QSpinBox *modelScale;
	QTreeWidget *modelAnims;
	FieldModel *modelPreview;
	ColorDisplay *modelColorDisplay;
	QToolBar *toolBar2;
private slots:
	void setGlobalScale(int value);
	void showModelInfos(QTreeWidgetItem *, QTreeWidgetItem *);
	void modifyHRC(const QString &hrc);
	void modifyAnimation(const QString &a);
	void addModel();
	void delModel();
	void upModel();
	void downModel();
	void renameOKModel(QTreeWidgetItem *);
	void setModelName(const QString &modelName);
	void setModelUnknown(int unknown);
	void setModelScale(int scale);
	void setModelColor(int id, QRgb color);
	void addAnim();
	void delAnim();
	void upAnim();
	void downAnim();
	void renameOKAnim(QTreeWidgetItem *item, int column);
	void showModel(QTreeWidgetItem *item);
};

#endif // MODELMANAGER_H
