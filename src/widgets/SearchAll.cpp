/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#include "SearchAll.h"
#include "Window.h"
#include "core/field/FieldArchive.h"
#include "core/Config.h"

SearchAll::SearchAll(Window *parent) :
    QDialog(parent, Qt::Tool), _fieldArchive(nullptr)
{
	setWindowTitle(tr("Find All"));

	resize(800, 600);
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

	connect(_resultList, SIGNAL(itemActivated(QTreeWidgetItem*,int)), SLOT(gotoResult(QTreeWidgetItem*)));

	QAction *copy = new QAction(QIcon(":/images/copy.png"), tr("Copy"), this);
	copy->setShortcut(QKeySequence("Ctrl+C"));
	copy->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(copy, SIGNAL(triggered()), SLOT(copySelected()));

	_resultList->addAction(copy);

	setScriptSearch();
}

void SearchAll::clear()
{
	_resultList->clear();
	itemByMapID.clear();
}

void SearchAll::setScriptSearch()
{
	clear();
	_resultList->setColumnCount(4);
	_resultList->setHeaderLabels(QStringList() << tr("Group") << tr("Script") << tr("Lines") << tr("Instruction"));
	_searchMode = ScriptSearch;
}

void SearchAll::setTextSearch()
{
	clear();
	_resultList->setColumnCount(2);
	_resultList->setHeaderLabels(QStringList() << tr("Text #") << tr("Text"));
	_searchMode = TextSearch;
}

void SearchAll::setFieldArchive(FieldArchive *fieldArchive)
{
	_fieldArchive = fieldArchive;
	setEnabled(fieldArchive);
	if(!fieldArchive) {
		clear();
	}
}

void SearchAll::addResultOpcode(int mapID, int grpScriptID, int scriptID, int opcodeID)
{
	addResult(mapID, createItemOpcode(mapID, grpScriptID, scriptID, opcodeID));
}

void SearchAll::addResultText(int mapID, int textID, int index, int size)
{
	addResult(mapID, createItemText(mapID, textID, index, size));
}

void SearchAll::addResult(int mapID, QTreeWidgetItem *item)
{
	QTreeWidgetItem *itemToAdd = itemByMapID.value(mapID);
	if (!itemToAdd) {
		itemToAdd = createItemField(mapID);
		itemByMapID.insert(mapID, itemToAdd);
		_resultList->addTopLevelItem(itemToAdd);
		itemToAdd->setExpanded(true);
	}
	itemToAdd->addChild(item);
}

QTreeWidgetItem *SearchAll::createItemField(int mapID) const
{
	QTreeWidgetItem *item = new QTreeWidgetItem(
								QStringList()
								<< QString("%1").arg(mapID, 3));

	item->setData(0, Qt::UserRole, mapID);
	if(_fieldArchive) {
		Field *f = _fieldArchive->field(quint32(mapID));
		if(f) {
			item->setText(0, QString("%1 : %2")
			                     .arg(mapID, 3)
			                     .arg(f->name()));
		}
	}

	for (int col = 0; col < _resultList->columnCount(); ++col) {
		item->setBackground(col, Qt::gray);
		item->setForeground(col, Qt::white);
	}

	return item;
}

QTreeWidgetItem *SearchAll::createItemOpcode(int mapID, int grpScriptID, int scriptID, int opcodeID) const
{
	QTreeWidgetItem *item = new QTreeWidgetItem(
								QStringList()
								<< QString("%1").arg(grpScriptID, 3)
								<< QString("%1").arg(scriptID, 2)
								<< QString("%1").arg(opcodeID + 1, 5));

	item->setData(0, Qt::UserRole, mapID);
	item->setData(1, Qt::UserRole, grpScriptID);
	item->setData(2, Qt::UserRole, scriptID);
	item->setData(3, Qt::UserRole, opcodeID);

	if(_fieldArchive) {
		Field *f = _fieldArchive->field(quint32(mapID));
		if(f) {
			GrpScript *grp = f->scriptsAndTexts()->grpScripts().value(grpScriptID);
			if(grp) {
				item->setText(0, QString("%1 : %2").arg(grpScriptID, 3).arg(grp->name()));
				item->setText(1, grp->scriptName(quint8(scriptID)));
				item->setText(3, grp->script(quint8(scriptID))->opcode(quint16(opcodeID))->toString(f));
			}
		}
	}

	return item;
}

QTreeWidgetItem *SearchAll::createItemText(int mapID, int textID, int index, int size) const
{
	Q_UNUSED(size)
	QTreeWidgetItem *item = new QTreeWidgetItem(
								QStringList()
								<< QString("%1").arg(textID, 3)
								<< QString("%1").arg(index, 3));

	item->setData(0, Qt::UserRole, mapID);
	item->setData(1, Qt::UserRole, textID);
	item->setData(2, Qt::UserRole, index);

	if(_fieldArchive) {
		Field *f = _fieldArchive->field(quint32(mapID));
		if(f) {
			const FF7Text &text = f->scriptsAndTexts()->text(textID);
			item->setText(1, text.text(Config::value("jp_txt", false).toBool(), true));
		}
	}

	return item;
}

void SearchAll::gotoResult(QTreeWidgetItem *item)
{
	int mapID = item->data(0, Qt::UserRole).toInt();

	if (_searchMode == ScriptSearch) {
		int grpScriptID = item->data(1, Qt::UserRole).toInt(),
				scriptID = item->data(2, Qt::UserRole).toInt(),
				opcodeID = item->data(3, Qt::UserRole).toInt();
		mainWindow()->gotoOpcode(mapID, grpScriptID, scriptID, opcodeID);

	} else {
		int textID = item->data(1, Qt::UserRole).toInt(),
				index = item->data(2, Qt::UserRole).toInt(),
				size = item->data(3, Qt::UserRole).toInt();
		mainWindow()->gotoText(mapID, textID, index, size);
	}
}

void SearchAll::copySelected() const
{
	QList<QTreeWidgetItem *> selectedItems = _resultList->selectedItems();

	QString str;
	QTreeWidgetItemIterator it(_resultList);
	while (*it) {
		if(selectedItems.contains(*it)) {
			QStringList cols;
			for (int col = 0; col < (*it)->columnCount(); ++col) {
				cols.append((*it)->text(col).trimmed());
			}
			str.append(cols.join("\t").trimmed() + "\n");
			if (cols.value(1).isEmpty()) { // Field row
				str.append(QString(cols.value(0).size(), '=') + "\n");
			}
		}
		++it;
	}

	QApplication::clipboard()->setText(str);
}
