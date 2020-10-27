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
#include "Data.h"

GrpScriptList::GrpScriptList(QWidget *parent) :
    QTreeWidget(parent)
{
	qreal scale = qApp->desktop()->logicalDpiX() / 96.0;
	setColumnCount(3);
	setHeaderLabels(QStringList() << tr("Id") << tr("Group") << tr("Type"));
	setIndentation(0);
	setItemsExpandable(false);
	setSortingEnabled(true);
	setColumnWidth(0, fontMetrics().width("8888"));
	setColumnWidth(1, fontMetrics().width("WWWWWWWW"));
	setColumnWidth(2, 0);
	setContextMenuPolicy(Qt::ActionsContextMenu);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	sortByColumn(0, Qt::AscendingOrder);

	QAction *rename_A = new QAction(tr("Rename group"), this);
	rename_A->setShortcut(QKeySequence("F2"));
	rename_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *add_A = new QAction(QIcon(":/images/plus.png"), tr("Add group"), this);
	add_A->setShortcut(QKeySequence("Ctrl++"));
	add_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *del_A = new QAction(QIcon(":/images/minus.png"), tr("Delete group"), this);
	del_A->setShortcut(QKeySequence(Qt::Key_Delete));
	del_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	del_A->setEnabled(false);
	QAction *cut_A = new QAction(QIcon(":/images/cut.png"), tr("Cut group"), this);
	cut_A->setShortcut(QKeySequence("Ctrl+X"));
	cut_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	cut_A->setEnabled(false);
	QAction *copy_A = new QAction(QIcon(":/images/copy.png"), tr("Copy group"), this);
	copy_A->setShortcut(QKeySequence("Ctrl+C"));
	copy_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	copy_A->setEnabled(false);
	QAction *paste_A = new QAction(QIcon(":/images/paste.png"), tr("Paste group"), this);
	paste_A->setShortcut(QKeySequence("Ctrl+V"));
	paste_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	paste_A->setEnabled(false);
	QAction *up_A = new QAction(QIcon(":/images/up.png"), tr("Move up"), this);
	up_A->setShortcut(QKeySequence("Shift+Up"));
	up_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	up_A->setEnabled(false);
	QAction *down_A = new QAction(QIcon(":/images/down.png"), tr("Move down"), this);
	down_A->setShortcut(QKeySequence("Shift+Down"));
	down_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	down_A->setEnabled(false);

	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(rename(QTreeWidgetItem *, int)));
	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(evidence(QTreeWidgetItem*,QTreeWidgetItem*)));
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

	_toolBar = new QToolBar(tr("&Group Editor"));
	_toolBar->setIconSize(QSize(14*scale,14*scale));
	_toolBar->addAction(add_A);
	add_A->setStatusTip(tr("Add a group"));
	_toolBar->addAction(del_A);
	del_A->setStatusTip(tr("Remove a group"));
	_toolBar->addSeparator();
	_toolBar->addAction(up_A);
	up_A->setStatusTip(tr("Up"));
	_toolBar->addAction(down_A);
	down_A->setStatusTip(tr("Down"));

	_helpWidget = new HelpWidget(32, HelpWidget::IconWarning, this);
	_helpWidget->hide();

	enableActions(false);

	setMinimumWidth(_toolBar->sizeHint().width());
}

GrpScriptList::~GrpScriptList()
{
	qDeleteAll(grpScriptCopied);
}

