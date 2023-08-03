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
#include "core/field/AFile.h"

ModelManagerPS::ModelManagerPS(QWidget *parent) :
    ModelManager(parent)
{
	modelAnims->setColumnCount(1);
	modelAnims->setHeaderLabels(QStringList() << tr("Animation"));

	toolBar2 = new QToolBar();
	toolBar2->setIconSize(QSize(14, 14));
	toolBar2->addAction(QIcon::fromTheme(QStringLiteral("document-export")), QString(), this, &ModelManagerPS::exportAnim);

	QGridLayout *frameLayout = new QGridLayout(modelFrame);
	frameLayout->addWidget(new QLabel(tr("Unknown")), 0, 0);
	frameLayout->addWidget(modelUnknown, 0, 1);
	frameLayout->addWidget(new QLabel(tr("Model size")), 1, 0);
	frameLayout->addWidget(modelScaleWidget, 1, 1);
	frameLayout->addWidget(new QLabel(tr("Global light")), 2, 0);
	frameLayout->addWidget(modelGlobalColorWidget, 2, 1);
	frameLayout->addWidget(new QLabel(tr("Directional light")), 3, 0);
	frameLayout->addLayout(modelColorsLayout, 4, 0, 1, 2);
	frameLayout->addWidget(toolBar2, 0, 2);
	frameLayout->addWidget(modelAnims, 1, 2, 5, 1);
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

void ModelManagerPS::exportAnim()
{
	int modelID = currentModelID();
	if (modelID < 0) {
		return;
	}

	QTreeWidgetItem *item = modelAnims->currentItem();
	if (item == nullptr) {
		return;
	}

	int animID = currentAnimID(item);
	if (animID < 0) {
		return;
	}
	
	QString filePath = QString("%1/%2-model-%3-animation-%4.a")
	                       .arg(
	                           Config::value("exportPath").toString().isEmpty() ? field()->io()->directory() : Config::value("exportPath").toString(),
	                           field()->name()
	                       )
	                       .arg(modelID)
	                       .arg(animID);
	filePath = QFileDialog::getSaveFileName(this, tr("Export animation"), filePath, tr("FF7 PC animation file (*.a)"));
	
	if (filePath.isNull()) {
		return;
	}
	
	bool ok = false;
	FieldModelAnimation animation = field()->fieldModel(modelID)->animation(animID).toPC(&ok);
	
	if (!ok) {
		QMessageBox::warning(this, tr("Cannot convert"), tr("This animation cannot be converted to PC format"));
		return;
	}
	
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QMessageBox::warning(this, tr("Cannot export"), tr("The destination file cannot be opened for writing"));
		return;
	}

	AFile a(&file);
	if (!a.write(animation)) {
		QMessageBox::warning(this, tr("Cannot export"), tr("Unable to write into the destination file"));
		return;
	}
}
