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
#include "IdFile.h"
#include "Field.h"

IdFile::IdFile(Field *field) :
	FieldPart(field)
{
}

void IdFile::initEmpty()
{
	_triangles.clear();
	_access.clear();

	const quint8 base = 132;
	const quint16 w = base * 3;
	const quint16 offsetX = -(w + base);
	const quint16 offsetY = -(w * 2);

	for (quint8 y = 3; y > 0; --y) {
		for (quint8 x = 0; x < 3; ++x) {
			// Make a square
			Triangle tr = Triangle();
			tr.vertices[0].x = offsetX + x * w;
			tr.vertices[0].y = offsetY + y * w;
			tr.vertices[1].x = offsetX + (x + 1) * w;
			tr.vertices[1].y = offsetY + y * w;
			tr.vertices[2].x = offsetX + x * w;
			tr.vertices[2].y = offsetY + (y - 1) * w;
			_triangles.append(tr);
			tr.vertices[0].x = offsetX + (x + 1) * w;
			tr.vertices[1].y = offsetY + (y - 1) * w;
			_triangles.append(tr);

			Access acc1, acc2;
			acc1.a[0] = -1;
			// Access to the other triangle
			acc1.a[1] = _triangles.size() - 1;
			acc1.a[2] = -1;
			acc2.a[0] = -1;
			acc2.a[1] = -1;
			// Access to the other triangle
			acc2.a[2] = _triangles.size() - 2;

			if (x < 2) {
				// Access on the right
				acc2.a[0] = _triangles.size();
			}
			if (x > 0) {
				// Access on the left
				acc1.a[2] = _triangles.size() - 3;
			}
			if (y < 2) {
				// Access on the bottom
				acc2.a[1] = _triangles.size() + 4;
			}
			if (y > 0) {
				// Access on the top
				acc1.a[0] = _triangles.size() - 7;
			}

			_access.append(acc1);
			_access.append(acc2);
		}
	}
}

bool IdFile::open()
{
	return open(field()->sectionData(Field::Walkmesh));
}

bool IdFile::open(const QByteArray &data)
{
	const char *constData = data.constData();
	quint32 nbSector;
	int sizeId = data.size();

	if (sizeof(Triangle) != 24 || sizeof(Access) != 6) {
		qWarning() << "Error id struct size" << sizeof(Triangle) << sizeof(Access);
		Q_ASSERT(false);
	}

	if (sizeId < 4) {
		qWarning() << "size id error" << sizeId;
		return false;
	}

	memcpy(&nbSector, constData, 4);

	quint32 accessStart = 4 + nbSector*24, i;

	// Padding possible
	if ((quint32)sizeId != accessStart + nbSector*6
			&& (quint32)sizeId != accessStart + nbSector*6 + 2) {
		qWarning() << "size id error" << sizeId << (accessStart + nbSector*6);
		return false;
	}

	Triangle triangle;
	Access acc;
	_triangles.clear();
	_access.clear();
	for (i=0; i<nbSector; ++i) {
		memcpy(&triangle, constData + 4 + i*24, 24);
		_triangles.append(triangle);
		memcpy(&acc, constData + accessStart + i*6, 6);
		_access.append(acc);
	}

	setOpen(true);

	return true;
}

QByteArray IdFile::save() const
{
	QByteArray id;
	quint32 count = triangleCount();

	id.append((char *)&count, 4);

	for (Triangle triangle : _triangles) {
		triangle.vertices[0].res = triangle.vertices[0].z;
		triangle.vertices[1].res = triangle.vertices[0].z;
		triangle.vertices[2].res = triangle.vertices[0].z;
		id.append((char *)&triangle.vertices, sizeof(Triangle));
	}

	for (const Access &access : _access) {
		id.append((char *)&access, sizeof(Access));
	}

	return id;
}

void IdFile::clear()
{
	_triangles.clear();
	_access.clear();
}

bool IdFile::hasTriangle() const
{
	return !_triangles.empty();
}

int IdFile::triangleCount() const
{
	return _triangles.size();
}

const QList<Triangle> &IdFile::triangles() const
{
	return _triangles;
}

const Triangle &IdFile::triangle(int triangleID) const
{
	return _triangles.at(triangleID);
}

void IdFile::setTriangle(int triangleID, const Triangle &triangle)
{
	_triangles[triangleID] = triangle;
	setModified(true);
}

void IdFile::insertTriangle(int triangleID, const Triangle &triangle, const Access &access)
{
	_triangles.insert(triangleID, triangle);
	_access.insert(triangleID, access);
	setModified(true);
}

void IdFile::removeTriangle(int triangleID)
{
	_triangles.removeAt(triangleID);
	_access.removeAt(triangleID);
	setModified(true);
}

const Access &IdFile::access(int triangleID) const
{
	return _access.at(triangleID);
}

void IdFile::setAccess(int triangleID, const Access &access)
{
	_access[triangleID] = access;
	setModified(true);
}

Vertex_sr IdFile::fromVertex_s(const Vertex_s &vertex_s)
{
	Vertex_sr vertex_sr;

	vertex_sr.x = vertex_s.x;
	vertex_sr.y = vertex_s.y;
	vertex_sr.z = vertex_s.z;
	vertex_sr.res = 0;

	return vertex_sr;
}

Vertex_s IdFile::toVertex_s(const Vertex_sr &vertex_sr)
{
	Vertex_s vertex_s;

	vertex_s.x = vertex_sr.x;
	vertex_s.y = vertex_sr.y;
	vertex_s.z = vertex_sr.z;

	return vertex_s;
}
