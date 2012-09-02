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
#include "GrpScriptList.h"

GrpScriptList::GrpScriptList(QWidget *parent)
	: QTreeWidget(parent), hasCut(false)
{
	setColumnCount(3);
	setHeaderLabels(QStringList() << tr("Id") << tr("Groupe") << tr("Type"));
	setIndentation(0);
	setItemsExpandable(false);
	setSortingEnabled(true);
	setColumnWidth(0,25);
	setColumnWidth(1,62);
	resizeColumnToContents(2);
	setContextMenuPolicy(Qt::ActionsContextMenu);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	sortByColumn(0, Qt::AscendingOrder);
	
	QAction *rename_A = new QAction(tr("Renommer groupe"), this);
	rename_A->setShortcut(QKeySequence("F2"));
	rename_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *add_A = new QAction(QIcon(":/images/plus.png"), tr("Ajouter groupe"), this);
	add_A->setShortcut(QKeySequence("Ctrl++"));
	add_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *del_A = new QAction(QIcon(":/images/minus.png"), tr("Supprimer groupe"), this);
	del_A->setShortcut(QKeySequence(Qt::Key_Delete));
	del_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	del_A->setEnabled(false);
	QAction *cut_A = new QAction(QIcon(":/images/cut.png"), tr("Couper groupe"), this);
	cut_A->setShortcut(QKeySequence("Ctrl+X"));
	cut_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	cut_A->setEnabled(false);
	QAction *copy_A = new QAction(QIcon(":/images/copy.png"), tr("Copier groupe"), this);
	copy_A->setShortcut(QKeySequence("Ctrl+C"));
	copy_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	copy_A->setEnabled(false);
	QAction *paste_A = new QAction(QIcon(":/images/paste.png"), tr("Coller groupe"), this);
	paste_A->setShortcut(QKeySequence("Ctrl+V"));
	paste_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	paste_A->setEnabled(false);
	QAction *up_A = new QAction(QIcon(":/images/up.png"), tr("Déplacer vers le haut"), this);
	up_A->setShortcut(QKeySequence("Shift+Up"));
	up_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	up_A->setEnabled(false);
	QAction *down_A = new QAction(QIcon(":/images/down.png"), tr("Déplacer vers le bas"), this);
	down_A->setShortcut(QKeySequence("Shift+Down"));
	down_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	down_A->setEnabled(false);
	
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(rename(QTreeWidgetItem *, int)));
	connect(this, SIGNAL(itemSelectionChanged()), SLOT(upDownEnabled()));
	
	connect(rename_A, SIGNAL(triggered()), SLOT(rename()));
	connect(add_A, SIGNAL(triggered()), SLOT(add()));
	connect(del_A, SIGNAL(triggered()), SLOT(del()));
	connect(cut_A, SIGNAL(triggered()), SLOT(cut()));
	connect(copy_A, SIGNAL(triggered()), SLOT(copy()));
	connect(paste_A, SIGNAL(triggered()), SLOT(paste()));
	connect(up_A, SIGNAL(triggered()), SLOT(up()));
	connect(down_A, SIGNAL(triggered()), SLOT(down()));
	
	this->addAction(rename_A);
	this->addAction(add_A);
	this->addAction(del_A);
	QAction *separator = new QAction(this);
	separator->setSeparator(true);
	this->addAction(separator);
	this->addAction(cut_A);
	this->addAction(copy_A);
	this->addAction(paste_A);
	QAction *separator2 = new QAction(this);
	separator2->setSeparator(true);
	this->addAction(separator2);
	this->addAction(up_A);
	this->addAction(down_A);
	
	_toolBar = new QToolBar(tr("Édition des &groupes"));
	_toolBar->setIconSize(QSize(14,14));
	_toolBar->addAction(add_A);
	add_A->setStatusTip(tr("Ajouter un groupe"));
	_toolBar->addAction(del_A);
	del_A->setStatusTip(tr("Supprimer un groupe"));
	_toolBar->addSeparator();
	_toolBar->addAction(up_A);
	up_A->setStatusTip(tr("Monter un groupe"));
	_toolBar->addAction(down_A);
	down_A->setStatusTip(tr("Descendre un groupe"));
	
	enableActions(false);
}

