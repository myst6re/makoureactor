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
#include "ScriptEditorView.h"

ScriptEditorView::ScriptEditorView(Script *script, QWidget *parent)
	: QWidget(parent), _script(script), _opcode(0)
{
}

ScriptEditorView::~ScriptEditorView()
{
}

Opcode *ScriptEditorView::opcode()
{
	return _opcode;
}

void ScriptEditorView::setOpcode(Opcode *opcode)
{
	_opcode = opcode;
}

ScriptEditorGotoPage::ScriptEditorGotoPage(Script *script, QWidget *parent) :
	ScriptEditorView(script, parent)
{
	label = new QComboBox();
	for(int i=0 ; i<script->size() ; ++i) {
		if(script->getOpcode(i)->isLabel()) {
			quint32 lbl = ((OpcodeLabel *)script->getOpcode(i))->label();
			label->addItem(tr("Label %1").arg(lbl), lbl);
		}
	}

	range = new QComboBox();
	range->addItem(QString("Court (8 bits)"));
	range->addItem(QString("Long (16 bits)"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Label")), 0, 0);
	layout->addWidget(label, 0, 1);
	layout->addWidget(new QLabel(tr("Type")), 1, 0);
	layout->addWidget(range, 1, 1);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());
}

Opcode *ScriptEditorGotoPage::opcode()
{
	OpcodeJump *jumpOpcode = (OpcodeJump *)_opcode;
	jumpOpcode->setLabel(label->itemData(label->currentIndex()).toUInt());

	return ScriptEditorView::opcode();
}

void ScriptEditorGotoPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	OpcodeJump *jumpOpcode = (OpcodeJump *)opcode;
	label->setCurrentIndex(label->findData(jumpOpcode->label()));

	if(opcode->id() == Opcode::JMPF || opcode->id() == Opcode::JMPB) {
		range->setCurrentIndex(0);
	} else {
		range->setCurrentIndex(1);
	}
}
