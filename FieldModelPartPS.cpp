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
#include "FieldModelPartPS.h"

FieldModelPartPS::FieldModelPartPS() :
	_boneID(-1)
{
}

bool FieldModelPartPS::open(const char *data, quint32 offset, quint32 size)
{
	quint32 i;

	QList<PolyVertex> vertices;
	QList<TexCoord> texCoords;
	Part partHeader;
	PolyVertex vertex;
	TexCoord texCoord;

	if(offset + sizeof(Part) >= size) {
		qWarning() << "Invalid part size" << size;
		return false;
	}

	// Header
	memcpy(&partHeader, &data[offset], sizeof(Part));

	_boneID = partHeader.bone_index;

	qDebug() << "BoneID" << _boneID << "unknown" << partHeader.unknown;

	// Vertices
	quint32 offsetToVertex = partHeader.offset_vertex;

	qDebug() << "Offset to vertex" << offsetToVertex;

	if(offsetToVertex + partHeader.numVertices*8 >= size) {
		qWarning() << "Invalid part size2" << offsetToVertex << partHeader.numVertices << size;
		return false;
	}

	quint32 unknown;
	memcpy(&unknown, &data[offsetToVertex], 4);

	qDebug() << "unknown data vertex" << unknown << QString::number(unknown, 16);

	for(i=0 ; i<partHeader.numVertices ; ++i) {
		qint16 v;
		memcpy(&v, &data[offsetToVertex + i*8 + 4], 2);
		vertex.x = v / 31.0f;
		memcpy(&v, &data[offsetToVertex + i*8 + 6], 2);
		vertex.y = v / 31.0f;
		memcpy(&v, &data[offsetToVertex + i*8 + 8], 2);
		vertex.z = v / 31.0f;

		vertices.append(vertex);
	}

	qDebug() << "Offset to vertex (after all)" << (offsetToVertex + 4 + partHeader.numVertices*8);

	// Texture coordinates
	quint32 offsetToTexCoords = offsetToVertex + partHeader.offset_texcoord;

	qDebug() << "Offset to tex Coords" << offsetToTexCoords;

	if(offsetToTexCoords + partHeader.numTexCs*2 >= size) {
		qWarning() << "Invalid part size3" << size;
		return false;
	}

	for(i=0 ; i<partHeader.numTexCs ; ++i) {
		texCoord.x = (quint8)data[offsetToTexCoords + i * 2] / 512.0f;
		texCoord.y = (quint8)data[offsetToTexCoords + i * 2 + 1] / 256.0f;

		texCoords.append(texCoord);
	}

	qDebug() << "Offset to tex Coords (after all)" << (offsetToTexCoords + partHeader.numTexCs*2);

	// Polygons
	quint32 offsetToPoly = offsetToVertex + partHeader.offset_poly;
	FieldModelGroup *group = new FieldModelGroup();

	qDebug() << "Offset to flags" << (offsetToVertex+partHeader.offset_flags);
	qDebug() << "Offset to flags (after all)" << (offsetToVertex+partHeader.offset_flags+partHeader.num_flags*4) << "num" << partHeader.num_flags;
	qDebug() << "Offset to control" << (offsetToVertex+partHeader.offset_control);
	qDebug() << "Offset to unknown" << partHeader.offset_unknown;
	qDebug() << "Offset to prec" << partHeader.offset_prec;
	qDebug() << "color tex quad count" << partHeader.num_quad_color_tex << sizeof(TexturedQuad);
	qDebug() << "mono tex quad count" << partHeader.num_quad_mono_tex << sizeof(TexturedTriangle);
	qDebug() << "color quad count" << partHeader.num_quad_color << sizeof(MonochromeTexturedQuad);
	qDebug() << "mono quad count" << partHeader.num_quad_mono << sizeof(MonochromeTexturedTriangle);
	qDebug() << "mono tex triangle count" << partHeader.num_tri_mono_tex << sizeof(MonochromeTriangle);
	qDebug() << "color tex triangle count" << partHeader.num_tri_color_tex << sizeof(MonochromeQuad);
	qDebug() << "mono triangle count" << partHeader.num_tri_mono << sizeof(ColorTriangle);
	qDebug() << "color triangle count" << partHeader.num_tri_color << sizeof(ColorQuad);

	qDebug() << "Offset to poly (tex quads)" << offsetToPoly;

	bool notAdd;

	//--> Textured Quads

	for(i=0 ; i<partHeader.num_quad_color_tex ; ++i) {
		TexturedQuad texturedQuad;
		memcpy(&texturedQuad, &data[offsetToPoly], sizeof(TexturedQuad));

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for(int j=0 ; j<4 ; ++j) {
			quint8 vertexIndex = texturedQuad.vertexIndex[j];
			quint8 texCoordIndex = texturedQuad.texCoordId[j];
			if(vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				Color2 color = texturedQuad.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qDebug() << "error index col tex quad" << i << j;
				break;
			}
		}

		if(!notAdd)
			group->addPolygon(new QuadPoly(polyVertices, polyColors, polyTexCoords));

		offsetToPoly += sizeof(TexturedQuad);
	}

	qDebug() << "Offset to poly (tex triangles)" << offsetToPoly;

	//--> Textured Triangles

	for(i=0 ; i<partHeader.num_tri_color_tex ; ++i) {
		TexturedTriangle texturedTriangle;
		memcpy(&texturedTriangle, &data[offsetToPoly], sizeof(TexturedTriangle));

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for(int j=0 ; j<3 ; ++j) {
			quint8 vertexIndex = texturedTriangle.vertexIndex[j];
			quint8 texCoordIndex = texturedTriangle.texCoordId[j];
			if(vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				Color2 color = texturedTriangle.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qDebug() << "error index col tex tri" << i << j;
				break;
			}
		}

		if(!notAdd)
			group->addPolygon(new TrianglePoly(polyVertices, polyColors, polyTexCoords));

		offsetToPoly += sizeof(TexturedTriangle);
	}

	qDebug() << "Offset to poly (mono tex quads)" << offsetToPoly;

	//--> Monochrome Textured Quads

	for(i=0 ; i<partHeader.num_quad_mono_tex ; ++i) {
		MonochromeTexturedQuad monochromeTexturedQuad;
		memcpy(&monochromeTexturedQuad, &data[offsetToPoly], sizeof(MonochromeTexturedQuad));

		QList<PolyVertex> polyVertices;
		Color2 color = monochromeTexturedQuad.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for(int j=0 ; j<4 ; ++j) {
			quint8 vertexIndex = monochromeTexturedQuad.vertexIndex[j];
			quint8 texCoordIndex = monochromeTexturedQuad.texCoordId[j];
			if(vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qDebug() << "error index mono tex quad" << i << j;
				break;
			}
		}

		if(!notAdd)
			group->addPolygon(new QuadPoly(polyVertices, polyColor, polyTexCoords));

		offsetToPoly += sizeof(MonochromeTexturedQuad);
	}

	qDebug() << "Offset to poly (mono tex triangles)" << offsetToPoly;

	//--> Monochrome Textured Triangles

	for(i=0 ; i<partHeader.num_tri_mono_tex ; ++i) {
		MonochromeTexturedTriangle monochromeTexturedTriangle;
		memcpy(&monochromeTexturedTriangle, &data[offsetToPoly], sizeof(MonochromeTexturedTriangle));

		QList<PolyVertex> polyVertices;
		Color2 color = monochromeTexturedTriangle.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for(int j=0 ; j<3 ; ++j) {
			quint8 vertexIndex = monochromeTexturedTriangle.vertexIndex[j];
			quint8 texCoordIndex = monochromeTexturedTriangle.texCoordId[j];
			if(vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qDebug() << "error index mono tex tri" << i << j;
				break;
			}
		}

		if(!notAdd)
			group->addPolygon(new TrianglePoly(polyVertices, polyColor, polyTexCoords));

		offsetToPoly += sizeof(MonochromeTexturedTriangle);
	}

	qDebug() << "Offset to poly (mono triangles)" << offsetToPoly;

	//--> Monochrome Triangles

	for(i=0 ; i<partHeader.num_tri_mono ; ++i) {
		MonochromeTriangle monochromeTriangle;
		memcpy(&monochromeTriangle, &data[offsetToPoly], sizeof(MonochromeTriangle));

		QList<PolyVertex> polyVertices;
		Color2 color = monochromeTriangle.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		notAdd = false;

		for(int j=0 ; j<3 ; ++j) {
			quint8 vertexIndex = monochromeTriangle.vertexIndex[j];
			if(vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
			} else {
				notAdd = true;
				qDebug() << "error index mono tri" << i << j;
				break;
			}
		}

		if(!notAdd)
			group->addPolygon(new TrianglePoly(polyVertices, polyColor));

		offsetToPoly += sizeof(MonochromeTriangle);
	}

	qDebug() << "Offset to poly (mono quads)" << offsetToPoly;

	//--> Monochrome Quads

	for(i=0 ; i<partHeader.num_quad_mono ; ++i) {
		MonochromeQuad monochromeQuad;
		memcpy(&monochromeQuad, &data[offsetToPoly], sizeof(MonochromeQuad));

		QList<PolyVertex> polyVertices;
		Color2 color = monochromeQuad.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		notAdd = false;

		for(int j=0 ; j<4 ; ++j) {
			quint8 vertexIndex = monochromeQuad.vertexIndex[j];
			if(vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
			} else {
				notAdd = true;
				qDebug() << "error index mono quad" << i << j;
				break;
			}
		}

		if(!notAdd)
			group->addPolygon(new QuadPoly(polyVertices, polyColor));

		offsetToPoly += sizeof(MonochromeQuad);
	}

	qDebug() << "Offset to poly (color triangles)" << offsetToPoly;

	//--> Color Triangles

	for(i=0 ; i<partHeader.num_tri_color ; ++i) {
		ColorTriangle colorTriangle;
		memcpy(&colorTriangle, &data[offsetToPoly], sizeof(ColorTriangle));

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		notAdd = false;

		for(int j=0 ; j<3 ; ++j) {
			quint8 vertexIndex = colorTriangle.vertexIndex[j];
			if(vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				Color2 color = colorTriangle.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
			} else {
				notAdd = true;
				qDebug() << "error index col tri" << i << j;
				break;
			}
		}

		if(!notAdd)
			group->addPolygon(new TrianglePoly(polyVertices, polyColors));

		offsetToPoly += sizeof(ColorTriangle);
	}

	qDebug() << "Offset to poly (color quads)" << offsetToPoly;

	//--> Color Quads

	for(i=0 ; i<partHeader.num_quad_color ; ++i) {
		ColorQuad colorQuad;
		memcpy(&colorQuad, &data[offsetToPoly], sizeof(ColorQuad));

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		notAdd = false;

		for(int j=0 ; j<4 ; ++j) {
			quint8 vertexIndex = colorQuad.vertexIndex[j];
			if(vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				Color2 color = colorQuad.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
			} else {
				notAdd = true;
				qDebug() << "error index col quad" << i << j;
				break;
			}
		}

		if(!notAdd)
			group->addPolygon(new QuadPoly(polyVertices, polyColors));

		offsetToPoly += sizeof(ColorQuad);
	}

	_groups.append(group);

	qDebug() << "Offset to poly (after all)" << offsetToPoly;

	return true;
}

qint8 FieldModelPartPS::boneID() const
{
	return _boneID;
}
