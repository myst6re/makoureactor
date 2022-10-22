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
#include "BackgroundTiles.h"
#include "BackgroundTextures.h"

int operator==(const LayerParam &layerParam, const LayerParam &other)
{
	return memcmp(&layerParam, &other, sizeof(layerParam));
}

bool operator<(const LayerParam &layerParam, const LayerParam &other)
{
	return memcmp(&layerParam, &other, sizeof(layerParam)) < 0;
}

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
			insert(tile);
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

BackgroundTiles BackgroundTiles::filter(const QHash<quint8, quint8> *paramActifs, const qint16 *z,
                                        const bool *layers, const QSet<quint16> *IDs, bool onlyParams) const
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
			if (((!onlyParams && tile.state == 0) || (paramActifs == nullptr || paramActifs->value(tile.param, 0) & tile.state))
			    && (layers == nullptr || layers[1]) && (IDs == nullptr || IDs->contains(tile.ID))) {
				ret.insert(tile);
			}
			break;
		case 2:
			if (((!onlyParams && tile.state == 0) || (paramActifs == nullptr || paramActifs->value(tile.param, 0) & tile.state))
			    && (layers == nullptr || layers[2]) && (IDs == nullptr || IDs->contains(tile.ID))) {
				ret.insert(4096 - ((z && z[0] != -1) ? z[0] : tile.ID), tile);
			}
			break;
		case 3:
			if (((!onlyParams && tile.state == 0) || (paramActifs == nullptr || paramActifs->value(tile.param, 0) & tile.state))
			    && (layers == nullptr || layers[3]) && (IDs == nullptr || IDs->contains(tile.ID))) {
				ret.insert(4096 - ((z && z[1] != -1) ? z[1] : tile.ID), tile);
			}
			break;
		}
	}
	
	return ret;
}

