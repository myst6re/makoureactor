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

ScriptEditorView::ScriptEditorView(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent)
	: QWidget(parent), _field(field), _grpScript(grpScript), _script(script),
	  _opcode(0), _opcodeID(opcodeID), _valid(true)
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

bool ScriptEditorView::isValid() const
{
	return _valid;
}

ScriptEditorReturnToPage::ScriptEditorReturnToPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
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
	OpcodeRETTO *opcodeRETTO = (OpcodeRETTO *)_opcode;
	opcodeRETTO->scriptID = scriptList->currentIndex();
	opcodeRETTO->priority = priority->value();

	return ScriptEditorView::opcode();
}

void ScriptEditorReturnToPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	OpcodeRETTO *opcodeRETTO = (OpcodeRETTO *)_opcode;
	scriptList->setCurrentIndex(opcodeRETTO->scriptID);
	priority->setValue(opcodeRETTO->priority);
}

ScriptEditorExecPage::ScriptEditorExecPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
	groupList = new QComboBox(this);
	int i=0;
	foreach(const QString &groupName, Data::currentGrpScriptNames) {
		groupList->addItem(QString("%1 - %2").arg(i++).arg(groupName));
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
	OpcodeExec *opcodeExec = (OpcodeExec *)_opcode;
	opcodeExec->groupID = groupList->currentIndex();
	opcodeExec->scriptID = scriptList->currentIndex();
	opcodeExec->priority = priority->value();

	return ScriptEditorView::opcode();
}

void ScriptEditorExecPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	switch((Opcode::Keys)opcode->id()) {
	case Opcode::REQ:		execType->setCurrentIndex(0);	break;
	case Opcode::REQSW:		execType->setCurrentIndex(1);	break;
	case Opcode::REQEW:		execType->setCurrentIndex(2);	break;
	default:	break;
	}

	OpcodeExec *opcodeExec = (OpcodeExec *)_opcode;

	GrpScript *grp = _field->grpScripts().value(opcodeExec->groupID, 0);
	if(!grp) {
		_valid = false;
		return;
	} else {
		_valid = true;
	}

	groupList->setCurrentIndex(opcodeExec->groupID);

	for(int i=0 ; i<32 ; ++i) {
		scriptList->setItemText(i, grp->getScriptName(i+1));
	}
	scriptList->setCurrentIndex(opcodeExec->scriptID);

	priority->setValue(opcodeExec->priority);
}

void ScriptEditorExecPage::updateScriptList(int groupID)
{
	GrpScript *grp = _field->grpScripts().value(groupID, 0);
	if(!grp) {
		return;
	}

	for(int i=0 ; i<32 ; ++i) {
		scriptList->setItemText(i, grp->getScriptName(i+1));
	}
}

void ScriptEditorExecPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	switch(index) {
	case 0:		key = Opcode::REQ;		break;
	case 1:		key = Opcode::REQSW;	break;
	case 2:		key = Opcode::REQEW;	break;
	default:	key = (Opcode::Keys)_opcode->id();
	}

	if(key != _opcode->id()) {
		convertOpcode(key);

		emit(opcodeChanged());
	}
}

void ScriptEditorExecPage::convertOpcode(Opcode::Keys key)
{
	if(key == _opcode->id())	return;

	OpcodeExec *exec = (OpcodeExec *)_opcode;

	switch(key) {
	case Opcode::REQ:		_opcode = new OpcodeREQ(*exec);		break;
	case Opcode::REQSW:		_opcode = new OpcodeREQSW(*exec);	break;
	case Opcode::REQEW:		_opcode = new OpcodeREQEW(*exec);	break;
	default:	return;
	}

	delete exec;
}

ScriptEditorExecCharPage::ScriptEditorExecCharPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
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
	OpcodeExecChar *opcodeExecChar = (OpcodeExecChar *)_opcode;
	opcodeExecChar->partyID = partyID->value();
	opcodeExecChar->scriptID = scriptList->currentIndex();
	opcodeExecChar->priority = priority->value();

	return ScriptEditorView::opcode();
}

void ScriptEditorExecCharPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	switch((Opcode::Keys)opcode->id()) {
	case Opcode::PREQ:		execType->setCurrentIndex(0);	break;
	case Opcode::PRQSW:		execType->setCurrentIndex(1);	break;
	case Opcode::PRQEW:		execType->setCurrentIndex(2);	break;
	default:	break;
	}

	OpcodeExecChar *opcodeExecChar = (OpcodeExecChar *)_opcode;
	partyID->setValue(opcodeExecChar->partyID);
	scriptList->setCurrentIndex(opcodeExecChar->scriptID);
	priority->setValue(opcodeExecChar->priority);
}


void ScriptEditorExecCharPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	switch(index) {
	case 0:		key = Opcode::PREQ;		break;
	case 1:		key = Opcode::PRQSW;	break;
	case 2:		key = Opcode::PRQEW;	break;
	default:	key = (Opcode::Keys)_opcode->id();
	}

	if(key != _opcode->id()) {
		convertOpcode(key);

		emit(opcodeChanged());
	}
}

void ScriptEditorExecCharPage::convertOpcode(Opcode::Keys key)
{
	if(key == _opcode->id())	return;

	OpcodeExecChar *exec = (OpcodeExecChar *)_opcode;

	switch(key) {
	case Opcode::PREQ:		_opcode = new OpcodePREQ(*exec);	break;
	case Opcode::PRQSW:		_opcode = new OpcodePRQSW(*exec);	break;
	case Opcode::PRQEW:		_opcode = new OpcodePRQEW(*exec);	break;
	default:	return;
	}

	delete exec;
}

ScriptEditorLabelPage::ScriptEditorLabelPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
	label = new QLabel(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Label")), 0, 0);
	layout->addWidget(label, 0, 1);
	layout->setRowStretch(1, 1);
	layout->setContentsMargins(QMargins());
}

Opcode *ScriptEditorLabelPage::opcode()
{
	OpcodeLabel *opcodeLabel = (OpcodeLabel *)_opcode;
	opcodeLabel->setLabel(label->text().toUInt());

	return ScriptEditorView::opcode();
}

void ScriptEditorLabelPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	OpcodeLabel *opcodeLabel = (OpcodeLabel *)opcode;
	if(opcodeLabel->label() == 0) {
		quint32 greaterLabel=1;
		foreach(Opcode *op, _script->getOpcodes()) {
			if(op->isLabel()) {
				quint32 lbl = ((OpcodeLabel *)op)->label();
				if(lbl >= greaterLabel) {
					greaterLabel = lbl + 1;
				}
			}
		}

		label->setNum((double)greaterLabel);
		emit opcodeChanged();
	} else {
		label->setNum((double)opcodeLabel->label());
	}
}

ScriptEditorJumpPage::ScriptEditorJumpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
	label = new QComboBox(this);

//	range = new QComboBox();
//	range->addItem(QString("Court (8 bits)"));
//	range->addItem(QString("Long (16 bits)"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Label")), 0, 0);
	layout->addWidget(label, 0, 1);
//	layout->addWidget(new QLabel(tr("Type")), 1, 0);
//	layout->addWidget(range, 1, 1);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(label, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
//	connect(range, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

Opcode *ScriptEditorJumpPage::opcode()
{
	OpcodeJump *opcodeJump = (OpcodeJump *)_opcode;
	opcodeJump->setLabel(label->itemData(label->currentIndex()).toUInt());

	return ScriptEditorView::opcode();
}

void ScriptEditorJumpPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	label->clear();

	OpcodeJump *opcodeJump = (OpcodeJump *)opcode;
	int i=0, pos=0, posOpcodeJump=-1, maxJump;
	bool isBackward = opcodeJump->id() == Opcode::JMPB || opcodeJump->id() == Opcode::JMPBL;

	foreach(Opcode *op, _script->getOpcodes()) {
		if(i == _opcodeID) {
			posOpcodeJump = pos;
			break;
		}
		pos += op->size();
		++i;
	}
	if(posOpcodeJump == -1) {
		posOpcodeJump = pos;
	}

	pos = i = 0;

	if(opcodeJump->isLongJump()) {
		maxJump = 65535 + opcodeJump->jumpPosData();
	} else {
		maxJump = 255 + opcodeJump->jumpPosData();
	}

	foreach(Opcode *op, _script->getOpcodes()) {

		if(op->isLabel() &&
				((isBackward && (i <= _opcodeID && pos >= posOpcodeJump - maxJump)) ||
				(!isBackward && (i >= _opcodeID && pos <= posOpcodeJump + maxJump)))) {
			quint32 lbl = ((OpcodeLabel *)op)->label();
			label->addItem(tr("Label %1").arg(lbl), lbl);
		}

		pos += op->size();
		++i;

		if(isBackward && i > _opcodeID)
			break;
	}

	_valid = label->count() > 0;

	if(_valid) {
		int index = label->findData(opcodeJump->label());
		if(index < 0)		index = 0;
		label->setCurrentIndex(index);
	}

//	if(opcode->id() == Opcode::JMPF || opcode->id() == Opcode::JMPB) {
//		range->setCurrentIndex(0);
//	} else {
//		range->setCurrentIndex(1);
//	}
}

