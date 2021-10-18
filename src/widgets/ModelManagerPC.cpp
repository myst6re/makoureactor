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
#include "ModelManagerPC.h"
#include "core/Config.h"
#include "core/field/CharArchive.h"

ModelManagerPC::ModelManagerPC(QWidget *parent) :
    ModelManager(parent), copied(false)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
	qreal scale = qApp->desktop()->physicalDpiX() / qApp->desktop()->logicalDpiX();
#else
	qreal scale = 1.0;
#endif
	
	QToolBar *toolBar1 = new QToolBar();
	toolBar1->setIconSize(QSize(int(scale * 14), int(scale * 14)));
	toolBar1->addAction(QIcon(":/images/plus.png"), QString(), this, SLOT(addModel()));
	toolBar1->addAction(QIcon(":/images/minus.png"), QString(), this, SLOT(delModel()));
	toolBar1->addAction(QIcon(":/images/up.png"), QString(), this, SLOT(upModel()));
	toolBar1->addAction(QIcon(":/images/down.png"), QString(), this, SLOT(downModel()));

	models->setContextMenuPolicy(Qt::ActionsContextMenu);
	models->setSelectionMode(QAbstractItemView::ExtendedSelection);
	cutModelAction = new QAction(QIcon(":/images/cut.png"), tr("Cut"), models);
	copyModelAction = new QAction(QIcon(":/images/copy.png"), tr("Copy"), models);
	pasteModelAction = new QAction(QIcon(":/images/paste.png"), tr("Paste"), models);
	models->addAction(cutModelAction);
	models->addAction(copyModelAction);
	models->addAction(pasteModelAction);
	cutModelAction->setShortcut(QKeySequence::Cut);
	copyModelAction->setShortcut(QKeySequence::Copy);
	pasteModelAction->setShortcut(QKeySequence::Paste);

	modelName = new QLineEdit();
	modelName->setMaxLength(256);

	toolBar2 = new QToolBar();
	toolBar2->setIconSize(QSize(int(scale * 14), int(scale * 14)));
	toolBar2->addAction(QIcon(":/images/plus.png"), QString(), this, SLOT(addAnim()));
	toolBar2->addAction(QIcon(":/images/minus.png"), QString(), this, SLOT(delAnim()));
	toolBar2->addAction(QIcon(":/images/up.png"), QString(), this, SLOT(upAnim()));
	toolBar2->addAction(QIcon(":/images/down.png"), QString(), this, SLOT(downAnim()));

	modelAnims->setColumnCount(3);
	modelAnims->setHeaderLabels(QStringList() << tr("Id") << tr("Animation") << tr("?"));

	QGridLayout *frameLayout = new QGridLayout(modelFrame);
	frameLayout->addWidget(new QLabel(tr("Name (unused)")), 0, 0);
	frameLayout->addWidget(modelName, 0, 1);
	frameLayout->addWidget(new QLabel(tr("Unknown")), 1, 0);
	frameLayout->addWidget(modelUnknown, 1, 1);
	frameLayout->addWidget(new QLabel(tr("Model size")), 2, 0);
	frameLayout->addWidget(modelScaleWidget, 2, 1);
	frameLayout->addWidget(new QLabel(tr("Global light")), 3, 0);
	frameLayout->addWidget(modelGlobalColorWidget, 3, 1);
	frameLayout->addWidget(new QLabel(tr("Directional light")), 4, 0);
	frameLayout->addLayout(modelColorsLayout, 5, 0, 1, 2);
	frameLayout->addWidget(toolBar2, 0, 2);
	frameLayout->addWidget(modelAnims, 1, 2, 6, 1);
	frameLayout->addWidget(modelWidget, 0, 3, 7, 1);
	frameLayout->setRowStretch(6, 1);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(toolBar1, 0, 0);
	layout->addWidget(models, 1, 0);
	layout->addWidget(modelFrame, 0, 1, 2, 4);
	layout->setColumnStretch(1, 1);

	adjustSize();

	connect(models, SIGNAL(doubleClicked(QModelIndex)), models, SLOT(edit(QModelIndex)));
	connect(models, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(renameOKModel(QTreeWidgetItem *)));

	connect(modelAnims, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(editAnim(QTreeWidgetItem*,int)));
	connect(modelAnims, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(renameOKAnim(QTreeWidgetItem *, int)));

	connect(modelName, SIGNAL(textEdited(QString)), SLOT(setModelName(QString)));

	connect(cutModelAction, SIGNAL(triggered()), SLOT(cutCurrentModel()));
	connect(copyModelAction, SIGNAL(triggered()), SLOT(copyCurrentModel()));
	connect(pasteModelAction, SIGNAL(triggered()), SLOT(pasteOnCurrentModel()));
	connect(modelAnims, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(updateActionsState()));
}

