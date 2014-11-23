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
	_resultList->setIndentation(0);
	_resultList->setItemsExpandable(false);
	_resultList->setSortingEnabled(true);
	_resultList->setAutoScroll(false);
	_resultList->setFrameShape(QFrame::NoFrame);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_resultList, 0, 0);
	layout->setContentsMargins(QMargins());

	connect(_resultList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(gotoResult(QTreeWidgetItem*)));

	setScriptSearch();
}

void SearchAll::setScriptSearch()
{
	_resultList->clear();
	_resultList->setColumnCount(5);
	_resultList->setHeaderLabels(QStringList() << tr("Écran") << tr("Groupe") << tr("Script") << tr("Ligne") << tr("Commande"));
	_searchMode = ScriptSearch;
}

void SearchAll::setTextSearch()
{
	_resultList->clear();
	_resultList->setColumnCount(3);
	_resultList->setHeaderLabels(QStringList() << tr("Écran") << tr("N°") << tr("Texte"));
	_searchMode = TextSearch;
}

void SearchAll::setFieldArchive(FieldArchive *fieldArchive)
{
	_fieldArchive = fieldArchive;
}

void SearchAll::addResultOpcode(int fieldID, int grpScriptID, int scriptID, int opcodeID)
{
	_resultList->addTopLevelItem(createItemOpcode(fieldID, grpScriptID, scriptID, opcodeID));
}

void SearchAll::addResultText(int fieldID, int textID, int index, int size)
{
	_resultList->addTopLevelItem(createItemText(fieldID, textID, index, size));
}

QTreeWidgetItem *SearchAll::createItemOpcode(int fieldID, int grpScriptID, int scriptID, int opcodeID) const
{
	QTreeWidgetItem *item = new QTreeWidgetItem(
								QStringList()
								<< QString("%1").arg(fieldID, 3)
								<< QString("%1").arg(grpScriptID, 3)
								<< QString("%1").arg(scriptID, 2)
								<< QString("%1").arg(opcodeID, 5));

	item->setData(0, Qt::UserRole, fieldID);
	item->setData(1, Qt::UserRole, grpScriptID);
	item->setData(2, Qt::UserRole, scriptID);
	item->setData(3, Qt::UserRole, opcodeID);

	if(_fieldArchive) {
		Field *f = _fieldArchive->field(fieldID);
		if(f) {
			item->setText(0, QString("%1 : %2").arg(fieldID, 3).arg(f->name()));
			GrpScript *grp = f->scriptsAndTexts()->grpScripts().value(grpScriptID);
			if(grp) {
				item->setText(1, QString("%1 : %2").arg(grpScriptID, 3).arg(grp->name()));
				item->setText(2, grp->scriptName(scriptID));
				item->setText(4, grp->script(scriptID)->opcode(opcodeID)->toString(f));
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
								<< QString("%1").arg(fieldID, 3)
								<< QString("%1").arg(textID, 3)
								<< QString("%1").arg(index, 3));

	item->setData(0, Qt::UserRole, fieldID);
	item->setData(1, Qt::UserRole, textID);
	item->setData(2, Qt::UserRole, index);

	if(_fieldArchive) {
		Field *f = _fieldArchive->field(fieldID);
		if(f) {
			item->setText(0, QString("%1 : %2").arg(fieldID, 3).arg(f->name()));
			const FF7Text &text = f->scriptsAndTexts()->text(textID);
			item->setText(2, text.text(Config::value("jp_txt", false).toBool(), true));
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
