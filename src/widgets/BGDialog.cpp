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
#include "BGDialog.h"
#include "core/Config.h"
#include "core/field/Field.h"

BGDialog::BGDialog(QWidget *parent) :
    QDialog(parent, Qt::Tool), _field(nullptr), editorPage(nullptr), palettesPage(nullptr),
    zoomFactor(1.0f)
{
	setWindowTitle(tr("Background"));

	viewerPage = new QWidget();

	image = new PreviewBGLabel();
	image->setAlignment(Qt::AlignCenter);

	imageBox = new QScrollArea(this);
	imageBox->setAlignment(Qt::AlignCenter);
	imageBox->viewport()->installEventFilter(this);
	imageBox->setMinimumSize(320, 240);
	imageBox->setWidget(image);
	imageBox->setWidgetResizable(true);
	imageBox->setBackgroundRole(QPalette::Dark);

	QPalette pal = imageBox->palette();
	pal.setColor(QPalette::Active, QPalette::Dark, Qt::black);
	pal.setColor(QPalette::Inactive, QPalette::Dark, Qt::black);
	pal.setColor(QPalette::Disabled, QPalette::Dark, qRgb(50, 50, 50));
	imageBox->setPalette(pal);

	parametersWidget = new QComboBox(this);
	parametersWidget->setEnabled(false);
	statesWidget = new QListWidget(this);
	layersWidget = new QListWidget(this);
	sectionsWidget = new QListWidget(this);
	sectionsWidget->hide();
	zWidget = new QSpinBox(this);
	zWidget->setRange(0, 4096);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(new QLabel(tr("Z:")));
	hLayout->addWidget(zWidget, 1);

	QVBoxLayout *page1Layout = new QVBoxLayout;
	page1Layout->addWidget(layersWidget);
	page1Layout->addWidget(sectionsWidget);
	page1Layout->addLayout(hLayout);

	tabBar = new QTabBar(this);
	tabBar->setDrawBase(false);
	tabBar->addTab(tr("Layers"));
	tabBar->addTab(tr("Sections (layer 1)"));

	buttonRepair = new QPushButton(tr("Repair"), this);
	buttonRepair->hide();

	QGridLayout *layout = new QGridLayout(viewerPage);
	layout->addWidget(imageBox, 0, 0, 5, 1);
	layout->addWidget(parametersWidget, 0, 1);
	layout->addWidget(statesWidget, 1, 1);
	layout->addWidget(tabBar, 2, 1);
	layout->addLayout(page1Layout, 3, 1);
	layout->addWidget(buttonRepair, 4, 1);
	layout->setColumnStretch(0, 2);
	layout->setColumnStretch(1, 1);
	
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	mainTabBar = new QTabBar(this);
	mainTabBar->addTab(tr("Viewer"));
	mainTabBar->addTab(tr("Editor"));
	mainTabBar->addTab(tr("Palettes"));
	mainTabBar->setExpanding(false);
	mainTabBar->setDrawBase(false);

	stackedLayout = new QStackedLayout;
	stackedLayout->addWidget(viewerPage);

	mainLayout->addWidget(mainTabBar, 0, mainTabBar->documentMode() ? Qt::AlignCenter : Qt::Alignment());
	mainLayout->addLayout(stackedLayout);

	connect(image, &PreviewBGLabel::saveRequested, this, &BGDialog::saveImage);
	connect(parametersWidget, &QComboBox::currentIndexChanged, this, &BGDialog::parameterChanged);
	connect(layersWidget, &QListWidget::itemSelectionChanged, this, &BGDialog::layerChanged);
	connect(sectionsWidget, &QListWidget::itemSelectionChanged, this, &BGDialog::sectionChanged);
	connect(statesWidget, &QListWidget::itemChanged, this, &BGDialog::enableState);
	connect(layersWidget, &QListWidget::itemChanged, this, &BGDialog::enableLayer);
	connect(sectionsWidget, &QListWidget::itemChanged, this, &BGDialog::enableSection);
	connect(zWidget, &QSpinBox::valueChanged, this, &BGDialog::changeZ);
	connect(buttonRepair, &QPushButton::clicked, this, &BGDialog::tryToRepairBG);
	connect(tabBar, &QTabBar::currentChanged, this, &BGDialog::updateBG);
	connect(tabBar, &QTabBar::currentChanged, this, &BGDialog::showLayersPage);
	connect(mainTabBar, &QTabBar::currentChanged, this, &BGDialog::setCurrentPage);

	if (Config::value("backgroundEditorSize").toSize() != QSize(0, 0)) {
		resize(Config::value("backgroundEditorSize").toSize());
	}
}

void BGDialog::createEditorPage()
{
	editorPage = new BackgroundEditor();
	
	connect(editorPage, &BackgroundEditor::modified, this, &BGDialog::updateBG);
	connect(editorPage, &BackgroundEditor::modified, this, &BGDialog::modified);

	stackedLayout->addWidget(editorPage);
}