GrpScriptList::~GrpScriptList()
{
	if(hasCut)
	{
		foreach(GrpScript *grpScript, grpScriptCopied)
			delete grpScript;
	}
}

void GrpScriptList::setEnabled(bool enabled)
{
	QTreeWidget::setEnabled(enabled);
	_toolBar->setEnabled(enabled);
	enableActions(enabled);
}

GrpScript *GrpScriptList::currentGrpScript()
{
	int grpScriptID = selectedID();
	if(grpScriptID != -1)
		return scripts->grpScript(grpScriptID);
	return NULL;
}

QToolBar *GrpScriptList::toolBar() { return _toolBar; }

void GrpScriptList::enableActions(bool enabled)
{
	_toolBar->setEnabled(enabled);
	foreach(QAction *action, actions())
		action->setEnabled(enabled);
	setContextMenuPolicy(enabled ? Qt::ActionsContextMenu : Qt::NoContextMenu);
}

void GrpScriptList::upDownEnabled()
{
	if(selectedItems().isEmpty())
	{
		actions().at(2)->setEnabled(false);
		actions().at(4)->setEnabled(false);
		actions().at(5)->setEnabled(false);
		actions().at(8)->setEnabled(false);
		actions().at(9)->setEnabled(false);
	}
	else
	{
		actions().at(2)->setEnabled(topLevelItemCount() > 0);
		actions().at(4)->setEnabled(true);
		actions().at(5)->setEnabled(true);
		actions().at(8)->setEnabled(topLevelItemCount() > 1 && currentItem() != topLevelItem(0));
		actions().at(9)->setEnabled(topLevelItemCount() > 1 && currentItem() != topLevelItem(topLevelItemCount()-1));
	}
}

void GrpScriptList::fill(Section1File *scripts)
{
	if(scripts)		this->scripts = scripts;
	Data::currentGrpScriptNames.clear();
	QTreeWidgetItem *item;
	clear();
	
	int i=0;
	foreach(GrpScript *grpScript, this->scripts->grpScripts())
	{
		item = new QTreeWidgetItem(this, QStringList() << QString("%1").arg(i++, 3) << grpScript->getName() << grpScript->getType());
		item->setForeground(2, QBrush(grpScript->getTypeColor()));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		Data::currentGrpScriptNames.append(grpScript->getName());
	}
	
	actions().at(0)->setEnabled(true);
	actions().at(1)->setEnabled(topLevelItemCount() < 256);
	// actions().at(2)->setEnabled(topLevelItemCount() > 0);
	// actions().at(4)->setEnabled(true);
	// actions().at(5)->setEnabled(true);
	actions().at(6)->setEnabled(!grpScriptCopied.isEmpty());
	upDownEnabled();
}

void GrpScriptList::localeRefresh()
{
	int grpScriptID = selectedID();
	QTreeWidgetItem *currentItem = this->currentItem();
	if(grpScriptID != -1 && currentItem != NULL)
	{
		GrpScript *currentGrpScript = scripts->grpScript(grpScriptID);
		currentItem->setText(2, currentGrpScript->getType());
		currentItem->setForeground(2, currentGrpScript->getTypeColor());
	}
}

void GrpScriptList::rename()
{
	rename(currentItem(), 1);
}

void GrpScriptList::rename(QTreeWidgetItem *item, int column)
{
	if(item==NULL || column != 1)	return;
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
	editItem(item, 1);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(renameOK(QTreeWidgetItem *, int)));
}

void GrpScriptList::renameOK(QTreeWidgetItem *item, int column)
{
	if(column != 1)	return;
	disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(renameOK(QTreeWidgetItem *, int)));
	QString newName = item->text(1).left(8);
	item->setText(1, newName);
	Data::currentGrpScriptNames.replace(item->text(0).toInt(), newName);
	scripts->grpScript(selectedID())->setName(newName);
	emit changed();
}

