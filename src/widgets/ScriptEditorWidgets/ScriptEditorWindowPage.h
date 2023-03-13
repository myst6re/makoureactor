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
#include "ScriptEditorView.h"
#include "../VarOrValueWidget.h"
#include "../TextPreview.h"

class ScriptEditorWindowPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWindowPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
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
	void build() override;
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
	explicit ScriptEditorWindowModePage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private slots:
	void updatePreview();
private:
	void build() override;
	TextPreview *textPreview;
	QSpinBox *winID;
	QComboBox *winType, *winClose;
};

class ScriptEditorWindowMovePage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWindowMovePage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private:
	void build() override;
	QSpinBox *winID, *x, *y;
};

class ScriptEditorWindowVariablePage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWindowVariablePage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private:
	void build() override;
	QSpinBox *winID, *winVar;
	VarOrValueWidget *varOrValue;
};

class ScriptEditorWindowNumDisplayPage : public ScriptEditorView
{
	Q_OBJECT
public:
	explicit ScriptEditorWindowNumDisplayPage(const Section1File *scriptsAndTexts, const GrpScript &grpScript, const Script &script, int opcodeID, QWidget *parent = nullptr);
	Opcode buildOpcode() override;
	void setOpcode(const Opcode &opcode) override;
private slots:
	void updatePreview();
	void updatePreviewTextList();
	void updatePreviewWindowList();
private:
	void build() override;
	TextPreview *textPreview;
	QSpinBox *winID, *relativeX, *relativeY;
	QComboBox *displayMode;
	QComboBox *previewText, *previewWindow;
};
