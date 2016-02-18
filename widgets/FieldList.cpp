#include "FieldList.h"
#include "Data.h"

FieldList::FieldList(QWidget *parent) :
    QTreeWidget(parent)
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
	resizeColumnToContents(0);
	setFont(font);
	sortByColumn(1, Qt::AscendingOrder);

	_lineSearch = new QLineEdit(parent);
	_lineSearch->setStatusTip(tr("Quick search"));
	_lineSearch->setPlaceholderText(tr("Search..."));

	connect(_lineSearch, SIGNAL(textEdited(QString)), SLOT(filterMap(QString)));
	connect(_lineSearch, SIGNAL(returnPressed()), SLOT(filterMap()));
}

void FieldList::setEnabled(bool enabled)
{
	QTreeWidget::setEnabled(enabled);
	_lineSearch->setEnabled(enabled);
}

void FieldList::fill(FieldArchive *fieldArchive)
{
	QList<QTreeWidgetItem *> items;

	for(int fieldID = 0 ; fieldID < fieldArchive->size() ; ++fieldID) {
		Field *f = fieldArchive->field(fieldID, false);
		if(f) {
			const QString &name = f->name();
			QString id;
			int index = Data::field_names.indexOf(name);
			if(index != -1) {
				id = QString("%1").arg(index, 3);
			} else {
				id = "~";
			}

			QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << name << id);
			item->setData(0, Qt::UserRole, fieldID);
			items.append(item);
		}
	}

	if(!items.isEmpty()) {
		addTopLevelItems(items);
		resizeColumnToContents(0);
	}
}

int FieldList::currentFieldId() const
{
	QList<QTreeWidgetItem *> selectedItems = this->selectedItems();
	if(selectedItems.isEmpty()) {
		return -1;
	}

	return selectedItems.first()->data(0, Qt::UserRole).toInt();
}

FieldArchive::Sorting FieldList::getFieldSorting()
{
	switch(sortColumn()) {
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
	if(!items.isEmpty()) {
		scrollToItem(items.first(), QAbstractItemView::PositionAtTop);
	}
}
