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
#include "core/field/Field.h"
#include "core/field/GrpScript.h"
#include "core/field/Script.h"
#include "core/field/Opcode.h"

class OpcodeList : public QTreeWidget
{
    Q_OBJECT
public:
	enum HistoricType {
		Add, Remove, Modify, ModifyAndAddLabel, Up, Down
	};

	struct Historic {
		HistoricType type;
		QList<int> opcodeIDs;
		QList<Opcode> data;
	};

	explicit OpcodeList(QWidget *parent = nullptr);
	virtual ~OpcodeList() override {}

	int selectedID();
	int selectedOpcode();
	void setIsInit(bool);
	void saveExpandedItems();

	inline QToolBar *toolBar() {
		return _toolBar;
	}
	void clear();
	void setEnabled(bool enabled);
	void fill(Field *field, const GrpScript *grpScript, Script *script);
	void scroll(int, bool focus = true);
	void enableActions(bool);
	void setErrorLine(int opcodeID);
	inline bool isTreeEnabled() const {
		return _treeEnabled;
	}

	bool itemIsExpanded(int opcodeID) const;
public slots:
	void refreshOpcode(int opcodeID);
private slots:
	void adjustPasteAction();
	void add();
	void scriptEditor(bool modify = true);
	void del(bool totalDel = true);
	void cut();
	void copy();
	void copyText();
	void paste();
	inline void up() {
		move(Script::Up);
	}
	inline void down() {
		move(Script::Down);
	}
	void itemSelected();
	void evidence(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void undo();
	void redo();
	void editText();
	void gotoLabel(QTreeWidgetItem *item = nullptr);
	void setTreeEnabled(bool enabled);
	inline void toggleTree() {
		setTreeEnabled(!_treeEnabled);
	}
	void searchOpcode();
signals:
	void changed();
	void editText(int textID);
	void gotoScript(int groupID, int scriptID);
	void gotoField(int fieldID);
	void searchOpcode(int opcodeID);
protected:
	void mouseReleaseEvent(QMouseEvent *event) override;
private:
	void fill();
	void upDownEnabled();
	void move(Script::MoveDirection direction);
	QTreeWidgetItem *findItem(int id);
	QList<int> selectedIDs();

//	QString showHistoric();
	void changeHist(HistoricType type, int opcodeID, const Opcode &data = Opcode());
	void changeHist(HistoricType type, const QList<int> &opcodeIDs, const QList<Opcode> &data);
	void clearHist();

	static QPixmap &posNumber(int num, const QPixmap &fontPixmap, QPixmap &wordPixmap);

	QToolBar *_toolBar;
	QLabel *_help;

	Field *_field;
	const GrpScript *_grpScript;
	Script *_script;

	QBrush previousBG, previousErrorBg;

	QAction *edit_A, *add_A, *del_A;
	QAction *cut_A, *copy_A, *copyText_A, *paste_A;
	QAction *up_A, *down_A, *expand_A;
	QAction *undo_A, *redo_A, *text_A, *goto_A;
	QAction *disableTree_A, *search_A;

	QStack<Historic> hists;
	QStack<Historic> restoreHists;
//	int currentHistPos;
	int errorLine;
	bool hasCut, isInit, _treeEnabled;
};
