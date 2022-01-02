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
#include "ScriptEditorWalkmeshPage.h"

ScriptEditorWalkmeshPage::ScriptEditorWalkmeshPage(Field *field, const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
    ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent), _field(field)
{
}

void ScriptEditorWalkmeshPage::build()
{
	_point1 = new VertexWidget(this);
	_point2 = new VertexWidget(this);
	_previewWalkmesh = new WalkmeshWidget(this);
	_previewWalkmesh->fill(_field);
	_previewWalkmesh->setModelsVisible(false);
	_previewWalkmesh->setBackgroundVisible(true);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_previewWalkmesh, 0, 0, 3, 1);
	layout->addWidget(_point1, 0, 1);
	layout->addWidget(_point2, 1, 1);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(_point1, &VertexWidget::valuesChanged, this, &ScriptEditorWalkmeshPage::updatePreview);
	connect(_point2, &VertexWidget::valuesChanged, this, &ScriptEditorWalkmeshPage::updatePreview);
}

Opcode ScriptEditorWalkmeshPage::buildOpcode()
{
	OpcodeLINE &opcodeLine = opcode().op().opcodeLINE;
	Vertex_s vertex1 = _point1->values(), vertex2 = _point2->values();
	opcodeLine.targetX1 = vertex1.x;
	opcodeLine.targetY1 = vertex1.y;
	opcodeLine.targetZ1 = vertex1.z;
	opcodeLine.targetX2 = vertex2.x;
	opcodeLine.targetY2 = vertex2.y;
	opcodeLine.targetZ2 = vertex2.z;

	return opcode();
}

void ScriptEditorWalkmeshPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	const OpcodeLINE &opcodeLine = opcode.op().opcodeLINE;
	Vertex_s vertex;

	vertex.x = opcodeLine.targetX1;
	vertex.y = opcodeLine.targetY1;
	vertex.z = opcodeLine.targetZ1;

	_point1->setValues(vertex);

	vertex.x = opcodeLine.targetX2;
	vertex.y = opcodeLine.targetY2;
	vertex.z = opcodeLine.targetZ2;

	_point2->setValues(vertex);

	_previewWalkmesh->setCustomLine(_point1->values(), _point2->values());
	_previewWalkmesh->setCustomLineVisible(true);
}

void ScriptEditorWalkmeshPage::updatePreview()
{
	_previewWalkmesh->setCustomLine(_point1->values(), _point2->values());
	_previewWalkmesh->setCustomLineVisible(true);

	emit opcodeChanged();
}
