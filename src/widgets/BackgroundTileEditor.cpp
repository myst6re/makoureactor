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
#include "BackgroundTileEditor.h"
#include "core/field/BackgroundFile.h"
#include "core/field/Field.h"
#include "PsColorDialog.h"
#include "core/Config.h"
#include "QColorPicker.h"

BackgroundTileEditor::BackgroundTileEditor(QWidget *parent)
    : QWidget(parent)
{
	_formPage = new QWidget(this);

	_tileWidget = new ImageGridWidget(_formPage);
	_tileWidget->setCellSize(1);
	_tileWidget->setFixedHeight(32 * 4);
	_tileWidget->setCursor(QCursor(Qt::CrossCursor));

	_bgParamGroup = new QGroupBox(tr("Parameter"), _formPage);
	_bgParamGroup->setCheckable(true);
	
	_bgParamInput = new QSpinBox(_bgParamGroup);
	_bgParamInput->setRange(1, 255);
	_bgParamStateInput = new QSpinBox(_bgParamGroup);
	_bgParamStateInput->setRange(0, 7);

	QFormLayout *tileParameterEditorLayout = new QFormLayout(_bgParamGroup);
	tileParameterEditorLayout->addRow(tr("Param ID"), _bgParamInput);
	tileParameterEditorLayout->addRow(tr("State ID"), _bgParamStateInput);

	_blendTypeInput = new QComboBox(_formPage);
	_blendTypeInput->addItem(tr("None"), -1);
	_blendTypeInput->addItem(tr("Average"), 0);
	_blendTypeInput->addItem(tr("Plus"), 1);
	_blendTypeInput->addItem(tr("Minus"), 2);
	_blendTypeInput->addItem(tr("Source +25% destination"), 3);

	_paletteIdInput = new QSpinBox(_formPage);
	_paletteIdInput->setRange(0, 255);

	_depthInput = new QSpinBox(_formPage);
	_depthInput->setRange(0, 4096);

	_depthTuningInput = new QSpinBox(_formPage);
	_depthTuningInput->setRange(0, 10000000);

	_depthTuningAutoInput = new QCheckBox(_formPage);
	_depthTuningAutoInput->setChecked(true);

	_tileEditorLayout = new QFormLayout();
	_tileEditorLayout->addRow(tr("Blend type"), _blendTypeInput);
	_tileEditorLayout->addRow(tr("Palette ID"), _paletteIdInput);
	_tileEditorLayout->addRow(tr("Depth (Z)"), _depthInput);
	_tileEditorLayout->addRow(tr("Depth fine tune auto"), _depthTuningAutoInput);
	_tileEditorLayout->addRow(tr("Depth fine tune"), _depthTuningInput);

	QPushButton *removeButton = new QPushButton(tr("Delete selected tiles"), _formPage);
	_exportButton = new QPushButton(tr("Export tile image"), _formPage);
	_importButton = new QPushButton(tr("Import tile image"), _formPage);

	QVBoxLayout *layout = new QVBoxLayout(_formPage);
	layout->addWidget(_tileWidget);
	layout->addWidget(_exportButton);
	layout->addWidget(_importButton);
	layout->addLayout(_tileEditorLayout);
	layout->addWidget(_bgParamGroup);
	layout->addWidget(removeButton, 0, Qt::AlignRight);
	layout->addStretch(1);
	layout->setContentsMargins(QMargins());

	_createPage = new QWidget(this);

	_colorTypeInput = new QComboBox(_createPage);
	_colorTypeInput->addItem(tr("Paletted 8-bit"), 1);
	_colorTypeInput->addItem(tr("Direct color 16-bit"), 2);

	_createDepthInput = new QSpinBox(_createPage);
	_createDepthInput->setRange(0, 4096);

	QPushButton *createButton = new QPushButton(tr("Create tile"), _createPage);

	_tileCreateLayout = new QFormLayout();
	_tileCreateLayout->addRow(tr("Color type"), _colorTypeInput);
	_tileCreateLayout->addRow(tr("Depth (Z)"), _createDepthInput);

	_colorShowLabel = new QColorShowLabel(_createPage);
	_colorShowLabel->setMinimumHeight(60);

	QVBoxLayout *createPageLayout = new QVBoxLayout(_createPage);
	createPageLayout->addLayout(_tileCreateLayout);
	createPageLayout->addWidget(_colorShowLabel);
	createPageLayout->addWidget(createButton, 0, Qt::AlignRight);
	createPageLayout->addStretch(1);

	_stackedLayout = new QStackedLayout(this);
	_stackedLayout->addWidget(_formPage);
	_stackedLayout->addWidget(_createPage);
	
	connect(_tileWidget, &ImageGridWidget::clicked, this, &BackgroundTileEditor::choosePixelColor);
	connect(_blendTypeInput, &QComboBox::currentIndexChanged, this, &BackgroundTileEditor::updateBlendType);
	connect(createButton, &QPushButton::clicked, this, &BackgroundTileEditor::createTile);
	connect(_bgParamInput, &QSpinBox::valueChanged, this, &BackgroundTileEditor::updateBgParam);
	connect(_bgParamStateInput, &QSpinBox::valueChanged, this, &BackgroundTileEditor::updateBgState);
	connect(_bgParamGroup, &QGroupBox::clicked, this, &BackgroundTileEditor::updateBgParamEnabled);
	connect(_paletteIdInput, &QSpinBox::valueChanged, this, &BackgroundTileEditor::updatePaletteId);
	connect(_depthInput, &QSpinBox::valueChanged, this, &BackgroundTileEditor::updateDepth);
	connect(_depthTuningAutoInput, &QAbstractButton::toggled, this, &BackgroundTileEditor::disableDepthTuningInput);
	connect(_depthTuningInput, &QSpinBox::valueChanged, this, &BackgroundTileEditor::updateDepthTuning);
	connect(removeButton, &QPushButton::clicked, this, &BackgroundTileEditor::removeTiles);
	connect(_exportButton, &QPushButton::clicked, this, &BackgroundTileEditor::exportImage);
	connect(_importButton, &QPushButton::clicked, this, &BackgroundTileEditor::importImage);
	connect(_colorShowLabel, &QColorShowLabel::clicked, this, &BackgroundTileEditor::selectNewColor);
	connect(_colorTypeInput, &QComboBox::currentIndexChanged, this, &BackgroundTileEditor::updateColorShowEnabled);
}

