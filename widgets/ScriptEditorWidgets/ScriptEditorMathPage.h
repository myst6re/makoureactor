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
#ifndef DEF_SCRIPTEDITORMATHPAGE
#define DEF_SCRIPTEDITORMATHPAGE

#include <QtWidgets>
#include "ScriptEditorView.h"

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
	void build();
	Opcode *convertOpcode(Opcode::Keys key);
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
	void build();
	Opcode *convertOpcode(Opcode::Keys key);
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
	void build();
	Opcode *convertOpcode(Opcode::Keys key);
	VarOrValueWidget *var, *position;
	QComboBox *operationList;
};

#endif // DEF_SCRIPTEDITORMATHPAGE
