#include "BackgroundTiles.h"

BackgroundTiles::BackgroundTiles() :
	QMultiMap<qint16, Tile>()
{
}

BackgroundTiles::BackgroundTiles(const QMultiMap<qint16, Tile> &tiles) :
	QMultiMap<qint16, Tile>(tiles)
{
}

BackgroundTiles BackgroundTiles::filter(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers) const
{
	BackgroundTiles ret;

	foreach(const Tile &tile, *this) {
		switch(tile.layerID) {
		case 0:
			if(layers==NULL || layers[0]) {
				ret.insert(1, tile);
			}
			break;
		case 1:
			if((tile.state==0 || paramActifs.value(tile.param, 0) & tile.state)
					&& (layers==NULL || layers[1])) {
				ret.insert(4096 - tile.ID, tile);
			}
			break;
		case 2:
			if((tile.state==0 || paramActifs.value(tile.param, 0) & tile.state)
					&& (layers==NULL || layers[2])) {
				ret.insert(4096 - (z[0]!=-1 ? z[0] : tile.ID), tile);
			}
			break;
		case 3:
			if((tile.state==0 || paramActifs.value(tile.param, 0) & tile.state)
					&& (layers==NULL || layers[3])) {
				ret.insert(4096 - (z[1]!=-1 ? z[1] : tile.ID), tile);
			}
			break;
		}
	}

	return ret;
}

BackgroundTiles BackgroundTiles::tiles(quint8 layerID, bool orderedForSaving) const
{
	BackgroundTiles ret;

	foreach(const Tile &tile, *this) {
		if(tile.layerID == layerID) {
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

	foreach(const Tile &tile, *this) {
		if(ret.contains((tile.layerID << 16) | tile.tileID)) {
			qWarning() << "BackgroundTiles::sortedTiles() tile not unique!" << tile.layerID << tile.tileID;
		}
		ret.insert((tile.layerID << 16) | tile.tileID, tile);
	}

	return ret;
}

QHash<quint8, quint8> BackgroundTiles::usedParams(bool *layerExists) const
{
	QHash<quint8, quint8> ret;
	layerExists[0] = layerExists[1] = layerExists[2] = false;

	foreach(const Tile &tile, *this) {
		switch(tile.layerID) {
		case 0:
			break;
		case 1:
			layerExists[0] = true;
			if(tile.param) {
				ret.insert(tile.param, ret.value(tile.param) | tile.state);
			}
			break;
		case 2:
			layerExists[1] = true;
			if(tile.param) {
				ret.insert(tile.param, ret.value(tile.param) | tile.state);
			}
			break;
		case 3:
			layerExists[2] = true;
			if(tile.param) {
				ret.insert(tile.param, ret.value(tile.param) | tile.state);
			}
			break;
		}
	}

	return ret;
}

void BackgroundTiles::area(quint16 &minWidth, quint16 &minHeight,
						   int &width, int &height) const
{
	quint16 maxWidth=0, maxHeight=0;
	minWidth = minHeight = 0;

	foreach(const Tile &tile, *this) {
		quint8 toAdd = tile.size - 16;
		if(tile.dstX >= 0 && tile.dstX+toAdd > maxWidth)
			maxWidth = tile.dstX+toAdd;
		else if(tile.dstX < 0 && -tile.dstX > minWidth)
			minWidth = -tile.dstX;
		if(tile.dstY >= 0 && tile.dstY+toAdd > maxHeight)
			maxHeight = tile.dstY+toAdd;
		else if(tile.dstY < 0 && -tile.dstY > minHeight)
			minHeight = -tile.dstY;
	}

	width = minWidth + maxWidth + 16;
	height = minHeight + maxHeight + 16;
}

Tile BackgroundTiles::search(quint8 textureID1, quint8 textureID2,
							 quint8 srcX, quint8 srcY) const
{
	foreach(const Tile &tile, *this) {
		if(tile.textureID == textureID1 &&
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
