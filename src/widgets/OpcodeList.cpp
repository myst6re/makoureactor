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
#include "OpcodeList.h"
#include "ScriptEditor.h"
#include "core/Config.h"
#include "core/Clipboard.h"
#include "Data.h"
#include "core/field/Field.h"
#include "core/field/Opcode.h"

OpcodeList::OpcodeList(QWidget *parent) :
    QTreeWidget(parent), _field(nullptr), _grpScript(nullptr), _script(nullptr), errorLine(-1),
    isInit(false), _treeEnabled(true)
{
	setColumnCount(1);
	setHeaderHidden(true);
	setAutoScroll(false);
	setIconSize(QSize(32, 11));
	setAlternatingRowColors(true);
	setExpandsOnDoubleClick(false);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setTextElideMode(Qt::ElideNone);

	edit_A = new QAction(tr("Edit"), this);
	edit_A->setShortcut(QKeySequence(Qt::Key_Return));
	edit_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	edit_A->setEnabled(false);
	add_A = new QAction(QIcon::fromTheme(QStringLiteral("list-add")), tr("Add"), this);
	add_A->setShortcut(QKeySequence("Ctrl++"));
	add_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	del_A = new QAction(QIcon::fromTheme(QStringLiteral("list-remove")), tr("Delete"), this);
	del_A->setShortcut(QKeySequence(Qt::Key_Delete));
	del_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	del_A->setEnabled(false);
	cut_A = new QAction(QIcon::fromTheme(QStringLiteral("edit-cut")), tr("Cut"), this);
	cut_A->setShortcut(QKeySequence("Ctrl+X"));
	cut_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	cut_A->setEnabled(false);
	copy_A = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), tr("Copy"), this);
	copy_A->setShortcut(QKeySequence("Ctrl+C"));
	copy_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	copy_A->setEnabled(false);
	copyText_A = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), tr("Copy text"), this);
	copyText_A->setShortcut(QKeySequence("Ctrl+Shift+C"));
	copyText_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	copyText_A->setEnabled(false);
	paste_A = new QAction(QIcon::fromTheme(QStringLiteral("edit-paste")), tr("Paste"), this);
	paste_A->setShortcut(QKeySequence("Ctrl+V"));
	paste_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	paste_A->setEnabled(false);
	up_A = new QAction(QIcon::fromTheme(QStringLiteral("go-up")), tr("Up"), this);
	up_A->setShortcut(QKeySequence("Shift+Up"));
	up_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	up_A->setEnabled(false);
	down_A = new QAction(QIcon::fromTheme(QStringLiteral("go-down")), tr("Down"), this);
	down_A->setShortcut(QKeySequence("Shift+Down"));
	down_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	down_A->setEnabled(false);
	expand_A = new QAction(tr("Expand the tree"), this);
	undo_A = new QAction(QIcon::fromTheme(QStringLiteral("edit-undo")), tr("Undo"), this);
	undo_A->setShortcut(QKeySequence::Undo);
	undo_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	undo_A->setEnabled(false);
	redo_A = new QAction(QIcon::fromTheme(QStringLiteral("edit-redo")), tr("Redo"), this);
	redo_A->setShortcut(QKeySequence::Redo);
	redo_A->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	redo_A->setEnabled(false);
	text_A = new QAction(tr("Edit text"), this);
	text_A->setVisible(false);
	goto_A = new QAction(this);
	goto_A->setVisible(false);
	disableTree_A = new QAction(tr("Disable tree"), this);
	search_A = new QAction(tr("Search opcode..."), this);

	connect(edit_A, &QAction::triggered, this, &OpcodeList::scriptEditor);
	connect(add_A, &QAction::triggered, this, &OpcodeList::add);
	connect(del_A, &QAction::triggered, this, &OpcodeList::del);
	connect(cut_A, &QAction::triggered, this, &OpcodeList::cut);
	connect(copy_A, &QAction::triggered, this, &OpcodeList::copy);
	connect(copyText_A, &QAction::triggered, this, &OpcodeList::copyText);
	connect(paste_A, &QAction::triggered, this, &OpcodeList::paste);
	connect(up_A, &QAction::triggered, this, &OpcodeList::up);
	connect(down_A, &QAction::triggered, this, &OpcodeList::down);
	connect(expand_A, &QAction::triggered, this, &OpcodeList::expandAll);
	connect(text_A, &QAction::triggered, this, qOverload<>(&OpcodeList::editText));
	connect(undo_A, &QAction::triggered, this, &OpcodeList::undo);
	connect(redo_A, &QAction::triggered, this, &OpcodeList::redo);
	connect(goto_A, &QAction::triggered, this, [&] {gotoLabel(nullptr);} );
	connect(disableTree_A, &QAction::triggered, this, &OpcodeList::toggleTree);
	connect(search_A, &QAction::triggered, this, qOverload<>(&OpcodeList::searchOpcode));

	addAction(edit_A);
	addAction(add_A);
	addAction(del_A);
	addAction(text_A);
	addAction(goto_A);
	addAction(disableTree_A);
	addAction(search_A);
	QAction *separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);
	addAction(cut_A);
	addAction(copy_A);
	addAction(copyText_A);
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
	_help = new QLabel;
	_help->hide();
	QHBoxLayout *helpLayout = new QHBoxLayout(help);
	helpLayout->addStretch();
	helpLayout->addWidget(_help);
	helpLayout->setContentsMargins(QMargins());
	
	_toolBar = new QToolBar(tr("&Script editor"));
	_toolBar->setIconSize(QSize(14, 14));
	_toolBar->setFloatable(false);
	_toolBar->setAllowedAreas(Qt::NoToolBarArea);
	_toolBar->setMovable(false);
	_toolBar->addAction(add_A);
	add_A->setStatusTip(tr("Add line"));
	_toolBar->addAction(del_A);
	del_A->setStatusTip(tr("Remove line"));
	_toolBar->addSeparator();
	_toolBar->addAction(up_A);
	up_A->setStatusTip(tr("Move up"));
	_toolBar->addAction(down_A);
	down_A->setStatusTip(tr("Move down"));
	_toolBar->addSeparator();
	_toolBar->addAction(expand_A);
	_toolBar->addAction(text_A);
	_toolBar->addAction(goto_A);
	_toolBar->addAction(disableTree_A);
	_toolBar->addSeparator();
	_toolBar->addAction(undo_A);
	undo_A->setStatusTip(undo_A->text());
	_toolBar->addAction(redo_A);
	redo_A->setStatusTip(redo_A->text());
	_toolBar->addWidget(help);

	enableActions(false);

	setMinimumWidth(_toolBar->sizeHint().width());
	setMinimumHeight(_toolBar->sizeHint().width());

	connect(this, &OpcodeList::itemDoubleClicked, this, &OpcodeList::scriptEditor);
	connect(this, &OpcodeList::itemSelectionChanged, this, &OpcodeList::itemSelected);
	connect(this, &OpcodeList::currentItemChanged, this, &OpcodeList::evidence);
	connect(QApplication::clipboard(), &QClipboard::changed, this, &OpcodeList::adjustPasteAction);
}

