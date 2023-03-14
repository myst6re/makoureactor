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
#include "ScriptEditorModelPage.h"
#include "core/field/Field.h"
#include "core/field/FieldModelLoaderPC.h"
#include "core/field/FieldModelLoaderPS.h"

ScriptEditorModelPage::ScriptEditorModelPage(Field *field, const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
    ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent), _field(field)
{
}

void ScriptEditorModelPage::build()
{
	_modelPreview = new FieldModel(this);
	_models = new QComboBox(this);

	int modelCount = _field->fieldModelLoader()->modelCount();

	for (int i = 0; i < modelCount; ++i) {
		QString name;
		if (_field->isPC()) {
			name = QString("%1 - %2").arg(i).arg(static_cast<FieldModelLoaderPC *>(_field->fieldModelLoader())->HRCName(i));
		} else {
			name = tr("%1 - Model #%2").arg(i).arg(static_cast<FieldModelLoaderPS *>(_field->fieldModelLoader())->model(i).modelID);
		}
		_models->addItem(name, i);
	}

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_models, 0, 0);
	layout->addWidget(_modelPreview, 1, 0);
	layout->setRowStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(_models, &QComboBox::currentIndexChanged, this, &ScriptEditorModelPage::updatePreview);
}

Opcode ScriptEditorModelPage::buildOpcode()
{
	OpcodeCHAR_ &opcodeChar = opcode().op().opcodeCHAR_;

	opcodeChar.object3DID = _models->currentData().toInt();

	return opcode();
}

void ScriptEditorModelPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	const OpcodeCHAR_ &opcodeChar = opcode.op().opcodeCHAR_;

	_models->setCurrentIndex(_models->findData(opcodeChar.object3DID));

	updatePreview();
}

void ScriptEditorModelPage::updatePreview()
{
	_modelPreview->setFieldModelFile(_field->fieldModel(_models->currentData().toInt()));

	emit opcodeChanged();
}
