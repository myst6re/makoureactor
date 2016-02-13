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
	setWindowTitle(tr("Décor"));
	zoomFactor=1;
	imageFrame = new QFrame(this);
	imageFrame->setMinimumSize(320, 240);
	imageFrame->setAutoFillBackground(true);

	QPalette pal = imageFrame->palette();
	pal.setColor(QPalette::Active, QPalette::Window, Qt::black);
	pal.setColor(QPalette::Inactive, QPalette::Window, Qt::black);
	pal.setColor(QPalette::Disabled, QPalette::Window, .60*Qt::black);
	imageFrame->setPalette(pal);

	image = new ApercuBGLabel();
	image->setAlignment(Qt::AlignCenter);
	imageBox = new QScrollArea;
	imageBox->viewport()->installEventFilter(this);
	imageBox->setMinimumSize(320,240);
	imageBox->setPalette(pal);
	imageBox->setWidget(image);
	imageBox->setWidgetResizable(true);

	QVBoxLayout *imageLayout = new QVBoxLayout();
	imageLayout->addWidget(imageBox);
	imageLayout->setContentsMargins(0,0,0,0);
	imageFrame->setLayout(imageLayout);

	parametersWidget = new QComboBox(this);
	parametersWidget->setEnabled(false);
	statesWidget = new QListWidget(this);
	layersWidget = new QListWidget(this);
	sectionsWidget = new QListWidget(this);
	sectionsWidget->hide();
	zWidget = new QSpinBox(this);
	zWidget->setRange(0, 4096);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(new QLabel(tr("Z :")));
	hLayout->addWidget(zWidget, 1);

	QVBoxLayout *page1Layout = new QVBoxLayout;
	page1Layout->addWidget(layersWidget);
	page1Layout->addWidget(sectionsWidget);
	page1Layout->addLayout(hLayout);

	tabBar = new QTabBar(this);
	tabBar->addTab(tr("Couches"));
	tabBar->addTab(tr("Sections (couche 1)"));

	buttonRepair = new QPushButton(tr("Réparer"), this);
	buttonRepair->hide();

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(imageFrame, 0, 0, 5, 1);
	layout->addWidget(parametersWidget, 0, 1);
	layout->addWidget(statesWidget, 1, 1);
	layout->addWidget(tabBar, 2, 1);
	layout->addLayout(page1Layout, 3, 1);
	layout->addWidget(buttonRepair, 4, 1);
	layout->setColumnStretch(0, 2);
	layout->setColumnStretch(1, 1);

	connect(parametersWidget, SIGNAL(currentIndexChanged(int)), SLOT(parameterChanged(int)));
	connect(layersWidget, SIGNAL(itemSelectionChanged()), SLOT(layerChanged()));
	connect(sectionsWidget, SIGNAL(itemSelectionChanged()), SLOT(sectionChanged()));
	connect(statesWidget, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(enableState(QListWidgetItem*)));
	connect(layersWidget, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(enableLayer(QListWidgetItem*)));
	connect(sectionsWidget, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(enableSection(QListWidgetItem*)));
	connect(zWidget, SIGNAL(valueChanged(int)), SLOT(changeZ(int)));
	connect(buttonRepair, SIGNAL(released()), SLOT(tryToRepairBG()));
	connect(tabBar, SIGNAL(currentChanged(int)), SLOT(updateBG()));
	connect(tabBar, SIGNAL(currentChanged(int)), SLOT(showLayersPage(int)));
}

void BGDialog::fill(Field *field, bool reload)
{
	if((!reload && this->field == field) || !field)	return;

	this->field = field;

	fillWidgets();

	parameterChanged(0);
	layerChanged();
	image->setName(field->name());
	updateBG();
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
	sectionsWidget->clear();
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
	QSet<quint16> usedIDs;

	parametersWidget->clear();
	layersWidget->clear();
	sectionsWidget->clear();
	layers[0] = true;
	layers[3] = layers[2] = layers[1] = false;
	allparams.clear();
	params.clear();
	sections.clear();

	if(field->background()->usedParams(usedParams, layerExists, &usedIDs)) {

		QList<quint8> usedParamsList = usedParams.keys();
		qSort(usedParamsList);
		foreach(const quint8 param, usedParamsList) {
			parametersWidget->addItem(tr("Paramètre %1").arg(param), param);
		}
		parametersWidget->setEnabled(parametersWidget->count());

		QListWidgetItem *item;

		QList<quint16> usedIDsList = usedIDs.toList();
		qSort(usedIDsList);
		int sectionID = 0;
		foreach(const quint16 ID, usedIDsList) {
			item = new QListWidgetItem(tr("Section %1").arg(sectionID++));
			item->setData(Qt::UserRole, ID);
			item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setCheckState(Qt::Checked);
			sectionsWidget->addItem(item);
		}

		sections = usedIDs;

		layers[0] = true;
		layers[1] = layerExists[0];
		layers[2] = layerExists[1];
		layers[3] = layerExists[2];

		for(quint8 layerID = 0 ; layerID < 4 ; ++layerID) {
			if (layers[layerID]) {
				item = new QListWidgetItem(tr("Couche %1").arg(layerID));
				item->setData(Qt::UserRole, layerID);
				Qt::ItemFlags flags = Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
				if (layerID > 1) {
					flags |= Qt::ItemIsSelectable;
				}
				item->setFlags(flags);
				item->setCheckState(Qt::Checked);
				layersWidget->addItem(item);
			}
		}

		allparams = usedParams;

		params.clear();
		x[0] = -1;
		x[1] = -1;
		y[0] = -1;
		y[1] = -1;
		z[0] = -1;
		z[1] = -1;
		field->scriptsAndTexts()->bgParamAndBgMove(params, z, x, y);
	}
}

