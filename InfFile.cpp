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

		if((quint32)contenu.size() <= debutSection9) {
			qWarning() << "Error inf data size" << debutSection9 << contenu.size();
			return false;
		}
	} else {
		memcpy(&debutSection1, constData, 4);
		memcpy(&debutSection, &constData[16], 4);// section 5
		memcpy(&debutSection6, &constData[20], 4);
		debutSection = debutSection - debutSection1 + 28;
		debutSection6 = debutSection6 - debutSection1 + 28;

		size = debutSection6-debutSection;

		if((quint32)contenu.size() <= debutSection6) {
			qWarning() << "Error inf data size" << debutSection6 << contenu.size();
			return false;
		}
	}

	if(size != 740 && size != 536) {
		qWarning() << "Error inf size" << size;
//		QFile debinf("debugInf.inf");
//		debinf.open(QIODevice::WriteOnly);
//		debinf.write(&constData[debutSection], size);
//		debinf.close();
		return false;
	}

	data = InfData();
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

qint16 InfFile::cameraFocusHeight() const
{
	return data.cameraFocusHeight;
}

void InfFile::setCameraFocusHeight(qint16 cameraFocusHeight)
{
	data.cameraFocusHeight = cameraFocusHeight;
	_isModified = true;
}

const Range &InfFile::cameraRange() const
{
	return data.camera_range;
}

void InfFile::setCameraRange(const Range &range)
{
	data.camera_range = range;
	_isModified = true;
}

qint16 InfFile::bgLayer3Width() const
{
	return data.bg_layer3_width;
}

void InfFile::setBgLayer3Width(qint16 width)
{
	data.bg_layer3_width = width;
	_isModified = true;
}

qint16 InfFile::bgLayer3Height() const
{
	return data.bg_layer3_height;
}

void InfFile::setBgLayer3Height(qint16 height)
{
	data.bg_layer3_height = height;
	_isModified = true;
}

qint16 InfFile::bgLayer4Width() const
{
	return data.bg_layer4_width;
}

void InfFile::setBgLayer4Width(qint16 width)
{
	data.bg_layer4_width = width;
	_isModified = true;
}

qint16 InfFile::bgLayer4Height() const
{
	return data.bg_layer4_height;
}

void InfFile::setBgLayer4Height(qint16 height)
{
	data.bg_layer4_height = height;
	_isModified = true;
}

QList<Exit> InfFile::exitLines() const
{
	QList<Exit> exit;
	for(int i=0 ; i<12 ; ++i) {
		exit.append(data.doors[i]);
	}
	return exit;
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

QList<Trigger> InfFile::triggers() const
{
	QList<Trigger> trigger;
	for(int i=0 ; i<12 ; ++i) {
		trigger.append(data.triggers[i]);
	}
	return trigger;
}

const Trigger &InfFile::trigger(quint8 id) const
{
	return data.triggers[id];
}

void InfFile::setTrigger(quint8 id, const Trigger &trigger)
{
	data.triggers[id] = trigger;
	_isModified = true;
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

QList<Arrow> InfFile::arrows() const
{
	QList<Arrow> arrowList;
	for(int i=0 ; i<12 ; ++i) {
		arrowList.append(data.arrows[i]);
	}
	return arrowList;
}

const Arrow &InfFile::arrow(quint8 id) const
{
	return data.arrows[id];
}

void InfFile::setArrow(quint8 id, const Arrow &arrow)
{
	data.arrows[id] = arrow;
	_isModified = true;
}

QByteArray InfFile::unknown1() const
{
	return QByteArray((char *)&data.u1, 4);
}

void InfFile::setUnknown1(const QByteArray &u)
{
	memcpy(&data.u1, u.leftJustified(4, '\0', true).constData(), 4);
	_isModified = true;
}

QByteArray InfFile::unknown2() const
{
	return QByteArray((char *)data.u2, 24);
}

void InfFile::setUnknown2(const QByteArray &u)
{
	memcpy(data.u2, u.leftJustified(24, '\0', true).constData(), 24);
	_isModified = true;
}

void InfFile::test()
{
//	for(int i=0 ; i<12 ; ++i) {
//		if(data.arrows[i].type == 0) {
//			qDebug() << "arrow unknown" << QByteArray((char *)&data.arrows[i].type, 4).toHex();
//		}
//	}
//	return;
	if(data.u1 != 0) {
		qDebug() << "blank1" << QByteArray((char *)&data.u1, 4).toHex() << data.u1 << int((data.u1 >> 16) & 0xFFFF) << int(data.u1 & 0xFFFF);
	}
	for(int i=0 ; i<24 ; ++i) {
		if(data.u2[i] != 0) {
			qDebug() << "blank2" << QByteArray((char *)&data.u2, 24).toHex();
			break;
		}
	}
}
