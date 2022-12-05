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
#include <IO>
#include "BackgroundTiles.h"

class InfFile;
class Palette;

#define MAX_TILE_DST 1024

//Sizeof : 36
struct TilePC {
	qint16 dstX, dstY;
	quint32 unused1;
	quint8 srcX, unused2;
	quint8 srcY, unused3;
	quint8 srcX2, unused4;
	quint8 srcY2, unused5;
	quint16 width, height; // Normaly unused // 16
	quint8 paletteID, unused6;
	quint16 ID;
	quint8 param;
	quint8 state;
	quint8 blending; // boolean
	quint8 unknown7;
	quint8 typeTrans, unused8;
	quint8 textureID, unused9;
	quint8 textureID2, unused10; // if blending > 0, textureID2 is used instead of textureID except for layer 0
	quint8 depth, unused11; // Normaly unused
	quint32 IDBig; // 32
};

//Sizeof : 16
struct HeaderLayer2TilePC {
	quint16 firstPalettedTextureId; // Unused
	quint16 nextPalettedTextureId; // Unused
	quint16 firstPalettedBlendingTypeTransTextureId;
	quint16 nextPalettedBlendingTypeTransTextureId;
	quint16 firstDirectColorTextureId; // Unused
	quint16 nextDirectColorTextureId; // Unused
	quint16 firstDirectColorBlendingTypeTransTextureId; // Unused
	quint16 nextDirectColorBlendingTypeTransTextureId; // Unused
};

//Sizeof : 10
struct HeaderLayer4TilePC {
	quint16 firstPalettedBlendingTextureId;
	quint16 nextPalettedBlendingTextureId;
	quint16 firstDirectColorBlendingTextureId; // Unused
	quint16 nextDirectColorBlendingTextureId; // Unused
};

//Sizeof : 8
struct layer1Tile {
	qint16 dstX, dstY;
	quint8 srcX, srcY;
	qint16 palID; // 6 bits = Always 30 | 4 bits = PaletteID | 6 bits = Always 0
};

//Sizeof : 2
struct layer2Tile {
	unsigned page_x:4;
	unsigned page_y:1;
	unsigned typeTrans:2;
	unsigned depth:2;
	unsigned ZZZ:7; // Always 0
};

//Sizeof : 2
struct layer3Tile {
	quint8 param; // 1 bit = blending | 7 bits = param
	quint8 state;
};

//Sizeof : 4
struct paramTile {
	quint16 ID;
	quint8 param; // 1 bit = blending | 7 bits = param
	quint8 state;
};

class BackgroundTilesIO : public IO
{
public:
	explicit BackgroundTilesIO(QIODevice *device);
	virtual ~BackgroundTilesIO() override;

	bool read(BackgroundTiles &tiles) const;
	bool write(const BackgroundTiles &tiles) const;
protected:
	virtual bool readData(BackgroundTiles &tiles) const=0;
	virtual bool writeData(const BackgroundTiles &tiles) const=0;
};

class BackgroundTilesIOPC : public BackgroundTilesIO
{
public:
	explicit BackgroundTilesIOPC(QIODevice *device, InfFile *inf, const QList<Palette *> &palettes);
protected:
	bool readData(BackgroundTiles &tiles) const override;
	bool writeData(const BackgroundTiles &tiles) const override;
private:
	void generateHeaders(const BackgroundTiles &tiles, HeaderLayer2TilePC &headerLayer2, HeaderLayer4TilePC &headerLayer4) const;
	bool writeTile(const Tile &tile) const;
	static Tile tilePC2Tile(const TilePC &tile, quint8 layerID, quint16 tileID);
	static TilePC tile2TilePC(const Tile &tile);
	InfFile *_inf;
	const QList<Palette *> &_palettes;
};

class BackgroundTilesIOPS : public BackgroundTilesIO
{
public:
	explicit BackgroundTilesIOPS(QIODevice *device, bool demo = false);
protected:
	bool readData(BackgroundTiles &tiles) const override;
	bool writeData(const BackgroundTiles &tiles) const override;
private:
	bool writeTileBase(const Tile &tile) const;
	bool writeTileTex(const Tile &tile) const;
	bool writeTileID(const Tile &tile) const;
	bool writeTileParam(const Tile &tile) const;
	bool _demo;
};
