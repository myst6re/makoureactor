/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
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
#include "Data.h"

ModelManagerPC::ModelManagerPC(const QGLWidget *shareWidget, QWidget *parent) :
	ModelManager(shareWidget, parent)
{
	globalScale = new QSpinBox();
	globalScale->setRange(0, 65535);

	QToolBar *toolBar1 = new QToolBar();
	toolBar1->setIconSize(QSize(14, 14));
	toolBar1->addAction(QIcon(":/images/plus.png"), QString(), this, SLOT(addModel()));
	toolBar1->addAction(QIcon(":/images/minus.png"), QString(), this, SLOT(delModel()));
	toolBar1->addAction(QIcon(":/images/up.png"), QString(), this, SLOT(upModel()));
	toolBar1->addAction(QIcon(":/images/down.png"), QString(), this, SLOT(downModel()));

	models->setColumnCount(1);

	modelName = new QLineEdit();
	modelName->setMaxLength(256);
	modelScale = new QSpinBox();
	modelScale->setRange(0, 4096);
	modelColorDisplay = new ColorDisplay();

	toolBar2 = new QToolBar();
	toolBar2->setIconSize(QSize(14, 14));
	toolBar2->addAction(QIcon(":/images/plus.png"), QString(), this, SLOT(addAnim()));
	toolBar2->addAction(QIcon(":/images/minus.png"), QString(), this, SLOT(delAnim()));
	toolBar2->addAction(QIcon(":/images/up.png"), QString(), this, SLOT(upAnim()));
	toolBar2->addAction(QIcon(":/images/down.png"), QString(), this, SLOT(downAnim()));

	modelAnims->setColumnCount(2);
	modelAnims->setHeaderLabels(QStringList() << tr("Animations") << tr("?"));

	QGridLayout *frameLayout = new QGridLayout(modelFrame);
	frameLayout->addWidget(new QLabel(tr("Nom (non utilisé)")), 0, 0);
	frameLayout->addWidget(modelName, 0, 1);
	frameLayout->addWidget(new QLabel(tr("Inconnu")), 1, 0);
	frameLayout->addWidget(modelUnknown, 1, 1);
	frameLayout->addWidget(new QLabel(tr("Taille modèle")), 2, 0);
	frameLayout->addWidget(modelScale, 2, 1);
	frameLayout->addWidget(new QLabel(tr("Lumière")), 3, 0);
	frameLayout->addWidget(modelColorDisplay, 3, 1);
	frameLayout->addWidget(toolBar2, 0, 2);
	frameLayout->addWidget(modelAnims, 1, 2, 4, 1);
	frameLayout->addWidget(modelWidget, 0, 3, 5, 1);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(toolBar1, 0, 0);
	layout->addWidget(new QLabel(tr("Taille modèles (non utilisé)")), 0, 1);
	layout->addWidget(globalScale, 0, 2);
	layout->addWidget(models, 1, 0);
	layout->addWidget(modelFrame, 1, 1, 1, 4);

	adjustSize();

	connect(globalScale, SIGNAL(valueChanged(int)), SLOT(setGlobalScale(int)));

	connect(models, SIGNAL(doubleClicked(QModelIndex)), models, SLOT(edit(QModelIndex)));
	connect(models, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(renameOKModel(QTreeWidgetItem *)));

	connect(modelAnims, SIGNAL(doubleClicked(QModelIndex)), modelAnims, SLOT(edit(QModelIndex)));
	connect(modelAnims, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(renameOKAnim(QTreeWidgetItem *, int)));

	connect(modelName, SIGNAL(textEdited(QString)), SLOT(setModelName(QString)));
	connect(modelScale, SIGNAL(valueChanged(int)), SLOT(setModelScale(int)));
	connect(modelColorDisplay, SIGNAL(colorEdited(int,QRgb)), SLOT(setModelColor(int,QRgb)));
}

void ModelManagerPC::fill2()
{
	globalScale->blockSignals(true);
	globalScale->setValue(modelLoader()->globalScale());
	globalScale->blockSignals(false);

	ModelManager::fill2();
}

QList<QStringList> ModelManagerPC::modelNames() const
{
	QList<QStringList> ret;
	foreach(const QString &name, modelLoader()->HRCNames()) {
		ret.append(QStringList(name));
	}
	return ret;
}

QList<QStringList> ModelManagerPC::animNames(int row) const
{
	QList<QStringList> ret;
	int numA=0;
	foreach(const QString &name, modelLoader()->ANames(row)) {
		ret.append(QStringList() << name << QString::number(modelLoader()->animUnknown(row, numA)));
		++numA;
	}
	return ret;
}

void ModelManagerPC::setGlobalScale(int value)
{
	modelLoader()->setGlobalScale(value);
	emit modified();
}