ScriptEditorIfPage::ScriptEditorIfPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
	varOrValue1 = new VarOrValueWidget(this);
	varOrValue2 = new VarOrValueWidget(this);

	operatorList = new QComboBox(this);
	operatorList->addItems(Data::operateur_names);

	label = new QComboBox(this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Test à effectuer")), 0, 0);
	layout->addWidget(varOrValue1, 0, 1);
	layout->addWidget(operatorList, 0, 2);
	layout->addWidget(varOrValue2, 0, 3);
	layout->addWidget(new QLabel(tr("Label")), 1, 0);
	layout->addWidget(label, 1, 1, 1, 3);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(varOrValue1, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(varOrValue2, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(operatorList, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
	connect(label, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

Opcode *ScriptEditorIfPage::opcode()
{
	OpcodeIf *opcodeIf = (OpcodeIf *)_opcode;

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

	OpcodeJump *opcodeJump = (OpcodeJump *)_opcode;
	opcodeJump->setLabel(label->itemData(label->currentIndex()).toUInt());

	return ScriptEditorView::opcode();
}

void ScriptEditorIfPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	OpcodeIf *opcodeIf = (OpcodeIf *)opcode;

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

	if(opcodeIf->id() == Opcode::IFUB || opcodeIf->id() == Opcode::IFUBL) {
		varOrValue1->setLongValueType(false);
		varOrValue2->setLongValueType(false);
	} else {
		varOrValue1->setLongValueType(true);
		varOrValue2->setLongValueType(true);
	}

	if(opcodeIf->id() == Opcode::IFSW || opcodeIf->id() == Opcode::IFSWL) {
		varOrValue1->setSignedValueType(false);
		varOrValue2->setSignedValueType(false);
	} else {
		varOrValue1->setSignedValueType(true);
		varOrValue2->setSignedValueType(true);
	}

	operatorList->setCurrentIndex(opcodeIf->oper);

	label->clear();

	OpcodeJump *opcodeJump = (OpcodeJump *)opcode;

	int i=0, pos=0, posOpcodeJump=-1, maxJump;

	if(opcodeJump->isLongJump()) {
		maxJump = 65535 + opcodeJump->jumpPosData();
	} else {
		maxJump = 255 + opcodeJump->jumpPosData();
	}

	foreach(Opcode *op, _script->getOpcodes()) {
		if(i == _opcodeID) {
			posOpcodeJump = pos;
		}

		if(i >= _opcodeID && op->isLabel() && pos < posOpcodeJump + maxJump) {
			quint32 lbl = ((OpcodeLabel *)op)->label();
			label->addItem(tr("Label %1").arg(lbl), lbl);
		}

		pos += op->size();
		++i;
	}

	_valid = label->count() > 0;

	if(_valid) {
		int index = label->findData(opcodeJump->label());
		if(index < 0)		index = 0;
		label->setCurrentIndex(index);
	}
}

ScriptEditorIfKeyPage::ScriptEditorIfKeyPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
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

	layout->addWidget(new QLabel(tr("Label")), row, column);
	layout->addWidget(label, row, column+1, 1, 3);
	layout->setRowStretch(row+1, 1);
	layout->setContentsMargins(QMargins());

	connect(label, SIGNAL(currentIndexChanged(int)), SIGNAL(opcodeChanged()));
}

Opcode *ScriptEditorIfKeyPage::opcode()
{
	OpcodeIfKey *opcodeIfKey = (OpcodeIfKey *)_opcode;

	quint16 result = 0;

	for(int i=0 ; i<keys.size() ; ++i) {
		if(keys.at(i)->isChecked()) {
			result |= 1 << i;
		}
	}

	opcodeIfKey->keys = result;

	OpcodeJump *opcodeJump = (OpcodeJump *)_opcode;
	opcodeJump->setLabel(label->itemData(label->currentIndex()).toUInt());

	return ScriptEditorView::opcode();
}

void ScriptEditorIfKeyPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	OpcodeIfKey *opcodeIfKey = (OpcodeIfKey *)opcode;

	for(int i=0 ; i<keys.size() ; ++i) {
		keys.at(i)->setChecked(bool((opcodeIfKey->keys >> i) & 1));
	}

	label->clear();

	OpcodeJump *opcodeJump = (OpcodeJump *)opcode;

	int i=0, pos=0, posOpcodeJump=-1, maxJump;

	if(opcodeJump->isLongJump()) {
		maxJump = 65535 + opcodeJump->jumpPosData();
	} else {
		maxJump = 255 + opcodeJump->jumpPosData();
	}

	foreach(Opcode *op, _script->getOpcodes()) {
		if(i == _opcodeID) {
			posOpcodeJump = pos;
		}

		if(i >= _opcodeID && op->isLabel() && pos < posOpcodeJump + maxJump) {
			quint32 lbl = ((OpcodeLabel *)op)->label();
			label->addItem(tr("Label %1").arg(lbl), lbl);
		}

		pos += op->size();
		++i;
	}

	_valid = label->count() > 0;

	if(_valid) {
		int index = label->findData(opcodeJump->label());
		if(index < 0)		index = 0;
		label->setCurrentIndex(index);
	}
}

ScriptEditorIfQPage::ScriptEditorIfQPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
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
	OpcodeIfQ *opcodeIfQ = (OpcodeIfQ *)_opcode;
	opcodeIfQ->charID = charList->currentIndex();

	OpcodeJump *opcodeJump = (OpcodeJump *)_opcode;
	opcodeJump->setLabel(label->itemData(label->currentIndex()).toUInt());

	return ScriptEditorView::opcode();
}

void ScriptEditorIfQPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	OpcodeIfQ *opcodeIfQ = (OpcodeIfQ *)opcode;
	charList->setCurrentIndex(opcodeIfQ->charID);

	label->clear();

	OpcodeJump *opcodeJump = (OpcodeJump *)opcode;

	int i=0, pos=0, posOpcodeJump=-1, maxJump;

	if(opcodeJump->isLongJump()) {
		maxJump = 65535 + opcodeJump->jumpPosData();
	} else {
		maxJump = 255 + opcodeJump->jumpPosData();
	}

	foreach(Opcode *op, _script->getOpcodes()) {
		if(i == _opcodeID) {
			posOpcodeJump = pos;
		}

		if(i >= _opcodeID && op->isLabel() && pos < posOpcodeJump + maxJump) {
			quint32 lbl = ((OpcodeLabel *)op)->label();
			label->addItem(tr("Label %1").arg(lbl), lbl);
		}

		pos += op->size();
		++i;
	}

	_valid = label->count() > 0;

	if(_valid) {
		int index = label->findData(opcodeJump->label());
		if(index < 0)		index = 0;
		label->setCurrentIndex(index);
	}
}

