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
#include "OpcodeList.h"
#include "ScriptEditor.h"
#include "core/Config.h"

OpcodeList::OpcodeList(QWidget *parent) :
	QTreeWidget(parent), isInit(false),
	field(0), grpScript(0), script(0), errorLine(-1)
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
	
	edit_A = new QAction(tr("Modifier"), this);
	edit_A->setShortcut(QKeySequence(Qt::Key_Return));
	edit_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	edit_A->setEnabled(false);
	add_A = new QAction(QIcon(":/images/plus.png"), tr("Ajouter"), this);
	add_A->setShortcut(QKeySequence("Ctrl++"));
	add_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	del_A = new QAction(QIcon(":/images/minus.png"), tr("Supprimer"), this);
	del_A->setShortcut(QKeySequence(Qt::Key_Delete));
	del_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	del_A->setEnabled(false);
	cut_A = new QAction(QIcon(":/images/cut.png"), tr("Couper"), this);
	cut_A->setShortcut(QKeySequence("Ctrl+X"));
	cut_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	cut_A->setEnabled(false);
	copy_A = new QAction(QIcon(":/images/copy.png"), tr("Copier"), this);
	copy_A->setShortcut(QKeySequence("Ctrl+C"));
	copy_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	copy_A->setEnabled(false);
	paste_A = new QAction(QIcon(":/images/paste.png"), tr("Coller"), this);
	paste_A->setShortcut(QKeySequence("Ctrl+V"));
	paste_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	paste_A->setEnabled(false);
	up_A = new QAction(QIcon(":/images/up.png"), tr("Déplacer vers le haut"), this);
	up_A->setShortcut(QKeySequence("Shift+Up"));
	up_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	up_A->setEnabled(false);
	down_A = new QAction(QIcon(":/images/down.png"), tr("Déplacer vers le bas"), this);
	down_A->setShortcut(QKeySequence("Shift+Down"));
	down_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	down_A->setEnabled(false);
	expand_A = new QAction(tr("Étendre l'arbre"), this);
	undo_A = new QAction(QIcon(":/images/undo.png"), tr("Annuler"), this);
	undo_A->setShortcut(QKeySequence::Undo);
	undo_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	undo_A->setEnabled(false);
	redo_A = new QAction(QIcon(":/images/redo.png"), tr("Refaire"), this);
	redo_A->setShortcut(QKeySequence::Redo);
	redo_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	redo_A->setEnabled(false);
	text_A = new QAction(tr("Modifier texte"), this);
	text_A->setVisible(false);
	goto_A = new QAction(tr("Aller au label"), this);
	goto_A->setVisible(false);

	connect(edit_A, SIGNAL(triggered()), SLOT(scriptEditor()));
	connect(add_A, SIGNAL(triggered()), SLOT(add()));
	connect(del_A, SIGNAL(triggered()), SLOT(del()));
	connect(cut_A, SIGNAL(triggered()), SLOT(cut()));
	connect(copy_A, SIGNAL(triggered()), SLOT(copy()));
	connect(paste_A, SIGNAL(triggered()), SLOT(paste()));
	connect(up_A, SIGNAL(triggered()), SLOT(up()));
	connect(down_A, SIGNAL(triggered()), SLOT(down()));
	connect(expand_A, SIGNAL(triggered()), SLOT(expandAll()));
	connect(text_A, SIGNAL(triggered()), SLOT(editText()));
	connect(undo_A, SIGNAL(triggered()), SLOT(undo()));
	connect(redo_A, SIGNAL(triggered()), SLOT(redo()));
	connect(goto_A, SIGNAL(triggered()), SLOT(gotoLabel()));
	
	addAction(edit_A);
	addAction(add_A);
	addAction(del_A);
	addAction(text_A);
	addAction(goto_A);
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
	separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);
	addAction(undo_A);
	addAction(redo_A);

	QWidget *help = new QWidget;
	_help = new QLabel(tr("Alt + clic pour aller au label"));
	_help->hide();
	QHBoxLayout *helpLayout = new QHBoxLayout(help);
	helpLayout->addStretch();
	helpLayout->addWidget(_help);
	helpLayout->setContentsMargins(QMargins());
	
	_toolBar = new QToolBar(tr("Édition du &script"));
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
	_toolBar->addAction(text_A);
	_toolBar->addAction(goto_A);
	_toolBar->addSeparator();
	_toolBar->addAction(undo_A);
	undo_A->setStatusTip(undo_A->text());
	_toolBar->addAction(redo_A);
	redo_A->setStatusTip(redo_A->text());
	_toolBar->addWidget(help);

	enableActions(false);

	setMinimumWidth(_toolBar->sizeHint().width());
	setMinimumHeight(_toolBar->sizeHint().width());

	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(scriptEditor()));
	connect(this, SIGNAL(itemSelectionChanged()), SLOT(itemSelected()));
	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(evidence(QTreeWidgetItem*,QTreeWidgetItem*)));
}

