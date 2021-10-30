#ifndef SCRIPTEDITORWALKMESHPAGE_H
#define SCRIPTEDITORWALKMESHPAGE_H

#include "ScriptEditorView.h"
#include "widgets/VertexWidget.h"
#include "3d/WalkmeshWidget.h"

class ScriptEditorWalkmeshPage : public ScriptEditorView
{
	Q_OBJECT
public:
	ScriptEditorWalkmeshPage(Field *field, const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	OpcodeBox buildOpcode() override;
	void setOpcode(const OpcodeBox &opcode) override;
private slots:
	void updatePreview();
private:
	void build() override;
	VertexWidget *_point1, *_point2;
	WalkmeshWidget *_previewWalkmesh;
	Field *_field;
};

#endif // SCRIPTEDITORWALKMESHPAGE_H