ScriptEditorWaitPage::ScriptEditorWaitPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
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
	OpcodeWAIT *opcodeWAIT = (OpcodeWAIT *)_opcode;
	opcodeWAIT->frameCount = frames->value();

	return ScriptEditorView::opcode();
}

void ScriptEditorWaitPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	OpcodeWAIT *opcodeWAIT = (OpcodeWAIT *)opcode;
	frames->setValue(opcodeWAIT->frameCount);
}

ScriptEditorBinaryOpPage::ScriptEditorBinaryOpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
	var = new VarOrValueWidget(this);
	var->setOnlyVar(true);

	operationList = new QComboBox(this);
	operationList->addItem(tr("Affectation"));
	operationList->addItem(tr("Addition (cyclique)"));
	operationList->addItem(tr("Addition (plafonnée)"));
	operationList->addItem(tr("Soustraction (cyclique)"));
	operationList->addItem(tr("Soustraction (avec plancher à 0)"));
	operationList->addItem(tr("Multiplication"));
	operationList->addItem(tr("Division"));
	operationList->addItem(tr("Modulo"));
	operationList->addItem(tr("Et"));
	operationList->addItem(tr("Ou"));
	operationList->addItem(tr("Ou exclusif"));
	operationList->addItem(tr("Octet de poids faible"));
	operationList->addItem(tr("Octet de poids fort"));

	QWidget *typeGroup = new QWidget(this);
	type1 = new QRadioButton(tr("8 bits"), typeGroup);
	type2 = new QRadioButton(tr("16 bits"), typeGroup);
	QHBoxLayout *typeLayout = new QHBoxLayout(typeGroup);
	typeLayout->addWidget(type1);
	typeLayout->addWidget(type2);
	typeLayout->addStretch();
	typeLayout->setContentsMargins(QMargins());

	varOrValue = new VarOrValueWidget(this);
	varOrValue->setSignedValueType(false);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(var, 0, 0);
	layout->addWidget(operationList, 1, 0);
	layout->addWidget(typeGroup, 1, 1);
	layout->addWidget(varOrValue, 2, 0);
	layout->setRowStretch(3, 1);
	layout->setColumnStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(var, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(varOrValue, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(type1, SIGNAL(toggled(bool)), SLOT(updateValueRange()));
	connect(operationList, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
}

Opcode *ScriptEditorBinaryOpPage::opcode()
{
	OpcodeBinaryOperation *opcodeBinaryOperation = (OpcodeBinaryOperation *)_opcode;

	quint8 bank1, bank2, adress1;
	int value;

	var->var(bank1, adress1);

	if(varOrValue->isValue()) {
		bank2 = 0;
		value = varOrValue->value();
	} else {
		quint8 adress2;
		varOrValue->var(bank2, adress2);
		value = adress2;
	}

	opcodeBinaryOperation->banks = (bank1 << 4) | bank2;
	opcodeBinaryOperation->var = adress1;
	opcodeBinaryOperation->value = value;

	return ScriptEditorView::opcode();
}

void ScriptEditorBinaryOpPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	type1->setEnabled(true);
	type2->setEnabled(true);

	switch((Opcode::Keys)opcode->id()) {
	case Opcode::SETBYTE:case Opcode::SETWORD:
		operationList->setCurrentIndex(0);
		break;
	case Opcode::PLUS:case Opcode::PLUS2:
		operationList->setCurrentIndex(1);
		break;
	case Opcode::PLUSX:case Opcode::PLUS2X:
		operationList->setCurrentIndex(2);
		break;
	case Opcode::MINUS:case Opcode::MINUS2:
		operationList->setCurrentIndex(3);
		break;
	case Opcode::MINUSX:case Opcode::MINUS2X:
		operationList->setCurrentIndex(4);
		break;
	case Opcode::MUL:case Opcode::MUL2:
		operationList->setCurrentIndex(5);
		break;
	case Opcode::DIV:case Opcode::DIV2:
		operationList->setCurrentIndex(6);
		break;
	case Opcode::MOD:case Opcode::MOD2:
		operationList->setCurrentIndex(7);
		break;
	case Opcode::AND:case Opcode::AND2:
		operationList->setCurrentIndex(8);
		break;
	case Opcode::OR:case Opcode::OR2:
		operationList->setCurrentIndex(9);
		break;
	case Opcode::XOR:case Opcode::XOR2:
		operationList->setCurrentIndex(10);
		break;
	case Opcode::LBYTE:
		operationList->setCurrentIndex(11);
		type1->setEnabled(false);
		type2->setEnabled(false);
		break;
	case Opcode::HBYTE:
		operationList->setCurrentIndex(12);
		type1->setEnabled(false);
		type2->setEnabled(false);
		break;
	default:
		break;
	}

	OpcodeBinaryOperation *opcodeBinaryOperation = (OpcodeBinaryOperation *)_opcode;

	var->setVar(B1(opcodeBinaryOperation->banks), opcodeBinaryOperation->var);

	varOrValue->setLongValueType(opcodeBinaryOperation->isLong());
	type1->setChecked(!opcodeBinaryOperation->isLong());
	type2->setChecked(opcodeBinaryOperation->isLong());
	if(B2(opcodeBinaryOperation->banks) != 0) {
		varOrValue->setVar(B2(opcodeBinaryOperation->banks), opcodeBinaryOperation->value & 0xFF);
		varOrValue->setIsValue(false);
	} else {
		varOrValue->setValue(opcodeBinaryOperation->value);
		varOrValue->setIsValue(true);
	}
}

void ScriptEditorBinaryOpPage::updateValueRange()
{
	varOrValue->setLongValueType(type2->isChecked());
	Opcode::Keys key = (Opcode::Keys)_opcode->id();

	if(type1->isChecked()) {
		switch(key) {
		case Opcode::SETWORD:	key = Opcode::SETBYTE;	break;
		case Opcode::PLUS2:		key = Opcode::PLUS;		break;
		case Opcode::PLUS2X:	key = Opcode::PLUSX;	break;
		case Opcode::MINUS2:	key = Opcode::MINUS;	break;
		case Opcode::MINUS2X:	key = Opcode::MINUSX;	break;
		case Opcode::MUL2:		key = Opcode::MUL;		break;
		case Opcode::DIV2:		key = Opcode::DIV;		break;
		case Opcode::MOD2:		key = Opcode::MOD;		break;
		case Opcode::AND2:		key = Opcode::AND;		break;
		case Opcode::OR2:		key = Opcode::OR;		break;
		case Opcode::XOR2:		key = Opcode::XOR;		break;
		default:	break;
		}
	} else {
		switch(key) {
		case Opcode::SETBYTE:	key = Opcode::SETWORD;	break;
		case Opcode::PLUS:		key = Opcode::PLUS2;	break;
		case Opcode::PLUSX:		key = Opcode::PLUS2X;	break;
		case Opcode::MINUS:		key = Opcode::MINUS2;	break;
		case Opcode::MINUSX:	key = Opcode::MINUS2X;	break;
		case Opcode::MUL:		key = Opcode::MUL2;		break;
		case Opcode::DIV:		key = Opcode::DIV2;		break;
		case Opcode::MOD:		key = Opcode::MOD2;		break;
		case Opcode::AND:		key = Opcode::AND2;		break;
		case Opcode::OR:		key = Opcode::OR2;		break;
		case Opcode::XOR:		key = Opcode::XOR2;		break;
		default:	break;
		}
	}

	if(key != _opcode->id()) {
		convertOpcode(key);

		emit(opcodeChanged());
	}
}

void ScriptEditorBinaryOpPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	type1->setEnabled(true);
	type2->setEnabled(true);

	switch(index) {
	case 0:
		key = type1->isChecked()
				? Opcode::SETBYTE
				: Opcode::SETWORD;
		break;
	case 1:
		key = type1->isChecked()
				? Opcode::PLUS
				: Opcode::PLUS2;
		break;
	case 2:
		key = type1->isChecked()
				? Opcode::PLUSX
				: Opcode::PLUS2X;
		break;
	case 3:
		key = type1->isChecked()
				? Opcode::MINUS
				: Opcode::MINUS2;
		break;
	case 4:
		key = type1->isChecked()
				? Opcode::MINUSX
				: Opcode::MINUS2X;
		break;
	case 5:
		key = type1->isChecked()
				? Opcode::MUL
				: Opcode::MUL2;
		break;
	case 6:
		key = type1->isChecked()
				? Opcode::DIV
				: Opcode::DIV2;
		break;
	case 7:
		key = type1->isChecked()
				? Opcode::MOD
				: Opcode::MOD2;
		break;
	case 8:
		key = type1->isChecked()
				? Opcode::AND
				: Opcode::AND2;
		break;
	case 9:
		key = type1->isChecked()
				? Opcode::OR
				: Opcode::OR2;
		break;
	case 10:
		key = type1->isChecked()
				? Opcode::XOR
				: Opcode::XOR2;
		break;
	case 11:
		key = Opcode::LBYTE;
		type1->setEnabled(false);
		type2->setEnabled(false);
		type1->blockSignals(true);
		type2->blockSignals(true);
		type1->setChecked(true);
		type2->setChecked(false);
		type1->blockSignals(false);
		type2->blockSignals(false);
		break;
	case 12:
		key = Opcode::HBYTE;
		type1->setEnabled(false);
		type2->setEnabled(false);
		type1->blockSignals(true);
		type2->blockSignals(true);
		type1->setChecked(false);
		type2->setChecked(true);
		type1->blockSignals(false);
		type2->blockSignals(false);
		break;
	default:
		key = (Opcode::Keys)_opcode->id();
		break;
	}

	if(key != _opcode->id()) {
		convertOpcode(key);

		emit(opcodeChanged());
	}
}

