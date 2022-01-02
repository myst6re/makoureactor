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

class Window;
class FieldArchive;

class SearchAll : public QDialog
{
	Q_OBJECT
public:
	explicit SearchAll(Window *parent);
	void setScriptSearch();
	void setTextSearch();
	void setFieldArchive(FieldArchive *fieldArchive);
	void clear();
public slots:
	void addResultOpcode(int mapID, int grpScriptID, int scriptID, int opcodeID);
	void addResultText(int mapID, int textID, int index, int size);
private slots:
	void gotoResult(QTreeWidgetItem *item);
	void copySelected() const;
private:
	enum Mode {
		ScriptSearch, TextSearch
	};

	inline Window *mainWindow() const {
		return reinterpret_cast<Window *>(parentWidget());
	}
	void addResult(int fieldID, QTreeWidgetItem *item);
	QTreeWidgetItem *createItemField(int mapID) const;
	QTreeWidgetItem *createItemOpcode(int mapID, int grpScriptID, int scriptID, int opcodeID) const;
	QTreeWidgetItem *createItemText(int mapID, int textID, int index, int size) const;
	QTreeWidget *_resultList;
	FieldArchive *_fieldArchive;
	QMap<int, QTreeWidgetItem *> itemByMapID;
	Mode _searchMode;
};
