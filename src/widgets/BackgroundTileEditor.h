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
#include "core/field/BackgroundTiles.h"

class BackgroundFile;

class BackgroundTileEditor : public QWidget
{
	Q_OBJECT
public:
	explicit BackgroundTileEditor(QWidget *parent = nullptr);
	void setBackgroundFile(BackgroundFile *backgroundFile);
	void clear();
signals:
public slots:
	inline void setTile(const Tile &tile) {
		setTiles(QList<Tile>() << tile);
	}
	void setTiles(const QList<Tile> &tiles);
private slots:
	void createTile();
private:
	ImageGridWidget *_tileWidget;
	QGroupBox *_bgParamGroup;
	QSpinBox *_bgParamInput, *_bgParamStateInput;
	QComboBox *_blendTypeInput, *_depthInput;
	QFormLayout *_tileEditorLayout;
	QSpinBox *_paletteIdInput;
	QStackedLayout *_stackedLayout;
	QWidget *_formPage;
	QWidget *_createPage;

	BackgroundFile *_backgroundFile;
	QList<Tile> _tiles;
};