void BackgroundTileEditor::setBackgroundFile(BackgroundFile *backgroundFile)
{
	_backgroundFile = backgroundFile;
	if (_backgroundFile != nullptr) {
		_paletteIdInput->setMaximum(_backgroundFile->palettes().size() - 1);
	}
}

void BackgroundTileEditor::clear()
{
	_backgroundFile = nullptr;
	setTiles(QList<Tile>());
}

void BackgroundTileEditor::setTiles(const QList<Tile> &tiles)
{
	_tiles = tiles;

	bool isInvalid = false;
	int layerID = 0;
	QSet<int> params, states, paletteIDs, blendings, typeTranss, depths, depthBigs, depthBigsManual;

	for (const Tile &tile: tiles) {
		layerID = tile.layerID;
		params.insert(tile.param);
		states.insert(tile.state);
		if (tile.depth < 2) {
			paletteIDs.insert(tile.paletteID);
		}
		blendings.insert(tile.blending);
		typeTranss.insert(tile.typeTrans);
		depths.insert(tile.ID);
		depthBigs.insert(tile.IDBig);
		depthBigsManual.insert(tile.manualIDBig);
		if (tile.tileID == quint16(-1)) {
			isInvalid = true;
		}
	}

	qDebug() << "BackgroundTileEditor::setTiles" << tiles.size() << params << states << paletteIDs << blendings << typeTranss << depths << depthBigs;

	if (isInvalid) {
		_stackedLayout->setCurrentWidget(_createPage);

		_colorTypeInput->setCurrentIndex(_colorTypeInput->findData(2));
		_colorTypeInput->setEnabled(true);
		updateColorShowEnabled();
		_tileCreateLayout->labelForField(_colorTypeInput)->setEnabled(true);
		if (layerID == 1) {
			_createDepthInput->setValue(!depths.isEmpty() ? *depths.begin() : 0);
			_createDepthInput->setVisible(true);
			_tileCreateLayout->labelForField(_createDepthInput)->setVisible(true);
		} else {
			_createDepthInput->setVisible(false);
			_tileCreateLayout->labelForField(_createDepthInput)->setVisible(false);
		}
		
	} else {
		_stackedLayout->setCurrentWidget(_formPage);

		if (tiles.size() == 1 && _backgroundFile != nullptr) {
			_tileWidget->setPixmap(QPixmap::fromImage(_backgroundFile->textures()->toImage(tiles.first(), _backgroundFile->palettes())));
			_tileWidget->setEnabled(true);
			_exportButton->setEnabled(true);
			_importButton->setEnabled(true);
		} else {
			_tileWidget->setPixmap(QPixmap());
			_tileWidget->setEnabled(false);
			_exportButton->setEnabled(false);
			_importButton->setEnabled(false);
		}

		if (params.size() == 1 && layerID > 0) {
			_bgParamGroup->setEnabled(true);
			_bgParamGroup->setChecked(*params.begin() > 0);
			_bgParamInput->blockSignals(true);
			_bgParamInput->setValue(*params.begin());
			_bgParamInput->blockSignals(false);
			_bgParamInput->setEnabled(true);

			if (states.size() == 1) {
				_bgParamStateInput->blockSignals(true);
				quint8 state = *states.begin();
				for (quint8 i = 0; i < 8; ++i) {
					if ((state >> i) & 1) {
						state = i;
						break;
					}
				}
				_bgParamStateInput->setValue(state);
				_bgParamStateInput->blockSignals(false);
				_bgParamStateInput->setEnabled(true);
			} else {
				_bgParamStateInput->blockSignals(true);
				_bgParamStateInput->setValue(0);
				_bgParamStateInput->blockSignals(false);
				_bgParamStateInput->setEnabled(false);
			}
		} else {
			_bgParamGroup->setEnabled(false);
			_bgParamGroup->setChecked(false);
			_bgParamInput->blockSignals(true);
			_bgParamInput->setValue(1);
			_bgParamInput->blockSignals(false);
			_bgParamInput->setEnabled(false);
		}

		_paletteIdInput->blockSignals(true);
		_paletteIdInput->setVisible(!paletteIDs.isEmpty());
		_tileEditorLayout->labelForField(_paletteIdInput)->setVisible(!paletteIDs.isEmpty());
		if (paletteIDs.size() == 1) {
			_paletteIdInput->setValue(*paletteIDs.begin());
			_paletteIdInput->setEnabled(true);
			_tileEditorLayout->labelForField(_paletteIdInput)->setEnabled(true);
		} else {
			_paletteIdInput->setValue(0);
			_paletteIdInput->setEnabled(false);
			_tileEditorLayout->labelForField(_paletteIdInput)->setEnabled(false);
		}
		_paletteIdInput->blockSignals(false);

		_blendTypeInput->blockSignals(true);
		if (blendings.size() == 1 && (typeTranss.size() == 1 || !*blendings.begin())) {
			_blendTypeInput->setCurrentIndex(*blendings.begin() ? *typeTranss.begin() + 1 : 0);
			_blendTypeInput->setEnabled(layerID > 0);
			_tileEditorLayout->labelForField(_blendTypeInput)->setEnabled(layerID > 0);
		} else {
			_blendTypeInput->setCurrentIndex(-1);
			_blendTypeInput->setEnabled(false);
			_tileEditorLayout->labelForField(_blendTypeInput)->setEnabled(false);
		}
		_blendTypeInput->blockSignals(false);
		
		_depthInput->blockSignals(true);
		if (depths.size() == 1) {
			_depthInput->setValue(*depths.begin());
			_depthInput->setEnabled(layerID == 1);
			_tileEditorLayout->labelForField(_depthInput)->setEnabled(layerID == 1);
		} else {
			_depthInput->setValue(0);
			_depthInput->setEnabled(false);
			_tileEditorLayout->labelForField(_depthInput)->setEnabled(false);
		}
		_depthInput->blockSignals(false);
		
		_depthTuningInput->blockSignals(true);
		if (depthBigs.size() == 1) {
			_depthTuningInput->setValue(*depthBigs.begin());
			_depthTuningInput->setEnabled(layerID == 1);
		} else {
			_depthTuningInput->setValue(0);
			_depthTuningInput->setEnabled(false);
		}
		_tileEditorLayout->labelForField(_depthTuningInput)->setEnabled(_depthTuningInput->isEnabled());
		
		_depthTuningAutoInput->blockSignals(true);
		_depthTuningAutoInput->setChecked(depthBigsManual.size() != 1 || !(*depthBigsManual.begin()));
		_depthTuningAutoInput->setEnabled(_depthTuningInput->isEnabled());
		if (_depthTuningAutoInput->isChecked()) {
			_depthTuningInput->setEnabled(false);
		}
		_tileEditorLayout->labelForField(_depthTuningAutoInput)->setEnabled(_depthTuningAutoInput->isEnabled());
		_depthTuningAutoInput->blockSignals(false);
		_depthTuningInput->blockSignals(false);
	}
}

