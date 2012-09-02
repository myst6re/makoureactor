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

ScriptList::ScriptList(QWidget *parent)
	: QListWidget(parent)
{
	setFixedWidth(88);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setContextMenuPolicy(Qt::ActionsContextMenu);
}

/* ScriptList::~ScriptList()
{
} */

Script *ScriptList::currentScript()
{
	int scriptID = selectedID();
	if(scriptID != -1)
		return grpScript->getScript(scriptID);
	return NULL;
}

void ScriptList::fill(GrpScript *_grpScript)
{
	if(_grpScript != NULL)	grpScript = _grpScript;
	
	clear();
	int i=0;
	
	foreach(Script *script, grpScript->getScripts())
	{
		QListWidgetItem *item = new QListWidgetItem(grpScript->getScriptName(i), this);
		if(script->isEmpty())			item->setForeground(QColor(0xCC,0xCC,0xCC));
		else if(script->isVoid())		item->setForeground(QColor(0x66,0x66,0x66));
		++i;
	}
}

void ScriptList::localeRefresh()
{
	int i=0;
	foreach(Script *script, grpScript->getScripts())
	{
		QListWidgetItem *itm = item(i);
		itm->setText(grpScript->getScriptName(i));
		if(script->isEmpty())			itm->setForeground(QColor(0xCC,0xCC,0xCC));
		else if(script->isVoid())		itm->setForeground(QColor(0x66,0x66,0x66));
		else							itm->setForeground(QBrush());
		++i;
	}
}

int ScriptList::selectedID()
{
	if(currentItem()==NULL)	return -1;
	return row(currentItem());
}

void ScriptList::scroll(int id, bool focus)
{
	setCurrentItem(item(id));
	scrollToItem(item(id), QAbstractItemView::PositionAtTop);
	if(focus)	setFocus();
}
