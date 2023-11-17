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
#include "CaFile.h"

void Tile::calcIDBig(CaFile *ca)
{
	if (manualIDBig) {
		return;
	}

	// Used for conversion to PC format 
	if (ID == 0) {
		IDBig = 999;
	} else if (ID < 50) { // all: 50 (42480000h) | 466: 1 (3F800000h) | 138: 16 (41800000h) | 139: 30 (41F00000h) | 253: 56 (42600000h) | <= 55???: 512 (44000000h)
		IDBig = 10000 * ID;
	} else if (ID >= 4095) {
		IDBig = 9998999;
	} else if (ca != nullptr) {
		/* float sourceDir[3]; // [esp+44h] [ebp-58h] BYREF
		float camera_structure_copy[16]; // [esp+50h] [ebp-4Ch] BYREF
		int caMatrixX = 160, caMatrixY = 90; // FIXME
		Camera camera = ca->camera(0);
		
		sourceDir[0] = (((double)dstX - caMatrixX) * 4 * ID / camera.camera_zoom) - (double)camera.camera_position[0];
		sourceDir[1] = (((double)dstY - caMatrixY) * 4 * ID / camera.camera_zoom) - (double)camera.camera_position[1];
		sourceDir[2] = 4 * ID - (double)camera.camera_position[2];
		
		// matrix view: 1.000000, 0.000000, -0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000
		// matrix projection: 0.700000, 0.000000, 0.000000, 0.000000, 0.000000, -1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.438642, 0.437956, 0.000000, 0.000000, -21.932079, 0.000000, 
		
		camera_structure_copy[0] = (double)*camera_axis_and_position / 4096.0;
		camera_structure_copy[1] = (double)camera_axis_and_position[1] / 4096.0;
		camera_structure_copy[2] = (double)camera_axis_and_position[2] / 4096.0;
		camera_structure_copy[4] = (double)camera_axis_and_position[3] / 4096.0;
		camera_structure_copy[5] = (double)camera_axis_and_position[4] / 4096.0;
		camera_structure_copy[6] = (double)camera_axis_and_position[5] / 4096.0;
		camera_structure_copy[8] = (double)camera_axis_and_position[6] / 4096.0;
		camera_structure_copy[9] = (double)camera_axis_and_position[7] / 4096.0;
		camera_structure_copy[10] = (double)camera_axis_and_position[8] / 4096.0;
		camera_structure_copy[3] = float_div_by_one(*(_DWORD *)(camera_axis_and_position + 9));
		camera_structure_copy[7] = float_div_by_one(*(_DWORD *)(camera_axis_and_position + 11));
		camera_structure_copy[11] = float_div_by_one(*(_DWORD *)(camera_axis_and_position + 13));
		camera_structure_copy[12] = 0.0;
		camera_structure_copy[13] = 0.0;
		camera_structure_copy[14] = 0.0;
		camera_structure_copy[15] = 1.0;
		
		sub_67C323(camera_structure_copy);
		matrix_vector_multiplication(camera_structure_copy, sourceDir, matrice_out);
		
		field_bg_camera_to_float(camera_axis_and_position, camera_with_floats);
		camera_with_floats[12] = (float)*(int *)(camera_axis_and_position + 9);
		camera_with_floats[13] = (float)*(int *)(camera_axis_and_position + 11);
		camera_with_floats[14] = (float)*(int *)(camera_axis_and_position + 13);
		camera_with_floats[15] = 1.0;
		
		float v55[16]; // [esp+0h] [ebp-44h] BYREF
		float in[4];
		float out[4];
		int result; // [esp+40h] [ebp-4h]
		
		result = 0;
		if ( sub_67CEA3(camera_with_floats, v55) )
		{
			sub_66CDE0(v55, matrice_out, in);
			field_bg_z_computation_related_sub_67B997(in, out);
			if ( in[3] > 0.0 ) {
				result = 1;
			}
		} */
		// FIXME: approximation
		IDBig = quint32((float(ID) / 4096.0f) * 10000000.0);
	} else {
		// FIXME: approximation
		IDBig = quint32((float(ID) / 4096.0f) * 10000000.0);
	}
}

int operator==(const LayerParam &layerParam, const LayerParam &other)
{
	return memcmp(&layerParam, &other, sizeof(layerParam));
}

bool operator<(const LayerParam &layerParam, const LayerParam &other)
{
	return memcmp(&layerParam, &other, sizeof(layerParam)) < 0;
}

int operator==(const Conflict &conflict, const Conflict &other)
{
	return memcmp(&conflict, &other, sizeof(Conflict));
}

bool operator<(const Conflict &conflict, const Conflict &other)
{
	return memcmp(&conflict, &other, sizeof(Conflict)) < 0;
}

int operator==(const DstTex &dstTex, const DstTex &other)
{
	return memcmp(&dstTex, &other, sizeof(DstTex));
}

