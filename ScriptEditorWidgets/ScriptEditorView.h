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
#include "VarOrValueWidget.h"
#include "Field.h"

class ScriptEditorView : public QWidget
{
	Q_OBJECT

public:
	ScriptEditorView(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent=0);
	virtual ~ScriptEditorView();

	virtual Opcode *opcode();
	virtual void setOpcode(Opcode *opcode);
	bool isValid() const;
signals:
	void opcodeChanged();
	
protected:
	Field *_field;
	GrpScript *_grpScript;
	Script *_script;
	Opcode *_opcode;
	int _opcodeID;
	bool _valid;
};

class ScriptEditorReturnToPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorReturnToPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	QComboBox *scriptList;
	QSpinBox *priority;
};

class ScriptEditorExecPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorExecPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private slots:
	void updateScriptList(int groupID);
private:
	QComboBox *groupList, *scriptList;
	QSpinBox *priority;
};

class ScriptEditorExecCharPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorExecCharPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	QComboBox *scriptList;
	QSpinBox *partyID, *priority;
};

class ScriptEditorLabelPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorLabelPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private slots:
	void showWarning();
private:
	QDoubleSpinBox *label;
	QLabel *warningLabel;
	QList<quint32> labels;
};

class ScriptEditorJumpPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorJumpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	QComboBox *label/*, *range*/;
};

class ScriptEditorIfPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorIfPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	VarOrValueWidget *varOrValue1, *varOrValue2;
	QComboBox *operatorList, *label;
};

class ScriptEditorIfKeyPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorIfKeyPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	QList<QCheckBox *> keys;
	QComboBox *label;
};

class ScriptEditorIfQPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorIfQPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	QComboBox *charList, *label;
};

class ScriptEditorWaitPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWaitPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	QSpinBox *frames;
};

class ScriptEditorBinaryOpPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorBinaryOpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private slots:
	void updateValueRange();
	void changeCurrentOpcode(int);
private:
	void convertOpcode(Opcode::Keys key);
	VarOrValueWidget *var, *varOrValue;
	QComboBox *operationList;
	QRadioButton *type1, *type2;
};

class ScriptEditorUnaryOpPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorUnaryOpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private slots:
	void updateValueRange();
	void changeCurrentOpcode(int);
private:
	void convertOpcode(Opcode::Keys key);
	VarOrValueWidget *var;
	QComboBox *operationList;
	QRadioButton *type1, *type2;
};

class ScriptEditorBitOpPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorBitOpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private slots:
	void changeCurrentOpcode(int);
private:
	void convertOpcode(Opcode::Keys key);
	VarOrValueWidget *var, *position;
	QComboBox *operationList;
};

#endif // DEF_SCRIPTEDITORVIEW
