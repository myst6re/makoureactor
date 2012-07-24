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
#include "WalkmeshFile.h"

WalkmeshFile::WalkmeshFile()
	: _isOpenCa(false), _isOpenId(false), _isModifiedCa(false), _isModifiedId(false)
{
}

WalkmeshFile::WalkmeshFile(const QByteArray &contenu)
	:  _isOpenCa(false), _isOpenId(false), _isModifiedCa(false), _isModifiedId(false)
{
	open(contenu);
}

bool WalkmeshFile::isOpen() const
{
	return _isOpenCa && _isOpenId;
}

bool WalkmeshFile::isOpenCa() const
{
	return _isOpenCa;
}

bool WalkmeshFile::isOpenId() const
{
	return _isOpenId;
}

bool WalkmeshFile::isModifiedCa() const
{
	return _isOpenCa && _isModifiedCa;
}

bool WalkmeshFile::isModifiedId() const
{
	return _isOpenId && _isModifiedId;
}

void WalkmeshFile::setModifiedCa(bool modified)
{
	_isModifiedCa = modified;
}

void WalkmeshFile::setModifiedId(bool modified)
{
	_isModifiedId = modified;
}

bool WalkmeshFile::open(const QByteArray &contenu, bool openCa)
{
	const char *constData = contenu.constData();
	quint32 debutSection1, debutSectionCa, debutSection3, debutSectionId, debutSection5, debutSection6, nbSector;

	if(contenu.startsWith(QByteArray("\x00\x00\x09\x00\x00\x00", 6))) {
		memcpy(&debutSectionCa, &constData[10], 4);// section 2
		memcpy(&debutSection3, &constData[14], 4);
		memcpy(&debutSectionId, &constData[22], 4);// section 5
		memcpy(&debutSection6, &constData[26], 4);

		debutSectionCa += 4;
		debutSectionId += 4;

		// warning : some padding data in ca section
		if((quint32)contenu.size() <= debutSection6 || debutSection3-debutSectionCa < 38) {
			qWarning() << "invalid ca size" << (debutSection3-debutSectionCa);
			return false;
		}
	} else {
		memcpy(&debutSection1, constData, 4);
		memcpy(&debutSectionId, &constData[4], 4);// section 2
		memcpy(&debutSectionCa, &constData[12], 4);// section 4
		memcpy(&debutSection5, &constData[16], 4);
		debutSectionId = debutSectionId - debutSection1 + 28;
		debutSectionCa = debutSectionCa - debutSection1 + 28;
		debutSection5 = debutSection5 - debutSection1 + 28;

		// warning : some padding data in ca section
		if((quint32)contenu.size() <= debutSection5 || debutSection5-debutSectionCa < 38) {
			qWarning() << "invalid ca size" << (debutSection5-debutSectionCa);
			return false;
		}
	}

	//TODO: multiple camera

	if(!_isOpenCa && openCa) {
		memcpy(&camera_axis, &constData[debutSectionCa], 18);
		memcpy(&camera_axis2z, &constData[debutSectionCa+18], 2);
		memcpy(&camera_position, &constData[debutSectionCa+20], 12);
		memcpy(&camera_blank, &constData[debutSectionCa+32], 4);
		memcpy(&camera_zoom, &constData[debutSectionCa+36], 2);

		/*qDebug() << camera_axis[0].x << camera_axis[0].y << camera_axis[0].z
				<< camera_axis[1].x << camera_axis[1].y << camera_axis[1].z
				<< camera_axis[2].x << camera_axis[2].y << camera_axis[2].z;
		qDebug() << camera_position[0] << camera_position[1] << camera_position[2];*/
		_isOpenCa = true;
	}

	if(!_isOpenId) {
		memcpy(&nbSector, &constData[debutSectionId], 4);

		debutSectionId += 4;

		quint32 accessStart = debutSectionId+nbSector*24, i;

		Triangle triangle;
		Access acc;
		for(i=0 ; i<nbSector ; ++i) {
			memcpy(&triangle, &constData[debutSectionId+i*24], 24);
			triangles.append(triangle);
//			qDebug() << triangle.vertices[0].x << triangle.vertices[0].y << triangle.vertices[0].z << triangle.vertices[0].res;
//			qDebug() << triangle.vertices[1].x << triangle.vertices[1].y << triangle.vertices[1].z << triangle.vertices[1].res;
//			qDebug() << triangle.vertices[2].x << triangle.vertices[2].y << triangle.vertices[2].z << triangle.vertices[2].res;
//			qDebug() << "=====";
			memcpy(&acc, &constData[accessStart+i*6], 6);
			access.append(acc);
//			qDebug() << acc.a1 << acc.a2 << acc.a3;
//			qDebug() << "=====";
		}
		_isOpenId = true;
	}

	return true;
}

QByteArray WalkmeshFile::saveCa() const
{
	if(!_isOpenCa)	return QByteArray();
	QByteArray ca;

	ca.append((char *)&camera_axis, 18);
	ca.append((char *)&camera_axis2z, 2);
	ca.append((char *)&camera_position, 12);
	ca.append((char *)&camera_blank, 4);
	ca.append((char *)&camera_zoom, 2);

	if(ca.size() != 38) {
		qWarning() << "Mauvaise taille save ca";
	}

	return ca;
}

QByteArray WalkmeshFile::saveId() const
{
	if(!_isOpenId)	return QByteArray();
	QByteArray id;
	quint32 nbSector = qMin(triangles.size(), access.size());
	Triangle triangle;
	Access acc;

	id.append((char *)&nbSector, 4);

	for(quint32 i=0 ; i<nbSector ; ++i) {
		id.append((char *)&(triangle = triangles.at(i)), 24);
	}
	for(quint32 i=0 ; i<nbSector ; ++i) {
		id.append((char *)&(acc = access.at(i)), 6);
	}

	return id;
}

bool WalkmeshFile::test() const
{
	if(camera_axis[2].z != camera_axis2z) {
		qDebug() << "camera unknown1" << camera_axis[2].z << camera_axis2z;
		return false;
	}

	if(0 != camera_blank) {
		qDebug() << "camera unknown2" << camera_position[2] << camera_blank;
		return false;
	}

	return true;
}
