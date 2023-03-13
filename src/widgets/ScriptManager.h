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
#include "GrpScriptList.h"
#include "ScriptList.h"
#include "OpcodeList.h"

class ScriptManager : public QWidget
{
	Q_OBJECT
public:
	explicit ScriptManager(QWidget *parent = nullptr);
	void saveConfig();
	void removeCopiedReferences();
	void clear();
	inline GrpScriptList *groupScriptList() const {
		return _groupScriptList;
	}
	inline ScriptList *scriptList() const {
		return _scriptList;
	}
	inline OpcodeList *opcodeList() const {
		return _opcodeList;
	}
	inline int currentGrpScriptId() const {
		return _groupScriptList->selectedID();
	}
	inline int currentScriptId() const {
		return _scriptList->selectedID();
	}
	inline int currentOpcodeId() const {
		return _opcodeList->selectedID();
	}
	inline int selectedOpcode() const {
		return _opcodeList->selectedOpcode();
	}
signals:
	void changed();
	void editText(int textID);
	void groupScriptCurrentChanged(int groupID);
	void gotoField(int fieldID);
	void searchOpcode(int opcodeID);
public slots:
	void refreshOpcode(int groupID, int scriptID, int opcodeID);
	void fill(Field *field);
	void fillScripts();
	void fillOpcodes();
	void compile();
	void refresh();
	void gotoScript(int grpScriptID, int scriptID);
	void gotoOpcode(int grpScriptID, int scriptID, int opcodeID);
private:
	GrpScriptList *_groupScriptList;
	ScriptList *_scriptList;
	OpcodeList *_opcodeList;
	QLabel *_compileScriptIcon, *_compileScriptLabel;
	Field *_field;
};
