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
#include "InfFile.h"

InfFile::InfFile()
	: _isOpen(false), _isModified(false)
{
}

InfFile::InfFile(const QByteArray &contenu)
	: _isOpen(false), _isModified(false)
{
	open(contenu);
}

bool InfFile::isOpen()
{
	return _isOpen;
}

bool InfFile::isModified()
{
	return _isModified;
}

void InfFile::setModified(bool modified)
{
	_isModified = modified;
}

bool InfFile::open(const QByteArray &contenu, bool importMode)
{
	const char *constData = contenu.constData();
	quint32 debutSection1, debutSection6, debutSection9, debutSection;
	quint32 oldSize = size;

	if(sizeof(InfData) != 740) {
		qWarning() << "Error InfData" << sizeof(InfData) << "must be 740";
		return false;
	}

	if(contenu.startsWith(QByteArray("\x00\x00\x09\x00\x00\x00", 6))) {
		memcpy(&debutSection, &constData[34], 4);// section 8
		memcpy(&debutSection9, &constData[38], 4);
		debutSection += 4;

		size = debutSection9-debutSection;

		if((quint32)contenu.size() <= debutSection9 || size != sizeof(InfData))
			return false;
	} else {
		memcpy(&debutSection1, constData, 4);
		memcpy(&debutSection, &constData[16], 4);// section 5
		memcpy(&debutSection6, &constData[20], 4);
		debutSection = debutSection - debutSection1 + 28;
		debutSection6 = debutSection6 - debutSection1 + 28;

		size = debutSection6-debutSection;

		if((quint32)contenu.size() <= debutSection6 || size > sizeof(InfData))
			return false;
	}

	memset(&data, 0, sizeof(InfData));
	memcpy(&data, &constData[debutSection], size);
	data.name[8] = '\x00';

	if(importMode && _isOpen)		size = oldSize;

	_isOpen = true;

	return true;
}

QByteArray InfFile::save()
{
	return QByteArray((char *)&data, size);
}

bool InfFile::isJap()
{
	return size != sizeof(InfData);
}

QString InfFile::mapName()
{
	return QString(data.name);
}

void InfFile::setMapName(const QString &name)
{
	strcpy(data.name, name.toLatin1().constData());
	data.name[8] = '\x00';
	_isModified = true;
}

quint8 InfFile::control()
{
	return data.control;
}

void InfFile::setControl(quint8 control)
{
	data.control = control;
	_isModified = true;
}

Exit InfFile::exitLine(quint8 id)
{
	return data.doors[id];
}

void InfFile::setExitLine(quint8 id, const Exit &line)
{
	data.doors[id] = line;
	_isModified = true;
}

const U1 &InfFile::unknownData1(quint8 id) const
{
	return data.u2[id];
}

void InfFile::setUnknownData1(quint8 id, const U1 &unknown)
{
	data.u2[id] = unknown;
}

bool InfFile::arrowIsDisplayed(quint8 id)
{
	return data.display_arrow[id] & 1;
}

void InfFile::setArrowDiplay(quint8 id, bool display)
{
	data.display_arrow[id] = (data.display_arrow[id] & 0xFE) | display;
	_isModified = true;
}

const U2 &InfFile::unknownData2(quint8 id) const
{
	return data.u3[id];
}

void InfFile::setUnknownData2(quint8 id, const U2 &unknown)
{
	data.u3[id] = unknown;
}

void InfFile::test()
{
	if(data._blank1 != 0) {
		qDebug() << "blank1" << QByteArray((char *)&data._blank1, 4).toHex();
	}
	for(int i=0 ; i<24 ; ++i) {
		if(data._blank2[i] != 0) {
			qDebug() << "blank2" << QByteArray((char *)&data._blank2, 24).toHex();
			break;
		}
	}
}
