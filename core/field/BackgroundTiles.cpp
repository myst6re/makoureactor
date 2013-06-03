#include "BackgroundTiles.h"

BackgroundTiles::BackgroundTiles() :
	QMultiMap<qint16, Tile>()
{
}

BackgroundTiles::BackgroundTiles(const QMultiMap<qint16, Tile> &tiles) :
	QMultiMap<qint16, Tile>(tiles)
{
}

BackgroundTiles BackgroundTiles::tiles(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers) const
{
	QMultiMap<qint16, Tile> ret;

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

	return BackgroundTiles(ret);
}

BackgroundTiles BackgroundTiles::tiles(quint8 layerID) const
{
	QMultiMap<qint16, Tile> ret;

	foreach(const Tile &tile, *this) {
		if(tile.layerID == layerID) {
			ret.insert(4096 - tile.ID, tile);
		}
	}

	return BackgroundTiles(ret);
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
