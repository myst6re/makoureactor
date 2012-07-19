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
#ifndef FIELDMODELPART_H
#define FIELDMODELPART_H

#include <QtGui>

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
	quint8 unknown;					// 0 - not calculate stage lighting and color. 1 - calculate.
	quint8 bone_index;				// bone to which this part attached to.
	quint8 numVertices;				// Number of vertices
	quint8 numTexCs;				// Number of Texture coord
	quint8 num_quad_color_tex;		// number of textured quads (Gourad Shading)
	quint8 num_tri_color_tex;		// number of textured triangles (Gourad Shading)
	quint8 num_quad_mono_tex;		// number of textured quads (Flat Shading)
	quint8 num_tri_mono_tex;		// number of textured triangles (Flat Shading)
	quint8 num_tri_mono;			// number of monochrome triangles
	quint8 num_quad_mono;			// number of monochrome quads
	quint8 num_tri_color;			// number of gradated triangles
	quint8 num_quad_color;			// number of gradated quads
	quint16 num_flags;				// number of data in block 4 (flags).
	quint16 offset_poly;			// Relative offset to ?
	quint16 offset_texcoord;		// Relative offset to ?
	quint16 offset_flags;			// Relative offset to texture settings. Indexed by 5th block data (control).
	quint16 offset_control;			// Relative offset to one byte stream for every packet with texture.
	quint16 buffer_size;			// Relative offset to ?
	quint32 offset_vertex;			// Offset to skeleton data section
	quint32 offset_prec;			// Offset to ?
} Part;

typedef struct {
	float x, y, z;
} VertexPC;

typedef struct {
	qint16 x, z, y;
} VertexPS;

typedef struct {
	VertexPS v;
	quint16 unknown;
} Vertex;

typedef struct {
	float x, y;
} Coord;

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

class FieldModelPart
{
public:
    FieldModelPart();
	bool open_p(QFile *);
	QList<VertexPC> vertices, normals;
	QList<Coord> texCs;
	QList<Color> vertexColors;
	QList<Polygon_p> polys;
	QList<Group> groups;
};

#endif // FIELDMODELPART_H
