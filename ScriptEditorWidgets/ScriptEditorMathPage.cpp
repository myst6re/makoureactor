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
#include "ScriptEditorMathPage.h"

ScriptEditorBinaryOpPage::ScriptEditorBinaryOpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorBinaryOpPage::build()
{
	qDebug() << "ScriptEditorBinaryOpPage";

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
}

void ScriptEditorUnaryOpPage::build()
{
	qDebug() << "ScriptEditorUnaryOpPage";

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
}

void ScriptEditorBitOpPage::build()
{
	qDebug() << "ScriptEditorBitOpPage";

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
