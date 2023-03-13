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
#include "FieldModelPart.h"

#define MODEL_SCALE_PC 132.0f // 1.0f

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

struct Edge {
	quint16 vertex1, vertex2;
};

struct Hundred {
	quint32 zz1;
	quint32 zz2;
	quint32 zz3;
	quint32 zz4;
	quint32 texture_id;
	quint32 texture_set;
	quint32 zz7;
	quint32 zz8;
	quint32 zz9;
	quint32 shademode;
	quint32 lightstate_ambient;
	quint32 zz12;
	quint32 lightstate_material_pointer;
	quint32 srcblend;
	quint32 destblend;
	quint32 zz16;
	quint32 alpharef;
	quint32 blend_mode;
	quint32 zsort;
	quint32 zz20;
	quint32 zz21;
	quint32 zz22;
	quint32 zz23;
	quint32 vertex_alpha;
	quint32 zz25;
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

struct BoundingBox {
	quint32 field_0;
	float max_x;
	float max_y;
	float max_z;
	float min_x;
	float min_y;
	float min_z;
};

class PFile : public IO
{
public:
	explicit PFile(QIODevice *io);
	virtual ~PFile() override;

	bool read(FieldModelPart *part, const QList<int> &texIds) const;
	bool write(const FieldModelPart *part, const QList<int> &texIds) const;
};
