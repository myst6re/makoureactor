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

#include <QtWidgets>
#include "core/field/FieldPC.h"
#include "ColorDisplay.h"
#include "ModelManager.h"

class ModelManagerPC : public ModelManager
{
	Q_OBJECT
public:
	ModelManagerPC(const QGLWidget *shareWidget=0, QWidget *parent=0);
protected:
	QList<QStringList> modelNames() const;
	QList<QTreeWidgetItem *> animItems(int modelID) const;
	void showModelInfos2(int row);
	FieldModelFile *modelData(QTreeWidgetItem *item);
	FieldModelLoaderPC *modelLoader() const;
	FieldPC *field() const;
	const QList<FieldModelColorDir> &lightColors(int modelID) const;
	QRgb globalColor(int modelID) const;
	quint16 modelScale(int modelID) const;
private:
	void copyModels(const QList<int> &modelIDs);
	void cutModels(const QList<int> &modelIDs);
	void pasteModels(int modelID);

	QLineEdit *modelName;
	QToolBar *toolBar2;
	QAction *copyModelAction, *cutModelAction, *pasteModelAction;
	QList<FieldModelInfosPC> _copiedModels;
	bool copied;
private slots:
	void modifyHRC(const QString &hrc);
	void modifyAnimation(const QString &a);
	void addModel();
	void delModel();
	void upModel();
	void downModel();
	void renameOKModel(QTreeWidgetItem *);
	void setModelName(const QString &modelName);
	void setModelScale(int scale);
	void setModelGlobalColor(int id, QRgb color);
	void setModelColor(int id, const FieldModelColorDir &color);
	void addAnim();
	void delAnim();
	void upAnim();
	void downAnim();
	void editAnim(QTreeWidgetItem *item, int column);
	void renameOKAnim(QTreeWidgetItem *item, int column);
	void copyCurrentModel();
	void cutCurrentModel();
	void pasteOnCurrentModel();
	void updateActionsState();
};

#endif // MODELMANAGERPC_H
