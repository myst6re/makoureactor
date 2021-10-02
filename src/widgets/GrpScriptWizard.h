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
#include <QListWidget>
#include <QGridLayout>
#include "core/field/GrpScript.h"

class GrpScriptWizardPageType : public QWizardPage
{
	Q_OBJECT
public:
	enum {
		TypeModel, TypeOther
	};
	explicit GrpScriptWizardPageType(QWidget *parent = nullptr);
	void initializePage();
	bool isComplete() const;
	int nextId() const;
private slots:
	void fillSubTypeList(int type);
private:
	void fillTypeList();
	QListWidget *_type, *_subType;
};

class GrpScriptWizard : public QWizard
{
public:
	explicit GrpScriptWizard(QWidget *parent = 0);
	GrpScript *selectedGroup() const;
protected:
	enum {
		PageType
	};
};
