/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef BACKGROUNDFILEPC_H
#define BACKGROUNDFILEPC_H

#include "BackgroundFile.h"

//Sizeof : 36
typedef struct {
	qint16 dstX, dstY;
	quint32 unused1;
	quint8 srcX, unused2;
	quint8 srcY, unused3;
	quint8 srcX2, unused4;
	quint8 srcY2, unused5;
	quint32 unused6;
	quint8 paletteID, unused7;
	quint16 ID;
	quint8 param;
	quint8 state;
	quint8 blending;
	quint8 unused8;
	quint8 typeTrans, size;//Normaly unused
	quint8 textureID, unused10;
	quint8 textureID2, unused11;
	quint8 depth, unused12;
} TilePC;

class BackgroundFilePC : public BackgroundFile
{
public:
	BackgroundFilePC();

	QPixmap openBackground(const QByteArray &data, const QByteArray &palData, const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL);
	bool usedParams(const QByteArray &data, QHash<quint8, quint8> &usedParams, bool *layerExists);
protected:
	quint16 textureWidth(const Tile &tile) const;
	quint8 depth(const Tile &tile) const;
	quint32 originInData(const Tile &tile) const;
	QRgb directColor(quint16 color) const;
private:
	static QList<Palette *> openPalettes(const QByteArray &data, const QByteArray &palData);
	static Tile tilePC2Tile(const TilePC &tile);
	static QHash<quint8, quint32> posTextures;
	static QByteArray data;
};

#endif // BACKGROUNDFILEPC_H