void BGDialog::createPalettesPage()
{
	palettesPage = new BackgroundPaletteEditor();

	connect(palettesPage, &BackgroundPaletteEditor::modified, this, &BGDialog::updateBG);
	connect(palettesPage, &BackgroundPaletteEditor::modified, this, &BGDialog::modified);

	stackedLayout->addWidget(palettesPage);
}

void BGDialog::setCurrentPage(int index)
{
	if (index >= 1 && editorPage == nullptr) {
		if (editorPage == nullptr) {
			createEditorPage();
		}
		if (palettesPage == nullptr) {
			createPalettesPage();
		}
	}

	stackedLayout->setCurrentIndex(index);
	
	if (_field != nullptr) {
		if (index == 1) {
			editorPage->setBackgroundFile(_field->background());
		} else if (index == 2) {
			palettesPage->setBackgroundFile(_field->background());
		}
	}
}

void BGDialog::saveConfig()
{
	Config::setValue("backgroundEditorSize", size());
	if (editorPage != nullptr) {
		editorPage->saveConfig();
	}
}

void BGDialog::fill(Field *field, bool reload)
{
	if ((!reload && _field == field) || field == nullptr) {
		return;
	}

	_field = field;
	if (editorPage != nullptr) {
		editorPage->clear();
	}

	fillWidgets();

	parameterChanged(0);
	layerChanged();
	updateBG();
}

void BGDialog::clear()
{
	parametersWidget->blockSignals(true);
	statesWidget->blockSignals(true);
	layersWidget->blockSignals(true);
	zWidget->blockSignals(true);
	if (editorPage != nullptr) {
		editorPage->blockSignals(true);
	}

	_field = nullptr;
	allparams.clear();
	params.clear();
	image->clear();
	parametersWidget->clear();
	statesWidget->clear();
	layersWidget->clear();
	sectionsWidget->clear();
	zWidget->clear();
	if (editorPage != nullptr) {
		editorPage->clear();
	}

	parametersWidget->blockSignals(false);
	statesWidget->blockSignals(false);
	layersWidget->blockSignals(false);
	zWidget->blockSignals(false);
	if (editorPage != nullptr) {
		editorPage->blockSignals(false);
	}
}