QList<QStringList> ModelManagerPC::modelNames() const
{
	QList<QStringList> ret;
	for (const QString &name : modelLoader()->HRCNames()) {
		ret.append(QStringList(name));
	}
	return ret;
}

QList<QTreeWidgetItem *> ModelManagerPC::animItems(int modelID) const
{
	QList<QTreeWidgetItem *> ret;
	int numA=0;
	for (const QString &name : modelLoader()->ANames(modelID)) {
		QStringList cols = QStringList() << QString::number(numA) << name << QString::number(modelLoader()->animUnknown(modelID, numA));
		QTreeWidgetItem *item = new QTreeWidgetItem(cols);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
		++numA;
		ret.append(item);
	}
	return ret;
}

void ModelManagerPC::showModelInfos2(int row)
{
	modelName->blockSignals(true);

	modelName->setText(modelLoader()->charName(row));

	modelName->blockSignals(false);

	ModelManager::showModelInfos2(row);
}

void ModelManagerPC::addModel()
{
	if (modelLoader()->modelCount() >= modelLoader()->maxModelCount()) {
		return;
	}

	QString hrc("    .HRC");
	QTreeWidgetItem *item = models->currentItem(), *newItem;
	CharArchive *charLgp = CharArchive::instance();

	if (charLgp->isOpen()) {
		QDialog dialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
		dialog.setWindowTitle(tr("Add a field model"));

		QComboBox list(&dialog);
		list.setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
		list.setEditable(true);
		QStringList files = charLgp->hrcFiles();

		std::sort(files.begin(), files.end());
		list.addItems(files);

		QPushButton OKButton(tr("OK"), &dialog);

		QVBoxLayout layout(&dialog);
		layout.addWidget(&list);
		layout.addWidget(&OKButton);

		connect(&list, SIGNAL(currentIndexChanged(QString)), SLOT(modifyHRC(QString)));
		connect(&OKButton, SIGNAL(released()), &dialog, SLOT(accept()));

		if (dialog.exec() == QDialog::Accepted) {
			hrc = list.currentText().left(8).toUpper();
		}
		else {
			if (modelPreview) {
				newItem = modelAnims->currentItem();
				if (item!=nullptr && newItem!=nullptr) {
					modelPreview->setIsAnimated(true);
					modelPreview->setFieldModelFile(field()->fieldModel(item->text(0), newItem->text(1)));
				} else
					modelPreview->clear();
			}
			return;
		}
	}

	if (modelPreview)	modelPreview->clear();

	models->blockSignals(true);

	newItem = new QTreeWidgetItem(QStringList(hrc));

	if (item == nullptr) {
		models->addTopLevelItem(newItem);

		modelLoader()->insertModel(models->topLevelItemCount(), hrc);
	}
	else {
		int row = currentModelID(item);
		models->insertTopLevelItem(row+1, newItem);

		modelLoader()->insertModel(row+1, hrc);
	}
	newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

	models->blockSignals(false);

	emit modified();

	models->setCurrentItem(newItem);

	if (!charLgp->isOpen()) {
		models->editItem(newItem);
	}
}

void ModelManagerPC::delModel()
{
	QTreeWidgetItem *item = models->currentItem();
	if (item == nullptr) return;
	int row = currentModelID(item);
	if (row < 0)		return;

	modelLoader()->removeModel(row);

	/* Delete a row will trigger currentItemChanged signal
	 * before the list was really updated */
	models->blockSignals(true);
	delete models->takeTopLevelItem(row);
	models->blockSignals(false);

	models->setCurrentItem(models->topLevelItem(row));
	// Trigger currentItemChanged manually
	showModelInfos();

	emit modified();
}