bool operator<(const DstTex &dstTex, const DstTex &other)
{
	return memcmp(&dstTex, &other, sizeof(DstTex)) < 0;
}

BackgroundTiles::BackgroundTiles() :
      QMultiMap<qint32, Tile>()
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

BackgroundTiles::BackgroundTiles(const QMultiMap<qint32, Tile> &tiles) :
      QMultiMap<qint32, Tile>(tiles)
{
}

BackgroundTiles BackgroundTiles::filter(const QHash<quint8, quint8> *paramActifs, const qint16 *z,
                                        const bool *layers, const QSet<quint16> *IDs, const QList<quint16> *tileIds, bool onlyParams) const
{
	BackgroundTiles ret;

	for (const Tile &tile : *this) {
		if ((layers == nullptr || layers[tile.layerID]) && (IDs == nullptr || IDs->contains(tile.ID))) {
			if (tile.layerID == 0) {
				ret.insert(tile);
			} else if (((!onlyParams && tile.state == 0) || (paramActifs == nullptr || (paramActifs->value(tile.param, 0) & tile.state)))
			           && (tileIds == nullptr || tileIds->contains(tile.tileID))) {
				if (tile.layerID == 1) {
					ret.insert(tile);
				} else {
					ret.insert(4096 - ((z != nullptr && z[tile.layerID - 2] != -1) ? z[tile.layerID - 2] : tile.ID), tile);
				}
			}
		}
	}

	return ret;
}

BackgroundTiles BackgroundTiles::orderedTiles(bool orderedForSaving) const
{
	BackgroundTiles ret;

	for (const Tile &tile : *this) {
		ret.insert(orderedForSaving
		           ? (tile.layerID << 28) | tile.tileID
		           : qint32(4096 - tile.ID), tile);
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
			               : qint32(4096 - tile.ID),
			           tile);
		}
	}
	
	return ret;
}

