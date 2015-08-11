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

Poly::Poly(int count, const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords) :
	_count(count)
{
	setVertices(vertices, colors, texCoords);
}

Poly::Poly(int count, const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords) :
	_count(count)
{
	setVertices(vertices, color, texCoords);
}

Poly::~Poly()
{
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

const QRgb &Poly::color() const
{
	return _colors.first();
}

QRgb Poly::color(quint8 id) const
{
	return _colors.value(id, _colors.first());
}

const TexCoord &Poly::texCoord(quint8 id) const
{
	return _texCoords.at(id);
}

void Poly::setTexCoord(quint8 id, const TexCoord &texCoord)
{
	_texCoords.replace(id, texCoord);
}

bool Poly::isMonochrome() const
{
	return _colors.size() == 1;
}

bool Poly::hasTexture() const
{
	return !_texCoords.isEmpty();
}

QuadPoly::QuadPoly(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords) :
	Poly(4, vertices, colors, texCoords)
{
	// swapping the two last vertices for right OpenGL quad order

	_vertices.swap(2, 3);

	if(colors.size() == 4) {
		_colors.swap(2, 3);
	}

	if(!texCoords.isEmpty()) {
		_texCoords.swap(2, 3);
	}
}

QuadPoly::QuadPoly(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords) :
	Poly(4, vertices, color, texCoords)
{
	// swapping the two last vertices for right OpenGL quad order

	_vertices.swap(2, 3);

	if(!texCoords.isEmpty()) {
		_texCoords.swap(2, 3);
	}
}

TrianglePoly::TrianglePoly(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords) :
	Poly(3, vertices, colors, texCoords)
{
}

TrianglePoly::TrianglePoly(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords) :
	Poly(3, vertices, color, texCoords)
{
}

FieldModelGroup::FieldModelGroup() :
	_textureRef(0)
{
}

FieldModelGroup::FieldModelGroup(FieldModelTextureRef *texRef) :
	_textureRef(texRef)
{
}

FieldModelGroup::~FieldModelGroup()
{
	qDeleteAll(_polys);
	if (_textureRef) {
		delete _textureRef;
	}
}

void FieldModelGroup::setTextureRef(FieldModelTextureRef *texRef)
{
	if (_textureRef) {
		delete _textureRef;
	}
	_textureRef = texRef;
}

void FieldModelGroup::removeSpriting(float texWidth, float texHeight)
{
	float minX = -1,
			minY = -1;

	foreach (Poly *poly, polygons()) {
		if (poly->hasTexture() && poly->count() > 0) {
			for (quint16 i = 0 ; i < (quint8)poly->count(); ++i) {
				const TexCoord &texCoord = poly->texCoord(i);
				if (minX < 0) {
					minX = texCoord.x;
				}
				if (minY < 0) {
					minY = texCoord.y;
				}
				minX = qMin(texCoord.x, minX);
				minY = qMin(texCoord.y, minY);
				if (minX == 0 && minY == 0) {
					break;
				}
			}

			if (minX == 0 && minY == 0) {
				break;
			}
		}
	}

	if (minX < 0) {
		minX = 0;
	}
	if (minY < 0) {
		minY = 0;
	}

	foreach (Poly *poly, polygons()) {
		if (poly->hasTexture()) {
			for (quint16 i = 0; i < (quint8)poly->count(); ++i) {
				TexCoord texCoord = poly->texCoord(i);

				texCoord.x -= minX;
				texCoord.y -= minY;
				if (texWidth != 0) {
					texCoord.x /= texWidth;
				}
				if (texHeight != 0) {
					texCoord.y /= texHeight;
				}

				poly->setTexCoord(i, texCoord);
			}
		}
	}
}

void FieldModelGroup::setFloatCoords(float texWidth, float texHeight)
{
	foreach (Poly *poly, polygons()) {
		if (poly->hasTexture()) {
			for (quint16 i = 0; i < (quint8)poly->count(); ++i) {
				TexCoord texCoord = poly->texCoord(i);

				if (texWidth != 0) {
					texCoord.x /= texWidth;
				}
				if (texHeight != 0) {
					texCoord.y /= texHeight;
				}

				poly->setTexCoord(i, texCoord);
			}
		}
	}
}

FieldModelPart::FieldModelPart()
{
}

FieldModelPart::~FieldModelPart()
{
	qDeleteAll(_groups);
}

QString FieldModelPart::toString() const
{
	QString ret;
	int groupID=0, ID;

	foreach(FieldModelGroup *group, _groups) {
		ret.append(QString("==== GROUP %1 ==== texNumber: %2\n").arg(groupID).arg(group->textureRef()->textureIdentifier()));

		ID = 0;
		foreach(Poly *poly, group->polygons()) {
			ret.append(QString("==== poly %1 ====\n").arg(ID));

			for(int i=0 ; i<poly->count() ; ++i) {
				ret.append(QString("%1: vertex(%2, %3, %4) color(%5, %6, %7)")
						   .arg(i)
						   .arg(poly->vertex(i).x).arg(poly->vertex(i).y).arg(poly->vertex(i).z)
						   .arg(qRed(poly->color(i))).arg(qGreen(poly->color(i))).arg(qBlue(poly->color(i))));
				if(poly->hasTexture()) {
					ret.append(QString(" texCoord(%1, %2)")
							   .arg(poly->texCoord(i).x).arg(poly->texCoord(i).y));
				}
				ret.append("\n");
			}

			++ID;
		}

		++groupID;
	}

	return ret;
}
