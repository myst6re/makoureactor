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
#include "ModelManagerPS.h"
#include "core/Config.h"
#include "Data.h"

ModelManagerPS::ModelManagerPS(const QGLWidget *shareWidget, QWidget *parent) :
	ModelManager(shareWidget, parent)
{
	models->setColumnCount(1);
	modelAnims->setColumnCount(1);
	modelAnims->setHeaderLabels(QStringList() << tr("Animations") << tr("?"));

	QGridLayout *frameLayout = new QGridLayout(modelFrame);
	frameLayout->addWidget(new QLabel(tr("Inconnu")), 0, 0);
	frameLayout->addWidget(modelUnknown, 0, 1);
	frameLayout->addWidget(modelAnims, 0, 2);
	frameLayout->addWidget(modelWidget, 0, 3);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(models, 0, 0);
	layout->addWidget(modelFrame, 0, 1);

	adjustSize();
}

void ModelManagerPS::fill2()
{
	ModelManager::fill2();
}

QList<QStringList> ModelManagerPS::modelNames() const
{
	QList<QStringList> ret;
	for(int modelId=0 ; modelId<modelLoader()->modelCount() ; ++modelId) {
		ret.append(QStringList(QString("Modèle %1").arg(modelId)));
	}
	return ret;
}

QList<QStringList> ModelManagerPS::animNames(int row) const
{
	QList<QStringList> ret;
	for(int animId=0 ; animId<modelLoader()->animCount(row) ; ++animId) {
		ret.append(QStringList(QString("Animation %1").arg(animId)));
	}
	return ret;
}

void ModelManagerPS::showModelInfos2(int row)
{
	ModelManager::showModelInfos2(row);
}

void ModelManagerPS::showModel2(QTreeWidgetItem *item)
{
	modelPreview->load(field(), currentModelID(), currentAnimID(item));
}

FieldModelLoaderPS *ModelManagerPS::modelLoader() const
{
	return (FieldModelLoaderPS *)ModelManager::modelLoader();
}

FieldPS *ModelManagerPS::field() const
{
	return (FieldPS *)ModelManager::field();
}
