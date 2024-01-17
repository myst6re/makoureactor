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
#include "core/Config.h"

BackgroundEditor::BackgroundEditor(QWidget *parent)
    : QWidget(parent), _backgroundFile(nullptr)
{

	_layersComboBox = new QComboBox(this);
	_layersComboBox->addItem(tr("Layers 0-1"));
	_layersComboBox->addItem(tr("Layer 2"));
	_layersComboBox->addItem(tr("Layer 3"));
	_sectionsList = new QTreeWidget(this);
	_sectionsList->setUniformRowHeights(true);
	_sectionsList->setItemsExpandable(true);
	_sectionsList->setHeaderHidden(true);
	
	int firstColumnWidth = fontMetrics().horizontalAdvance("WWWWWWWWWWW") + _sectionsList->indentation() * 2;
	_layersComboBox->setFixedWidth(firstColumnWidth);
	_sectionsList->setFixedWidth(firstColumnWidth);

	addTopLevelItem(tr("By Depth"), SubLayer);
	addTopLevelItem(tr("By Param"), BackgroundParameter);
	addTopLevelItem(tr("Conflicts"), Effect);

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
	
	_compileButton = new QPushButton(tr("Check errors"), this);

	QWidget *topRow = new QWidget(this);
	QGridLayout *topRowLayout = new QGridLayout(topRow);
	topRowLayout->addWidget(new QLabel(tr("Shift X:"), this), 0, 0);
	topRowLayout->addWidget(_shiftX, 0, 1);
	topRowLayout->addWidget(new QLabel(tr("Shift Y:"), this), 0, 2);
	topRowLayout->addWidget(_shiftY, 0, 3);
	topRowLayout->addWidget(_compileButton, 0, 6);
	topRowLayout->addWidget(_backgroundLayerScrollArea, 1, 0, 1, 7);
	topRowLayout->addWidget(_backgroundTileEditor, 0, 7, 2, 1);
	topRowLayout->setColumnStretch(5, 1);
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

	_topBottomSplitter = new QSplitter(Qt::Vertical, this);
	_topBottomSplitter->addWidget(topRow);
	_topBottomSplitter->addWidget(bottomRow);
	_topBottomSplitter->setStretchFactor(0, 10);
	_topBottomSplitter->setStretchFactor(1, 2);
	_topBottomSplitter->setCollapsible(0, false);
	_topBottomSplitter->restoreState(Config::value("backgroundEditorHorizontalSplitterState").toByteArray());

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_layersComboBox, 0, 0);
	layout->addWidget(_sectionsList, 1, 0);
	layout->addWidget(_topBottomSplitter, 0, 1, 2, 1);
	layout->setColumnStretch(1, 2);

	connect(_layersComboBox, &QComboBox::currentIndexChanged, this, &BackgroundEditor::updateCurrentLayer);
	connect(_sectionsList, &QTreeWidget::currentItemChanged, this, &BackgroundEditor::updateCurrentSection);
	connect(_backgroundLayerWidget, &ImageGridWidget::currentSelectionChanged, this, &BackgroundEditor::updateSelectedTiles);
	connect(_texturesWidget, &ImageGridWidget::clicked, this, &BackgroundEditor::updateSelectedTileTexture);
	connect(_backgroundTileEditor, &BackgroundTileEditor::changed, this, &BackgroundEditor::updateTiles);
	connect(_backgroundTileEditor, &BackgroundTileEditor::changed, this, &BackgroundEditor::modified);
	connect(_compileButton, &QPushButton::clicked, this, &BackgroundEditor::compile);
}

void BackgroundEditor::saveConfig()
{
	Config::setValue("backgroundEditorHorizontalSplitterState", _topBottomSplitter->saveState());
}

void BackgroundEditor::addTopLevelItem(const QString &name, LayerSubType subType)
{
	QTreeWidgetItem *item = new QTreeWidgetItem(_sectionsList, QStringList(name), int(subType));
	item->setExpanded(true);
	item->setFlags(Qt::ItemIsEnabled);
}

