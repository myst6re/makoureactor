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
#pragma once

#include <QtCore>
#include <QRgb>
#include "FieldModelTextureRef.h"

struct PolyVertex {
	float x, y, z;
};

struct TexCoord {
	float x, y;
};

class Poly
{
public:
	Poly(int count, const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords=QList<TexCoord>());
	Poly(int count, const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords=QList<TexCoord>());
	virtual ~Poly();
	void setVertices(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords=QList<TexCoord>());
	void setVertices(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords=QList<TexCoord>());
	inline int count() const {
		return _count;
	}
	const PolyVertex &vertex(int id) const;
	const QRgb &color() const;
	QRgb color(int id) const;
	const TexCoord &texCoord(int id) const;
	void setTexCoord(int id, const TexCoord &texCoord);
	bool isMonochrome() const;
	bool hasTexture() const;
protected:
	int _count;
	QList<PolyVertex> _vertices;
	QList<QRgb> _colors;
	QList<TexCoord> _texCoords;
};

class QuadPoly : public Poly
{
public:
	QuadPoly(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords=QList<TexCoord>());
	QuadPoly(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords=QList<TexCoord>());
};

class TrianglePoly : public Poly
{
public:
	TrianglePoly(const QList<PolyVertex> &vertices, const QList<QRgb> &colors, const QList<TexCoord> &texCoords=QList<TexCoord>());
	TrianglePoly(const QList<PolyVertex> &vertices, const QRgb &color, const QList<TexCoord> &texCoords=QList<TexCoord>());
};

class FieldModelFile;

class FieldModelGroup
{
public:
	FieldModelGroup();
	explicit FieldModelGroup(FieldModelTextureRef *texRef);
	virtual ~FieldModelGroup();
	inline const QList<Poly *> &polygons() const {
		return _polys;
	}
	inline void addPolygon(Poly *polygon) {
		_polys.append(polygon);
	}
	inline bool hasTexture() const {
		return _textureRef;
	}
	void setTextureRef(FieldModelTextureRef *texRef);
	virtual inline FieldModelTextureRef *textureRef() const {
		return _textureRef;
	}
	inline void setBlendMode(quint8 blend) {
		_blendMode = blend;
	}
	void removeSpriting(float texWidth, float texHeight) const;
	void setFloatCoords(float texWidth, float texHeight) const;
private:
	FieldModelTextureRef *_textureRef;
	QList<Poly *> _polys;
	quint8 _blendMode;
};

class FieldModelPart
{
public:
	FieldModelPart();
	virtual ~FieldModelPart();
	inline const QList<FieldModelGroup *> &groups() const {
		return _groups;
	}
	inline void setGroups(const QList<FieldModelGroup *> &groups) {
		_groups = groups;
	}
	QString toString() const;
protected:
	QList<FieldModelGroup *> _groups;
};
