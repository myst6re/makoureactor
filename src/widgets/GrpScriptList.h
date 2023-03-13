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
#include "widgets/HelpWidget.h"
#include "core/field/GrpScript.h"

class Section1File;

class GrpScriptList : public QTreeWidget
{
    Q_OBJECT
public:
	explicit GrpScriptList(QWidget *parent = nullptr);

	GrpScript *currentGrpScript();
	int selectedID();

	inline QToolBar *toolBar() const {
		return _toolBar;
	}
	inline HelpWidget *helpWidget() const {
		return _helpWidget;
	}
	void clearCopiedGroups();
	void setEnabled(bool enabled);

	void clear();
	void fill(Section1File *scripts);
	void fill();
	void localeRefresh();
	void scroll(int, bool focus=true);
	void enableActions(bool enabled);

private slots:
	void evidence(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	inline void rename() {
		rename(currentItem(), 1);
	}
	void rename(QTreeWidgetItem *item, int column);
	void renameOK(QTreeWidgetItem *item, int column);
	void add();
	void del(bool totalDel=true);
	void cut();
	void copy();
	void paste();
	inline void up() {
		move(false);
	}
	inline void down() {
		move(true);
	}
	void upDownEnabled();

signals:
	void changed();

private:
	enum Actions {
		RenameAction = 0,
		AddAction = 1,
		DelAction = 2,
		CutAction = 4,
		CopyAction = 5,
		PasteAction = 6,
		UpAction = 8,
		DownAction = 9
	};

	void updateHelpWidget();
	void move(bool direction);
	QTreeWidgetItem *findItem(int id);
	QList<int> selectedIDs();

	QToolBar *_toolBar;
	HelpWidget *_helpWidget;

	Section1File *_scripts;
	QList<GrpScript> _grpScriptCopied;
};
