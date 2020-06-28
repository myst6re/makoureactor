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
#include "ScriptList.h"

ScriptList::ScriptList(QWidget *parent) :
    QListWidget(parent), grpScript(0)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setContextMenuPolicy(Qt::ActionsContextMenu);
	connect(this, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
	        SLOT(evidence(QListWidgetItem*,QListWidgetItem*)));
}

void ScriptList::evidence(QListWidgetItem *current, QListWidgetItem *previous)
{
	if(current) {
		current->setBackground(QColor(196,196,255));
	}

	if(previous) {
		previous->setBackground(QBrush());
	}
}

Script *ScriptList::currentScript()
{
	if(!grpScript) {
		return NULL;
	}

	int scriptID = selectedID();
	if(scriptID != -1)
		return grpScript->script(scriptID);
	return NULL;
}

void ScriptList::fill(GrpScript *_grpScript)
{
	clear();

	if(_grpScript) {
		grpScript = _grpScript;
	}

	int i=0;

	foreach(Script *script, grpScript->scripts()) {
		QListWidgetItem *item = new QListWidgetItem(grpScript->scriptName(i), this);
		if(script->isEmpty()) {
			item->setForeground(QColor(0xCC,0xCC,0xCC));
		} else if(script->isVoid()) {
			item->setForeground(QColor(0x66,0x66,0x66));
		}
		++i;
	}
}

void ScriptList::localeRefresh()
{
	if(!grpScript) {
		return;
	}

	int i = 0;
	foreach(Script *script, grpScript->scripts()) {
		QListWidgetItem *itm = item(i);
		itm->setText(grpScript->scriptName(i));
		if(script->isEmpty()) {
			itm->setForeground(QColor(0xCC,0xCC,0xCC));
		} else if(script->isVoid()) {
			itm->setForeground(QColor(0x66,0x66,0x66));
		} else {
			itm->setForeground(QBrush());
		}
		++i;
	}
}

int ScriptList::selectedID()
{
	QListWidgetItem *itm = currentItem();
	if(!itm) {
		return -1;
	}
	return row(itm);
}

void ScriptList::scroll(int id, bool focus)
{
	if (selectedID() != id) {
		QListWidgetItem *itm = item(id);
		setCurrentItem(itm);
		scrollToItem(itm, QAbstractItemView::PositionAtTop);
	}
	if(focus) {
		setFocus();
	}
}
