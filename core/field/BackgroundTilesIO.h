#ifndef BACKGROUNDTILESIO_H
#define BACKGROUNDTILESIO_H

#include <QtCore>
#include "BackgroundTiles.h"

#define MAX_TILE_DST	1024

//Sizeof : 36
typedef struct {
	qint16 dstX, dstY;
	quint32 unused1;
	quint8 srcX, unused2;
	quint8 srcY, unused3;
	quint8 srcX2, unused4;
	quint8 srcY2, unused5;
	quint16 width, height;//Normaly unused
	quint8 paletteID, unused6;
	quint16 ID;
	quint8 param;
	quint8 state;
	quint8 blending;
	quint8 unused7;
	quint8 typeTrans, unused8;
	quint8 textureID, unused9;
	quint8 textureID2, unused10;
	quint8 depth, unused11;//Normaly unused
} TilePC;

//Sizeof : 8
typedef struct {
	qint16 dstX, dstY;
	quint8 srcX, srcY;
	unsigned ZZ1:6; // Always 0
	unsigned palID:4;
	unsigned ZZ2:6; // Always 30
} layer1Tile;

//Sizeof : 2
typedef struct {
	unsigned page_x:4;
	unsigned page_y:1;
	unsigned typeTrans:2;
	unsigned depth:2;
	unsigned ZZZ:7; // Always 0
} layer2Tile;

//Sizeof : 2
typedef struct {
	unsigned param:7;
	unsigned blending:1;
	quint8 state;
} layer3Tile;

//Sizeof : 4
typedef struct {
	quint16 ID;
	unsigned param:7;
	unsigned blending:1;
	quint8 state;
} paramTile;

class BackgroundTilesIO
{
public:
	explicit BackgroundTilesIO(QIODevice *device);
	virtual ~BackgroundTilesIO();

	void setDevice(QIODevice *device);
	QIODevice *device() const;

	bool canRead() const;
	bool canWrite() const;

	bool read(BackgroundTiles &tiles) const;
	bool write(const BackgroundTiles &tiles) const;
protected:
	virtual bool readData(BackgroundTiles &tiles) const=0;
	virtual bool writeData(const BackgroundTiles &tiles) const=0;
private:
	QIODevice *_device;
};

class BackgroundTilesIOPC : public BackgroundTilesIO
{
public:
	explicit BackgroundTilesIOPC(QIODevice *device);
protected:
	bool readData(BackgroundTiles &tiles) const;
	bool writeData(const BackgroundTiles &tiles) const;
private:
	static Tile tilePC2Tile(const TilePC &tile, quint8 layerID, quint16 tileID);
	static TilePC tile2TilePC(const Tile &tile);
};

class BackgroundTilesIOPS : public BackgroundTilesIO
{
public:
	explicit BackgroundTilesIOPS(QIODevice *device);
protected:
	bool readData(BackgroundTiles &tiles) const;
	bool writeData(const BackgroundTiles &tiles) const;
};

#endif // BACKGROUNDTILESIO_H
