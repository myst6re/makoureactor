/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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

#include <QtGui>
#include <QWizardPage>
#include <QLineEdit>
#include <QListWidget>
#include <QGridLayout>
#include <QWidget>
#include "core/field/GrpScript.h"
#include "3d/FieldModel.h"

#define FIELD_NAME "name"
#define FIELD_TYPE "type"
#define FIELD_SUB_TYPE "subType"

class GrpScriptWizardPageModel : public QWizardPage
{
	Q_OBJECT
public:
	explicit GrpScriptWizardPageModel(QWidget *parent = nullptr);
	void initializePage() override;
	int nextId() const override;
private:
	FieldModel *_modelPreview;
	QWidget *_modelWidget;
	QListWidget *_model;
};

class GrpScriptWizardPageType : public QWizardPage
{
	Q_OBJECT
public:
	enum {
		TypeEmpty, TypeModel, TypeLine, TypeAnimation, TypeOther
	};
	explicit GrpScriptWizardPageType(QWidget *parent = nullptr);
	void initializePage() override;
	int nextId() const override;
private slots:
	void fillSubTypeList(int type);
	void updateButtons();
private:
	QListWidget *_type, *_subType;
};

class GrpScriptWizard : public QWizard
{
public:
	enum {
		PageType,
		PageModelType
	};
	explicit GrpScriptWizard(QWidget *parent = nullptr);
	GrpScript selectedGroup() const;
};