void BGDialog::fillWidgets()
{
	if (_field == nullptr) {
		return;
	}

	QMap<LayerParam, quint8> usedParams;
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

	if (_field->background()->usedParams(usedParams, layerExists, &usedIDs)) {

		QList<LayerParam> usedParamsList = usedParams.keys();
		int index = 0;
		for (const LayerParam &param : std::as_const(usedParamsList)) {
			parametersWidget->addItem(tr("Parameter %1").arg(param.param), param.param);
			parametersWidget->setItemData(index, param.layer, Qt::UserRole + 1);
			++index;
		}
		parametersWidget->setEnabled(index > 0);

		QListWidgetItem *item;

		QList<quint16> usedIDsList = usedIDs.values();
		std::sort(usedIDsList.begin(), usedIDsList.end(), std::greater<>());
		int sectionID = 0;
		for (const quint16 ID : std::as_const(usedIDsList)) {
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

		for (quint8 layerID = 0; layerID < 4; ++layerID) {
			if (layers[layerID]) {
				item = new QListWidgetItem(tr("Layer %1").arg(layerID));
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
		_field->scriptsAndTexts()->bgParamAndBgMove(params, z, x, y);

		if (editorPage != nullptr && mainTabBar->currentIndex() == 1) {
			editorPage->setBackgroundFile(_field->background());
		}
		if (palettesPage != nullptr && mainTabBar->currentIndex() == 2) {
			palettesPage->setBackgroundFile(_field->background());
		}
	}
}

void BGDialog::showLayersPage(int index)
{
	bool page1 = index <= 0;

	layersWidget->setVisible(page1);
	sectionsWidget->setVisible(!page1);
	if (page1) {
		layerChanged();
	} else {
		sectionChanged();
	}
}

void BGDialog::parameterChanged(int index)
{
	LayerParam layerParam;
	layerParam.param = quint8(parametersWidget->itemData(index).toInt());
	layerParam.layer = quint8(parametersWidget->itemData(index, Qt::UserRole + 1).toInt());
	quint8 states = allparams.value(layerParam);
	QListWidgetItem *item;
	statesWidget->clear();
	for (int i=0; i<8; ++i) {
		if ((states >> i) & 1) {
			item = new QListWidgetItem(tr("State %1").arg(i));
			item->setData(Qt::UserRole, 1 << i);
			item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			item->setCheckState(((params.value(layerParam.param) >> i) & 1) ? Qt::Checked : Qt::Unchecked);
			statesWidget->addItem(item);
		}
	}
}

void BGDialog::layerChanged()
{
	QList<QListWidgetItem *> items = layersWidget->selectedItems();
	if (items.isEmpty()) {
		zWidget->setValue(0);
		zWidget->setEnabled(false);
		return;
	}
	zWidget->setEnabled(true);
	zWidget->setReadOnly(false);
	int layer = items.first()->data(Qt::UserRole).toInt();
	int zValue = z[layer-2];
	if (zValue == -1) {
		switch (layer) {
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
	if (items.isEmpty()) {
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
	quint8 parameter = quint8(parametersWidget->itemData(parametersWidget->currentIndex()).toInt()),
	        state = quint8(item->data(Qt::UserRole).toInt());

	if (enabled) {
		params.insert(parameter, params.value(parameter) | state);
	} else {
		params.insert(parameter, params.value(parameter) & ~state);
	}

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
	quint8 ID = quint8(item->data(Qt::UserRole).toInt());

	if (enabled) {
		sections.insert(ID);
	} else {
		sections.remove(ID);
	}

	updateBG();
}

void BGDialog::changeZ(int value)
{
	if (_field == nullptr || tabBar->currentIndex() != 0) {
		return;
	}
	QList<QListWidgetItem *> items = layersWidget->selectedItems();
	if (items.isEmpty()) {
		return;
	}
	int layer = items.first()->data(Qt::UserRole).toInt();
	z[layer - 2] = qint16(value);

	updateBG();
}

void BGDialog::saveImage()
{
	if (_field == nullptr) {
		return;
	}

	QString path = Config::value("saveBGPath").toString();
	if (!path.isEmpty()) {
		path.append("/");
	}
	path = QFileDialog::getSaveFileName(this, tr("Save Background"),
	                                    path + _field->name() + ".png",
	                                    tr("PNG image (*.png);;JPG image (*.jpg);;BMP image (*.bmp);;Portable Pixmap (*.ppm)"));
	if (path.isNull()) {
		return;
	}
	
	QDialog dialog(this);
	dialog.setWindowTitle(tr("Export options"));
	QCheckBox checkBox(tr("Transparent background"), &dialog);
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
	connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	QVBoxLayout layout(&dialog);
	layout.addWidget(&checkBox, 1);
	layout.addWidget(&buttonBox);

	if (dialog.exec() != QDialog::Accepted) {
		return;
	}

	background(nullptr, checkBox.isChecked()).save(path);

	qsizetype index = path.lastIndexOf('/');
	Config::setValue("saveBGPath", index == -1 ? path : path.left(index));
}

void BGDialog::tryToRepairBG()
{
	if (_field == nullptr) {
		return;
	}

	if (_field->background()->repair()) {
		QMessageBox::information(this, tr("Background Repaired"), tr("Errors were found and repaired, save to apply the changes."));
		emit modified();
		updateBG();
	} else {
		QMessageBox::warning(this, tr("Repair Failed"), tr("The errors were not corrected."));
	}
}

QImage BGDialog::background(bool *bgWarning, bool transparent)
{
	if (_field == nullptr) {
		return QImage();
	}

	if (tabBar->currentIndex() == 0) {
		return _field->background()->openBackground(&params, z, layers, nullptr, false, transparent, bgWarning);
	}

	bool layers[4] = { false, true, false, false };
	return _field->background()->openBackground(&params, z, layers, &sections, false, transparent, bgWarning);
}

void BGDialog::updateBG()
{
	if (_field == nullptr) {
		return;
	}

	bool bgWarning;
	QImage img = background(&bgWarning);

	if (tabBar->currentIndex() == 0) {
		img = _field->background()->openBackground(&params, z, layers, nullptr, false, false, &bgWarning);
	} else {
		bool layers[4] = { false, true, false, false };
		img = _field->background()->openBackground(&params, z, layers, &sections, false, false, &bgWarning);
	}

	if (img.isNull()) {
		image->setPixmap(QPixmap::fromImage(img));
		bgWarning = false;
	} else {
		image->setPixmap(QPixmap::fromImage(img)
		    .scaled(int(float(imageBox->contentsRect().width()) * zoomFactor),
		            int(float(imageBox->contentsRect().height()) * zoomFactor),
			Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}

	buttonRepair->setVisible(bgWarning);
}

void BGDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	updateBG();
}

void BGDialog::resizeEvent(QResizeEvent *event)
{
	if (event->type() == QEvent::Resize) {
		updateBG();
	}
}

bool BGDialog::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Wheel && obj == imageBox->viewport()) {
		QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
		if (wheelEvent->modifiers() == Qt::CTRL) {
			const float step = 0.25f;
			if (zoomFactor == 0.0f) {
				zoomFactor = 0.25f;
			} else if (wheelEvent->angleDelta().y() > 0) {
				if (zoomFactor >= 4){
					return false; // cap zoom in at 400%
				}
				zoomFactor += step;
			} else if (wheelEvent->angleDelta().y() < 0) {
				if (zoomFactor <= 0.25f){
					return false; // cap zoom in at 25%
				}
				zoomFactor -= step;
			} else {
				return false; // A delta of 0 should never happen
			}

			updateBG();

			return true;
		}
	}

	return false;
}