void ScriptEditorBinaryOpPage::convertOpcode(Opcode::Keys key)
{
	if(key == _opcode->id())	return;

	OpcodeBinaryOperation *binop = (OpcodeBinaryOperation *)_opcode;

	switch(key) {
	case Opcode::SETBYTE:	_opcode = new OpcodeSETBYTE(*binop);	break;
	case Opcode::SETWORD:	_opcode = new OpcodeSETWORD(*binop);	break;
	case Opcode::PLUS:		_opcode = new OpcodePLUS(*binop);		break;
	case Opcode::PLUS2:		_opcode = new OpcodePLUS2(*binop);		break;
	case Opcode::PLUSX:		_opcode = new OpcodePLUSX(*binop);		break;
	case Opcode::PLUS2X:	_opcode = new OpcodePLUS2X(*binop);		break;
	case Opcode::MINUS:		_opcode = new OpcodeMINUS(*binop);		break;
	case Opcode::MINUS2:	_opcode = new OpcodeMINUS2(*binop);		break;
	case Opcode::MINUSX:	_opcode = new OpcodeMINUSX(*binop);		break;
	case Opcode::MINUS2X:	_opcode = new OpcodeMINUS2X(*binop);	break;
	case Opcode::MUL:		_opcode = new OpcodeMUL(*binop);		break;
	case Opcode::MUL2:		_opcode = new OpcodeMUL2(*binop);		break;
	case Opcode::DIV:		_opcode = new OpcodeDIV(*binop);		break;
	case Opcode::DIV2:		_opcode = new OpcodeDIV2(*binop);		break;
	case Opcode::MOD:		_opcode = new OpcodeMOD(*binop);		break;
	case Opcode::MOD2:		_opcode = new OpcodeMOD2(*binop);		break;
	case Opcode::AND:		_opcode = new OpcodeAND(*binop);		break;
	case Opcode::AND2:		_opcode = new OpcodeAND2(*binop);		break;
	case Opcode::OR:		_opcode = new OpcodeOR(*binop);			break;
	case Opcode::OR2:		_opcode = new OpcodeOR2(*binop);		break;
	case Opcode::XOR:		_opcode = new OpcodeXOR(*binop);		break;
	case Opcode::XOR2:		_opcode = new OpcodeXOR2(*binop);		break;
	case Opcode::LBYTE:		_opcode = new OpcodeLBYTE(*binop);		break;
	case Opcode::HBYTE: 	_opcode = new OpcodeHBYTE(*binop);		break;
	default:	return;
	}

	delete binop;
}