void ModelManagerPC::upModel()
{
	QTreeWidgetItem *item = models->currentItem();
	if (item == nullptr) return;
	int row = currentModelID(item);
	if (row <= 0)	return;

	QTreeWidgetItem *itemAbove = models->itemAbove(item);

	models->blockSignals(true);

	QString savText = itemAbove->text(0);
	itemAbove->setText(0, item->text(0));
	item->setText(0, savText);

	modelLoader()->swapModel(row, row-1);

	models->blockSignals(false);

	models->setCurrentItem(itemAbove);

	emit modified();
}

void ModelManagerPC::downModel()
{
	QTreeWidgetItem *item = models->currentItem();
	if (item == nullptr) return;
	int row = currentModelID(item);
	if (row < 0 || row >= models->topLevelItemCount()-1)	return;

	QTreeWidgetItem *itemBelow = models->itemBelow(item);

	models->blockSignals(true);

	QString savText = itemBelow->text(0);
	itemBelow->setText(0, item->text(0));
	item->setText(0, savText);

	modelLoader()->swapModel(row, row+1);

	models->blockSignals(false);

	models->setCurrentItem(itemBelow);

	emit modified();
}

void ModelManagerPC::renameOKModel(QTreeWidgetItem *item)
{
	int modelID = currentModelID();
	if (modelID < 0)		return;

	QString text = item->text(0).left(8);

	item->setText(0, text);
	modelLoader()->setHRCName(modelID, text);

	emit modified();
}

void ModelManagerPC::modifyHRC(const QString &hrc)
{
	if (modelPreview) {
		modelPreview->setIsAnimated(false);
		modelPreview->setFieldModelFile(field()->fieldModel(hrc, "aafe"));
		QString a;
		switch (modelPreview->boneCount()) {
		case 5:		a = "avhd";	break;
		case 9:		a = "geaf";	break;
		case 11:	a = "bdfe";	break;
		case 12:	a = "atcf";	break;
		case 13:	a = "bria";	break;
		case 14:	a = "fgad";	break;
		case 15:	a = "gcad";	break;
		case 17:	a = "hkga";	break;
		case 18:	a = "gsia";	break;
		case 20:	a = "FZGB";	break;
		case 23:	a = "anhd";	break;
		case 24:	a = "abcd";	break;
		case 25:	a = "afdf";	break;
		case 26:	a = "hlfb";	break;
		case 27:	a = "fhhb";	break;
		case 28:	a = "aeha";	break;
		case 29:	a = "aeae";	break;
		}

		if (!a.isEmpty()) {
			modelPreview->setIsAnimated(false);
			modelPreview->setFieldModelFile(field()->fieldModel(hrc, a));
		}
	}
}

void ModelManagerPC::setModelName(const QString &modelName)
{
	int modelID = currentModelID();
	if (modelID < 0)	return;

	modelLoader()->setCharName(modelID, modelName);

	emit modified();
}

void ModelManagerPC::addAnim()
{
	int modelID = currentModelID();

	if (modelID < 0 ||
			modelLoader()->animCount(modelID) >= modelLoader()->maxAnimCount()) {
		return;
	}

	QTreeWidgetItem *item = modelAnims->currentItem(), *newItem;
	QString a;
	CharArchive *charLgp = CharArchive::instance();

	if (charLgp->isOpen()) {
		toolBar2->setEnabled(false);
		QDialog dialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
		dialog.setWindowTitle(tr("Add a field model"));

		QComboBox list(&dialog);
		list.setEditable(true);
		list.setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

		QTimer t(this);
		connect(&t, SIGNAL(timeout()), SLOT(processEvents()));
		t.start(700);

		int boneCount = modelPreview ? modelPreview->boneCount() : 0;
		QStringList files = charLgp->aFiles(boneCount);
		if (boneCount != 0) {
			files.append(charLgp->aFiles(0)); // Animations without bones work too
		}

		t.stop();

		std::sort(files.begin(), files.end());
		list.addItems(files);

		if (item!=nullptr) {
			QString text = item->text(1).toLower();
			int index = text.lastIndexOf('.');
			if (index != -1)
				text.truncate(index);
			index = list.findText(text);
			if (index != -1)
				list.setCurrentIndex(index);
		}

		QPushButton OKButton(tr("OK"), &dialog);

		QVBoxLayout layout(&dialog);
		layout.addWidget(&list);
		layout.addWidget(&OKButton);

		connect(&list, SIGNAL(currentIndexChanged(QString)), SLOT(modifyAnimation(QString)));
		connect(&OKButton, SIGNAL(released()), &dialog, SLOT(accept()));
		toolBar2->setEnabled(true);

		if (dialog.exec() == QDialog::Accepted) {
			a = list.currentText();
		}
		else {
			if (modelPreview) {
				if (item != nullptr) {
					modelPreview->setIsAnimated(true);
					modelPreview->setFieldModelFile(field()->fieldModel(models->currentItem()->text(0), item->text(1)));
				} else
					modelPreview->clear();
			}
			return;
		}
	}

	modelAnims->blockSignals(true);

	newItem = new QTreeWidgetItem(QStringList() << "" << a << "1");

	if (item == nullptr) {
		modelAnims->addTopLevelItem(newItem);

		modelLoader()->insertAnim(modelID, modelAnims->topLevelItemCount(), a);
	}
	else {
		int animID = currentAnimID(item);
		modelAnims->insertTopLevelItem(animID+1, newItem);

		modelLoader()->insertAnim(modelID, animID+1, a);
	}

	// Update animation IDs
	for (int aID=0; aID<modelAnims->topLevelItemCount(); ++aID) {
		modelAnims->topLevelItem(aID)->setText(0, QString::number(aID));
	}

	modelAnims->blockSignals(false);

	emit modified();

	newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
	modelAnims->setCurrentItem(newItem);
	if (!charLgp->isOpen()) {
		modelAnims->editItem(newItem);
	}
}

