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

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_previewWalkmesh, 0, 0, 3, 1);
	layout->addWidget(_point1, 0, 1);
	layout->addWidget(_point2, 1, 1);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(_point1, SIGNAL(valuesChanged(Vertex_s)), SLOT(updatePreview()));
	connect(_point2, SIGNAL(valuesChanged(Vertex_s)), SLOT(updatePreview()));
}

OpcodeBox ScriptEditorWalkmeshPage::buildOpcode()
{
	OpcodeLINE &opcodeLine = opcode().cast<OpcodeLINE>();
	Vertex_s vertex1 = _point1->values(), vertex2 = _point2->values();
	opcodeLine.targetX1 = vertex1.x;
	opcodeLine.targetY1 = vertex1.y;
	opcodeLine.targetZ1 = vertex1.z;
	opcodeLine.targetX2 = vertex2.x;
	opcodeLine.targetY2 = vertex2.y;
	opcodeLine.targetZ2 = vertex2.z;

	return opcode();
}

void ScriptEditorWalkmeshPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	const OpcodeLINE &opcodeLine = opcode.cast<OpcodeLINE>();
	Vertex_s vertex;

	vertex.x = opcodeLine.targetX1;
	vertex.y = opcodeLine.targetY1;
	vertex.z = opcodeLine.targetZ1;

	_point1->setValues(vertex);

	vertex.x = opcodeLine.targetX2;
	vertex.y = opcodeLine.targetY2;
	vertex.z = opcodeLine.targetZ2;

	_point2->setValues(vertex);
}

void ScriptEditorWalkmeshPage::updatePreview()
{
	_previewWalkmesh->update();

	emit opcodeChanged();
}
