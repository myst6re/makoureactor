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
	_layersComboBox->setFixedWidth(fontMetrics().horizontalAdvance("WWWWWWWWWWW"));
	_sectionsList = new QListWidget(this);
	_sectionsList->setUniformItemSizes(true);
	_sectionsList->setFixedWidth(fontMetrics().horizontalAdvance("WWWWWWWWWWW"));
	_paramsList = new QTreeWidget(this);
	_paramsList->setUniformRowHeights(true);
	_paramsList->setItemsExpandable(false);
	_paramsList->setHeaderHidden(true);
	_paramsList->setFixedWidth(fontMetrics().horizontalAdvance("WWWWWWWWWWW"));
	_backgroundLayerWidget = new ImageGridWidget(this);
	_backgroundLayerWidget->setSelectionMode(ImageGridWidget::MultiSelection);
	_backgroundLayerScrollArea = new QScrollArea(this);
	_backgroundLayerScrollArea->setWidget(_backgroundLayerWidget);
	_backgroundLayerScrollArea->setWidgetResizable(true);
	_backgroundLayerScrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	_backgroundTileEditor = new BackgroundTileEditor(this);

	QWidget *topRow = new QWidget(this);
	QHBoxLayout *topRowLayout = new QHBoxLayout(topRow);
	topRowLayout->addWidget(_backgroundLayerScrollArea, 1);
	topRowLayout->addWidget(_backgroundTileEditor);
	topRowLayout->setContentsMargins(QMargins());

	_texturesWidget = new ImageGridWidget(this);
	_texturesWidget->setCellSize(16);
	_texturesWidget->setSelectionMode(ImageGridWidget::NoSelection);
	QScrollArea *texturesScrollArea = new QScrollArea(this);
	texturesScrollArea->setWidget(_texturesWidget);
	texturesScrollArea->setWidgetResizable(true);
	texturesScrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	_topBottomSplitter = new QSplitter(Qt::Vertical, this);
	_topBottomSplitter->addWidget(topRow);
	_topBottomSplitter->addWidget(texturesScrollArea);
	_topBottomSplitter->setStretchFactor(0, 10);
	_topBottomSplitter->setStretchFactor(1, 2);
	_topBottomSplitter->setCollapsible(0, false);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_layersComboBox, 0, 0);
	layout->addWidget(_sectionsList, 1, 0);
	layout->addWidget(_paramsList, 2, 0);
	layout->addWidget(_topBottomSplitter, 0, 1, 3, 1);
	layout->setColumnStretch(1, 2);

	connect(_layersComboBox, &QComboBox::currentIndexChanged, this, &BackgroundEditor::updateCurrentLayer);
	connect(_sectionsList, &QListWidget::currentItemChanged, this, &BackgroundEditor::updateCurrentSection);
	connect(_paramsList, &QTreeWidget::currentItemChanged, this, &BackgroundEditor::updateCurrentParam);
	connect(_backgroundLayerWidget, &ImageGridWidget::currentSelectionChanged, this, &BackgroundEditor::updateSelectedTiles);
	connect(_backgroundTileEditor, &BackgroundTileEditor::changed, this, &BackgroundEditor::updateTiles);
}

void BackgroundEditor::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);

	_backgroundLayerScrollArea->horizontalScrollBar()->setValue(_backgroundLayerScrollArea->horizontalScrollBar()->maximum() / 2);
	_backgroundLayerScrollArea->verticalScrollBar()->setValue(_backgroundLayerScrollArea->verticalScrollBar()->maximum() / 2);
}

void BackgroundEditor::setSections(const QList<quint16> &sections)
{
	_sectionsList->blockSignals(true);
	_sectionsList->clear();

	for (quint16 id: sections) {
		QListWidgetItem *item = new QListWidgetItem(tr("Section %1").arg(id));
		item->setData(Qt::UserRole, id);
		_sectionsList->addItem(item);
	}
	_sectionsList->blockSignals(false);
}