void ModelManagerPC::processEvents() const
{
	QCoreApplication::processEvents();
}

void ModelManagerPC::modifyAnimation(const QString &a)
{
	if (modelPreview) {
		modelPreview->setIsAnimated(true);
		modelPreview->setFieldModelFile(field()->fieldModel(models->currentItem()->text(0), a));
	}
}

void ModelManagerPC::delAnim()
{
	int modelID = currentModelID();
	if (modelID < 0)		return;
	QTreeWidgetItem *item = modelAnims->currentItem();
	if (item == nullptr) return;
	int animID = currentAnimID(item);
	if (animID < 0)	return;

	modelLoader()->removeAnim(modelID, animID);
	if (modelPreview)	modelPreview->clear();

	delete item;

	// Update animation IDs
	for (int aID=0; aID<modelAnims->topLevelItemCount(); ++aID) {
		modelAnims->topLevelItem(aID)->setText(0, QString::number(aID));
	}

	emit modified();
}

void ModelManagerPC::upAnim()
{
	int modelID = currentModelID();
	if (modelID < 0)		return;
	QTreeWidgetItem *item = modelAnims->currentItem();
	if (item == nullptr) return;
	int animID = currentAnimID(item);
	if (animID <= 0)	return;

	QTreeWidgetItem *itemAbove = modelAnims->itemAbove(item);

	modelAnims->blockSignals(true);

	QString savText = itemAbove->text(1);
	itemAbove->setText(1, item->text(1));
	item->setText(1, savText);
	savText = itemAbove->text(2);
	itemAbove->setText(2, item->text(2));
	item->setText(2, savText);

	modelLoader()->swapAnim(modelID, animID, animID-1);

	modelAnims->blockSignals(false);

	modelAnims->setCurrentItem(itemAbove);

	emit modified();
}

void ModelManagerPC::downAnim()
{
	int modelID = currentModelID();
	if (modelID < 0)		return;
	QTreeWidgetItem *item = modelAnims->currentItem();
	if (item == nullptr) return;
	int animID = currentAnimID(item);
	if (animID < 0 || animID >= modelAnims->topLevelItemCount()-1)	return;

	QTreeWidgetItem *itemBelow = modelAnims->itemBelow(item);

	modelAnims->blockSignals(true);

	QString savText = itemBelow->text(1);
	itemBelow->setText(1, item->text(1));
	item->setText(1, savText);
	savText = itemBelow->text(2);
	itemBelow->setText(2, item->text(2));
	item->setText(2, savText);

	modelLoader()->swapAnim(modelID, animID, animID+1);

	modelAnims->blockSignals(false);

	modelAnims->setCurrentItem(itemBelow);

	emit modified();
}

// Filter to not edit column 0
void ModelManagerPC::editAnim(QTreeWidgetItem *item, int column)
{
	if (column != 0) {
		modelAnims->editItem(item, column);
	}
}

