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
#include "BackgroundEditor.h"
#include "core/field/BackgroundFile.h"
#include "core/field/BackgroundTilesIO.h"

BackgroundEditor::BackgroundEditor(QWidget *parent)
    : QWidget(parent), _backgroundFile(nullptr)
{
	_layersComboBox = new QComboBox(this);
	for (quint8 i = 0; i < 4; ++i) {
		_layersComboBox->addItem(tr("Layer %1").arg(i + 1));
	}
	_sectionsList = new QListWidget(this);
	_sectionsList->setUniformItemSizes(true);
	_sectionsList->setFixedWidth(fontMetrics().horizontalAdvance("WWWWWWWWWWW"));
	_tileCountWidthSpinBox = new QSpinBox(this);
	_tileCountWidthSpinBox->setRange(-MAX_TILE_DST, MAX_TILE_DST);
	_tileCountHeightSpinBox = new QSpinBox(this);
	_tileCountHeightSpinBox->setRange(-MAX_TILE_DST, MAX_TILE_DST);
	_zSpinBox = new QSpinBox(this);
	_zSpinBox->setRange(0, 4096);
	_backgroundLayerWidget = new ImageGridWidget(this);
	_backgroundLayerWidget->setSelectionMode(ImageGridWidget::MultiSelection);
	QScrollArea *backgroundLayerScrollArea = new QScrollArea(this);
	backgroundLayerScrollArea->setWidget(_backgroundLayerWidget);
	backgroundLayerScrollArea->setWidgetResizable(true);
	backgroundLayerScrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	_tileWidget = new ImageGridWidget(this);
	_tileWidget->setCellSize(1);
	_tileWidget->setFixedHeight(32 * 4);

	_texturesWidget = new ImageGridWidget(this);
	_texturesWidget->setCellSize(16);
	_texturesWidget->setSelectionMode(ImageGridWidget::NoSelection);
	QScrollArea *texturesScrollArea = new QScrollArea(this);
	texturesScrollArea->setWidget(_texturesWidget);
	texturesScrollArea->setWidgetResizable(true);
	texturesScrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	_currentTileComboBox = new QComboBox(this);

	_bgParamGroup = new QGroupBox(tr("Parameter"), this);
	_bgParamGroup->setCheckable(true);
	
	_bgParamInput = new QSpinBox(_bgParamGroup);
	_bgParamInput->setRange(1, 255);
	_bgParamStateInput = new QSpinBox(_bgParamGroup);
	_bgParamStateInput->setRange(0, 255);

	QFormLayout *tileParameterEditorLayout = new QFormLayout(_bgParamGroup);
	tileParameterEditorLayout->addRow(tr("Param ID"), _bgParamInput);
	tileParameterEditorLayout->addRow(tr("State ID"), _bgParamStateInput);

	_blendTypeInput = new QComboBox(this);
	_blendTypeInput->addItem(tr("None"));
	_blendTypeInput->addItem(tr("Average"));
	_blendTypeInput->addItem(tr("Plus"));
	_blendTypeInput->addItem(tr("Minus"));
	_blendTypeInput->addItem(tr("Source +25% destination"));

	_depthInput = new QComboBox(this);
	_depthInput->addItem(tr("Paletted 4-bit"));
	_depthInput->addItem(tr("Paletted 8-bit"));
	_depthInput->addItem(tr("Direct color 16-bit"));

	_paletteIdInput = new QSpinBox(this);
	_paletteIdInput->setRange(0, 255);

	_tileEditorLayout = new QFormLayout();
	_tileEditorLayout->addRow(tr("Blend type"), _blendTypeInput);
	_tileEditorLayout->addRow(tr("Depth"), _depthInput);
	_tileEditorLayout->addRow(tr("Palette ID"), _paletteIdInput);

	_rightPaneLayout = new QVBoxLayout();
	_rightPaneLayout->addWidget(_currentTileComboBox);
	_rightPaneLayout->addWidget(_tileWidget);
	_rightPaneLayout->addLayout(_tileEditorLayout);
	_rightPaneLayout->addWidget(_bgParamGroup);
	_rightPaneLayout->setContentsMargins(QMargins());

	QHBoxLayout *layerEditorLayout = new QHBoxLayout();
	layerEditorLayout->addWidget(new QLabel(tr("Width (in tile unit):"), this));
	layerEditorLayout->addWidget(_tileCountWidthSpinBox);
	layerEditorLayout->addWidget(new QLabel(tr("Height (in tile unit):"), this));
	layerEditorLayout->addWidget(_tileCountHeightSpinBox);
	layerEditorLayout->addWidget(new QLabel(tr("Z:"), this));
	layerEditorLayout->addWidget(_zSpinBox);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_layersComboBox, 0, 0);
	layout->addWidget(_sectionsList, 1, 0);
	layout->addLayout(layerEditorLayout, 0, 1);
	layout->addWidget(backgroundLayerScrollArea, 1, 1);
	layout->addLayout(_rightPaneLayout, 0, 2, 2, 1);
	layout->addWidget(texturesScrollArea, 2, 0, 1, 3);
	layout->setColumnStretch(0, 1);
	layout->setColumnStretch(1, 2);
	layout->setColumnStretch(2, 1);
	layout->setRowStretch(1, 2);

	connect(_layersComboBox, &QComboBox::currentIndexChanged, this, &BackgroundEditor::updateCurrentLayer);
	connect(_sectionsList, &QListWidget::currentItemChanged, this, &BackgroundEditor::updateCurrentSection);
	connect(_zSpinBox, &QSpinBox::valueChanged, this, &BackgroundEditor::updateZ);
	connect(_backgroundLayerWidget, &ImageGridWidget::currentSelectionChanged, this, &BackgroundEditor::updateSelectedTiles);
	connect(_currentTileComboBox, &QComboBox::currentIndexChanged, this, &BackgroundEditor::updateSelectedTile);
}

