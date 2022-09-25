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
#include "EditBGLabel.h"
#include "ImageGridWidget.h"
#include "core/field/BackgroundTiles.h"

class BackgroundFile;

class BackgroundEditor : public QWidget
{
	Q_OBJECT
public:
	explicit BackgroundEditor(QWidget *parent = nullptr);
	void setSections(const QList<quint16> &sections);
	void setBackgroundFile(BackgroundFile *backgroundFile);
	void clear();
signals:
	void modified();
private slots:
	void updateCurrentLayer(int layer);
	void updateCurrentSection(QListWidgetItem *current, QListWidgetItem *previous);
	void updateZ(int z);
	void updateSelectedTiles(const QList<Cell> &cells);
	void updateSelectedTile(int index);
private:
	int currentSection() const;
	void updateCurrentSection2(int section);
	void updateImageLabel(int layer, int section);

	QComboBox *_layersComboBox;
	QListWidget *_sectionsList;
	QSpinBox *_tileCountWidthSpinBox, *_tileCountHeightSpinBox, *_zSpinBox;
	EditBGLabel *_editBGLabel;
	ImageGridWidget *_backgroundLayerWidget, *_tileWidget;
	QVBoxLayout *_rightPaneLayout;
	QComboBox *_currentTileComboBox;
	QGroupBox *_bgParamGroup;
	QSpinBox *_bgParamInput, *_bgParamStateInput;
	QComboBox *_blendTypeInput, *_depthInput;
	QFormLayout *_tileEditorLayout;
	QSpinBox *_paletteIdInput;

	ImageGridWidget *_texturesWidget;

	BackgroundFile *_backgroundFile;
	QList<Tile> _selectedTiles;
	quint16 _currentOffsetX, _currentOffsetY;
};
