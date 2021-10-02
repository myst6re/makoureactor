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
#include "ScriptEditorView.h"
#include "../TextPreview.h"

class ScriptEditorWindowPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWindowPage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private slots:
	void updatePreview();
	void updateText(int textID);
	void setPositionWindow(const QPoint &point);
	void resizeWindow();
	inline void alignHorizontally() {
		align(Qt::AlignHCenter);
	}
	inline void alignVertically() {
		align(Qt::AlignVCenter);
	}
private:
	void build();
	void align(Qt::Alignment alignment);
	QLabel *xLabel, *yLabel, *wLabel, *hLabel;
	TextPreview *textPreview;
	QSpinBox *winID, *x, *y, *w, *h;
	QComboBox *previewText;
	QPushButton *hAlign, *vAlign, *autoSize;
};

class ScriptEditorWindowModePage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWindowModePage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private slots:
	void updatePreview();
private:
	void build();
	TextPreview *textPreview;
	QSpinBox *winID;
	QComboBox *winType, *winClose;
};

class ScriptEditorWindowMovePage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWindowMovePage(Field *field, GrpScript *grpScript, Script *script, int opcodeID, QWidget *parent = 0);
	Opcode *opcode();
	void setOpcode(Opcode *opcode);
private:
	void build();
	QSpinBox *winID, *x, *y;
};
