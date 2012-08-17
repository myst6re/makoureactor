/****************************************************************************
 ** Deling Final Fantasy VIII Field Editor
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
#include "Listwidget.h"

ListWidget::ListWidget(QWidget *parent) :
	QWidget(parent)
{
	setContextMenuPolicy(Qt::ActionsContextMenu);
	setFixedWidth(120);

	_toolBar = new QToolBar(this);
	_toolBar->setIconSize(QSize(16, 16));

	QFont font;
	font.setPointSize(8);

	_listWidget = new QListWidget(this);
	_listWidget->setUniformItemSizes(true);
	_listWidget->setFont(font);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(_toolBar);
	layout->addWidget(_listWidget);
	layout->setContentsMargins(QMargins());
}

QAction *ListWidget::addAction(ActionType type, const QString &text,
							   const QObject *receiver, const char *member)
{
	QIcon icon;
	QKeySequence shortcut;
	QAction *action;

	switch(type) {
	case Add:
		icon = QIcon(":images/plus.png");
		shortcut = QKeySequence("Ctrl++");
		break;
	case Invisible:
		action = new QAction(text, this);
		action->setStatusTip(text);
		insertAction(0, action);
		connect(action, SIGNAL(triggered()), receiver, member);
		return action;
	case Rem:
		icon = QIcon(":images/minus.png");
		shortcut = QKeySequence::Delete;
		break;
	case Up:
		icon = QIcon(":images/up.png");
		shortcut = QKeySequence("Shift+Up");
		break;
	case Down:
		icon = QIcon(":images/down.png");
		shortcut = QKeySequence("Shift+Down");
		break;
	}

	action = _toolBar->addAction(icon, text, receiver, member);
	action->setShortcut(shortcut);
	action->setStatusTip(text);
	insertAction(0, action);

	return action;
}

QToolBar *ListWidget::toolBar() const
{
	return _toolBar;
}

QListWidget *ListWidget::listWidget() const
{
	return _listWidget;
}