void BackgroundTileEditor::choosePixelColor(const Cell &cell)
{
	if (_backgroundFile == nullptr || _tiles.isEmpty()) {
		return;
	}

	const QPixmap &pixmap = _tileWidget->pixmap();
	QList<QRgb> colors;

	QList<uint> indexes = _backgroundFile->textures()->tile(_tiles.first());
	qsizetype i = cell.x() + cell.y() * pixmap.width();
	if (i >= indexes.size()) {
		return;
	}
	uint indexOrColor = indexes.at(i);

	if (_paletteIdInput->isEnabled() && _paletteIdInput->isVisible()) {
		colors = _backgroundFile->palettes().value(_paletteIdInput->value())->colors();
	}
	
	PsColorDialog *dialog = !colors.isEmpty() ? new PsColorDialog(colors, quint8(indexOrColor), this) : new PsColorDialog(QColor::fromRgba(QRgb(indexOrColor)), this);
	if (dialog->exec() == QDialog::Accepted) {
		uint indexOrColor = dialog->indexOrColor();
		qsizetype i = cell.x() + cell.y() * pixmap.width();
		bool modified = false;

		for (Tile &tile: _tiles) {
			QList<uint> indexOrColors = _backgroundFile->textures()->tile(tile);
			if (i < indexOrColors.size()) {
				indexOrColors.replace(i, indexOrColor);

				if (!_backgroundFile->textures()->setTile(tile, indexOrColors)) {
					qWarning() << "BackgroundTileEditor::choosePixelColor cannot modify tile" << tile.tileID;
				} else {
					modified = true;
				}
			}
		}

		if (modified) {
			_backgroundFile->setModified(_backgroundFile->field()->isPC());

			emit changed(_tiles);
	
			_tileWidget->setPixmap(QPixmap::fromImage(_backgroundFile->textures()->toImage(_tiles.first(), _backgroundFile->palettes())));
		}
	}
}

