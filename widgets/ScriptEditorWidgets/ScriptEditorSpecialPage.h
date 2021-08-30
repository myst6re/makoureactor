/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2020 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef SCRIPTEDITORSPECIAL_H
#define SCRIPTEDITORSPECIAL_H

#include <QtWidgets>
#include "ScriptEditorView.h"

class ScriptEditorDLPBSavemap : public ScriptEditorView
{
	Q_OBJECT
public:
	ScriptEditorDLPBSavemap(Field *field, GrpScript *grpScript, Script *script,
	                        int opcodeID, QWidget *parent = nullptr);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	void build();
	QSpinBox *_from, *_to;
	QDoubleSpinBox *_absValue;
	QComboBox *_flag;
	QCheckBox *_fromIsPointer, *_toIsPointer;
};

class ScriptEditorDLPBWriteToMemory : public ScriptEditorView
{
	Q_OBJECT
public:
	ScriptEditorDLPBWriteToMemory(Field *field, GrpScript *grpScript, Script *script,
	                              int opcodeID, QWidget *parent = nullptr);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	void build();
	QDoubleSpinBox *_address;
	QLineEdit *_bytes;
};

#endif // SCRIPTEDITORSPECIAL_H
