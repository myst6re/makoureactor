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
#include "ScriptEditorMathPage.h"

ScriptEditorBinaryOpPage::ScriptEditorBinaryOpPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
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

	connect(var, &VarOrValueWidget::changed, this, &ScriptEditorBinaryOpPage::opcodeChanged);
	connect(varOrValue, &VarOrValueWidget::changed, this, &ScriptEditorBinaryOpPage::opcodeChanged);
	connect(type1, &QRadioButton::toggled, this, &ScriptEditorBinaryOpPage::updateValueRange);
	connect(operationList, &QComboBox::currentIndexChanged, this, &ScriptEditorBinaryOpPage::changeCurrentOpcode);
}

Opcode ScriptEditorBinaryOpPage::buildOpcode()
{
	quint8 bank1, bank2, address1;
	int value;

	var->var(bank1, address1);

	if (varOrValue->isValue()) {
		bank2 = 0;
		value = varOrValue->value();
	} else {
		quint8 address2;
		varOrValue->var(bank2, address2);
		value = address2;
	}

	FF7BinaryOperation opcodeBinaryOperation = FF7BinaryOperation();
	opcodeBinaryOperation.bank1 = bank1;
	opcodeBinaryOperation.bank2 = bank2;
	opcodeBinaryOperation.var = address1;
	opcodeBinaryOperation.value = quint16(value);

	opcode().setBinaryOperation(opcodeBinaryOperation);

	return opcode();
}

void ScriptEditorBinaryOpPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	type1->setEnabled(true);
	type2->setEnabled(true);
	helpWidget->hide();

	switch (opcode.id()) {
	case OpcodeKey::SETBYTE:case OpcodeKey::SETWORD:
		operationList->setCurrentIndex(0);
		break;
	case OpcodeKey::PLUS:case OpcodeKey::PLUS2:
		operationList->setCurrentIndex(1);
		break;
	case OpcodeKey::PLUSX:case OpcodeKey::PLUS2X:
		operationList->setCurrentIndex(2);
		break;
	case OpcodeKey::MINUS:case OpcodeKey::MINUS2:
		operationList->setCurrentIndex(3);
		break;
	case OpcodeKey::MINUSX:case OpcodeKey::MINUS2X:
		operationList->setCurrentIndex(4);
		break;
	case OpcodeKey::MUL:case OpcodeKey::MUL2:
		operationList->setCurrentIndex(5);
		break;
	case OpcodeKey::DIV:case OpcodeKey::DIV2:
		operationList->setCurrentIndex(6);
		helpWidget->show();
		break;
	case OpcodeKey::MOD:case OpcodeKey::MOD2:
		operationList->setCurrentIndex(7);
		helpWidget->show();
		break;
	case OpcodeKey::AND:case OpcodeKey::AND2:
		operationList->setCurrentIndex(8);
		break;
	case OpcodeKey::OR:case OpcodeKey::OR2:
		operationList->setCurrentIndex(9);
		break;
	case OpcodeKey::XOR:case OpcodeKey::XOR2:
		operationList->setCurrentIndex(10);
		break;
	case OpcodeKey::LBYTE:
		operationList->setCurrentIndex(11);
		type1->setEnabled(false);
		type2->setEnabled(false);
		break;
	case OpcodeKey::HBYTE:
		operationList->setCurrentIndex(12);
		type1->setEnabled(false);
		type2->setEnabled(false);
		break;
	default:
		break;
	}

	FF7BinaryOperation opcodeBinaryOperation = FF7BinaryOperation();
	opcode.binaryOperation(opcodeBinaryOperation);

	var->setVar(opcodeBinaryOperation.bank1, opcodeBinaryOperation.var);

	if (opcodeBinaryOperation.isLong) {
		var->setLong();
		varOrValue->setLong();
	} else {
		var->setShort();
		varOrValue->setShort();
	}
	type1->setChecked(!opcodeBinaryOperation.isLong);
	type2->setChecked(opcodeBinaryOperation.isLong);
	varOrValue->setVarOrValue(opcodeBinaryOperation.bank2, opcodeBinaryOperation.value);

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorBinaryOpPage::updateValueRange()
{
	OpcodeKey key = opcode().id();

	if (type1->isChecked()) {
		var->setShort();
		varOrValue->setShort();
		switch (key) {
		case OpcodeKey::SETWORD:
			key = OpcodeKey::SETBYTE;
			break;
		case OpcodeKey::PLUS2:
			key = OpcodeKey::PLUS;
			break;
		case OpcodeKey::PLUS2X:
			key = OpcodeKey::PLUSX;
			break;
		case OpcodeKey::MINUS2:
			key = OpcodeKey::MINUS;
			break;
		case OpcodeKey::MINUS2X:
			key = OpcodeKey::MINUSX;
			break;
		case OpcodeKey::MUL2:
			key = OpcodeKey::MUL;
			break;
		case OpcodeKey::DIV2:
			key = OpcodeKey::DIV;
			break;
		case OpcodeKey::MOD2:
			key = OpcodeKey::MOD;
			break;
		case OpcodeKey::AND2:
			key = OpcodeKey::AND;
			break;
		case OpcodeKey::OR2:
			key = OpcodeKey::OR;
			break;
		case OpcodeKey::XOR2:
			key = OpcodeKey::XOR;
			break;
		default:
			break;
		}
	} else {
		var->setLong();
		varOrValue->setLong();
		switch (key) {
		case OpcodeKey::SETBYTE:
			key = OpcodeKey::SETWORD;
			break;
		case OpcodeKey::PLUS:
			key = OpcodeKey::PLUS2;
			break;
		case OpcodeKey::PLUSX:
			key = OpcodeKey::PLUS2X;
			break;
		case OpcodeKey::MINUS:
			key = OpcodeKey::MINUS2;
			break;
		case OpcodeKey::MINUSX:
			key = OpcodeKey::MINUS2X;
			break;
		case OpcodeKey::MUL:
			key = OpcodeKey::MUL2;
			break;
		case OpcodeKey::DIV:
			key = OpcodeKey::DIV2;
			break;
		case OpcodeKey::MOD:
			key = OpcodeKey::MOD2;
			break;
		case OpcodeKey::AND:
			key = OpcodeKey::AND2;
			break;
		case OpcodeKey::OR:
			key = OpcodeKey::OR2;
			break;
		case OpcodeKey::XOR:
			key = OpcodeKey::XOR2;
			break;
		default:
			break;
		}
	}

	if (key != opcode().id()) {
		opcode().op().id = key;

		emit opcodeChanged();
	}
}

void ScriptEditorBinaryOpPage::changeCurrentOpcode(int index)
{
	OpcodeKey key;

	type1->setEnabled(true);
	type2->setEnabled(true);
	helpWidget->hide();

	switch (index) {
	case 0:
		key = type1->isChecked()
				? OpcodeKey::SETBYTE
				: OpcodeKey::SETWORD;
		break;
	case 1:
		key = type1->isChecked()
				? OpcodeKey::PLUS
				: OpcodeKey::PLUS2;
		break;
	case 2:
		key = type1->isChecked()
				? OpcodeKey::PLUSX
				: OpcodeKey::PLUS2X;
		break;
	case 3:
		key = type1->isChecked()
				? OpcodeKey::MINUS
				: OpcodeKey::MINUS2;
		break;
	case 4:
		key = type1->isChecked()
				? OpcodeKey::MINUSX
				: OpcodeKey::MINUS2X;
		break;
	case 5:
		key = type1->isChecked()
				? OpcodeKey::MUL
				: OpcodeKey::MUL2;
		break;
	case 6:
		key = type1->isChecked()
				? OpcodeKey::DIV
				: OpcodeKey::DIV2;
		helpWidget->show();
		break;
	case 7:
		key = type1->isChecked()
				? OpcodeKey::MOD
				: OpcodeKey::MOD2;
		helpWidget->show();
		break;
	case 8:
		key = type1->isChecked()
				? OpcodeKey::AND
				: OpcodeKey::AND2;
		break;
	case 9:
		key = type1->isChecked()
				? OpcodeKey::OR
				: OpcodeKey::OR2;
		break;
	case 10:
		key = type1->isChecked()
				? OpcodeKey::XOR
				: OpcodeKey::XOR2;
		break;
	case 11:
		key = OpcodeKey::LBYTE;
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
		key = OpcodeKey::HBYTE;
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
		opcode().op().id = key;

		emit opcodeChanged();
	}
}

