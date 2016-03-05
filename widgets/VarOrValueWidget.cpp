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
#include "VarOrValueWidget.h"

VarOrValueWidget::VarOrValueWidget(QWidget *parent) :
	QWidget(parent), _longValueType(true), _signedValueType(true)
{
	typeSelect = new QComboBox(this);
	typeSelect->addItem(tr("Value"));
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
	typeSelect->setDisabled(onlyVar);
	setIsValue(!onlyVar);
}
