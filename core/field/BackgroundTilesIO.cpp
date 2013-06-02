#include "BackgroundTilesIO.h"

BackgroundTilesIO::BackgroundTilesIO(QIODevice *device) :
	_device(device)
{
}

BackgroundTilesIO::~BackgroundTilesIO()
{
}

void BackgroundTilesIO::setDevice(QIODevice *device)
{
	_device = device;
}

QIODevice *BackgroundTilesIO::device() const
{
	return _device;
}

bool BackgroundTilesIO::canRead() const
{
	if(_device) {
		if(!_device->isOpen()) {
			return _device->open(QIODevice::ReadOnly);
		}
		return _device->isReadable();
	}
	return false;
}

bool BackgroundTilesIO::canWrite() const
{
	if(_device) {
		if(!_device->isOpen()) {
			return _device->open(QIODevice::WriteOnly);
		}
		return _device->isWritable();
	}
	return false;
}

bool BackgroundTilesIO::read(BackgroundTiles &tiles) const
{
	if(!canRead()) {
		return false;
	}

	return readData(tiles);
}

bool BackgroundTilesIO::write(const BackgroundTiles &tiles) const
{
	if(!canWrite()) {
		return false;
	}

	return writeData(tiles);
}

BackgroundTilesIOPC::BackgroundTilesIOPC(QIODevice *device) :
	BackgroundTilesIO(device)
{
}

bool BackgroundTilesIOPC::readData(BackgroundTiles &tiles) const
{
	qint64 cur;
	quint32 i;
	quint16 nbTiles1, nbTiles2 = 0, nbTiles3 = 0, nbTiles4 = 0;
	TilePC tile;
	QMultiMap<qint16, Tile> tileList;
	QByteArray data;
	char exists;

	if(!device()->seek(44)) {
		return false;
	}

	if(device()->read((char *)&nbTiles1, 2) != 2) {
		return false;
	}

	if(!device()->seek(52)) {
		return false;
	}

	if(nbTiles1 > 0) {
		data = device()->read(nbTiles1 * 52 - 52 + 34);

		if(data.size() != nbTiles1 * 52 - 52 + 34) {
			return false;
		}

		// BG 0
		for(i=0 ; i<nbTiles1 ; ++i) {
			memcpy(&tile, data.constData() + i*52, 34);

			if(qAbs(tile.dstX) < 1024 && qAbs(tile.dstY) < 1024) {
				tile.size = 16;
				tile.layerID = 0;

				tileList.insert(1, tilePC2Tile(tile));
			}
		}

		if(!device()->seek(52 + nbTiles1 * 52)) {
			return false;
		}
	}

	cur = device()->pos();

	// BG 1
	if(device()->getChar(&exists) && bool(exists)) {

		if(!device()->seek(cur + 5)) {
			return false;
		}

		if(device()->read((char *)&nbTiles2, 2) != 2) {
			return false;
		}

		if(nbTiles2 > 0) {

			if(!device()->seek(cur + 27)) {
				return false;
			}

			data = device()->read(nbTiles2 * 52 - 52 + 34);

			if(data.size() != nbTiles2 * 52 - 52 + 34) {
				return false;
			}

			for(i=0 ; i<nbTiles2 ; ++i) {
				memcpy(&tile, data.constData() + i*52, 34);

				if(qAbs(tile.dstX) < 1024 && qAbs(tile.dstY) < 1024) {
					if(tile.textureID2 > 0) {
						tile.srcX = tile.srcX2;
						tile.srcY = tile.srcY2;
						tile.textureID = tile.textureID2;
					}

					tile.size = 16;
					tile.layerID = 1;

					tileList.insert(4096-tile.ID, tilePC2Tile(tile));
				}
			}
		}

		if(!device()->seek(cur + 27 + nbTiles2 * 52)) {
			return false;
		}

		cur = device()->pos();
	}

	// BG 2
	if(device()->getChar(&exists) && bool(exists)) {

		if(!device()->seek(cur + 5)) {
			return false;
		}

		if(device()->read((char *)&nbTiles3, 2) != 2) {
			return false;
		}

		if(nbTiles3 > 0) {

			if(!device()->seek(cur + 21)) {
				return false;
			}

			data = device()->read(nbTiles3 * 52 - 52 + 34);

			if(data.size() != nbTiles3 * 52 - 52 + 34) {
				return false;
			}

			for(i=0 ; i<nbTiles3 ; ++i) {
				memcpy(&tile, data.constData() + i*52, 34);

				if(qAbs(tile.dstX) < 1024 && qAbs(tile.dstY) < 1024) {
					if(tile.textureID2 > 0) {
						tile.srcX = tile.srcX2;
						tile.srcY = tile.srcY2;
						tile.textureID = tile.textureID2;
					}

					tile.size = 32;
					tile.layerID = 2;

					tileList.insert(4096-tile.ID, tilePC2Tile(tile));
				}
			}

		}

		if(!device()->seek(cur + 21 + nbTiles3 * 52)) {
			return false;
		}

		cur = device()->pos();
	}

	// BG 3
	if(device()->getChar(&exists) && bool(exists)) {

		if(!device()->seek(cur + 5)) {
			return false;
		}

		if(device()->read((char *)&nbTiles4, 2) != 2) {
			return false;
		}

		if(nbTiles4 > 0) {

			if(!device()->seek(cur + 21)) {
				return false;
			}

			data = device()->read(nbTiles4 * 52 - 52 + 34);

			if(data.size() != nbTiles4 * 52 - 52 + 34) {
				return false;
			}

			for(i=0 ; i<nbTiles4 ; ++i) {
				memcpy(&tile, data.constData() + i*52, 34);

				if(qAbs(tile.dstX) < 1024 && qAbs(tile.dstY) < 1024) {
					if(tile.textureID2 > 0) {
						tile.srcX = tile.srcX2;
						tile.srcY = tile.srcY2;
						tile.textureID = tile.textureID2;
					}

					tile.size = 32;
					tile.layerID = 3;

					tileList.insert(4096-tile.ID, tilePC2Tile(tile));
				}
			}
		}

		if(!device()->seek(cur + 21 + nbTiles4 * 52)) {
			return false;
		}
	}

	tiles.setTiles(tileList);

	return true;
}

