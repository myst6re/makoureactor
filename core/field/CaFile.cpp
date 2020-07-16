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

void CaFile::initEmpty()
{
	cameras.clear();
	Camera ca = Camera();
	ca.camera_axis[0].x = 4094;
	ca.camera_axis[0].y = -101;
	ca.camera_axis[0].z = -1;
	ca.camera_axis[1].x = -59;
	ca.camera_axis[1].y = -2390;
	ca.camera_axis[1].z = -3327;
	ca.camera_axis[2].x = 81;
	ca.camera_axis[2].y = 3325;
	ca.camera_axis[2].z = -2391;
	ca.camera_position[0] = -42;
	ca.camera_position[1] = -255;
	ca.camera_position[2] = 1476;
	ca.camera_zoom = 525;
	cameras.append(ca);
}

bool CaFile::open()
{
	return open(field()->sectionData(Field::Camera));
}

bool CaFile::open(const QByteArray &data)
{
	const char *constData = data.constData();
	int caSize = data.size();
	bool isPC;

	if (sizeof(Camera) != 40) {
		qWarning() << "sizeof ca struct error" << sizeof(Camera);
		return false;
	}

	if (caSize == 40 || ((caSize - 38) > 0 && (caSize - 38) % 18 == 0)) {
		isPC = false;
	} else if (caSize == 38 || caSize % 38 == 0) {
		isPC = true;
	} else {
		qWarning() << "invalid ca size" << caSize;
		return false;
	}

	clear();

	Camera camera = Camera();

	// First camera (mandatory)
	memcpy(&camera, constData, caSize != 40 ? 38 : 40);
	cameras.append(camera);

	if (caSize > 40) {
		/* The structure can be partially filled
		 * In that case, values of the first camera
		 * are implicitly used to complete. */
		quint32 caCount;
		if (isPC) {
			caCount = caSize/38;
			for (quint32 i=1; i<caCount; ++i) {
				memcpy(&camera, constData + i * 38, 38);
				cameras.append(camera);
			}
		} else {
			caCount = (caSize - 38) / 18;
			for (quint32 i=0; i<caCount; ++i) {
				memcpy(&camera, constData + 38 + i * 18, 18);
				cameras.append(camera);
			}
		}
	}

	setOpen(true);

	return true;
}

QByteArray CaFile::save() const
{
	QByteArray ca;

	// Camera(s)
	int camId = 0;
	for (Camera camera : cameras) {

		if (field()->isPS() && camId > 0) {
			ca.append((char *)&camera, 18);
		} else {
			camera.camera_axis2z = camera.camera_axis[2].z;
			ca.append((char *)&camera, 38);
		}

		++camId;
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
	if (cameras.size() > 1) {
		cameras.removeAt(camID);
		setModified(true);
		return true;
	}
	return false;
}