BackgroundTiles BackgroundTiles::orderedTiles() const
{
	BackgroundTiles ret;

	for (const Tile &tile : *this) {
		ret.insert(tile.tileID, tile);
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

BackgroundTiles BackgroundTiles::tiles(quint8 layerID, quint16 ID) const
{
	BackgroundTiles ret;

	for (const Tile &tile : *this) {
		if (tile.layerID == layerID &&
		        ((layerID == 1 && tile.ID == ID) || layerID != 1)) {
			ret.insert(tile);
		}
	}

	return ret;
}

BackgroundTiles BackgroundTiles::tiles(quint8 layerID, ParamState paramState) const
{
	BackgroundTiles ret;

	for (const Tile &tile : *this) {
		if (tile.layerID == layerID &&
		        layerID >= 1 && tile.param == paramState.param && tile.state == paramState.state) {
			ret.insert(tile);
		}
	}

	return ret;
}

BackgroundTiles BackgroundTiles::tiles(quint8 layerID, quint16 ID, qint16 dstX, qint16 dstY) const
{
	BackgroundTiles ret;

	for (const Tile &tile : *this) {
		if (tile.layerID == layerID &&
		        ((layerID == 1 && tile.ID == ID) || layerID != 1) &&
		        tile.dstX == dstX &&
		        tile.dstY == dstY) {
			ret.insert(tile);
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

QMap<LayerParam, quint8> BackgroundTiles::usedParams(bool *layerExists, QSet<quint16> *usedIDs) const
{
	QMap<LayerParam, quint8> ret;
	layerExists[0] = layerExists[1] = layerExists[2] = false;
	
	for (const Tile &tile : *this) {
		LayerParam layerParam;
		layerParam.layer = tile.layerID;
		layerParam.param = tile.param;

		switch (tile.layerID) {
		case 0:
			break;
		case 1:
			layerExists[0] = true;
			if (tile.param) {
				quint8 state =  ret.value(layerParam);
				ret.insert(layerParam, state | tile.state);
			}
			if (usedIDs) {
				usedIDs->insert(tile.ID);
			}
			break;
		case 2:
			layerExists[1] = true;
			if (tile.param) {
				ret.insert(layerParam, ret.value(layerParam) | tile.state);
			}
			break;
		case 3:
			layerExists[2] = true;
			if (tile.param) {
				ret.insert(layerParam, ret.value(layerParam) | tile.state);
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

QRect BackgroundTiles::rect() const
{
	quint16 minWidth, minHeight;
	int width, height;

	area(minWidth, minHeight, width, height);

	return QRect(minWidth, minHeight, width, height);
}

Tile BackgroundTiles::search(quint8 textureID1, quint8 textureID2,
                             quint8 srcX, quint8 srcY) const
{
	for (const Tile &tile : *this) {
		if (tile.textureID == textureID1 &&
		    (textureID2 == quint8(-1) || tile.textureY == textureID2) &&
		    tile.srcX == srcX &&
		    tile.srcY == srcY) {
			return tile;
		}
	}
	
	Tile nullTile = Tile();
	nullTile.tileID = quint16(-1);
	return nullTile;
}

void BackgroundTiles::setZLayer1(quint16 oldZ, quint16 newZ)
{
	if (oldZ == newZ) {
		return;
	}

	for (Tile &tile : *this) {
		if (tile.layerID == 1 && tile.ID == oldZ) {
			tile.ID = newZ;
		}
	}
}

bool BackgroundTiles::replace(const Tile &tile)
{
	for (Tile &t : *this) {
		if (t.tileID == tile.tileID) {
			t = tile;
			return true;
		}
	}

	return false;
}

struct TextureCursor {
	TextureCursor() : x(0), y(0), firstTextureId(-1), lastTextureId(-1) {}
	QSet<quint8> textureIds;
	int x, y;
	qint16 firstTextureId, lastTextureId;
};

bool BackgroundTiles::checkOrdering() const
{
	const QMap<qint32, Tile> &tiles = sortedTiles();
	QMap<BackgroundTexturesPC::TextureGroups, TextureCursor> textureCursors;
	QSet<quint8> layerIds;
	qint16 lastLayerId = -1, previousTextureId = -1;
	QList<Hole> holes = detectHoles();

	if (!holes.isEmpty()) {
		qWarning() << "BackgroundTiles::checkOrdering" << holes.size() << "holes detected";
	}

	for (const Tile &tile : tiles) {
		if (!layerIds.contains(tile.layerID)) {
			//qDebug() << "layer" << tile.layerID;
			if (tile.layerID <= lastLayerId) {
				qWarning() << "BackgroundTiles::checkOrdering wrong layerID ordering" << tile.layerID << tile.textureID << tile.srcX << tile.srcY;
				return false;
			}
			layerIds.insert(tile.layerID);
			if (previousTextureId >= 0 && tile.layerID > 1 && tile.textureID == previousTextureId) {
				qWarning() << "BackgroundTiles::checkOrdering wrong texture id break on layer 2 or 3" << tile.layerID << tile.textureID << tile.srcX << tile.srcY;
				return false;
			}
		}

		lastLayerId = tile.layerID;
		BackgroundTexturesPC::TextureGroups group = BackgroundTexturesPC::textureGroup(tile);
		TextureCursor &cur = textureCursors[group];
		//qDebug() << "BackgroundTiles::checkOrdering" << tile.layerID << tile.textureID << tile.tileID << tile.size << tile.depth << tile.blending << tile.typeTrans;

		if (cur.firstTextureId == -1) {
			cur.firstTextureId = tile.textureID;
		}
		previousTextureId = tile.textureID;

		if (!cur.textureIds.contains(tile.textureID)) {
			//qDebug() << "texture alpha" << tile.textureID;
			if (tile.textureID <= cur.lastTextureId) {
				qWarning() << "BackgroundTiles::checkOrdering wrong textureId ordering" << group << cur.lastTextureId << tile.layerID << tile.textureID << tile.srcX << tile.srcY << cur.x << cur.y << cur.textureIds;
				return false;
			}
			cur.textureIds.insert(tile.textureID);
			cur.x = 0;
			cur.y = 0;
			//qDebug() << "reset x,y";
		}
		if (cur.y >= 256) {
			qWarning() << "BackgroundTiles::checkOrdering wrong y" << group << cur.lastTextureId << tile.layerID << tile.textureID << tile.srcX << tile.srcY << cur.x << cur.y << cur.textureIds;
			return false;
		}

		if (tile.srcX != cur.x || tile.srcY != cur.y) {
			bool foundHole = false;
			while (tile.srcX > cur.x || tile.srcY > cur.y) {
				for (const Hole &hole: holes) {
					if (cur.x == hole.srcX && cur.y == hole.srcY) {
						foundHole = true;
						break;
					}
				}

				if (!foundHole) {
					break;
				}

				cur.x += tile.size;

				if (cur.x >= 256) {
					cur.x = 0;
					cur.y += tile.size;
				}
			}
			if (!foundHole) {
				qWarning() << "BackgroundTiles::checkOrdering wrong pos ordering" << group << tile.layerID << tile.textureID << tile.tileID << tile.srcX << tile.srcY << cur.x << cur.y << cur.textureIds;
				return false;
			}
		}
		cur.lastTextureId = tile.textureID;
		cur.x += tile.size;

		if (cur.x >= 256) {
			cur.x = 0;
			cur.y += tile.size;
		}
	}

	QList<int> firstTextureIds, lastTextureIds;
	for (const TextureCursor &cur: qAsConst(textureCursors)) {
		firstTextureIds.append(cur.firstTextureId);
		int last = lastTextureIds.isEmpty() ? -1 : lastTextureIds.last();
		lastTextureIds.append(cur.lastTextureId);
		
		if (last != -1 && cur.lastTextureId != -1 && last >= cur.lastTextureId) {
			qWarning() << "BackgroundTiles::checkOrdering wrong texture ordering" << lastTextureIds;
			return false;
		}
	}

	qDebug() << "BackgroundTiles::checkOrdering OK" << firstTextureIds << lastTextureIds;

	return true;
}

QList<Hole> BackgroundTiles::detectHoles() const
{
	QSet<quint16> uniqueHoles;
	QList<Hole> holes;
	QMultiMap<quint8, quint16> usedTextureTiles;
	QMap<quint8, quint8> textureSizes;

	for (const Tile &tile: *this) {
		textureSizes.insert(tile.textureID, tile.size);
		quint8 textureWidth = tile.size == 16 ? 16 : 8;
		quint16 t = tile.srcY / tile.size * textureWidth + tile.srcX / tile.size;
		if (!usedTextureTiles.values(tile.textureID).contains(t)) {
			usedTextureTiles.insert(tile.textureID, t);
		}
	}

	QList<quint8> textureIds = usedTextureTiles.keys();
	for (quint8 textureId: textureIds) {
		QList<quint16> tiles = usedTextureTiles.values(textureId);
		std::sort(tiles.begin(), tiles.end());
		quint8 tileSize = textureSizes.value(textureId);
		quint8 textureWidth = tileSize == 16 ? 16 : 8;
		int i = 0;
		for (quint16 tileId: tiles) {
			if (i != tileId) {
				Tile tile = Tile();
				tile.srcX = (i % textureWidth) * tileSize;
				tile.srcY = (i / textureWidth) * tileSize;
				if (!uniqueHoles.contains(tile.srcX | (tile.srcY << 8))) {
					uniqueHoles.insert(tile.srcX | (tile.srcY << 8));
					Hole hole;
					hole.srcX = tile.srcX;
					hole.srcY = tile.srcY;
					holes.append(hole);
				}
				i = tileId;
			}
			++i;
		}
	}

	return holes;
}

int operator==(const Tile &tile, const Tile &other)
{
	return memcmp(&tile, &other, sizeof(tile) - 4);
}