OpcodeList::~OpcodeList()
{
	qDeleteAll(opcodeCopied);
}

void OpcodeList::clear()
{
	saveExpandedItems();
	enableActions(false);
	QTreeWidget::clear();
	clearHist();
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
	foreach(QAction *action, actions()) {
		if(action == undo_A || action == redo_A)
			continue;
		action->setEnabled(enabled);
	}
	setContextMenuPolicy(enabled ? Qt::ActionsContextMenu : Qt::NoContextMenu);
}

void OpcodeList::setErrorLine(int opcodeID)
{
	QTreeWidgetItem *item;

	item = findItem(errorLine);
	if(item != NULL) {
		item->setBackground(0, previousErrorBg);
	}

	item = findItem(opcodeID);
	if(item != NULL) {
		previousErrorBg = item->background(0);
		item->setBackground(0, QColor(0xFF,0xCC,0xCC));// red
	}

	errorLine = opcodeID;
}

void OpcodeList::itemSelected()
{
	upDownEnabled();
	int opcodeID = selectedID();
	if(opcodeID <= -1 || opcodeID >= script->size()) {
		return;
	}
	Opcode *opcode = script->opcode(opcodeID);

	switch(opcode->id()) {
	case Opcode::ASK:
	case Opcode::MESSAGE:
	case Opcode::MPNAM:
		text_A->setVisible(true);
		break;
	case Opcode::SPECIAL:
	{
		OpcodeSPECIAL *op = (OpcodeSPECIAL *)opcode;
		text_A->setVisible(op && op->opcode->id() == 0xFD);
	}
		break;
	default:
		text_A->setVisible(false);
		break;
	}

	goto_A->setVisible(opcode->isJump());
	_help->setVisible(opcode->isJump());
}

void OpcodeList::upDownEnabled()
{
	if(selectedItems().isEmpty())
	{
		edit_A->setEnabled(false);
		del_A->setEnabled(false);
		cut_A->setEnabled(false);
		copy_A->setEnabled(false);
		up_A->setEnabled(false);
		down_A->setEnabled(false);
	}
	else
	{
		edit_A->setEnabled(true);
		del_A->setEnabled(script && !script->isEmpty());
		cut_A->setEnabled(true);
		copy_A->setEnabled(true);
		up_A->setEnabled(/* topLevelItemCount() > 1 && */ currentItem() != topLevelItem(0));
		down_A->setEnabled(true/*  topLevelItemCount() > 1 && currentItem() != topLevelItem(topLevelItemCount()-1) */);
	}
}

void OpcodeList::editText()
{
	if(!script)	return;

	int opcodeID = selectedID();
	if(opcodeID >= 0 && opcodeID < script->size()) {
		Opcode *op = script->opcode(opcodeID);
		int textID = op->getTextID();
		if(textID >= 0) {
			emit editText(textID);
		}
	}
}

void OpcodeList::saveExpandedItems()
{
	if(script) {
		QList<const Opcode *> expandedItems;
		int size = topLevelItemCount();
		for(int i=0 ; i<size ; ++i) {
			QTreeWidgetItem *item = topLevelItem(i);
			if(item->isExpanded()) {
				int opcodeID = item->data(0, Qt::UserRole).toInt();
				if(opcodeID >= 0 && opcodeID < script->size()) {
					expandedItems.append(script->opcode(opcodeID));
				}
			}
		}
		if(size>0) setExpandedItems(expandedItems);
	}
}

bool OpcodeList::itemIsExpanded(const Opcode *opcode) const
{
	return script && (
			(!expandedItems.contains(script)
			 && Config::value("scriptItemExpandedByDefault", false).toBool())
			|| expandedItems.value(script).contains(opcode));
}

void OpcodeList::setExpandedItems(const QList<const Opcode *> &expandedItems)
{
	if(script) {
		this->expandedItems.insert(script, expandedItems);
	}
}