void OpcodeList::adjustPasteAction()
{
	paste_A->setEnabled(Clipboard::instance()->hasFf7FieldScriptOpcodes());
}

void OpcodeList::searchOpcode()
{
	int opcodeID = selectedID();

	if (opcodeID <= -1 || opcodeID >= _script->size()) {
		return;
	}

	emit searchOpcode(_script->opcode(opcodeID).id());
}

void OpcodeList::clear()
{
	enableActions(false);
	QTreeWidget::clear();
	clearHist();
	_field = nullptr;
	_grpScript = nullptr;
	_script = nullptr;
}

void OpcodeList::setEnabled(bool enabled)
{
	_toolBar->setEnabled(enabled);
	QTreeWidget::setEnabled(enabled);
	enableActions(enabled);
}

void OpcodeList::enableActions(bool enabled)
{
	_toolBar->setEnabled(enabled);
	QList<QAction *> acts = actions();
	for (QAction *action : std::as_const(acts)) {
		if (action == undo_A || action == redo_A) {
			continue;
		}
		action->setEnabled(enabled);
	}
	setContextMenuPolicy(enabled ? Qt::ActionsContextMenu : Qt::NoContextMenu);
}

void OpcodeList::setErrorLine(int opcodeID)
{
	QTreeWidgetItem *item;

	item = findItem(errorLine);
	if (item != nullptr) {
		item->setBackground(0, previousErrorBg);
	}

	item = findItem(opcodeID);
	if (item != nullptr) {
		previousErrorBg = item->background(0);
		item->setBackground(0, Data::color(Data::ColorRedBackground));
	}

	errorLine = opcodeID;
}

