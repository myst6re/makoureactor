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
#include "CaFile.h"

CaFile::CaFile() :
	opened(false), modified(false)
{
}

bool CaFile::open(const QByteArray &data)
{
	const char *constData = data.constData();
	quint32 posSection1, posSectionCa, posSection3, posSection5;
	int caSize;

	if(sizeof(Camera) != 40) {
		qWarning() << "sizeof ca struct error" << sizeof(Camera);
		return false;
	}

	if(data.startsWith(QByteArray("\x00\x00\x09\x00\x00\x00", 6))) {
		memcpy(&posSectionCa, &constData[10], 4);// section 2
		memcpy(&posSection3, &constData[14], 4);

		posSectionCa += 4;

		caSize = posSection3-posSectionCa;

		// warning : some padding data in ca section
		if((quint32)data.size() <= posSection3) {
			qWarning() << "invalid data size ca" << posSection3 << data.size();
			return false;
		}
	} else {
		memcpy(&posSection1, constData, 4);
		memcpy(&posSectionCa, &constData[12], 4);// section 4
		memcpy(&posSection5, &constData[16], 4);
		posSectionCa = posSectionCa - posSection1 + 28;
		posSection5 = posSection5 - posSection1 + 28;

		caSize = posSection5-posSectionCa;

		// warning : some padding data in ca section
		if((quint32)data.size() <= posSection5) {
			qWarning() << "invalid data size ca" << posSection5 << data.size();
			return false;
		}
	}

	if(caSize < 38) {
		qWarning() << "invalid ca size" << caSize;
		return false;
	}

	cameras.clear();

	Camera camera;

	quint32 caCount = caSize / 38;

	for(quint32 i=0 ; i<caCount ; ++i) {
		memcpy(&camera, &constData[posSectionCa + i*38], 38);

		cameras.append(camera);
	}

	opened = true;
	modified = false;

	return true;
}

bool CaFile::save(QByteArray &ca)
{
	if(!opened)		return false;

	foreach(Camera camera, cameras) {
		camera.camera_axis2z = camera.camera_axis[2].z;
		ca.append((char *)&camera, 38);
	}

	if(ca.size() % 38 != 0) {
		qWarning() << "Bad size save ca" << ca.size();
	}

	return true;
}

bool CaFile::isOpen() const
{
	return opened;
}

bool CaFile::isModified() const
{
	return modified;
}

void CaFile::setModified(bool modified)
{
	this->modified = modified;
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
	modified = true;
}

void CaFile::insertCamera(int camID, const Camera &cam)
{
	cameras.insert(camID, cam);
	modified = true;
}

void CaFile::removeCamera(int camID)
{
	if(cameras.size() > 1) {
		cameras.removeAt(camID);
		modified = true;
	}
}
