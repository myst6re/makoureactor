#include "FieldList.h"
#include "Data.h"
#include "core/field/FieldPC.h"

FieldList::FieldList(QWidget *parent) :
      QTreeWidget(parent), _fieldArchive(nullptr)
{
	qreal scale = qApp->desktop()->logicalDpiX() / 96.0;
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

	connect(_lineSearch, SIGNAL(textEdited(QString)), SLOT(filterMap(QString)));
	connect(_lineSearch, SIGNAL(returnPressed()), SLOT(filterMap()));

	QAction *rename_A = new QAction(tr("Rename field"), this);
	rename_A->setShortcut(QKeySequence("F2"));
	rename_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *add_A = new QAction(QIcon(":/images/plus.png"), tr("Add field"), this);
	add_A->setShortcut(QKeySequence("Ctrl++"));
	add_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *del_A = new QAction(QIcon(":/images/minus.png"), tr("Delete field"), this);
	del_A->setShortcut(QKeySequence(Qt::Key_Delete));
	del_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);

	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(rename(QTreeWidgetItem *, int)));
	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(evidence(QTreeWidgetItem*,QTreeWidgetItem*)));

	connect(rename_A, SIGNAL(triggered()), SLOT(rename()));
	connect(add_A, SIGNAL(triggered()), SLOT(add()));
	connect(del_A, SIGNAL(triggered()), SLOT(del()));

	this->addAction(rename_A);
	this->addAction(add_A);
	this->addAction(del_A);

	_toolBar = new QToolBar(tr("&Field List Toolbar"));
	_toolBar->setIconSize(QSize(14*scale,14*scale));
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
	int index = Data::maplist().indexOf(name);
	if (index != -1) {
		id = QString("%1").arg(index, 3);
	} else {
		id = "  ~";
	}

	QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << name << id);
	item->setData(0, Qt::UserRole, mapID);

	if (!f->io() || index == -1) {
		item->setForeground(0, Qt::darkGray);
		item->setForeground(1, Qt::darkGray);
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
	editItem(item, 0);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(renameOK(QTreeWidgetItem *, int)));
}

void FieldList::renameOK(QTreeWidgetItem *item, int column)
{
	if (column != 1) {
		return;
	}

	disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(renameOK(QTreeWidgetItem *, int)));
	QString newName = item->text(1).left(8);

	if (newName.isEmpty()) {
		QMessageBox::warning(this, tr("Name not filled"),
		                     tr("Please set a new field name."));
		return;
	} else if (Data::maplist().contains(newName)) {
		QMessageBox::warning(this,
		                     tr("Name already present in archive"),
		                     tr("Please choose another name."));
		return;
	}

	item->setText(1, newName);

	Field *f = _fieldArchive->field(currentMapId());
	if (f) {
		f->setName(newName);
		InfFile *inf = f->inf();
		if (inf != nullptr) {
			inf->setMapName(newName);
		}
	}

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
	while (newName.isEmpty() || Data::maplist().contains(newName)){
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
		} else if (Data::maplist().contains(newName)) {
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

	for (QTreeWidgetItem *item : qAsConst(selected)) {
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