void BackgroundEditor::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);

	_backgroundLayerScrollArea->horizontalScrollBar()->setValue(_backgroundLayerScrollArea->horizontalScrollBar()->maximum() / 2);
	_backgroundLayerScrollArea->verticalScrollBar()->setValue(_backgroundLayerScrollArea->verticalScrollBar()->maximum() / 2);
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

	_backgroundLayerScrollArea->horizontalScrollBar()->setValue(_backgroundLayerScrollArea->horizontalScrollBar()->maximum() / 2);
	_backgroundLayerScrollArea->verticalScrollBar()->setValue(_backgroundLayerScrollArea->verticalScrollBar()->maximum() / 2);
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
	refreshList(0);
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
	QTreeWidgetItem *item = _sectionsList->currentItem();
	if (item == nullptr || item->parent() == nullptr || LayerSubType(item->type()) != SubLayer) {
		return -1;
	}

	return item->data(0, Qt::UserRole).toInt();
}

ParamState BackgroundEditor::currentParamState() const
{
	QTreeWidgetItem *item = _sectionsList->currentItem();
	if (item == nullptr || item->parent() == nullptr || item->parent()->parent() == nullptr || LayerSubType(item->type()) != BackgroundParameter) {
		return ParamState();
	}

	int state = item->data(0, Qt::UserRole).toInt();
	int param = item->parent()->data(0, Qt::UserRole).toInt();

	return ParamState(quint8(param), quint8(state));
}

QList<quint16> BackgroundEditor::currentEffect() const
{
	QList<quint16> ret;
	QTreeWidgetItem *item = _sectionsList->currentItem();
	if (item == nullptr || item->parent() == nullptr || LayerSubType(item->type()) != Effect) {
		return ret;
	}

	QList<QVariant> tileIds = item->data(0, Qt::UserRole).toList();
	for (const QVariant &tileId: tileIds) {
		ret.append(quint16(tileId.toInt()));
	}

	return ret;
}

void BackgroundEditor::updateCurrentLayer(int index)
{
	Q_UNUSED(index)

	int layer = currentLayer();

	refreshList(layer);

	if (_sectionsList->currentItem() == nullptr || _sectionsList->currentItem()->parent() == nullptr) {
		_sectionsList->blockSignals(true);
		_sectionsList->setCurrentItem(_sectionsList->topLevelItem(0)->child(0));
		_sectionsList->blockSignals(false);
	}

	qDebug() << "BackgroundEditor::updateCurrentLayer" << layer;

	updateCurrentSection(_sectionsList->currentItem(), nullptr);

	refreshTexture();
}