void BackgroundTileEditor::updateBlendType(int index)
{
	if (index < 0) {
		return;
	}

	quint8 blendType = quint8(_blendTypeInput->itemData(index).toInt());
	bool blending = true;

	if (blendType > 3) {
		blendType = 0;
		blending = false;
	}

	for (Tile &tile: _tiles) {
		tile.typeTrans = blendType;
		tile.blending = blending;
		
		if (!_backgroundFile->setTile(tile)) {
			qWarning() << "BackgroundTileEditor::updateBlendType tile not found" << tile.tileID;
		}
	}

	emit changed(_tiles);
}

void BackgroundTileEditor::createTile()
{
	QList<Tile> modifiedTiles;

	for (Tile &tile: _tiles) {
		if (tile.tileID == quint16(-1)) {
			tile.depth = quint8(_colorTypeInput->currentData().toInt());
			if (tile.layerID == 1) {
				tile.ID = quint16(_createDepthInput->value());
			}
			uint colorOrIndex = 0;
			if (tile.depth == 2) {
				colorOrIndex = _colorShowLabel->color().rgba();
			}
			if (!_backgroundFile->addTile(tile, colorOrIndex)) {
				QMessageBox::warning(this, tr("No more space"), tr("No more space available in the file for a new Tile"));
			} else {
				modifiedTiles.append(tile);
			}
		}
	}

	setTiles(_tiles);

	emit changed(modifiedTiles);
}

