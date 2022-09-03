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
#include "core/field/FieldPC.h"
#include "core/field/FieldModelLoaderPC.h"
#include "ModelManager.h"

class ModelManagerPC : public ModelManager
{
	Q_OBJECT
public:
	ModelManagerPC(QWidget *parent = nullptr);
protected:
	QList<QStringList> modelNames() const override;
	QList<QTreeWidgetItem *> animItems(int modelID) const override;
	void showModelInfos2(int row) override;
	FieldModelFile *modelData(QTreeWidgetItem *item) override;
	FieldModelLoaderPC *modelLoader() const override;
	FieldPC *field() const override;
	const QList<FieldModelColorDir> &lightColors(int modelID) const override;
	void setLightColor(int modelID, int id, const FieldModelColorDir &color) override;
	QRgb globalColor(int modelID) const override;
	void setGlobalColor(int modelID, QRgb color) override;
	quint16 modelScale(int modelID) const override;
	void setModelScale(int modelID, quint16 scale) override;
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
	void processEvents() const;
};