void BackgroundEditor::setSections(const QList<quint16> &sections)
{
	_sectionsList->clear();
	int i = 0;

	for (quint16 id: sections) {
		QListWidgetItem *item = new QListWidgetItem(tr("Section %1").arg(i++));
		item->setData(Qt::UserRole, id);
		_sectionsList->addItem(item);
	}
}

void BackgroundEditor::setBackgroundFile(BackgroundFile *backgroundFile)
{
	_backgroundFile = backgroundFile;

	updateCurrentLayer(_layersComboBox->currentIndex());

	QPixmap pix;

	if (_backgroundFile != nullptr) {
		pix = QPixmap::fromImage(_backgroundFile->textures()->toImage(_backgroundFile->tiles(), _backgroundFile->palettes()));
	}

	_texturesWidget->setPixmap(pix);
	_texturesWidget->setMinimumSize(pix.size());
}

void BackgroundEditor::clear()
{
	_backgroundFile = nullptr;
	_sectionsList->blockSignals(true);
	_sectionsList->clear();
	_sectionsList->blockSignals(false);
	_backgroundLayerWidget->setPixmap(QPixmap());
}

int BackgroundEditor::currentSection() const
{
	QListWidgetItem *item = _sectionsList->currentItem();
	if (item == nullptr) {
		return -1;
	}

	return item->data(Qt::UserRole).toInt();
}

void BackgroundEditor::updateCurrentLayer(int layer)
{
	bool hasSections = layer == 1;

	_sectionsList->setEnabled(hasSections);

	if (hasSections && _sectionsList->currentItem() == nullptr) {
		_sectionsList->blockSignals(true);
		_sectionsList->setCurrentRow(0);
		_sectionsList->blockSignals(false);
	}

	updateImageLabel(layer, hasSections ? currentSection() : -1);
}

void BackgroundEditor::updateCurrentSection(QListWidgetItem *current, QListWidgetItem *previous)
{
	Q_UNUSED(previous)

	updateCurrentSection2(current != nullptr ? current->data(Qt::UserRole).toInt() : -1);
}

void BackgroundEditor::updateCurrentSection2(int section)
{
	updateImageLabel(_layersComboBox->currentIndex(), section);
}

