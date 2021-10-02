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

class ListWidget : public QWidget
{
public:
    enum ActionType {
		Add, Rem, Up, Down, Cut, Copy, Paste
    };

	explicit ListWidget(QWidget *parent = nullptr);
	void addSeparator(bool invisible = false);
    QAction *addAction(ActionType type, const QString &text,
	                   const QObject *receiver, const char *member, bool invisible = false);

    QToolBar *toolBar() const;
    QListWidget *listWidget() const;
private:
    QToolBar *_toolBar;
    QListWidget *_listWidget;
};
