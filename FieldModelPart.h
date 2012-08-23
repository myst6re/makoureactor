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
#include "IdFile.h"


typedef struct {
	float x, y, z;
} PolyVertex;

typedef struct {
	float x, y;
} TexCoord;

class Poly
{
public:
	Poly(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords=QList<TexCoord>());
	Poly(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords=QList<TexCoord>());
	virtual ~Poly();
	void setVertices(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords=QList<TexCoord>());
	void setVertices(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords=QList<TexCoord>());
	void divTexCoords(float texWidth, float texHeight);
	virtual int count() const=0;
	const PolyVertex &vertex(quint8 id) const;
	const QRgb &color() const;
	QRgb color(quint8 id) const;
	const TexCoord &texCoord(quint8 id) const;
	bool isMonochrome() const;
	bool hasTexture() const;
protected:
	QList<PolyVertex> _vertices;
	QList<QRgb> _colors;
	QList<TexCoord> _texCoords;
};

class QuadPoly : public Poly
{
public:
	QuadPoly(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords=QList<TexCoord>());
	QuadPoly(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords=QList<TexCoord>());
	int count() const { return 4; }
};

class TrianglePoly : public Poly
{
public:
	TrianglePoly(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords=QList<TexCoord>());
	TrianglePoly(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords=QList<TexCoord>());
	int count() const { return 3; }
};

class FieldModelGroup
{
public:
	FieldModelGroup();
	FieldModelGroup(int texNumber);
	virtual ~FieldModelGroup();
	const QList<Poly *> &polygons() const;
	void addPolygon(Poly *polygon);
	int textureNumber() const;
	void setTextureNumber(int texNumber);
private:
	int _textureNumber;
	QList<Poly *> _polys;
};

class FieldModelPart
{
public:
	FieldModelPart();
	virtual ~FieldModelPart();
	const QList<FieldModelGroup *> &groups() const;
	QString toString() const;
protected:
	QList<FieldModelGroup *> _groups;
};

#endif // FIELDMODELPART_H
