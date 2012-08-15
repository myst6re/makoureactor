/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel J�r�me <myst6re@gmail.com>
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
#include "OpcodeList.h"

OpcodeList::OpcodeList(QWidget *parent)
	: QTreeWidget(parent), hasCut(false), isInit(false), script(0)
{
	setColumnCount(1);
	setHeaderLabels(QStringList(tr("Action")));
	setAutoScroll(false);
	setIconSize(QSize(32,11));
	setAlternatingRowColors(true);
	header()->setStretchLastSection(false);
	header()->setResizeMode(0, QHeaderView::ResizeToContents);
	setExpandsOnDoubleClick(false);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	
	QAction *edit_A = new QAction(tr("Modifier"), this);
	edit_A->setShortcut(QKeySequence(Qt::Key_Return));
	edit_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	edit_A->setEnabled(false);
	QAction *add_A = new QAction(QIcon(":/images/plus.png"), tr("Ajouter"), this);
	add_A->setShortcut(QKeySequence("Ctrl++"));
	add_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *del_A = new QAction(QIcon(":/images/minus.png"), tr("Supprimer"), this);
	del_A->setShortcut(QKeySequence(Qt::Key_Delete));
	del_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	del_A->setEnabled(false);
	QAction *cut_A = new QAction(QIcon(":/images/cut.png"), tr("Couper"), this);
	cut_A->setShortcut(QKeySequence("Ctrl+X"));
	cut_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	cut_A->setEnabled(false);
	QAction *copy_A = new QAction(QIcon(":/images/copy.png"), tr("Copier"), this);
	copy_A->setShortcut(QKeySequence("Ctrl+C"));
	copy_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	copy_A->setEnabled(false);
	QAction *paste_A = new QAction(QIcon(":/images/paste.png"), tr("Coller"), this);
	paste_A->setShortcut(QKeySequence("Ctrl+V"));
	paste_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	paste_A->setEnabled(false);
	QAction *up_A = new QAction(QIcon(":/images/up.png"), tr("D�placer vers le haut"), this);
	up_A->setShortcut(QKeySequence("Shift+Up"));
	up_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	up_A->setEnabled(false);
	QAction *down_A = new QAction(QIcon(":/images/down.png"), tr("D�placer vers le bas"), this);
	down_A->setShortcut(QKeySequence("Shift+Down"));
	down_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	down_A->setEnabled(false);
	QAction *expand_A = new QAction(tr("�tendre l'arbre"), this);
	
	connect(edit_A, SIGNAL(triggered()), SLOT(scriptEditor()));
	connect(add_A, SIGNAL(triggered()), SLOT(add()));
	connect(del_A, SIGNAL(triggered()), SLOT(del()));
	connect(cut_A, SIGNAL(triggered()), SLOT(cut()));
	connect(copy_A, SIGNAL(triggered()), SLOT(copy()));
	connect(paste_A, SIGNAL(triggered()), SLOT(paste()));
	connect(up_A, SIGNAL(triggered()), SLOT(up()));
	connect(down_A, SIGNAL(triggered()), SLOT(down()));
	connect(expand_A, SIGNAL(triggered()), SLOT(expandAll()));
	
	addAction(edit_A);
	addAction(add_A);
	addAction(del_A);
	QAction *separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);
	addAction(cut_A);
	addAction(copy_A);
	addAction(paste_A);
	separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);
	addAction(up_A);
	addAction(down_A);
	
	_toolBar = new QToolBar(tr("�dition du &script"));
	_toolBar->setIconSize(QSize(14,14));
	_toolBar->setFloatable(false);
	_toolBar->setAllowedAreas(Qt::NoToolBarArea);
	_toolBar->setMovable(false);
	_toolBar->addAction(add_A);
	add_A->setStatusTip(tr("Ajouter une commande"));
	_toolBar->addAction(del_A);
	del_A->setStatusTip(tr("Supprimer une commande"));
	_toolBar->addSeparator();
	_toolBar->addAction(up_A);
	up_A->setStatusTip(tr("Monter une commande"));
	_toolBar->addAction(down_A);
	down_A->setStatusTip(tr("Descendre une commande"));
	_toolBar->addSeparator();
	_toolBar->addAction(expand_A);
	
	enableActions(false);
}

OpcodeList::~OpcodeList()
{
	if(hasCut)
	{
		foreach(const Opcode *opcode, opcodeCopied)
			delete opcode;
	}
}

void OpcodeList::setEnabled(bool enabled)
{
	_toolBar->setEnabled(enabled);
	QTreeWidget::setEnabled(enabled);
	enableActions(enabled);
}

QToolBar *OpcodeList::toolBar() { return _toolBar; }