void OpcodeList::fill(Field *_field, GrpScript *_grpScript, Script *_script)
{
	if(_script) {
		saveExpandedItems();
		clearHist();
		field = _field;
		grpScript = _grpScript;
		script = _script;
	}

	previousBG = QBrush();
	blockSignals(true);
	QTreeWidget::clear();
	blockSignals(false);
	header()->setMinimumSectionSize(0);
	
	if(!script->isEmpty()) {
		QList<quint16> indent;
		QList<QTreeWidgetItem *> items;
		QTreeWidgetItem *parentItem = 0;
		int id;
		quint16 opcodeID = 0;
		QPixmap fontPixmap(":/images/chiffres.png");

		foreach(Opcode *curOpcode, script->opcodes()) {

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

			QTreeWidgetItem *item = new QTreeWidgetItem(parentItem, QStringList(curOpcode->toString(field)));
			item->setData(0, Qt::UserRole, opcodeID);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			items.append(item);

			QPixmap wordPixmap(32,11);
			item->setIcon(0, QIcon(posNumber(opcodeID+1, fontPixmap, wordPixmap)));
			item->setToolTip(0, curOpcode->name());
			if((id>=0x14 && id<=0x19) || (id>=0x30 && id<=0x32) || id==0xcb || id==0xcc)
			{
				item->setForeground(0, QColor(0x00,0x66,0xcc));
				if(!((OpcodeJump *)curOpcode)->isBadJump()) {
					indent.append(((OpcodeJump *)curOpcode)->label());
				}
				parentItem = item;
			}
			else if(id >= Opcode::REQ && id <= Opcode::RETTO)
				item->setForeground(0, QColor(0xcc,0x66,0x00));
			else if(id >= Opcode::JMPF && id <= Opcode::JMPBL)
				item->setForeground(0, QColor(0x66,0xcc,0x00));
			else if(id == Opcode::RET)
				item->setForeground(0, QColor(0x66,0x66,0x66));
			else if(curOpcode->isLabel())
				item->setForeground(0, QColor(0xcc,0x00,0x00));

			++opcodeID;
		}

		addTopLevelItems(items);

		opcodeID = 0;
		foreach(QTreeWidgetItem *item, items) {
			if(itemIsExpanded(script->opcode(opcodeID))) {
				item->setExpanded(true);
			}
			++opcodeID;
		}
	} else {
		QTreeWidgetItem *item = new QTreeWidgetItem(this, QStringList(tr("Si ce script est exécuté,\n considérez que c'est le dernier script non vide qui est exécuté")));
		item->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
		item->setFlags(Qt::NoItemFlags);
		item->setData(0, Qt::UserRole, -2);
	}

	scrollToTop();

	enableActions(true);

	if(header()->sectionSize(0) < width()) {
		header()->setMinimumSectionSize(width() - 2);
	}
	
	// edit_A->setEnabled(true);
	add_A->setEnabled(true);
	// del_A->setEnabled(!script->isEmpty());
	// cut_A->setEnabled(true);
	// copy_A->setEnabled(true);
	paste_A->setEnabled(!opcodeCopied.isEmpty());
	itemSelected();
}

void OpcodeList::evidence(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	QTreeWidgetItem *errorItem = findItem(errorLine);

	if(current && errorItem != current)
	{
		previousBG = current->background(0);
		current->setBackground(0, QColor(196,196,255));
	}
	if(previous && errorItem != previous) {
		previous->setBackground(0, previousBG);
	}
}

void OpcodeList::setIsInit(bool isInit)
{
	this->isInit = isInit;
}

void OpcodeList::add()
{
	scriptEditor(false);
}

/*QString OpcodeList::showHistoric()
{
	QStringList ret;
	foreach(const Historic &h, hists) {
		QString type;
		switch(h.type) {
		case Add:	type = "Add";	break;
		case Modify:	type = "Modify";	break;
		case ModifyAndAddLabel:	type = "ModifyAndAddLabel";	break;
		case Remove:	type = "Remove";	break;
		case Up:	type = "Up";	break;
		case Down:	type = "Down";	break;
		}
		ret.append(type);
	}
	QStringList ret2;
	foreach(const Historic &h, restoreHists) {
		QString type;
		switch(h.type) {
		case Add:	type = "Add";	break;
		case Modify:	type = "Modify";	break;
		case ModifyAndAddLabel:	type = "ModifyAndAddLabel";	break;
		case Remove:	type = "Remove";	break;
		case Up:	type = "Up";	break;
		case Down:	type = "Down";	break;
		}
		ret2.append(type);
	}
	return "(" + ret.join(", ") + ") (" + ret2.join(", ") + ")";
}*/

void OpcodeList::changeHist(HistoricType type, int opcodeID, Opcode *data)
{
	Historic hist;
	hist.type = type;
	hist.opcodeIDs = QList<int>() << opcodeID;
	hist.data = QList<Opcode *>() << data;

	undo_A->setEnabled(true);
	redo_A->setEnabled(false);
	hists.push(hist);
	restoreHists.clear();
//	qDebug() << showHistoric();
}

