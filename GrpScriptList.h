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
#ifndef DEF_GRPSCRIPTLIST
#define DEF_GRPSCRIPTLIST

#include <QtGui>
#include "Field.h"
#include "GrpScript.h"

class GrpScriptList : public QTreeWidget
{
    Q_OBJECT
public:
	GrpScriptList(QWidget *parent=0);
	virtual ~GrpScriptList();

	GrpScript *currentGrpScript();
	int selectedID();

	QToolBar *toolBar();
	void clearCopiedGroups();
	void setEnabled(bool);

	void fill(Field *field=NULL);
	void localeRefresh();
	void scroll(int, bool focus=true);
	void enableActions(bool);

private slots:
	void rename();
	void rename(QTreeWidgetItem *, int);
	void renameOK(QTreeWidgetItem *, int);
	void add();
	void del(bool totalDel=true);
	void cut();
	void copy();
	void paste();
	void up();
	void down();
	void upDownEnabled();

signals:
	void changed();

private:
	bool hasCut;
	void move(bool direction);
	QTreeWidgetItem *findItem(int id);
	QList<int> selectedIDs();

	QToolBar *_toolBar;

	Field *field;
	QList<GrpScript *> grpScriptCopied;
};

#endif
