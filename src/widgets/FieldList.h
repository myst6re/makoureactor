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
#include "core/field/FieldArchive.h"

class FieldList : public QTreeWidget
{
	Q_OBJECT
public:
	explicit FieldList(QWidget *parent = nullptr);
	int currentMapId() const;
	inline QToolBar *toolBar() {
		return _toolBar;
	}
	FieldArchive::Sorting getFieldSorting();
	inline QLineEdit *lineSearch() const {
		return _lineSearch;
	}
	void setEnabled(bool enabled);
	void enableActions(bool enabled);
public slots:
	inline void rename() {
		rename(currentItem(), 1);
	}
	void rename(QTreeWidgetItem *item, int column);
	void renameOK(QTreeWidgetItem *item, int column);
	void fill(FieldArchive *fieldArchive);
	void filterMap(const QString &name = QString());
private slots:
	void evidence(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void add();
	void del();

signals:
	void changed();
	void fieldDeleted();

private:
	static QTreeWidgetItem *createItem(Field *f, int mapID);
	void adjustWidth();

	QLineEdit *_lineSearch;
	QToolBar *_toolBar;
	
	FieldArchive *_fieldArchive;
};