ScriptEditorUnaryOpPage::ScriptEditorUnaryOpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
	var = new VarOrValueWidget(this);
	var->setOnlyVar(true);

	operationList = new QComboBox(this);
	operationList->addItem(tr("Incrémentation (cyclique)"));
	operationList->addItem(tr("Incrémentation (plafonnée)"));
	operationList->addItem(tr("Décrémentation (cyclique)"));
	operationList->addItem(tr("Décrémentation (avec plancher à 0)"));
	operationList->addItem(tr("Aléatoire"));

	QWidget *typeGroup = new QWidget(this);
	type1 = new QRadioButton(tr("8 bits"), typeGroup);
	type2 = new QRadioButton(tr("16 bits"), typeGroup);
	QHBoxLayout *typeLayout = new QHBoxLayout(typeGroup);
	typeLayout->addWidget(type1);
	typeLayout->addWidget(type2);
	typeLayout->addStretch();
	typeLayout->setContentsMargins(QMargins());

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(var, 0, 0);
	layout->addWidget(operationList, 1, 0);
	layout->addWidget(typeGroup, 1, 1);
	layout->setRowStretch(2, 1);
	layout->setColumnStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(var, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(type1, SIGNAL(toggled(bool)), SLOT(updateValueRange()));
	connect(operationList, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
}

Opcode *ScriptEditorUnaryOpPage::opcode()
{
	OpcodeUnaryOperation *opcodeUnaryOperation = (OpcodeUnaryOperation *)_opcode;

	quint8 bank2, adress;

	var->var(bank2, adress);

	opcodeUnaryOperation->banks = bank2;
	opcodeUnaryOperation->var = adress;

	return ScriptEditorView::opcode();
}

void ScriptEditorUnaryOpPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	type1->setEnabled(true);
	type2->setEnabled(true);

	switch((Opcode::Keys)opcode->id()) {
	case Opcode::INC:case Opcode::INC2:
		operationList->setCurrentIndex(0);
		break;
	case Opcode::INCX:case Opcode::INC2X:
		operationList->setCurrentIndex(1);
		break;
	case Opcode::DEC:case Opcode::DEC2:
		operationList->setCurrentIndex(2);
		break;
	case Opcode::DECX:case Opcode::DEC2X:
		operationList->setCurrentIndex(3);
		break;
	case Opcode::RANDOM:
		operationList->setCurrentIndex(4);
		type1->setEnabled(false);
		type2->setEnabled(false);
		break;
	default:
		break;
	}

	OpcodeUnaryOperation *opcodeUnaryOperation = (OpcodeUnaryOperation *)_opcode;

	var->setVar(B2(opcodeUnaryOperation->banks), opcodeUnaryOperation->var);

	type1->setChecked(!opcodeUnaryOperation->isLong());
	type2->setChecked(opcodeUnaryOperation->isLong());
}

