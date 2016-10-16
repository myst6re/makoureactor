#include "FieldList.h"
#include "Data.h"
#include "core/field/FieldPC.h"

FieldList::FieldList(QWidget *parent) :
    QTreeWidget(parent), _fieldArchive(0)
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
	setFont(font);
	sortByColumn(1, Qt::AscendingOrder);

	_lineSearch = new QLineEdit(parent);
	_lineSearch->setStatusTip(tr("Quick search"));
	_lineSearch->setPlaceholderText(tr("Search..."));

	connect(_lineSearch, SIGNAL(textEdited(QString)), SLOT(filterMap(QString)));
	connect(_lineSearch, SIGNAL(returnPressed()), SLOT(filterMap()));

	QAction *add_A = new QAction(QIcon(":/images/plus.png"), tr("Add field"), this);
	add_A->setShortcut(QKeySequence("Ctrl++"));
	add_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	QAction *del_A = new QAction(QIcon(":/images/minus.png"), tr("Delete field"), this);
	del_A->setShortcut(QKeySequence(Qt::Key_Delete));
	del_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);

	_toolBar = new QToolBar(tr("&Field List Toolbar"));
	_toolBar->setIconSize(QSize(14*scale,14*scale));
	_toolBar->addAction(add_A);
	add_A->setStatusTip(tr("Add a field"));
	_toolBar->addAction(del_A);
	del_A->setStatusTip(tr("Remove a field"));

	connect(add_A, SIGNAL(triggered()), SLOT(add()));
	connect(del_A, SIGNAL(triggered()), SLOT(del()));

	setMinimumWidth(_toolBar->sizeHint().width());
}

void FieldList::setEnabled(bool enabled)
{
	QTreeWidget::setEnabled(enabled);
	_lineSearch->setEnabled(enabled);
	_toolBar->setEnabled(enabled);
}

QTreeWidgetItem *FieldList::createItem(Field *f, int fieldID)
{
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
	return item;
}

void FieldList::adjustWidth()
{
	resizeColumnToContents(0);

	setMinimumWidth(columnWidth(0) +
	                columnWidth(1));
}

void FieldList::fill(FieldArchive *fieldArchive)
{
	QList<QTreeWidgetItem *> items;

	for(int fieldID = 0 ; fieldID < fieldArchive->size() ; ++fieldID) {
		Field *f = fieldArchive->field(fieldID, false);
		if(f) {
			items.append(createItem(f, fieldID));
		}
	}

	_fieldArchive = fieldArchive;

	// TODO: not implemented
	if(_fieldArchive->isPS()) {
		_toolBar->setEnabled(false);
	}

	if(!items.isEmpty()) {
		addTopLevelItems(items);
		adjustWidth();
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

void FieldList::add()
{
	if(!_fieldArchive) {
		return;
	}

	QStringList filter;
	if(_fieldArchive->isPC()) {
		filter.append(tr("PC Field File (*)"));
	} else {
		filter.append(tr("DAT File (*.DAT)"));
	}

	QString filePath = QFileDialog::getOpenFileName(this, tr("Add a field"),
	                                                QString(),
	                                                filter.join(";;"));
	if(filePath.isNull()) {
		return;
	}

	if(_fieldArchive->isPC()) {
		FieldPC *field = new FieldPC("", _fieldArchive->io());
		if(field->open(filePath, false, true) == 0) {
			InfFile *inf = field->inf();
			QString newName;
			if(inf && inf->isOpen()) {
				newName = inf->mapName();
			} else {
				newName = QFileInfo(filePath).baseName();
			}

			// Name set by user
			while(newName.isEmpty() || Data::field_names.contains(newName)){
				bool ok;
				newName = QInputDialog::getText(this, tr("Choose a name"),
				                                tr("Field name:"),
				                                QLineEdit::Normal, newName,
				                                &ok);
				if(!ok) {
					return;
				}
				if(newName.isEmpty()) {
					QMessageBox::warning(this, tr("Name not filled"),
					                     tr("Please set a new field name."));
				} else if(Data::field_names.contains(newName)) {
					QMessageBox::warning(this, tr("Name already present in archive"),
					                     tr("Please choose another name."));
				}
			}

			field->setName(newName);
			int fieldID;
			FieldArchiveIO::ErrorCode err = _fieldArchive->addField(field,
			                                                        filePath,
			                                                        fieldID); // ref
			switch(err){
			case FieldArchiveIO::NotImplemented:
				QMessageBox::warning(this, tr("Error"), tr("Not implemented."));
				break;
			case FieldArchiveIO::FieldExists:
				QMessageBox::warning(this, tr("Error"), tr("Field with this name already exist."));
				break;
			case FieldArchiveIO::Ok:
				break;
			default:
				QMessageBox::warning(this, tr("Error"), tr("Unknown error."));
				break;
			}

			addTopLevelItem(createItem(field, fieldID));
			adjustWidth();
		} else {
			QMessageBox::warning(this, tr("Error"), tr("Cannot open file."));
		}
	} else {
		// TODO
	}
}

void FieldList::del()
{
	
}
