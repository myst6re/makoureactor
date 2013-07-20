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
#include "core/Config.h"
#include "Data.h"

ModelManager::ModelManager(const QGLWidget *shareWidget, QWidget *parent) :
	QDialog(parent, Qt::Tool), _field(0), fieldModelLoader(0)
{
	setWindowTitle(tr("Modèles 3D"));

	QFont font;
	font.setPointSize(8);

	models = new QTreeWidget();
	models->setIndentation(0);
	models->setHeaderLabel(tr("Modèles 3D"));
	models->setFixedWidth(120);

	modelFrame = new QFrame();
	modelFrame->setFrameShape(QFrame::StyledPanel);

	modelUnknown = new QSpinBox();
	modelUnknown->setRange(0, 65535);

	modelScaleWidget = new QSpinBox();
	modelScaleWidget->setRange(0, 4096);
	modelColorDisplay = new ColorDisplay();
	modelColorLabel = new QLabel();
	QPalette modelColorLabelPalette = modelColorLabel->palette();
	modelColorLabelPalette.setColor(QPalette::Active, QPalette::WindowText,
									modelColorLabelPalette.color(QPalette::Disabled, QPalette::WindowText));
	modelColorLabelPalette.setColor(QPalette::Inactive, QPalette::WindowText,
									modelColorLabelPalette.color(QPalette::Disabled, QPalette::WindowText));
	modelColorLabel->setPalette(modelColorLabelPalette);

	modelAnims = new QTreeWidget();
	modelAnims->setColumnCount(2);
	modelAnims->setIndentation(0);
	modelAnims->setFixedWidth(120);

	if(Config::value("OpenGL", true).toBool()) {
		modelPreview = new FieldModel(0, shareWidget);
		modelWidget = modelPreview;
		modelPreview->setFixedSize(304, 214);
	} else {
		modelPreview = 0;
		modelWidget = new QWidget(this);
	}

	connect(models, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(showModelInfos(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(modelAnims, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(showModel(QTreeWidgetItem*)));
	connect(modelUnknown, SIGNAL(valueChanged(int)), SLOT(setModelUnknown(int)));
	connect(modelColorDisplay, SIGNAL(colorHovered(int)), SLOT(setModelColorLabel(int)));
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

void ModelManager::fill(Field *field, bool reload)
{
	if(!reload && _field == field)	return;

	clear();

	if(!field->fieldModelLoader()->isOpen()) {
		return;
	}

	fieldModelLoader = field->fieldModelLoader();
	_field = field;

	fill2();
}

void ModelManager::fill2()
{
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
	foreach(const QStringList &name, modelNames())
	{
		item = new QTreeWidgetItem(name);
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

QList<int> ModelManager::selectedModelIDs() const
{
	QList<int> modelIDs;
	foreach(QTreeWidgetItem *item, models->selectedItems()) {
		modelIDs.append(models->indexOfTopLevelItem(item));
	}
	return modelIDs;
}

int ModelManager::currentAnimID(QTreeWidgetItem *item) const
{
	if(!item) {
		item = modelAnims->currentItem();
		if(!item)	return -1;
	}
	return modelAnims->indexOfTopLevelItem(item);
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
	foreach(const QStringList &animName, animNames(row)) {
		item = new QTreeWidgetItem(animName);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
		modelAnims->addTopLevelItem(item);
		++numA;
	}

	for(int i=0 ; i<modelAnims->columnCount() ; ++i) {
		modelAnims->resizeColumnToContents(i);
	}

	showModelInfos2(row);

	if(modelAnims->topLevelItemCount() > 0)
		modelAnims->setCurrentItem(modelAnims->topLevelItem(0));
}

void ModelManager::showModelInfos2(int row)
{
	modelUnknown->blockSignals(true);

	modelUnknown->setValue(fieldModelLoader->unknown(row));

	modelUnknown->blockSignals(false);
}

void ModelManager::setModelUnknown(int unknown)
{
	int modelID = currentModelID();
	if(modelID < 0)	return;

	fieldModelLoader->setUnknown(modelID, unknown);

	emit modified();
}

void ModelManager::setModelColorLabel(int colorId)
{
	int modelID = currentModelID();
	if(modelID < 0)	return;

	modelColorLabel->setText(QString("#%1")
							 .arg(lightColors(modelID)
								  .value(colorId) & 0xFFFFFF, 6, 16)
							 .toUpper());
}

void ModelManager::showModel(QTreeWidgetItem *item)
{
	if(modelPreview) {
		if(item == NULL) {
			modelPreview->clear();
			return;
		}
//		qDebug() << "showModel()" << item->text(0);
		modelPreview->setFieldModelFile(modelData(item));
	}
}

FieldModelLoader *ModelManager::modelLoader() const
{
	return fieldModelLoader;
}

Field *ModelManager::field() const
{
	return _field;
}
