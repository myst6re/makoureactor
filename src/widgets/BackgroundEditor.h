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
#include "ImageGridWidget.h"
#include "BackgroundTileEditor.h"
#include "core/field/BackgroundTiles.h"

class BackgroundFile;

class BackgroundEditor : public QWidget
{
	Q_OBJECT
public:
	explicit BackgroundEditor(QWidget *parent = nullptr);
	void setSections(const QList<quint16> &sections);
	void setParams(const QMap<LayerParam, quint8> &params);
	void setBackgroundFile(BackgroundFile *backgroundFile);
	void clear();
signals:
	void modified();
protected:
	void showEvent(QShowEvent *event) override;
private slots:
	void updateCurrentLayer(int layer);
	void updateCurrentSection(QListWidgetItem *current, QListWidgetItem *previous);
	void updateCurrentParam(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void updateSelectedTiles(const QList<Cell> &cells);
	void updateSelectedTilesTexture(const QList<Cell> &cells);
	void updateTiles(const QList<Tile> &tiles);
private:
	void fillSectionList();
	int currentLayer() const;
	int currentSection() const;
	ParamState currentParamState() const;
	void updateCurrentSection2(int section);
	void updateCurrentParam2(int param, int state);
	void updateImageLabel(int layer, int section, int param, int state);
	void refreshImage(int layer, int section, int param, int state);
	void refreshTexture();

	QComboBox *_layersComboBox;
	QListWidget *_sectionsList;
	QTreeWidget *_paramsList;
	QScrollArea *_backgroundLayerScrollArea;
	ImageGridWidget *_backgroundLayerWidget, *_texturesWidget, *_palettesWidget;
	BackgroundTileEditor *_backgroundTileEditor;
	bool _isParamMode;

	BackgroundFile *_backgroundFile;
	QList<quint16> _sections;
	QList<quint8> _texIdKeys;
};
