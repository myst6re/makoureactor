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
#include "ScriptEditorMathPage.h"

ScriptEditorBinaryOpPage::ScriptEditorBinaryOpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorBinaryOpPage::build()
{
	var = new VarOrValueWidget(this);
	var->setOnlyVar(true);

	operationList = new QComboBox(this);
	operationList->addItem(tr("Assignment"));
	operationList->addItem(tr("Addition (wrapped)"));
	operationList->addItem(tr("Addition (capped)"));
	operationList->addItem(tr("Subtraction (wrapped)"));
	operationList->addItem(tr("Subtraction (capped)"));
	operationList->addItem(tr("Multiplication"));
	operationList->addItem(tr("Division"));
	operationList->addItem(tr("Modulo"));
	operationList->addItem(tr("And"));
	operationList->addItem(tr("Or"));
	operationList->addItem(tr("Exclusive or"));
	operationList->addItem(tr("Low-byte"));
	operationList->addItem(tr("High-byte"));

	QWidget *typeGroup = new QWidget(this);
	type1 = new QRadioButton(tr("8-bit"), typeGroup);
	type2 = new QRadioButton(tr("16-bit"), typeGroup);
	QHBoxLayout *typeLayout = new QHBoxLayout(typeGroup);
	typeLayout->addWidget(type1);
	typeLayout->addWidget(type2);
	typeLayout->addStretch();
	typeLayout->setContentsMargins(QMargins());

	varOrValue = new VarOrValueWidget(this);
	varOrValue->setSignedValueType(false);

	helpWidget = new HelpWidget(32, HelpWidget::IconInfo, this);
	helpWidget->setText(tr("It is not possible to divide per 0 or use mod 0, or the game will crash."));
	helpWidget->hide();

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(var, 0, 0);
	layout->addWidget(operationList, 1, 0);
	layout->addWidget(typeGroup, 1, 1);
	layout->addWidget(varOrValue, 2, 0);
	layout->addWidget(helpWidget, 4, 0, 1, 2, Qt::AlignBottom);
	layout->setRowStretch(3, 1);
	layout->setColumnStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(var, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(varOrValue, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(type1, SIGNAL(toggled(bool)), SLOT(updateValueRange()));
	connect(operationList, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
}

OpcodeBox ScriptEditorBinaryOpPage::buildOpcode()
{
	OpcodeBinaryOperation &opcodeBinaryOperation = opcode().cast<OpcodeBinaryOperation>();

	quint8 bank1, bank2, adress1;
	int value;

	var->var(bank1, adress1);

	if (varOrValue->isValue()) {
		bank2 = 0;
		value = varOrValue->value();
	} else {
		quint8 adress2;
		varOrValue->var(bank2, adress2);
		value = adress2;
	}

	opcodeBinaryOperation.banks = quint8((bank1 << 4) | bank2);
	opcodeBinaryOperation.var = adress1;
	opcodeBinaryOperation.value = quint16(value);

	return opcode();
}

void ScriptEditorBinaryOpPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	type1->setEnabled(true);
	type2->setEnabled(true);
	helpWidget->hide();

	switch (opcode.id()) {
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
		helpWidget->show();
		break;
	case Opcode::MOD:case Opcode::MOD2:
		operationList->setCurrentIndex(7);
		helpWidget->show();
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

	const OpcodeBinaryOperation &opcodeBinaryOperation = opcode.cast<OpcodeBinaryOperation>();

	var->setVar(B1(opcodeBinaryOperation.banks), opcodeBinaryOperation.var);

	varOrValue->setLongValueType(opcodeBinaryOperation.isLong());
	type1->setChecked(!opcodeBinaryOperation.isLong());
	type2->setChecked(opcodeBinaryOperation.isLong());
	if (B2(opcodeBinaryOperation.banks) != 0) {
		varOrValue->setVar(B2(opcodeBinaryOperation.banks), opcodeBinaryOperation.value & 0xFF);
		varOrValue->setIsValue(false);
	} else {
		varOrValue->setValue(opcodeBinaryOperation.value);
		varOrValue->setIsValue(true);
	}
	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorBinaryOpPage::updateValueRange()
{
	varOrValue->setLongValueType(type2->isChecked());
	Opcode::Keys key = opcode().id();

	if (type1->isChecked()) {
		switch (key) {
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
		switch (key) {
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

	if (key != opcode().id()) {
		convertOpcode(key);

		emit opcodeChanged();
	}
}

void ScriptEditorBinaryOpPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	type1->setEnabled(true);
	type2->setEnabled(true);
	helpWidget->hide();

	switch (index) {
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
		helpWidget->show();
		break;
	case 7:
		key = type1->isChecked()
				? Opcode::MOD
				: Opcode::MOD2;
		helpWidget->show();
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
		key = opcode().id();
		break;
	}

	if (key != opcode().id()) {
		convertOpcode(key);

		emit opcodeChanged();
	}
}

void ScriptEditorBinaryOpPage::convertOpcode(Opcode::Keys key)
{
	if (key == opcode().id()) {
		return;
	}
	
	const OpcodeBinaryOperation &binop = opcode().cast<OpcodeBinaryOperation>();
	
	switch (key) {
	case Opcode::SETBYTE:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeSETBYTE(binop)));    break;
	case Opcode::SETWORD:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeSETWORD(binop)));    break;
	case Opcode::PLUS:       ScriptEditorView::setOpcode(OpcodeBox(new OpcodePLUS(binop)));       break;
	case Opcode::PLUS2:      ScriptEditorView::setOpcode(OpcodeBox(new OpcodePLUS2(binop)));      break;
	case Opcode::PLUSX:      ScriptEditorView::setOpcode(OpcodeBox(new OpcodePLUSX(binop)));      break;
	case Opcode::PLUS2X:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodePLUS2X(binop)));     break;
	case Opcode::MINUS:      ScriptEditorView::setOpcode(OpcodeBox(new OpcodeMINUS(binop)));      break;
	case Opcode::MINUS2:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodeMINUS2(binop)));     break;
	case Opcode::MINUSX:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodeMINUSX(binop)));     break;
	case Opcode::MINUS2X:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeMINUS2X(binop)));    break;
	case Opcode::MUL:        ScriptEditorView::setOpcode(OpcodeBox(new OpcodeMUL(binop)));        break;
	case Opcode::MUL2:       ScriptEditorView::setOpcode(OpcodeBox(new OpcodeMUL2(binop)));       break;
	case Opcode::DIV:        ScriptEditorView::setOpcode(OpcodeBox(new OpcodeDIV(binop)));        break;
	case Opcode::DIV2:       ScriptEditorView::setOpcode(OpcodeBox(new OpcodeDIV2(binop)));       break;
	case Opcode::MOD:        ScriptEditorView::setOpcode(OpcodeBox(new OpcodeMOD(binop)));        break;
	case Opcode::MOD2:       ScriptEditorView::setOpcode(OpcodeBox(new OpcodeMOD2(binop)));       break;
	case Opcode::AND:        ScriptEditorView::setOpcode(OpcodeBox(new OpcodeAND(binop)));        break;
	case Opcode::AND2:       ScriptEditorView::setOpcode(OpcodeBox(new OpcodeAND2(binop)));       break;
	case Opcode::OR:         ScriptEditorView::setOpcode(OpcodeBox(new OpcodeOR(binop)));         break;
	case Opcode::OR2:        ScriptEditorView::setOpcode(OpcodeBox(new OpcodeOR2(binop)));        break;
	case Opcode::XOR:        ScriptEditorView::setOpcode(OpcodeBox(new OpcodeXOR(binop)));        break;
	case Opcode::XOR2:       ScriptEditorView::setOpcode(OpcodeBox(new OpcodeXOR2(binop)));       break;
	case Opcode::LBYTE:      ScriptEditorView::setOpcode(OpcodeBox(new OpcodeLBYTE(binop)));      break;
	case Opcode::HBYTE:      ScriptEditorView::setOpcode(OpcodeBox(new OpcodeHBYTE(binop)));      break;
	default: break;
	}
}