void OpcodeList::enableActions(bool enabled)
{
	_toolBar->setEnabled(enabled);
	foreach(QAction *action, actions())
		action->setEnabled(enabled);
	setContextMenuPolicy(enabled ? Qt::ActionsContextMenu : Qt::NoContextMenu);
}

void OpcodeList::itemSelected()
{
	upDownEnabled();
	/*int opcode = selectedOpcode();

	switch(opcode) {
	case 0xa3: // ANIME1

		break;
	}*/
}

void OpcodeList::upDownEnabled()
{
	if(selectedItems().isEmpty())
	{
		actions().at(0)->setEnabled(false);
		actions().at(2)->setEnabled(false);
		actions().at(4)->setEnabled(false);
		actions().at(5)->setEnabled(false);
		actions().at(8)->setEnabled(false);
		actions().at(9)->setEnabled(false);
	}
	else
	{
		actions().at(0)->setEnabled(true);
		actions().at(2)->setEnabled(script && !script->isEmpty());
		actions().at(4)->setEnabled(true);
		actions().at(5)->setEnabled(true);
		actions().at(8)->setEnabled(/* topLevelItemCount() > 1 && */ currentItem() != topLevelItem(0));
		actions().at(9)->setEnabled(true/*  topLevelItemCount() > 1 && currentItem() != topLevelItem(topLevelItemCount()-1) */);
	}
}

void OpcodeList::saveExpandedItems()
{
	qDebug() << "OpcodeList::saveExpandedItems";

	if(script) {
		QList<Opcode *> expandedItems;
		int size = topLevelItemCount();
		for(int i=0 ; i<size ; ++i) {
			QTreeWidgetItem *item = topLevelItem(i);
			if(item->isExpanded()) {
				expandedItems.append(script->getOpcode(item->data(0, Qt::UserRole).toInt()));
			}
		}
		if(size>0) script->setExpandedItems(expandedItems);
	}

	qDebug() << "/OpcodeList::saveExpandedItems";
}

void OpcodeList::fill(Script *_script)
{
	qDebug() << "OpcodeList::fill";

	if(_script) {
		saveExpandedItems();
		script = _script;
	}
	previousBG = QBrush();
	clear();
	header()->setMinimumSectionSize(0);
	
	if(!script->isEmpty()) {
		QList<quint16> indent;
		QList<QTreeWidgetItem *> items;
		QTreeWidgetItem *parentItem = 0;
		int id;
		quint16 opcodeID = 0;
		QPixmap fontPixmap(":/images/chiffres.png");

		foreach(Opcode *curOpcode, script->getOpcodes()) {

			if(curOpcode->isLabel()) {
				while(!indent.isEmpty() &&
					  ((OpcodeLabel *)curOpcode)->label() == indent.last())
				{
					indent.removeLast();
					if(parentItem != 0)
						parentItem = parentItem->parent();
				}
			}

			id = curOpcode->id();

			QTreeWidgetItem *item = new QTreeWidgetItem(parentItem, QStringList(curOpcode->toString()));
			item->setData(0, Qt::UserRole, opcodeID);
			items.append(item);

			QPixmap wordPixmap(32,11);
			item->setIcon(0, QIcon(posNumber(opcodeID, fontPixmap, wordPixmap)));
			item->setToolTip(0, curOpcode->name());
			if((id>=0x14 && id<=0x19) || (id>=0x30 && id<=0x32) || id==0xcb || id==0xcc)
			{
				item->setForeground(0, QColor(0x00,0x66,0xcc));
				indent.append(((OpcodeJump *)curOpcode)->label());
				parentItem = item;
			}
			else if(id>=0x01 && id<=0x07)
				item->setForeground(0, QColor(0xcc,0x66,0x00));
			else if(id>=0x10 && id<=0x13)
				item->setForeground(0, QColor(0x66,0xcc,0x00));
			else if(id==0x00 || id==0x07 || curOpcode->isLabel())
				item->setForeground(0, QColor(0x66,0x66,0x66));

			++opcodeID;
		}

		addTopLevelItems(items);

		opcodeID = 0;
		foreach(QTreeWidgetItem *item, items) {
			if(script->getExpandedItems().contains(script->getOpcode(opcodeID))) {
				item->setExpanded(true);
			}
			++opcodeID;
		}
	}
	else
	{
		QTreeWidgetItem *item = new QTreeWidgetItem(this, QStringList(tr("Si ce script est ex�cut�,\n consid�rez que c'est le dernier script non vide qui est ex�cut�")));
		item->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
		item->setFlags(Qt::NoItemFlags);
		item->setData(0, Qt::UserRole, -1);
	}
	qDebug() << "OpcodeList::fill2";
	scrollToTop();

	if(header()->sectionSize(0) < width())	header()->setMinimumSectionSize(width()-2);
	
	// actions().at(0)->setEnabled(true);
	actions().at(1)->setEnabled(true);
	// actions().at(2)->setEnabled(!script->isEmpty());
	// actions().at(4)->setEnabled(true);
	// actions().at(5)->setEnabled(true);
	actions().at(6)->setEnabled(!opcodeCopied.isEmpty());
	upDownEnabled();
	disconnect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(scriptEditor()));
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(scriptEditor()));
	disconnect(this, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelected()));
	connect(this, SIGNAL(itemSelectionChanged()), SLOT(itemSelected()));
	disconnect(this, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(evidence(QTreeWidgetItem *, QTreeWidgetItem *)));
	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), SLOT(evidence(QTreeWidgetItem *, QTreeWidgetItem *)));
	qDebug() << "/OpcodeList::fill";
}

