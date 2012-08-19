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
#ifndef FIELDMODELPARTPC_H
#define FIELDMODELPARTPC_H

#include <QtGui>
#include "FieldModelPart.h"

typedef struct {
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
} p_header;

typedef struct {
	float x, y, z;
} VertexPC;

typedef struct {
	Vertex_s v;
	quint16 unknown;
} Vertex;

typedef struct {
	quint8 blue, green, red, alpha;
} Color;

typedef struct {
	quint16 zero;
	quint16 VertexIndex[3], NormalIndex[3], EdgeIndex[3];
	quint16 u[2];
} Polygon_p;

typedef struct {
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
} Group;

class FieldModelPartPC : public FieldModelPart
{
public:
	FieldModelPartPC();
	bool open(QFile *);
};

#endif // FIELDMODELPARTPC_H
