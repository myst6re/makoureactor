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

class CaFile;

struct Tile {
	qint16 dstX, dstY;
	quint8 srcX, srcY;
	quint16 ID;
	quint8 paletteID;
	quint8 param, state;
	bool blending;
	quint8 typeTrans, size;
	quint8 textureID; // textureX on PS
	quint8 textureY; // Only on PS
	quint8 depth;
	quint8 layerID;
	quint16 tileID;
	quint32 IDBig; // Only on PC
	quint8 unknown7; // Only on PC (unused)
	bool manualIDBig;
	void calcIDBig(CaFile *ca = nullptr);
};

struct LayerParam {
	quint8 layer, param;
};

int operator==(const LayerParam &layerParam, const LayerParam &other);
bool operator<(const LayerParam &layerParam, const LayerParam &other);

struct SrcTex {
	quint8 srcX, srcY;
};

struct ParamState {
	ParamState(): param(0), state(0) {}
	ParamState(quint8 param, quint8 state) : param(param), state(state) {}
	inline bool isValid() const {
		return param > 0;
	}
	quint8 param, state;
};

struct Conflict {
	quint16 tileID;
	ParamState paramState;
};

int operator==(const Conflict &conflict, const Conflict &other);
bool operator<(const Conflict &conflict, const Conflict &other);

struct DstTex {
	qint16 dstX, dstY;
};

int operator==(const DstTex &layerSrcTex, const DstTex &other);
bool operator<(const DstTex &layerSrcTex, const DstTex &other);

class BackgroundTiles : public QMultiMap<qint32, Tile>
{
public:
	BackgroundTiles();
	explicit BackgroundTiles(const QList<Tile> &tiles);
	explicit BackgroundTiles(const QMultiMap<qint32, Tile> &tiles);

	BackgroundTiles filter(const QHash<quint8, quint8> *paramActifs, const qint16 *z,
	                       const bool *layers, const QSet<quint16> *IDs, const QList<quint16> *tileIds = nullptr, bool onlyParams = false) const;
	BackgroundTiles orderedTiles(bool orderedForSaving) const;
	BackgroundTiles tiles(quint8 layerID, bool orderedForSaving = false) const;
	BackgroundTiles tiles(const QList<quint8> &layerIDs) const;
	BackgroundTiles tilesByID(quint16 ID, bool orderedForSaving = false) const;
	BackgroundTiles tiles(quint8 layerID, quint16 ID, qint16 dstX, qint16 dstY) const;
	BackgroundTiles tiles(quint8 layerID, ParamState paramState) const;
	BackgroundTiles tiles(quint8 layerID, quint16 ID) const;
	QMap<qint32, Tile> sortedTiles() const;
	QMap<LayerParam, quint8> usedParams(bool *layerExists, QSet<quint16> *usedIDs = nullptr, QList<QList<quint16> > *effectLayers = nullptr) const;
	QSet<quint8> usedPalettes() const;
	void shiftPalettes(quint8 palID, int steps);
	void area(quint16 &minWidth, quint16 &minHeight,
	          int &width, int &height) const;
	QPoint minTopLeft() const;
	QRect rect() const;
	QPoint dstShift(quint8 tileSize) const;
	Tile search(quint8 textureID1, quint8 textureID2, quint8 srcX, quint8 srcY) const;
	void setZLayer1(quint16 oldZ, quint16 newZ);
	bool replace(const Tile &tile);
	using QMultiMap<qint32, Tile>::replace;
	inline void insert(const Tile &tile) {
		QMultiMap<qint32, Tile>::insert(qint32(4096 - tile.ID), tile);
	}
	using QMultiMap<qint32, Tile>::insert;
	bool remove(const Tile &tile);
	using QMultiMap<qint32, Tile>::remove;
	bool checkOrdering() const;
	QList<SrcTex> detectHoles() const;
	
};

int operator==(const Tile &tile, const Tile &other);
