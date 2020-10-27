/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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
#include "BackgroundTilesIO.h"

BackgroundTilesIO::BackgroundTilesIO(QIODevice *device) :
	IO(device)
{
}

bool BackgroundTilesIO::read(BackgroundTiles &tiles) const
{
	if (!canRead()) {
		return false;
	}

	tiles.clear();

	return readData(tiles);
}

bool BackgroundTilesIO::write(const BackgroundTiles &tiles) const
{
	if (!canWrite()) {
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
	QByteArray data;
	char exists;
	const quint8 sizeofTile = sizeof(TilePC);

	if (!device()->seek(44)) {
		qWarning() << "BackgroundTilesIOPC::readData device size too short";
		return false;
	}

	if (device()->read((char *)&nbTiles1, 2) != 2) {
		qWarning() << "BackgroundTilesIOPC::readData cannot read nbTiles1";
		return false;
	}

	if (!device()->seek(52)) {
		qWarning() << "BackgroundTilesIOPC::readData cannot seek (1)";
		return false;
	}

	if (nbTiles1 > 0) {
		data = device()->read(nbTiles1 * 52 - 52 + sizeofTile);

		if (data.size() != nbTiles1 * 52 - 52 + sizeofTile) {
			qWarning() << "BackgroundTilesIOPC::readData cannot read tiles data (1)";
			return false;
		}

		// BG 0
		for (i=0; i<nbTiles1; ++i) {
			memcpy(&tile, data.constData() + i*52, sizeofTile);

			if (qAbs(tile.dstX) < MAX_TILE_DST && qAbs(tile.dstY) < MAX_TILE_DST) {
				tiles.insert(1, tilePC2Tile(tile, 0, i));
			} else {
				qWarning() << "Tile destination overflow 0" << i << tile.dstX << tile.dstY;
			}
		}

		if (!device()->seek(52 + nbTiles1 * 52)) {
			qWarning() << "BackgroundTilesIOPC::readData cannot seek (2)";
			return false;
		}
	}

	cur = device()->pos();

	if (!device()->getChar(&exists)) {
		qWarning() << "BackgroundTilesIOPC::readData cannot read exists2";
		return false;
	}

	// BG 1
	if (bool(exists)) {

		if (!device()->seek(cur + 5)) {
			qWarning() << "BackgroundTilesIOPC::readData cannot seek (3)";
			return false;
		}

		if (device()->read((char *)&nbTiles2, 2) != 2) {
			qWarning() << "BackgroundTilesIOPC::readData cannot read nbTiles2";
			return false;
		}

		if (nbTiles2 > 0) {

			if (!device()->seek(cur + 27)) {
				qWarning() << "BackgroundTilesIOPC::readData cannot seek (4)";
				return false;
			}

			data = device()->read(nbTiles2 * 52 - 52 + sizeofTile);

			if (data.size() != nbTiles2 * 52 - 52 + sizeofTile) {
				qWarning() << "BackgroundTilesIOPC::readData cannot read tiles data (2)";
				return false;
			}

			for (i=0; i<nbTiles2; ++i) {
				memcpy(&tile, data.constData() + i*52, sizeofTile);

				if (qAbs(tile.dstX) < MAX_TILE_DST && qAbs(tile.dstY) < MAX_TILE_DST) {
					tiles.insert(4096 - tile.ID, tilePC2Tile(tile, 1, i));
				} else {
					qWarning() << "Tile destination overflow 1" << i << tile.dstX << tile.dstY;
				}
			}
		}

		if (!device()->seek(cur + 27 + nbTiles2 * 52)) {
			qWarning() << "BackgroundTilesIOPC::readData cannot seek (5)";
			return false;
		}
	}

	cur = device()->pos();

	if (!device()->getChar(&exists)) {
		qWarning() << "BackgroundTilesIOPC::readData cannot read exists3";
		return false;
	}

	// BG 2
	if (bool(exists)) {

		if (!device()->seek(cur + 5)) {
			qWarning() << "BackgroundTilesIOPC::readData cannot seek (6)";
			return false;
		}

		if (device()->read((char *)&nbTiles3, 2) != 2) {
			qWarning() << "BackgroundTilesIOPC::readData cannot read nbTiles3";
			return false;
		}

		if (nbTiles3 > 0) {

			if (!device()->seek(cur + 21)) {
				qWarning() << "BackgroundTilesIOPC::readData cannot seek (7)";
				return false;
			}

			data = device()->read(nbTiles3 * 52 - 52 + sizeofTile);

			if (data.size() != nbTiles3 * 52 - 52 + sizeofTile) {
				qWarning() << "BackgroundTilesIOPC::readData cannot read tiles data (3)";
				return false;
			}

			for (i=0; i<nbTiles3; ++i) {
				memcpy(&tile, data.constData() + i*52, sizeofTile);

				if (qAbs(tile.dstX) < MAX_TILE_DST && qAbs(tile.dstY) < MAX_TILE_DST) {
					tiles.insert(0, tilePC2Tile(tile, 2, i));
				} else {
					qWarning() << "Tile destination overflow 2" << i << tile.dstX << tile.dstY;
				}
			}

		}

		if (!device()->seek(cur + 21 + nbTiles3 * 52)) {
			qWarning() << "BackgroundTilesIOPC::readData cannot seek (8)";
			return false;
		}
	}

	cur = device()->pos();

	if (!device()->getChar(&exists)) {
		qWarning() << "BackgroundTilesIOPC::readData cannot read exists4";
		return false;
	}

	// BG 3
	if (bool(exists)) {

		if (!device()->seek(cur + 5)) {
			qWarning() << "BackgroundTilesIOPC::readData cannot seek (9)";
			return false;
		}

		if (device()->read((char *)&nbTiles4, 2) != 2) {
			qWarning() << "BackgroundTilesIOPC::readData cannot read nbTiles4";
			return false;
		}

		if (nbTiles4 > 0) {

			if (!device()->seek(cur + 21)) {
				qWarning() << "BackgroundTilesIOPC::readData cannot seek (10)";
				return false;
			}

			data = device()->read(nbTiles4 * 52 - 52 + sizeofTile);

			if (data.size() != nbTiles4 * 52 - 52 + sizeofTile) {
				qWarning() << "BackgroundTilesIOPC::readData cannot read tiles data (4)";
				return false;
			}

			for (i=0; i<nbTiles4; ++i) {
				memcpy(&tile, data.constData() + i*52, sizeofTile);

				if (qAbs(tile.dstX) < MAX_TILE_DST && qAbs(tile.dstY) < MAX_TILE_DST) {
					tiles.insert(4096, tilePC2Tile(tile, 3, i));
				} else {
					qWarning() << "Tile destination overflow 3" << i << tile.dstX << tile.dstY;
				}
			}
		}

		if (!device()->seek(cur + 21 + nbTiles4 * 52)) {
			qWarning() << "BackgroundTilesIOPC::readData cannot seek (11)";
			return false;
		}
	}

	return true;
}

bool BackgroundTilesIOPC::writeTile(const Tile &tile) const
{
	quint32 srcXBig, srcYBig;

	device()->write("\0\0", 2);
	TilePC tilePC = tile2TilePC(tile);
	device()->write((char *)&tilePC, sizeof(TilePC));
	srcXBig = tilePC.srcX / 16 * 625000;
	srcYBig = tilePC.srcY / 16 * 625000;
	device()->write((char *)&srcXBig, 4);
	device()->write((char *)&srcYBig, 4);
	device()->write("\0\0", 2);

	return true;
}

bool BackgroundTilesIOPC::writeData(const BackgroundTiles &tiles) const
{
	quint16 nbTiles, depth = 1;
	int w, h;

	// Layer 1

	QSize area = tiles.area();

	w = area.width() + 16;
	h = area.height() + 16;

	BackgroundTiles tiles1 = tiles.tiles(0, true);
	nbTiles = tiles1.size();

	device()->write((char *)&w, 2);
	device()->write((char *)&h, 2);
	device()->write((char *)&nbTiles, 2);
	device()->write((char *)&depth, 2);
	device()->write("\0\0", 2);

	for (const Tile &tile : qAsConst(tiles1)) {
		writeTile(tile);
	}

	device()->write("\0\0", 2);

	w = 640;
	h = 480;

	// Layer 2

	BackgroundTiles tiles2 = tiles.tiles(1, true);

	if (!tiles2.isEmpty()) {
		nbTiles = tiles2.size();

		device()->putChar('\x01');
		device()->write((char *)&w, 2);
		device()->write((char *)&h, 2);
		device()->write((char *)&nbTiles, 2);
		device()->write(QByteArray(16, '\0')); // Unknown but unused
		device()->write("\0\0", 2);

		for (const Tile &tile : qAsConst(tiles2)) {
			writeTile(tile);
		}

		device()->write("\0\0", 2);
	} else {
		device()->putChar('\0');
	}

	// Layer 3

	BackgroundTiles tiles3 = tiles.tiles(2, true);

	if (!tiles3.isEmpty()) {
		nbTiles = tiles3.size();

		device()->putChar('\x01');
		device()->write((char *)&w, 2);
		device()->write((char *)&h, 2);
		device()->write((char *)&nbTiles, 2);
		device()->write(QByteArray(10, '\0'));
		device()->write("\0\0", 2);

		for (const Tile &tile : qAsConst(tiles3)) {
			writeTile(tile);
		}

		device()->write("\0\0", 2);
	} else {
		device()->putChar('\0');
	}

	// Layer 4

	BackgroundTiles tiles4 = tiles.tiles(3, true);

	if (!tiles4.isEmpty()) {
		nbTiles = tiles4.size();

		device()->putChar('\x01');
		device()->write((char *)&w, 2);
		device()->write((char *)&h, 2);
		device()->write((char *)&nbTiles, 2);
		device()->write(QByteArray(10, '\0')); // Unknown but unused
		device()->write("\0\0", 2);

		for (const Tile &tile : qAsConst(tiles4)) {
			writeTile(tile);
		}

		device()->write("\0\0", 2);
	} else {
		device()->putChar('\0');
	}

	return true;
}

Tile BackgroundTilesIOPC::tilePC2Tile(const TilePC &tile, quint8 layerID, quint16 tileID)
{
	Tile ret;

	if (layerID > 0 && tile.textureID2 > 0) {
		ret.srcX = tile.srcX2;
		ret.srcY = tile.srcY2;
		ret.textureID = tile.textureID2;
	} else {
		ret.srcX = tile.srcX;
		ret.srcY = tile.srcY;
		ret.textureID = tile.textureID;
	}
	ret.dstX = tile.dstX;
	ret.dstY = tile.dstY;
	ret.paletteID = tile.paletteID;
	if (layerID > 0) {
		ret.param = tile.param;
		ret.state = tile.state;
		ret.blending = tile.blending;
	} else {
		ret.param = ret.state = ret.blending = 0;
	}
	switch (layerID) {
	case 0:		ret.ID = 4095;		break;
	case 2:		ret.ID = 4096;		break;
	case 3:		ret.ID = 0;			break;
	default:	ret.ID = tile.ID;	break;
	}
	ret.typeTrans = tile.typeTrans;
	ret.textureID2 = tile.textureID2;
	ret.depth = tile.depth;
	ret.size = layerID > 1 ? 32 : 16;
	ret.layerID = layerID;
	ret.tileID = tileID;
	ret.IDBig = tile.IDBig;

	return ret;
}

TilePC BackgroundTilesIOPC::tile2TilePC(const Tile &tile)
{
	TilePC ret = TilePC();

	ret.dstX = tile.dstX;
	ret.dstY = tile.dstY;
	ret.srcX = tile.srcX;
	ret.srcY = tile.srcY;
	ret.srcX2 = tile.srcX;
	ret.srcY2 = tile.srcY;
	ret.width = tile.size;
	ret.height = tile.size;
	ret.paletteID = tile.paletteID;
	ret.ID = tile.ID;
	ret.param = tile.param;
	ret.state = tile.state;
	ret.blending = tile.blending;
	ret.typeTrans = tile.typeTrans;
	ret.textureID = tile.textureID2 > 0 ? 0 : tile.textureID;
	ret.textureID2 = tile.textureID2;
	ret.depth = tile.depth;
	ret.IDBig = tile.IDBig;

	return ret;
}

BackgroundTilesIOPS::BackgroundTilesIOPS(QIODevice *device, bool demo) :
	BackgroundTilesIO(device), _demo(demo)
{
}

bool BackgroundTilesIOPS::readData(BackgroundTiles &tiles) const
{
	QByteArray datDataDec = device()->readAll();
	const char *constDatData = datDataDec.constData();
	quint32 datDataSize = datDataDec.size();
	quint32 start1, start2, start3, start4;
	qint64 i;
	bool isDemoFormat = _demo;

	if (datDataSize < 16) {
		return false;
	}

	memcpy(&start1, constDatData, 4);
	memcpy(&start2, constDatData + 4, 4);
	memcpy(&start3, constDatData + 8, 4);
	memcpy(&start4, constDatData + 12, 4);

	if (start2 < start1
			|| start3 < start2
			|| start4 < start3
			|| datDataSize < start4) {
		isDemoFormat = start2 >= start1
					   && start3 >= start2
					   && datDataSize >= start3;
		if (!isDemoFormat) {
			qWarning() << "BackgroundTilesIOPS::readData invalid header"
					   << start1 << start2 << start3 << start4 << datDataSize;
			return false;
		}
		start4 = datDataSize;
	}

	quint16 tilePos=0, tileCount=0;
	QList<quint32> nbTilesTex, nbTilesLayer;
	quint8 layerID=0;

	i = isDemoFormat ? 12 : 16;
	while (i < start1) {
		if (start1 < i+2) {
			return false;
		}

		qint16 type;

		memcpy(&type, constDatData + i, 2);

		if (type == 0x7FFF) {
			nbTilesLayer.append(tilePos + tileCount);
			++layerID;
		} else {
			if (type == 0x7FFE) {
				if (i - 4 < 16) {
					qWarning() << "BackgroundTilesIOPS::readData 0x7FFE positionned too early";
					return false;
				}

				memcpy(&tilePos, constDatData + i-4, 2);
				memcpy(&tileCount, constDatData + i-2, 2);

				nbTilesTex.append(tilePos + tileCount);
			} else {
				if (start1 < i+6) {
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
	Tile tile;
	quint16 texID=0;
	quint32 size, tileID=0;

	size = (start3-start2)/2;

	if ((start3-start2) % 2 != 0) {
		qWarning() << "BackgroundTilesIOPS::open padding after (1)" << ((start3-start2) % 2);
	}

	for (i=0; i<size; ++i) {
		quint16 tile2data; // Note: we can't use bitfields directly
		memcpy(&tile2data, constDatData + start2+i*2, 2);
		tile2.page_x = tile2data & 0xF;
		tile2.page_y = (tile2data >> 4) & 0x1;
		tile2.typeTrans = (tile2data >> 5) & 0x3;
		tile2.depth = (tile2data >> 7) & 0x3;
		tiles2.append(tile2);
	}

	bool hasTiles2 = !tiles2.isEmpty();
	tile2 = tiles2.value(0);

	size = (start2-start1)/8;

	if ((start2-start1) % 8 != 0) {
		qWarning() << "BackgroundTilesIOPS::open padding after (2)" << ((start2-start1) % 8);
	}

	for (i=0; i<size && hasTiles2; ++i) {
		memcpy(&tile1, constDatData + start1+i*8, 8);
		if (qAbs(tile1.dstX) < MAX_TILE_DST && qAbs(tile1.dstY) < MAX_TILE_DST) {
			tile.dstX = tile1.dstX;
			tile.dstY = tile1.dstY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = (tile1.palID >> 6) & 0xF;

			if (texID+1<tiles2.size() && texID+1<nbTilesTex.size() && tileID>=nbTilesTex.at(texID)) {
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
			tile.tileID = i;
			tile.IDBig = 0;

			tiles.insert(1, tile);
		} else {
			qWarning() << "Tile destination overflow 0" << tile1.dstX << tile1.dstY;
		}
		++tileID;
	}

	size = (start4-start3)/14;

	if ((start4-start3) % 14 != 0) {
		qWarning() << "BackgroundTilesIOPS::open padding after (3)" << ((start4-start3) % 14);
	}

	for (i=0; i<size; ++i) {
		memcpy(&tile1, constDatData + start3+i*14, 8);
		if (qAbs(tile1.dstX) < MAX_TILE_DST || qAbs(tile1.dstY) < MAX_TILE_DST) {
			tile.dstX = tile1.dstX;
			tile.dstY = tile1.dstY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = (tile1.palID >> 6) & 0xF;

			quint16 tile2data;
			memcpy(&tile2data, constDatData + start3+i*14+8, 2);
			memcpy(&tile3, constDatData + start3+i*14+10, 4);

			tile.param = tile3.param & 0x7F;
			tile.state = tile3.state;
			tile.textureID = tile2data & 0xF;
			tile.textureID2 = (tile2data >> 4) & 0x1;
			tile.depth = (tile2data >> 7) & 0x3;
			tile.typeTrans = (tile2data >> 5) & 0x3;

			tile.blending = (tile3.param >> 7) & 0x1;
			tile.ID = tile3.ID;

			tile.size = 16;
			tile.layerID = 1;
			tile.tileID = i;
			// Used for conversion to PC format 
			if (tile.ID == 0) {
				tile.IDBig = 999;
			} else if (tile.ID < 50) { // all: 50 (42480000h) | 466: 1 (3F800000h) | 138: 16 (41800000h) | 139: 30 (41F00000h) | 253: 56 (42600000h) | <= 55???: 512 (44000000h)
				tile.IDBig = 10000 * tile.ID;
			} else if (tile.ID >= 4095) {
				tile.IDBig = 9998999;
			} else {
				// FIXME: approximation
				tile.IDBig = quint32((float(tile.ID) / 4096.0f) * 10000000.0f);
			}

			tiles.insert(4096 - tile.ID, tile);
		} else {
			qWarning() << "Tile destination overflow 1" << tile1.dstX << tile1.dstY;
		}
		++tileID;
	}

	layerID = 2;

	size = (datDataSize-start4)/10;

	// Possible padding (2) at the end
	if ((datDataSize-start4) % 10 != 0) {
		qWarning() << "BackgroundTilesIOPS::open padding after (4)" << ((datDataSize-start4) % 10);
	}

	quint32 j=0;

	for (i=0; i<size; ++i) {
		memcpy(&tile1, constDatData + start4+i*10, 8);
		if (qAbs(tile1.dstX) < MAX_TILE_DST || qAbs(tile1.dstY) < MAX_TILE_DST) {
			tile.dstX = tile1.dstX;
			tile.dstY = tile1.dstY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = (tile1.palID >> 6) & 0xF;

			memcpy(&tile4, constDatData + start4+i*10+8, 2);

			tile.param = tile4.param & 0x7F;
			tile.state = tile4.state;

			if (texID+1<tiles2.size() && texID+1<nbTilesTex.size() && tileID>=nbTilesTex.at(texID)) {
				++texID;
				tile2 = tiles2.at(texID);
			}

			if (layerID+1<nbTilesLayer.size() && tileID>=nbTilesLayer.at(layerID)) {
				++layerID;
				j = 0;
			}

			tile.blending = (tile4.param >> 7) & 0x1;
			tile.textureID = tile2.page_x;
			tile.textureID2 = tile2.page_y;
			tile.depth = tile2.depth;
			tile.typeTrans = tile2.typeTrans;

			tile.ID = layerID==2 ? 4096 : 0;
			tile.size = 32;
			tile.layerID = layerID;
			tile.tileID = j;
			tile.IDBig = 0;

			tiles.insert(4096 - tile.ID, tile);
		} else {
			qWarning() << "Tile destination overflow 2/3" << tile1.dstX << tile1.dstY;
		}
		++tileID;
		++j;
	}

	return true;
}

bool BackgroundTilesIOPS::writeData(const BackgroundTiles &tiles) const
{
	qint64 beginPos = device()->pos();

	if (!device()->seek(beginPos + (_demo ? 12 : 16))) {
		return false;
	}

	QMultiMap< quint8, QMultiMap< qint16, QMultiMap<qint16, Tile> > > tilesByDst;

	for (const Tile &tile : tiles) {
		QMultiMap< qint16, QMultiMap<qint16, Tile> > tilesByDstY = tilesByDst.value(tile.layerID);
		QMultiMap<qint16, Tile> tilesByDstX = tilesByDstY.value(tile.dstY);
		tilesByDstX.insert(tile.dstX, tile);
		tilesByDstY.replace(tile.dstY, tilesByDstX);
		tilesByDst.replace(tile.layerID, tilesByDstY);
	}

	quint32 pos = 0;
	qint16 currentLayer = -1;
	QMapIterator<quint8, QMultiMap< qint16, QMultiMap<qint16, Tile> > > itLayer(tilesByDst);
	while (itLayer.hasNext()) {
		itLayer.next();
		quint8 layerID = itLayer.key();
		const QMultiMap<qint16, QMultiMap<qint16, Tile> > &tilesByDstY = itLayer.value();
		quint16 line = 0;

		QMapIterator< qint16, QMultiMap<qint16, Tile> > itDstY(tilesByDstY);
		while (itDstY.hasNext()) {
			itDstY.next();
			const QMultiMap<qint16, Tile> &tilesByDstX = itDstY.value();

			for (qint16 dstX : tilesByDstX.keys()) {
				int count = tilesByDstX.count(dstX);

				if (pos > 65535) {
					qWarning() << "BackgroundTilesIOPS::writeData Tile pos overflow" << pos;
					return false;
				}
				if (count > 65535 || count < 0) {
					qWarning() << "BackgroundTilesIOPS::writeData Tile count overflow" << count;
					return false;
				}

				// Separation between layers
				if (currentLayer >= 0 && layerID != currentLayer) {
					quint16 flag = 0x7FFF;
					device()->write((char *)&flag, 2);
					// Infos about the next tile
					device()->write((char *)&dstX, 2);
					device()->write((char *)&pos, 2);
				}

				device()->write((char *)&dstX, 2);
				device()->write((char *)&pos, 2);
				device()->write((char *)&count, 2);

				pos += count;
			}

			// Separation between Y (only when several lines)
			if (tilesByDstY.keys().size() > 1) {
				quint16 flag = 0x7FFE;
				device()->write((char *)&flag, 2);
				device()->write((char *)&line, 2);
				device()->write("\0\0", 2);
				line += 1;
			}
		}

		currentLayer = layerID;
	}

	// End of layer, no more after
	quint16 flag = 0x7FFF;
	device()->write((char *)&flag, 2);
	device()->write((char *)&flag, 2);
	device()->write((char *)&flag, 2);

	QVector<quint32> positions(_demo ? 3 : 4);

	positions[0] = device()->pos();

	QList<Tile> tilesLayers2And3 = tiles.tiles(2, true).values()
	                             + tiles.tiles(3, true).values();
	QList<Tile> tiles2;
	qint16 dstY;
	bool firstTurn = true;

	for (const Tile &tile : tiles.tiles(0, true)) {
		writeTileBase(tile);

		if (firstTurn || tile.dstY != dstY) {
			tiles2.append(tile);
			dstY = tile.dstY;
			firstTurn = false;
		}
	}

	if (!_demo) {
		// Collect tiles2 for layers 2 and 3
		firstTurn = true;
		for (const Tile &tile : qAsConst(tilesLayers2And3)) {
			if (firstTurn || tile.dstY != dstY) {
				tiles2.append(tile);
				dstY = tile.dstY;
				firstTurn = false;
			}
		}
	}

	positions[1] = device()->pos();

	for (const Tile &tile : tiles2) {
		writeTileTex(tile);
	}

	positions[2] = device()->pos();

	for (const Tile &tile : tiles.tiles(1, true)) {
		writeTileBase(tile);
		writeTileTex(tile);
		writeTileID(tile);
		writeTileParam(tile);
	}

	if (!_demo) {
		positions[3] = device()->pos();

		for (const Tile &tile : qAsConst(tilesLayers2And3)) {
			writeTileBase(tile);
			writeTileParam(tile);
		}
	}

	// Write pointers to sections
	if (!device()->seek(beginPos)) {
		return false;
	}

	for (quint32 pos : qAsConst(positions)) {
		device()->write((char *)&pos, 4);
	}

	return false;
}

bool BackgroundTilesIOPS::writeTileBase(const Tile &tile) const
{
	layer1Tile tile1;

	tile1.dstX = tile.dstX;
	tile1.dstY = tile.dstY;
	tile1.srcX = tile.srcX;
	tile1.srcY = tile.srcY;
	tile1.palID = (30 << 10) | (tile.paletteID << 6);

	return device()->write((char *)&tile1, 8) == 8;
}

bool BackgroundTilesIOPS::writeTileTex(const Tile &tile) const
{
	// Note: we can't use bitfields directly
	quint16 tile2data = (tile.textureID & 0xF) |
	                    ((tile.textureID2 & 0x1) << 4) |
	                    ((tile.typeTrans & 0x3) << 5) |
	                    ((tile.depth & 0x3) << 7);

	return device()->write((char *)&tile2data, 2) == 2;
}

bool BackgroundTilesIOPS::writeTileID(const Tile &tile) const
{
	return device()->write((char *)&tile.ID, 2) == 2;
}

bool BackgroundTilesIOPS::writeTileParam(const Tile &tile) const
{
	layer3Tile tile4;

	tile4.param = (tile.blending << 7) | tile.param;
	tile4.state = tile.state;

	return device()->write((char *)&tile4, 4) == 4;
}
