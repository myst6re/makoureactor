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
#include "core/field/BackgroundFilePC.h"
#include "core/field/BackgroundTilesIO.h"
#include "core/field/Field.h"

BackgroundEditor::BackgroundEditor(QWidget *parent)
    : QWidget(parent), _backgroundFile(nullptr)
{
	_layersComboBox = new QComboBox(this);
	_layersComboBox->addItem(tr("Layers 0-1"));
	_layersComboBox->addItem(tr("Layer 2"));
	_layersComboBox->addItem(tr("Layer 3"));
	_layersComboBox->setFixedWidth(fontMetrics().horizontalAdvance("WWWWWWWWWWW"));
	_sectionsList = new QListWidget(this);
	_sectionsList->setUniformItemSizes(true);
	_sectionsList->setFixedWidth(fontMetrics().horizontalAdvance("WWWWWWWWWWW"));
	_paramsList = new QTreeWidget(this);
	_paramsList->setUniformRowHeights(true);
	_paramsList->setItemsExpandable(false);
	_paramsList->setHeaderHidden(true);
	_paramsList->setFixedWidth(fontMetrics().horizontalAdvance("WWWWWWWWWWW"));

	_shiftX = new QSpinBox(this);
	_shiftX->setRange(0, 8);
	_shiftY = new QSpinBox(this);
	_shiftY->setRange(0, 8);
	_shiftX->setEnabled(false);
	_shiftY->setEnabled(false);

	_backgroundLayerWidget = new ImageGridWidget(this);
	_backgroundLayerWidget->setSelectionMode(ImageGridWidget::MultiSelection);
	_backgroundLayerScrollArea = new QScrollArea(this);
	_backgroundLayerScrollArea->setWidget(_backgroundLayerWidget);
	_backgroundLayerScrollArea->setWidgetResizable(true);
	_backgroundLayerScrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	_backgroundTileEditor = new BackgroundTileEditor(this);

	QWidget *topRow = new QWidget(this);
	QGridLayout *topRowLayout = new QGridLayout(topRow);
	topRowLayout->addWidget(new QLabel(tr("Shift X:"), this), 0, 0);
	topRowLayout->addWidget(_shiftX, 0, 1);
	topRowLayout->addWidget(new QLabel(tr("Shift Y:"), this), 0, 2);
	topRowLayout->addWidget(_shiftY, 0, 3);
	topRowLayout->addWidget(_backgroundLayerScrollArea, 1, 0, 1, 6);
	topRowLayout->addWidget(_backgroundTileEditor, 0, 6, 2, 1);
	topRowLayout->setColumnStretch(4, 1);
	topRowLayout->setRowStretch(1, 1);
	topRowLayout->setContentsMargins(QMargins(0, 0, 0, topRowLayout->contentsMargins().bottom()));

	_texturesWidget = new ImageGridWidget(this);
	_texturesWidget->setGroupedCellSize(256);
	QScrollArea *texturesScrollArea = new QScrollArea(this);
	texturesScrollArea->setWidget(_texturesWidget);
	texturesScrollArea->setWidgetResizable(true);
	texturesScrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	_palettesWidget = new ImageGridWidget(this);
	_palettesWidget->setSelectionMode(ImageGridWidget::NoSelection);
	_palettesWidget->hide();

	QWidget *bottomRow = new QWidget(this);
	QHBoxLayout *bottomRowLayout = new QHBoxLayout(bottomRow);
	bottomRowLayout->addWidget(texturesScrollArea, 1);
	bottomRowLayout->addWidget(_palettesWidget);
	bottomRowLayout->setContentsMargins(QMargins(0, bottomRowLayout->contentsMargins().top(), 0, 0));

	QSplitter *topBottomSplitter = new QSplitter(Qt::Vertical, this);
	topBottomSplitter->addWidget(topRow);
	topBottomSplitter->addWidget(bottomRow);
	topBottomSplitter->setStretchFactor(0, 10);
	topBottomSplitter->setStretchFactor(1, 2);
	topBottomSplitter->setCollapsible(0, false);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_layersComboBox, 0, 0);
	layout->addWidget(_sectionsList, 1, 0);
	layout->addWidget(_paramsList, 2, 0);
	layout->addWidget(topBottomSplitter, 0, 1, 3, 1);
	layout->setColumnStretch(1, 2);

	connect(_layersComboBox, &QComboBox::currentIndexChanged, this, &BackgroundEditor::updateCurrentLayer);
	connect(_sectionsList, &QListWidget::currentItemChanged, this, &BackgroundEditor::updateCurrentSection);
	connect(_paramsList, &QTreeWidget::currentItemChanged, this, &BackgroundEditor::updateCurrentParam);
	connect(_backgroundLayerWidget, &ImageGridWidget::currentSelectionChanged, this, &BackgroundEditor::updateSelectedTiles);
	connect(_texturesWidget, &ImageGridWidget::currentSelectionChanged, this, &BackgroundEditor::updateSelectedTilesTexture);
	connect(_backgroundTileEditor, &BackgroundTileEditor::changed, this, &BackgroundEditor::updateTiles);
	connect(_backgroundTileEditor, &BackgroundTileEditor::changed, this, &BackgroundEditor::modified);
}

