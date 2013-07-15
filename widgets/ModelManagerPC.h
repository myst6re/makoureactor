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
#ifndef MODELMANAGERPC_H
#define MODELMANAGERPC_H

#include <QtGui>
#include "core/field/FieldPC.h"
#include "ColorDisplay.h"
#include "ModelManager.h"

class ModelManagerPC : public ModelManager
{
	Q_OBJECT
public:
	ModelManagerPC(const QGLWidget *shareWidget=0, QWidget *parent=0);
protected:
	void fill2();
	QList<QStringList> modelNames() const;
	QList<QStringList> animNames(int row) const;
	void showModelInfos2(int row);
	FieldModelFile *modelData(QTreeWidgetItem *item);
	FieldModelLoaderPC *modelLoader() const;
	FieldPC *field() const;
	QList<QRgb> lightColors(int modelID) const;
	quint16 modelScale(int modelID) const;
private:
	void copyModel(int modelID);
	void cutModel(int modelID);
	void pasteModel(int modelID);

	QSpinBox *globalScale;
	QLineEdit *modelName;
	QToolBar *toolBar2;
	QAction *copyModelAction, *cutModelAction, *pasteModelAction;
	FieldModelInfosPC _copiedModel;
	bool copied;
private slots:
	void setGlobalScale(int value);
	void modifyHRC(const QString &hrc);
	void modifyAnimation(const QString &a);
	void addModel();
	void delModel();
	void upModel();
	void downModel();
	void renameOKModel(QTreeWidgetItem *);
	void setModelName(const QString &modelName);
	void setModelScale(int scale);
	void setModelColor(int id, QRgb color);
	void addAnim();
	void delAnim();
	void upAnim();
	void downAnim();
	void renameOKAnim(QTreeWidgetItem *item, int column);
	void copyCurrentModel();
	void cutCurrentModel();
	void pasteOnCurrentModel();
	void updateActionsState();
};

#endif // MODELMANAGERPC_H