ScriptEditorUnaryOpPage::ScriptEditorUnaryOpPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
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

	connect(var, &VarOrValueWidget::changed, this, &ScriptEditorUnaryOpPage::opcodeChanged);
	connect(type1, &QRadioButton::toggled, this, &ScriptEditorUnaryOpPage::updateValueRange);
	connect(operationList, &QComboBox::currentIndexChanged, this, &ScriptEditorUnaryOpPage::changeCurrentOpcode);
}

Opcode ScriptEditorUnaryOpPage::buildOpcode()
{
	quint8 bank2, address;

	var->var(bank2, address);

	FF7UnaryOperation opcodeUnaryOperation = FF7UnaryOperation();
	opcodeUnaryOperation.bank2 = bank2;
	opcodeUnaryOperation.var = address;

	opcode().setUnaryOperation(opcodeUnaryOperation);

	return opcode();
}

void ScriptEditorUnaryOpPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	type1->setEnabled(true);
	type2->setEnabled(true);

	switch (opcode.id()) {
	case OpcodeKey::INC:case OpcodeKey::INC2:
		operationList->setCurrentIndex(0);
		break;
	case OpcodeKey::INCX:case OpcodeKey::INC2X:
		operationList->setCurrentIndex(1);
		break;
	case OpcodeKey::DEC:case OpcodeKey::DEC2:
		operationList->setCurrentIndex(2);
		break;
	case OpcodeKey::DECX:case OpcodeKey::DEC2X:
		operationList->setCurrentIndex(3);
		break;
	case OpcodeKey::RANDOM:
		operationList->setCurrentIndex(4);
		type1->setEnabled(false);
		type2->setEnabled(false);
		break;
	default:
		break;
	}

	FF7UnaryOperation opcodeUnaryOperation = FF7UnaryOperation();
	opcode.unaryOperation(opcodeUnaryOperation);

	var->setVar(opcodeUnaryOperation.bank2, opcodeUnaryOperation.var);
	if (opcodeUnaryOperation.isLong) {
		var->setLong();
	} else {
		var->setShort();
	}

	type1->setChecked(!opcodeUnaryOperation.isLong);
	type2->setChecked(opcodeUnaryOperation.isLong);

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorUnaryOpPage::updateValueRange()
{
	OpcodeKey key = opcode().id();

	if (type1->isChecked()) {
		var->setShort();
		switch (key) {
		case OpcodeKey::INC2X:
			key = OpcodeKey::INCX;
			break;
		case OpcodeKey::INC2:
			key = OpcodeKey::INC;
			break;
		case OpcodeKey::DEC2X:
			key = OpcodeKey::DECX;
			break;
		case OpcodeKey::DEC2:
			key = OpcodeKey::DEC;
			break;
		default:
			break;
		}
	} else {
		var->setLong();
		switch (key) {
		case OpcodeKey::INCX:
			key = OpcodeKey::INC2X;
			break;
		case OpcodeKey::INC:
			key = OpcodeKey::INC2;
			break;
		case OpcodeKey::DECX:
			key = OpcodeKey::DEC2X;
			break;
		case OpcodeKey::DEC:
			key = OpcodeKey::DEC2;
			break;
		default:
			break;
		}
	}

	if (key != opcode().id()) {
		opcode().op().id = key;

		emit opcodeChanged();
	}
}