bool BackgroundTilesIOPC::writeData(const BackgroundTiles &tiles) const
{
	return false;
}

Tile BackgroundTilesIOPC::tilePC2Tile(const TilePC &tile)
{
	Tile ret;

	ret.dstX = tile.dstX;
	ret.dstY = tile.dstY;
	ret.srcX = tile.srcX;
	ret.srcY = tile.srcY;
	ret.paletteID = tile.paletteID;
	ret.ID = tile.ID;
	ret.param = tile.param;
	ret.state = tile.state;
	ret.blending = tile.blending;
	ret.typeTrans = tile.typeTrans;
	ret.size = tile.size;
	ret.textureID = tile.textureID;
	ret.textureID2 = tile.textureID2;
	ret.depth = tile.depth;
	ret.layerID = tile.layerID;

	return ret;
}

BackgroundTilesIOPS::BackgroundTilesIOPS(QIODevice *device) :
	BackgroundTilesIO(device)
{
}

bool BackgroundTilesIOPS::readData(BackgroundTiles &tiles) const
{
	QByteArray datDataDec = device()->readAll();
	const char *constDatData = datDataDec.constData();
	quint32 datDataSize = datDataDec.size();
	quint32 start1, start2, start3, start4;
	qint64 i;

	if(datDataSize < 16) {
		return false;
	}

	memcpy(&start1, constDatData, 4);
	memcpy(&start2, constDatData + 4, 4);
	memcpy(&start3, constDatData + 8, 4);
	memcpy(&start4, constDatData + 12, 4);

	quint16 tilePos=0, tileCount=0;
	QList<quint32> nbTilesTex, nbTilesLayer;
	quint8 layerID=0;
	qint16 type;

	i = 16;
	while(i<start1) {
		if(datDataSize < i+2) {
			return false;
		}

		memcpy(&type, constDatData + i, 2);

		if(type == 0x7FFF) {
			nbTilesLayer.append(tilePos+tileCount);
			++layerID;
		}
		else {
			if(type == 0x7FFE) {
				memcpy(&tilePos, constDatData + i-4, 2);
				memcpy(&tileCount, constDatData + i-2, 2);

				nbTilesTex.append(tilePos+tileCount);
			} else {
				if(datDataSize < i+6) {
					return false;
				}

				memcpy(&tilePos, constDatData + i+2, 2);
				memcpy(&tileCount, constDatData + i+4, 2);
			}
			i += 4;
		}
		i += 2;
	}

	QList<layer2Tile> tiles2;
	layer1Tile tile1;
	layer2Tile tile2;
	layer3Tile tile4;
	paramTile tile3;
	QMultiMap<qint16, Tile> tileList;
	Tile tile;
	quint16 texID=0;
	quint32 size, tileID=0;

	size = (start3-start2)/2;

	if(datDataSize < start2+size*2) {
		return false;
	}

	for(i=0 ; i<size ; ++i) {
		memcpy(&tile2, constDatData + start2+i*2, 2);
		tiles2.append(tile2);
	}

	if(tiles2.isEmpty()) {
		return false;
	}

	tile2 = tiles2.first();

	size = (start2-start1)/8;

	if(datDataSize < start1+size*8) {
		return false;
	}

	for(i=0 ; i<size ; ++i) {
		memcpy(&tile1, constDatData + start1+i*8, 8);
		if(qAbs(tile1.dstX) < 1024 && qAbs(tile1.dstY) < 1024) {
			tile.dstX = tile1.dstX;
			tile.dstY = tile1.dstY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = tile1.palID;

			if(texID+1<tiles2.size() && texID+1<nbTilesTex.size() && tileID>=nbTilesTex.at(texID)) {
				++texID;
				tile2 = tiles2.at(texID);
			}

			tile.textureID = tile2.page_x;
			tile.textureID2 = tile2.page_y;
			tile.depth = tile2.depth;
			tile.typeTrans = tile2.typeTrans;

			tile.param = tile.state = tile.blending = 0;
			tile.ID = 4095;

			tile.size = 16;
			tile.layerID = 0;

			tileList.insert(4096 - tile.ID, tile);
		}
		++tileID;
	}

	size = (start4-start3)/14;

	if(datDataSize < start3+size*14) {
		return false;
	}

	for(i=0 ; i<size ; ++i) {
		memcpy(&tile1, constDatData + start3+i*14, 8);
		if(qAbs(tile1.dstX) < 1000 || qAbs(tile1.dstY) < 1000) {
			tile.dstX = tile1.dstX;
			tile.dstY = tile1.dstY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = tile1.palID;

			memcpy(&tile2, constDatData + start3+i*14+8, 2);
			memcpy(&tile3, constDatData + start3+i*14+10, 4);

			tile.param = tile3.param;
			tile.state = tile3.state;
			tile.textureID = tile2.page_x;
			tile.textureID2 = tile2.page_y;
			tile.depth = tile2.depth;
			tile.typeTrans = tile2.typeTrans;

			tile.blending = tile3.blending;
			tile.ID = tile3.group;

			tile.size = 16;
			tile.layerID = 1;

			tileList.insert(4096 - tile.ID, tile);
		}
		++tileID;
	}

	layerID = 2;

	size = (datDataSize-start4)/10;

	if(datDataSize < start4+size*10) {
		return false;
	}

	for(i=0 ; i<size ; ++i) {
		memcpy(&tile1, constDatData + start4+i*10, 8);
		if(qAbs(tile1.dstX) < 1000 || qAbs(tile1.dstY) < 1000) {
			tile.dstX = tile1.dstX;
			tile.dstY = tile1.dstY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = tile1.palID;

			memcpy(&tile4, constDatData + start4+i*10+8, 2);

			tile.param = tile4.param;
			tile.state = tile4.state;

			if(texID+1<tiles2.size() && texID+1<nbTilesTex.size() && tileID>=nbTilesTex.at(texID)) {
				++texID;
				tile2 = tiles2.at(texID);
			}

			if(layerID+1<nbTilesLayer.size() && tileID>=nbTilesLayer.at(layerID)) {
				++layerID;
			}

			tile.blending = tile4.blending;
			tile.textureID = tile2.page_x;
			tile.textureID2 = tile2.page_y;
			tile.depth = tile2.depth;
			tile.typeTrans = tile2.typeTrans;

			tile.ID = layerID==2 ? 4096 : 0;
			tile.size = 32;
			tile.layerID = layerID;

			tileList.insert(4096 - tile.ID, tile);
		}
		++tileID;
	}

	tiles.setTiles(tileList);

	return true;
}

bool BackgroundTilesIOPS::writeData(const BackgroundTiles &tiles) const
{
	return false;
}