void BackgroundTileEditor::updateBgParam(int value)
{
	if (value < 0) {
		return;
	}

	quint8 bgParam = quint8(value);

	for (Tile &tile: _tiles) {
		tile.param = bgParam;

		if (!_backgroundFile->setTile(tile)) {
			qWarning() << "BackgroundTileEditor::updateBgParam tile not found" << tile.tileID;
		}
	}

	emit changed(_tiles);
}

void BackgroundTileEditor::updateBgState(int value)
{
	if (value < 0) {
		return;
	}

	quint8 bgState = quint8(1 << value);

	for (Tile &tile: _tiles) {
		tile.state = bgState;

		if (!_backgroundFile->setTile(tile)) {
			qWarning() << "BackgroundTileEditor::updateBgState tile not found" << tile.tileID;
		}
	}

	emit changed(_tiles);
}

void BackgroundTileEditor::updateBgParamEnabled(bool enabled)
{
	blockSignals(true);
	updateBgParam(enabled ? _bgParamInput->value() : 0);
	blockSignals(false);
	updateBgState(enabled ? _bgParamStateInput->value() : 0);
}

void BackgroundTileEditor::updatePaletteId(int value)
{
	if (value < 0) {
		return;
	}

	quint8 paletteId = quint8(value);

	for (Tile &tile: _tiles) {
		tile.paletteID = paletteId;
		
		if (!_backgroundFile->setTile(tile)) {
			qWarning() << "BackgroundTileEditor::updatePaletteId tile not found" << tile.tileID;
		}
	}

	// Update tile preview
	if (_backgroundFile != nullptr && _tiles.size() == 1) {
		_tileWidget->setPixmap(QPixmap::fromImage(_backgroundFile->textures()->toImage(_tiles.first(), _backgroundFile->palettes())));
	}

	emit changed(_tiles);
}

void BackgroundTileEditor::updateDepth(int value)
{
	if (value < 0) {
		return;
	}

	quint16 depth = quint16(value);

	for (Tile &tile: _tiles) {
		tile.ID = depth;

		if (!_backgroundFile->setTile(tile)) {
			qWarning() << "BackgroundTileEditor::updateDepth tile not found" << tile.tileID;
		}
	}

	emit changed(_tiles);
}

void BackgroundTileEditor::disableDepthTuningInput(bool disabled)
{
	_depthTuningInput->setEnabled(!disabled);

	for (Tile &tile: _tiles) {
		tile.manualIDBig = !disabled;
		
		if (!_backgroundFile->setTile(tile)) {
			qWarning() << "BackgroundTileEditor::updateDepth tile not found" << tile.tileID;
		}
	}

	emit changed(_tiles);
}

