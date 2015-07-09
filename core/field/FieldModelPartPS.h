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
/* Many thanks to Akari for his work. */
#ifndef FIELDMODELPARTPS_H
#define FIELDMODELPARTPS_H

#include <QtCore>
#include "FieldModelPart.h"

#define MODEL_SCALE_PS			4096.0f //31.0f

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
	quint8 num_flags;				// number of data in block 4 (flags).
	quint8 num_control;				// number of data in block 5 (control).
	quint16 offset_poly;			// Relative offset to ?
	quint16 offset_texcoord;		// Relative offset to ?
	quint16 offset_flags;			// Relative offset to texture settings. Indexed by 5th block data (control).
	quint16 offset_control;			// Relative offset to one byte stream for every packet with texture.
	quint16 buffer_size;			// Relative offset to ?
	quint32 offset_vertex;			// Offset to skeleton data section
	quint32 offset_prec;			// Offset to ?
} Part;

typedef struct {
	quint8 red, green, blue, alpha;
} ColorRGBA;

typedef struct {
	quint8 vertexIndex[4];
	ColorRGBA color[4];
	quint8 texCoordId[4];
} TexturedQuad;

typedef struct {
	quint8 vertexIndex[3];
	quint8 padding1;
	ColorRGBA color[3];
	quint8 texCoordId[3];
	quint8 padding2;
} TexturedTriangle;

typedef struct {
	quint8 vertexIndex[4];
	ColorRGBA color;
	quint8 texCoordId[4];
} MonochromeTexturedQuad;

typedef struct {
	quint8 vertexIndex[3];
	quint8 padding1;
	ColorRGBA color;
	quint8 texCoordId[3];
	quint8 padding2;
} MonochromeTexturedTriangle;

typedef struct {
	quint8 vertexIndex[3];
	quint8 padding;
	ColorRGBA color;
} MonochromeTriangle;

typedef struct {
	quint8 vertexIndex[4];
	ColorRGBA color;
} MonochromeQuad;

typedef struct {
	quint8 vertexIndex[3];
	quint8 padding;
	ColorRGBA color[3];
} ColorTriangle;

typedef struct {
	quint8 vertexIndex[4];
	ColorRGBA color[4];
} ColorQuad;

typedef struct TextureInfo_ {
	quint8 type; // 0: eye, 1: mouth, 2: normal
	quint8 bpp;
	quint16 imgX, imgY;
	quint16 palX, palY;

	bool operator==(const struct TextureInfo_ &ti) const
	{
		return ti.bpp == bpp
				&& ti.imgX == imgX
				&& ti.imgY == imgY
				&& ti.type == type;
	}

	bool operator<(const struct TextureInfo_ &) const
	{
		return false;
	}
} TextureInfo;

class FieldModelPartPS : public FieldModelPart
{
public:
	FieldModelPartPS();
	inline qint8 boneID() const {
		return _boneID;
	}
	inline void setBoneID(qint8 boneID) {
		_boneID = boneID;
	}
	inline const QList<TextureInfo> &textures() const {
		return _textures;
	}
	inline void setTextures(const QList<TextureInfo> &textures) {
		_textures = textures;
	}
	void addTexturedPolygon(quint8 control, Poly *polygon);
	void addPolygon(Poly *polygon);
private:
	qint8 _boneID;
	QList<TextureInfo> _textures;
};

#endif // FIELDMODELPARTPS_H