void OpcodeList::setTreeEnabled(bool enabled)
{
	if (enabled != _treeEnabled) {
		_treeEnabled = enabled;
		if (_treeEnabled) {
			disableTree_A->setText(tr("Disable tree"));
		} else {
			disableTree_A->setText(tr("Enable tree"));
		}
		fill();
	}
}

void OpcodeList::itemSelected()
{
	upDownEnabled();
	int opcodeID = selectedID();
	if (opcodeID <= -1 || opcodeID >= _script->size()) {
		return;
	}

	const Opcode &opcode = _script->opcode(opcodeID);

	switch (opcode.id()) {
	case OpcodeKey::ASK:
	case OpcodeKey::MESSAGE:
	case OpcodeKey::MPNAM:
		text_A->setVisible(true);
		break;
	case OpcodeKey::SPECIAL:
		text_A->setVisible(opcode.op().opcodeSPECIAL.subKey == OpcodeSpecialKey::SPCNM);
		break;
	default:
		text_A->setVisible(false);
		break;
	}

	bool visible = true;
	if (opcode.isJump()) {
		goto_A->setText(tr("Goto label"));
#ifndef Q_OS_MAC
		_help->setText(tr("Alt + Click to go to the label"));
#else
		_help->setText(tr("option + Click to go to the label"));
#endif
	} else if (opcode.isExec()) {
		goto_A->setText(tr("Goto script"));
#ifndef Q_OS_MAC
		_help->setText(tr("Alt + Click to go to the script"));
#else
		_help->setText(tr("option + Click to go to the script"));
#endif
	} else {
		visible = false;
	}
	goto_A->setVisible(visible);
	_help->setVisible(visible);
}

void OpcodeList::upDownEnabled()
{
	if (selectedItems().isEmpty()) {
		edit_A->setEnabled(false);
		del_A->setEnabled(false);
		cut_A->setEnabled(false);
		copy_A->setEnabled(false);
		copyText_A->setEnabled(false);
		up_A->setEnabled(false);
		down_A->setEnabled(false);
	} else {
		edit_A->setEnabled(true);
		del_A->setEnabled(_script && !_script->isEmpty());
		cut_A->setEnabled(true);
		copy_A->setEnabled(true);
		copyText_A->setEnabled(true);
		up_A->setEnabled(/* topLevelItemCount() > 1 && */ currentItem() != topLevelItem(0));
		down_A->setEnabled(true/*  topLevelItemCount() > 1 && currentItem() != topLevelItem(topLevelItemCount()-1) */);
	}
}

void OpcodeList::editText()
{
	if (!_script) {
		return;
	}

	int opcodeID = selectedID();
	if (opcodeID >= 0 && opcodeID < _script->size()) {
		int textID = _script->opcode(opcodeID).textID();
		if (textID >= 0) {
			emit editText(textID);
		}
	}
}

