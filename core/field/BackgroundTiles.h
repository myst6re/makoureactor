#ifndef BACKGROUNDTILES_H
#define BACKGROUNDTILES_H

#include <QtCore>

typedef struct {
	qint16 dstX, dstY;
	quint8 srcX, srcY;
	quint8 paletteID;
	quint16 ID;
	quint8 param, state;
	bool blending;
	quint8 typeTrans, size;
	quint8 textureID, textureID2;
	quint8 depth;
	quint8 layerID;
	quint16 tileID;
} Tile;

class BackgroundTiles : public QMultiMap<qint16, Tile>
{
public:
	BackgroundTiles();
	BackgroundTiles(const QMultiMap<qint16, Tile> &tiles);

	BackgroundTiles filter(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers) const;
	BackgroundTiles tiles(quint8 layerID, bool orderedForSaving) const;
	QMap<qint32, Tile> sortedTiles() const;
	QHash<quint8, quint8> usedParams(bool *layerExists) const;
	void area(quint16 &minWidth, quint16 &minHeight,
			  int &width, int &height) const;
	Tile search(quint8 textureID1, quint8 textureID2, quint8 srcX, quint8 srcY) const;
};

#endif // BACKGROUNDTILES_H