void ModelManagerPC::renameOKAnim(QTreeWidgetItem *item, int column)
{
	int modelID = currentModelID();
	if (modelID < 0)		return;
	int animID = currentAnimID(item);
	if (animID < 0)		return;

	if (column == 1) {
		QString text = item->text(1).left(256);
		item->setText(1, text);

		modelLoader()->setAName(modelID, animID, text);
		emit modified();
	} else if (column == 2) {
		bool ok;
		int value = item->text(2).toInt(&ok);
		if (ok) {
			if (value > 65535) {
				value = 65535;
				item->setText(2, QString::number(value));
			}
			modelLoader()->setAnimUnknown(modelID, animID, quint16(value));
			emit modified();
		} else {
			// reset old value
			item->setText(2, QString::number(modelLoader()->animUnknown(modelID, animID)));
		}
	}
}

FieldModelFile *ModelManagerPC::modelData(QTreeWidgetItem *item)
{
	return field()->fieldModel(models->currentItem()->text(0), item->text(1));
}

FieldModelLoaderPC *ModelManagerPC::modelLoader() const
{
	return static_cast<FieldModelLoaderPC *>(ModelManager::modelLoader());
}

FieldPC *ModelManagerPC::field() const
{
	return static_cast<FieldPC *>(ModelManager::field());
}

const QList<FieldModelColorDir> &ModelManagerPC::lightColors(int modelID) const
{
	return modelLoader()->lightColors(modelID);
}

void ModelManagerPC::setLightColor(int modelID, int id, const FieldModelColorDir &color)
{
	modelLoader()->setLightColor(modelID, id, color);
}

QRgb ModelManagerPC::globalColor(int modelID) const
{
	return modelLoader()->globalColor(modelID);
}

void ModelManagerPC::setGlobalColor(int modelID, QRgb color)
{
	modelLoader()->setGlobalColor(modelID, color);
}

quint16 ModelManagerPC::modelScale(int modelID) const
{
	return modelLoader()->scale(modelID);
}

void ModelManagerPC::setModelScale(int modelID, quint16 scale)
{
	modelLoader()->setScale(modelID, scale);
}

void ModelManagerPC::copyModels(const QList<int> &modelIDs)
{
	if (modelIDs.isEmpty()) {
		return;
	}

	_copiedModels.clear();
	for (int modelID : modelIDs) {
		_copiedModels.append(modelLoader()->modelInfos(modelID));
	}
	copied = true;
}

void ModelManagerPC::cutModels(const QList<int> &modelIDs)
{
	if (modelIDs.isEmpty()) {
		return;
	}

	copyModels(modelIDs);
	QList<int> mIDs = modelIDs;
	std::sort(mIDs.begin(), mIDs.end());
	for (int i=mIDs.size() - 1; i>=0; --i) {
		int modelID = mIDs.at(i);
		modelLoader()->removeModel(modelID);
		delete models->topLevelItem(modelID);
	}

	emit modified();
}

void ModelManagerPC::pasteModels(int modelID)
{
	if (!copied) {
		return;
	}

	for (int i=_copiedModels.size() - 1; i>=0; --i) {
		const FieldModelInfosPC &modelInfos = _copiedModels.at(i);
		modelLoader()->insertModelInfos(modelID, modelInfos);

		QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(modelInfos.nameHRC));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

		models->insertTopLevelItem(modelID, item);
	}

	models->selectionModel()->select(
				QItemSelection(models->model()->index(modelID, 0),
							   models->model()->index(modelID + _copiedModels.size()-1, 0)),
				QItemSelectionModel::ClearAndSelect);

	emit modified();
}

void ModelManagerPC::copyCurrentModel()
{
	copyModels(selectedModelIDs());
}

void ModelManagerPC::cutCurrentModel()
{
	cutModels(selectedModelIDs());
}

void ModelManagerPC::pasteOnCurrentModel()
{
	int modelID = currentModelID();
	if (modelID < 0) {
		modelID = models->topLevelItemCount();
	} else {
		modelID += 1;
	}
	pasteModels(modelID);
}

void ModelManagerPC::updateActionsState()
{
	bool isModelSelected = currentModelID() >= 0;

	cutModelAction->setEnabled(isModelSelected);
	copyModelAction->setEnabled(isModelSelected);
}
