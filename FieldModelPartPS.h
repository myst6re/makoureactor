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
#ifndef FIELDMODELPARTPS_H
#define FIELDMODELPARTPS_H

#include <QtGui>
#include "FieldModelPart.h"

typedef struct {
	quint8 unknown;					// 0 - not calculate stage lighting and color. 1 - calculate.
	qint8 bone_index;				// bone to which this part attached to.
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
	quint16 offset_vertex;			// Offset to skeleton data section
	quint16 offset_unknown;
	quint32 offset_prec;			// Offset to ?
} Part;

typedef struct {
	quint8 red, green, blue, alpha;
} Color2;

typedef struct {
	quint8 vertexIndex[4];
	Color2 color[4];
	quint8 texCoordId[4];
} TexturedQuad;

typedef struct {
	quint8 vertexIndex[3];
	quint8 padding1;
	Color2 color[3];
	quint8 texCoordId[3];
	quint8 padding2;
} TexturedTriangle;

typedef struct {
	quint8 vertexIndex[4];
	Color2 color;
	quint8 texCoordId[4];
} MonochromeTexturedQuad;

typedef struct {
	quint8 vertexIndex[3];
	quint8 padding1;
	Color2 color;
	quint8 texCoordId[3];
	quint8 padding2;
} MonochromeTexturedTriangle;

typedef struct {
	quint8 vertexIndex[3];
	quint8 padding;
	Color2 color;
} MonochromeTriangle;

typedef struct {
	quint8 vertexIndex[4];
	Color2 color;
} MonochromeQuad;

typedef struct {
	quint8 vertexIndex[3];
	quint8 padding;
	Color2 color[3];
} ColorTriangle;

typedef struct {
	quint8 vertexIndex[4];
	Color2 color[4];
} ColorQuad;

typedef struct {
	quint8 type, bpp;
	quint8 imgX, imgY;
	quint8 palX, palY;
} TextureInfo;

class FieldModelPartPS : public FieldModelPart
{
public:
	FieldModelPartPS();
	bool open(const char *data, quint32 offset, quint32 size);
	qint8 boneID() const;
private:
	void addTexture(const char *data, quint32 offsetControl, quint32 offsetFlag, quint32 size);
	qint8 _boneID;
	QList<TextureInfo> _textures;
};

#endif // FIELDMODELPARTPS_H
