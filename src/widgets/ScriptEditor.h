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
#pragma once

#include <QtWidgets>
#include "core/field/GrpScript.h"
#include "ScriptEditorWidgets/ScriptEditorView.h"

class ScriptEditor : public QDialog
{
	Q_OBJECT

public:
	ScriptEditor(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, bool modify, bool isInit, QWidget *parent = nullptr);

	bool needslabel() const;
	OpcodeBox buildOpcode() const;

private slots:
	void refreshTextEdit();
	void changeCurrentOpcode(int);
	void buildList(int);
	
private:
	void fillEditor();
	void fillView();
	void setCurrentMenu(int id);

	static QList<Opcode::Keys> crashIfInit;

	QComboBox *comboBox0, *comboBox;
	QPlainTextEdit *textEdit;
	QStackedWidget *editorLayout;
	QPushButton *ok;
	ScriptEditorView *editorWidget;

	const Section1File *scriptsAndTexts;
	const Script &script;
	int opcodeID;
	OpcodeBox opcode;
	bool isInit, modify, change;
};