void GrpScriptList::add()
{
	if(topLevelItemCount() > 255)		return;
	
	int grpScriptID = selectedID()+1;

	scripts->insertGrpScript(grpScriptID);
	fill();
	scroll(grpScriptID);
	emit changed();
	rename();
}

void GrpScriptList::del(bool totalDel)
{
	if(topLevelItemCount() == 0)	return;
	QList<int> selectedIDs = this->selectedIDs();
	if(selectedIDs.isEmpty())	return;
	
	if(totalDel && QMessageBox::warning(this, tr("Suppression"), tr("Voulez-vous vraiment supprimer %1 ?").arg(selectedIDs.size()==1 ? tr("le groupe sélectionné") : tr("les groupes sélectionnés")), QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Cancel)		return;
	
	qSort(selectedIDs);
	for(int i=selectedIDs.size()-1 ; i>=0 ; --i)
		totalDel ? scripts->deleteGrpScript(selectedIDs.at(i)) : scripts->removeGrpScript(selectedIDs.at(i));
	fill();
	emit changed();
	if(topLevelItemCount() != 0)
	{
		if(selectedIDs.at(0) >= topLevelItemCount() && selectedIDs.at(0) > 0)	scroll(selectedIDs.at(0)-1);
		else if(selectedIDs.at(0) < topLevelItemCount())	scroll(selectedIDs.at(0));
	}
}

void GrpScriptList::cut()
{
	copy();
	del(false);
	hasCut = true;
}

void GrpScriptList::copy()
{
	QList<int> selectedIDs = this->selectedIDs();
	if(selectedIDs.isEmpty())	return;
	hasCut = false;
	clearCopiedGroups();
	foreach(const int &id, selectedIDs)
		grpScriptCopied.append(scripts->grpScript(id));

	actions().at(6)->setEnabled(true);
}

void GrpScriptList::paste()
{
	int grpScriptID = selectedID()+1, scrollID = grpScriptID;
	if(grpScriptID == 0)	return;
	foreach(GrpScript *GScopied, grpScriptCopied)
		scripts->insertGrpScript(grpScriptID++, GScopied);

	fill();
	scroll(scrollID);
	emit changed();
}

void GrpScriptList::clearCopiedGroups()
{
	if(hasCut)
	{
		foreach(GrpScript *grpScript, grpScriptCopied)
			delete grpScript;
	}
	grpScriptCopied.clear();
}

void GrpScriptList::up() { move(false); }
void GrpScriptList::down() { move(true); }

void GrpScriptList::move(bool direction)
{
	int grpScriptID = selectedID();
	if(grpScriptID == -1)	return;
	if(scripts->moveGrpScript(grpScriptID, direction))
	{
		fill();
		scroll(direction ? grpScriptID+1 : grpScriptID-1);
		emit changed();
	}
	else	setFocus();	
}

void GrpScriptList::scroll(int id, bool focus)
{
	QTreeWidgetItem *item = findItem(id);
	if(item==NULL)	return;
	setCurrentItem(item);
	scrollToItem(item, QAbstractItemView::PositionAtTop);
	if(focus)	setFocus();
}

QTreeWidgetItem *GrpScriptList::findItem(int id)
{
	QList<QTreeWidgetItem *> items = this->findItems(QString("%1").arg(id, 3), Qt::MatchExactly);
	if(items.isEmpty())	return NULL;
	return items.at(0);
}

int GrpScriptList::selectedID()
{
	if(currentItem()==NULL || currentItem()->text(0)=="")	return -1;
	return currentItem()->text(0).toInt();
}

QList<int> GrpScriptList::selectedIDs()
{
	QList<int> liste;
	QList<QTreeWidgetItem *> items = this->selectedItems();
	foreach(QTreeWidgetItem *item, items)
		liste << item->text(0).toInt();
	
	return liste;
}
