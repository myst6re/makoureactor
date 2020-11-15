/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2015 Arzel Jérôme <myst6re@gmail.com>
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
#include "GrpScriptWizard.h"

GrpScriptWizardPageType::GrpScriptWizardPageType(QWidget *parent) :
	QWizardPage(parent)
{
	_type = new QListWidget(this);
	_subType = new QListWidget(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_type, 0, 0);
	layout->addWidget(_subType, 0, 1);

	connect(_type, SIGNAL(currentRowChanged(int)), SLOT(fillSubTypeList(int)));

	registerField("type", _type);
	registerField("subType", _subType);

	fillTypeList();
}

void GrpScriptWizardPageType::fillTypeList()
{
	_type->blockSignals(true);
	_type->clear();
	_type->addItems(QStringList() <<
	                tr("3D Model") <<
	                tr("Other"));
	_type->blockSignals(false);

	_type->setCurrentRow(0);
}

void GrpScriptWizardPageType::fillSubTypeList(int type)
{
	_subType->blockSignals(true);
	_subType->clear();
	QStringList items;

	switch(type) {
	case TypeModel: // 3D model
		items << "Main Character" <<
		         "Non Playable Character" <<
		         "Item" <<
		         "Save Point";
		break;
	default:
		items << "Empty" <<
		         "Position debugging";
		break;
	}

	_subType->addItems(items);
	_subType->blockSignals(false);

	_subType->setCurrentRow(0);
}

void GrpScriptWizardPageType::initializePage()
{
	_type->setCurrentRow(0);
}

bool GrpScriptWizardPageType::isComplete() const
{
	return !_type->selectedItems().isEmpty() && !_subType->selectedItems().isEmpty();
}

int GrpScriptWizardPageType::nextId() const
{
	return -1;
}

GrpScriptWizard::GrpScriptWizard(QWidget *parent) :
	QWizard(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setPage(PageType, new GrpScriptWizardPageType);
}

GrpScript *GrpScriptWizard::selectedGroup() const
{
	int type = field("type").toInt(),
			subType = field("subType").toInt();

	switch(type) {
	case GrpScriptWizardPageType::TypeModel:
		return GrpScript::createGroupModel(0);
	case GrpScriptWizardPageType::TypeOther:
		break;
	}

	return new GrpScript(field("name").toString());
}