void OpcodeList::changeHist(HistoricType type, const QList<int> &opcodeIDs, const QList<Opcode *> &data)
{
	Historic hist;
	hist.type = type;
	hist.opcodeIDs = opcodeIDs;
	hist.data = data;

	undo_A->setEnabled(true);
	redo_A->setEnabled(false);
	hists.push(hist);
	restoreHists.clear();
//	qDebug() << showHistoric();
}

void OpcodeList::clearHist()
{
	undo_A->setEnabled(false);
	redo_A->setEnabled(false);

	while(!hists.isEmpty()) {
		Historic hist = hists.pop();
		qDeleteAll(hist.data);
	}

	while(!restoreHists.isEmpty()) {
		Historic hist = restoreHists.pop();
		qDeleteAll(hist.data);
	}

//	qDebug() << showHistoric();
}

void OpcodeList::undo()
{
	if(hists.isEmpty())		return;

	Historic hist = hists.pop();
	undo_A->setEnabled(!hists.isEmpty());

	int firstOpcode = hist.opcodeIDs.first();
	Opcode *sav;

	switch(hist.type) {
	case Add:
		// del opcodes
		for(int i=hist.opcodeIDs.size()-1 ; i>=0 ; --i) {
			hist.data.prepend(Script::copyOpcode(script->opcode(hist.opcodeIDs.at(i))));
			script->delOpcode(hist.opcodeIDs.at(i));
		}
		break;
	case Remove:
		// restore opcodes
		for(int i=0 ; i<hist.opcodeIDs.size() ; ++i)
			script->insertOpcode(hist.opcodeIDs.at(i), hist.data.at(i));
		hist.data.clear();
		break;
	case Modify:
		// restore old version
		sav = Script::copyOpcode(script->opcode(firstOpcode));
		script->setOpcode(firstOpcode, hist.data.first());
		hist.data.replace(0, sav);
		break;
	case ModifyAndAddLabel:
		// del label
		hist.data.prepend(Script::copyOpcode(script->opcode(firstOpcode+1)));
		script->delOpcode(firstOpcode+1);
		// restore old version
		sav = Script::copyOpcode(script->opcode(firstOpcode));
		script->setOpcode(firstOpcode, hist.data.first());
		hist.data.replace(0, sav);
		break;
	case Up:
		// move down
		script->moveOpcode(firstOpcode-1, Script::Down);
		break;
	case Down:
		// move up
		script->moveOpcode(firstOpcode+1, Script::Up);
		break;
	}

	restoreHists.push(hist);
	redo_A->setEnabled(true);

//	qDebug() << showHistoric();

	fill();// Refresh view

	emit changed();

	scroll(firstOpcode);
}

void OpcodeList::redo()
{
	if(restoreHists.isEmpty())		return;

	Historic hist = restoreHists.pop();
	redo_A->setEnabled(!restoreHists.isEmpty());

	int firstOpcode = hist.opcodeIDs.first();
	Opcode *sav;

	switch(hist.type) {
	case Add:
		for(int i=0 ; i<hist.opcodeIDs.size() ; ++i)
			script->insertOpcode(hist.opcodeIDs.at(i), hist.data.at(i));
		hist.data.clear();
		break;
	case Remove:
		for(int i=hist.opcodeIDs.size()-1 ; i>=0 ; --i) {
			hist.data.prepend(Script::copyOpcode(script->opcode(hist.opcodeIDs.at(i))));
			script->delOpcode(hist.opcodeIDs.at(i));
		}
		break;
	case Modify:
		sav = Script::copyOpcode(script->opcode(firstOpcode));
		script->setOpcode(firstOpcode, hist.data.first());
		hist.data.replace(0, sav);
		break;
	case ModifyAndAddLabel:
		sav = Script::copyOpcode(script->opcode(firstOpcode));
		script->setOpcode(firstOpcode, hist.data.first());
		hist.data.replace(0, sav);
		script->insertOpcode(hist.opcodeIDs.at(1), hist.data.at(1));
		hist.data.removeAt(1);
		break;
	case Up:
		script->moveOpcode(--firstOpcode, Script::Down);
	break;
	case Down:
		script->moveOpcode(++firstOpcode, Script::Up);
	break;
	}

	hists.push(hist);
	undo_A->setEnabled(true);

//	qDebug() << showHistoric();

	fill();// Refresh view

	emit changed();

	scroll(firstOpcode);
}

