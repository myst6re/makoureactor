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
#include "ScriptEditorView.h"

ScriptEditorView::ScriptEditorView(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent)
	: QWidget(parent), _builded(false), _field(field), _grpScript(grpScript), _script(script),
	  _opcode(0), _opcodeID(opcodeID), _valid(true)
{
}

ScriptEditorView::~ScriptEditorView()
{
}

void ScriptEditorView::showEvent(QShowEvent *e)
{
	if (!_builded) {
		build();
		_builded = true;
	}

	QWidget::showEvent(e);
}

void ScriptEditorView::clear()
{
}

void ScriptEditorView::setOpcode(Opcode *opcode)
{
	if (!_builded) {
		build();
		_builded = true;
	}
	_opcode = opcode;
}

bool ScriptEditorView::isValid() const
{
	return _valid;
}

void ScriptEditorView::setValid(bool valid)
{
	_valid = valid;
}

Field *ScriptEditorView::field() const
{
	return _field;
}

GrpScript *ScriptEditorView::grpScript() const
{
	return _grpScript;
}

Script *ScriptEditorView::script() const
{
	return _script;
}

int ScriptEditorView::opcodeID() const
{
	return _opcodeID;
}
