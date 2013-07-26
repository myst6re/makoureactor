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
