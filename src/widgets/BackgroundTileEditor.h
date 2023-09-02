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
#pragma once

#include <QtWidgets>
#include <ImageGridWidget>
#include "core/field/BackgroundTiles.h"

class BackgroundFile;
class QColorShowLabel;

class BackgroundTileEditor : public QWidget
{
	Q_OBJECT
public:
	explicit BackgroundTileEditor(QWidget *parent = nullptr);
	void setBackgroundFile(BackgroundFile *backgroundFile);
	void clear();
signals:
	void changed(const QList<Tile> &tiles);
public slots:
	void choosePixelColor(const Cell &cell);
	inline void setTile(const Tile &tile) {
		setTiles(QList<Tile>() << tile);
	}
	void setTiles(const QList<Tile> &tiles);
private slots:
	void updateBlendType(int index);
	void createTile();
	void updateBgParam(int value);
	void updateBgState(int value);
	void updateBgParamEnabled(bool enabled);
	void updatePaletteId(int value);
	void updateDepth(int value);
	void disableDepthTuningInput(bool disabled);
	void updateDepthTuning(int value);
	void exportImage();
	void importImage();
	void removeTiles();
	void selectNewColor();
	void updateColorShowEnabled();
private:
	ImageGridWidget *_tileWidget;
	QGroupBox *_bgParamGroup;
	QSpinBox *_bgParamInput, *_bgParamStateInput;
	QComboBox *_blendTypeInput, *_colorTypeInput;
	QSpinBox *_depthInput, *_depthTuningInput;
	QCheckBox *_depthTuningAutoInput;
	QFormLayout *_tileEditorLayout, *_tileCreateLayout;
	QSpinBox *_paletteIdInput;
	QStackedLayout *_stackedLayout;
	QWidget *_formPage;
	QWidget *_createPage;
	QSpinBox *_createDepthInput;
	QPushButton *_exportButton, *_importButton;
	QColorShowLabel *_colorShowLabel;

	BackgroundFile *_backgroundFile;
	QList<Tile> _tiles;
};
