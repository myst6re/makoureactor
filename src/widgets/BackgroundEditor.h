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
#include <ImageGridWidget.h>
#include "BackgroundTileEditor.h"
#include "core/field/BackgroundTiles.h"

class BackgroundFile;

class BackgroundEditor : public QWidget
{
	Q_OBJECT
public:
	explicit BackgroundEditor(QWidget *parent = nullptr);
	void setBackgroundFile(BackgroundFile *backgroundFile);
	void clear();
	void saveConfig();
signals:
	void modified();
protected:
	void showEvent(QShowEvent *event) override;
private slots:
	void updateCurrentLayer(int layer);
	void updateCurrentSection(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void updateSelectedTiles(const QList<Cell> &cells);
	void updateSelectedTileTexture(const Cell &cell);
	void updateTiles(const QList<Tile> &tiles);
	void compile();
private:
	enum LayerSubType {
		SubLayer = QTreeWidgetItem::UserType,
		BackgroundParameter,
		Effect
	};
	void addTopLevelItem(const QString &name, LayerSubType subType);
	int currentLayer() const;
	int currentSection() const;
	ParamState currentParamState() const;
	QList<quint16> currentEffect() const;
	void updateImageLabel(int layer, int section, ParamState paramState, const QList<quint16> &effectTileIds);
	void refreshList(int layer);
	void refreshImage(int layer, int section, ParamState paramState, const QList<quint16> &effectTileIds);
	void refreshTexture();
	static QPoint backgroundPositionFromTile(const QPoint &tile, const QPoint &shift);
	static QPoint tilePositionFromCell(const QPoint &cell, quint8 cellSize, const QPoint &shift);

	QSplitter *_topBottomSplitter;
	QComboBox *_layersComboBox;
	QTreeWidget *_sectionsList;
	QSpinBox *_shiftX, *_shiftY;
	QScrollArea *_backgroundLayerScrollArea;
	ImageGridWidget *_backgroundLayerWidget, *_texturesWidget, *_palettesWidget;
	BackgroundTileEditor *_backgroundTileEditor;
	QPushButton *_compileButton;

	BackgroundFile *_backgroundFile;
	QList<quint8> _texIdKeys;
};
