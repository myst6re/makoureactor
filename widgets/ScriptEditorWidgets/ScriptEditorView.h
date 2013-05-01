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
#ifndef DEF_SCRIPTEDITORVIEW
#define DEF_SCRIPTEDITORVIEW

#include <QtGui>
#include "../VarOrValueWidget.h"
#include "core/field/Field.h"

class ScriptEditorView : public QWidget
{
	Q_OBJECT

public:
	ScriptEditorView(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent=0);
	virtual ~ScriptEditorView();

	virtual Opcode *opcode()=0;
	virtual void setOpcode(Opcode *opcode);
	bool isValid() const;
	virtual void clear();
signals:
	void opcodeChanged();
	
protected:
	virtual void build()=0;
	virtual void showEvent(QShowEvent *e);
	inline Opcode *opcodePtr() const {
		return _opcode;
	}
	Field *field() const;
	GrpScript *grpScript() const;
	Script *script() const;
	int opcodeID() const;
	void setValid(bool valid);
private:
	bool _builded;
	Field *_field;
	GrpScript *_grpScript;
	Script *_script;
	Opcode *_opcode;
	int _opcodeID;
	bool _valid;
};

#endif // DEF_SCRIPTEDITORVIEW