ScriptEditorUnaryOpPage::ScriptEditorUnaryOpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorUnaryOpPage::build()
{
	var = new VarOrValueWidget(this);
	var->setOnlyVar(true);

	operationList = new QComboBox(this);
	operationList->addItem(tr("Increment (wrapped)"));
	operationList->addItem(tr("Increment (capped)"));
	operationList->addItem(tr("Decrement (wrapped)"));
	operationList->addItem(tr("Decrement (capped)"));
	operationList->addItem(tr("Random"));

	QWidget *typeGroup = new QWidget(this);
	type1 = new QRadioButton(tr("8-bit"), typeGroup);
	type2 = new QRadioButton(tr("16-bit"), typeGroup);
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

OpcodeBox ScriptEditorUnaryOpPage::buildOpcode()
{
	OpcodeUnaryOperation &opcodeUnaryOperation = opcode().cast<OpcodeUnaryOperation>();

	quint8 bank2, adress;

	var->var(bank2, adress);

	opcodeUnaryOperation.banks = bank2;
	opcodeUnaryOperation.var = adress;

	return opcode();
}

void ScriptEditorUnaryOpPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	type1->setEnabled(true);
	type2->setEnabled(true);

	switch (opcode.id()) {
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

	const OpcodeUnaryOperation &opcodeUnaryOperation = opcode.cast<OpcodeUnaryOperation>();

	var->setVar(B2(opcodeUnaryOperation.banks), opcodeUnaryOperation.var);

	type1->setChecked(!opcodeUnaryOperation.isLong());
	type2->setChecked(opcodeUnaryOperation.isLong());

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorUnaryOpPage::updateValueRange()
{
	Opcode::Keys key = opcode().id();

	if (type1->isChecked()) {
		switch (key) {
		case Opcode::INC2X:    key = Opcode::INCX;    break;
		case Opcode::INC2:     key = Opcode::INC;     break;
		case Opcode::DEC2X:    key = Opcode::DECX;    break;
		case Opcode::DEC2:     key = Opcode::DEC;     break;
		default:	break;
		}
	} else {
		switch (key) {
		case Opcode::INCX:    key = Opcode::INC2X;    break;
		case Opcode::INC:     key = Opcode::INC2;     break;
		case Opcode::DECX:    key = Opcode::DEC2X;    break;
		case Opcode::DEC:     key = Opcode::DEC2;     break;
		default: break;
		}
	}

	if (key != opcode().id()) {
		convertOpcode(key);

		emit opcodeChanged();
	}
}

void ScriptEditorUnaryOpPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	type1->setEnabled(true);
	type2->setEnabled(true);

	switch (index) {
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
		key = opcode().id();
		break;
	}

	if (key != opcode().id()) {
		convertOpcode(key);

		emit opcodeChanged();

		type1->blockSignals(true);
		type2->blockSignals(true);
		type1->setChecked(!opcode().cast<OpcodeUnaryOperation>().isLong());
		type2->setChecked(opcode().cast<OpcodeUnaryOperation>().isLong());
		type1->blockSignals(false);
		type2->blockSignals(false);
	}
}

void ScriptEditorUnaryOpPage::convertOpcode(Opcode::Keys key)
{
	if (key == opcode().id()) {
		return;
	}

	const OpcodeUnaryOperation &unop = opcode().cast<OpcodeUnaryOperation>();

	switch (key) {
	case Opcode::INC:       ScriptEditorView::setOpcode(OpcodeBox(new OpcodeINC(unop)));       break;
	case Opcode::INC2:      ScriptEditorView::setOpcode(OpcodeBox(new OpcodeINC2(unop)));      break;
	case Opcode::INCX:      ScriptEditorView::setOpcode(OpcodeBox(new OpcodeINCX(unop)));      break;
	case Opcode::INC2X:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodeINC2X(unop)));     break;
	case Opcode::DEC:       ScriptEditorView::setOpcode(OpcodeBox(new OpcodeDEC(unop)));       break;
	case Opcode::DEC2:      ScriptEditorView::setOpcode(OpcodeBox(new OpcodeDEC2(unop)));      break;
	case Opcode::DECX:      ScriptEditorView::setOpcode(OpcodeBox(new OpcodeDECX(unop)));      break;
	case Opcode::DEC2X:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodeDEC2X(unop)));     break;
	case Opcode::RANDOM:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeRANDOM(unop)));    break;
	default: break;
	}
}