void BackgroundEditor::refreshList(int layer)
{
	_sectionsList->blockSignals(true);
	for (int row = 0; row < _sectionsList->topLevelItemCount(); ++row) {
		qDeleteAll(_sectionsList->topLevelItem(row)->takeChildren());
	}
	_sectionsList->blockSignals(false);

	if (_backgroundFile == nullptr) {
		return;
	}

	QMap<LayerParam, quint8> usedParams;
	bool layerExists[] = {false, false, false};
	QSet<quint16> usedIDs;
	QList<QList<quint16> > effectLayers[] = {QList<QList<quint16> >(), QList<QList<quint16> >(), QList<QList<quint16> >()};

	_backgroundFile->usedParams(usedParams, layerExists, &usedIDs, effectLayers);

	QList<quint16> usedIDsList = usedIDs.values();
	std::sort(usedIDsList.begin(), usedIDsList.end(), std::greater<>());

	QMapIterator<LayerParam, quint8> itParams(usedParams);

	for (int row = 0; row < _sectionsList->topLevelItemCount(); ++row) {
		QTreeWidgetItem *topLevelItem = _sectionsList->topLevelItem(row);
		QTreeWidgetItem *item = nullptr;
		QList<QTreeWidgetItem *> items;
		
		switch (LayerSubType(topLevelItem->type())) {
		case SubLayer:
			item = new QTreeWidgetItem(QStringList(layer <= 1 ? tr("Base Section") : tr("Single Section")), SubLayer);
			item->setExpanded(false);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			items.append(item);

			if (layer <= 1) {
				item->setData(0, Qt::UserRole, 4097);

				for (quint16 id: std::as_const(usedIDsList)) {
					item = new QTreeWidgetItem(QStringList(tr("Section %1").arg(id)), SubLayer);
					item->setExpanded(false);
					item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
					item->setData(0, Qt::UserRole, id);
					items.append(item);
				}
			}
			break;
		case BackgroundParameter:
			while (layer > 0 && itParams.hasNext()) {
				itParams.next();
				LayerParam layerAndParam = itParams.key();
				quint8 states = itParams.value();

				if (layer == layerAndParam.layer) {
					QTreeWidgetItem *parent = new QTreeWidgetItem(topLevelItem, QStringList(tr("Param %1").arg(layerAndParam.param)), BackgroundParameter);
					parent->setData(0, Qt::UserRole, layerAndParam.param);
					parent->setData(0, Qt::UserRole + 1, layerAndParam.layer);
					parent->setExpanded(true);
					parent->setFlags(Qt::ItemIsEnabled);

					int state = 0;
					while (states) {
						if (states & 1) {
							item = new QTreeWidgetItem(parent, QStringList(tr("State %1").arg(state)), BackgroundParameter);
							item->setData(0, Qt::UserRole, 1 << state);
							item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
						}
						state++;
						states >>= 1;
					}
				}
			}
			break;
		case Effect:
			if (layer == 0) {
				break;
			}

			int i = 0;
			for (const QList<quint16> &tileIds: effectLayers[layer - 1]) {
				QList<QVariant> tilesIdsVariant;
				for (quint16 tileId: tileIds) {
					tilesIdsVariant.append(tileId);
				}
				item = new QTreeWidgetItem(QStringList(tr("Conflict %1").arg(i)), Effect);
				item->setData(0, Qt::UserRole, tilesIdsVariant);
				item->setExpanded(false);
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				items.append(item);

				++i;
			}
			break;
		}
		topLevelItem->addChildren(items);
	}
}

void BackgroundEditor::updateCurrentSection(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	Q_UNUSED(current)
	Q_UNUSED(previous)

	int section = currentSection();
	ParamState paramState = currentParamState();
	QList<quint16> effectTileIds = currentEffect();
	int layerId = section >= 0 || paramState.isValid() || !effectTileIds.isEmpty() ? currentLayer() : -1;

	qDebug() << "BackgroundEditor::updateCurrentSection" << "layerId" << layerId << "section" << section << "param" << paramState.param << "effectTileIds" << effectTileIds.size();

	updateImageLabel(layerId, section, paramState, effectTileIds);
}

void BackgroundEditor::updateImageLabel(int layer, int section, ParamState paramState, const QList<quint16> &effectTileIds)
{
	refreshImage(layer, section, paramState, effectTileIds);
	qDebug() << "BackgroundEditor::updateImageLabel";

	updateSelectedTiles(_backgroundLayerWidget->selectedCells());
}

QPoint BackgroundEditor::backgroundPositionFromTile(const QPoint &tile, const QPoint &shift)
{
	return tile + QPoint(MAX_TILE_DST, MAX_TILE_DST) - shift;
}

QPoint BackgroundEditor::tilePositionFromCell(const QPoint &cell, quint8 cellSize, const QPoint &shift)
{
	return cell * cellSize - QPoint(MAX_TILE_DST, MAX_TILE_DST) + shift;
}