void BackgroundEditor::setParams(const QMap<quint8, quint8> &params)
{
	_paramsList->blockSignals(true);
	_paramsList->clear();

	QMapIterator<quint8, quint8> it(params);

	while (it.hasNext()) {
		it.next();
		quint8 param = it.key(), states = it.value();
		QTreeWidgetItem *parent = new QTreeWidgetItem(_paramsList, QStringList(tr("Param %1").arg(param)));
		parent->setData(0, Qt::UserRole, param);
		parent->setExpanded(true);
		parent->setFlags(Qt::ItemIsEnabled);

		int state = 0;
		while (states) {
			if (states & 1) {
				QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList(tr("State %1").arg(state)));
				item->setData(0, Qt::UserRole, state);
				parent->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			}
			state++;
			states >>= 1;
		}
	}

	_paramsList->blockSignals(false);
}

void BackgroundEditor::setBackgroundFile(BackgroundFile *backgroundFile)
{
	_backgroundFile = backgroundFile;

	_layersComboBox->blockSignals(true);
	_layersComboBox->setCurrentIndex(0);
	_layersComboBox->blockSignals(false);
	// setCurrentIndex does not always trigger currentIndexChanged slot
	updateCurrentLayer(0);

	refreshTexture();

	_backgroundTileEditor->setBackgroundFile(backgroundFile);
}

void BackgroundEditor::refreshTexture()
{
	QPixmap pix;

	if (_backgroundFile != nullptr) {
		pix = QPixmap::fromImage(_backgroundFile->textures()->toImage(_backgroundFile->tiles(), _backgroundFile->palettes()));
	}

	_texturesWidget->setPixmap(pix);
	_texturesWidget->setMinimumSize(_texturesWidget->gridSize() * 16);
}

void BackgroundEditor::clear()
{
	_backgroundFile = nullptr;
	_backgroundTileEditor->clear();
	_sectionsList->blockSignals(true);
	_sectionsList->clear();
	_sectionsList->blockSignals(false);
	_backgroundLayerWidget->blockSignals(true);
	_backgroundLayerWidget->setPixmap(QPixmap());
	_backgroundLayerWidget->setSelectedCells(QList<Cell>());
	_backgroundLayerWidget->blockSignals(false);
}

int BackgroundEditor::currentLayer() const
{
	return _layersComboBox->currentIndex();
}

int BackgroundEditor::currentSection() const
{
	QListWidgetItem *item = _sectionsList->currentItem();
	if (item == nullptr) {
		return -1;
	}

	return item->data(Qt::UserRole).toInt();
}

ParamState BackgroundEditor::currentParamState() const
{
	QTreeWidgetItem *item = _paramsList->currentItem();
	if (item == nullptr) {
		return ParamState();
	}

	int state = item->data(0, Qt::UserRole).toInt();
	int param = item->parent()->data(0, Qt::UserRole).toInt();

	return ParamState(quint8(param), quint8(state));
}

void BackgroundEditor::updateCurrentLayer(int layer)
{
	qDebug() << "BackgroundEditor::updateCurrentLayer" << layer;
	bool hasSections = layer == 1,
	        hasParams = layer > 0;

	_sectionsList->setVisible(hasSections);

	if (hasSections && _sectionsList->currentItem() == nullptr) {
		_sectionsList->blockSignals(true);
		_sectionsList->setCurrentRow(0);
		_sectionsList->blockSignals(false);
	}

	_paramsList->setVisible(hasParams);

	updateImageLabel(layer, hasSections ? currentSection() : -1, -1, -1);
}

void BackgroundEditor::updateCurrentSection(QListWidgetItem *current, QListWidgetItem *previous)
{
	Q_UNUSED(previous)

	updateCurrentSection2(current != nullptr ? current->data(Qt::UserRole).toInt() : -1);
}

void BackgroundEditor::updateCurrentSection2(int section)
{
	updateImageLabel(section < 0 ? -1 : currentLayer(), section, -1, -1);
}

void BackgroundEditor::updateCurrentParam(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	Q_UNUSED(previous)

	updateCurrentParam2(current != nullptr && current->parent() != nullptr ? current->parent()->data(0, Qt::UserRole).toInt() : -1,
	                    current != nullptr ? current->data(0, Qt::UserRole).toInt() : -1);
}

void BackgroundEditor::updateCurrentParam2(int param, int state)
{
	updateImageLabel(param <= 0 ? -1 : currentLayer(), -1, param, state);
}

void BackgroundEditor::updateImageLabel(int layer, int section, int param, int state)
{
	refreshImage(layer, section, param, state);

	updateSelectedTiles(_backgroundLayerWidget->selectedCells());
}

