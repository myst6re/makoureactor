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
#include "CaFile.h"
#include "Field.h"

CaFile::CaFile(Field *field) :
	FieldPart(field)
{
}

bool CaFile::open()
{
	return open(field()->sectionData(Field::Camera));
}

bool CaFile::open(const QByteArray &data)
{
	const char *constData = data.constData();
	int caSize = data.size();

	if(sizeof(Camera) != 40) {
		qWarning() << "sizeof ca struct error" << sizeof(Camera);
		return false;
	}

	if(caSize < 38) {
		qWarning() << "invalid ca size" << caSize;
		return false;
	}

	clear();

	Camera camera;

	quint32 caCount = caSize / 38;

	for(quint32 i=0 ; i<caCount ; ++i) {
		memcpy(&camera, constData + i*38, 38);

		cameras.append(camera);
	}

	setOpen(true);
	setModified(false);

	return true;
}

QByteArray CaFile::save() const
{
	QByteArray ca;

	foreach(Camera camera, cameras) {
		camera.camera_axis2z = camera.camera_axis[2].z;
		ca.append((char *)&camera, 38);
	}

	if(ca.size() % 38 != 0) {
		qWarning() << "Bad size save ca" << ca.size();
	}

	return ca;
}

void CaFile::clear()
{
	cameras.clear();
}

bool CaFile::hasCamera() const
{
	return !cameras.isEmpty();
}

int CaFile::cameraCount() const
{
	return cameras.size();
}

const Camera &CaFile::camera(int camID) const
{
	return cameras.at(camID);
}

void CaFile::setCamera(int camID, const Camera &cam)
{
	cameras[camID] = cam;
	setModified(true);
}

void CaFile::insertCamera(int camID, const Camera &cam)
{
	cameras.insert(camID, cam);
	setModified(true);
}

bool CaFile::removeCamera(int camID)
{
	if(cameras.size() > 1) {
		cameras.removeAt(camID);
		setModified(true);
		return true;
	}
	return false;
}
