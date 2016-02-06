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
#ifndef DEF_OPCODELIST
#define DEF_OPCODELIST

#include <QtGui>
#include "core/field/Field.h"
#include "core/field/Opcode.h"

class OpcodeList : public QTreeWidget
{
    Q_OBJECT
public:
	enum HistoricType {
		Add, Remove, Modify, ModifyAndAddLabel, Up, Down
	};

	typedef struct{
		HistoricType type;
		QList<int> opcodeIDs;
		QList<Opcode *> data;
	} Historic;

	OpcodeList(QWidget *parent=0);
	virtual ~OpcodeList();

	int selectedID();
	int selectedOpcode();
	void setIsInit(bool);
	void saveExpandedItems();

	QToolBar *toolBar();
	void clear();
	void setEnabled(bool enabled);
	void clearCopiedOpcodes();
	void fill(Field *_field=0, GrpScript *_grpScript=0, Script *_script=0);
	void scroll(int, bool focus=true);
	void enableActions(bool);
	void setErrorLine(int opcodeID);

	bool itemIsExpanded(const Opcode *opcode) const;
	void setExpandedItems(const QList<const Opcode *> &expandedItems);

private slots:
	void add();
	void scriptEditor(bool modify=true);
	void del(bool totalDel=true);
	void cut();
	void copy();
	void paste();
	void up();
	void down();
	void itemSelected();
	void evidence(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void undo();
	void redo();
	void editText();
	void gotoLabel(QTreeWidgetItem *item = NULL);
signals:
	void changed();
	void editText(int textID);
protected:
	void mouseReleaseEvent(QMouseEvent *event);
private:
	void upDownEnabled();
	void move(Script::MoveDirection direction);
	QTreeWidgetItem *findItem(int id);
	QList<int> selectedIDs();

//	QString showHistoric();
	void changeHist(HistoricType type, int opcodeID=0, Opcode *data=0);
	void changeHist(HistoricType type, const QList<int> &opcodeIDs, const QList<Opcode *> &data);
	void clearHist();

	static QPixmap &posNumber(int num, const QPixmap &fontPixmap, QPixmap &wordPixmap);

	bool hasCut, isInit;

	QToolBar *_toolBar;
	QWidget *_help;

	Field *field;
	GrpScript *grpScript;
	Script *script;
	QList<Opcode *> opcodeCopied;
	QHash<const Script *, QList<const Opcode *> > expandedItems;

	QBrush previousBG, previousErrorBg;
	int errorLine;

	QAction *edit_A, *add_A, *del_A;
	QAction *cut_A, *copy_A, *paste_A;
	QAction *up_A, *down_A, *expand_A;
	QAction *undo_A, *redo_A, *text_A, *goto_A;

	QStack<Historic> hists;
	QStack<Historic> restoreHists;
//	int currentHistPos;
};

#endif // DEF_OPCODELIST
