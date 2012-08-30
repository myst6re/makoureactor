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
#ifndef DEF_SCRIPTEDITORSTRUCTPAGE
#define DEF_SCRIPTEDITORSTRUCTPAGE

#include <QtGui>
#include "ScriptEditorView.h"

class ScriptEditorReturnToPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorReturnToPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	void build();
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
	void changeCurrentOpcode(int index);
private:
	void build();
	void convertOpcode(Opcode::Keys key);
	QComboBox *groupList, *scriptList;
	QSpinBox *priority;
	QComboBox *execType;
};

class ScriptEditorExecCharPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorExecCharPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private slots:
	void changeCurrentOpcode(int index);
private:
	void build();
	void convertOpcode(Opcode::Keys key);
	QComboBox *scriptList;
	QSpinBox *partyID, *priority;
	QComboBox *execType;
};

class ScriptEditorLabelPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorLabelPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	void build();
	QLabel *label;
};

class ScriptEditorJumpPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorJumpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	void build();
	void convertOpcode(Opcode::Keys key);
	QComboBox *label, *range;
};

class ScriptEditorIfPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorIfPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private slots:
	void changeTestRange();
private:
	void build();
	void convertOpcode(Opcode::Keys key);
	VarOrValueWidget *varOrValue1, *varOrValue2;
	QComboBox *operatorList, *rangeTest, *label, *rangeJump;
};

class ScriptEditorIfKeyPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorIfKeyPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	void build();
	void convertOpcode(Opcode::Keys key);
	QList<QCheckBox *> keys;
	QComboBox *label, *typeList;
};

class ScriptEditorIfQPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorIfQPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	void build();
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
	void build();
	QSpinBox *frames;
};

#endif // DEF_SCRIPTEDITORSTRUCTPAGE
