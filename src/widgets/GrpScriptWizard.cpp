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
#include "GrpScriptWizard.h"
#include "core/Config.h"
#include "core/field/CharArchive.h"

GrpScriptWizardPageModel::GrpScriptWizardPageModel(QWidget *parent) :
	QWizardPage(parent)
{
	_model = new QListWidget(this);
	_model->setUniformItemSizes(true);

	if (Config::value("OpenGL", true).toBool()) {
		_modelPreview = new FieldModel();
		_modelWidget = _modelPreview;
		_modelPreview->setFixedSize(304, 214);
	} else {
		_modelPreview = nullptr;
		_modelWidget = new QWidget(this);
	}

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_model, 0, 0);
	layout->addWidget(_modelWidget, 0, 1);
}

void GrpScriptWizardPageModel::initializePage()
{
	int subType = field(FIELD_SUB_TYPE).toInt();
	CharArchive *charLgp = CharArchive::instance();
	QStringList files = charLgp->hrcFiles();

	// Remove extension
	for (QString &file: files) {
		file = file.left(file.size() - 4);
	}

	std::sort(files.begin(), files.end());

	_model->addItems(files);
}

int GrpScriptWizardPageModel::nextId() const
{
	return -1;
}

GrpScriptWizardPageType::GrpScriptWizardPageType(QWidget *parent) :
	QWizardPage(parent)
{
	_type = new QListWidget(this);
	_type->setUniformItemSizes(true);
	_type->addItems(QStringList()
	                << tr("Empty")
	                << tr("3D Model")
	                << tr("Line")
	                << tr("Animation")
	                << tr("Other"));
	_subType = new QListWidget(this);
	_subType->setUniformItemSizes(true);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_type, 0, 0);
	layout->addWidget(_subType, 0, 1);

	connect(_type, SIGNAL(currentRowChanged(int)), SLOT(fillSubTypeList(int)));
	connect(_subType, SIGNAL(currentRowChanged(int)), SLOT(updateButtons()));

	registerField(FIELD_TYPE"*", _type);
	registerField(FIELD_SUB_TYPE"*", _subType);
}

void GrpScriptWizardPageType::fillSubTypeList(int type)
{
	_subType->blockSignals(true);
	_subType->clear();
	QStringList items;

	switch(type) {
	case TypeModel: // 3D model
		items << tr("Main Character")
		      << tr("Non Playable Character")
		      << tr("Item")
		      << tr("Save Point");
		break;
	default:
		items << tr("Empty")
		      << tr("Position debugging");
		break;
	}

	_subType->addItems(items);
	_subType->blockSignals(false);

	_subType->setCurrentRow(0);
	updateButtons();
}

void GrpScriptWizardPageType::initializePage()
{
	_type->setCurrentRow(0);
}

int GrpScriptWizardPageType::nextId() const
{
	int row = _type->currentRow();

	switch (row) {
	case TypeModel:
		return GrpScriptWizard::PageModelType;
	default:
		return -1;
	}
}

void GrpScriptWizardPageType::updateButtons()
{
	setFinalPage(nextId() == -1);
}

GrpScriptWizard::GrpScriptWizard(QWidget *parent) :
	QWizard(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
{
	setPage(PageType, new GrpScriptWizardPageType);
	setPage(PageModelType, new GrpScriptWizardPageModel);
}

GrpScript GrpScriptWizard::selectedGroup() const
{
	int type = field(FIELD_TYPE).toInt(),
			subType = field(FIELD_SUB_TYPE).toInt();

	switch (type) {
	case GrpScriptWizardPageType::TypeModel:
		return GrpScript::createGroupModel(0);
	}

	return GrpScript(QList<Script>() << Script(QList<Opcode>() << OpcodeRET())
	                 << Script(QList<Opcode>() << OpcodeRET()));
}
