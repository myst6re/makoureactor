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
#include "ScriptList.h"
#include "Data.h"

ScriptList::ScriptList(QWidget *parent) :
    QListWidget(parent), _grpScript(nullptr)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setContextMenuPolicy(Qt::ActionsContextMenu);
	connect(this, &ScriptList::currentItemChanged, this, &ScriptList::evidence);
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
	if (_grpScript == nullptr) {
		return nullptr;
	}

	int scriptID = selectedID();
	if (scriptID >= 0 && scriptID < SCRIPTS_SIZE) {
		return &_grpScript->script(quint8(scriptID));
	}
	return nullptr;
}

void ScriptList::clear()
{
	QListWidget::clear();
	_grpScript = nullptr;
}

void ScriptList::fill(GrpScript *grpScript)
{
	QListWidget::clear();
	_grpScript = grpScript;

	if (_grpScript == nullptr) {
		return;
	}

	for (quint8 i = 0; i < SCRIPTS_SIZE; ++i) {
		Script script = _grpScript->script(i);
		QListWidgetItem *item = new QListWidgetItem(_grpScript->scriptName(i), this);
		if (script.isEmpty()) {
			item->setForeground(Data::color(Data::ColorDisabledForeground));
		} else if (script.isVoid()) {
			item->setForeground(Data::color(Data::ColorGreyForeground));
		}
	}
}

void ScriptList::localeRefresh()
{
	if (_grpScript == nullptr) {
		return;
	}

	const QPalette &pal = palette();

	for (quint8 i = 0; i < SCRIPTS_SIZE; ++i) {
		Script script = _grpScript->script(i);
		QListWidgetItem *itm = item(i);
		itm->setText(_grpScript->scriptName(i));
		if (script.isEmpty()) {
			itm->setForeground(Data::color(Data::ColorDisabledForeground));
		} else if (script.isVoid()) {
			itm->setForeground(Data::color(Data::ColorGreyForeground));
		} else {
			itm->setForeground(pal.brush(QPalette::WindowText));
		}
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