void OpcodeList::scriptEditor(bool modify)
{
	if(!script)		return;

	int opcodeID = selectedID();
	if(opcodeID==-1) {
		modify = false;
	}

	Opcode *oldVersion = 0;

	saveExpandedItems();

	if(modify)
		oldVersion = Script::copyOpcode(script->opcode(opcodeID));
	else
		++opcodeID;

	ScriptEditor editor(field, grpScript, script, opcodeID, modify, isInit, this);
	
	if(editor.exec()==QDialog::Accepted)
	{
		fill();
		scroll(opcodeID);
		if(modify) {
			if(editor.needslabel()) {
				changeHist(ModifyAndAddLabel, opcodeID, oldVersion);
			} else {
				changeHist(Modify, opcodeID, oldVersion);
			}
		}
		else {
			if(editor.needslabel()) {
				changeHist(Add, QList<int>() << opcodeID << (opcodeID + 1), QList<Opcode *>());
			} else {
				changeHist(Add, opcodeID);
			}
		}
		emit changed();
	}
}

void OpcodeList::del(bool totalDel)
{
	if(topLevelItemCount() == 0)	return;
	QList<int> selectedIDs = this->selectedIDs();
	if(selectedIDs.isEmpty())	return;
	QList<Opcode *> oldVersions;

	if(totalDel && QMessageBox::warning(this, tr("Suppression"), tr("Voulez-vous vraiment supprimer %1 ?").arg(selectedIDs.size()==1 ? tr("la commande sélectionnée") : tr("les commandes sélectionnées")), QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Cancel)
		return;

	saveExpandedItems();
	
	qSort(selectedIDs);
	for(int i=selectedIDs.size()-1 ; i>=0 ; --i) {
		oldVersions.prepend(Script::copyOpcode(script->opcode(selectedIDs.at(i))));
		if(totalDel) {
			script->delOpcode(selectedIDs.at(i));
		} else {
			script->removeOpcode(selectedIDs.at(i));
		}
	}

	fill();
	emit changed();
	changeHist(Remove, selectedIDs, oldVersions);

	if(topLevelItemCount() != 0) {
		if(selectedIDs.at(0) >= topLevelItemCount() && selectedIDs.at(0) > 0)	scroll(selectedIDs.at(0)-1);
		else if(selectedIDs.at(0) < topLevelItemCount())	scroll(selectedIDs.at(0));
	}
	// else	emit empty();
}

void OpcodeList::cut()
{
	copy();
	del(false);
}

void OpcodeList::copy()
{
	QList<int> selectedIDs = this->selectedIDs();
	if(selectedIDs.isEmpty())	return;

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
	foreach(const int &id, selectedIDs) {
		opcodeCopied.append(Script::copyOpcode(script->opcode(id)));
	}

	paste_A->setEnabled(true);
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
	changeHist(Add, IDs, QList<Opcode *>());
}

void OpcodeList::up()	{	move(Script::Up);		}
void OpcodeList::down() {	move(Script::Down);		}

void OpcodeList::move(Script::MoveDirection direction)
{
	int opcodeID = selectedID();
	if(opcodeID == -1)	return;
	saveExpandedItems();
	if(script->moveOpcode(opcodeID, direction))
	{
		fill();
		scroll(direction == Script::Down ? opcodeID+1 : opcodeID-1);
		emit changed();
		if(direction == Script::Down)
			changeHist(Down, opcodeID);
		else
			changeHist(Up, opcodeID);
	}
	else	setFocus();
}

void OpcodeList::clearCopiedOpcodes()
{
	qDeleteAll(opcodeCopied);
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
	return opcodeID <= -1 || opcodeID >= script->size()
			? -1
			: script->opcode(opcodeID)->id();
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

void OpcodeList::gotoLabel(QTreeWidgetItem *item)
{
	if(item == NULL) {
		item = currentItem();
	}
	int opcodeID = item->data(0, Qt::UserRole).toInt();
	Opcode *op = script->opcode(opcodeID);

	if(op->isJump()) {
		OpcodeJump *opJ = (OpcodeJump *)op;

		if(!opJ->isBadJump()) {
			int opcodeID = 0;

			foreach(const Opcode *op, script->opcodes()) {
				if(op->isLabel() && ((OpcodeLabel *)op)->label() == opJ->label()) {
					scroll(opcodeID);
					break;
				}
				++opcodeID;
			}
		}
	}
}

void OpcodeList::mouseReleaseEvent(QMouseEvent *event)
{
	// Alt + left click
	if(event->button() == Qt::LeftButton &&
			event->modifiers().testFlag(Qt::AltModifier)) {
		QTreeWidgetItem *item = itemAt(event->pos());
		if(item == NULL) {
			return;
		}

		gotoLabel(item);
	}
	QTreeWidget::mouseReleaseEvent(event);
}