void BGDialog::showLayersPage(int index)
{
	bool page1 = index <= 0;

	layersWidget->setVisible(page1);
	sectionsWidget->setVisible(!page1);
	if(page1) {
		layerChanged();
	} else {
		sectionChanged();
	}
}

void BGDialog::parameterChanged(int index)
{
	int parameter = parametersWidget->itemData(index).toInt();
	quint8 states = allparams.value(parameter);
	QListWidgetItem *item;
	zoomFactor = 1;
	statesWidget->clear();
	for(int i=0 ; i<8 ; ++i) {
		if((states >> i) & 1) {
			item = new QListWidgetItem(tr("État %1").arg(i));
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
	zWidget->setReadOnly(false);
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

void BGDialog::sectionChanged()
{
	QList<QListWidgetItem *> items = sectionsWidget->selectedItems();
	if(items.isEmpty()) {
		zWidget->setValue(0);
		zWidget->setEnabled(false);
		return;
	}
	zWidget->setEnabled(true);
	zWidget->setReadOnly(true);
	int ID = items.first()->data(Qt::UserRole).toInt();
	zWidget->setValue(ID);
}

void BGDialog::enableState(QListWidgetItem *item)
{
	bool enabled = item->data(Qt::CheckStateRole).toBool();
	int parameter = parametersWidget->itemData(parametersWidget->currentIndex()).toInt(), state = item->data(Qt::UserRole).toInt();

	if(enabled)
		params.insert(parameter, params.value(parameter) | state);
	else
		params.insert(parameter, params.value(parameter) & ~state);

	updateBG();
}

void BGDialog::enableLayer(QListWidgetItem *item)
{
	bool enabled = item->data(Qt::CheckStateRole).toBool();
	int layer = item->data(Qt::UserRole).toInt();

	layers[layer] = enabled;

	updateBG();
}

void BGDialog::enableSection(QListWidgetItem *item)
{
	bool enabled = item->data(Qt::CheckStateRole).toBool();
	int ID = item->data(Qt::UserRole).toInt();

	if (enabled) {
		sections.insert(ID);
	} else {
		sections.remove(ID);
	}

	updateBG();
}

void BGDialog::changeZ(int value)
{
	if(tabBar->currentIndex() != 0) {
		return;
	}
	QList<QListWidgetItem *> items = layersWidget->selectedItems();
	if(items.isEmpty()) return;
	z[items.first()->data(Qt::UserRole).toInt()-2] = value;// z[layer - 2]

	updateBG();
}

void BGDialog::tryToRepairBG()
{
	if (field->background()->repair()) {
		QMessageBox::information(this, tr("Décor réparé"), tr("Des erreurs ont été trouvées et réparées, sauvegardez l'écran pour appliquer les changements."));
		emit modified();
		updateBG();
	} else {
		QMessageBox::warning(this, tr("Réparation impossible"), tr("Les erreurs n'ont pas été corrigées."));
	}
}

void BGDialog::updateBG()
{
	if(!field)	return;

	QImage img;
	bool bgWarning;

	if(tabBar->currentIndex() == 0) {
		img = field->background()->openBackground(params, z, layers, NULL, &bgWarning);
	} else {
		bool layers[4] = { false, true, false, false };
		img = field->background()->openBackground(params, z, layers, &sections, &bgWarning);
	}

	if(img.isNull()) {
		image->setPixmap(QPixmap::fromImage(img));
		bgWarning = false;
	} else {
		image->setPixmap(QPixmap::fromImage(img)
						 .scaled(imageFrame->width() * zoomFactor, (imageFrame->height() -4) * zoomFactor,
		                         Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}

	buttonRepair->setVisible(bgWarning);
}

void BGDialog::resizeEvent(QResizeEvent *event)
{
	if(event->type()==QEvent::Resize)
	{
		updateBG();
	}
}

bool BGDialog::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() == QEvent::Wheel && obj == imageBox->viewport())
	{
		QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
		if(wheelEvent->modifiers()==Qt::CTRL)
		{
			if(wheelEvent->delta() > 0)
			{
				if(zoomFactor==0)
				{
					zoomFactor=0.25;
				}
				else if (zoomFactor==4){return false;}/*cap zoom in at 400%*/
				else
				{
					zoomFactor+=0.25;
				}
			}
			else if(wheelEvent->delta() < 0)
			{
				if(zoomFactor==0)
				{
					zoomFactor= 0.25;
				}
				else if (zoomFactor == 0.25){return false;}/*cap zoom out at 25% */
				else
				{
					zoomFactor+= -0.25;
				}
			}
			else{return false;}/* A delta of 0 should never happen */
			updateBG();
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

