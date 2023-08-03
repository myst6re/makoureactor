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
#include "ModelManager.h"
#include "core/Config.h"
#include "core/field/Field.h"
#include "Data.h"

ModelManager::ModelManager(QWidget *parent) :
    QDialog(parent, Qt::Tool), _field(nullptr), fieldModelLoader(nullptr)
{
	setWindowTitle(tr("Field Models"));

	QFont font;
	font.setPointSize(8);

	models = new QTreeWidget();
	models->setIndentation(0);
	models->setHeaderLabel(tr("Field Models"));
	models->setFixedWidth(
	    models->fontMetrics().boundingRect("WWWWWWWWWWWWWWW").width());
	models->setColumnCount(1);

	modelFrame = new QFrame();
	modelFrame->setFrameShape(QFrame::StyledPanel);

	modelUnknown = new QSpinBox();
	modelUnknown->setRange(0, 65535);

	modelScaleWidget = new QSpinBox();
	modelScaleWidget->setRange(0, 4096);

	modelGlobalColorWidget = new ColorDisplay();
	modelGlobalColorWidget->setMinimumSize(modelScaleWidget->minimumSizeHint());

	modelColorsLayout = new ModelColorsLayout();

	modelAnims = new QTreeWidget();
	modelAnims->setIndentation(0);
	modelAnims->setFixedWidth(
	    models->fontMetrics().boundingRect("WWWWWWWWWWWWWWW").width());
	modelAnims->setEditTriggers(QAbstractItemView::NoEditTriggers);

	if (Config::value("OpenGL", true).toBool()) {
		modelPreview = new FieldModel();
		modelWidget = modelPreview;
		modelPreview->setFixedSize(304, 214);
	} else {
		modelPreview = nullptr;
		modelWidget = new QWidget(this);
	}

	connect(models, &QTreeWidget::currentItemChanged, this, qOverload<QTreeWidgetItem *, QTreeWidgetItem *>(&ModelManager::showModelInfos));
	connect(modelAnims, &QTreeWidget::currentItemChanged, this, &ModelManager::showModel);
	connect(modelUnknown, &QSpinBox::valueChanged, this, &ModelManager::setModelUnknown);
	connect(modelScaleWidget, &QSpinBox::valueChanged, this, qOverload<int>(&ModelManager::setModelScale));
	connect(modelGlobalColorWidget, &ColorDisplay::colorEdited, this, &ModelManager::setModelGlobalColor);
	connect(modelColorsLayout, &ModelColorsLayout::colorDirEdited, this, &ModelManager::setModelColor);
}

void ModelManager::clear()
{
	models->blockSignals(true);
	modelAnims->blockSignals(true);

	models->clear();
	modelAnims->clear();
	if (modelPreview)	modelPreview->clear();

	models->blockSignals(false);
	modelAnims->blockSignals(false);
}

void ModelManager::fill(Field *field, bool reload)
{
	if (!reload && _field == field)	return;

	clear();

	if (!field->fieldModelLoader()->isOpen()) {
		return;
	}

	fieldModelLoader = field->fieldModelLoader();
	_field = field;

	fillModelList();

	QTreeWidgetItem *currentItem = models->topLevelItem(Data::currentModelID);
	if (currentItem) {
		models->setCurrentItem(currentItem);
	} else {
		models->setCurrentItem(models->topLevelItem(0));
	}
	models->setFocus();

	modelFrame->setEnabled(models->topLevelItemCount() != 0);
}

void ModelManager::fillModelList()
{
	models->blockSignals(true);
	for (const QStringList &name : modelNames()) {
		QTreeWidgetItem *item = new QTreeWidgetItem(name);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
		models->addTopLevelItem(item);
	}
	models->blockSignals(false);
}

int ModelManager::currentModelID(QTreeWidgetItem *item) const
{
	if (!item) {
		item = models->currentItem();
		if (!item)	return -1;
	}
	return models->indexOfTopLevelItem(item);
}

QList<int> ModelManager::selectedModelIDs() const
{
	QList<int> modelIDs;
	for (QTreeWidgetItem *item : models->selectedItems()) {
		modelIDs.append(models->indexOfTopLevelItem(item));
	}
	return modelIDs;
}

int ModelManager::currentAnimID(QTreeWidgetItem *item) const
{
	if (!item) {
		item = modelAnims->currentItem();
		if (!item)	return -1;
	}
	return modelAnims->indexOfTopLevelItem(item);
}

void ModelManager::showModelInfos(QTreeWidgetItem *item, QTreeWidgetItem *previous)
{
	Q_UNUSED(previous)
	if (item == nullptr) {
		modelFrame->setEnabled(false);
		return;
	}
	modelFrame->setEnabled(true);
	int modelID = currentModelID(item);
	modelAnims->clear();

	if (modelID < 0 || modelID >= fieldModelLoader->modelCount()) {
		return;
	}

	modelAnims->addTopLevelItems(animItems(modelID));

	for (int i=0; i<modelAnims->columnCount(); ++i) {
		modelAnims->resizeColumnToContents(i);
	}

	showModelInfos2(modelID);
	
	if (modelAnims->topLevelItemCount() > 0) {
		modelAnims->setCurrentItem(modelAnims->topLevelItem(0));
	}
}

void ModelManager::showModelInfos2(int row)
{
	Q_UNUSED(row)

	modelUnknown->blockSignals(true);
	modelScaleWidget->blockSignals(true);
	modelColorsLayout->blockSignals(true);

	int modelID = currentModelID();

	modelUnknown->setValue(fieldModelLoader->unknown(modelID));
	modelColorsLayout->setModelColorDirs(lightColors(modelID));
	modelGlobalColorWidget->setColors(QList<QRgb>() << globalColor(modelID));
	modelScaleWidget->setValue(modelScale(modelID));

	modelColorsLayout->blockSignals(false);
	modelScaleWidget->blockSignals(false);
	modelUnknown->blockSignals(false);
}

void ModelManager::setModelUnknown(int unknown)
{
	int modelID = currentModelID();
	if (modelID < 0) {
		return;
	}

	fieldModelLoader->setUnknown(modelID, quint16(unknown));

	emit modified();
}

void ModelManager::setModelScale(int scale)
{
	int modelID = currentModelID();
	if (modelID < 0)	return;

	setModelScale(modelID, quint16(scale));

	emit modified();
}

void ModelManager::setModelGlobalColor(int id, QRgb color)
{
	Q_UNUSED(id)
	int modelID = currentModelID();
	if (modelID < 0)	return;

	setGlobalColor(modelID, color);

	emit modified();
}

void ModelManager::setModelColor(int id, const FieldModelColorDir &color)
{
	int modelID = currentModelID();
	if (modelID < 0)	return;

	setLightColor(modelID, id, color);

	emit modified();
}

void ModelManager::showModel(QTreeWidgetItem *item)
{
	if (modelPreview) {
		if (item == nullptr) {
			modelPreview->clear();
			return;
		}
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
