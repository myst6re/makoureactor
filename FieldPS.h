/****************************************************************************
 ** Makou Reactor Final Fantasy VII FieldPS Script Editor
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
#ifndef DEF_FIELDPS
#define DEF_FIELDPS

#include <QtGui>
#include "Field.h"
#include "IsoArchive.h"
#include "FieldModelLoaderPS.h"
#include "FieldModelFilePS.h"

//Sizeof : 8
typedef struct {
	qint16 cibleX, cibleY;
	quint8 srcX, srcY;
	unsigned ZZ1:6;
	unsigned palID:4;
	unsigned ZZ2:6;
} layer1Tile;

//Sizeof : 2
typedef struct {
	unsigned page_x:4;
	unsigned page_y:1;
	unsigned typeTrans:2;//transparence n°3
	unsigned deph:2;
	unsigned ZZZ:7;
} layer2Tile;

//Sizeof : 2
typedef struct {
	unsigned param:7;
	unsigned blending:1;//transparence n°1
	quint8 state;
} layer3Tile;

//Sizeof : 4
typedef struct {
	quint16 group;//id
	unsigned param:7;
	unsigned blending:1;//transparence n°1
	quint8 state;
} paramTile;

//Sizeof : 12
typedef struct {
	quint32 size;// = 12 + w*2*h
	quint16 x, y;
	quint16 w, h;
} MIM;

class FieldPS : public Field
{
public:
	FieldPS(const QString &name, FieldArchive *fieldArchive);
	FieldPS(const Field &field);
	virtual ~FieldPS();

	bool open(bool dontOptimize=false);
	QPixmap openBackground();
	QPixmap openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL);

	bool getUsedParams(QHash<quint8, quint8> &usedParams, bool *layerExists);

	QByteArray save(const QByteArray &fileData, bool compress);

	FieldModelLoaderPS *getFieldModelLoader(bool open=true);
	FieldModelFilePS *getFieldModel(int modelID, int animationID=0, bool animate=true);
protected:
	QByteArray sectionData(FieldPart part);
	QByteArray sectionData(int idPart);
private:
	QPixmap openBackground(const QByteArray &mimDataDec, const QByteArray &datDataDec);
	QPixmap openBackground(const QByteArray &mimDataDec, const QByteArray &datDataDec, const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL) const;
	bool getUsedParams(const QByteArray &datData, QHash<quint8, quint8> &usedParams, bool *layerExists) const;

	quint32 sectionPositions[7];
};

#endif // DEF_FIELDPS
