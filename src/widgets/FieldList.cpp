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
#include "FieldList.h"
#include "Data.h"
#include "core/field/FieldPC.h"

FieldList::FieldList(QWidget *parent) :
      QTreeWidget(parent), _fieldArchive(nullptr)
{
	QFont font;
	font.setPointSize(8);

	setColumnCount(2);
	setHeaderLabels(QStringList() << tr("File") << tr("Id"));
	setMinimumHeight(120);
	setIndentation(0);
	setItemsExpandable(false);
	setSortingEnabled(true);
	setAutoScroll(false);
	setColumnWidth(1, 0);
	//setFont(font);
	sortByColumn(1, Qt::AscendingOrder);

	_lineSearch = new QLineEdit(parent);
	_lineSearch->setStatusTip(tr("Quick search"));
	_lineSearch->setPlaceholderText(tr("Search..."));

	connect(_lineSearch, &QLineEdit::textEdited, this, &FieldList::filterMap);
	connect(_lineSearch, &QLineEdit::returnPressed, this, [&] { filterMap(_lineSearch->text()); });

	QAction *rename_A = new QAction(QIcon::fromTheme(QStringLiteral("document-edit")), tr("Rename field"), this);
	rename_A->setShortcut(QKeySequence("F2"));
	rename_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *add_A = new QAction(QIcon::fromTheme(QStringLiteral("list-add")), tr("Add field"), this);
	add_A->setShortcut(QKeySequence("Ctrl++"));
	add_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *del_A = new QAction(QIcon::fromTheme(QStringLiteral("list-remove")), tr("Delete field"), this);
	del_A->setShortcut(QKeySequence(Qt::Key_Delete));
	del_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);

	connect(this, &FieldList::itemDoubleClicked, this, qOverload<QTreeWidgetItem*,int>(&FieldList::rename));
	connect(this, &QTreeWidget::currentItemChanged, this, &FieldList::evidence);

	connect(rename_A, &QAction::triggered, this, qOverload<>(&FieldList::rename));
	connect(add_A, &QAction::triggered, this, &FieldList::add);
	connect(del_A, &QAction::triggered, this, &FieldList::del);

	this->addAction(rename_A);
	this->addAction(add_A);
	this->addAction(del_A);

	_toolBar = new QToolBar(tr("&Field List Toolbar"));
	_toolBar->setIconSize(QSize(14, 14));
	_toolBar->addAction(add_A);
	add_A->setStatusTip(tr("Add a field"));
	_toolBar->addAction(del_A);
	del_A->setStatusTip(tr("Remove a field"));

	enableActions(false);
}

void FieldList::evidence(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	if (current) {
		QColor color = Data::color(Data::ColorEvidence);
		current->setBackground(0, color);
		current->setBackground(1, color);
	}

	if (previous) {
		previous->setBackground(0, QBrush());
		previous->setBackground(1, QBrush());
	}
}

void FieldList::setEnabled(bool enabled)
{
	QTreeWidget::setEnabled(enabled);
	_lineSearch->setEnabled(enabled);
	enableActions(enabled);
}

void FieldList::enableActions(bool enabled)
{
	_toolBar->setEnabled(enabled);
	setContextMenuPolicy(enabled ? Qt::ActionsContextMenu : Qt::NoContextMenu);
}

QTreeWidgetItem *FieldList::createItem(Field *f, int mapID)
{
	const QString &name = f->name();
	QString id;
	if (mapID < 1200) {
		id = QString("%1").arg(mapID, 3);
	} else {
		id = "  ~";
	}

	QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << name << id);
	item->setData(0, Qt::UserRole, mapID);

	if (!f->io() || mapID >= 1200) {
		item->setForeground(0, Data::color(Data::ColorDisabledForeground));
		item->setForeground(1, Data::color(Data::ColorDisabledForeground));
	}

	return item;
}

void FieldList::adjustWidth()
{
	resizeColumnToContents(0);
}

void FieldList::fill(FieldArchive *fieldArchive)
{
	QList<QTreeWidgetItem *> items;
	FieldArchiveIterator it(*fieldArchive);

	while (it.hasNext()) {
		Field *f = it.next(false);
		if (f) {
			items.append(createItem(f, it.mapId()));
		}
	}

	_fieldArchive = fieldArchive;

	setEnabled(true);
	enableActions(_fieldArchive->isPC());

	if (!items.isEmpty()) {
		addTopLevelItems(items);
		adjustWidth();
	}
}

int FieldList::currentMapId() const
{
	QList<QTreeWidgetItem *> selectedItems = this->selectedItems();
	if (selectedItems.isEmpty()) {
		return -1;
	}

	return selectedItems.first()->data(0, Qt::UserRole).toInt();
}

