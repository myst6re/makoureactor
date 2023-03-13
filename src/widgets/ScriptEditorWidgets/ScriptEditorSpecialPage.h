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

class ScriptEditorSpecialPName : public ScriptEditorView
{
	Q_OBJECT
public:
	ScriptEditorSpecialPName(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script,
	                         int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private:
	void build() override;
	VarOrValueWidget *_varOrValue;
	VarOrValueWidget *_bank;
	QSpinBox *_size;
};

class ScriptEditorDLPBSavemap : public ScriptEditorView
{
	Q_OBJECT
public:
	ScriptEditorDLPBSavemap(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script,
	                        int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private slots:
	void setAbsValueEnabled(int value);
private:
	void build() override;
	QSpinBox *_from, *_to;
	QDoubleSpinBox *_absValue;
	QComboBox *_flag;
	QCheckBox *_fromIsPointer, *_toIsPointer;
};

class ScriptEditorDLPBWriteToMemory : public ScriptEditorView
{
	Q_OBJECT
public:
	ScriptEditorDLPBWriteToMemory(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script,
	                              int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private:
	void build() override;
	QSpinBox *_address;
	QLineEdit *_bytes;
};