void GrpScriptList::evidence(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	if (current) {
		QColor color = Data::color(Data::ColorEvidence);
		current->setBackground(0, color);
		current->setBackground(1, color);
		current->setBackground(2, color);
	}

	if (previous) {
		previous->setBackground(0, QBrush());
		previous->setBackground(1, QBrush());
		previous->setBackground(2, QBrush());
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
	if (grpScriptID != -1) {
		return scripts->grpScript(grpScriptID);
	}
	return nullptr;
}

void GrpScriptList::enableActions(bool enabled)
{
	_toolBar->setEnabled(enabled);
	setContextMenuPolicy(enabled ? Qt::ActionsContextMenu : Qt::NoContextMenu);
}

void GrpScriptList::upDownEnabled()
{
	if (selectedItems().isEmpty()) {
		actions().at(DelAction)->setEnabled(false);
		actions().at(CutAction)->setEnabled(false);
		actions().at(CopyAction)->setEnabled(false);
		actions().at(UpAction)->setEnabled(false);
		actions().at(DownAction)->setEnabled(false);
	} else {
		actions().at(DelAction)->setEnabled(topLevelItemCount() > 0);
		actions().at(CutAction)->setEnabled(true);
		actions().at(CopyAction)->setEnabled(true);
		actions().at(UpAction)->setEnabled(topLevelItemCount() > 1 && currentItem() != topLevelItem(0));
		actions().at(DownAction)->setEnabled(topLevelItemCount() > 1 && currentItem() != topLevelItem(topLevelItemCount()-1));
	}
}

void GrpScriptList::fill(Section1File *scripts)
{
	if (scripts) {
		this->scripts = scripts;
	}
	clear();

	int i=0;
	for (GrpScript *grpScript : this->scripts->grpScripts()) {
		QTreeWidgetItem *item = new QTreeWidgetItem(this, QStringList() << QString("%1").arg(i++, 3) << grpScript->name() << grpScript->type());
		item->setForeground(2, QBrush(grpScript->typeColor()));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		item->setToolTip(0, grpScript->type());
		QIcon icon;

		if (grpScript->character() >= 0 && grpScript->character() < 9) {
			icon = QIcon(QString(":/images/icon-char-%1.png").arg(grpScript->character()));
		} else {
			switch (grpScript->typeID()) {
			case GrpScript::Director:
				icon = QIcon(":/images/main.png");
				break;
			case GrpScript::Model:
				icon = QIcon(":/images/model.png");
				break;
			case GrpScript::Animation:
				icon = QIcon(":/images/background.png");
				break;
			case GrpScript::Location:
				icon = QIcon(":/images/location.png");
				break;
			default:
				QPixmap pixnull(16, 16);
				pixnull.fill(Qt::transparent);
				icon = QIcon(pixnull);
				break;
			}
		}

		if (!icon.isNull()) {
			item->setIcon(0, icon);
		}
	}

	resizeColumnToContents(0);
	resizeColumnToContents(1);

	updateHelpWidget();

	setMinimumWidth(columnWidth(0) +
	                columnWidth(1) +
	                fontMetrics().boundingRect("WWWWWWWW").width());

	actions().at(RenameAction)->setEnabled(true);
	actions().at(AddAction)->setEnabled(topLevelItemCount() < 256);
	upDownEnabled();
}

void GrpScriptList::localeRefresh()
{
	int grpScriptID = selectedID();
	QTreeWidgetItem *currentItem = this->currentItem();
	if (grpScriptID != -1 && currentItem != nullptr) {
		GrpScript *currentGrpScript = scripts->grpScript(grpScriptID);
		currentItem->setText(2, currentGrpScript->type());
		currentItem->setForeground(2, currentGrpScript->typeColor());

		updateHelpWidget();
	}
}

void GrpScriptList::updateHelpWidget()
{
	_helpWidget->hide();
	QStringList texts;

	if (scripts->modelCount() > 16) {
		_helpWidget->show();
		texts.append(tr("You have more than 16 models in this field, "
		                                  "the game may crash."));
	}

	if (scripts->grpScriptCount() > 48) {
		_helpWidget->show();
		texts.append(tr("You have more than 48 groups in this field, "
		                                  "the game may crash."));
	}

	_helpWidget->setText(texts.join("\n"));
}

void GrpScriptList::rename(QTreeWidgetItem *item, int column)
{
	if (item==nullptr || column != 1) {
		return;
	}
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
	editItem(item, 1);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(renameOK(QTreeWidgetItem *, int)));
}

void GrpScriptList::renameOK(QTreeWidgetItem *item, int column)
{
	if (column != 1) {
		return;
	}
	disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(renameOK(QTreeWidgetItem *, int)));
	QString newName = item->text(1).left(8);
	item->setText(1, newName);
	scripts->grpScript(selectedID())->setName(newName);
	emit changed();
}

