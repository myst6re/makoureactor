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
#include "ScriptEditorView.h"

ScriptEditorView::ScriptEditorView(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent)
	: QWidget(parent), _scriptsAndTexts(scriptsAndTexts), _grpScript(grpScript), _script(script),
	  _opcodeID(opcodeID), _builded(false), _valid(true)
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

void ScriptEditorView::setOpcode(const Opcode &opcode)
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

const Section1File *ScriptEditorView::scriptsAndTexts() const
{
	return _scriptsAndTexts;
}

const GrpScript &ScriptEditorView::grpScript() const
{
	return _grpScript;
}

const Script &ScriptEditorView::script() const
{
	return _script;
}

int ScriptEditorView::opcodeID() const
{
	return _opcodeID;
}