void BackgroundEditor::refreshImage(int layer, int section, int param, int state)
{
	qDebug() << "BackgroundEditor::refreshImag" << layer << section << param << state;
	if (_backgroundFile == nullptr || !_backgroundFile->isOpen() || layer < 0) {
		_backgroundLayerWidget->setPixmap(QPixmap());
		return;
	}

	QHash<quint8, quint8> paramsEnabled;
	if (param > 0 && state >= 0) {
		paramsEnabled.insert(param, state);
	}
	qint16 z[] = {-1, -1};
	bool layers[4] = {false, false, false, false};
	if (layer >= 0 && layer < 4) {
		layers[layer] = true;
	}
	QSet<quint16> sections;
	if (section >= 0) {
		sections.insert(quint16(section));
	}

	quint16 currentOffsetX, currentOffsetY;
	int width, height;
	_backgroundFile->tiles().area(currentOffsetX, currentOffsetY, width, height);

	QImage background = _backgroundFile->openBackground(param > 0 ? &paramsEnabled : nullptr, z, layers, section >= 0 ? &sections : nullptr, true);
	_backgroundLayerWidget->setPixmap(QPoint(MAX_TILE_DST - currentOffsetX, MAX_TILE_DST - currentOffsetY), QPixmap::fromImage(background));

	quint8 tileSize = layer > 1 ? 32 : 16;
	_backgroundLayerWidget->setCellSize(tileSize);
	_backgroundLayerWidget->setGridSize(QSize(MAX_TILE_DST * 2 / tileSize, MAX_TILE_DST * 2 / tileSize));
	_backgroundLayerWidget->setMinimumSize(_backgroundLayerWidget->gridSize() * tileSize);
}

void BackgroundEditor::updateSelectedTiles(const QList<Cell> &cells)
{
	if (_backgroundFile == nullptr) {
		return;
	}

	QList<Tile> selectedTiles;

	if (!cells.isEmpty()) {
		int cellSize = _backgroundLayerWidget->cellSize();
		quint8 layerID = quint8(currentLayer());
		quint16 ID = quint16(currentSection());
		QMultiHash<Cell, Tile> matches;
		BackgroundTiles tiles = _backgroundFile->tiles().tiles(layerID, ID);
		
		for (const Tile &tile : tiles) {
			for (const Cell &cell: cells) {
				qint16 dstX = qint16(cell.x() * cellSize - MAX_TILE_DST),
				        dstY = qint16(cell.y() * cellSize - MAX_TILE_DST);

				if (tile.dstX == dstX &&
				        tile.dstY == dstY) {
					selectedTiles.append(tile);
					if (matches.contains(cell)) {
						qWarning() << "Multi match!" << cell;
					}
					matches.insert(cell, tile);

					break;
				}
			}
		}
		
		for (const Cell &cell: cells) {
			if (!matches.contains(cell)) {
				Tile tile = Tile();
				tile.tileID = quint16(-1);
				tile.layerID = quint8(currentLayer());
				if (_sectionsList->isEnabled() && _sectionsList->currentItem() != nullptr) {
					tile.ID = currentSection();
				}
				if (_paramsList->isEnabled() && _paramsList->currentItem() != nullptr) {
					ParamState paramState = currentParamState();
					tile.param = paramState.param;
					tile.state = paramState.state;
				}
				tile.dstX = qint16(cell.x() * cellSize - MAX_TILE_DST);
				tile.dstY = qint16(cell.y() * cellSize - MAX_TILE_DST);
				tile.depth = 1;
				tile.size = tile.layerID > 1 ? 32 : 16;
				selectedTiles.append(tile);
				matches.insert(cell, tile);
			}
		}
	}

	QList<Cell> texturesSelectedCells;

	for (const Tile &tile: selectedTiles) {
		texturesSelectedCells.append(Cell(((tile.textureID % 15) * 256 + tile.srcX) / 16, ((tile.textureID / 15) * 256 + tile.srcY) / 16));
	}

	_backgroundTileEditor->setTiles(selectedTiles);
	_backgroundTileEditor->setEnabled(!selectedTiles.isEmpty());
	_texturesWidget->setSelectedCells(texturesSelectedCells);
}

void BackgroundEditor::updateTiles(const QList<Tile> &tiles)
{
	Q_UNUSED(tiles)

	ParamState paramState = currentParamState();

	refreshImage(currentLayer(), currentSection(), paramState.param, paramState.state);
	refreshTexture();
}