void ScriptEditorUnaryOpPage::updateValueRange()
{
	Opcode::Keys key = (Opcode::Keys)_opcode->id();

	if(type1->isChecked()) {
		switch(key) {
		case Opcode::INC2X:	key = Opcode::INCX;	break;
		case Opcode::INC2:	key = Opcode::INC;	break;
		case Opcode::DEC2X:	key = Opcode::DECX;	break;
		case Opcode::DEC2:	key = Opcode::DEC;	break;
		default:	break;
		}
	} else {
		switch(key) {
		case Opcode::INCX:	key = Opcode::INC2X;	break;
		case Opcode::INC:	key = Opcode::INC2;		break;
		case Opcode::DECX:	key = Opcode::DEC2X;	break;
		case Opcode::DEC:	key = Opcode::DEC2;		break;
		default:	break;
		}
	}

	if(key != _opcode->id()) {
		convertOpcode(key);

		emit(opcodeChanged());
	}
}

void ScriptEditorUnaryOpPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	type1->setEnabled(true);
	type2->setEnabled(true);

	switch(index) {
	case 0:
		key = type1->isChecked()
				? Opcode::INC
				: Opcode::INC2;
		break;
	case 1:
		key = type1->isChecked()
				? Opcode::INCX
				: Opcode::INC2X;
		break;
	case 2:
		key = type1->isChecked()
				? Opcode::DEC
				: Opcode::DEC2;
		break;
	case 3:
		key = type1->isChecked()
				? Opcode::DECX
				: Opcode::DEC2X;
		break;
	case 4:
		key = Opcode::RANDOM;
		type1->setEnabled(false);
		type2->setEnabled(false);
		break;
	default:
		key = (Opcode::Keys)_opcode->id();
		break;
	}

	if(key != _opcode->id()) {
		convertOpcode(key);

		emit(opcodeChanged());

		type1->blockSignals(true);
		type2->blockSignals(true);
		type1->setChecked(!((OpcodeUnaryOperation *)_opcode)->isLong());
		type2->setChecked(((OpcodeUnaryOperation *)_opcode)->isLong());
		type1->blockSignals(false);
		type2->blockSignals(false);
	}
}

