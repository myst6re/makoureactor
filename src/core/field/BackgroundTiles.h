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

#include <QtCore>

struct Tile {
	qint16 dstX, dstY;
	quint8 srcX, srcY;
	quint8 paletteID;
	quint16 ID;
	quint8 param, state;
	bool blending;
	quint8 typeTrans, size;
	quint8 textureID, textureID2;
	quint8 depth;
	quint8 layerID;
	quint16 tileID;
	quint32 IDBig; // Only on PC
};

class BackgroundTiles : public QMultiMap<qint16, Tile>
{
public:
	BackgroundTiles();
	explicit BackgroundTiles(const QList<Tile> &tiles);
	explicit BackgroundTiles(const QMultiMap<qint16, Tile> &tiles);

	BackgroundTiles filter(const QHash<quint8, quint8> *paramActifs, const qint16 *z,
	                       const bool *layers, const QSet<quint16> *IDs) const;
	BackgroundTiles tiles(quint8 layerID, bool orderedForSaving = false) const;
	BackgroundTiles tilesByID(quint16 ID, bool orderedForSaving = false) const;
	QMap<qint32, Tile> sortedTiles() const;
	QHash<quint8, quint8> usedParams(bool *layerExists, QSet<quint16> *usedIDs = nullptr) const;
	QSet<quint8> usedPalettes() const;
	void area(quint16 &minWidth, quint16 &minHeight,
	          int &width, int &height) const;
	QSize area() const;
	Tile search(quint8 textureID1, quint8 textureID2, quint8 srcX, quint8 srcY) const;
	Tile search(quint8 layerID, qint16 dstX, qint16 dstY, quint16 ID = 0) const;
	void setZLayer1(quint16 oldZ, quint16 newZ);
};

int operator==(const Tile &tile, const Tile &other);