BackgroundTiles BackgroundTiles::tiles(const QList<quint8> &layerIDs) const
{
	BackgroundTiles ret;
	
	for (const Tile &tile : *this) {
		if (layerIDs.contains(tile.layerID)) {
			ret.insert(qint32(4096 - tile.ID), tile);
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

	if (layerID == 0 || !paramState.isValid()) {
		return ret;
	}

	for (const Tile &tile : *this) {
		if (tile.layerID == layerID && tile.param == paramState.param && tile.state == paramState.state) {
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

QMap<LayerParam, quint8> BackgroundTiles::usedParams(bool *layerExists, QSet<quint16> *usedIDs, QList< QList<quint16> > *effectLayers) const
{
	QMap<LayerParam, quint8> ret;
	QMap< Conflict, QMap<DstTex, QList<quint16> > > collectEffectLayerTexPos[3];
	layerExists[0] = layerExists[1] = layerExists[2] = false;

	if (effectLayers != nullptr) {
		effectLayers[0] = effectLayers[1] = effectLayers[2] = QList< QList<quint16> >();
	}

	for (const Tile &tile : *this) {
		if (tile.layerID > 0) {
			layerExists[tile.layerID - 1] = true;

			if (usedIDs != nullptr && tile.layerID == 1) {
				usedIDs->insert(tile.ID);
			}

			if (tile.param) {
				LayerParam layerParam;
				layerParam.layer = tile.layerID;
				layerParam.param = tile.param;
				ret.insert(layerParam, ret.value(layerParam) | tile.state);
			}

			if (effectLayers != nullptr) {
				Conflict conflict;
				conflict.tileID = tile.ID;
				ParamState paramState;
				paramState.param = tile.param;
				paramState.state = tile.state;
				conflict.paramState = paramState;
				DstTex layerDst;
				layerDst.dstX = tile.dstX;
				layerDst.dstY = tile.dstY;

				collectEffectLayerTexPos[tile.layerID - 1][conflict][layerDst].append(tile.tileID);
			}
		}
	}
	
	if (effectLayers != nullptr) {
		for (quint8 layer = 0; layer < 3; ++layer) {
			for (const QMap< DstTex, QList<quint16> > &dst: std::as_const(collectEffectLayerTexPos[layer])) {
				qsizetype maxSize = -1;
				for (const QList<quint16> &tileIDs: dst) {
					if (tileIDs.size() > maxSize) {
						maxSize = tileIDs.size();
					}
				}
				if (maxSize > 1) {
					qsizetype id = effectLayers->size();
					for (const QList<quint16> &tileIDs: dst) {
						if (tileIDs.size() > 1) {
							for (qsizetype i = 0; i < tileIDs.size(); ++i) {
								if (id + i >= effectLayers->size()) {
									effectLayers->append(QList<quint16>());
								}
								(*effectLayers)[id + i].append(tileIDs.at(i));
							}
						}
					}
				}
			}
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

void BackgroundTiles::shiftPalettes(quint8 palID, int steps)
{
	for (Tile &tile : *this) {
		if (tile.depth < 2 && tile.paletteID && tile.paletteID > palID) {
			tile.paletteID = quint8(std::min(palID + steps, 255));
		}
	}
}

void BackgroundTiles::area(quint16 &minWidth, quint16 &minHeight,
                           int &width, int &height) const
{
	quint16 maxWidth=0, maxHeight=0;
	minWidth = minHeight = 0;

	for (const Tile &tile : *this) {
		quint8 toAdd = tile.size - 16;
		if (tile.dstX >= 0 && tile.dstX+toAdd > maxWidth)
			maxWidth = tile.dstX + toAdd;
		else if (tile.dstX < 0 && -tile.dstX > minWidth)
			minWidth = -tile.dstX;
		if (tile.dstY >= 0 && tile.dstY+toAdd > maxHeight)
			maxHeight = tile.dstY + toAdd;
		else if (tile.dstY < 0 && -tile.dstY > minHeight)
			minHeight = -tile.dstY;
	}

	width = minWidth + maxWidth + 16;
	height = minHeight + maxHeight + 16;
}

QPoint BackgroundTiles::minTopLeft() const
{
	qint16 minDstX = 1024, minDstY = 1024;

	for (const Tile &tile : *this) {
		if (tile.dstX < minDstX) {
			minDstX = tile.dstX;
		}
		if (tile.dstY < minDstY) {
			minDstY = tile.dstY;
		}
	}
	qDebug() << "BackgroundTiles::minTopLeft" << QPoint(minDstX, minDstY);

	return QPoint(minDstX, minDstY);
}

QRect BackgroundTiles::rect() const
{
	quint16 minWidth, minHeight;
	int width, height;

	area(minWidth, minHeight, width, height);

	return QRect(minWidth, minHeight, width, height);
}

QPoint BackgroundTiles::dstShift(quint8 tileSize) const
{
	int shiftX = 0, shiftY = 0;
	bool shiftXComputed = false, shiftYComputed = false;

	for (const Tile &tile : *this) {
		if (tile.size != tileSize) {
			continue;
		}

		if (!shiftXComputed && tile.dstX != 0) {
			shiftX = std::abs(tile.dstX) % tile.size;
			shiftXComputed = true;
		}

		if (!shiftYComputed && tile.dstY != 0) {
			shiftY = std::abs(tile.dstY) % tile.size;
			shiftYComputed = true;
		}

		if (shiftXComputed && shiftYComputed) {
			break;
		}
	}

	return QPoint(shiftX, shiftY);
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
		if (t.tileID == tile.tileID && t.layerID == tile.layerID) {
			t = tile;
			return true;
		}
	}

	return false;
}

bool BackgroundTiles::remove(const Tile &tile)
{
	QMutableMultiMapIterator it(*this);

	while (it.hasNext()) {
		it.next();
		const Tile &t = it.value();

		if (t.layerID == tile.layerID && t.tileID == tile.tileID) {
			it.remove();

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
	QList<SrcTex> holes = detectHoles();

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
				for (const SrcTex &hole: holes) {
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
	for (const TextureCursor &cur: std::as_const(textureCursors)) {
		firstTextureIds.append(cur.firstTextureId);
		int last = lastTextureIds.isEmpty() ? -1 : lastTextureIds.last();
		lastTextureIds.append(cur.lastTextureId);
		
		if (last != -1 && cur.lastTextureId != -1 && last >= cur.lastTextureId) {
			qWarning() << "BackgroundTiles::checkOrdering wrong texture ordering" << lastTextureIds;
			return false;
		}
	}
	
	// Check tileID
	
	QList<BackgroundTiles> reordered(4);

	for (const Tile &tile : *this) {
		qint32 ordering = 0;
		if (tile.layerID == 0) {
			ordering = (tile.paletteID << 20) | (tile.dstY << 10) | tile.dstX;
		} else {
			ordering = (tile.dstY << 20) | (tile.dstX << 10) | tile.param;
		}
		reordered[tile.layerID].insert(ordering, tile);
	}

	for (quint8 layerID = 0; layerID < 4; ++layerID) {
		int tileID = 0;
		for (const Tile &tile : reordered.at(layerID)) {
			if (tile.layerID != layerID) {
				continue;
			}

			if (tile.tileID != tileID) {
				qDebug() << "BackgroundTiles::checkOrdering wrong tile order" << layerID << tile.tileID << tileID << tile.textureID << tile.srcX << tile.srcY;
				break;
			}
			++tileID;
		}
	}

	qDebug() << "BackgroundTiles::checkOrdering OK" << firstTextureIds << lastTextureIds;

	return true;
}

QList<SrcTex> BackgroundTiles::detectHoles() const
{
	QSet<quint16> uniqueHoles;
	QList<SrcTex> holes;
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
					SrcTex hole;
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
