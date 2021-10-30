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
#include "ScriptEditorStructPage.h"
#include "Data.h"

ScriptEditorWithPriorityPage::ScriptEditorWithPriorityPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
      ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorWithPriorityPage::build()
{
	priority = new QSpinBox(this);
	priority->setRange(0, 6);

	helpWidget = new HelpWidget(16, tr("Lower priority number is higher priority in the game"));

	connect(priority, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
}

ScriptEditorReturnToPage::ScriptEditorReturnToPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorWithPriorityPage(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorReturnToPage::build()
{
	ScriptEditorWithPriorityPage::build();

	scriptList = new QComboBox(this);
	for (int i=0; i<32; ++i) {
		scriptList->addItem(tr("Script %1").arg(i));
	}

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Script")), 0, 0);
	layout->addWidget(scriptList, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Priority")), 1, 0);
	layout->addWidget(priority, 1, 1, 1, 3);
	layout->addWidget(helpWidget, 2, 0, 1, 4);
	layout->setRowStretch(3, 1);
	layout->setContentsMargins(QMargins());

	connect(scriptList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

OpcodeBox ScriptEditorReturnToPage::buildOpcode()
{
	OpcodeRETTO &opcodeRETTO = opcode().cast<OpcodeRETTO>();
	opcodeRETTO.scriptID = quint8(scriptList->currentIndex());
	opcodeRETTO.priority = quint8(priority->value());

	return opcode();
}

void ScriptEditorReturnToPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	const OpcodeRETTO &opcodeRETTO = opcode.cast<OpcodeRETTO>();
	scriptList->setCurrentIndex(opcodeRETTO.scriptID);
	priority->setValue(qMin(quint8(6), opcodeRETTO.priority));

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

ScriptEditorExecPage::ScriptEditorExecPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorWithPriorityPage(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorExecPage::build()
{
	ScriptEditorWithPriorityPage::build();

	groupList = new QComboBox(this);
	int i=0;
	for (const GrpScript &group : scriptsAndTexts()->grpScripts()) {
		groupList->addItem(QString("%1 - %2").arg(i++).arg(group.name()));
	}

	scriptList = new QComboBox(this);
	for (int i=0; i<32; ++i) {
		scriptList->addItem("");
	}

	execType = new QComboBox(this);
	execType->addItem(tr("Asynchronous, no wait"));
	execType->addItem(tr("Asynchronous, wait"));
	execType->addItem(tr("Synchronous, wait"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Group")), 0, 0);
	layout->addWidget(groupList, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Script")), 1, 0);
	layout->addWidget(scriptList, 1, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Priority")), 2, 0);
	layout->addWidget(priority, 2, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Type")), 3, 0);
	layout->addWidget(execType, 3, 1, 1, 3);
	layout->addWidget(helpWidget, 4, 0, 1, 4);
	layout->setRowStretch(5, 1);
	layout->setContentsMargins(QMargins());

	connect(groupList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(scriptList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(groupList, SIGNAL(currentIndexChanged(int)), SLOT(updateScriptList(int)));
	connect(execType, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
}

OpcodeBox ScriptEditorExecPage::buildOpcode()
{
	OpcodeExec &opcodeExec = opcode().cast<OpcodeExec>();
	opcodeExec.groupID = quint8(groupList->currentIndex());
	opcodeExec.scriptID = quint8(scriptList->currentIndex());
	opcodeExec.priority = quint8(priority->value());

	return opcode();
}

void ScriptEditorExecPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	switch (Opcode::Keys(opcode.id())) {
	case Opcode::REQ:		execType->setCurrentIndex(0);	break;
	case Opcode::REQSW:		execType->setCurrentIndex(1);	break;
	case Opcode::REQEW:		execType->setCurrentIndex(2);	break;
	default:	break;
	}

	const OpcodeExec &opcodeExec = opcode.cast<OpcodeExec>();
	if (opcodeExec.groupID >= this->scriptsAndTexts()->grpScriptCount()) {
		setValid(false);

		return;
	}

	const GrpScript &grp = this->scriptsAndTexts()->grpScript(opcodeExec.groupID);

	setValid(true);

	groupList->setCurrentIndex(opcodeExec.groupID);

	for (quint8 i = 0; i < 32; ++i) {
		scriptList->setItemText(i, grp.scriptName(i + 1));
	}
	scriptList->setCurrentIndex(opcodeExec.scriptID);

	priority->setValue(qMin(quint8(6), opcodeExec.priority));

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorExecPage::updateScriptList(int groupID)
{
	if (groupID < 0 || groupID >= scriptsAndTexts()->grpScriptCount()) {
		return;
	}

	const GrpScript &grp = scriptsAndTexts()->grpScript(groupID);

	for (quint8 i = 0; i < 32; ++i) {
		scriptList->setItemText(i, grp.scriptName(i + 1));
	}
}

void ScriptEditorExecPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	switch (index) {
	case 0:     key = Opcode::REQ;      break;
	case 1:     key = Opcode::REQSW;    break;
	case 2:     key = Opcode::REQEW;    break;
	default:    key = opcode().id();
	}

	if (key != opcode().id()) {
		convertOpcode(key);

		emit opcodeChanged();
	}
}

void ScriptEditorExecPage::convertOpcode(Opcode::Keys key)
{
	if (key == opcode().id()) {
		return;
	}

	const OpcodeExec &exec = opcode().cast<OpcodeExec>();

	switch (key) {
	case Opcode::REQ:      ScriptEditorView::setOpcode(OpcodeBox(new OpcodeREQ(exec)));      break;
	case Opcode::REQSW:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeREQSW(exec)));    break;
	case Opcode::REQEW:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeREQEW(exec)));    break;
	default: break;
	}
}

ScriptEditorExecCharPage::ScriptEditorExecCharPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorWithPriorityPage(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorExecCharPage::build()
{
	ScriptEditorWithPriorityPage::build();

	partyID = new QSpinBox(this);
	partyID->setRange(0, 255);
	scriptList = new QComboBox(this);
	for (int i=0; i<32; ++i) {
		scriptList->addItem(tr("Script %1").arg(i));
	}

	execType = new QComboBox(this);
	execType->addItem(tr("Asynchronous, no wait"));
	execType->addItem(tr("Asynchronous, wait"));
	execType->addItem(tr("Synchronous, wait"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Team member")), 0, 0);
	layout->addWidget(partyID, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Script")), 1, 0);
	layout->addWidget(scriptList, 1, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Priority")), 2, 0);
	layout->addWidget(priority, 2, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Type")), 3, 0);
	layout->addWidget(execType, 3, 1, 1, 3);
	layout->addWidget(helpWidget, 4, 0, 1, 4);
	layout->setRowStretch(5, 1);
	layout->setContentsMargins(QMargins());

	connect(partyID, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
	connect(scriptList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(execType, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
}

OpcodeBox ScriptEditorExecCharPage::buildOpcode()
{
	OpcodeExecChar &opcodeExecChar = opcode().cast<OpcodeExecChar>();
	opcodeExecChar.partyID = quint8(partyID->value());
	opcodeExecChar.scriptID = quint8(scriptList->currentIndex());
	opcodeExecChar.priority = quint8(priority->value());

	return opcode();
}

void ScriptEditorExecCharPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	switch (opcode.id()) {
	case Opcode::PREQ:		execType->setCurrentIndex(0);	break;
	case Opcode::PRQSW:		execType->setCurrentIndex(1);	break;
	case Opcode::PRQEW:		execType->setCurrentIndex(2);	break;
	default:	break;
	}

	const OpcodeExecChar &opcodeExecChar = opcode.cast<OpcodeExecChar>();
	partyID->setValue(opcodeExecChar.partyID);
	scriptList->setCurrentIndex(opcodeExecChar.scriptID);
	priority->setValue(qMin(quint8(6), opcodeExecChar.priority));

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}


void ScriptEditorExecCharPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	switch (index) {
	case 0:		key = Opcode::PREQ;		break;
	case 1:		key = Opcode::PRQSW;	break;
	case 2:		key = Opcode::PRQEW;	break;
	default:	key = opcode().id();
	}

	if (key != opcode().id()) {
		convertOpcode(key);

		emit opcodeChanged();
	}
}

void ScriptEditorExecCharPage::convertOpcode(Opcode::Keys key)
{
	if (key == opcode().id()) {
		return;
	}

	const OpcodeExecChar &exec = opcode().cast<OpcodeExecChar>();

	switch (key) {
	case Opcode::PREQ:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodePREQ(exec)));     break;
	case Opcode::PRQSW:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodePRQSW(exec)));    break;
	case Opcode::PRQEW:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodePRQEW(exec)));    break;
	default: break;
	}
}

ScriptEditorLabelPage::ScriptEditorLabelPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorLabelPage::build()
{
	label = new QDoubleSpinBox(this);
	label->setDecimals(0);
	label->setRange(0, pow(2, 32)-1);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Label")), 0, 0);
	layout->addWidget(label, 0, 1, 1, 3);
	layout->setRowStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(label, SIGNAL(valueChanged(double)), SIGNAL(opcodeChanged()));
}

OpcodeBox ScriptEditorLabelPage::buildOpcode()
{
	OpcodeLabel &opcodeLabel = opcode().cast<OpcodeLabel>();
	opcodeLabel.setLabel(quint32(label->value()));

	return opcode();
}

void ScriptEditorLabelPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	const OpcodeLabel &opcodeLabel = opcode.cast<OpcodeLabel>();
	if (opcodeLabel.label() == 0) {
		quint32 greaterLabel = 1;
		for (const OpcodeBox &op : script().opcodes()) {
			if (op->isLabel()) {
				quint32 lbl = op.cast<OpcodeLabel>().label();
				if (lbl >= greaterLabel) {
					greaterLabel = lbl + 1;
				}
			}
		}

		label->setValue(double(greaterLabel));
	} else {
		label->blockSignals(true);
		label->setValue(double(opcodeLabel.label()));
		label->blockSignals(false);
	}
}

void ScriptEditorJumpPageInterface::fillLabelList(bool jumpBack)
{
	const OpcodeJump &opcodeJump = opcode().cast<OpcodeJump>();

	label->blockSignals(true);
	label->clear();

	quint32 greaterLabel = 1;
	int i = 0;

	for (const OpcodeBox &op : script().opcodes()) {
		if (op->isLabel()) {
			quint32 lbl = op.cast<OpcodeLabel>().label();
			if (jumpBack || i >= opcodeID()) {
				label->addItem(tr("Label %1").arg(lbl), lbl);
			}
			if (lbl >= greaterLabel) {
				greaterLabel = lbl + 1;
			}
		}

		++i;
	}

	label->addItem(tr("New label"), greaterLabel);

	int index = label->findData(opcodeJump.label());
	if (index < 0) {
		index = 0;
	}
//	_valid = label->count() > 0;
	label->setCurrentIndex(index);
	label->blockSignals(false);
}

ScriptEditorJumpPage::ScriptEditorJumpPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(scriptsAndTexts, grpScript, script, opcodeID, parent), addJump(false)
{
}

void ScriptEditorJumpPage::build()
{
	label = new QComboBox(this);

	range = new QComboBox();
	range->addItem(tr("Short (8-bit)"));
	range->addItem(tr("Long (16-bit)"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Label")), 0, 0);
	layout->addWidget(label, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Type (expert)")), 1, 0);
	layout->addWidget(range, 1, 1, 1, 3);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(label, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(range, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

void ScriptEditorJumpPage::clear()
{
//	if (addJump) {
//		qDebug() << "ScriptEditorJumpPage deleteOpcode" << (_opcodeID+1) << _script->opcode(_opcodeID+1)->name();
//		_script->delOpcode(_opcodeID+1);
//		addJump = false;
//	}
}

OpcodeBox ScriptEditorJumpPage::buildOpcode()
{
	int opcodeIDLabel=0;
	quint32 labelVal = label->itemData(label->currentIndex()).toUInt();

	// Search opcode ID for label
	for (const OpcodeBox &op : script().opcodes()) {
		if (op->isLabel() && op.cast<OpcodeLabel>().label() == labelVal) {
			break;
		}
		++opcodeIDLabel;
	}

	// Create a new label if chosen
//	if (opcodeIDLabel >= _script->size()) {
//		qDebug() << "ScriptEditorJumpPage insertOpcode" << (_opcodeID+1);
//		opcodeIDLabel = _opcodeID+1;
//		_script->insertOpcode(opcodeIDLabel, OpcodeBox(new OpcodeLabel(labelVal)));
//		addJump = true;
//	}

	// Remove the new label if not chosen
//	if (addJump && label->currentIndex() < label->count() - 1) {
//		qDebug() << "ScriptEditorJumpPage deleteOpcode" << (_opcodeID+1) << _script->opcode(_opcodeID+1)->name();
//		_script->delOpcode(_opcodeID+1);
//		addJump = false;
//	}

	if (opcodeIDLabel < opcodeID()) { // Jump Back
		if (range->currentIndex() == 0) { // short
			convertOpcode(Opcode::JMPB);
		} else { // long
			convertOpcode(Opcode::JMPBL);
		}
	} else { // Jump Forward
		if (range->currentIndex() == 0) { // short
			convertOpcode(Opcode::JMPF);
		} else { // long
			convertOpcode(Opcode::JMPFL);
		}
	}

	OpcodeJump &opcodeJump = opcode().cast<OpcodeJump>();
	opcodeJump.setLabel(labelVal);

	return opcode();
}

void ScriptEditorJumpPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	addJump = false;

	fillLabelList(true);

	if (opcode.id() == Opcode::JMPF || opcode.id() == Opcode::JMPB) {
		range->setCurrentIndex(0);
	} else {
		range->setCurrentIndex(1);
	}

	for (QObject *o : children()) {
		o->blockSignals(false);
	}

	emit opcodeChanged();
}

bool ScriptEditorJumpPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

void ScriptEditorJumpPage::convertOpcode(Opcode::Keys key)
{
	if (key == opcode().id()) {
		return;
	}

	const OpcodeJump &jump = opcode().cast<OpcodeJump>();

	switch (key) {
	case Opcode::JMPF:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodeJMPF(jump)));     break;
	case Opcode::JMPFL:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeJMPFL(jump)));    break;
	case Opcode::JMPB:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodeJMPB(jump)));     break;
	case Opcode::JMPBL:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeJMPBL(jump)));    break;
	default: break;
	}
}


ScriptEditorJumpNanakiPage::ScriptEditorJumpNanakiPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
    ScriptEditorJumpPageInterface(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorJumpNanakiPage::build()
{
	label = new QComboBox(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Label")), 0, 0);
	layout->addWidget(label, 0, 1);
	layout->setRowStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(label, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

OpcodeBox ScriptEditorJumpNanakiPage::buildOpcode()
{
	OpcodeJump &opcodeJump = opcode().cast<OpcodeJump>();
	opcodeJump.setLabel(label->itemData(label->currentIndex()).toUInt());

	return opcode();
}

void ScriptEditorJumpNanakiPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	fillLabelList();

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

bool ScriptEditorJumpNanakiPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

ScriptEditorIfPage::ScriptEditorIfPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(scriptsAndTexts, grpScript, script, opcodeID, parent), addJump(false)
{
}

void ScriptEditorIfPage::build()
{
	varOrValue1 = new VarOrValueWidget(this);
	varOrValue2 = new VarOrValueWidget(this);

	operatorList = new QComboBox(this);
	for (int i = 0; i < OPERATORS_SIZE; ++i) {
		operatorList->addItem(QString::fromUtf8(Opcode::operators[i]));
	}

	rangeTest = new QComboBox();
	rangeTest->addItem(tr("8-bit unsigned"));
	rangeTest->addItem(tr("16-bit signed"));
	rangeTest->addItem(tr("16-bit unsigned"));

	label = new QComboBox(this);

	rangeJump = new QComboBox();
	rangeJump->addItem(tr("Short (8-bit)"));
	rangeJump->addItem(tr("Long (16-bit)"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Test to be performed")), 0, 0);
	layout->addWidget(varOrValue1, 0, 1);
	layout->addWidget(operatorList, 0, 2);
	layout->addWidget(varOrValue2, 0, 3);
	layout->addWidget(new QLabel(tr("Compare type")), 1, 0);
	layout->addWidget(rangeTest, 1, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Label")), 2, 0);
	layout->addWidget(label, 2, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Jump type (expert)")), 3, 0);
	layout->addWidget(rangeJump, 3, 1, 1, 3);
	layout->setRowStretch(4, 1);
	layout->setContentsMargins(QMargins());

	connect(varOrValue1, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(varOrValue2, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(operatorList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(label, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(rangeTest, SIGNAL(currentIndexChanged(int)), SLOT(changeTestRange()));
}

void ScriptEditorIfPage::clear()
{
//	if (addJump) {
//		qDebug() << "ScriptEditorIfPage deleteOpcode" << (_opcodeID+1) << _script->opcode(_opcodeID+1)->name();
//		_script->delOpcode(_opcodeID+1);
//		addJump = false;
//	}
}

OpcodeBox ScriptEditorIfPage::buildOpcode()
{
//	qDebug() << "opcode" << _opcode->name();
	if (rangeJump->currentIndex() == 0) { // short jump
		switch (opcode().id()) {
		case Opcode::IFUBL:		convertOpcode(Opcode::IFUB);	break;
		case Opcode::IFSWL:		convertOpcode(Opcode::IFSW);	break;
		case Opcode::IFUWL:		convertOpcode(Opcode::IFUW);	break;
		default: break;
		}
	} else { // long jump
		switch (opcode().id()) {
		case Opcode::IFUB:		convertOpcode(Opcode::IFUBL);	break;
		case Opcode::IFSW:		convertOpcode(Opcode::IFSWL);	break;
		case Opcode::IFUW:		convertOpcode(Opcode::IFUWL);	break;
		default: break;
		}
	}

	if (rangeTest->currentIndex() == 0) { // byte test
		switch (opcode().id()) {
		case Opcode::IFSW:
		case Opcode::IFUW:		convertOpcode(Opcode::IFUB);	break;
		case Opcode::IFSWL:
		case Opcode::IFUWL:		convertOpcode(Opcode::IFUBL);	break;
		default: break;
		}
	} else if (rangeTest->currentIndex() == 1) { // signed word test
		switch (opcode().id()) {
		case Opcode::IFUB:
		case Opcode::IFUW:		convertOpcode(Opcode::IFSW);	break;
		case Opcode::IFUBL:
		case Opcode::IFUWL:		convertOpcode(Opcode::IFSWL);	break;
		default: break;
		}
	} else { // unsigned word test
		switch (opcode().id()) {
		case Opcode::IFUB:
		case Opcode::IFSW:		convertOpcode(Opcode::IFUW);	break;
		case Opcode::IFUBL:
		case Opcode::IFSWL:		convertOpcode(Opcode::IFUWL);	break;
		default: break;
		}
	}

	OpcodeIf &opcodeIf = opcode().cast<OpcodeIf>();

	quint8 bank1, bank2;
	int value1, value2;

	if (varOrValue1->isValue()) {
		bank1 = 0;
		value1 = varOrValue1->value();
	} else {
		quint8 adress1;
		varOrValue1->var(bank1, adress1);
		value1 = adress1;
	}
	if (varOrValue2->isValue()) {
		bank2 = 0;
		value2 = varOrValue2->value();
	} else {
		quint8 adress2;
		varOrValue2->var(bank2, adress2);
		value2 = adress2;
	}

	opcodeIf.banks = quint8((bank1 << 4) | bank2);
	opcodeIf.value1 = value1;
	opcodeIf.value2 = value2;
	opcodeIf.oper = quint8(operatorList->currentIndex());

	quint32 labelVal = label->itemData(label->currentIndex()).toUInt();

	// Create a new label if chosen
//	if (label->currentIndex() == label->count() - 1) {
//		qDebug() << "ScriptEditorIfPage insertOpcode" << (_opcodeID+1);
//		_script->insertOpcode(_opcodeID+1, OpcodeBox(new OpcodeLabel(labelVal)));
//		addJump = true;
//	} else if (addJump) { // Remove the new label if not chosen
//		qDebug() << "ScriptEditorIfPage deleteOpcode" << (_opcodeID+1) << _script->opcode(_opcodeID+1)->name();
//		_script->delOpcode(_opcodeID+1);
//		addJump = false;
//	}

	opcodeIf.setLabel(labelVal);
//	qDebug() << "/opcode" << _opcode->name();
	return opcode();
}

void ScriptEditorIfPage::setOpcode(const OpcodeBox &opcode)
{
//	qDebug() << "setOpcode" << opcode->name();

	ScriptEditorView::setOpcode(opcode);

	addJump = false;

	const OpcodeIf &opcodeIf = opcode.cast<OpcodeIf>();
	varOrValue1->blockSignals(true);
	varOrValue2->blockSignals(true);
	if (opcode.id() == Opcode::IFUB || opcode.id() == Opcode::IFUBL) {
		varOrValue1->setLongValueType(false);
		varOrValue2->setLongValueType(false);
	} else {
		varOrValue1->setLongValueType(true);
		varOrValue2->setLongValueType(true);
	}

	if (opcode.id() == Opcode::IFSW || opcode.id() == Opcode::IFSWL) {
		varOrValue1->setSignedValueType(true);
		varOrValue2->setSignedValueType(true);
	} else {
		varOrValue1->setSignedValueType(false);
		varOrValue2->setSignedValueType(false);
	}

	if (B1(opcodeIf.banks) != 0) {
		varOrValue1->setIsValue(false);
		varOrValue1->setVar(B1(opcodeIf.banks), quint8(opcodeIf.value1));
	} else {
		varOrValue1->setIsValue(true);
		varOrValue1->setValue(opcodeIf.value1);
	}
	if (B2(opcodeIf.banks) != 0) {
		varOrValue2->setIsValue(false);
		varOrValue2->setVar(B2(opcodeIf.banks), quint8(opcodeIf.value2));
	} else {
		varOrValue2->setIsValue(true);
		varOrValue2->setValue(opcodeIf.value2);
	}
	varOrValue1->blockSignals(false);
	varOrValue2->blockSignals(false);

	operatorList->blockSignals(true);
	operatorList->setCurrentIndex(opcodeIf.oper);
	operatorList->blockSignals(false);

	fillLabelList();

	rangeJump->blockSignals(true);
	if (opcode.id() == Opcode::IFUB
			|| opcode.id() == Opcode::IFSW
			|| opcode.id() == Opcode::IFUW) {
		rangeJump->setCurrentIndex(0);
	} else {
		rangeJump->setCurrentIndex(1);
	}
	rangeJump->blockSignals(false);

	rangeTest->blockSignals(true);
	if (opcode.id() == Opcode::IFUB || opcode.id() == Opcode::IFUBL) {
		rangeTest->setCurrentIndex(0);
	} else if (opcode.id() == Opcode::IFSW || opcode.id() == Opcode::IFSWL) {
		rangeTest->setCurrentIndex(1);
	} else {
		rangeTest->setCurrentIndex(2);
	}
	rangeTest->blockSignals(false);

//	qDebug() << "/setOpcode" << opcode->name();
}

bool ScriptEditorIfPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

void ScriptEditorIfPage::changeTestRange()
{
//	qDebug() << "changeTestRange" << _opcode->name();
	setOpcode(opcode());

	emit opcodeChanged();
//	qDebug() << "/changeTestRange" << _opcode->name();
}

void ScriptEditorIfPage::convertOpcode(Opcode::Keys key)
{
	if (key == opcode().id()) {
		return;
	}

	const OpcodeIf &ifop = opcode().cast<OpcodeIf>();

	switch (key) {
	case Opcode::IFUB:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodeIFUB(ifop)));     break;
	case Opcode::IFUBL:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeIFUBL(ifop)));    break;
	case Opcode::IFSW:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodeIFSW(ifop)));     break;
	case Opcode::IFSWL:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeIFSWL(ifop)));    break;
	case Opcode::IFUW:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodeIFUW(ifop)));     break;
	case Opcode::IFUWL:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeIFUWL(ifop)));    break;
	default: break;
	}
}

ScriptEditorIfKeyPage::ScriptEditorIfKeyPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorIfKeyPage::build()
{
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Keys")), 0, 0, 1, 4);

	int row=1, column=0;

	for (const QString &keyName : qAsConst(Data::key_names)) {
		QCheckBox *key = new QCheckBox(keyName, this);
		keys.append(key);
		connect(key, SIGNAL(toggled(bool)), SIGNAL(opcodeChanged()));
		layout->addWidget(key, row, column);

		if (column == 3) {
			column = 0;
			++row;
		} else {
			++column;
		}
	}

	if (column != 0) {
		column = 0;
		++row;
	}

	label = new QComboBox(this);
	typeList = new QComboBox(this);
	typeList->addItem(tr("Key pressed"));
	typeList->addItem(tr("Key pressed once"));
	typeList->addItem(tr("Key released once"));

	layout->addWidget(new QLabel(tr("Type")), row, column);
	layout->addWidget(typeList, row, column+1, 1, 3);
	layout->addWidget(new QLabel(tr("Label")), row+1, column);
	layout->addWidget(label, row+1, column+1, 1, 3);
	layout->setRowStretch(row+2, 1);
	layout->setContentsMargins(QMargins());

	connect(label, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(typeList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

OpcodeBox ScriptEditorIfKeyPage::buildOpcode()
{
	if (typeList->currentIndex() == 0) {
		convertOpcode(Opcode::IFKEY);
	} else if (typeList->currentIndex() == 1) {
		convertOpcode(Opcode::IFKEYON);
	} else {
		convertOpcode(Opcode::IFKEYOFF);
	}

	OpcodeIfKey &opcodeIfKey = opcode().cast<OpcodeIfKey>();

	quint16 result = 0;

	for (int i=0; i<keys.size(); ++i) {
		if (keys.at(i)->isChecked()) {
			result |= 1 << i;
		}
	}

	opcodeIfKey.keys = result;
	opcodeIfKey.setLabel(label->itemData(label->currentIndex()).toUInt());

	return opcode();
}

void ScriptEditorIfKeyPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	const OpcodeIfKey &opcodeIfKey = opcode.cast<OpcodeIfKey>();

	switch (opcode.id()) {
	case Opcode::IFKEY:
		typeList->setCurrentIndex(0);
		break;
	case Opcode::IFKEYON:
		typeList->setCurrentIndex(1);
		break;
	case Opcode::IFKEYOFF:
		typeList->setCurrentIndex(2);
		break;
	default: break;
	}

	for (int i=0; i<keys.size(); ++i) {
		keys.at(i)->setChecked(bool((opcodeIfKey.keys >> i) & 1));
	}

	fillLabelList();

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

bool ScriptEditorIfKeyPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

void ScriptEditorIfKeyPage::convertOpcode(Opcode::Keys key)
{
	if (key == opcode().id()) {
		return;
	}

	const OpcodeIfKey &ifkey = opcode().cast<OpcodeIfKey>();

	switch (key) {
	case Opcode::IFKEY:       ScriptEditorView::setOpcode(OpcodeBox(new OpcodeIFKEY(ifkey)));       break;
	case Opcode::IFKEYON:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodeIFKEYON(ifkey)));     break;
	case Opcode::IFKEYOFF:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeIFKEYOFF(ifkey)));    break;
	default: break;
	}
}

ScriptEditorIfQPage::ScriptEditorIfQPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorIfQPage::build()
{
	charList = new QComboBox(this);
	charList->addItems(Data::char_names);
	int nbItems = charList->count();
	for (int i=nbItems; i<100; i++)
		charList->addItem(QString("%1").arg(i));
	for (int i=100; i<254; i++)
		charList->addItem(Data::char_names.last());

	charList->addItem(tr("(Empty)"));
	charList->addItem(tr("(Empty)"));

	label = new QComboBox(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Character")), 0, 0);
	layout->addWidget(charList, 0, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Label")), 1, 0);
	layout->addWidget(label, 1, 1, 1, 3);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(charList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(label, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

OpcodeBox ScriptEditorIfQPage::buildOpcode()
{
	OpcodeIfQ &opcodeIfQ = opcode().cast<OpcodeIfQ>();
	opcodeIfQ.charID = quint8(charList->currentIndex());
	opcodeIfQ.setLabel(label->itemData(label->currentIndex()).toUInt());

	return opcode();
}

void ScriptEditorIfQPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	const OpcodeIfQ &opcodeIfQ = opcode.cast<OpcodeIfQ>();
	charList->setCurrentIndex(opcodeIfQ.charID);

	fillLabelList();

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

bool ScriptEditorIfQPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

ScriptEditorWaitPage::ScriptEditorWaitPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorWaitPage::build()
{
	frames = new QSpinBox(this);
	frames->setRange(0, 65535);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Images")), 0, 0);
	layout->addWidget(frames, 0, 1);
	layout->setRowStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(frames, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
}

OpcodeBox ScriptEditorWaitPage::buildOpcode()
{
	OpcodeWAIT &opcodeWAIT = opcode().cast<OpcodeWAIT>();
	opcodeWAIT.frameCount = quint16(frames->value());

	return opcode();
}

void ScriptEditorWaitPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	const OpcodeWAIT &opcodeWAIT = opcode.cast<OpcodeWAIT>();
	frames->setValue(opcodeWAIT.frameCount);

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}
