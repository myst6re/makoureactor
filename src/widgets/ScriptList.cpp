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
#include "Data.h"

ScriptList::ScriptList(QWidget *parent) :
    QListWidget(parent), grpScript(nullptr)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setContextMenuPolicy(Qt::ActionsContextMenu);
	connect(this, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
	        SLOT(evidence(QListWidgetItem*,QListWidgetItem*)));
}

void ScriptList::evidence(QListWidgetItem *current, QListWidgetItem *previous)
{
	if (current) {
		current->setBackground(Data::color(Data::ColorEvidence));
	}

	if (previous) {
		previous->setBackground(QBrush());
	}
}

Script *ScriptList::currentScript()
{
	if (!grpScript) {
		return nullptr;
	}

	int scriptID = selectedID();
	if (scriptID != -1) {
		return grpScript->script(quint8(scriptID));
	}
	return nullptr;
}

void ScriptList::fill(GrpScript *_grpScript)
{
	clear();

	if (_grpScript) {
		grpScript = _grpScript;
	}

	quint8 i = 0;

	for (Script *script : grpScript->scripts()) {
		QListWidgetItem *item = new QListWidgetItem(grpScript->scriptName(i), this);
		if (script->isEmpty()) {
			item->setForeground(Data::color(Data::ColorDisabledForeground));
		} else if (script->isVoid()) {
			item->setForeground(Data::color(Data::ColorGreyForeground));
		}
		++i;
	}
}

void ScriptList::localeRefresh()
{
	if (!grpScript) {
		return;
	}

	const QPalette &pal = palette();

	quint8 i = 0;

	for (Script *script : grpScript->scripts()) {
		QListWidgetItem *itm = item(i);
		itm->setText(grpScript->scriptName(i));
		if (script->isEmpty()) {
			itm->setForeground(Data::color(Data::ColorDisabledForeground));
		} else if (script->isVoid()) {
			itm->setForeground(Data::color(Data::ColorGreyForeground));
		} else {
			itm->setForeground(pal.brush(QPalette::WindowText));
		}
		++i;
	}
}

int ScriptList::selectedID()
{
	QListWidgetItem *itm = currentItem();
	if (!itm) {
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
	if (focus) {
		setFocus();
	}
}
