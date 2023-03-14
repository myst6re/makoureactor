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
#include "core/field/GrpScript.h"
#include "ScriptEditorWidgets/ScriptEditorView.h"

class ScriptEditor : public QDialog
{
	Q_OBJECT

public:
	ScriptEditor(Field *field, const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, bool modify, bool isInit, QWidget *parent = nullptr);

	bool needslabel() const;
	Opcode buildOpcode() const;

private slots:
	void refreshTextEdit();
	void changeCurrentOpcode(int);
	void buildList(int);
	
private:
	enum PageType {
		GenericList = 0,
		NoParameters,
		Boolean,
		OneVarOrValue,
		ReturnTo,
		Exec,
		ExecChar,
		Label,
		Jump,
		If,
		IfKey,
		IfQ,
		Wait,
		BinaryOp,
		UnaryOp,
		BitOp,
		Variable,
		SinCos,
		ToByte,
		Window,
		WindowMode,
		WindowMove,
		WindowVar,
		WindowNumDisplay,
		Movie,
		Model,
		Walkmesh,
		JumpNanaki,
		SpecialPName,
		DLPBSavemap,
		DLPBWriteToMemory
	};
	void fillEditor();
	void fillView();
	void setCurrentMenu(int id);
	ScriptEditorView *buildEditorPage(PageType id);

	static QList<OpcodeKey> crashIfInit;

	QComboBox *comboBox0, *comboBox;
	QPlainTextEdit *textEdit;
	QStackedWidget *editorLayout;
	QPushButton *ok;
	QMap<PageType, int> _typeToIndex;
	ScriptEditorView *_currentPageWidget;
	PageType _currentPageType;

	Field *field;
	const Section1File *scriptsAndTexts;
	const GrpScript &grpScript;
	const Script &script;
	Opcode opcode;
	int opcodeID;
	bool isInit, modify, change;
};
