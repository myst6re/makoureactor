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
#pragma once

#include <QtWidgets>
#include "ScriptEditorView.h"
#include "../VarOrValueWidget.h"
#include "../HelpWidget.h"

class ScriptEditorWithPriorityPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWithPriorityPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
protected:
	void build() override;
	QSpinBox *priority;
	HelpWidget *helpWidget;
};

class ScriptEditorReturnToPage : public ScriptEditorWithPriorityPage
{
	Q_OBJECT
public:
	explicit ScriptEditorReturnToPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private:
	void build() override;
	QComboBox *scriptList;
};

class ScriptEditorExecPage : public ScriptEditorWithPriorityPage
{
	Q_OBJECT
public:
	explicit ScriptEditorExecPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private slots:
	void updateScriptList(int groupID);
	void changeCurrentOpcode(int index);
private:
	void build() override;
	QComboBox *groupList, *scriptList;
	QComboBox *execType;
};

class ScriptEditorExecCharPage : public ScriptEditorWithPriorityPage
{
	Q_OBJECT
public:
	explicit ScriptEditorExecCharPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private slots:
	void changeCurrentOpcode(int index);
private:
	void build() override;
	QComboBox *scriptList;
	QSpinBox *partyID;
	QComboBox *execType;
};

class ScriptEditorLabelPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorLabelPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private:
	void build() override;
	QDoubleSpinBox *label;
};

class ScriptEditorJumpPageInterface : public ScriptEditorView
{
	Q_OBJECT
public:
	ScriptEditorJumpPageInterface(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr) :
		ScriptEditorView(scriptsAndTexts, grpScript, script, opcodeID, parent), label(nullptr) {}
	virtual bool needsLabel() const=0;
protected:
	void fillLabelList(bool jumpBack=false);
	QComboBox *label;
};

class ScriptEditorJumpPage : public ScriptEditorJumpPageInterface
{
	Q_OBJECT
public:
	explicit ScriptEditorJumpPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	void clear() override;
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
	bool needsLabel() const override;
private:
	void build() override;
	bool addJump;
};

class ScriptEditorJumpNanakiPage : public ScriptEditorJumpPageInterface
{
	Q_OBJECT
public:
	explicit ScriptEditorJumpNanakiPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
	bool needsLabel() const override;
private:
	void build() override;
};

class ScriptEditorIfPage : public ScriptEditorJumpPageInterface
{
	Q_OBJECT
public:
	explicit ScriptEditorIfPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
	bool needsLabel() const override;
private slots:
	void updateOpcode();
private:
	void build() override;
	VarOrValueWidget *varOrValue1, *varOrValue2;
	QComboBox *operatorList, *rangeTest;
	bool addJump;
};

class ScriptEditorIfKeyPage : public ScriptEditorJumpPageInterface
{
	Q_OBJECT
public:
	explicit ScriptEditorIfKeyPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
	bool needsLabel() const override;
private:
	void build() override;
	QList<QCheckBox *> keys;
	QComboBox *typeList;
};

class ScriptEditorIfQPage : public ScriptEditorJumpPageInterface
{
	Q_OBJECT
public:
	explicit ScriptEditorIfQPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
	bool needsLabel() const override;
private:
	void build() override;
	QComboBox *charList;
};

class ScriptEditorWaitPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWaitPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private:
	void build() override;
	QSpinBox *frames;
};
