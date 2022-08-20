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
#include "Listwidget.h"

ListWidget::ListWidget(QWidget *parent) :
	QWidget(parent)
{
	setContextMenuPolicy(Qt::ActionsContextMenu);
	setFixedWidth(120);

	_toolBar = new QToolBar(this);
	_toolBar->setIconSize(QSize(16, 16));

	_listWidget = new QListWidget(this);
	_listWidget->setUniformItemSizes(true);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(_toolBar);
	layout->addWidget(_listWidget);
	layout->setContentsMargins(QMargins());
}

void ListWidget::addSeparator(bool invisible)
{
	if (!invisible) {
		_toolBar->addSeparator();
	}
	QAction *action = new QAction(_listWidget);
	action->setSeparator(true);
	insertAction(nullptr, action);
}

QAction *ListWidget::addAction(ActionType type, const QString &overrideText, bool visible)
{
	QAction *action = new QAction(this);
	QString text;

	switch (type) {
	case Add:
		action->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
		action->setShortcut(QKeySequence("Ctrl++"));
		text = tr("Add");
		connect(action, &QAction::triggered, this, &ListWidget::addTriggered);
		break;
	case Remove:
		action->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		action->setShortcut(QKeySequence::Delete);
		text = tr("Remove");
		connect(action, &QAction::triggered, this, &ListWidget::removeTriggered);
		break;
	case Up:
		action->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
		action->setShortcut(QKeySequence("Shift+Up"));
		text = tr("Up");
		connect(action, &QAction::triggered, this, &ListWidget::upTriggered);
		break;
	case Down:
		action->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
		action->setShortcut(QKeySequence("Shift+Down"));
		text = tr("Down");
		connect(action, &QAction::triggered, this, &ListWidget::downTriggered);
		break;
	case Cut:
		action->setIcon(QIcon::fromTheme(QStringLiteral("edit-cut")));
		action->setShortcut(QKeySequence::Cut);
		text = tr("Cut");
		connect(action, &QAction::triggered, this, &ListWidget::cutTriggered);
		break;
	case Copy:
		action->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
		action->setShortcut(QKeySequence::Copy);
		text = tr("Copy");
		connect(action, &QAction::triggered, this, &ListWidget::copyTriggered);
		break;
	case Paste:
		action->setIcon(QIcon::fromTheme(QStringLiteral("edit-paste")));
		action->setShortcut(QKeySequence::Paste);
		text = tr("Paste");
		connect(action, &QAction::triggered, this, &ListWidget::pasteTriggered);
		break;
	}

	if(!overrideText.isEmpty()) {
		text = overrideText;
	}

	action->setText(text);
	action->setStatusTip(text);

	if (visible) {
		_toolBar->addAction(action);
	}

	insertAction(nullptr, action);

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