void OpcodeList::saveExpandedItems()
{
	if (!_script) {
		return;
	}

	QList<Opcode> &opcodes = _script->opcodes();
	for (Opcode &opcode: opcodes) {
		if (opcode.isIf()) {
			opcode.setItemIsExpanded(false);
		}
	}

	QQueue<QTreeWidgetItem *> items;
	const int size = topLevelItemCount();
	for (int i = 0; i < size; ++i) {
		QTreeWidgetItem *item = topLevelItem(i);
		if (item->childCount() > 0) {
			items.enqueue(item);
		}
	}

	while (!items.empty()) {
		QTreeWidgetItem *item = items.dequeue();
		const int opcodeID = item->data(0, Qt::UserRole).toInt();
		if (opcodeID >= 0 && opcodeID < _script->size()) {
			Opcode &opcode = _script->opcode(opcodeID);
			
			if (opcode.isIf()) {
				opcode.setItemIsExpanded(item->isExpanded());
			}
		}

		// Add children
		const int size = item->childCount();
		for (int i = 0; i < size; ++i) {
			QTreeWidgetItem *child = item->child(i);
			if (child->childCount() > 0) {
				items.enqueue(child);
			}
		}
	}
}

bool OpcodeList::itemIsExpanded(int opcodeID) const
{
	if (!_script) {
		return false;
	}

	if (Config::value("scriptItemExpandedByDefault", false).toBool()) {
		return true; // FIXME: not the correct behavior
	}

	return _script->opcode(opcodeID).isIf() && _script->opcode(opcodeID).itemIsExpanded();
}

void OpcodeList::refreshOpcode(int opcodeID)
{
	if (_field == nullptr || _field->scriptsAndTexts() == nullptr) {
		return;
	}

	QTreeWidgetItem *item = findItem(opcodeID);

	if (item == nullptr || opcodeID >= _script->size()) {
		return;
	}

	item->setText(0, _script->opcode(opcodeID).toString(_field->scriptsAndTexts()));
}

void OpcodeList::fill(Field *field, const GrpScript *grpScript, Script *script)
{
	saveExpandedItems();
	clearHist();
	_field = field;
	_grpScript = grpScript;
	_script = script;

	fill();

	scrollToTop();
}

void OpcodeList::fill()
{
	previousBG = QBrush();
	blockSignals(true);
	QTreeWidget::clear();
	blockSignals(false);

	if (_field == nullptr || _script == nullptr) {
		return;
	}

	header()->setMinimumSectionSize(0);
	Section1File *scriptsAndTexts = _field->scriptsAndTexts();
	
	if (!_script->isEmpty()) {
		QList<quint32> indent;
		QList<QTreeWidgetItem *> items;
		QTreeWidgetItem *parentItem = nullptr;
		int opcodeID = 0;
		QPixmap fontPixmap(":/images/numbers.png");
		QColor blue = Data::color(Data::ColorBlueForeground),
		       orange = Data::color(Data::ColorOrangeForeground),
		       green = Data::color(Data::ColorGreenForeground),
		       grey = Data::color(Data::ColorGreyForeground),
		       red = Data::color(Data::ColorRedForeground);

		for (const Opcode &curOpcode : _script->opcodes()) {
			OpcodeKey id = curOpcode.id();

			if (id == OpcodeKey::LABEL) {
				while (!indent.isEmpty() &&
				      curOpcode.op().opcodeLABEL._label == indent.last())
				{
					indent.removeLast();
					if (parentItem != nullptr) {
						parentItem = parentItem->parent();
					}
				}
			}

			QTreeWidgetItem *item = new QTreeWidgetItem(parentItem, QStringList(curOpcode.toString(scriptsAndTexts)));
			item->setData(0, Qt::UserRole, opcodeID);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			items.append(item);

			QPixmap wordPixmap(32,11);
			item->setIcon(0, QIcon(posNumber(opcodeID + 1, fontPixmap, wordPixmap)));
			item->setToolTip(0, curOpcode.name() + QString(" ") + QString::number(curOpcode.size()));
			if (curOpcode.isIf()) {
				item->setForeground(0, blue);
				if (curOpcode.badJump() == BadJumpError::Ok) {
					indent.append(quint16(curOpcode.label()));
				}
				if (_treeEnabled) {
					parentItem = item;
				}
			}
			else if (id >= OpcodeKey::REQ && id <= OpcodeKey::RETTO) {
				item->setForeground(0, orange);
			} else if (curOpcode.isJump()) {
				item->setForeground(0, green);
			} else if (id == OpcodeKey::RET) {
				item->setForeground(0, grey);
			} else if (id == OpcodeKey::LABEL) {
				item->setForeground(0, red);
			}

			++opcodeID;
		}

		addTopLevelItems(items);

		// QTreeWidgetItem::setExpanded must be done after addTopLevelItems
		for (QTreeWidgetItem *item : items) {
			if (item->childCount() > 0) {
				int opcodeID = item->data(0, Qt::UserRole).toInt();
				const Opcode &curOpcode = _script->opcode(opcodeID);

				if (curOpcode.isIf()) {
					item->setExpanded(curOpcode.itemIsExpanded());
				}
			}
		}
	} else {
		QTreeWidgetItem *item = new QTreeWidgetItem(this, QStringList(tr("If this script is run,\n assume that the last non-empty script that runs")));
		item->setIcon(0, QIcon::fromTheme(QStringLiteral("dialog-information")));
		item->setFlags(Qt::NoItemFlags);
		item->setData(0, Qt::UserRole, -2);
	}

	// Adjust items size
	resizeColumnToContents(0);
	if (header()->sectionSize(0) < viewport()->width()) {
		header()->setMinimumSectionSize(viewport()->width());
	}

	enableActions(true);

	// edit_A->setEnabled(true);
	add_A->setEnabled(true);
	// del_A->setEnabled(!script->isEmpty());
	// cut_A->setEnabled(true);
	// copy_A->setEnabled(true);
	adjustPasteAction();
	itemSelected();
}

