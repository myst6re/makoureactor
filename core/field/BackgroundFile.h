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
#ifndef BACKGROUNDFILE_H
#define BACKGROUNDFILE_H

#include <QtCore>
#include <QPixmap>
#include "Palette.h"

typedef struct {
	qint16 dstX, dstY;
	quint8 srcX, srcY;
	quint8 paletteID;
	quint16 ID;
	quint8 param, state;
	quint8 blending;
	quint8 typeTrans, size;
	quint8 textureID, textureID2;
	quint8 depth;
} Tile;

class BackgroundFile
{
public:
	BackgroundFile();
	virtual ~BackgroundFile();

	virtual QPixmap openBackground(const QByteArray &data1, const QByteArray &data2, const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL)=0;
	virtual bool usedParams(const QByteArray &data, QHash<quint8, quint8> &usedParams, bool *layerExists)=0;
protected:
	QPixmap drawBackground(const QMultiMap<qint16, Tile> &tiles, const QList<Palette *> &palettes, const QByteArray &textureData) const;
	static QRgb blendColor(quint8 type, QRgb color0, QRgb color1);
	virtual quint16 textureWidth(const Tile &tile) const=0;
	virtual quint8 depth(const Tile &tile) const=0;
	virtual quint32 originInData(const Tile &tile) const=0;
	virtual QRgb directColor(quint16 color) const=0;
private:
	static void area(const QMultiMap<qint16, Tile> &tiles, quint16 &minWidth, quint16 &minHeight, int &width, int &height);
};

#endif // BACKGROUNDFILE_H
