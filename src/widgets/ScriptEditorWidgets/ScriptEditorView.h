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
#include "core/field/Section1File.h"
#include "core/field/GrpScript.h"
#include "core/field/Script.h"
#include "core/field/Opcode.h"

class ScriptEditorView : public QWidget
{
	Q_OBJECT

public:
	ScriptEditorView(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	virtual ~ScriptEditorView() override;

	virtual Opcode buildOpcode()=0;
	virtual void setOpcode(const Opcode &opcode);
	bool isValid() const;
	virtual void clear();
signals:
	void opcodeChanged();
	
protected:
	virtual void build()=0;
	virtual void showEvent(QShowEvent *e) override;
	inline Opcode &opcode() {
		return _opcode;
	}
	inline const Opcode &opcode() const {
		return _opcode;
	}
	const Section1File *scriptsAndTexts() const;
	const GrpScript &grpScript() const;
	const Script &script() const;
	int opcodeID() const;
	void setValid(bool valid);
private:
	const Section1File *_scriptsAndTexts;
	const GrpScript &_grpScript;
	const Script &_script;
	Opcode _opcode;
	int _opcodeID;
	bool _builded, _valid;
};
