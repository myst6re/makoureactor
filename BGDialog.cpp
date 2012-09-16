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
#include "BGDialog.h"

BGDialog::BGDialog(QWidget *parent) :
	QDialog(parent, Qt::Tool), field(0)
{
	setWindowTitle(tr("Aperçu écran"));

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setMinimumSize(320, 240);

	QPalette pal = scrollArea->palette();
	pal.setColor(QPalette::Active, QPalette::Window, Qt::black);
	pal.setColor(QPalette::Inactive, QPalette::Window, Qt::black);
	pal.setColor(QPalette::Disabled, QPalette::Window, .60*Qt::black);
	scrollArea->setPalette(pal);

	image = new ApercuBGLabel();
	image->setAlignment(Qt::AlignCenter);
	scrollArea->setWidget(image);

	parametersWidget = new QComboBox(this);
	parametersWidget->setFixedWidth(150);
	parametersWidget->setEnabled(false);
	statesWidget = new QListWidget(this);
	statesWidget->setFixedWidth(150);
	layersWidget = new QListWidget(this);
	layersWidget->setFixedWidth(150);
	zWidget = new QSpinBox(this);
	zWidget->setRange(0, 4096);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(new QLabel(tr("Z :")));
	hLayout->addWidget(zWidget, 1);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(scrollArea, 0, 0, 4, 1);
	layout->addWidget(parametersWidget, 0, 1, Qt::AlignRight);
	layout->addWidget(statesWidget, 1, 1, Qt::AlignRight);
	layout->addWidget(layersWidget, 2, 1, Qt::AlignRight);
	layout->addLayout(hLayout, 3, 1, Qt::AlignRight);
	layout->setColumnStretch(0, 1);

	connect(parametersWidget, SIGNAL(currentIndexChanged(int)), SLOT(parameterChanged(int)));
	connect(layersWidget, SIGNAL(itemSelectionChanged()), SLOT(layerChanged()));
	connect(statesWidget, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(enableState(QListWidgetItem*)));
	connect(layersWidget, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(enableLayer(QListWidgetItem*)));
	connect(zWidget, SIGNAL(valueChanged(int)), SLOT(changeZ(int)));
}

void BGDialog::fill(Field *field)
{
	if(this->field == field || !field)	return;

	this->field = field;

	setWindowTitle(tr("Aperçu %1").arg(field->getName()));

	fillWidgets();

	parameterChanged(0);
	layerChanged();
	fill();
}

void BGDialog::clear()
{
	parametersWidget->blockSignals(true);
	statesWidget->blockSignals(true);
	layersWidget->blockSignals(true);
	zWidget->blockSignals(true);

	field = 0;
	allparams.clear();
	params.clear();
	image->clear();
	parametersWidget->clear();
	statesWidget->clear();
	layersWidget->clear();
	zWidget->clear();

	parametersWidget->blockSignals(false);
	statesWidget->blockSignals(false);
	layersWidget->blockSignals(false);
	zWidget->blockSignals(false);
}

void BGDialog::fillWidgets()
{
	if(!field)	return;

	QHash<quint8, quint8> usedParams;
	bool layerExists[] = {false, false, false};

	parametersWidget->clear();
	layersWidget->clear();
	layers[0] = true;
	layers[3] = layers[2] = layers[1] = false;
	allparams.clear();
	params.clear();

	if(field->getUsedParams(usedParams, layerExists))
	{
		foreach(const quint8 &param, usedParams.keys()) {
			parametersWidget->addItem(tr("Paramètre %1").arg(param), param);
		}
		parametersWidget->setEnabled(parametersWidget->count());

		QListWidgetItem *item;

		item = new QListWidgetItem(tr("Couche 1"));
		item->setData(Qt::UserRole, 0);
		item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		item->setCheckState(Qt::Checked);
		layersWidget->addItem(item);
		if(layerExists[0]) {
			item = new QListWidgetItem(tr("Couche 2"));
			item->setData(Qt::UserRole, 1);
			item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			item->setCheckState(Qt::Checked);
			layersWidget->addItem(item);
		}
		if(layerExists[1]) {
			item = new QListWidgetItem(tr("Couche 3"));
			item->setData(Qt::UserRole, 2);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			item->setCheckState(Qt::Checked);
			layersWidget->addItem(item);
		}
		if(layerExists[2]) {
			item = new QListWidgetItem(tr("Couche 4"));
			item->setData(Qt::UserRole, 3);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			item->setCheckState(Qt::Checked);
			layersWidget->addItem(item);
		}
		layers[0] = true;
		layers[1] = layerExists[0];
		layers[2] = layerExists[1];
		layers[3] = layerExists[2];
		allparams = usedParams;

		params.clear();
		x[0] = -1;
		x[1] = -1;
		y[0] = -1;
		y[1] = -1;
		z[0] = -1;
		z[1] = -1;
		field->scriptsAndTexts()->getBgParamAndBgMove(params, z, x, y);
	}
}

void BGDialog::parameterChanged(int index)
{
	int parameter = parametersWidget->itemData(index).toInt();
	quint8 states = allparams.value(parameter);
	QListWidgetItem *item;

	statesWidget->clear();
	for(int i=0 ; i<8 ; ++i) {
		if((states >> i) & 1) {
			item = new QListWidgetItem(tr("État %1").arg(i+1));
			item->setData(Qt::UserRole, 1 << i);
			item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			item->setCheckState((params.value(parameter) >> i) & 1 ? Qt::Checked : Qt::Unchecked);
			statesWidget->addItem(item);
		}
	}
}

void BGDialog::layerChanged()
{
	QList<QListWidgetItem *> items = layersWidget->selectedItems();
	if(items.isEmpty()) {
		zWidget->setValue(0);
		zWidget->setEnabled(false);
		return;
	}
	zWidget->setEnabled(true);
	int layer = items.first()->data(Qt::UserRole).toInt();
	int zValue = z[layer-2];
	if(zValue == -1) {
		switch(layer) {
		case 2:
			zValue = 4096;
			break;
		case 3:
			zValue = 0;
			break;
		}
	}

	zWidget->setValue(zValue);
}

void BGDialog::enableState(QListWidgetItem *item)
{
	bool enabled = item->data(Qt::CheckStateRole).toBool();
	int parameter = parametersWidget->itemData(parametersWidget->currentIndex()).toInt(), state = item->data(Qt::UserRole).toInt();

	if(enabled)
		params.insert(parameter, params.value(parameter) | state);
	else
		params.insert(parameter, params.value(parameter) & ~state);

	fill();
}

void BGDialog::enableLayer(QListWidgetItem *item)
{
	bool enabled = item->data(Qt::CheckStateRole).toBool();
	int layer = item->data(Qt::UserRole).toInt();

	layers[layer] = enabled;

	fill();
}

void BGDialog::changeZ(int value)
{
	QList<QListWidgetItem *> items = layersWidget->selectedItems();
	if(items.isEmpty()) return;
	z[items.first()->data(Qt::UserRole).toInt()-2] = value;// z[layer - 2]

	fill();
}

void BGDialog::fill()
{
	if(!field)	return;
	image->setPixmap(field->openBackground(params, z, layers));
	image->setName(field->getName());
}
