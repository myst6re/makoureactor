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
#include "ModelManager.h"
#include "Config.h"
#include "Data.h"

ModelManager::ModelManager(const QGLWidget *shareWidget, QWidget *parent) :
	QDialog(parent, Qt::Tool), field(0), fieldModelLoader(0)
{
	setWindowTitle(tr("Modèles 3D"));

	QFont font;
	font.setPointSize(8);

	globalScale = new QSpinBox();
	globalScale->setRange(0, 65535);

	QToolBar *toolBar1 = new QToolBar();
	toolBar1->setIconSize(QSize(14, 14));
	toolBar1->addAction(QIcon(":/images/plus.png"), QString(), this, SLOT(addModel()));
	toolBar1->addAction(QIcon(":/images/minus.png"), QString(), this, SLOT(delModel()));
	toolBar1->addAction(QIcon(":/images/up.png"), QString(), this, SLOT(upModel()));
	toolBar1->addAction(QIcon(":/images/down.png"), QString(), this, SLOT(downModel()));

	models = new QTreeWidget();
	models->setColumnCount(1);
	models->setIndentation(0);
	models->setHeaderLabel(tr("Modèles 3D"));
	models->setFixedWidth(120);

	modelFrame = new QFrame();
	modelFrame->setFrameShape(QFrame::StyledPanel);

	modelName = new QLineEdit();
	modelName->setMaxLength(256);
	modelUnknown = new QSpinBox();
	modelUnknown->setRange(0, 65535);
	modelScale = new QSpinBox();
	modelScale->setRange(0, 4096);
	modelColorDisplay = new ColorDisplay();

	toolBar2 = new QToolBar();
	toolBar2->setIconSize(QSize(14, 14));
	toolBar2->addAction(QIcon(":/images/plus.png"), QString(), this, SLOT(addAnim()));
	toolBar2->addAction(QIcon(":/images/minus.png"), QString(), this, SLOT(delAnim()));
	toolBar2->addAction(QIcon(":/images/up.png"), QString(), this, SLOT(upAnim()));
	toolBar2->addAction(QIcon(":/images/down.png"), QString(), this, SLOT(downAnim()));

	modelAnims = new QTreeWidget();
	modelAnims->setColumnCount(2);
	modelAnims->setIndentation(0);
	modelAnims->setHeaderLabels(QStringList() << tr("Animations") << tr("?"));
	modelAnims->setFixedWidth(120);

	QWidget *modelWidget;
	if(Config::value("OpenGL", true).toBool()) {
		modelPreview = new FieldModel(0, shareWidget);
		modelWidget = modelPreview;
		modelPreview->setFixedSize(304, 214);
	} else {
		modelPreview = 0;
		modelWidget = new QWidget(this);
	}

	QGridLayout *frameLayout = new QGridLayout();
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
	modelFrame->setLayout(frameLayout);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(toolBar1, 0, 0);
	layout->addWidget(new QLabel(tr("Taille modèles (non utilisé)")), 0, 1);
	layout->addWidget(globalScale, 0, 2);
	layout->addWidget(models, 1, 0);
	layout->addWidget(modelFrame, 1, 1, 1, 4);

	setLayout(layout);
	adjustSize();

	connect(globalScale, SIGNAL(valueChanged(int)), SLOT(setGlobalScale(int)));

	connect(models, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(showModelInfos(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(models, SIGNAL(doubleClicked(QModelIndex)), models, SLOT(edit(QModelIndex)));
	connect(models, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(renameOKModel(QTreeWidgetItem *)));

	connect(modelAnims, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(showModel(QTreeWidgetItem*)));
	connect(modelAnims, SIGNAL(doubleClicked(QModelIndex)), modelAnims, SLOT(edit(QModelIndex)));
	connect(modelAnims, SIGNAL(itemChanged(QTreeWidgetItem *, int)), SLOT(renameOKAnim(QTreeWidgetItem *, int)));

	connect(modelName, SIGNAL(textEdited(QString)), SLOT(setModelName(QString)));
	connect(modelUnknown, SIGNAL(valueChanged(int)), SLOT(setModelUnknown(int)));
	connect(modelScale, SIGNAL(valueChanged(int)), SLOT(setModelScale(int)));
	connect(modelColorDisplay, SIGNAL(colorEdited(int,QRgb)), SLOT(setModelColor(int,QRgb)));
}

void ModelManager::clear()
{
	models->blockSignals(true);
	modelAnims->blockSignals(true);

	models->clear();
	modelAnims->clear();
	if(modelPreview)	modelPreview->clear();

	models->blockSignals(false);
	modelAnims->blockSignals(false);
}

void ModelManager::fill(FieldPC *field, bool reload)
{
	if(!reload && this->field == field)	return;

	clear();

	if(!field->getFieldModelLoader()->isLoaded()) {
		return;
	}

	fieldModelLoader = field->getFieldModelLoader();
	this->field = field;

	globalScale->blockSignals(true);
	globalScale->setValue(fieldModelLoader->globalScale());
	globalScale->blockSignals(false);

	fillModelList();

	QTreeWidgetItem *currentItem = models->topLevelItem(Data::currentModelID);
	if(currentItem) {
		models->setCurrentItem(currentItem);
	} else {
		models->setCurrentItem(models->topLevelItem(0));
	}
	models->setFocus();
}

void ModelManager::fillModelList()
{
	models->blockSignals(true);
	QTreeWidgetItem *item;
	foreach(const QString &hrcName, fieldModelLoader->HRCNames())
	{
		item = new QTreeWidgetItem(QStringList(hrcName));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
		models->addTopLevelItem(item);
	}
	models->blockSignals(false);
}

int ModelManager::currentModelID(QTreeWidgetItem *item) const
{
	if(!item) {
		item = models->currentItem();
		if(!item)	return -1;
	}
	return models->indexOfTopLevelItem(item);
}

int ModelManager::currentAnimID(QTreeWidgetItem *item) const
{
	if(!item) {
		item = modelAnims->currentItem();
		if(!item)	return -1;
	}
	return modelAnims->indexOfTopLevelItem(item);
}

void ModelManager::setGlobalScale(int value)
{
	fieldModelLoader->setGlobalScale(value);
	emit modified();
}

void ModelManager::showModelInfos(QTreeWidgetItem *item, QTreeWidgetItem *)
{
	if(item == NULL) {
		modelFrame->setEnabled(false);
		return;
	}
	modelFrame->setEnabled(true);
	int row = currentModelID(item);
	modelAnims->clear();

	if(row < 0 || row >= fieldModelLoader->modelCount())	return;

	int numA=0;
	foreach(const QString &animName, fieldModelLoader->ANames(row))
	{
		item = new QTreeWidgetItem(QStringList() << animName << QString::number(fieldModelLoader->animUnknown(row, numA)));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
		modelAnims->addTopLevelItem(item);
		++numA;
	}

	modelAnims->resizeColumnToContents(0);
	modelAnims->resizeColumnToContents(1);

	modelName->blockSignals(true);
	modelUnknown->blockSignals(true);
	modelScale->blockSignals(true);
	modelColorDisplay->blockSignals(true);

	modelName->setText(fieldModelLoader->charName(row));
	modelUnknown->setValue(fieldModelLoader->unknown(row));
	modelScale->setValue(fieldModelLoader->scale(row));
	modelColorDisplay->setColors(fieldModelLoader->lightColors(row));

	modelName->blockSignals(false);
	modelUnknown->blockSignals(false);
	modelScale->blockSignals(false);
	modelColorDisplay->blockSignals(false);

	if(modelAnims->topLevelItemCount() > 0)
		modelAnims->setCurrentItem(modelAnims->topLevelItem(0));
}

void ModelManager::addModel()
{
	QString hrc("    .HRC");
	QTreeWidgetItem *item = models->currentItem(), *newItem;

	if(!Data::charlgp_listPos.isEmpty()) {
		QDialog dialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
		dialog.setWindowTitle(tr("Ajouter un modèle 3D"));

		QComboBox list(&dialog);
		list.setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
		list.setEditable(true);
		QStringList files;
		QString file;

		QHashIterator<QString, int> i(Data::charlgp_listPos);
		while(i.hasNext()) {
			i.next();
			file = i.key();
			if(file.endsWith(QString(".hrc"), Qt::CaseInsensitive)) {
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
					modelPreview->load(field, item->text(0), newItem->text(0));
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

		fieldModelLoader->insertModel(models->topLevelItemCount(), hrc);
	}
	else {
		int row = currentModelID(item);
		models->insertTopLevelItem(row+1, newItem);

		fieldModelLoader->insertModel(row+1, hrc);
	}
	newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

	models->blockSignals(false);

	emit modified();

	models->setCurrentItem(newItem);

	if(Data::charlgp_listPos.isEmpty())
		models->editItem(newItem);
}

void ModelManager::delModel()
{
	QTreeWidgetItem *item = models->currentItem();
	if(item == NULL) return;
	int row = currentModelID(item);
	if(row < 0)		return;

	fieldModelLoader->removeModel(row);

	delete item;

	emit modified();
}

void ModelManager::upModel()
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

	fieldModelLoader->swapModel(row, row-1);

	models->blockSignals(false);

	models->setCurrentItem(itemAbove);

	emit modified();
}

void ModelManager::downModel()
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

	fieldModelLoader->swapModel(row, row+1);

	models->blockSignals(false);

	models->setCurrentItem(itemBelow);

	emit modified();
}

void ModelManager::renameOKModel(QTreeWidgetItem *item)
{
	int modelID = currentModelID();
	if(modelID < 0)		return;

	QString text = item->text(0).left(8);

	item->setText(0, text);
	fieldModelLoader->setHRCName(modelID, text);

	emit modified();
}

void ModelManager::modifyHRC(const QString &hrc)
{
	if(modelPreview) {
		modelPreview->load(field, hrc, "aafe", false);
		QString a;
		switch(modelPreview->nb_bones()) {
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
			modelPreview->load(field, hrc, a, false);
		}
	}
}

void ModelManager::setModelName(const QString &modelName)
{
	int modelID = currentModelID();
	if(modelID < 0)	return;

	fieldModelLoader->setCharName(modelID, modelName);

	emit modified();
}

void ModelManager::setModelUnknown(int unknown)
{
	int modelID = currentModelID();
	if(modelID < 0)	return;

	fieldModelLoader->setUnknown(modelID, unknown);

	emit modified();
}

void ModelManager::setModelScale(int scale)
{
	int modelID = currentModelID();
	if(modelID < 0)	return;

	fieldModelLoader->setScale(modelID, scale);

	emit modified();
}

void ModelManager::setModelColor(int id, QRgb color)
{
	int modelID = currentModelID();
	if(modelID < 0)	return;

	fieldModelLoader->setLightColor(modelID, id, color);

	emit modified();
}

void ModelManager::addAnim()
{
	QTreeWidgetItem *item = modelAnims->currentItem(), *newItem;
	QString a;
	if(!Data::charlgp_listPos.isEmpty())
	{
		toolBar2->setEnabled(false);
		QDialog dialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
		dialog.setWindowTitle(tr("Ajouter un modèle 3D"));

		QComboBox list(&dialog);
		list.setEditable(true);
		list.setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

		Data::charlgp_loadAnimBoneCount();

		QStringList files;
		QString file;
		QHashIterator<QString, int> i(Data::charlgp_listPos);

		int nb_bones = modelPreview ? modelPreview->nb_bones() : 0;

		while(i.hasNext()) {
			QCoreApplication::processEvents();
			i.next();
			file = i.key();
			if(file.endsWith(".a", Qt::CaseInsensitive))
			{
				if(nb_bones == 0 || Data::charlgp_animBoneCount.value(file.toLower()) == nb_bones)
					files.append(file.left(file.size()-2).toUpper());
			}
		}

		qSort(files);
		list.addItems(files);

		if(item!=NULL) {
			QString texte = item->text(0).toLower();
			int index;
			if((index=texte.lastIndexOf('.')) != -1)
				texte.truncate(index);
			if((index=list.findText(texte)) != -1)
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
					modelPreview->load(field, models->currentItem()->text(0), item->text(0));
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

		fieldModelLoader->insertAnim(modelID, modelAnims->topLevelItemCount(), a);
	}
	else {
		int animID = currentAnimID(item);
		modelAnims->insertTopLevelItem(animID+1, newItem);

		fieldModelLoader->insertAnim(modelID, animID+1, a);
	}

	modelAnims->blockSignals(false);

	emit modified();

	newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
	modelAnims->setCurrentItem(newItem);
	if(Data::charlgp_listPos.isEmpty())
		modelAnims->editItem(newItem);
}

void ModelManager::modifyAnimation(const QString &a)
{
	if(modelPreview)	modelPreview->load(field, models->currentItem()->text(0), a);
}

void ModelManager::delAnim()
{
	QTreeWidgetItem *item = modelAnims->currentItem();
	if(item == NULL) return;
	int modelID = currentModelID();
	if(modelID < 0)		return;

	fieldModelLoader->removeAnim(modelID, currentAnimID());
	if(modelPreview)	modelPreview->clear();

	delete item;

	emit modified();
}

void ModelManager::upAnim()
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

	fieldModelLoader->swapAnim(modelID, animID, animID-1);

	modelAnims->blockSignals(false);

	modelAnims->setCurrentItem(itemAbove);

	emit modified();
}

void ModelManager::downAnim()
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

	fieldModelLoader->swapAnim(modelID, animID, animID+1);

	modelAnims->blockSignals(false);

	modelAnims->setCurrentItem(itemBelow);

	emit modified();
}

void ModelManager::renameOKAnim(QTreeWidgetItem *item, int column)
{
	int modelID = currentModelID();
	if(modelID < 0)		return;
	int animID = currentAnimID(item);
	if(animID < 0)		return;

	if(column == 0) {
		QString text = item->text(0).left(256);
		item->setText(0, text);

		fieldModelLoader->setAName(modelID, animID, text);
		emit modified();
	} else if(column == 1) {
		bool ok;
		int value = item->text(1).toInt(&ok);
		if(ok) {
			if(value > 65535) {
				value = 65535;
				item->setText(1, QString::number(value));
			}
			fieldModelLoader->setAnimUnknown(modelID, animID, value);
			emit modified();
		} else {
			// reset old value
			item->setText(1, QString::number(fieldModelLoader->animUnknown(modelID, animID)));
		}
	}
}

void ModelManager::showModel(QTreeWidgetItem *item)
{
	if(modelPreview) {
		if(item == NULL) {
			modelPreview->clear();
			return;
		}
//		qDebug() << "showModel()" << item->text(0);
		modelPreview->load(field, models->currentItem()->text(0), item->text(0));
	}
}
