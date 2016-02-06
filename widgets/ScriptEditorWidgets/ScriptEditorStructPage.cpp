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
#include "ScriptEditorStructPage.h"
#include "Data.h"

ScriptEditorReturnToPage::ScriptEditorReturnToPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorReturnToPage::build()
{
	scriptList = new QComboBox(this);
	for(int i=0 ; i<32 ; ++i) {
		scriptList->addItem(tr("Script %1").arg(i));
	}

	priority = new QSpinBox(this);
	priority->setRange(0, 7);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Script")), 0, 0);
	layout->addWidget(scriptList, 0, 1);
	layout->addWidget(new QLabel(tr("Priorité")), 1, 0);
	layout->addWidget(priority, 1, 1);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(scriptList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(priority, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
}

Opcode *ScriptEditorReturnToPage::opcode()
{
	OpcodeRETTO *opcodeRETTO = (OpcodeRETTO *)opcodePtr();
	opcodeRETTO->scriptID = scriptList->currentIndex();
	opcodeRETTO->priority = priority->value();

	return opcodePtr();
}

void ScriptEditorReturnToPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	foreach(QObject *o, children()) {
		o->blockSignals(true);
	}

	OpcodeRETTO *opcodeRETTO = (OpcodeRETTO *)opcode;
	scriptList->setCurrentIndex(opcodeRETTO->scriptID);
	priority->setValue(opcodeRETTO->priority);

	foreach(QObject *o, children()) {
		o->blockSignals(false);
	}
}

ScriptEditorExecPage::ScriptEditorExecPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorExecPage::build()
{
	groupList = new QComboBox(this);
	int i=0;
	foreach(const GrpScript *group, field()->scriptsAndTexts()->grpScripts()) {
		groupList->addItem(QString("%1 - %2").arg(i++).arg(group->name()));
	}

	scriptList = new QComboBox(this);
	for(int i=0 ; i<32 ; ++i) {
		scriptList->addItem("");
	}

	priority = new QSpinBox(this);
	priority->setRange(0, 7);

	execType = new QComboBox(this);
	execType->addItem(tr("Asynchrone, n'attend pas"));
	execType->addItem(tr("Asynchrone, attend"));
	execType->addItem(tr("Synchrone, attend"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Groupe")), 0, 0);
	layout->addWidget(groupList, 0, 1);
	layout->addWidget(new QLabel(tr("Script")), 1, 0);
	layout->addWidget(scriptList, 1, 1);
	layout->addWidget(new QLabel(tr("Priorité")), 2, 0);
	layout->addWidget(priority, 2, 1);
	layout->addWidget(new QLabel(tr("Type")), 3, 0);
	layout->addWidget(execType, 3, 1);
	layout->setRowStretch(4, 1);
	layout->setContentsMargins(QMargins());

	connect(groupList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(scriptList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(priority, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
	connect(groupList, SIGNAL(currentIndexChanged(int)), SLOT(updateScriptList(int)));
	connect(execType, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
}

Opcode *ScriptEditorExecPage::opcode()
{
	OpcodeExec *opcodeExec = (OpcodeExec *)opcodePtr();
	opcodeExec->groupID = groupList->currentIndex();
	opcodeExec->scriptID = scriptList->currentIndex();
	opcodeExec->priority = priority->value();

	return opcodePtr();
}

void ScriptEditorExecPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	foreach(QObject *o, children()) {
		o->blockSignals(true);
	}

	switch((Opcode::Keys)opcode->id()) {
	case Opcode::REQ:		execType->setCurrentIndex(0);	break;
	case Opcode::REQSW:		execType->setCurrentIndex(1);	break;
	case Opcode::REQEW:		execType->setCurrentIndex(2);	break;
	default:	break;
	}

	OpcodeExec *opcodeExec = (OpcodeExec *)opcode;

	GrpScript *grp = field()->scriptsAndTexts()->grpScripts().value(opcodeExec->groupID, 0);
	if(!grp) {
		setValid(false);
		return;
	} else {
		setValid(true);
	}

	groupList->setCurrentIndex(opcodeExec->groupID);

	for(int i=0 ; i<32 ; ++i) {
		scriptList->setItemText(i, grp->scriptName(i+1));
	}
	scriptList->setCurrentIndex(opcodeExec->scriptID);

	priority->setValue(opcodeExec->priority);

	foreach(QObject *o, children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorExecPage::updateScriptList(int groupID)
{
	GrpScript *grp = field()->scriptsAndTexts()->grpScripts().value(groupID, 0);
	if(!grp) {
		return;
	}

	for(int i=0 ; i<32 ; ++i) {
		scriptList->setItemText(i, grp->scriptName(i+1));
	}
}

void ScriptEditorExecPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	switch(index) {
	case 0:		key = Opcode::REQ;		break;
	case 1:		key = Opcode::REQSW;	break;
	case 2:		key = Opcode::REQEW;	break;
	default:	key = (Opcode::Keys)opcodePtr()->id();
	}

	if(key != opcodePtr()->id()) {
		convertOpcode(key);

		emit(opcodeChanged());
	}
}

Opcode *ScriptEditorExecPage::convertOpcode(Opcode::Keys key)
{
	if(key == opcodePtr()->id())	return opcodePtr();

	OpcodeExec *exec = (OpcodeExec *)opcodePtr();

	switch(key) {
	case Opcode::REQ:		ScriptEditorView::setOpcode(new OpcodeREQ(*exec));		break;
	case Opcode::REQSW:		ScriptEditorView::setOpcode(new OpcodeREQSW(*exec));	break;
	case Opcode::REQEW:		ScriptEditorView::setOpcode(new OpcodeREQEW(*exec));	break;
	default:	return opcodePtr();
	}

	delete exec;
	return opcodePtr();
}

ScriptEditorExecCharPage::ScriptEditorExecCharPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorExecCharPage::build()
{
	partyID = new QSpinBox(this);
	partyID->setRange(0, 255);
	scriptList = new QComboBox(this);
	for(int i=0 ; i<32 ; ++i) {
		scriptList->addItem(tr("Script %1").arg(i));
	}

	priority = new QSpinBox(this);
	priority->setRange(0, 7);

	execType = new QComboBox(this);
	execType->addItem(tr("Asynchrone, n'attend pas"));
	execType->addItem(tr("Asynchrone, attend"));
	execType->addItem(tr("Synchrone, attend"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Équipier")), 0, 0);
	layout->addWidget(partyID, 0, 1);
	layout->addWidget(new QLabel(tr("Script")), 1, 0);
	layout->addWidget(scriptList, 1, 1);
	layout->addWidget(new QLabel(tr("Priorité")), 2, 0);
	layout->addWidget(priority, 2, 1);
	layout->addWidget(new QLabel(tr("Type")), 3, 0);
	layout->addWidget(execType, 3, 1);
	layout->setRowStretch(4, 1);
	layout->setContentsMargins(QMargins());

	connect(partyID, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
	connect(scriptList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(priority, SIGNAL(valueChanged(int)), SIGNAL(opcodeChanged()));
	connect(execType, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
}

Opcode *ScriptEditorExecCharPage::opcode()
{
	Opcode *ret = opcodePtr();
	OpcodeExecChar *opcodeExecChar = (OpcodeExecChar *)ret;
	opcodeExecChar->partyID = partyID->value();
	opcodeExecChar->scriptID = scriptList->currentIndex();
	opcodeExecChar->priority = priority->value();

	return ret;
}

void ScriptEditorExecCharPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	foreach(QObject *o, children()) {
		o->blockSignals(true);
	}

	switch((Opcode::Keys)opcode->id()) {
	case Opcode::PREQ:		execType->setCurrentIndex(0);	break;
	case Opcode::PRQSW:		execType->setCurrentIndex(1);	break;
	case Opcode::PRQEW:		execType->setCurrentIndex(2);	break;
	default:	break;
	}

	OpcodeExecChar *opcodeExecChar = (OpcodeExecChar *)opcode;
	partyID->setValue(opcodeExecChar->partyID);
	scriptList->setCurrentIndex(opcodeExecChar->scriptID);
	priority->setValue(opcodeExecChar->priority);

	foreach(QObject *o, children()) {
		o->blockSignals(false);
	}
}


void ScriptEditorExecCharPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	switch(index) {
	case 0:		key = Opcode::PREQ;		break;
	case 1:		key = Opcode::PRQSW;	break;
	case 2:		key = Opcode::PRQEW;	break;
	default:	key = (Opcode::Keys)opcodePtr()->id();
	}

	if(key != opcodePtr()->id()) {
		convertOpcode(key);

		emit(opcodeChanged());
	}
}

Opcode *ScriptEditorExecCharPage::convertOpcode(Opcode::Keys key)
{
	if(key == opcodePtr()->id())	return opcodePtr();

	OpcodeExecChar *exec = (OpcodeExecChar *)opcodePtr();

	switch(key) {
	case Opcode::PREQ:		ScriptEditorView::setOpcode(new OpcodePREQ(*exec));		break;
	case Opcode::PRQSW:		ScriptEditorView::setOpcode(new OpcodePRQSW(*exec));	break;
	case Opcode::PRQEW:		ScriptEditorView::setOpcode(new OpcodePRQEW(*exec));	break;
	default:	return opcodePtr();
	}

	delete exec;
	return opcodePtr();
}

ScriptEditorLabelPage::ScriptEditorLabelPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorLabelPage::build()
{
	label = new QDoubleSpinBox(this);
	label->setDecimals(0);
	label->setRange(0, pow(2, 32)-1);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Label")), 0, 0);
	layout->addWidget(label, 0, 1);
	layout->setRowStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(label, SIGNAL(valueChanged(double)), SIGNAL(opcodeChanged()));
}

Opcode *ScriptEditorLabelPage::opcode()
{
	Opcode *ret = opcodePtr();
	OpcodeLabel *opcodeLabel = (OpcodeLabel *)ret;
	opcodeLabel->setLabel(label->value());

	return ret;
}

void ScriptEditorLabelPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	OpcodeLabel *opcodeLabel = (OpcodeLabel *)opcode;
	if(opcodeLabel->label() == 0) {
		quint32 greaterLabel=1;
		foreach(Opcode *op, script()->opcodes()) {
			if(op->isLabel()) {
				quint32 lbl = ((OpcodeLabel *)op)->label();
				if(lbl >= greaterLabel) {
					greaterLabel = lbl + 1;
				}
			}
		}

		label->setValue((double)greaterLabel);
	} else {
		label->blockSignals(true);
		label->setValue((double)opcodeLabel->label());
		label->blockSignals(false);
	}
}

void ScriptEditorJumpPageInterface::fillLabelList(bool jumpBack)
{
	OpcodeJump *opcodeJump = (OpcodeJump *)opcodePtr();

	label->blockSignals(true);
	label->clear();

	quint32 greaterLabel=1;
	int i=0;

	foreach(Opcode *op, script()->opcodes()) {
		if(op->isLabel()) {
			quint32 lbl = ((OpcodeLabel *)op)->label();
			if(jumpBack || i >= opcodeID()) {
				label->addItem(tr("Label %1").arg(lbl), lbl);
			}
			if(lbl >= greaterLabel) {
				greaterLabel = lbl + 1;
			}
		}

		++i;
	}

	label->addItem(tr("Nouveau label"), greaterLabel);

	int index = label->findData(opcodeJump->label());
	if(index < 0)		index = 0;
//	_valid = label->count() > 0;
	label->setCurrentIndex(index);
	label->blockSignals(false);
}

ScriptEditorJumpPage::ScriptEditorJumpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(field, grpScript, script, opcodeID, parent), addJump(false)
{
}

void ScriptEditorJumpPage::build()
{
	label = new QComboBox(this);

	range = new QComboBox();
	range->addItem(tr("Court (8 bits)"));
	range->addItem(tr("Long (16 bits)"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Label")), 0, 0);
	layout->addWidget(label, 0, 1);
	layout->addWidget(new QLabel(tr("Type")), 1, 0);
	layout->addWidget(range, 1, 1);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(label, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(range, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

void ScriptEditorJumpPage::clear()
{
//	if(addJump) {
//		qDebug() << "ScriptEditorJumpPage deleteOpcode" << (_opcodeID+1) << _script->opcode(_opcodeID+1)->name();
//		_script->delOpcode(_opcodeID+1);
//		addJump = false;
//	}
}

Opcode *ScriptEditorJumpPage::opcode()
{
	int opcodeIDLabel=0;
	quint32 labelVal = label->itemData(label->currentIndex()).toUInt();

	// Search opcode ID for label
	foreach(Opcode *op, script()->opcodes()) {
		if(op->isLabel() && ((OpcodeLabel *)op)->label() == labelVal) {
			break;
		}
		++opcodeIDLabel;
	}

	// Create a new label if chosen
//	if(opcodeIDLabel >= _script->size()) {
//		qDebug() << "ScriptEditorJumpPage insertOpcode" << (_opcodeID+1);
//		opcodeIDLabel = _opcodeID+1;
//		_script->insertOpcode(opcodeIDLabel, new OpcodeLabel(labelVal));
//		addJump = true;
//	}

	// Remove the new label if not chosen
//	if(addJump && label->currentIndex() < label->count() - 1) {
//		qDebug() << "ScriptEditorJumpPage deleteOpcode" << (_opcodeID+1) << _script->opcode(_opcodeID+1)->name();
//		_script->delOpcode(_opcodeID+1);
//		addJump = false;
//	}

	if(opcodeIDLabel < opcodeID()) { // Jump Back
		if(range->currentIndex() == 0) { // short
			convertOpcode(Opcode::JMPB);
		} else { // long
			convertOpcode(Opcode::JMPBL);
		}
	} else { // Jump Forward
		if(range->currentIndex() == 0) { // short
			convertOpcode(Opcode::JMPF);
		} else { // long
			convertOpcode(Opcode::JMPFL);
		}
	}

	OpcodeJump *opcodeJump = (OpcodeJump *)opcodePtr();
	opcodeJump->setLabel(labelVal);

	return opcodeJump;
}

void ScriptEditorJumpPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	foreach(QObject *o, children()) {
		o->blockSignals(true);
	}

	addJump = false;

	fillLabelList(true);

	if(opcode->id() == Opcode::JMPF || opcode->id() == Opcode::JMPB) {
		range->setCurrentIndex(0);
	} else {
		range->setCurrentIndex(1);
	}

	foreach(QObject *o, children()) {
		o->blockSignals(false);
	}

	emit opcodeChanged();
}

bool ScriptEditorJumpPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

Opcode *ScriptEditorJumpPage::convertOpcode(Opcode::Keys key)
{
	if(key == opcodePtr()->id())	return opcodePtr();

	OpcodeJump *jump = (OpcodeJump *)opcodePtr();

	switch(key) {
	case Opcode::JMPF:	ScriptEditorView::setOpcode(new OpcodeJMPF(*jump));		break;
	case Opcode::JMPFL:	ScriptEditorView::setOpcode(new OpcodeJMPFL(*jump));	break;
	case Opcode::JMPB:	ScriptEditorView::setOpcode(new OpcodeJMPB(*jump));		break;
	case Opcode::JMPBL:	ScriptEditorView::setOpcode(new OpcodeJMPBL(*jump));	break;
	default:	return opcodePtr();
	}

	delete jump;
	return opcodePtr();
}

ScriptEditorIfPage::ScriptEditorIfPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(field, grpScript, script, opcodeID, parent), addJump(false)
{
}

void ScriptEditorIfPage::build()
{
	varOrValue1 = new VarOrValueWidget(this);
	varOrValue2 = new VarOrValueWidget(this);

	operatorList = new QComboBox(this);
	for(int i=0 ; i<OPERATORS_SIZE; ++i) {
		operatorList->addItem(Opcode::operators[i]);
	}

	rangeTest = new QComboBox();
	rangeTest->addItem(tr("Sur 8 bits non signés"));
	rangeTest->addItem(tr("Sur 16 bits signés"));
	rangeTest->addItem(tr("Sur 16 bits non signés"));

	label = new QComboBox(this);

	rangeJump = new QComboBox();
	rangeJump->addItem(tr("Court (8 bits)"));
	rangeJump->addItem(tr("Long (16 bits)"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Test à effectuer")), 0, 0);
	layout->addWidget(varOrValue1, 0, 1);
	layout->addWidget(operatorList, 0, 2);
	layout->addWidget(varOrValue2, 0, 3);
	layout->addWidget(new QLabel(tr("Type de comparaison")), 1, 0);
	layout->addWidget(rangeTest, 1, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Label")), 2, 0);
	layout->addWidget(label, 2, 1, 1, 3);
	layout->addWidget(new QLabel(tr("Type de saut")), 3, 0);
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
//	if(addJump) {
//		qDebug() << "ScriptEditorIfPage deleteOpcode" << (_opcodeID+1) << _script->opcode(_opcodeID+1)->name();
//		_script->delOpcode(_opcodeID+1);
//		addJump = false;
//	}
}

Opcode *ScriptEditorIfPage::opcode()
{
//	qDebug() << "opcode" << _opcode->name();
	if(rangeJump->currentIndex() == 0) { // short jump
		switch(opcodePtr()->id()) {
		case Opcode::IFUBL:		convertOpcode(Opcode::IFUB);	break;
		case Opcode::IFSWL:		convertOpcode(Opcode::IFSW);	break;
		case Opcode::IFUWL:		convertOpcode(Opcode::IFUW);	break;
		}
	} else { // long jump
		switch(opcodePtr()->id()) {
		case Opcode::IFUB:		convertOpcode(Opcode::IFUBL);	break;
		case Opcode::IFSW:		convertOpcode(Opcode::IFSWL);	break;
		case Opcode::IFUW:		convertOpcode(Opcode::IFUWL);	break;
		}
	}

	if(rangeTest->currentIndex() == 0) { // byte test
		switch(opcodePtr()->id()) {
		case Opcode::IFSW:
		case Opcode::IFUW:		convertOpcode(Opcode::IFUB);	break;
		case Opcode::IFSWL:
		case Opcode::IFUWL:		convertOpcode(Opcode::IFUBL);	break;
		}
	} else if(rangeTest->currentIndex() == 1) { // signed word test
		switch(opcodePtr()->id()) {
		case Opcode::IFUB:
		case Opcode::IFUW:		convertOpcode(Opcode::IFSW);	break;
		case Opcode::IFUBL:
		case Opcode::IFUWL:		convertOpcode(Opcode::IFSWL);	break;
		}
	} else { // unsigned word test
		switch(opcodePtr()->id()) {
		case Opcode::IFUB:
		case Opcode::IFSW:		convertOpcode(Opcode::IFUW);	break;
		case Opcode::IFUBL:
		case Opcode::IFSWL:		convertOpcode(Opcode::IFUWL);	break;
		}
	}

	OpcodeIf *opcodeIf = (OpcodeIf *)opcodePtr();

	quint8 bank1, bank2;
	int value1, value2;

	if(varOrValue1->isValue()) {
		bank1 = 0;
		value1 = varOrValue1->value();
	} else {
		quint8 adress1;
		varOrValue1->var(bank1, adress1);
		value1 = adress1;
	}
	if(varOrValue2->isValue()) {
		bank2 = 0;
		value2 = varOrValue2->value();
	} else {
		quint8 adress2;
		varOrValue2->var(bank2, adress2);
		value2 = adress2;
	}

	opcodeIf->banks = (bank1 << 4) | bank2;
	opcodeIf->value1 = value1;
	opcodeIf->value2 = value2;
	opcodeIf->oper = operatorList->currentIndex();

	quint32 labelVal = label->itemData(label->currentIndex()).toUInt();

	// Create a new label if chosen
//	if(label->currentIndex() == label->count() - 1) {
//		qDebug() << "ScriptEditorIfPage insertOpcode" << (_opcodeID+1);
//		_script->insertOpcode(_opcodeID+1, new OpcodeLabel(labelVal));
//		addJump = true;
//	} else if(addJump) { // Remove the new label if not chosen
//		qDebug() << "ScriptEditorIfPage deleteOpcode" << (_opcodeID+1) << _script->opcode(_opcodeID+1)->name();
//		_script->delOpcode(_opcodeID+1);
//		addJump = false;
//	}

	OpcodeJump *opcodeJump = (OpcodeJump *)opcodePtr();
	opcodeJump->setLabel(labelVal);
//	qDebug() << "/opcode" << _opcode->name();
	return opcodePtr();
}

void ScriptEditorIfPage::setOpcode(Opcode *opcode)
{
//	qDebug() << "setOpcode" << opcode->name();

	ScriptEditorView::setOpcode(opcode);

	addJump = false;

	OpcodeIf *opcodeIf = (OpcodeIf *)opcode;
	varOrValue1->blockSignals(true);
	varOrValue2->blockSignals(true);
	if(opcodeIf->id() == Opcode::IFUB || opcodeIf->id() == Opcode::IFUBL) {
		varOrValue1->setLongValueType(false);
		varOrValue2->setLongValueType(false);
	} else {
		varOrValue1->setLongValueType(true);
		varOrValue2->setLongValueType(true);
	}

	if(opcodeIf->id() == Opcode::IFSW || opcodeIf->id() == Opcode::IFSWL) {
		varOrValue1->setSignedValueType(true);
		varOrValue2->setSignedValueType(true);
	} else {
		varOrValue1->setSignedValueType(false);
		varOrValue2->setSignedValueType(false);
	}

	if(B1(opcodeIf->banks) != 0) {
		varOrValue1->setIsValue(false);
		varOrValue1->setVar(B1(opcodeIf->banks), opcodeIf->value1);
	} else {
		varOrValue1->setIsValue(true);
		varOrValue1->setValue(opcodeIf->value1);
	}
	if(B2(opcodeIf->banks) != 0) {
		varOrValue2->setIsValue(false);
		varOrValue2->setVar(B2(opcodeIf->banks), opcodeIf->value2);
	} else {
		varOrValue2->setIsValue(true);
		varOrValue2->setValue(opcodeIf->value2);
	}
	varOrValue1->blockSignals(false);
	varOrValue2->blockSignals(false);

	operatorList->blockSignals(true);
	operatorList->setCurrentIndex(opcodeIf->oper);
	operatorList->blockSignals(false);

	fillLabelList();

	rangeJump->blockSignals(true);
	if(opcode->id() == Opcode::IFUB
			|| opcode->id() == Opcode::IFSW
			|| opcode->id() == Opcode::IFUW) {
		rangeJump->setCurrentIndex(0);
	} else {
		rangeJump->setCurrentIndex(1);
	}
	rangeJump->blockSignals(false);

	rangeTest->blockSignals(true);
	if(opcode->id() == Opcode::IFUB || opcode->id() == Opcode::IFUBL) {
		rangeTest->setCurrentIndex(0);
	} else if(opcode->id() == Opcode::IFSW || opcode->id() == Opcode::IFSWL) {
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

Opcode *ScriptEditorIfPage::convertOpcode(Opcode::Keys key)
{
//	qDebug() << "convertOpcode" << _opcode->name();
	if(key == opcodePtr()->id()) {
//		qDebug() << "convertOpcode1" << _opcode->name();
		return opcodePtr();
	}

	OpcodeIf *ifop = (OpcodeIf *)opcodePtr();

	switch(key) {
	case Opcode::IFUB:	ScriptEditorView::setOpcode(new OpcodeIFUB(*ifop));		break;
	case Opcode::IFUBL:	ScriptEditorView::setOpcode(new OpcodeIFUBL(*ifop));	break;
	case Opcode::IFSW:	ScriptEditorView::setOpcode(new OpcodeIFSW(*ifop));		break;
	case Opcode::IFSWL:	ScriptEditorView::setOpcode(new OpcodeIFSWL(*ifop));	break;
	case Opcode::IFUW:	ScriptEditorView::setOpcode(new OpcodeIFUW(*ifop));		break;
	case Opcode::IFUWL:	ScriptEditorView::setOpcode(new OpcodeIFUWL(*ifop));	break;
	default:/*qDebug() << "/convertOpcode2" << _opcode->name();*/	return opcodePtr();
	}

	delete ifop;
//	qDebug() << "/convertOpcode" << _opcode->name();
	return opcodePtr();
}

ScriptEditorIfKeyPage::ScriptEditorIfKeyPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorIfKeyPage::build()
{
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Touches")), 0, 0, 1, 4);

	int row=1, column=0;

	foreach(const QString &keyName, Data::key_names) {
		QCheckBox *key = new QCheckBox(keyName, this);
		keys.append(key);
		connect(key, SIGNAL(toggled(bool)), SIGNAL(opcodeChanged()));
		layout->addWidget(key, row, column);

		if(column == 3) {
			column = 0;
			++row;
		} else {
			++column;
		}
	}

	if(column != 0) {
		column = 0;
		++row;
	}

	label = new QComboBox(this);
	typeList = new QComboBox(this);
	typeList->addItem(tr("Touche pressée"));
	typeList->addItem(tr("Touche pressée une fois"));
	typeList->addItem(tr("Touche lâchée une fois"));

	layout->addWidget(new QLabel(tr("Type")), row, column);
	layout->addWidget(typeList, row, column+1, 1, 3);
	layout->addWidget(new QLabel(tr("Label")), row+1, column);
	layout->addWidget(label, row+1, column+1, 1, 3);
	layout->setRowStretch(row+2, 1);
	layout->setContentsMargins(QMargins());

	connect(label, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(typeList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

Opcode *ScriptEditorIfKeyPage::opcode()
{
	if(typeList->currentIndex() == 0) {
		convertOpcode(Opcode::IFKEY);
	} else if(typeList->currentIndex() == 1) {
		convertOpcode(Opcode::IFKEYON);
	} else {
		convertOpcode(Opcode::IFKEYOFF);
	}

	OpcodeIfKey *opcodeIfKey = (OpcodeIfKey *)opcodePtr();

	quint16 result = 0;

	for(int i=0 ; i<keys.size() ; ++i) {
		if(keys.at(i)->isChecked()) {
			result |= 1 << i;
		}
	}

	opcodeIfKey->keys = result;

	OpcodeJump *opcodeJump = (OpcodeJump *)opcodePtr();
	opcodeJump->setLabel(label->itemData(label->currentIndex()).toUInt());

	return opcodePtr();
}

void ScriptEditorIfKeyPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	foreach(QObject *o, children()) {
		o->blockSignals(true);
	}

	OpcodeIfKey *opcodeIfKey = (OpcodeIfKey *)opcode;

	switch(opcode->id()) {
	case Opcode::IFKEY:
		typeList->setCurrentIndex(0);
		break;
	case Opcode::IFKEYON:
		typeList->setCurrentIndex(1);
		break;
	case Opcode::IFKEYOFF:
		typeList->setCurrentIndex(2);
		break;
	}

	for(int i=0 ; i<keys.size() ; ++i) {
		keys.at(i)->setChecked(bool((opcodeIfKey->keys >> i) & 1));
	}

	fillLabelList();

	foreach(QObject *o, children()) {
		o->blockSignals(false);
	}
}

bool ScriptEditorIfKeyPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

Opcode *ScriptEditorIfKeyPage::convertOpcode(Opcode::Keys key)
{
	if(key == opcodePtr()->id())	return opcodePtr();

	OpcodeIfKey *ifkey = (OpcodeIfKey *)opcodePtr();

	switch(key) {
	case Opcode::IFKEY:		ScriptEditorView::setOpcode(new OpcodeIFKEY(*ifkey));		break;
	case Opcode::IFKEYON:	ScriptEditorView::setOpcode(new OpcodeIFKEYON(*ifkey));		break;
	case Opcode::IFKEYOFF:	ScriptEditorView::setOpcode(new OpcodeIFKEYOFF(*ifkey));	break;
	default:	return opcodePtr();
	}

	delete ifkey;
	return opcodePtr();
}

ScriptEditorIfQPage::ScriptEditorIfQPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorJumpPageInterface(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorIfQPage::build()
{
	charList = new QComboBox(this);
	charList->addItems(Data::char_names);
	int nbItems = charList->count();
	for(int i=nbItems ; i<100 ; i++)
		charList->addItem(QString("%1").arg(i));
	for(int i=100 ; i<254 ; i++)
		charList->addItem(Data::char_names.last());

	charList->addItem(tr("(Vide)"));
	charList->addItem(tr("(Vide)"));

	label = new QComboBox(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Personnage")), 0, 0);
	layout->addWidget(charList, 0, 1);
	layout->addWidget(new QLabel(tr("Label")), 1, 0);
	layout->addWidget(label, 1, 1);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(charList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(label, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

Opcode *ScriptEditorIfQPage::opcode()
{
	OpcodeIfQ *opcodeIfQ = (OpcodeIfQ *)opcodePtr();
	opcodeIfQ->charID = charList->currentIndex();

	OpcodeJump *opcodeJump = (OpcodeJump *)opcodePtr();
	opcodeJump->setLabel(label->itemData(label->currentIndex()).toUInt());

	return opcodePtr();
}

void ScriptEditorIfQPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	foreach(QObject *o, children()) {
		o->blockSignals(true);
	}

	OpcodeIfQ *opcodeIfQ = (OpcodeIfQ *)opcode;
	charList->setCurrentIndex(opcodeIfQ->charID);

	fillLabelList();

	foreach(QObject *o, children()) {
		o->blockSignals(false);
	}
}

bool ScriptEditorIfQPage::needsLabel() const
{
	return label->currentIndex() == label->count() - 1;
}

ScriptEditorWaitPage::ScriptEditorWaitPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
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

Opcode *ScriptEditorWaitPage::opcode()
{
	OpcodeWAIT *opcodeWAIT = (OpcodeWAIT *)opcodePtr();
	opcodeWAIT->frameCount = frames->value();

	return opcodePtr();
}

void ScriptEditorWaitPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	foreach(QObject *o, children()) {
		o->blockSignals(true);
	}

	OpcodeWAIT *opcodeWAIT = (OpcodeWAIT *)opcode;
	frames->setValue(opcodeWAIT->frameCount);

	foreach(QObject *o, children()) {
		o->blockSignals(false);
	}
}