void BackgroundEditor::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);

	_backgroundLayerScrollArea->horizontalScrollBar()->setValue(_backgroundLayerScrollArea->horizontalScrollBar()->maximum() / 2);
	_backgroundLayerScrollArea->verticalScrollBar()->setValue(_backgroundLayerScrollArea->verticalScrollBar()->maximum() / 2);
}

void BackgroundEditor::setSections(const QList<quint16> &sections)
{
	_sections = sections;
}

void BackgroundEditor::fillSectionList()
{
	_sectionsList->blockSignals(true);
	_sectionsList->clear();

	int layer = currentLayer();

	if (layer <= 1) {
		QListWidgetItem *item = new QListWidgetItem(tr("Base Section"));
		item->setData(Qt::UserRole, 4097);
		_sectionsList->addItem(item);

		for (quint16 id: _sections) {
			QListWidgetItem *item = new QListWidgetItem(tr("Section %1").arg(id));
			item->setData(Qt::UserRole, id);
			_sectionsList->addItem(item);
		}
	} else if (layer > 1) {
		_sectionsList->addItem(tr("Single Section"));
	}
	_sectionsList->blockSignals(false);
}

void BackgroundEditor::setParams(const QMap<LayerParam, quint8> &params)
{
	_paramsList->blockSignals(true);
	_paramsList->clear();

	int layer = currentLayer();

	QMapIterator<LayerParam, quint8> it(params);

	while (it.hasNext()) {
		it.next();
		LayerParam layerAndParam = it.key();
		quint8 states = it.value();
		QTreeWidgetItem *parent = new QTreeWidgetItem(_paramsList, QStringList(tr("Param %1").arg(layerAndParam.param)));
		parent->setData(0, Qt::UserRole, layerAndParam.param);
		parent->setData(0, Qt::UserRole + 1, layerAndParam.layer);
		parent->setExpanded(true);
		parent->setFlags(Qt::ItemIsEnabled);
		parent->setHidden(layer != layerAndParam.layer);

		int state = 0;
		while (states) {
			if (states & 1) {
				QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList(tr("State %1").arg(state)));
				item->setData(0, Qt::UserRole, 1 << state);
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

	_backgroundTileEditor->setBackgroundFile(backgroundFile);

	if (backgroundFile != nullptr) {
		_palettesWidget->setPixmap(QPixmap::fromImage(backgroundFile->palettes().toImage()));
		_palettesWidget->setCellSize(1);
		_palettesWidget->setMinimumSize(_palettesWidget->pixmap().size());
	}
}

void BackgroundEditor::refreshTexture()
{
	QPixmap pix;
	int layer = currentLayer();
	
	qDebug() << "BackgroundEditor::refreshTexture" << layer;

	if (_backgroundFile != nullptr) {
		BackgroundTiles tiles;
		for (const Tile &tile : _backgroundFile->tiles()) {
			if ((layer > 1 && tile.layerID == layer) || (layer <= 1 && tile.layerID <= 1)) {
				tiles.insert(tile);
			}
		}
		if (_backgroundFile->field()->isPC()) {
			BackgroundFilePC *backgroundFilePC = static_cast<BackgroundFilePC *>(_backgroundFile);
			pix = QPixmap::fromImage(backgroundFilePC->textures()->toImage(tiles, _backgroundFile->palettes(), _texIdKeys));
		} else {
			pix = QPixmap::fromImage(_backgroundFile->textures()->toImage(tiles, _backgroundFile->palettes()));
		}
	}

	qDebug() << "/BackgroundEditor::refreshTexture" << layer;

	int cellSize = layer > 1 ? 32 : 16;

	_texturesWidget->setCellSize(cellSize);
	_texturesWidget->setPixmap(pix);
	_texturesWidget->setMinimumSize(_texturesWidget->gridSize() * cellSize);
}

void BackgroundEditor::clear()
{
	_backgroundFile = nullptr;
	_backgroundTileEditor->clear();
	_sectionsList->blockSignals(true);
	_sectionsList->clear();
	_sectionsList->blockSignals(false);
	_paramsList->blockSignals(true);
	_paramsList->clear();
	_paramsList->blockSignals(false);
	_backgroundLayerWidget->blockSignals(true);
	_backgroundLayerWidget->setPixmap(QPixmap());
	_backgroundLayerWidget->setSelectedCells(QList<Cell>());
	_backgroundLayerWidget->blockSignals(false);
	_texturesWidget->blockSignals(true);
	_texturesWidget->setPixmap(QPixmap());
	_texturesWidget->setSelectedCells(QList<Cell>());
	_texturesWidget->blockSignals(false);
	_shiftX->setEnabled(false);
	_shiftY->setEnabled(false);
}

int BackgroundEditor::currentLayer() const
{
	int index = _layersComboBox->currentIndex();

	return index == 0 && currentSection() == 4097 ? 0 : index + 1;
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
	if (item == nullptr || item->parent() == nullptr) {
		return ParamState();
	}

	int state = item->data(0, Qt::UserRole).toInt();
	int param = item->parent()->data(0, Qt::UserRole).toInt();

	return ParamState(quint8(param), quint8(state));
}

void BackgroundEditor::updateCurrentLayer(int index)
{
	Q_UNUSED(index)

	fillSectionList();

	if (_sectionsList->currentItem() == nullptr) {
		_sectionsList->blockSignals(true);
		_sectionsList->setCurrentRow(0);
		_sectionsList->blockSignals(false);
	}

	int layer = currentLayer();

	qDebug() << "BackgroundEditor::updateCurrentLayer" << layer;
	int layerNotZero = layer == 0 ? 1 : layer;

	for (int i = 0; i < _paramsList->topLevelItemCount(); ++i) {
		QTreeWidgetItem *item = _paramsList->topLevelItem(i);
		if (item != nullptr) {
			item->setHidden(item->data(0, Qt::UserRole + 1).toInt() != layerNotZero);
		}
	}

	updateImageLabel(layer, currentSection(), -1, -1);

	refreshTexture();
}

void BackgroundEditor::updateCurrentSection(QListWidgetItem *current, QListWidgetItem *previous)
{
	Q_UNUSED(previous)
	
	_paramsList->blockSignals(true);
	_paramsList->setCurrentItem(nullptr);
	_paramsList->blockSignals(false);

	updateCurrentSection2(current != nullptr ? current->data(Qt::UserRole).toInt() : -1);
}

void BackgroundEditor::updateCurrentSection2(int section)
{
	updateImageLabel(section < 0 ? -1 : currentLayer(), section, -1, -1);
}

void BackgroundEditor::updateCurrentParam(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	Q_UNUSED(previous)
	
	_sectionsList->blockSignals(true);
	_sectionsList->setCurrentRow(-1);
	_sectionsList->blockSignals(false);

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

QPoint BackgroundEditor::backgroundPositionFromTile(const QPoint &tile, quint8 cellSize)
{
	return QPoint(MAX_TILE_DST, MAX_TILE_DST) - tile - QPoint(tile.x() % cellSize, tile.y() % cellSize);
}

QPoint BackgroundEditor::tilePositionFromCell(const QPoint &cell, quint8 cellSize, const QPoint &shift)
{
	return cell * cellSize - QPoint(MAX_TILE_DST, MAX_TILE_DST) + shift;
}

void BackgroundEditor::refreshImage(int layer, int section, int param, int state)
{
	if (_backgroundFile == nullptr || !_backgroundFile->isOpen() || layer < 0) {
		_backgroundLayerWidget->setPixmap(QPixmap());
		return;
	}

	QHash<quint8, quint8> paramsEnabled;
	if (param > 0 && state >= 0) {
		paramsEnabled.insert(param, state);
		_isParamMode = true;
	}
	qint16 z[] = {-1, -1};
	bool layers[4] = {false, false, false, false};
	if (layer >= 0 && layer < 4) {
		layers[layer] = true;
	}
	QSet<quint16> sections;
	if (section >= 0) {
		if (layer == 1) {
			sections.insert(quint16(section));
		}
		_isParamMode = false;
	}
	qDebug() << "BackgroundEditor::refreshImage" << layer << section << param << state << paramsEnabled << sections;

	quint16 currentOffsetX, currentOffsetY;
	int width, height;
	_backgroundFile->tiles().area(currentOffsetX, currentOffsetY, width, height);

	qDebug() << "BackgroundEditor::refreshImage" << currentOffsetX << currentOffsetY;

	quint8 tileSize = layer > 1 ? 32 : 16;
	int shiftX = currentOffsetX % tileSize, shiftY = currentOffsetY % tileSize;

	_shiftX->setMaximum(tileSize / 2);
	_shiftY->setMaximum(tileSize / 2);
	_shiftX->setValue(shiftX);
	_shiftY->setValue(shiftY);
	_shiftX->setEnabled(true);
	_shiftY->setEnabled(true);

	QImage backgroundBelow;

	if (section >= 0 && layer == 1) {
		QSet<quint16> sectionsBelow;

		for (int row = 0; row < _sectionsList->count(); ++row) {
			int sectionId = _sectionsList->item(row)->data(Qt::UserRole).toInt();
			if (sectionId == section) {
				break;
			}
			sectionsBelow.insert(sectionId == 4097 ? 1 : sectionId);
		}

		qDebug() << "BackgroundEditor::refreshImage sectionsBelow" << sectionsBelow;

		if (!sectionsBelow.isEmpty()) {
			bool layers[4] = {true, true, false, false};
			backgroundBelow = _backgroundFile->openBackground(!paramsEnabled.isEmpty() ? &paramsEnabled : nullptr, z, layers, &sectionsBelow, !paramsEnabled.isEmpty(), true);
		}
	}

	QImage background = _backgroundFile->openBackground(!paramsEnabled.isEmpty() ? &paramsEnabled : nullptr, z, layers, !sections.isEmpty() ? &sections : nullptr, !paramsEnabled.isEmpty(), true);
	QPixmap pix(background.size());

	if (backgroundBelow.isNull()) {
		pix = QPixmap::fromImage(background);
	} else {
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		p.setOpacity(0.2);
		p.drawImage(0, 0, backgroundBelow);
		p.setOpacity(1.0);
		p.drawImage(0, 0, background);
		p.end();
	}

	_backgroundLayerWidget->setPixmap(backgroundPositionFromTile(QPoint(currentOffsetX, currentOffsetY), tileSize), pix);

	_backgroundLayerWidget->setCellSize(tileSize);
	QSize gridSize((MAX_TILE_DST - currentOffsetX % tileSize) * 2, (MAX_TILE_DST - currentOffsetY % tileSize) * 2);
	_backgroundLayerWidget->setGridSize(gridSize / tileSize);
	QList<QLine> axis;
	axis.append(QLine(0, gridSize.height() / 2, gridSize.width(), gridSize.height() / 2)); // x
	axis.append(QLine(gridSize.width() / 2, 0, gridSize.width() / 2, gridSize.height())); // y
	_backgroundLayerWidget->setCustomLines(axis);
	_backgroundLayerWidget->setMinimumSize(_backgroundLayerWidget->gridSize() * tileSize);
}

void BackgroundEditor::updateSelectedTiles(const QList<Cell> &cells)
{
	qDebug() << "updateSelectedTiles" << _isParamMode << cells;
	if (_backgroundFile == nullptr) {
		return;
	}

	QList<Tile> selectedTiles;

	if (!cells.isEmpty()) {
		int cellSize = _backgroundLayerWidget->cellSize();
		quint8 layerID = quint8(currentLayer());
		quint16 ID = quint16(currentSection());
		ParamState paramState = currentParamState();
		QMultiHash<Cell, Tile> matches;
		BackgroundTiles tiles = _isParamMode && layerID >= 1 && paramState.isValid() ? _backgroundFile->tiles().tiles(layerID, paramState) : _backgroundFile->tiles().tiles(layerID, ID);
		
		qDebug() << "updateSelectedTiles" << cellSize << _isParamMode << layerID << ID << paramState.param << paramState.state << tiles.size();
		QPoint shift(_shiftX->value(), _shiftY->value());

		for (const Tile &tile : tiles) {
			for (const Cell &cell: cells) {
				QPoint dst = tilePositionFromCell(cell, cellSize, shift);
				qint16 dstX = qint16(dst.x()),
				        dstY = qint16(dst.y());

				if (tile.dstX == dstX &&
				        tile.dstY == dstY) {
					qDebug() << tile.tileID << tile.dstX << tile.dstY;
					selectedTiles.append(tile);
					if (matches.contains(cell)) {
						qWarning() << "Multi match!" << cell;
					}
					matches.insert(cell, tile);
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
				} else {
					switch (tile.layerID) {
					case 0: tile.ID = 4095; break;
					case 2: tile.ID = 4096; break;
					}
				}
				if (_paramsList->isEnabled() && _paramsList->currentItem() != nullptr) {
					ParamState paramState = currentParamState();
					tile.param = paramState.param;
					tile.state = paramState.state;
				}
				QPoint dst = tilePositionFromCell(cell, cellSize, shift);
				tile.dstX = qint16(dst.x());
				tile.dstY = qint16(dst.y());
				tile.size = tile.layerID > 1 ? 32 : 16;
				tile.calcIDBig();
				selectedTiles.append(tile);
				matches.insert(cell, tile);
			}
		}
	}

	QList<Cell> texturesSelectedCells;

	for (const Tile &tile: selectedTiles) {
		int index = _texIdKeys.indexOf(tile.textureID);
		if (index < 0) {
			index = tile.textureID;
		}
		texturesSelectedCells.append(Cell((index * 256 + tile.srcX) / tile.size, tile.srcY / tile.size));
	}

	_backgroundTileEditor->setTiles(selectedTiles);
	_backgroundTileEditor->setEnabled(!selectedTiles.isEmpty());
	_texturesWidget->blockSignals(true);
	_texturesWidget->setSelectedCells(texturesSelectedCells);
	_texturesWidget->blockSignals(false);
}

void BackgroundEditor::updateSelectedTilesTexture(const QList<Cell> &cells)
{
	qDebug() << "updateSelectedTilesTexture" << cells;
	QList<Tile> selectedTiles;

	if (!cells.isEmpty()) {
		quint8 layerID = quint8(currentLayer());
		BackgroundTiles tiles = _backgroundFile->tiles().tiles(layerID <= 1 ? 0 : layerID);
		if (layerID <= 1) {
			tiles.insert(_backgroundFile->tiles().tiles(1));
		}

		for (const Tile &tile : tiles) {
			for (const Cell &cell: cells) {
				quint8 tileCount = 256 / tile.size;
				qint16 srcX = qint16((cell.x() % tileCount) * tile.size),
				    srcY = qint16(cell.y() * tile.size);
				quint8 textureID = _texIdKeys.value(cell.x() / tileCount, 0);
				
				if (tile.srcX == srcX &&
				    tile.srcY == srcY &&
				    tile.textureID == textureID) {
					qDebug() << tile.tileID << tile.textureID << tile.srcX << tile.srcY;
					selectedTiles.append(tile);
				}
			}
		}
	}

	_backgroundTileEditor->setTiles(selectedTiles);
	_backgroundTileEditor->setEnabled(!selectedTiles.isEmpty());
}

void BackgroundEditor::updateTiles(const QList<Tile> &tiles)
{
	Q_UNUSED(tiles)

	ParamState paramState = currentParamState();

	refreshImage(currentLayer(), currentSection(), paramState.param, paramState.state);
	refreshTexture();
}
