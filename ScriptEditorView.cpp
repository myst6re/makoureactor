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

ScriptEditorView::ScriptEditorView(Script *script, int opcodeID, QWidget *parent)
	: QWidget(parent), _script(script), _opcode(0), _opcodeID(opcodeID)
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

VarOrValueWidget::VarOrValueWidget(QWidget *parent) :
	QWidget(parent), _longValueType(true), _signedValueType(true)
{
	typeSelect = new QComboBox(this);
	typeSelect->addItem(tr("Valeur"));
	typeSelect->addItem(tr("Variable"));

	_value = new QSpinBox(this);
	updateValueRange();

	_bank = new QSpinBox(this);
	_bank->setRange(1, 15);

	_adress = new QSpinBox(this);
	_adress->setRange(0, 255);

	QWidget *bankAndAdress = new QWidget(this);
	QHBoxLayout *bankAndAdressLayout = new QHBoxLayout(bankAndAdress);
	bankAndAdressLayout->addWidget(_bank);
	bankAndAdressLayout->addWidget(_adress);
	bankAndAdressLayout->setContentsMargins(QMargins());

	varOrValuelayout = new QStackedLayout();
	varOrValuelayout->addWidget(_value);
	varOrValuelayout->addWidget(bankAndAdress);
	varOrValuelayout->setContentsMargins(QMargins());

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(typeSelect);
	layout->addLayout(varOrValuelayout);
	layout->setContentsMargins(QMargins());

	typeSelect->setCurrentIndex(0);
	varOrValuelayout->setCurrentIndex(0);

	connect(typeSelect, SIGNAL(currentIndexChanged(int)), varOrValuelayout, SLOT(setCurrentIndex(int)));
	connect(typeSelect, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()));
	connect(_value, SIGNAL(valueChanged(int)), SIGNAL(changed()));
	connect(_bank, SIGNAL(valueChanged(int)), SIGNAL(changed()));
	connect(_adress, SIGNAL(valueChanged(int)), SIGNAL(changed()));
}

int VarOrValueWidget::value() const
{
	return _value->value();
}

void VarOrValueWidget::setValue(int value)
{
	_value->setValue(value);
}

void VarOrValueWidget::var(quint8 &bank, quint8 &adress) const
{
	bank = _bank->value();
	adress = _adress->value();
}

void VarOrValueWidget::setVar(quint8 bank, quint8 adress)
{
	_bank->setValue(bank);
	_adress->setValue(adress);
}

bool VarOrValueWidget::isValue() const
{
	return typeSelect->currentIndex() == 0;
}

void VarOrValueWidget::setIsValue(bool isValue)
{
	typeSelect->setCurrentIndex(isValue ? 0 : 1);
}

bool VarOrValueWidget::isLongValueType() const
{
	return _longValueType;
}

void VarOrValueWidget::setLongValueType(bool longValueType)
{
	_longValueType = longValueType;
	updateValueRange();
}

bool VarOrValueWidget::isSignedValueType() const
{
	return _signedValueType;
}

void VarOrValueWidget::setSignedValueType(bool signedValueType)
{
	_signedValueType = signedValueType;
	updateValueRange();
}

void VarOrValueWidget::updateValueRange()
{
	if(isSignedValueType()) {
		if(isLongValueType()) {
			_value->setRange(-32768, 32767);
		} else {
			_value->setRange(-128, 127);
		}
	} else {
		if(isLongValueType()) {
			_value->setRange(0, 65535);
		} else {
			_value->setRange(0, 255);
		}
	}
}

bool VarOrValueWidget::isOnlyVar() const
{
	return typeSelect->isHidden();
}

void VarOrValueWidget::setOnlyVar(bool onlyVar)
{
	typeSelect->setHidden(onlyVar);
	setIsValue(onlyVar);
}

ScriptEditorGotoPage::ScriptEditorGotoPage(Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(script, opcodeID, parent)
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

Opcode *ScriptEditorGotoPage::opcode()
{
	qDebug() << "ScriptEditorGotoPage::opcode";
	OpcodeJump *opcodeJump = (OpcodeJump *)_opcode;
	opcodeJump->setLabel(label->itemData(label->currentIndex()).toUInt());

	return ScriptEditorView::opcode();
}

void ScriptEditorGotoPage::setOpcode(Opcode *opcode)
{
	qDebug() << "ScriptEditorGotoPage::setOpcode";
	ScriptEditorView::setOpcode(opcode);

	label->clear();

	foreach(Opcode *opcode, _script->getOpcodes()) {
		if(opcode->isLabel()) {
			quint32 lbl = ((OpcodeLabel *)opcode)->label();
			label->addItem(tr("Label %1").arg(lbl), lbl);
		}
	}

	OpcodeJump *opcodeJump = (OpcodeJump *)opcode;
	int index = label->findData(opcodeJump->label());
	if(index < 0)		index = 0;
	label->setCurrentIndex(index);

//	if(opcode->id() == Opcode::JMPF || opcode->id() == Opcode::JMPB) {
//		range->setCurrentIndex(0);
//	} else {
//		range->setCurrentIndex(1);
//	}
}

ScriptEditorIfPage::ScriptEditorIfPage(Script *script, int opcodeID, QWidget *parent) :
	ScriptEditorView(script, opcodeID, parent)
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
	qDebug() << "ScriptEditorGotoPage::opcode";
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
	qDebug() << "ScriptEditorGotoPage::setOpcode";
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

	for(int i=_opcodeID+1 ; i<_script->size() ; ++i) {
		if(_script->getOpcode(i)->isLabel()) {
			quint32 lbl = ((OpcodeLabel *)_script->getOpcode(i))->label();
			label->addItem(tr("Label %1").arg(lbl), lbl);
		}
	}

	OpcodeJump *opcodeJump = (OpcodeJump *)opcode;
	int index = label->findData(opcodeJump->label());
	if(index < 0)		index = 0;
	label->setCurrentIndex(index);
}