void OpcodeList::evidence(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	QTreeWidgetItem *errorItem = findItem(errorLine);

	if (current && errorItem != current) {
		previousBG = current->background(0);
		current->setBackground(0, Data::color(Data::ColorEvidence));
	}
	if (previous && errorItem != previous) {
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
	for (const Historic &h : hists) {
		QString type;
		switch (h.type) {
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
	for (const Historic &h : restoreHists) {
		QString type;
		switch (h.type) {
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

void OpcodeList::changeHist(HistoricType type, int opcodeID, const Opcode &data)
{
	Historic hist;
	hist.type = type;
	hist.opcodeIDs = QList<int>() << opcodeID;
	hist.data = QList<Opcode>() << data;

	undo_A->setEnabled(true);
	redo_A->setEnabled(false);
	hists.push(hist);
	restoreHists.clear();
//	qDebug() << showHistoric();
}

void OpcodeList::changeHist(HistoricType type, const QList<int> &opcodeIDs, const QList<Opcode> &data)
{
	Historic hist;
	hist.type = type;
	hist.opcodeIDs = opcodeIDs;
	hist.data = data;

	undo_A->setEnabled(true);
	redo_A->setEnabled(false);
	hists.push(hist);
	restoreHists.clear();
}

void OpcodeList::clearHist()
{
	undo_A->setEnabled(false);
	redo_A->setEnabled(false);

	hists.clear();
	restoreHists.clear();
}

void OpcodeList::undo()
{
	if (hists.isEmpty()) {
		return;
	}

	Historic hist = hists.pop();
	undo_A->setEnabled(!hists.isEmpty());

	int firstOpcode = hist.opcodeIDs.first();

	switch (hist.type) {
	case Add:
		// del opcodes
		for (qsizetype i = hist.opcodeIDs.size() - 1; i >= 0; --i) {
			hist.data.prepend(_script->opcode(hist.opcodeIDs.at(i)));
			_script->removeOpcode(hist.opcodeIDs.at(i));
		}
		break;
	case Remove:
		// restore opcodes
		for (qsizetype i = 0; i < hist.opcodeIDs.size(); ++i) {
			_script->insertOpcode(hist.opcodeIDs.at(i), hist.data.at(i));
		}
		hist.data.clear();
		break;
	case Modify: {
		// restore old version
		const Opcode &sav = _script->opcode(firstOpcode);
		_script->setOpcode(firstOpcode, hist.data.first());
		hist.data.replace(0, sav);
		} break;
	case ModifyAndAddLabel: {
		// del label
		hist.data.prepend(_script->opcode(firstOpcode + 1));
		_script->removeOpcode(firstOpcode + 1);
		// restore old version
		const Opcode &sav = _script->opcode(firstOpcode);
		_script->setOpcode(firstOpcode, hist.data.first());
		hist.data.replace(0, sav);
		} break;
	case Up:
		// move down
		_script->moveOpcode(firstOpcode - 1, Script::Down);
		break;
	case Down:
		// move up
		_script->moveOpcode(firstOpcode + 1, Script::Up);
		break;
	}

	restoreHists.push(hist);
	redo_A->setEnabled(true);

	fill(); // Refresh view

	emit changed();

	scroll(firstOpcode);
}

void OpcodeList::redo()
{
	if (restoreHists.isEmpty()) {
		return;
	}

	Historic hist = restoreHists.pop();
	redo_A->setEnabled(!restoreHists.isEmpty());

	int firstOpcode = hist.opcodeIDs.first();

	switch (hist.type) {
	case Add:
		for (qsizetype i = 0; i < hist.opcodeIDs.size(); ++i) {
			_script->insertOpcode(hist.opcodeIDs.at(i), hist.data.at(i));
		}
		hist.data.clear();
		break;
	case Remove:
		for (qsizetype i = hist.opcodeIDs.size() - 1; i >= 0; --i) {
			hist.data.prepend(_script->opcode(hist.opcodeIDs.at(i)));
			_script->removeOpcode(hist.opcodeIDs.at(i));
		}
		break;
	case Modify: {
		const Opcode &sav = _script->opcode(firstOpcode);
		_script->setOpcode(firstOpcode, hist.data.first());
		hist.data.replace(0, sav);
		} break;
	case ModifyAndAddLabel: {
		const Opcode &sav = _script->opcode(firstOpcode);
		_script->setOpcode(firstOpcode, hist.data.first());
		hist.data.replace(0, sav);

		_script->insertOpcode(hist.opcodeIDs.at(1), hist.data.at(1));
		hist.data.removeAt(1);
		} break;
	case Up:
		_script->moveOpcode(--firstOpcode, Script::Down);
		break;
	case Down:
		_script->moveOpcode(++firstOpcode, Script::Up);
		break;
	}

	hists.push(hist);
	undo_A->setEnabled(true);

	fill(); // Refresh view

	emit changed();

	scroll(firstOpcode);
}

void OpcodeList::scriptEditor(bool modify)
{
	if (!_script) {
		return;
	}

	int opcodeID = selectedID();
	if (opcodeID == -1) {
		modify = false;
	}

	Opcode oldVersion;

	saveExpandedItems();

	if (modify) {
		oldVersion = _script->opcode(opcodeID);
	} else {
		++opcodeID;
	}

	ScriptEditor editor(_field, _field->scriptsAndTexts(), *_grpScript, *_script, opcodeID, modify, isInit, this);

	if (editor.exec() == QDialog::Accepted) {
		Opcode opcode = editor.buildOpcode();
		
		if (editor.needslabel()) {
			OpcodeLABEL label;
			label._label = quint16(opcode.label());
			_script->insertOpcode(opcodeID, label);
		}
	
		if (modify) {
			_script->setOpcode(opcodeID, opcode);
		} else {
			_script->insertOpcode(opcodeID, opcode);
		}

		fill();
		QTreeWidgetItem *item = findItem(opcodeID);
		if (item != nullptr) {
			setCurrentItem(item);
		}
		if (modify) {
			if (editor.needslabel()) {
				changeHist(ModifyAndAddLabel, opcodeID, oldVersion);
			} else {
				changeHist(Modify, opcodeID, oldVersion);
			}
		}
		else {
			if (item != nullptr) {
				scrollToItem(item, QAbstractItemView::EnsureVisible);
			}
			if (editor.needslabel()) {
				changeHist(Add, QList<int>() << opcodeID << (opcodeID + 1), QList<Opcode>());
			} else {
				changeHist(Add, opcodeID);
			}
		}
		emit changed();
	}
}

void OpcodeList::del(bool totalDel)
{
	if (topLevelItemCount() == 0) {
		return;
	}
	QList<int> selectedIDs = this->selectedIDs();
	if (selectedIDs.isEmpty()) {
		return;
	}
	QList<Opcode> oldVersions;

	if (totalDel &&
	        QMessageBox::warning(this, tr("Delete"),
	                             tr("Are you sure you want to delete %1?")
	                             .arg(selectedIDs.size()==1
	                                  ? tr("the selected command")
	                                  : tr("the selected commands")),
	                             QMessageBox::Yes | QMessageBox::Cancel)
	        == QMessageBox::Cancel) {
		return;
	}

	saveExpandedItems();
	
	std::sort(selectedIDs.begin(), selectedIDs.end());
	for (qsizetype i = selectedIDs.size() - 1; i >= 0; --i) {
		oldVersions.prepend(_script->opcode(quint16(selectedIDs.at(i))));
		_script->removeOpcode(quint16(selectedIDs.at(i)));
	}

	fill();
	emit changed();
	changeHist(Remove, selectedIDs, oldVersions);

	if (topLevelItemCount() != 0) {
		if (selectedIDs.at(0) >= topLevelItemCount() && selectedIDs.at(0) > 0) {
			scroll(selectedIDs.at(0)-1);
		}
		else if (selectedIDs.at(0) < topLevelItemCount()) {
			scroll(selectedIDs.at(0));
		}
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
	QList<Opcode> opcodeCopied;
	QList<int> selIds = selectedIDs();
	for (int id : std::as_const(selIds)) {
		opcodeCopied.append(_script->opcode(id));
	}

	if (!opcodeCopied.isEmpty()) {
		Clipboard::instance()->setFF7FieldScriptOpcodes(opcodeCopied);
		paste_A->setEnabled(true);
	}
}

void OpcodeList::copyText()
{
	QMap<int, const QTreeWidgetItem *> listeitems;
	QList<QTreeWidgetItem *> selItems = selectedItems();
	for (const QTreeWidgetItem *item : std::as_const(selItems)) {
		listeitems.insert(item->data(0, Qt::UserRole).toInt(), item);
	}

	QString copiedText;
	const QTreeWidgetItem *lastitem = nullptr, *parentitem;
	QStack<const QTreeWidgetItem *> parentitems;
	int indent = 0;
	for (const QTreeWidgetItem *item : std::as_const(listeitems)) {
		if (lastitem != nullptr) {
			parentitem = item->parent();
			if (parentitem == lastitem) {
				indent++;
				parentitems.push(parentitem);
			}
			else {
				while (!parentitems.isEmpty() && parentitem != parentitems.top()) {
					indent--;
					parentitems.pop();
				}
			}

			for (int i=0; i<indent; ++i) {
				copiedText.append('\t');
			}
		}

		copiedText.append(item->text(0)).append('\n');
		lastitem = item;
	}
	if (!copiedText.isEmpty()) {
		QApplication::clipboard()->setText(copiedText);
	}
}

void OpcodeList::paste()
{
	QList<Opcode> pastedOpcodes = Clipboard::instance()->ff7FieldScriptOpcodes();
	if (!pastedOpcodes.isEmpty()) {
		saveExpandedItems();

		QList<int> IDs;
		int opcodeID = selectedID() + 1, i = opcodeID;

		for (const Opcode &opcode : std::as_const(pastedOpcodes)) {
			IDs.append(i);
			// TODO: label duplication case
			_script->insertOpcode(i, opcode);
			++i;
		}

		fill();
		scroll(opcodeID);
		emit changed();
		changeHist(Add, IDs, QList<Opcode>());
	}
}

void OpcodeList::move(Script::MoveDirection direction)
{
	int opcodeID = selectedID();
	if (opcodeID == -1) {
		return;
	}
	saveExpandedItems();
	if (_script->moveOpcode(opcodeID, direction)) {
		fill();
		scroll(direction == Script::Down ? opcodeID + 1 : opcodeID - 1);
		emit changed();
		if (direction == Script::Down) {
			changeHist(Down, opcodeID);
		} else {
			changeHist(Up, opcodeID);
		}
	} else {
		setFocus();
	}
}

void OpcodeList::scroll(int id, bool focus)
{
	QTreeWidgetItem *item = findItem(id);
	if (item == nullptr) {
		return;
	}
	setCurrentItem(item);
	scrollToItem(item, QAbstractItemView::PositionAtTop);
	if (focus) {
		setFocus();
	}
}

QTreeWidgetItem *OpcodeList::findItem(int id)
{
	QTreeWidgetItemIterator it(this);
	while (*it) {
		if ((*it)->data(0, Qt::UserRole).toInt() == id) {
			return *it;
		}
		++it;
	}

	return nullptr;
}

int OpcodeList::selectedID()
{
	if (currentItem() == nullptr) {
		return -1;
	}
	return currentItem()->data(0, Qt::UserRole).toInt();
}

QList<int> OpcodeList::selectedIDs()
{
	QList<int> list;
	QList<QTreeWidgetItem *> selItems = selectedItems();
	for (const QTreeWidgetItem *item : std::as_const(selItems)) {
		list.append(item->data(0, Qt::UserRole).toInt());
	}
	std::sort(list.begin(), list.end());
	return list;
}

int OpcodeList::selectedOpcode()
{
	int opcodeID = selectedID();
	return opcodeID <= -1 || opcodeID >= _script->size()
			? -1
	        : _script->opcode(opcodeID).id();
}

QPixmap &OpcodeList::posNumber(int num, const QPixmap &fontPixmap, QPixmap &wordPixmap)
{
	QString strNum = QString("%1").arg(num, 5, 10, QChar(' '));
	wordPixmap.fill(QColor(0,0,0,0));
	QPainter painter(&wordPixmap);

	if (strNum.at(0)!=' ')
		painter.drawTiledPixmap(1, 1, 5, 9, fontPixmap, 5*QStringView(strNum).mid(0, 1).toInt(), 0);
	if (strNum.at(1)!=' ')
		painter.drawTiledPixmap(7, 1, 5, 9, fontPixmap, 5*QStringView(strNum).mid(1, 1).toInt(), 0);
	if (strNum.at(2)!=' ')
		painter.drawTiledPixmap(13, 1, 5, 9, fontPixmap, 5*QStringView(strNum).mid(2, 1).toInt(), 0);
	if (strNum.at(3)!=' ')
		painter.drawTiledPixmap(19, 1, 5, 9, fontPixmap, 5*QStringView(strNum).mid(3, 1).toInt(), 0);
	if (strNum.at(4)!=' ')
		painter.drawTiledPixmap(25, 1, 5, 9, fontPixmap, 5*QStringView(strNum).mid(4, 1).toInt(), 0);

	painter.end();
	return wordPixmap;
}

void OpcodeList::gotoLabel(QTreeWidgetItem *item)
{
	if (item == nullptr) {
		item = currentItem();
	}
	int opcodeID = item->data(0, Qt::UserRole).toInt();
	const Opcode &op = _script->opcode(opcodeID);

	if (op.isJump()) {
		if (op.badJump() == BadJumpError::Ok) {
			int opcodeID = 0;

			for (const Opcode &op2 : _script->opcodes()) {
				if (op2.id() == OpcodeKey::LABEL && op2.label() == op.label()) {
					scroll(opcodeID);
					break;
				}
				++opcodeID;
			}
		}
	} else if (op.isExec()) {
		emit gotoScript(op.groupID(), op.scriptID() + 1);
	} /* else if (op.id() == OpcodeKey::MAPJUMP) {
		OpcodeMAPJUMP *opMJ = static_cast<OpcodeMAPJUMP *>(op);

		emit gotoField(opMJ->fieldID); // FIXME: fieldID is not the same in the field list
	} */
}

void OpcodeList::mouseReleaseEvent(QMouseEvent *event)
{
	// Alt + left click
	if (event->button() == Qt::LeftButton &&
			event->modifiers().testFlag(Qt::AltModifier)) {
		QTreeWidgetItem *item = itemAt(event->pos());
		if (item == nullptr) {
			return;
		}

		gotoLabel(item);
	}
	QTreeWidget::mouseReleaseEvent(event);
}
