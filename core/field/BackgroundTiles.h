#ifndef BACKGROUNDTILES_H
#define BACKGROUNDTILES_H

#include <QtCore>

typedef struct {
	qint16 dstX, dstY;
	quint8 srcX, srcY;
	quint8 paletteID;
	quint16 ID;
	quint8 param, state;
	quint8 blending;
	quint8 typeTrans, size;
	quint8 textureID, textureID2;
	quint8 depth, layerID;
} Tile;

class BackgroundTiles
{
public:
	BackgroundTiles();

	inline const QMultiMap<qint16, Tile> &tiles() const {
		return _tiles;
	}
	QMultiMap<qint16, Tile> tiles(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers) const;
	QMultiMap<qint16, Tile> tiles(quint8 layerID) const;
	inline void setTiles(const QMultiMap<qint16, Tile> &tiles) {
		_tiles = tiles;
	}
	QHash<quint8, quint8> usedParams(bool *layerExists) const;
	void area(quint16 &minWidth, quint16 &minHeight,
			  int &width, int &height) const;
	inline void clear() {
		_tiles.clear();
	}
	inline bool isEmpty() const {
		return _tiles.isEmpty();
	}

private:
	QMultiMap<qint16, Tile> _tiles;
};

#endif // BACKGROUNDTILES_H
