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
#include "ModelManagerPS.h"
#include "core/Config.h"
#include "Data.h"

ModelManagerPS::ModelManagerPS(QWidget *parent) :
    ModelManager(parent)
{
	modelAnims->setColumnCount(1);
	modelAnims->setHeaderLabels(QStringList() << tr("Animation"));

	QGridLayout *frameLayout = new QGridLayout(modelFrame);
	frameLayout->addWidget(new QLabel(tr("Unknown")), 0, 0);
	frameLayout->addWidget(modelUnknown, 0, 1);
	frameLayout->addWidget(new QLabel(tr("Model size")), 1, 0);
	frameLayout->addWidget(modelScaleWidget, 1, 1);
	frameLayout->addWidget(new QLabel(tr("Global light")), 2, 0);
	frameLayout->addWidget(modelGlobalColorWidget, 2, 1);
	frameLayout->addWidget(new QLabel(tr("Directional light")), 3, 0);
	frameLayout->addLayout(modelColorsLayout, 4, 0, 1, 2);
	frameLayout->addWidget(modelAnims, 0, 2, 6, 1);
	frameLayout->addWidget(modelWidget, 0, 3, 6, 1);
	frameLayout->setRowStretch(5, 1);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(models, 0, 0);
	layout->addWidget(modelFrame, 0, 1);
	layout->setColumnStretch(1, 1);

	adjustSize();
}

QList<QStringList> ModelManagerPS::modelNames() const
{
	QList<QStringList> ret;
	for (int modelId=0; modelId<modelLoader()->modelCount(); ++modelId) {
		ret.append(QStringList(QString("Model %1").arg(modelId)));
	}
	return ret;
}

QList<QTreeWidgetItem *> ModelManagerPS::animItems(int modelID) const
{
	QList<QTreeWidgetItem *> ret;
	for (int animId=0; animId<modelLoader()->animCount(modelID); ++animId) {
		QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(QString("Animation %1").arg(animId)));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		ret.append(item);
	}
	return ret;
}

FieldModelFile *ModelManagerPS::modelData(QTreeWidgetItem *item)
{
	return field()->fieldModel(currentModelID(), currentAnimID(item));
}

FieldModelLoaderPS *ModelManagerPS::modelLoader() const
{
	return static_cast<FieldModelLoaderPS *>(ModelManager::modelLoader());
}

FieldPS *ModelManagerPS::field() const
{
	return static_cast<FieldPS *>(ModelManager::field());
}

const QList<FieldModelColorDir> &ModelManagerPS::lightColors(int modelID) const
{
	return field()->fieldModel(modelID, 0)->lightColors();
}

void ModelManagerPS::setLightColor(int modelID, int id, const FieldModelColorDir &color)
{
	field()->fieldModel(modelID, 0)->setLightColor(id, color);
}

QRgb ModelManagerPS::globalColor(int modelID) const
{
	return field()->fieldModel(modelID, 0)->globalColor();
}

void ModelManagerPS::setGlobalColor(int modelID, QRgb color)
{
	field()->fieldModel(modelID, 0)->setGlobalColor(color);
}

quint16 ModelManagerPS::modelScale(int modelID) const
{
	return field()->fieldModel(modelID, 0)->scale();
}

void ModelManagerPS::setModelScale(int modelID, quint16 scale)
{
	field()->fieldModel(modelID, 0)->setScale(scale);
}
