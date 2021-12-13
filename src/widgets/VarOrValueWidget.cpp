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
#include "VarOrValueWidget.h"

VarOrValueWidget::VarOrValueWidget(QWidget *parent) :
	QWidget(parent), _size(Short | Long), _signedValueType(true), _onlyVar(false)
{
	_typeSelect = new QComboBox(this);
	updateValueType();

	_value = new QSpinBox(this);
	updateValueRange();

	_bank = new QComboBox(this);
	_bank->addItem(QString(), Bank1And2);
	_bank->addItem(QString(), Bank3And4);
	_bank->addItem(QString(), Bank5And6);
	_bank->addItem(QString(), Bank11And12);
	_bank->addItem(QString(), Bank13And14);
	_bank->addItem(QString(), Bank15And7);
	updateBankList();

	_adress = new QSpinBox(this);
	_adress->setRange(0, 255);

	QWidget *bankAndAdress = new QWidget(this);
	QHBoxLayout *bankAndAdressLayout = new QHBoxLayout(bankAndAdress);
	bankAndAdressLayout->addWidget(_bank);
	bankAndAdressLayout->addWidget(_adress);
	bankAndAdressLayout->setContentsMargins(QMargins());

	_varOrValuelayout = new QStackedLayout();
	_varOrValuelayout->addWidget(_value);
	_varOrValuelayout->addWidget(bankAndAdress);
	_varOrValuelayout->setContentsMargins(QMargins());

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(_typeSelect);
	layout->addLayout(_varOrValuelayout);
	layout->setContentsMargins(QMargins());

	_typeSelect->setCurrentIndex(0);
	_varOrValuelayout->setCurrentIndex(0);

	connect(_typeSelect, SIGNAL(currentIndexChanged(int)), SLOT(updateVarOrValueLayout()));
	connect(_typeSelect, SIGNAL(currentIndexChanged(int)), SLOT(updateBankList()));
	connect(_typeSelect, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()));
	connect(_bank, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()));
	connect(_value, SIGNAL(valueChanged(int)), SIGNAL(changed()));
	connect(_adress, SIGNAL(valueChanged(int)), SIGNAL(changed()));
}

int VarOrValueWidget::value() const
{
	return _value->value();
}

void VarOrValueWidget::setValue(int value)
{
	_value->setValue(value);

	if (!isOnlyVar()) {
		setIsValue(true);
	}
}

bool VarOrValueWidget::var(quint8 &bank, quint8 &adress) const
{
	if (isValue()) {
		return false;
	}

	const bool isShort = _typeSelect->currentData().toInt() == Var8;

	switch (Bank(_bank->currentData().toInt())) {
	case Bank1And2:
		bank = isShort ? 1 : 2;
		break;
	case Bank3And4:
		bank = isShort ? 3 : 4;
		break;
	case Bank5And6:
		bank = isShort ? 5 : 6;
		break;
	case Bank11And12:
		bank = isShort ? 11 : 12;
		break;
	case Bank13And14:
		bank = isShort ? 13 : 14;
		break;
	case Bank15And7:
		bank = isShort ? 15 : 7;
		break;
	}

	adress = quint8(_adress->value());

	return true;
}

void VarOrValueWidget::setVar(quint8 bank, quint8 adress)
{
	Bank bankData = Bank1And2;

	switch (bank) {
	case 1:case 2:
		bankData = Bank1And2;
		break;
	case 3:case 4:
		bankData = Bank3And4;
		break;
	case 5:case 6:
		bankData = Bank5And6;
		break;
	case 11:case 12:
		bankData = Bank11And12;
		break;
	case 13:case 14:
		bankData = Bank13And14;
		break;
	case 15:case 7:
		bankData = Bank15And7;
		break;
	}

	_bank->setCurrentIndex(_bank->findData(bankData));
	_adress->setValue(adress);

	if (!isOnlyVar()) {
		setIsValue(false);
	}
}

void VarOrValueWidget::setVarOrValue(quint8 bank, int valueOrAdress)
{
	if (bank != 0) {
		setVar(bank, valueOrAdress & 0xFF);
		setIsValue(false);
	} else {
		setValue(valueOrAdress);
		setIsValue(true);
	}
}

bool VarOrValueWidget::isValue() const
{
	return _typeSelect->currentData().toInt() == Value;
}

void VarOrValueWidget::setIsValue(bool isValue)
{
	_typeSelect->setCurrentIndex(isValue ? 0 : 1);
}

VarOrValueWidget::Sizes VarOrValueWidget::size() const
{
	return _size;
}

VarOrValueWidget::Size VarOrValueWidget::selectedSize() const
{
	if (_size.testFlag(Short) && _size.testFlag(Long)) {
		return _typeSelect->currentData().toInt() == Var8 ? Short : Long;
	}

	return _size.testFlag(Short) ? Short : Long;
}

void VarOrValueWidget::setSize(Sizes size)
{
	_size = size;
	updateValueRange();
	updateValueType();
	updateBankList();
}

void VarOrValueWidget::setShortAndLong()
{
	setSize(Short | Long);
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
	if (isSignedValueType()) {
		if (size().testFlag(Long)) {
			_value->setRange(-32768, 32767);
		} else {
			_value->setRange(-128, 127);
		}
	} else {
		if (size().testFlag(Long)) {
			_value->setRange(0, 65535);
		} else {
			_value->setRange(0, 255);
		}
	}
}

void VarOrValueWidget::updateValueType()
{
	int index = 0;
	if (!_onlyVar) {
		updateValueTypeAddOrSetItem(index, tr("Value"), Value);
		index += 1;
	}
	if (_size.testFlag(Short)) {
		updateValueTypeAddOrSetItem(index, tr("8-bit Var"), Var8);
		index += 1;
	}
	if (_size.testFlag(Long)) {
		updateValueTypeAddOrSetItem(index, tr("16-bit Var"), Var16);
		index += 1;
	}
	for (; index < _typeSelect->count(); ++index) {
		_typeSelect->removeItem(index);
	}
	_typeSelect->setDisabled(_typeSelect->count() <= 1);
}

void VarOrValueWidget::updateValueTypeAddOrSetItem(int index, const QString &text, const QVariant &data)
{
	if (index < _typeSelect->count()) {
		_typeSelect->setItemText(index, text);
		_typeSelect->setItemData(index, data);
	} else {
		_typeSelect->addItem(text, data);
	}
}

void VarOrValueWidget::updateVarOrValueLayout()
{
	_varOrValuelayout->setCurrentIndex(_typeSelect->currentData().toInt() == Value ? 0 : 1);
}

void VarOrValueWidget::updateBankList()
{
	if (_typeSelect->currentData().toInt() == Var8) {
		_bank->setItemText(0, QLatin1String("01"));
		_bank->setItemText(1, QLatin1String("03"));
		_bank->setItemText(2, QLatin1String("05"));
		_bank->setItemText(3, QLatin1String("11"));
		_bank->setItemText(4, QLatin1String("13"));
		_bank->setItemText(5, QLatin1String("15"));
	} else {
		_bank->setItemText(0, QLatin1String("02"));
		_bank->setItemText(1, QLatin1String("04"));
		_bank->setItemText(2, QLatin1String("06"));
		_bank->setItemText(3, QLatin1String("12"));
		_bank->setItemText(4, QLatin1String("14"));
		_bank->setItemText(5, QLatin1String("07"));
	}
}

bool VarOrValueWidget::isOnlyVar() const
{
	return _onlyVar;
}

void VarOrValueWidget::setOnlyVar(bool onlyVar)
{
	_onlyVar = onlyVar;
	updateValueType();
	setIsValue(!onlyVar);
}