void ModelManagerPC::showModelInfos2(int row)
{
	modelName->blockSignals(true);
	modelScale->blockSignals(true);
	modelColorDisplay->blockSignals(true);

	modelName->setText(modelLoader()->charName(row));
	modelScale->setValue(modelLoader()->scale(row));
	modelColorDisplay->setColors(modelLoader()->lightColors(row));

	modelName->blockSignals(false);
	modelScale->blockSignals(false);
	modelColorDisplay->blockSignals(false);

	ModelManager::showModelInfos2(row);
}

void ModelManagerPC::addModel()
{
	QString hrc("    .HRC");
	QTreeWidgetItem *item = models->currentItem(), *newItem;

	if(Data::charlgp_loadListPos()) {
		QDialog dialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
		dialog.setWindowTitle(tr("Ajouter un modèle 3D"));

		QComboBox list(&dialog);
		list.setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
		list.setEditable(true);
		QStringList files;

		foreach(const QString &file, Data::charLgp.fileList()) {
			if(file.endsWith(".hrc", Qt::CaseInsensitive)) {
				files.append(file);
			}
		}

		qSort(files);
		list.addItems(files);

		QPushButton OKButton(tr("OK"), &dialog);

		QVBoxLayout layout(&dialog);
		layout.addWidget(&list);
		layout.addWidget(&OKButton);

		connect(&list, SIGNAL(currentIndexChanged(QString)), SLOT(modifyHRC(QString)));
		connect(&OKButton, SIGNAL(released()), &dialog, SLOT(accept()));

		if(dialog.exec() == QDialog::Accepted) {
			hrc = list.currentText().left(8).toUpper();
		}
		else {
			if(modelPreview) {
				newItem = modelAnims->currentItem();
				if(item!=NULL && newItem!=NULL) {
					modelPreview->load(field(), item->text(0), newItem->text(0));
				} else
					modelPreview->clear();
			}
			return;
		}
	}

	if(modelPreview)	modelPreview->clear();

	models->blockSignals(true);

	newItem = new QTreeWidgetItem(QStringList(hrc));

	if(item == NULL) {
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

	if(!Data::charLgp.isOpen())
		models->editItem(newItem);
}

void ModelManagerPC::delModel()
{
	QTreeWidgetItem *item = models->currentItem();
	if(item == NULL) return;
	int row = currentModelID(item);
	if(row < 0)		return;

	modelLoader()->removeModel(row);

	delete item;

	emit modified();
}

void ModelManagerPC::upModel()
{
	QTreeWidgetItem *item = models->currentItem();
	if(item == NULL) return;
	int row = currentModelID(item);
	if(row <= 0)	return;

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
	if(item == NULL) return;
	int row = currentModelID(item);
	if(row < 0 || row >= models->topLevelItemCount()-1)	return;

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
	if(modelID < 0)		return;

	QString text = item->text(0).left(8);

	item->setText(0, text);
	modelLoader()->setHRCName(modelID, text);

	emit modified();
}

void ModelManagerPC::modifyHRC(const QString &hrc)
{
	if(modelPreview) {
		modelPreview->load(field(), hrc, "aafe", false);
		QString a;
		switch(modelPreview->boneCount()) {
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

		if(!a.isEmpty()) {
			modelPreview->load(field(), hrc, a, false);
		}
	}
}

void ModelManagerPC::setModelName(const QString &modelName)
{
	int modelID = currentModelID();
	if(modelID < 0)	return;

	modelLoader()->setCharName(modelID, modelName);

	emit modified();
}

void ModelManagerPC::setModelScale(int scale)
{
	int modelID = currentModelID();
	if(modelID < 0)	return;

	modelLoader()->setScale(modelID, scale);

	emit modified();
}

void ModelManagerPC::setModelColor(int id, QRgb color)
{
	int modelID = currentModelID();
	if(modelID < 0)	return;

	modelLoader()->setLightColor(modelID, id, color);

	emit modified();
}

void ModelManagerPC::addAnim()
{
	QTreeWidgetItem *item = modelAnims->currentItem(), *newItem;
	QString a;
	if(Data::charlgp_loadListPos())
	{
		toolBar2->setEnabled(false);
		QDialog dialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
		dialog.setWindowTitle(tr("Ajouter un modèle 3D"));

		QComboBox list(&dialog);
		list.setEditable(true);
		list.setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

		Data::charlgp_loadAnimBoneCount();

		QStringList files;

		int boneCount = modelPreview ? modelPreview->boneCount() : 0;

		foreach(const QString &file, Data::charLgp.fileList()) {
			QCoreApplication::processEvents();
			if(file.endsWith(".a", Qt::CaseInsensitive)) {
				if(boneCount == 0 || Data::charlgp_animBoneCount.value(file.toLower()) == boneCount)
					files.append(file.left(file.size()-2).toUpper());
			}
		}

		qSort(files);
		list.addItems(files);

		if(item!=NULL) {
			QString text = item->text(0).toLower();
			int index = text.lastIndexOf('.');
			if(index != -1)
				text.truncate(index);
			index = list.findText(text);
			if(index != -1)
				list.setCurrentIndex(index);
		}

		QPushButton OKButton(tr("OK"), &dialog);

		QVBoxLayout layout(&dialog);
		layout.addWidget(&list);
		layout.addWidget(&OKButton);

		connect(&list, SIGNAL(currentIndexChanged(QString)), SLOT(modifyAnimation(QString)));
		connect(&OKButton, SIGNAL(released()), &dialog, SLOT(accept()));
		toolBar2->setEnabled(true);

		if(dialog.exec()==QDialog::Accepted) {
			a = list.currentText();
		}
		else {
			if(modelPreview) {
				if(item!=NULL)
					modelPreview->load(field(), models->currentItem()->text(0), item->text(0));
				else
					modelPreview->clear();
			}
			return;
		}
	}

	int modelID = currentModelID();
	if(modelID < 0)		return;

	modelAnims->blockSignals(true);

	newItem = new QTreeWidgetItem(QStringList() << a << "1");

	if(item == NULL) {
		modelAnims->addTopLevelItem(newItem);

		modelLoader()->insertAnim(modelID, modelAnims->topLevelItemCount(), a);
	}
	else {
		int animID = currentAnimID(item);
		modelAnims->insertTopLevelItem(animID+1, newItem);

		modelLoader()->insertAnim(modelID, animID+1, a);
	}

	modelAnims->blockSignals(false);

	emit modified();

	newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
	modelAnims->setCurrentItem(newItem);
	if(!Data::charLgp.isOpen())
		modelAnims->editItem(newItem);
}

void ModelManagerPC::modifyAnimation(const QString &a)
{
	if(modelPreview)	modelPreview->load(field(), models->currentItem()->text(0), a);
}

void ModelManagerPC::delAnim()
{
	QTreeWidgetItem *item = modelAnims->currentItem();
	if(item == NULL) return;
	int modelID = currentModelID();
	if(modelID < 0)		return;

	modelLoader()->removeAnim(modelID, currentAnimID());
	if(modelPreview)	modelPreview->clear();

	delete item;

	emit modified();
}

void ModelManagerPC::upAnim()
{
	int modelID = currentModelID();
	if(modelID < 0)		return;
	QTreeWidgetItem *item = modelAnims->currentItem();
	if(item == NULL) return;
	int animID = currentAnimID(item);
	if(animID <= 0)	return;

	QTreeWidgetItem *itemAbove = modelAnims->itemAbove(item);

	modelAnims->blockSignals(true);

	QString savText = itemAbove->text(0);
	itemAbove->setText(0, item->text(0));
	item->setText(0, savText);

	modelLoader()->swapAnim(modelID, animID, animID-1);

	modelAnims->blockSignals(false);

	modelAnims->setCurrentItem(itemAbove);

	emit modified();
}

void ModelManagerPC::downAnim()
{
	int modelID = currentModelID();
	if(modelID < 0)		return;
	QTreeWidgetItem *item = modelAnims->currentItem();
	if(item == NULL) return;
	int animID = currentAnimID(item);
	if(animID < 0 || animID >= modelAnims->topLevelItemCount()-1)	return;

	QTreeWidgetItem *itemBelow = modelAnims->itemBelow(item);

	modelAnims->blockSignals(true);

	QString savText = itemBelow->text(0);
	itemBelow->setText(0, item->text(0));
	item->setText(0, savText);

	modelLoader()->swapAnim(modelID, animID, animID+1);

	modelAnims->blockSignals(false);

	modelAnims->setCurrentItem(itemBelow);

	emit modified();
}

void ModelManagerPC::renameOKAnim(QTreeWidgetItem *item, int column)
{
	int modelID = currentModelID();
	if(modelID < 0)		return;
	int animID = currentAnimID(item);
	if(animID < 0)		return;

	if(column == 0) {
		QString text = item->text(0).left(256);
		item->setText(0, text);

		modelLoader()->setAName(modelID, animID, text);
		emit modified();
	} else if(column == 1) {
		bool ok;
		int value = item->text(1).toInt(&ok);
		if(ok) {
			if(value > 65535) {
				value = 65535;
				item->setText(1, QString::number(value));
			}
			modelLoader()->setAnimUnknown(modelID, animID, value);
			emit modified();
		} else {
			// reset old value
			item->setText(1, QString::number(modelLoader()->animUnknown(modelID, animID)));
		}
	}
}

void ModelManagerPC::showModel2(QTreeWidgetItem *item)
{
	modelPreview->load(field(), models->currentItem()->text(0), item->text(0));
}

FieldModelLoaderPC *ModelManagerPC::modelLoader() const
{
	return (FieldModelLoaderPC *)ModelManager::modelLoader();
}

FieldPC *ModelManagerPC::field() const
{
	return (FieldPC *)ModelManager::field();
}