void BackgroundEditor::refreshImage(int layer, int section, ParamState paramState, const QList<quint16> &effectTileIds)
{
	if (_backgroundFile == nullptr || !_backgroundFile->isOpen() || layer < 0) {
		_backgroundLayerWidget->setPixmap(QPixmap());
		return;
	}

	QHash<quint8, quint8> paramsEnabled;
	if (paramState.isValid()) {
		paramsEnabled.insert(paramState.param, paramState.state);
	}
	bool layers[4] = {false, false, false, false};
	if (layer >= 0 && layer < 4) {
		if (layer < 2) {
			layers[0] = true;
			layers[1] = true;
		} else {
			layers[layer] = true;
		}
	}
	QSet<quint16> sections;
	if (section >= 0) {
		if (layer == 1) {
			sections.insert(quint16(section));
		}
	}

	BackgroundTiles layerTiles = _backgroundFile->tiles().filter(nullptr, nullptr, layers, nullptr, nullptr);
	quint8 tileSize = layer > 1 ? 32 : 16;
	QRect area = layerTiles.rect();
	QPoint shift = layerTiles.dstShift(tileSize);
	
	qDebug() << "BackgroundEditor::refreshImage" << "area" << area << shift;
	
	_shiftX->setMaximum(tileSize - 1);
	_shiftY->setMaximum(tileSize - 1);
	_shiftX->setValue(shift.x());
	_shiftY->setValue(shift.y());
	_shiftX->setEnabled(true);
	_shiftY->setEnabled(true);

	QImage backgroundBelow;

	if (section >= 0 && layer == 1) {
		QSet<quint16> sectionsBelow;
		
		QTreeWidgetItem *rootItem = _sectionsList->topLevelItem(0);
		
		for (int row = 0; row < rootItem->childCount(); ++row) {
			int sectionId = rootItem->child(row)->data(0, Qt::UserRole).toInt();
			if (sectionId == section) {
				break;
			}
			sectionsBelow.insert(sectionId == 4097 ? 4095 : sectionId);
		}
		
		qDebug() << "sectionsBelow" << sectionsBelow;

		if (!sectionsBelow.isEmpty()) {
			backgroundBelow = _backgroundFile->openBackground(layerTiles.filter(nullptr, nullptr, nullptr, &sectionsBelow, nullptr), area, true);
		}
	}

	QImage background = _backgroundFile->openBackground(
	            layerTiles.tiles(layer).filter(!paramsEnabled.isEmpty() ? &paramsEnabled : nullptr, nullptr, nullptr, !sections.isEmpty() ? &sections : nullptr, !effectTileIds.isEmpty() ? &effectTileIds : nullptr, !paramsEnabled.isEmpty()),
	            area, true);
	QPixmap pix;

	if (backgroundBelow.isNull()) {
		pix = QPixmap::fromImage(background);
	} else {
		pix = QPixmap(background.size());
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		p.setOpacity(0.2);
		p.drawImage(0, 0, backgroundBelow);
		p.setOpacity(1.0);
		p.drawImage(0, 0, background);
		p.end();
	}

	_backgroundLayerWidget->setCellSize(tileSize);
	QSize gridSize((MAX_TILE_DST - shift.x()) * 2, (MAX_TILE_DST - shift.y()) * 2);
	_backgroundLayerWidget->setGridSize(gridSize / tileSize);

	_backgroundLayerWidget->setPixmap(backgroundPositionFromTile(layerTiles.minTopLeft(), shift), pix);
	
	qDebug() << "gridSize" << gridSize;

	QList<QLine> axis;
	axis.append(QLine(0, gridSize.height() / 2, gridSize.width(), gridSize.height() / 2)); // x
	axis.append(QLine(gridSize.width() / 2, 0, gridSize.width() / 2, gridSize.height())); // y
	_backgroundLayerWidget->setCustomLines(axis);

	_backgroundLayerWidget->setMinimumSize(_backgroundLayerWidget->gridSize() * tileSize);
}