void OpcodeList::evidence(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	if(current)
	{
		previousBG = current->background(0);
		current->setBackground(0, QColor(196,196,255));
	}
	if(previous)	previous->setBackground(0, previousBG);
}

void OpcodeList::setIsInit(bool isInit)
{
	this->isInit = isInit;
}

void OpcodeList::add()
{
	scriptEditor(false);
}

void OpcodeList::emitHist(int type, int opcodeID, const QByteArray &data)
{
	Historic hist;
	QList<int> opcodeIDs;
	QList<QByteArray> datas;

	opcodeIDs << opcodeID;
	datas << data;

	hist.type = type;
	hist.opcodeIDs = opcodeIDs;
	hist.data = datas;

	emit historicChanged(hist);
}

void OpcodeList::emitHist(int type, const QList<int> &opcodeIDs, const QList<QByteArray> &data)
{
	Historic hist;
	hist.type = type;
	hist.opcodeIDs = opcodeIDs;
	hist.data = data;

	emit historicChanged(hist);
}

void OpcodeList::scriptEditor(bool modify)
{
	if(!script)		return;
	qDebug() << "OpcodeList::scriptEditor" << modify;

	int opcodeID = selectedID();
	if(opcodeID==-1) {
		opcodeID = 0;
		modify = false;
	}

	qDebug() << "OpcodeList::scriptEditor opcodeID" << opcodeID << script->size() << script->getOpcode(opcodeID)->name();

	QByteArray oldVersion;

	saveExpandedItems();

	qDebug() << "OpcodeList::scriptEditor 0";

	if(modify)
		oldVersion = script->getOpcode(opcodeID)->toByteArray();

	qDebug() << "OpcodeList::scriptEditor 00";
	
	ScriptEditor editor(script, opcodeID, modify, isInit, this);
	
	if(editor.exec()==QDialog::Accepted)
	{
		qDebug() << "OpcodeList::scriptEditor1";
		fill();
		qDebug() << "OpcodeList::scriptEditor2";
		scroll(modify ? opcodeID : opcodeID+1);
		if(modify) {
			emitHist(HIST_MOD, opcodeID, oldVersion);
		}
		else {
			emitHist(HIST_ADD, opcodeID+1);
		}
		qDebug() << "OpcodeList::scriptEditor3";
		emit changed();
		qDebug() << "OpcodeList::scriptEditor4";
	}
}

