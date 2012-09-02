/****************************************************************************
 ** Deling Final Fantasy VIII Field Editor
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

IdFile::IdFile() :
	modified(false), opened(false), _unknown(0), _hasUnknownData(false)
{
}

bool IdFile::open(const QByteArray &data)
{
	const char *constData = data.constData();
	quint32 nbSector;
	int sizeId = data.size();

	if(sizeof(Triangle) != 24 || sizeof(Access) != 6) {
		qWarning() << "Error id struct size" << sizeof(Triangle) << sizeof(Access);
		return false;
	}

	if(sizeId < 4) {
		qWarning() << "size id error" << sizeId;
		return false;
	}

	memcpy(&nbSector, constData, 4);

	quint32 accessStart = 4 + nbSector*24, i;

	if((quint32)sizeId != accessStart + nbSector*6) {
		if((quint32)sizeId == accessStart + nbSector*6 + 2) {
			memcpy(&_unknown, &constData[accessStart + nbSector*6], 2);
			_hasUnknownData = true;
		} else {
			qWarning() << "size id error" << sizeId << (accessStart + nbSector*6);
			return false;
		}
	}

	Triangle triangle;
	Access acc;
	triangles.clear();
	_access.clear();
	for(i=0 ; i<nbSector ; ++i) {
		memcpy(&triangle, &constData[4 + i*24], 24);
		triangles.append(triangle);
//		qDebug() << triangle.vertices[0].x << triangle.vertices[0].y << triangle.vertices[0].z << triangle.vertices[0].res;
//		qDebug() << triangle.vertices[1].x << triangle.vertices[1].y << triangle.vertices[1].z << triangle.vertices[1].res;
//		qDebug() << triangle.vertices[2].x << triangle.vertices[2].y << triangle.vertices[2].z << triangle.vertices[2].res;
//		qDebug() << "=====";
		memcpy(&acc, &constData[accessStart + i*6], 6);
		_access.append(acc);
//		qDebug() << acc.a1 << acc.a2 << acc.a3;
//		qDebug() << "=====";
	}

	opened = true;
	modified = false;

	return true;
}

bool IdFile::save(QByteArray &id)
{
	quint32 count=triangleCount();

	id.append((char *)&count, 4);

	foreach(Triangle triangle, triangles) {
		triangle.vertices[0].res = triangle.vertices[0].z;
		triangle.vertices[1].res = triangle.vertices[0].z;
		triangle.vertices[2].res = triangle.vertices[0].z;
		id.append((char *)&triangle.vertices, sizeof(Triangle));
	}

	foreach(const Access &access, _access) {
		id.append((char *)&access, sizeof(Access));
	}

	if(_hasUnknownData) {
		id.append((char *)&_unknown, 2);
	}

	return true;
}

bool IdFile::isOpen() const
{
	return opened;
}

bool IdFile::isModified() const
{
	return modified;
}

void IdFile::setModified(bool modified)
{
	this->modified = modified;
}

bool IdFile::hasTriangle() const
{
	return !triangles.empty();
}

int IdFile::triangleCount() const
{
	return triangles.size();
}

const QList<Triangle> &IdFile::getTriangles() const
{
	return triangles;
}

const Triangle &IdFile::triangle(int triangleID) const
{
	return triangles.at(triangleID);
}

void IdFile::setTriangle(int triangleID, const Triangle &triangle)
{
	triangles[triangleID] = triangle;
	modified = true;
}

void IdFile::insertTriangle(int triangleID, const Triangle &triangle, const Access &access)
{
	triangles.insert(triangleID, triangle);
	_access.insert(triangleID, access);
	modified = true;
}

void IdFile::removeTriangle(int triangleID)
{
	triangles.removeAt(triangleID);
	_access.removeAt(triangleID);
	modified = true;
}

const Access &IdFile::access(int triangleID) const
{
	return _access.at(triangleID);
}

void IdFile::setAccess(int triangleID, const Access &access)
{
	_access[triangleID] = access;
	modified = true;
}

bool IdFile::hasUnknownData() const
{
	return _hasUnknownData;
}

qint16 IdFile::unknown() const
{
	return _unknown;
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