void BackgroundTileEditor::updateDepthTuning(int depth)
{
	if (depth < 0) {
		return;
	}

	for (Tile &tile: _tiles) {
		tile.IDBig = depth;
		tile.manualIDBig = true;
		
		if (!_backgroundFile->setTile(tile)) {
			qWarning() << "BackgroundTileEditor::updateDepth tile not found" << tile.tileID;
		}
	}
	
	emit changed(_tiles);
}

void BackgroundTileEditor::exportImage()
{
	if (_tiles.size() != 1) {
		return;
	}

	QString filePath = Config::value("saveBGPath").toString();
	if (!filePath.isEmpty()) {
		filePath.append("/");
	}
	filePath = QFileDialog::getSaveFileName(this, tr("Export tile image"),
	                                        QString("%1%2-tile-%3.png").arg(filePath, _backgroundFile->field()->name()).arg(_tiles.first().tileID),
	                                        tr("PNG image (*.png);;BMP image (*.bmp)"));
	
	if (filePath.isNull()) {
		return;
	}

	_backgroundFile->textures()->toImage(_tiles.first(), _backgroundFile->palettes()).save(filePath);
}

void BackgroundTileEditor::importImage()
{
	if (_tiles.size() != 1) {
		return;
	}
	
	QString filePath = Config::value("saveBGPath").toString();
	filePath = QFileDialog::getOpenFileName(this, tr("Import tile image"), filePath,
	                             tr("PNG image (*.png);;BMP image (*.bmp)"));
	
	if (filePath.isNull()) {
		return;
	}
	
	QImage pix;
	pix.load(filePath);
	
	if (pix.width() != pix.height() || pix.width() != _tiles.first().size) {
		QMessageBox::warning(this, tr("Invalid size"), tr("Please import an image with size %1x%1").arg(_tiles.first().size));
		return;
	}
	QList<uint> indexOrColor;

	if (pix.format() == QImage::Format_Indexed8) {
		const uchar *bits = pix.constBits();
		for (int y = 0; y < pix.height(); ++y) {
			for (int x = 0; x < pix.width(); ++x) {
				indexOrColor.append(*bits);
				++bits;
			}
		}
	} else {
		const QRgb *bits = reinterpret_cast<const QRgb *>(pix.constBits());
		for (int y = 0; y < pix.height(); ++y) {
			for (int x = 0; x < pix.width(); ++x) {
				indexOrColor.append(*bits);
				++bits;
			}
		}
	}
	
	_backgroundFile->textures()->setTile(_tiles.first(), indexOrColor);
	_backgroundFile->setModified(_backgroundFile->field()->isPC());
	
	emit changed(_tiles);
	
	_tileWidget->setPixmap(QPixmap::fromImage(_backgroundFile->textures()->toImage(_tiles.first(), _backgroundFile->palettes())));
}

void BackgroundTileEditor::removeTiles()
{
	for (Tile &tile: _tiles) {
		if (!_backgroundFile->removeTile(tile)) {
			qWarning() << "BackgroundTileEditor::updateDepth tile not found" << tile.tileID;
		}
		tile.tileID = quint16(-1);
	}

	emit changed(_tiles);

	setTiles(_tiles);
}

void BackgroundTileEditor::selectNewColor()
{
	PsColorDialog *dialog = new PsColorDialog(QColor::fromRgba(qRgb(0, 0, 0)), this);
	if (dialog->exec() == QDialog::Accepted) {
		uint indexOrColor = dialog->indexOrColor();
		_colorShowLabel->setColor(QColor::fromRgba(indexOrColor));
	}
}

void BackgroundTileEditor::updateColorShowEnabled()
{
	_colorShowLabel->setEnabled(_colorTypeInput->currentData().toInt() == 2);
}