void ScriptEditorUnaryOpPage::changeCurrentOpcode(int index)
{
	OpcodeKey key;

	type1->setEnabled(true);
	type2->setEnabled(true);

	switch (index) {
	case 0:
		key = type1->isChecked()
		        ? OpcodeKey::INC
		        : OpcodeKey::INC2;
		break;
	case 1:
		key = type1->isChecked()
		        ? OpcodeKey::INCX
		        : OpcodeKey::INC2X;
		break;
	case 2:
		key = type1->isChecked()
		        ? OpcodeKey::DEC
		        : OpcodeKey::DEC2;
		break;
	case 3:
		key = type1->isChecked()
		        ? OpcodeKey::DECX
		        : OpcodeKey::DEC2X;
		break;
	case 4:
		key = OpcodeKey::RANDOM;
		type1->setEnabled(false);
		type2->setEnabled(false);
		break;
	default:
		key = opcode().id();
		break;
	}

	if (key != opcode().id()) {
		opcode().op().id = key;

		emit opcodeChanged();

		FF7UnaryOperation opcodeUnaryOperation = FF7UnaryOperation();
		opcode().unaryOperation(opcodeUnaryOperation);

		type1->blockSignals(true);
		type2->blockSignals(true);
		type1->setChecked(!opcodeUnaryOperation.isLong);
		type2->setChecked(opcodeUnaryOperation.isLong);
		type1->blockSignals(false);
		type2->blockSignals(false);
	}
}

ScriptEditorBitOpPage::ScriptEditorBitOpPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorBitOpPage::build()
{
	var = new VarOrValueWidget(this);
	var->setOnlyVar(true);
	var->setShort();

	position = new VarOrValueWidget(this);
	position->setShort();
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


	connect(var, &VarOrValueWidget::changed, this, &ScriptEditorBitOpPage::opcodeChanged);
	connect(position, &VarOrValueWidget::changed, this, &ScriptEditorBitOpPage::opcodeChanged);
	connect(operationList, &QComboBox::currentIndexChanged, this, &ScriptEditorBitOpPage::changeCurrentOpcode);
}

Opcode ScriptEditorBitOpPage::buildOpcode()
{
	quint8 bank1, bank2, address1;
	int value;

	var->var(bank1, address1);

	if (position->isValue()) {
		bank2 = 0;
		value = position->value();
	} else {
		quint8 address2;
		position->var(bank2, address2);
		value = address2;
	}

	FF7BitOperation opcodeBitOperation = FF7BitOperation();
	opcodeBitOperation.bank1 = bank1;
	opcodeBitOperation.bank2 = bank2;
	opcodeBitOperation.var = address1;
	opcodeBitOperation.position = quint8(value);

	opcode().setBitOperation(opcodeBitOperation);

	return opcode();
}

void ScriptEditorBitOpPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	switch (opcode.id()) {
	case OpcodeKey::BITON:
		operationList->setCurrentIndex(0);
		break;
	case OpcodeKey::BITOFF:
		operationList->setCurrentIndex(1);
		break;
	case OpcodeKey::BITXOR:
		operationList->setCurrentIndex(2);
		break;
	default:
		break;
	}

	FF7BitOperation opcodeBitOperation = FF7BitOperation();
	opcode.bitOperation(opcodeBitOperation);

	var->setVar(opcodeBitOperation.bank1, opcodeBitOperation.var);
	position->setVarOrValue(opcodeBitOperation.bank2, opcodeBitOperation.position);

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorBitOpPage::changeCurrentOpcode(int index)
{
	OpcodeKey key;

	switch (index) {
	case 0:
		key = OpcodeKey::BITON;
		break;
	case 1:
		key = OpcodeKey::BITOFF;
		break;
	case 2:
		key = OpcodeKey::BITXOR;
		break;
	default:
		key = opcode().id();
		break;
	}

	if (key != opcode().id()) {
		opcode().op().id = key;

		emit opcodeChanged();
	}
}

