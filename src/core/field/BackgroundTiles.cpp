/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#include "BackgroundTiles.h"

BackgroundTiles::BackgroundTiles() :
	QMultiMap<qint16, Tile>()
{
}

BackgroundTiles::BackgroundTiles(const QList<Tile> &tiles)
{
	for (const Tile &tile : tiles) {
		switch (tile.layerID) {
		case 0:
			insert(1, tile);
			break;
		case 1:
			insert(qint16(4096 - tile.ID), tile);
			break;
		case 2:
			insert(0, tile);
			break;
		case 3:
			insert(4096, tile);
			break;
		}
	}
}

BackgroundTiles::BackgroundTiles(const QMultiMap<qint16, Tile> &tiles) :
	QMultiMap<qint16, Tile>(tiles)
{
}

BackgroundTiles BackgroundTiles::filter(const QHash<quint8, quint8> &paramActifs, const qint16 *z,
                                        const bool *layers, const QSet<quint16> *IDs) const
{
	BackgroundTiles ret;

	for (const Tile &tile : *this) {
		switch (tile.layerID) {
		case 0:
			if ((layers == nullptr || layers[0]) && (IDs == nullptr || IDs->contains(1))) {
				ret.insert(1, tile);
			}
			break;
		case 1:
			if ((tile.state == 0 || paramActifs.value(tile.param, 0) & tile.state)
					&& (layers == nullptr || layers[1]) && (IDs == nullptr || IDs->contains(tile.ID))) {
				ret.insert(4096 - tile.ID, tile);
			}
			break;
		case 2:
			if ((tile.state == 0 || paramActifs.value(tile.param, 0) & tile.state)
					&& (layers == nullptr || layers[2]) && (IDs == nullptr || IDs->contains(tile.ID))) {
				ret.insert(4096 - ((z && z[0] != -1) ? z[0] : tile.ID), tile);
			}
			break;
		case 3:
			if ((tile.state == 0 || paramActifs.value(tile.param, 0) & tile.state)
					&& (layers == nullptr || layers[3]) && (IDs == nullptr || IDs->contains(tile.ID))) {
				ret.insert(4096 - ((z && z[1] != -1) ? z[1] : tile.ID), tile);
			}
			break;
		}
	}

	return ret;
}

BackgroundTiles BackgroundTiles::tiles(quint8 layerID, bool orderedForSaving) const
{
	BackgroundTiles ret;

	for (const Tile &tile : *this) {
		if (tile.layerID == layerID) {
			ret.insert(orderedForSaving
					   ? tile.tileID
					   : 4096 - tile.ID,
					   tile);
		}
	}

	return ret;
}

BackgroundTiles BackgroundTiles::tilesByID(quint16 ID, bool orderedForSaving) const
{
	BackgroundTiles ret;

	for (const Tile &tile : *this) {
		if (tile.ID == ID) {
			ret.insert(orderedForSaving
					   ? tile.tileID
					   : 4096 - tile.ID,
					   tile);
		}
	}

	return ret;
}

QMap<qint32, Tile> BackgroundTiles::sortedTiles() const
{
	QMap<qint32, Tile> ret;

	for (const Tile &tile : *this) {
		if (ret.contains((tile.layerID << 16) | tile.tileID)) {
			qWarning() << "BackgroundTiles::sortedTiles() tile not unique!" << tile.layerID << tile.tileID;
		}
		ret.insert((tile.layerID << 16) | tile.tileID, tile);
	}

	return ret;
}

QHash<quint8, quint8> BackgroundTiles::usedParams(bool *layerExists, QSet<quint16> *usedIDs) const
{
	QHash<quint8, quint8> ret;
	layerExists[0] = layerExists[1] = layerExists[2] = false;

	for (const Tile &tile : *this) {
		switch (tile.layerID) {
		case 0:
			break;
		case 1:
			layerExists[0] = true;
			if (tile.param) {
				ret.insert(tile.param, ret.value(tile.param) | tile.state);
			}
			if (usedIDs) {
				usedIDs->insert(tile.ID);
			}
			break;
		case 2:
			layerExists[1] = true;
			if (tile.param) {
				ret.insert(tile.param, ret.value(tile.param) | tile.state);
			}
			break;
		case 3:
			layerExists[2] = true;
			if (tile.param) {
				ret.insert(tile.param, ret.value(tile.param) | tile.state);
			}
			break;
		}
	}

	return ret;
}

QSet<quint8> BackgroundTiles::usedPalettes() const
{
	QSet<quint8> ret;

	for (const Tile &tile : *this) {
		if (tile.depth < 2) {
			ret.insert(tile.paletteID);
		}
	}

	return ret;
}

void BackgroundTiles::area(quint16 &minWidth, quint16 &minHeight,
						   int &width, int &height) const
{
	quint16 maxWidth=0, maxHeight=0;
	minWidth = minHeight = 0;

	for (const Tile &tile : *this) {
		quint8 toAdd = tile.size - 16;
		if (tile.dstX >= 0 && tile.dstX+toAdd > maxWidth)
			maxWidth = tile.dstX+toAdd;
		else if (tile.dstX < 0 && -tile.dstX > minWidth)
			minWidth = -tile.dstX;
		if (tile.dstY >= 0 && tile.dstY+toAdd > maxHeight)
			maxHeight = tile.dstY+toAdd;
		else if (tile.dstY < 0 && -tile.dstY > minHeight)
			minHeight = -tile.dstY;
	}

	width = minWidth + maxWidth + 16;
	height = minHeight + maxHeight + 16;
}

QSize BackgroundTiles::area() const
{
	quint16 minWidth, minHeight;
	int width, height;

	area(minWidth, minHeight, width, height);

	return QSize(width, height);
}

Tile BackgroundTiles::search(quint8 textureID1, quint8 textureID2,
							 quint8 srcX, quint8 srcY) const
{
	for (const Tile &tile : *this) {
		if (tile.textureID == textureID1 &&
				(textureID2 == quint8(-1) || tile.textureID2 == textureID2) &&
				tile.srcX == srcX &&
				tile.srcY == srcY) {
			return tile;
		}
	}

	Tile nullTile = Tile();
	nullTile.tileID = quint16(-1);
	return nullTile;
}

int operator==(const Tile &tile, const Tile &other)
{
	return memcmp(&tile, &other, sizeof(tile) - 4);
}