ScriptEditorBitOpPage::ScriptEditorBitOpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(field, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorBitOpPage::build()
{
	var = new VarOrValueWidget(this);
	var->setOnlyVar(true);

	position = new VarOrValueWidget(this);
	position->setLongValueType(false);
	position->setSignedValueType(false);

	operationList = new QComboBox(this);
	operationList->addItem(tr("Set a bit to 1"));
	operationList->addItem(tr("Set a bit to 0"));
	operationList->addItem(tr("Toggles a bit"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Variable")), 0, 0);
	layout->addWidget(var, 0, 1);
	layout->addWidget(new QLabel(tr("Position")), 1, 0);
	layout->addWidget(position, 1, 1);
	layout->addWidget(new QLabel(tr("Operation")), 2, 0);
	layout->addWidget(operationList, 2, 1);
	layout->setRowStretch(3, 1);
	layout->setContentsMargins(QMargins());

	connect(var, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(position, SIGNAL(changed()), SIGNAL(opcodeChanged()));
	connect(operationList, SIGNAL(currentIndexChanged(int)), SLOT(changeCurrentOpcode(int)));
}

OpcodeBox ScriptEditorBitOpPage::buildOpcode()
{
	OpcodeBitOperation &opcodeBitOperation = opcode().cast<OpcodeBitOperation>();

	quint8 bank1, bank2, adress1;
	int value;

	var->var(bank1, adress1);

	if (position->isValue()) {
		bank2 = 0;
		value = position->value();
	} else {
		quint8 adress2;
		position->var(bank2, adress2);
		value = adress2;
	}

	opcodeBitOperation.banks = quint8((bank1 << 4) | bank2);
	opcodeBitOperation.var = adress1;
	opcodeBitOperation.position = quint8(value);

	return opcode();
}

void ScriptEditorBitOpPage::setOpcode(const OpcodeBox &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	switch (opcode.id()) {
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

	const OpcodeBitOperation &opcodeBitOperation = opcode.cast<OpcodeBitOperation>();

	var->setVar(B1(opcodeBitOperation.banks), opcodeBitOperation.var);

	if (B2(opcodeBitOperation.banks) != 0) {
		position->setVar(B2(opcodeBitOperation.banks), opcodeBitOperation.position);
		position->setIsValue(false);
	} else {
		position->setValue(opcodeBitOperation.position);
		position->setIsValue(true);
	}

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorBitOpPage::changeCurrentOpcode(int index)
{
	Opcode::Keys key;

	switch (index) {
	case 0:     key = Opcode::BITON;     break;
	case 1:     key = Opcode::BITOFF;    break;
	case 2:     key = Opcode::BITXOR;    break;
	default:    key = opcode().id();     break;
	}

	if (key != opcode().id()) {
		convertOpcode(key);

		emit opcodeChanged();
	}
}

void ScriptEditorBitOpPage::convertOpcode(Opcode::Keys key)
{
	if (key == opcode().id()) {
		return;
	}

	const OpcodeBitOperation &bitop = opcode().cast<OpcodeBitOperation>();

	switch (key) {
	case Opcode::BITON:     ScriptEditorView::setOpcode(OpcodeBox(new OpcodeBITON(bitop)));     break;
	case Opcode::BITOFF:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeBITOFF(bitop)));    break;
	case Opcode::BITXOR:    ScriptEditorView::setOpcode(OpcodeBox(new OpcodeBITXOR(bitop)));    break;
	default: break;
	}
}