ScriptEditorVariablePage::ScriptEditorVariablePage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorVariablePage::build()
{
	varOrValue = new VarOrValueWidget(this);
	varOrValue->setSignedValueType(false);
	varOrValue->setShort();

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(varOrValue, 0, 0);
	layout->setRowStretch(1, 1);
	layout->setColumnStretch(1, 1);
	layout->setContentsMargins(QMargins());

	connect(varOrValue, &VarOrValueWidget::changed, this, &ScriptEditorVariablePage::opcodeChanged);
}

Opcode ScriptEditorVariablePage::buildOpcode()
{
	OpcodeRDMSD &opcodeRDMSD = opcode().op().opcodeRDMSD;

	quint8 value, bank2;

	if (varOrValue->isValue()) {
		bank2 = 0;
		value = quint8(varOrValue->value());
	} else {
		quint8 address2;
		varOrValue->var(bank2, address2);
		value = address2;
	}

	opcodeRDMSD.banks = bank2;
	opcodeRDMSD.value = value;

	return opcode();
}

void ScriptEditorVariablePage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	const OpcodeRDMSD &opcodeRDMSD = opcode.op().opcodeRDMSD;
	varOrValue->setVarOrValue(B2(opcodeRDMSD.banks), opcodeRDMSD.value);
}

ScriptEditor2BytePage::ScriptEditor2BytePage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditor2BytePage::build()
{
	var = new VarOrValueWidget(this);
	var->setOnlyVar(true);
	var->setLong();

	varOrValue1 = new VarOrValueWidget(this);
	varOrValue1->setSignedValueType(false);
	varOrValue1->setShort();

	varOrValue2 = new VarOrValueWidget(this);
	varOrValue2->setSignedValueType(false);
	varOrValue2->setShort();

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(var, 0, 0);
	layout->addWidget(varOrValue1, 1, 0);
	layout->addWidget(varOrValue2, 1, 1);
	layout->setRowStretch(2, 1);
	layout->setColumnStretch(2, 1);
	layout->setContentsMargins(QMargins());

	connect(var, &VarOrValueWidget::changed, this, &ScriptEditor2BytePage::opcodeChanged);
	connect(varOrValue1, &VarOrValueWidget::changed, this, &ScriptEditor2BytePage::opcodeChanged);
	connect(varOrValue2, &VarOrValueWidget::changed, this, &ScriptEditor2BytePage::opcodeChanged);}

Opcode ScriptEditor2BytePage::buildOpcode()
{
	quint8 bank1, bank2, bank4, address1;
	int value1, value2;

	var->var(bank1, address1);

	if (varOrValue1->isValue()) {
		bank2 = 0;
		value1 = varOrValue1->value();
	} else {
		quint8 address2;
		varOrValue1->var(bank2, address2);
		value1 = address2;
	}
	
	if (varOrValue2->isValue()) {
		bank4 = 0;
		value2 = varOrValue2->value();
	} else {
		quint8 address4;
		varOrValue2->var(bank4, address4);
		value2 = address4;
	}

	OpcodeTOBYTE &toByte = opcode().op().opcodeTOBYTE;
	toByte.banks[0] = BANK(bank1, bank2);
	toByte.banks[1] = BANK(0, bank4);
	toByte.value1 = quint8(value1);
	toByte.value2 = quint8(value2);
	toByte.var = address1;

	return opcode();
}

void ScriptEditor2BytePage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	const OpcodeTOBYTE &toByte = opcode.op().opcodeTOBYTE;

	var->setVar(B1(toByte.banks[0]), toByte.var);
	varOrValue1->setVarOrValue(B2(toByte.banks[0]), toByte.value1);
	varOrValue2->setVarOrValue(B2(toByte.banks[1]), toByte.value2);

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

ScriptEditorSinCosPage::ScriptEditorSinCosPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent) :
	ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent)
{
}

