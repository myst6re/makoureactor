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
#ifndef FIELDMODELPARTIOPC_H
#define FIELDMODELPARTIOPC_H

#include <QtCore>
#include "FieldModelPartIO.h"

#define MODEL_SCALE_PC				132.0f // 1.0f

struct PHeader {
	quint32 version;
	quint32 off04;
	quint32 vertexType;
	quint32 numVertices;
	quint32 numNormals;
	quint32 numUnknown1;
	quint32 numTexCs;
	quint32 numVertexColors;
	quint32 numEdges;
	quint32 numPolys;
	quint32 numUnknown2;
	quint32 numUnknown3;
	quint32 numHundreds;
	quint32 numGroups;
	quint32 numBoundingBoxes;
	quint32 normIndexTableFlag;
	quint32 runtime_data[16];
};

struct ColorBGRA {
	quint8 blue, green, red, alpha;
};

struct PolygonP {
	quint16 zero;
	quint16 VertexIndex[3], NormalIndex[3], EdgeIndex[3];
	quint16 u[2];
};

struct Group {
	quint32 primitiveType;
	quint32 polygonStartIndex;
	quint32 numPolygons;
	quint32 verticesStartIndex;
	quint32 numVertices;
	quint32 edgeStartIndex;
	quint32 numEdges;
	quint32 u1;
	quint32 u2;
	quint32 u3;
	quint32 u4;
	quint32 texCoordStartIndex;
	quint32 areTexturesUsed;
	quint32 textureNumber;
};

class Rsd {
	friend class FieldModelPartIOPC;
	QString pName;
	QList<int> texIds;
public:
	inline const QString &pFile() const {
		return pName;
	}
	inline const QList<int> &textureIds() const {
		return texIds;
	}
};

class FieldModelPartIOPC : public FieldModelPartIO
{
public:
	explicit FieldModelPartIOPC(QIODevice *ioRsd);
	bool readRsd(Rsd &rsd, QStringList &textureNames) const;
	bool read(FieldModelPart *part, const QList<int> &texIds) const;
	bool write(const FieldModelPart *part) const;
private:
	bool read(FieldModelPart *part) const {
		Q_UNUSED(part)
		return false;
	}
	inline QIODevice *deviceRsd() const {
		return _deviceRsd;
	}
	bool canReadRsd() const;

	QIODevice *_deviceRsd;
};

#endif // FIELDMODELPARTIOPC_H