void ScriptEditorUnaryOpPage::convertOpcode(Opcode::Keys key)
{
	if(key == _opcode->id())	return;

	OpcodeUnaryOperation *unop = (OpcodeUnaryOperation *)_opcode;

	switch(key) {
	case Opcode::INC:		_opcode = new OpcodeINC(*unop);		break;
	case Opcode::INC2:		_opcode = new OpcodeINC2(*unop);	break;
	case Opcode::INCX:		_opcode = new OpcodeINCX(*unop);	break;
	case Opcode::INC2X:		_opcode = new OpcodeINC2X(*unop);	break;
	case Opcode::DEC:		_opcode = new OpcodeDEC(*unop);		break;
	case Opcode::DEC2:		_opcode = new OpcodeDEC2(*unop);	break;
	case Opcode::DECX:		_opcode = new OpcodeDECX(*unop);	break;
	case Opcode::DEC2X:		_opcode = new OpcodeDEC2X(*unop);	break;
	case Opcode::RANDOM:	_opcode = new OpcodeRANDOM(*unop);	break;
	default:	return;
	}

	delete unop;
}

ScriptEditorBitOpPage::ScriptEditorBitOpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
	var = new VarOrValueWidget(this);
	var->setOnlyVar(true);

	position = new VarOrValueWidget(this);
	position->setLongValueType(false);
	position->setSignedValueType(false);

	operationList = new QComboBox(this);
	operationList->addItem(tr("Mettre un bit à 1"));
	operationList->addItem(tr("Mettre un bit à 0"));
	operationList->addItem(tr("Inverser la valeur d'un bit"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Variable")), 0, 0);
	layout->addWidget(var, 0, 1);
	layout->addWidget(new QLabel(tr("Position")), 1, 0);
	layout->addWidget(position, 1, 1);
	layout->addWidget(new QLabel(tr("Opération")), 2, 0);
	layout->addWidget(operationList, 2, 1);
	layout->setRowStretch(3, 1);
	layout->setContentsMargins(QMargins());

	connect(var, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(position, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(operationList, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
}

Opcode *ScriptEditorBitOpPage::opcode()
{
	OpcodeBitOperation *opcodeBitOperation = (OpcodeBitOperation *)_opcode;

	quint8 bank1, bank2, adress1;
	int value;

	var->var(bank1, adress1);

	if(position->isValue()) {
		bank2 = 0;
		value = position->value();
	} else {
		quint8 adress2;
		position->var(bank2, adress2);
		value = adress2;
	}

	opcodeBitOperation->banks = (bank1 << 4) | bank2;
	opcodeBitOperation->var = adress1;
	opcodeBitOperation->position = value;

	return ScriptEditorView::opcode();
}

void ScriptEditorBitOpPage::setOpcode(Opcode *opcode)
{
	ScriptEditorView::setOpcode(opcode);

	switch((Opcode::Keys)opcode->id()) {
	case Opcode::BITON:
		operationList->setCurrentIndex(0);
		break;
	case Opcode::BITOFF:
		operationList->setCurrentIndex(1);
		break;
	case Opcode::BITXOR:
		operationList->setCurrentIndex(2);
		break;
	default:
		break;
	}

	OpcodeBitOperation *opcodeBitOperation = (OpcodeBitOperation *)opcode;

	var->setVar(B1(opcodeBitOperation->banks), opcodeBitOperation->var);

	if(B2(opcodeBitOperation->banks) != 0) {
		position->setVar(B2(opcodeBitOperation->banks), opcodeBitOperation->position);
		position->setIsValue(false);
	} else {
		position->setValue(opcodeBitOperation->position);
		position->setIsValue(true);
	}
}

void ScriptEditorBitOpPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	switch(index) {
	case 0:	key = Opcode::BITON;	break;
	case 1:	key = Opcode::BITOFF;	break;
	case 2:	key = Opcode::BITXOR;	break;
	default: key = (Opcode::Keys)_opcode->id();	break;
	}

	if(key != _opcode->id()) {
		convertOpcode(key);

		emit(opcodeChanged());
	}
}

void ScriptEditorBitOpPage::convertOpcode(Opcode::Keys key)
{
	if(key == _opcode->id())	return;

	OpcodeBitOperation *bitop = (OpcodeBitOperation *)_opcode;

	switch(key) {
	case Opcode::BITON:		_opcode = new OpcodeBITON(*bitop);	break;
	case Opcode::BITOFF:	_opcode = new OpcodeBITOFF(*bitop);	break;
	case Opcode::BITXOR:	_opcode = new OpcodeBITXOR(*bitop);	break;
	default:	return;
	}

	delete bitop;
}
