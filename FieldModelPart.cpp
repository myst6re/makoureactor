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
#include "FieldModelPart.h"

Poly::Poly(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords)
{
	setVertices(vertices, colors, texCoords);
}

Poly::Poly(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords)
{
	setVertices(vertices, color, texCoords);
}

void Poly::setVertices(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords)
{
	_vertices = vertices;
	_colors = colors;
	_texCoords = texCoords;
}

void Poly::setVertices(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords)
{
	_vertices = vertices;
	_colors.clear();
	_colors.append(color);
	_texCoords = texCoords;
}

const PolyVertex &Poly::vertex(quint8 id) const
{
	return _vertices.at(id);
}

QRgb Poly::color(quint8 id) const
{
	return _colors.value(id, _colors.first());
}

const TexCoord &Poly::texCoord(quint8 id) const
{
	return _texCoords.at(id);
}

bool Poly::hasTexture() const
{
	return !_texCoords.isEmpty();
}

QuadPoly::QuadPoly(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords) :
	Poly(vertices, colors, texCoords)
{
}

QuadPoly::QuadPoly(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords) :
	Poly(vertices, color, texCoords)
{
}

TrianglePoly::TrianglePoly(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords) :
	Poly(vertices, colors, texCoords)
{
}

TrianglePoly::TrianglePoly(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords) :
	Poly(vertices, color, texCoords)
{
}

FieldModelGroup::FieldModelGroup() :
	_textureNumber(-1)
{
}

const QList<QuadPoly> &FieldModelGroup::quads() const
{
	return _quads;
}

const QList<TrianglePoly> &FieldModelGroup::triangles() const
{
	return _triangles;
}

int FieldModelGroup::textureNumber() const
{
	return _textureNumber;
}

void FieldModelGroup::setTextureNumber(int texNumber)
{
	_textureNumber = texNumber;
}


void FieldModelGroup::addQuad(const QuadPoly &quad)
{
	_quads.append(quad);
}

void FieldModelGroup::addTriangle(const TrianglePoly &triangle)
{
	_triangles.append(triangle);
}

FieldModelPart::FieldModelPart()
{
}

const QList<FieldModelGroup> &FieldModelPart::groups() const
{
	return _groups;
}
