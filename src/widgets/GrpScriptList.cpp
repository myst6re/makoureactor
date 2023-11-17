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
#include "GrpScriptList.h"
#include "Data.h"
#include "core/field/Section1File.h"
#include <FF7Char>

GrpScriptList::GrpScriptList(QWidget *parent) :
    QTreeWidget(parent), _scripts(nullptr)
{
	setColumnCount(3);
	setHeaderLabels(QStringList() << tr("Id") << tr("Group") << tr("Type"));
	setIndentation(0);
	setItemsExpandable(false);
	setSortingEnabled(true);
	setColumnWidth(0, fontMetrics().horizontalAdvance("8888"));
	setColumnWidth(1, fontMetrics().horizontalAdvance("WWWWWWWW"));
	setColumnWidth(2, 0);
	setContextMenuPolicy(Qt::ActionsContextMenu);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	sortByColumn(0, Qt::AscendingOrder);

	QAction *rename_A = new QAction(QIcon::fromTheme(QStringLiteral("document-edit")), tr("Rename group"), this);
	rename_A->setShortcut(QKeySequence("F2"));
	rename_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *add_A = new QAction(QIcon::fromTheme(QStringLiteral("list-add")), tr("Add group"), this);
	add_A->setShortcut(QKeySequence("Ctrl++"));
	add_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *del_A = new QAction(QIcon::fromTheme(QStringLiteral("list-remove")), tr("Delete group"), this);
	del_A->setShortcut(QKeySequence(Qt::Key_Delete));
	del_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	del_A->setEnabled(false);
	QAction *cut_A = new QAction(QIcon::fromTheme(QStringLiteral("edit-cut")), tr("Cut group"), this);
	cut_A->setShortcut(QKeySequence("Ctrl+X"));
	cut_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	cut_A->setEnabled(false);
	QAction *copy_A = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), tr("Copy group"), this);
	copy_A->setShortcut(QKeySequence("Ctrl+C"));
	copy_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	copy_A->setEnabled(false);
	QAction *paste_A = new QAction(QIcon::fromTheme(QStringLiteral("edit-paste")), tr("Paste group"), this);
	paste_A->setShortcut(QKeySequence("Ctrl+V"));
	paste_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	paste_A->setEnabled(false);
	QAction *up_A = new QAction(QIcon::fromTheme(QStringLiteral("go-up")), tr("Move up"), this);
	up_A->setShortcut(QKeySequence("Shift+Up"));
	up_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	up_A->setEnabled(false);
	QAction *down_A = new QAction(QIcon::fromTheme(QStringLiteral("go-down")), tr("Move down"), this);
	down_A->setShortcut(QKeySequence("Shift+Down"));
	down_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	down_A->setEnabled(false);

	connect(this, &GrpScriptList::itemDoubleClicked, this, qOverload<QTreeWidgetItem *, int>(&GrpScriptList::rename));
	connect(this, &GrpScriptList::currentItemChanged, this, &GrpScriptList::evidence);
	connect(this, &GrpScriptList::itemSelectionChanged, this, &GrpScriptList::upDownEnabled);

	connect(rename_A, &QAction::triggered, this, qOverload<>(&GrpScriptList::rename));
	connect(add_A, &QAction::triggered, this, &GrpScriptList::add);
	connect(del_A, &QAction::triggered, this, &GrpScriptList::del);
	connect(cut_A, &QAction::triggered, this, &GrpScriptList::cut);
	connect(copy_A, &QAction::triggered, this, &GrpScriptList::copy);
	connect(paste_A, &QAction::triggered, this, &GrpScriptList::paste);
	connect(up_A, &QAction::triggered, this, &GrpScriptList::up);
	connect(down_A, &QAction::triggered, this, &GrpScriptList::down);

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
	_toolBar->setIconSize(QSize(14, 14));
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
	if (grpScriptID >= 0 && grpScriptID < _scripts->grpScriptCount()) {
		return &_scripts->grpScript(grpScriptID);
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

void GrpScriptList::clear()
{
	QTreeWidget::clear();
	_scripts = nullptr;
}

void GrpScriptList::fill(Section1File *scripts)
{
	_scripts = scripts;
	fill();
}

void GrpScriptList::fill()
{
	QTreeWidget::clear();

	if (_scripts == nullptr) {
		return;
	}

	int i = 0;
	for (const GrpScript &grpScript : _scripts->grpScripts()) {
		QTreeWidgetItem *item = new QTreeWidgetItem(this, QStringList() << QString("%1").arg(i++, 3) << grpScript.name() << grpScript.typeString());
		item->setForeground(2, QBrush(grpScript.typeColor()));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		item->setToolTip(0, grpScript.typeString());
		QIcon icon;

		if (grpScript.character() >= 0 && grpScript.character() < 9) {
			icon = FF7Char::icon(grpScript.character());
		} else {
			switch (grpScript.type()) {
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
	if (_scripts == nullptr) {
		return;
	}

	int grpScriptID = selectedID();
	QTreeWidgetItem *currentItem = this->currentItem();
	if (grpScriptID >= 0 && grpScriptID < _scripts->grpScriptCount() && currentItem != nullptr) {
		const GrpScript &currentGrpScript = _scripts->grpScript(grpScriptID);
		currentItem->setText(2, currentGrpScript.typeString());
		currentItem->setForeground(2, currentGrpScript.typeColor());

		updateHelpWidget();
	}
}

void GrpScriptList::updateHelpWidget()
{
	if (_scripts == nullptr) {
		return;
	}

	_helpWidget->hide();
	QStringList texts;

	if (_scripts->modelCount() > 16) {
		_helpWidget->show();
		texts.append(tr("You have more than 16 models in this field, "
		                                  "the game may crash."));
	}

	if (_scripts->grpScriptCount() > 48) {
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
	editItem(item, column);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	connect(this, &GrpScriptList::itemChanged, this, &GrpScriptList::renameOK);
}

void GrpScriptList::renameOK(QTreeWidgetItem *item, int column)
{
	if (column != 1 || _scripts == nullptr) {
		return;
	}
	disconnect(this, &GrpScriptList::itemChanged, this, &GrpScriptList::renameOK);
	QString newName = item->text(column).left(8);
	if (newName != item->text(column)) {
		item->setText(column, newName);
	}

	int groupID = item->text(0).toInt();
	if (groupID >= 0 && _scripts->grpScript(groupID).name() != newName) {
		_scripts->grpScript(groupID).setName(newName);
		emit changed();
	}
}

void GrpScriptList::add()
{
	if (_scripts == nullptr || topLevelItemCount() > _scripts->maxGrpScriptCount()) {
		return;
	}

	int grpScriptID = selectedID() + 1;

	_scripts->insertGrpScript(grpScriptID, GrpScript());
	fill();
	scroll(grpScriptID);
	emit changed();
	rename();
}

void GrpScriptList::del(bool totalDel)
{
	if (_scripts == nullptr || topLevelItemCount() == 0) {
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
	for (int i = selectedIDs.size() - 1; i >= 0; --i) {
		_scripts->removeGrpScript(selectedIDs.at(i));
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
	if (_scripts == nullptr) {
		return;
	}
	QList<int> selectedIDs = this->selectedIDs();
	if (selectedIDs.isEmpty()) {
		return;
	}

	clearCopiedGroups();
	for (const int id : selectedIDs) {
		_grpScriptCopied.append(_scripts->grpScript(id));
	}

	actions().at(PasteAction)->setEnabled(true);
}

void GrpScriptList::paste()
{
	if (_scripts == nullptr || _grpScriptCopied.isEmpty()) {
		return;
	}
	int grpScriptID = selectedID() + 1;
	if (grpScriptID == 0) {
		grpScriptID = topLevelItemCount(); // Last position
	}
	int i = grpScriptID;
	for (const GrpScript &GScopied : std::as_const(_grpScriptCopied)) {
		_scripts->insertGrpScript(i++, GScopied);
	}

	fill();
	scroll(grpScriptID);
	emit changed();
}

void GrpScriptList::clearCopiedGroups()
{
	actions().at(PasteAction)->setEnabled(false);
	_grpScriptCopied.clear();
}

void GrpScriptList::move(bool direction)
{
	if (_scripts == nullptr) {
		return;
	}
	int grpScriptID = selectedID();
	if (grpScriptID == -1) {
		return;
	}
	if (_scripts->moveGrpScript(grpScriptID, direction)) {
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
	QList<QTreeWidgetItem *> selItems = selectedItems();

	for (const QTreeWidgetItem *item : selItems) {
		list.append(item->text(0).toInt());
	}

	return list;
}
