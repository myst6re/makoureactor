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

class ListWidget : public QWidget
{
	Q_OBJECT
public:
	enum ActionType {
		Add, Remove, Up, Down, Cut, Copy, Paste
	};
	
	explicit ListWidget(QWidget *parent = nullptr);
	void addSeparator(bool invisible = false);
	QAction *addAction(ActionType type, const QString &overrideText = QString(), bool visible = true);
	
	QToolBar *toolBar() const;
	QListWidget *listWidget() const;
signals:
	void addTriggered();
	void removeTriggered();
	void cutTriggered();
	void copyTriggered();
	void pasteTriggered();
	void upTriggered();
	void downTriggered();
private:
	QToolBar *_toolBar;
	QListWidget *_listWidget;
};