void BackgroundEditor::updateImageLabel(int layer, int section)
{
	if (_backgroundFile == nullptr || !_backgroundFile->isOpen()) {
		return;
	}

	qint16 z[] = {-1, -1};
	bool layers[4] = {false, false, false, false};
	if (layer >= 0 && layer < 4) {
		layers[layer] = true;
	}
	QSet<quint16> sections;
	_zSpinBox->blockSignals(true);
	if (section >= 0) {
		sections.insert(quint16(section));
		if (_zSpinBox->value() != section) {
			_zSpinBox->setValue(section);
			_zSpinBox->setEnabled(true);
		}
	} else {
		if (_zSpinBox->value() != 0) {
			_zSpinBox->setValue(0);
			_zSpinBox->setEnabled(false);
		}
	}
	_zSpinBox->blockSignals(false);

	QImage background = _backgroundFile->openBackground(nullptr, z, layers, sections.isEmpty() ? nullptr : &sections);
	_backgroundLayerWidget->setPixmap(QPixmap::fromImage(background));
	_backgroundLayerWidget->setMinimumSize(background.size());

	QSize size = _backgroundFile->tiles().filter(nullptr, z, layers, sections.isEmpty() ? nullptr : &sections).area();
	quint8 tileSize = layer > 1 ? 32 : 16;
	_backgroundLayerWidget->setCellSize(tileSize);
	_tileCountWidthSpinBox->setValue(size.width() / tileSize);
	_tileCountHeightSpinBox->setValue(size.height() / tileSize);

	int width, height;
	_backgroundFile->tiles().area(_currentOffsetX, _currentOffsetY, width, height);

	updateSelectedTiles(_backgroundLayerWidget->selectedCells());
}

void BackgroundEditor::updateZ(int z)
{
	if (_backgroundFile == nullptr) {
		return;
	}

	QListWidgetItem *item = _sectionsList->currentItem();
	int section = item->data(Qt::UserRole).toInt();

	if (section >= 0) {
		_backgroundFile->setZLayer1(quint16(section), quint16(z));
		item->setData(Qt::UserRole, z);
		updateImageLabel(1, z);
		emit modified();
	}
}

void BackgroundEditor::updateSelectedTiles(const QList<Cell> &cells)
{
	if (_backgroundFile == nullptr) {
		return;
	}

	_currentTileComboBox->blockSignals(true);
	_currentTileComboBox->clear();
	_selectedTiles.clear();

	if (!cells.isEmpty()) {
		int cellSize = _backgroundLayerWidget->cellSize();
		quint8 layerID = quint8(_layersComboBox->currentIndex());
		quint16 ID = quint16(_zSpinBox->value());

		for (const Tile &tile : _backgroundFile->tiles().tiles(layerID, ID)) {
			for (const Cell &cell: cells) {
				qint16 dstX = qint16(cell.x() * cellSize - _currentOffsetX),
				        dstY = qint16(cell.y() * cellSize - _currentOffsetY);

				if (tile.dstX == dstX &&
				        tile.dstY == dstY) {
					_currentTileComboBox->addItem(tr("Tile %1 (%2, %3) %4").arg(_selectedTiles.size()).arg(tile.srcX).arg(tile.srcY).arg(tile.param ? tr("Params %1 State %2").arg(tile.param).arg(tile.state) : "").trimmed());
					_selectedTiles.append(tile);
				}
			}
		}
	}

	_currentTileComboBox->blockSignals(false);
	_currentTileComboBox->setEnabled(_currentTileComboBox->count() > 1);
	updateSelectedTile(_currentTileComboBox->count() >= 0 ? 0 : -1);
}

void BackgroundEditor::updateSelectedTile(int index)
{
	bool enableRightPane = index >= 0 && index < _selectedTiles.size();

	_tileWidget->setEnabled(enableRightPane);
	_bgParamGroup->setEnabled(enableRightPane);
	_blendTypeInput->setEnabled(enableRightPane);
	_tileEditorLayout->labelForField(_blendTypeInput)->setEnabled(enableRightPane);
	_depthInput->setEnabled(enableRightPane);
	_tileEditorLayout->labelForField(_depthInput)->setEnabled(enableRightPane);
	_paletteIdInput->setEnabled(enableRightPane);
	_tileEditorLayout->labelForField(_paletteIdInput)->setEnabled(enableRightPane);

	if (!enableRightPane) {
		return;
	}

	const Tile &tile = _selectedTiles.at(index);
	QPixmap pix = QPixmap::fromImage(_backgroundFile->textures()->toImage(tile, _backgroundFile->palettes()));
	_tileWidget->setPixmap(pix);

	_bgParamGroup->setChecked(tile.param > 0);
	_bgParamInput->setValue(tile.param);
	_bgParamStateInput->setValue(tile.state);
	_depthInput->setCurrentIndex(tile.depth);
	_paletteIdInput->setValue(tile.paletteID);

	_blendTypeInput->setCurrentIndex(tile.blending ? tile.typeTrans + 1 : 0);

	_texturesWidget->setSelectedCell(Cell(((tile.textureID % 15) * 256 + tile.srcX) / 16, ((tile.textureID / 15) * 256 + tile.srcY) / 16));
}