FieldArchive::Sorting FieldList::getFieldSorting()
{
	switch (sortColumn()) {
	case 0:
		return FieldArchive::SortByName;
	default:
		return FieldArchive::SortByMapId;
	}
}

void FieldList::filterMap(const QString &name)
{
	QList<QTreeWidgetItem *> items = findItems(name.isNull() ? _lineSearch->text() : name,
	                                           Qt::MatchStartsWith);
	if (!items.isEmpty()) {
		scrollToItem(items.first(), QAbstractItemView::PositionAtTop);
	}
}

void FieldList::rename(QTreeWidgetItem *item, int column)
{
	if (item == nullptr || column != 0) {
		return;
	}

	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
	editItem(item, column);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	connect(this, &FieldList::itemChanged, this, &FieldList::renameOK);
}

void FieldList::renameOK(QTreeWidgetItem *item, int column)
{
	if (column != 0) {
		return;
	}

	disconnect(this, &FieldList::itemChanged, this, &FieldList::renameOK);
	QString newName = item->text(column).left(20);

	Field *f = _fieldArchive->field(item->data(0, Qt::UserRole).toInt(), true, true);
	if (f == nullptr || f->name() == newName) {
		item->setText(column, newName); // Truncate name
		return;
	}

	if (newName.isEmpty()) {
		QMessageBox::warning(this, tr("Name not filled"),
		                     tr("Please set a new field name."));
		item->setText(column, f->name());
		return;
	} else if (_fieldArchive->mapList().mapNames().contains(newName)) {
		QMessageBox::warning(this,
		                     tr("Name already present in archive"),
		                     tr("Please choose another name."));
		item->setText(column, f->name());
		return;
	}

	_fieldArchive->renameField(f, newName);

	item->setText(column, f->name());

	emit changed();
}

void FieldList::add()
{
	if (!_fieldArchive) {
		return;
	}

	if (!_fieldArchive->isPC()) {
		QMessageBox::warning(this, tr("Error"), tr("Not implemented for PS."));
		return;
	}

	QString newName;

	// Name set by user
	while (newName.isEmpty() || _fieldArchive->mapList().mapNames().contains(newName)){
		bool ok;
		newName = QInputDialog::getText(this, tr("Choose a name"),
		                                tr("Field name:"),
		                                QLineEdit::Normal, newName,
		                                &ok);
		if (!ok) {
			return;
		}
		if (newName.isEmpty()) {
			QMessageBox::warning(this, tr("Name not filled"),
			                     tr("Please set a new field name."));
		} else if (_fieldArchive->mapList().mapNames().contains(newName)) {
			QMessageBox::warning(this,
			                     tr("Name already present in archive"),
			                     tr("Please choose another name."));
		}
	}

	FieldPC *field = new FieldPC(newName);
	field->initEmpty();
	int mapID; // ref
	_fieldArchive->addNewField(field, mapID);

	QTreeWidgetItem *item = createItem(field, mapID);
	addTopLevelItem(item);
	adjustWidth();
	setCurrentItem(item);
	scrollToItem(item, QAbstractItemView::PositionAtTop);
	setFocus();
	emit changed();
}

void FieldList::del()
{
	if (!_fieldArchive->isPC()) {
		QMessageBox::warning(this, tr("Error"), tr("Not implemented for PS."));
		return;
	}

	if (topLevelItemCount() == 0) {
		return;
	}

	QList<QTreeWidgetItem *> selected = selectedItems();

	if (selected.isEmpty()) {
		return;
	}

	QString msg = tr("Are you sure you want to remove %1?\n"
	                 "Other maps can refer to it!")
	                  .arg(selected.size() == 1
	                           ? tr("the selected field")
	                           : tr("the selected fields"));
	QMessageBox::StandardButton but = QMessageBox::warning(
	    this, tr("Delete"), msg, QMessageBox::Yes | QMessageBox::Cancel);

	if (but == QMessageBox::Cancel) {
		return;
	}

	int lastIndex = -1;

	blockSignals(true);

	for (QTreeWidgetItem *item : std::as_const(selected)) {
		int mapID = item->data(0, Qt::UserRole).toInt();
		_fieldArchive->delField(mapID);
		lastIndex = qMax(lastIndex, indexOfTopLevelItem(item));
		delete item;
	}

	blockSignals(false);

	emit fieldDeleted();

	QTreeWidgetItem *itemToFocus = topLevelItem(lastIndex);

	if (itemToFocus != nullptr) {
		adjustWidth();
		setCurrentItem(itemToFocus);
		scrollToItem(itemToFocus, QAbstractItemView::PositionAtTop);
		setFocus();
	}
}