void BackgroundEditor::updateSelectedTiles(const QList<Cell> &cells)
{
	qDebug() << "updateSelectedTiles" << cells;
	if (_backgroundFile == nullptr) {
		return;
	}

	QList<Tile> selectedTiles;

	if (!cells.isEmpty()) {
		int cellSize = _backgroundLayerWidget->cellSize();
		quint8 layerID = quint8(currentLayer());
		int section = currentSection();
		quint16 ID = quint16(section);
		ParamState paramState = currentParamState();
		QList<quint16> effectTileIds = currentEffect();
		QMultiHash<Cell, Tile> matches;
		BackgroundTiles tiles;

		if (layerID >= 1 && paramState.isValid()) {
			tiles = _backgroundFile->tiles().tiles(layerID, paramState);
		} else if (layerID >= 1 && !effectTileIds.isEmpty()) {
			tiles = _backgroundFile->tiles().tiles(layerID, false);
		} else {
			tiles = _backgroundFile->tiles().tiles(layerID, ID);
		}
		
		QPoint shift(_shiftX->value(), _shiftY->value());
		qDebug() << "updateSelectedTiles" << "cellSize" << cellSize << "layerID" << layerID << "ID" << ID << "param" << paramState.param << "state" << paramState.state << "tilescount" << tiles.size() << "shift" << shift;
		
		for (const Tile &tile : std::as_const(tiles)) {
			if (layerID >= 1 && !effectTileIds.isEmpty() && !effectTileIds.contains(tile.tileID)) {
				continue;
			}

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
				tile.layerID = layerID;
				if (section >= 0) {
					tile.ID = section;
				} else {
					switch (tile.layerID) {
					case 0: tile.ID = 4095; break;
					case 2: tile.ID = 4096; break;
					}
				}
				if (paramState.isValid()) {
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

void BackgroundEditor::updateSelectedTileTexture(const Cell &cell)
{
	qDebug() << "updateSelectedTileTexture" << cell;
	Tile selectedTile = Tile();
	selectedTile.tileID = quint16(-1);

	quint8 layerID = quint8(currentLayer());
	BackgroundTiles tiles = _backgroundFile->tiles().tiles(layerID <= 1 ? 0 : layerID);
	if (layerID <= 1) {
		tiles.insert(_backgroundFile->tiles().tiles(1));
	}

	for (const Tile &tile : tiles) {
		quint8 tileCount = 256 / tile.size;
		qint16 srcX = qint16((cell.x() % tileCount) * tile.size),
			srcY = qint16(cell.y() * tile.size);
		quint8 textureID = _texIdKeys.value(cell.x() / tileCount, 0);
		
		if (tile.srcX == srcX &&
			tile.srcY == srcY &&
			tile.textureID == textureID) {
			qDebug() << "tileID" << tile.tileID << "texID" << tile.textureID << "src" << tile.srcX << tile.srcY << "dst" << tile.dstX << tile.dstY;
			selectedTile = tile;
			break;
		}
	}

	_backgroundTileEditor->setTiles(QList<Tile>() << selectedTile);
	_backgroundTileEditor->setEnabled(selectedTile.tileID != quint16(-1));
}

void BackgroundEditor::updateTiles(const QList<Tile> &tiles)
{
	Q_UNUSED(tiles)

	refreshImage(currentLayer(), currentSection(), currentParamState(), currentEffect());
	refreshTexture();
}

void BackgroundEditor::compile()
{
	if (_backgroundFile == nullptr) {
		return;
	}

	if (_backgroundFile->isModified() && _backgroundFile->field()->isPC()) {
		BackgroundFilePC *backgroundFilePC = static_cast<BackgroundFilePC *>(_backgroundFile);
		if (!backgroundFilePC->compile()) {
			QMessageBox::warning(this, tr("Compilation error"), backgroundFilePC->lastErrorString());
		} else {
			refreshTexture();
		}
	}
}