void GrpScriptList::add()
{
	if (topLevelItemCount() > scripts->maxGrpScriptCount()) {
		return;
	}

	int grpScriptID = selectedID()+1;

	scripts->insertGrpScript(grpScriptID);
	fill();
	scroll(grpScriptID);
	emit changed();
	rename();
}

void GrpScriptList::del(bool totalDel)
{
	if (topLevelItemCount() == 0) {
		return;
	}
	QList<int> selectedIDs = this->selectedIDs();
	if (selectedIDs.isEmpty()) {
		return;
	}

	if (totalDel && QMessageBox::warning(this, tr("Delete"), tr("Are you sure you want to remove %1?\n"
	                                                           "Some scripts can refer to it!")
	                                    .arg(selectedIDs.size()==1 ?
	                                         tr("the group selected") :
	                                         tr("the selected groups")),
	                                    QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Cancel) {
		return;
	}

	std::sort(selectedIDs.begin(), selectedIDs.end());
	for (int i = selectedIDs.size()-1; i >= 0; --i) {
		totalDel ? scripts->deleteGrpScript(selectedIDs.at(i)) : scripts->removeGrpScript(selectedIDs.at(i));
	}

	blockSignals(true);
	fill();
	blockSignals(false);

	emit changed();

	if (topLevelItemCount() != 0) {
		if (selectedIDs.at(0) >= topLevelItemCount() && selectedIDs.at(0) > 0) {
			scroll(selectedIDs.at(0)-1);
		} else if (selectedIDs.at(0) < topLevelItemCount()) {
			scroll(selectedIDs.at(0));
		}
	}
}

void GrpScriptList::cut()
{
	copy();
	del(false);
}

void GrpScriptList::copy()
{
	QList<int> selectedIDs = this->selectedIDs();
	if (selectedIDs.isEmpty()) {
		return;
	}

	clearCopiedGroups();
	for (const int &id : qAsConst(selectedIDs)) {
		grpScriptCopied.append(new GrpScript(*scripts->grpScript(id)));
	}

	actions().at(PasteAction)->setEnabled(true);
}

void GrpScriptList::paste()
{
	if (grpScriptCopied.isEmpty()) {
		return;
	}
	int grpScriptID = selectedID() + 1;
	if (grpScriptID == 0) {
		grpScriptID = topLevelItemCount(); // Last position
	}
	int i = grpScriptID;
	for (GrpScript *GScopied : qAsConst(grpScriptCopied)) {
		scripts->insertGrpScript(i++, new GrpScript(*GScopied));
	}

	fill();
	scroll(grpScriptID);
	emit changed();
}

void GrpScriptList::clearCopiedGroups()
{
	actions().at(PasteAction)->setEnabled(false);
	qDeleteAll(grpScriptCopied);
	grpScriptCopied.clear();
}

void GrpScriptList::move(bool direction)
{
	int grpScriptID = selectedID();
	if (grpScriptID == -1) {
		return;
	}
	if (scripts->moveGrpScript(grpScriptID, direction)) {
		fill();
		scroll(direction ? grpScriptID+1 : grpScriptID-1);
		emit changed();
	} else {
		setFocus();
	}
}

void GrpScriptList::scroll(int id, bool focus)
{
	if (selectedID() != id) {
		QTreeWidgetItem *item = findItem(id);
		if (!item) {
			return;
		}
		setCurrentItem(item);
		scrollToItem(item, QAbstractItemView::PositionAtTop);
	}

	if (focus) {
		setFocus();
	}
}

QTreeWidgetItem *GrpScriptList::findItem(int id)
{
	QList<QTreeWidgetItem *> items = findItems(QString("%1").arg(id, 3), Qt::MatchExactly);
	if (items.isEmpty()) {
		return nullptr;
	}
	return items.first();
}

int GrpScriptList::selectedID()
{
	if (!currentItem() || currentItem()->text(0).isEmpty()) {
		return -1;
	}
	return currentItem()->text(0).toInt();
}

QList<int> GrpScriptList::selectedIDs()
{
	QList<int> list;

	for (QTreeWidgetItem *item : selectedItems()) {
		list.append(item->text(0).toInt());
	}

	return list;
}
