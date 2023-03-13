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
#include "widgets/HelpWidget.h"
#include "../VarOrValueWidget.h"

class ScriptEditorBinaryOpPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorBinaryOpPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode();
	void setOpcode(const Opcode &opcode);
private slots:
	void updateValueRange();
	void changeCurrentOpcode(int);
private:
	void build();
	VarOrValueWidget *var, *varOrValue;
	QComboBox *operationList;
	QRadioButton *type1, *type2;
	HelpWidget *helpWidget;
};

class ScriptEditorUnaryOpPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorUnaryOpPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private slots:
	void updateValueRange();
	void changeCurrentOpcode(int);
private:
	void build() override;
	VarOrValueWidget *var;
	QComboBox *operationList;
	QRadioButton *type1, *type2;
};

class ScriptEditorBitOpPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorBitOpPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private slots:
	void changeCurrentOpcode(int);
private:
	void build() override;
	VarOrValueWidget *var, *position;
	QComboBox *operationList;
};

class ScriptEditorVariablePage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorVariablePage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private:
	void build() override;
	VarOrValueWidget *varOrValue;
};

class ScriptEditor2BytePage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditor2BytePage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode();
	void setOpcode(const Opcode &opcode);
private:
	void build();
	VarOrValueWidget *var, *varOrValue1, *varOrValue2;
};

class ScriptEditorSinCosPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorSinCosPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode();
	void setOpcode(const Opcode &opcode);
private slots:
	void changeCurrentOpcode(int index);
private:
	void build();
	QComboBox *operationList;
	VarOrValueWidget *var, *varOrValue1, *varOrValue2, *varOrValue3;
};
