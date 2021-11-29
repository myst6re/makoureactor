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
#pragma once

#include <QtWidgets>

class VarOrValueWidget : public QWidget
{
	Q_OBJECT
public:
	enum Size {
		Long = 0x1,
		Short = 0x2
	};
	Q_DECLARE_FLAGS(Sizes, Size)

	explicit VarOrValueWidget(QWidget *parent = nullptr);
	int value() const;
	void setValue(int value);
	bool var(quint8 &bank, quint8 &adress) const;
	void setVar(quint8 bank, quint8 adress);
	void setVarOrValue(quint8 bank, int valueOrAdress);
	bool isValue() const;
	void setIsValue(bool isValue);
	Sizes size() const;
	Size selectedSize() const;
	inline void setShort() {
		setSize(Short);
	}
	inline void setLong() {
		setSize(Long);
	}
	void setShortAndLong();
	bool isSignedValueType() const;
	void setSignedValueType(bool signedValueType);
	bool isOnlyVar() const;
	void setOnlyVar(bool onlyVar);
signals:
	void changed();
private slots:
	void updateVarOrValueLayout();
	void updateBankList();
private:
	enum Type {
		Value,
		Var8,
		Var16
	};
	enum Bank {
		Bank1And2,
		Bank3And4,
		Bank5And6,
		Bank11And12,
		Bank13And14,
		Bank15And7
	};
	void setSize(Sizes size);
	void updateValueRange();
	void updateValueType();
	void updateValueTypeAddOrSetItem(int index, const QString &text, const QVariant &data);
	QStackedLayout *_varOrValuelayout;
	QComboBox *_typeSelect, *_bank;
	QSpinBox *_value, *_adress;
	Sizes _size;
	bool _signedValueType, _onlyVar;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(VarOrValueWidget::Sizes)