void OpcodeList::del(bool totalDel)
{
	if(topLevelItemCount() == 0)	return;
	QList<int> selectedIDs = this->selectedIDs();
	if(selectedIDs.isEmpty())	return;
	QList<QByteArray> oldVersions;

	if(totalDel && QMessageBox::warning(this, tr("Suppression"), tr("Voulez-vous vraiment supprimer %1 ?").arg(selectedIDs.size()==1 ? tr("la commande s�lectionn�e") : tr("les commandes s�lectionn�es")), QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Cancel)
		return;

	saveExpandedItems();
	
	qSort(selectedIDs);
	for(int i=selectedIDs.size()-1 ; i>=0 ; --i) {
		oldVersions.prepend(script->getOpcode(selectedIDs.at(i))->toByteArray());
		if(totalDel)
			script->delOpcode(selectedIDs.at(i));
		else
			script->removeOpcode(selectedIDs.at(i));
	}
	fill();
	emit changed();
	emitHist(HIST_REM, selectedIDs, oldVersions);
	if(topLevelItemCount() != 0)
	{
		if(selectedIDs.at(0) >= topLevelItemCount() && selectedIDs.at(0) > 0)	scroll(selectedIDs.at(0)-1);
		else if(selectedIDs.at(0) < topLevelItemCount())	scroll(selectedIDs.at(0));
	}
	// else	emit empty();
}

void OpcodeList::cut()
{
	copy();
	del(false);
	hasCut = true;
}

void OpcodeList::copy()
{
	QList<int> selectedIDs = this->selectedIDs();
	if(selectedIDs.isEmpty())	return;
	hasCut = false;

	QMap<int, QTreeWidgetItem *> listeitems;
	foreach(QTreeWidgetItem *item, selectedItems())
		listeitems.insert(item->data(0, Qt::UserRole).toInt(), item);

	QString copiedText;
	QTreeWidgetItem *lastitem=NULL, *parentitem;
	QStack<QTreeWidgetItem *> parentitems;
	int indent=0;
	foreach(QTreeWidgetItem *item, listeitems)
	{
		if(lastitem!=NULL) {
			parentitem = item->parent();
			if(parentitem==lastitem) {
				indent++;
				parentitems.push(parentitem);
			}
			else {
				while(!parentitems.isEmpty() && parentitem != parentitems.top()) {
					indent--;
					parentitems.pop();
				}
			}

			for(int i=0 ; i<indent ; ++i)
				copiedText.append('\t');
		}

		copiedText.append(item->text(0)).append('\n');
		lastitem = item;
	}
	QApplication::clipboard()->setText(copiedText);
	
	clearCopiedOpcodes();
	foreach(const int &id, selectedIDs)
		opcodeCopied.append(script->getOpcode(id));

	actions().at(6)->setEnabled(true);
}

void OpcodeList::paste()
{
	saveExpandedItems();
	QList<int> IDs;
	int opcodeID = selectedID()+1, scrollID = opcodeID;
	foreach(Opcode *Ocopied, opcodeCopied) {
		IDs.append(opcodeID);
		// TODO: label duplication case
		script->insertOpcode(opcodeID++, Script::copyOpcode(Ocopied));
	}

	fill();
	scroll(scrollID);
	emit changed();
	emitHist(HIST_ADD, IDs, QList<QByteArray>());
}

void OpcodeList::up() { move(false); }
void OpcodeList::down() { move(true); }

void OpcodeList::move(bool direction)
{
	int opcodeID = selectedID();
	if(opcodeID == -1)	return;
	saveExpandedItems();
	if(script->moveOpcode(opcodeID, direction))
	{
		fill();
		scroll(direction ? opcodeID+1 : opcodeID-1);
		emit changed();
		if(direction)
			emitHist(HIST_DOW, opcodeID);
		else
			emitHist(HIST_UPW, opcodeID);
	}
	else	setFocus();
}

void OpcodeList::clearCopiedOpcodes()
{
	if(hasCut)
	{
		foreach(const Opcode *opcode, opcodeCopied)
			delete opcode;
	}
	opcodeCopied.clear();
}

void OpcodeList::scroll(int id, bool focus)
{
	QTreeWidgetItem *item = findItem(id);
	if(item==NULL)	return;
	setCurrentItem(item);
	scrollToItem(item, QAbstractItemView::PositionAtTop);
	if(focus)	setFocus();
}

QTreeWidgetItem *OpcodeList::findItem(int id)
{
	QTreeWidgetItemIterator it(this);
	while(*it) {
		if((*it)->data(0, Qt::UserRole).toInt() == id) {
			return *it;
		}
		++it;
	}

	return NULL;
}

int OpcodeList::selectedID()
{
	if(currentItem()==NULL)	return -1;
	return currentItem()->data(0, Qt::UserRole).toInt();
}

QList<int> OpcodeList::selectedIDs()
{
	QList<int> liste;
	foreach(QTreeWidgetItem *item, selectedItems())
		liste.append(item->data(0, Qt::UserRole).toInt());
	qSort(liste);
	return liste;
}

int OpcodeList::selectedOpcode()
{
	int opcodeID = selectedID();
	return opcodeID==-1 ? 0 : script->getOpcode(opcodeID)->id();
}

QPixmap &OpcodeList::posNumber(int num, const QPixmap &fontPixmap, QPixmap &wordPixmap)
{
	QString strNum = QString("%1").arg(num, 5, 10, QChar(' '));
	wordPixmap.fill(QColor(0,0,0,0));
	QPainter painter(&wordPixmap);

	if(strNum.at(0)!=' ')
		painter.drawTiledPixmap(1, 1, 5, 9, fontPixmap, 5*strNum.mid(0,1).toInt(), 0);
	if(strNum.at(1)!=' ')
		painter.drawTiledPixmap(7, 1, 5, 9, fontPixmap, 5*strNum.mid(1,1).toInt(), 0);
	if(strNum.at(2)!=' ')
		painter.drawTiledPixmap(13, 1, 5, 9, fontPixmap, 5*strNum.mid(2,1).toInt(), 0);
	if(strNum.at(3)!=' ')
		painter.drawTiledPixmap(19, 1, 5, 9, fontPixmap, 5*strNum.mid(3,1).toInt(), 0);
	if(strNum.at(4)!=' ')
		painter.drawTiledPixmap(25, 1, 5, 9, fontPixmap, 5*strNum.mid(4,1).toInt(), 0);

	painter.end();
	return wordPixmap;
}
