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
#include "ScriptEditorView.h"
#include "../HelpWidget.h"

class ScriptEditorWithPriorityPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWithPriorityPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr);
protected:
	void build() override;
	QSpinBox *priority;
	HelpWidget *helpWidget;
};

class ScriptEditorReturnToPage : public ScriptEditorWithPriorityPage
{
	Q_OBJECT
public:
	explicit ScriptEditorReturnToPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr);
	Opcode *opcode() override;
	void setOpcode(Opcode *opcode) override;
private:
	void build() override;
	QComboBox *scriptList;
};

class ScriptEditorExecPage : public ScriptEditorWithPriorityPage
{
	Q_OBJECT
public:
	explicit ScriptEditorExecPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr);
	Opcode *opcode() override;
	void setOpcode(Opcode *opcode) override;
private slots:
	void updateScriptList(int groupID);
	void changeCurrentOpcode(int index);
private:
	void build() override;
	Opcode *convertOpcode(Opcode::Keys key);
	QComboBox *groupList, *scriptList;
	QComboBox *execType;
};

class ScriptEditorExecCharPage : public ScriptEditorWithPriorityPage
{
	Q_OBJECT
public:
	explicit ScriptEditorExecCharPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr);
	Opcode *opcode() override;
	void setOpcode(Opcode *opcode) override;
private slots:
	void changeCurrentOpcode(int index);
private:
	void build() override;
	Opcode *convertOpcode(Opcode::Keys key);
	QComboBox *scriptList;
	QSpinBox *partyID;
	QComboBox *execType;
};

class ScriptEditorLabelPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorLabelPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr);
	Opcode *opcode() override;
	void setOpcode(Opcode *opcode) override;
private:
	void build() override;
	QDoubleSpinBox *label;
};

class ScriptEditorJumpPageInterface : public ScriptEditorView
{
	Q_OBJECT
public:
	ScriptEditorJumpPageInterface(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr) :
		ScriptEditorView(field, grpScript, script, opcodeID, parent), label(nullptr) {}
	virtual bool needsLabel() const=0;
protected:
	void fillLabelList(bool jumpBack=false);
	QComboBox *label;
};

class ScriptEditorJumpPage : public ScriptEditorJumpPageInterface
{
	Q_OBJECT
public:
	explicit ScriptEditorJumpPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr);
	void clear() override;
	Opcode *opcode() override;
	void setOpcode(Opcode *opcode) override;
	bool needsLabel() const override;
private:
	void build() override;
	Opcode *convertOpcode(Opcode::Keys key);
	QComboBox *range;
	bool addJump;
};

class ScriptEditorJumpNanakiPage : public ScriptEditorJumpPageInterface
{
	Q_OBJECT
public:
	explicit ScriptEditorJumpNanakiPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr);
	Opcode *opcode() override;
	void setOpcode(Opcode *opcode) override;
	bool needsLabel() const override;
private:
	void build() override;
};

class ScriptEditorIfPage : public ScriptEditorJumpPageInterface
{
	Q_OBJECT
public:
	explicit ScriptEditorIfPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr);
	void clear() override;
	Opcode *opcode() override;
	void setOpcode(Opcode *opcode) override;
	bool needsLabel() const override;
private slots:
	void changeTestRange();
private:
	void build() override;
	Opcode *convertOpcode(Opcode::Keys key);
	VarOrValueWidget *varOrValue1, *varOrValue2;
	QComboBox *operatorList, *rangeTest, *rangeJump;
	bool addJump;
};

class ScriptEditorIfKeyPage : public ScriptEditorJumpPageInterface
{
	Q_OBJECT
public:
	explicit ScriptEditorIfKeyPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr);
	Opcode *opcode() override;
	void setOpcode(Opcode *opcode) override;
	bool needsLabel() const override;
private:
	void build() override;
	Opcode *convertOpcode(Opcode::Keys key);
	QList<QCheckBox *> keys;
	QComboBox *typeList;
};

class ScriptEditorIfQPage : public ScriptEditorJumpPageInterface
{
	Q_OBJECT
public:
	explicit ScriptEditorIfQPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr);
	Opcode *opcode() override;
	void setOpcode(Opcode *opcode) override;
	bool needsLabel() const override;
private:
	void build() override;
	QComboBox *charList;
};

class ScriptEditorWaitPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWaitPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = nullptr);
	Opcode *opcode() override;
	void setOpcode(Opcode *opcode) override;
private:
	void build() override;
	QSpinBox *frames;
};
