#include "SearchAll.h"
#include "Window.h"
#include "core/field/FieldArchive.h"
#include "core/Config.h"

SearchAll::SearchAll(Window *parent) :
	QDialog(parent, Qt::Tool), _fieldArchive(0)
{
	resize(parent->width(), height());
	_resultList = new QTreeWidget(this);
	_resultList->setAlternatingRowColors(true);
	_resultList->setItemsExpandable(true);
	_resultList->setSortingEnabled(true);
	_resultList->setAutoScroll(false);
	_resultList->setFrameShape(QFrame::NoFrame);
	_resultList->setExpandsOnDoubleClick(false);
	_resultList->setContextMenuPolicy(Qt::ActionsContextMenu);
	_resultList->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_resultList, 0, 0);
	layout->setContentsMargins(QMargins());

	connect(_resultList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(gotoResult(QTreeWidgetItem*)));

	QAction *copy = new QAction(QIcon(":/images/copy.png"), tr("Copier"), this);
	copy->setShortcut(QKeySequence("Ctrl+C"));
	copy->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(copy, SIGNAL(triggered()), SLOT(copySelected()));

	_resultList->addAction(copy);

	setScriptSearch();
}

void SearchAll::setScriptSearch()
{
	_resultList->clear();
	_resultList->setColumnCount(4);
	_resultList->setHeaderLabels(QStringList() << tr("Groupe") << tr("Script") << tr("Ligne") << tr("Commande"));
	_searchMode = ScriptSearch;
}

void SearchAll::setTextSearch()
{
	_resultList->clear();
	_resultList->setColumnCount(2);
	_resultList->setHeaderLabels(QStringList() << tr("Texte n°") << tr("Texte"));
	_searchMode = TextSearch;
}

void SearchAll::setFieldArchive(FieldArchive *fieldArchive)
{
	_fieldArchive = fieldArchive;
	setEnabled(fieldArchive);
	if(!fieldArchive) {
		_resultList->clear();
	}
}

void SearchAll::addResultOpcode(int fieldID, int grpScriptID, int scriptID, int opcodeID)
{
	addResult(fieldID, createItemOpcode(fieldID, grpScriptID, scriptID, opcodeID));
}

void SearchAll::addResultText(int fieldID, int textID, int index, int size)
{
	addResult(fieldID, createItemText(fieldID, textID, index, size));
}

void SearchAll::addResult(int fieldID, QTreeWidgetItem *item)
{
	QTreeWidgetItem *itemToAdd = itemByFieldID.value(fieldID);
	if (!itemToAdd) {
		itemToAdd = createItemField(fieldID);
		itemByFieldID.insert(fieldID, itemToAdd);
		_resultList->addTopLevelItem(itemToAdd);
		itemToAdd->setExpanded(true);
	}
	itemToAdd->addChild(item);
}

QTreeWidgetItem *SearchAll::createItemField(int fieldID) const
{
	QTreeWidgetItem *item = new QTreeWidgetItem(
								QStringList()
								<< QString("%1").arg(fieldID, 3));

	item->setData(0, Qt::UserRole, fieldID);
	if(_fieldArchive) {
		Field *f = _fieldArchive->field(fieldID);
		if(f) {
			item->setText(0, QString("%1 : %2").arg(fieldID, 3).arg(f->name()));
		}
	}

	for(int col = 0 ; col < _resultList->columnCount() ; ++col) {
		item->setBackground(col, Qt::gray);
		item->setForeground(col, Qt::white);
	}

	return item;
}

QTreeWidgetItem *SearchAll::createItemOpcode(int fieldID, int grpScriptID, int scriptID, int opcodeID) const
{
	QTreeWidgetItem *item = new QTreeWidgetItem(
								QStringList()
								<< QString("%1").arg(grpScriptID, 3)
								<< QString("%1").arg(scriptID, 2)
								<< QString("%1").arg(opcodeID + 1, 5));

	item->setData(0, Qt::UserRole, fieldID);
	item->setData(1, Qt::UserRole, grpScriptID);
	item->setData(2, Qt::UserRole, scriptID);
	item->setData(3, Qt::UserRole, opcodeID);

	if(_fieldArchive) {
		Field *f = _fieldArchive->field(fieldID);
		if(f) {
			GrpScript *grp = f->scriptsAndTexts()->grpScripts().value(grpScriptID);
			if(grp) {
				item->setText(0, QString("%1 : %2").arg(grpScriptID, 3).arg(grp->name()));
				item->setText(1, grp->scriptName(scriptID));
				item->setText(3, grp->script(scriptID)->opcode(opcodeID)->toString(f));
			}
		}
	}

	return item;
}

QTreeWidgetItem *SearchAll::createItemText(int fieldID, int textID, int index, int size) const
{
	Q_UNUSED(size)
	QTreeWidgetItem *item = new QTreeWidgetItem(
								QStringList()
								<< QString("%1").arg(textID, 3)
								<< QString("%1").arg(index, 3));

	item->setData(0, Qt::UserRole, fieldID);
	item->setData(1, Qt::UserRole, textID);
	item->setData(2, Qt::UserRole, index);

	if(_fieldArchive) {
		Field *f = _fieldArchive->field(fieldID);
		if(f) {
			const FF7Text &text = f->scriptsAndTexts()->text(textID);
			item->setText(1, text.text(Config::value("jp_txt", false).toBool(), true));
		}
	}

	return item;
}

void SearchAll::gotoResult(QTreeWidgetItem *item)
{
	int fieldID = item->data(0, Qt::UserRole).toInt();

	if (_searchMode == ScriptSearch) {
		int grpScriptID = item->data(1, Qt::UserRole).toInt(),
				scriptID = item->data(2, Qt::UserRole).toInt(),
				opcodeID = item->data(3, Qt::UserRole).toInt();
		mainWindow()->gotoOpcode(fieldID, grpScriptID, scriptID, opcodeID);

	} else {
		int textID = item->data(1, Qt::UserRole).toInt(),
				index = item->data(2, Qt::UserRole).toInt(),
				size = item->data(3, Qt::UserRole).toInt();
		mainWindow()->gotoText(fieldID, textID, index, size);
	}
}

void SearchAll::copySelected() const
{
	QList<QTreeWidgetItem *> items = _resultList->selectedItems();

	QString str;
	foreach (QTreeWidgetItem *item, items) {
		QStringList cols;
		for (int col = 0 ; col < item->columnCount() ; ++col) {
			cols.append(item->text(col).trimmed());
		}
		str.append(cols.join("\t") + "\n");
	}

	QApplication::clipboard()->setText(str);
}
