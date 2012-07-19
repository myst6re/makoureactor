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
#ifndef DEF_COMMANDELIST
#define DEF_COMMANDELIST

#define HIST_ADD	0
#define HIST_REM	1
#define HIST_MOD	2
#define HIST_UPW	3
#define HIST_DOW	4

#include <QtGui>
#include "Field.h"
#include "Commande.h"
#include "ScriptEditor.h"

typedef struct{
	int type;
	int fieldID;
	int groupID;
	int scriptID;
	QList<int> commandeIDs;
	QList<QByteArray> data;
} Historic;

class CommandeList : public QTreeWidget
{
    Q_OBJECT
public:
	CommandeList(QWidget *parent=0);
	virtual ~CommandeList();

	int selectedID();
	int selectedOpcode();
	void setIsInit(bool);
	void saveExpandedItems();

	QToolBar *toolBar();
	void setEnabled(bool enabled);
	void clearCopiedCommandes();
	void fill(Script *script=NULL);
	void scroll(int, bool focus=true);
	void enableActions(bool);

private slots:
	void edit();
	void edit(QTreeWidgetItem *, int);
	void add();
	void del(bool totalDel=true);
	void cut();
	void copy();
	void paste();
	void up();
	void down();
	void itemSelected();
	void evidence(QTreeWidgetItem *current, QTreeWidgetItem *previous);

signals:
	void changed();
	void historicChanged(const Historic &);

private:
	void upDownEnabled();
	bool hasCut;
	bool isInit;
	void scriptEditor(bool modify);
	void move(bool direction);
	QTreeWidgetItem *findItem(int id);
	QList<int> selectedIDs();

	void emitHist(int type, int commandeID=0, const QByteArray &data=QByteArray());
	void emitHist(int type, const QList<int> &commandeIDs, const QList<QByteArray> &data);

	QToolBar *_toolBar;

	Script *script;
	QList<Commande *> commandeCopied;

	QBrush previousBG;
};

#endif
