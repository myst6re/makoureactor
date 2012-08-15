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
#ifndef DEF_SE
#define DEF_SE

#include <QtGui>
#include "GrpScript.h"
#include "ScriptEditorGenericList.h"
//#include "ScriptEditor_dialogs.h"

class ScriptEditor : public QDialog
{
	Q_OBJECT

public:
	ScriptEditor();
	ScriptEditor(Script *script, int opcodeID, bool modify, bool isInit, QWidget *parent=0);
	virtual ~ScriptEditor();

private slots:
	void modify();
	void add();
	void refreshTextEdit();
	void changeCurrentOpcode(int);
	void buildList(int);
	
private:
	void fillEditor();

	QVBoxLayout *layout;
	QHBoxLayout *buttonLayout;
	QComboBox *comboBox0, *comboBox;
	QPlainTextEdit *textEdit;
	QStackedLayout *editorLayout;
	QPushButton *ok, *cancel;
	ScriptEditorView *editorWidget;

	Script *script;
	quint16 opcodeID;
	Opcode *opcode;
	bool isInit;
	QList<quint8> crashIfInit;

	bool change;
};

#endif
