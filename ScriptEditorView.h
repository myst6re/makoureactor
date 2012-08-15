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
#ifndef DEF_SCRIPTEDITORVIEW
#define DEF_SCRIPTEDITORVIEW

#include <QtGui>
#include "GrpScript.h"

class ScriptEditorView : public QWidget
{
	Q_OBJECT

public:
	ScriptEditorView(Script *script, int opcodeID, QWidget *parent=0);
	virtual ~ScriptEditorView();

	virtual Opcode *opcode();
	virtual void setOpcode(Opcode *opcode);
signals:
	void opcodeChanged();
	
protected:
	Script *_script;
	Opcode *_opcode;
	int _opcodeID;
};

class VarOrValueWidget : public QWidget
{
	Q_OBJECT
public:
	explicit VarOrValueWidget(QWidget *parent = 0);
	int value() const;
	void setValue(int value);
	void var(quint8 &bank, quint8 &adress) const;
	void setVar(quint8 bank, quint8 adress);
	bool isValue() const;
	void setIsValue(bool isValue);
	bool isLongValueType() const;
	void setLongValueType(bool longValueType);
	bool isSignedValueType() const;
	void setSignedValueType(bool signedValueType);
	bool isOnlyVar() const;
	void setOnlyVar(bool onlyVar);
signals:
	void changed();
private:
	void updateValueRange();
	bool _longValueType, _signedValueType;
	QStackedLayout *varOrValuelayout;
	QComboBox *typeSelect;
	QSpinBox *_value, *_bank, *_adress;
};

class ScriptEditorGotoPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorGotoPage(Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	QComboBox *label/*, *range*/;
};

class ScriptEditorIfPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorIfPage(Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	VarOrValueWidget *varOrValue1, *varOrValue2;
	QComboBox *operatorList, *label;
};

#endif // DEF_SCRIPTEDITORVIEW