void ScriptEditorSinCosPage::build()
{
	operationList = new QComboBox(this);
	operationList->addItem(tr("Sinus"), OpcodeKey::SIN);
	operationList->addItem(tr("Cosinus"), OpcodeKey::COS);

	var = new VarOrValueWidget(this);
	var->setLong();
	var->setOnlyVar(true);

	varOrValue1 = new VarOrValueWidget(this);
	varOrValue1->setSignedValueType(true);
	varOrValue1->setLong();
	
	varOrValue2 = new VarOrValueWidget(this);
	varOrValue2->setSignedValueType(true);
	varOrValue2->setLong();
	
	varOrValue3 = new VarOrValueWidget(this);
	varOrValue3->setSignedValueType(true);
	varOrValue3->setLong();

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(operationList, 0, 0);
	layout->addWidget(var, 1, 0);
	layout->addWidget(varOrValue1, 2, 0);
	layout->addWidget(varOrValue2, 3, 0);
	layout->addWidget(varOrValue3, 4, 0);
	layout->setRowStretch(5, 1);
	layout->setColumnStretch(1, 1);
	layout->setContentsMargins(QMargins());
	
	connect(operationList, &QComboBox::currentIndexChanged, this, &ScriptEditorSinCosPage::changeCurrentOpcode);
	connect(var, &VarOrValueWidget::changed, this, &ScriptEditorSinCosPage::opcodeChanged);
	connect(varOrValue1, &VarOrValueWidget::changed, this, &ScriptEditorSinCosPage::opcodeChanged);
	connect(varOrValue2, &VarOrValueWidget::changed, this, &ScriptEditorSinCosPage::opcodeChanged);
	connect(varOrValue3, &VarOrValueWidget::changed, this, &ScriptEditorSinCosPage::opcodeChanged);
}

Opcode ScriptEditorSinCosPage::buildOpcode()
{
	quint8 bank1, bank2, bank3, bank4, address4;
	int value1, value2, value3;

	var->var(bank4, address4);

	if (varOrValue1->isValue()) {
		bank1 = 0;
		value1 = varOrValue1->value();
	} else {
		quint8 address1;
		varOrValue1->var(bank1, address1);
		value1 = address1;
	}

	if (varOrValue2->isValue()) {
		bank2 = 0;
		value2 = varOrValue2->value();
	} else {
		quint8 address2;
		varOrValue2->var(bank2, address2);
		value2 = address2;
	}

	if (varOrValue3->isValue()) {
		bank3 = 0;
		value3 = varOrValue3->value();
	} else {
		quint8 address3;
		varOrValue3->var(bank3, address3);
		value3 = address3;
	}

	OpcodeSIN &sin = opcode().op().opcodeSIN;
	sin.banks[0] = BANK(bank1, bank2);
	sin.banks[1] = BANK(bank3, bank4);
	sin.value1 = qint16(value1);
	sin.value2 = qint16(value2);
	sin.value3 = qint16(value3);
	sin.var = address4;

	return opcode();
}

void ScriptEditorSinCosPage::setOpcode(const Opcode &opcode)
{
	ScriptEditorView::setOpcode(opcode);

	for (QObject *o : children()) {
		o->blockSignals(true);
	}

	operationList->setCurrentIndex(operationList->findData(opcode.id()));

	const OpcodeSIN &sin = opcode.op().opcodeSIN;

	varOrValue1->setVarOrValue(B1(sin.banks[0]), sin.value1);
	varOrValue2->setVarOrValue(B2(sin.banks[0]), sin.value2);
	varOrValue3->setVarOrValue(B1(sin.banks[1]), sin.value3);
	var->setVar(B2(sin.banks[1]), sin.var);

	for (QObject *o : children()) {
		o->blockSignals(false);
	}
}

void ScriptEditorSinCosPage::changeCurrentOpcode(int index)
{
	OpcodeKey key = OpcodeKey(operationList->itemData(index).toInt());

	if (key != opcode().id()) {
		opcode().op().id = key;

		emit opcodeChanged();
	}
}
