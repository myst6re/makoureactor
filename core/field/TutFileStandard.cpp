/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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
#include "TutFileStandard.h"
#include "Field.h"

TutFileStandard::TutFileStandard(Field *field) :
	TutFile(field)
{
}

TutFileStandard::TutFileStandard(const QList<QByteArray> &tutos) :
	TutFile(tutos)
{
}

bool TutFileStandard::open()
{
	return TutFile::open(field()->sectionData(Field::Scripts));
}

QList<quint32> TutFileStandard::openPositions(const QByteArray &data) const
{
	const char *constData = data.constData();
	QList<quint32> positions;
	quint32 dataSize = data.size(), posAKAO, posAKAOList;
	quint16 version, nbAKAO;
	quint8 nbEntity;
	bool isDemo;

	if(dataSize <= 8) {
		return positions;
	}

	memcpy(&version, constData, 2);

	isDemo = version == 0x0301; // Check version format

	nbEntity = constData[2];
	memcpy(&nbAKAO, constData + 6, 2);

	posAKAOList = (isDemo ? 24 : 32) + nbEntity*8;

	if(dataSize <= posAKAOList+nbAKAO*4) {
		return positions;
	}

	for(int i=0 ; i<nbAKAO ; ++i) {
		memcpy(&posAKAO, constData + posAKAOList + i*4, 4);
		positions.append(posAKAO);
	}

	positions.append(dataSize);

	return positions;
}

QByteArray TutFileStandard::save(QByteArray &toc, quint32 firstPos) const
{
	quint32 pos, tutoID = 0;
	QByteArray ret;

	toc.clear();

	foreach(const QByteArray &tuto, dataList()) {
		pos = firstPos + ret.size();
		toc.append((char *)&pos, 4);
		ret.append(tuto);
		// 4 bytes aligned (not for tutos, only for AKAO)
		if (isAkao(tutoID) && tuto.size() % 4 != 0) {
			ret.append(QByteArray(4 - tuto.size() % 4, '\0'));
		}
		++tutoID;
	}

	return ret;
}

QByteArray TutFileStandard::save() const
{
	return QByteArray();
}

bool TutFileStandard::hasTut() const
{
	int size = this->size();
	for(int i=0 ; i<size ; ++i) {
		if(isTut(i))	return true;
	}
	return false;
}

bool TutFileStandard::isTut(int tutID) const
{
	return !isAkao(tutID);
}

bool TutFileStandard::isAkao(int tutID) const
{
	return data(tutID).startsWith("AKAO")/* || !field()->scriptsAndTexts()->listUsedTuts().contains(tutID) */;
}

QString TutFileStandard::parseScripts(int tutID) const
{
	QString ret;

	if(!isTut(tutID)) {
		const QByteArray &tuto = data(tutID);
		const char *constTuto = tuto.constData();

		quint16 id, length, firstPos;
		if(tuto.size() < 6) {
			return QObject::tr("Erreur");
		}
		memcpy(&id, constTuto + 4, 2);

		ret.append(QObject::tr("totalLength=%1\nid=%2\n").arg(tuto.size()).arg(id));

		if(tuto.size() < 8) {
			return ret;
		}
		memcpy(&length, constTuto + 6, 2);

		ret.append(QObject::tr("length=%1\n").arg(length));

		if(tuto.size() < 22) {
			return ret;
		}
		memcpy(&firstPos, constTuto + 20, 2);

		ret.append(QObject::tr("nbCanaux=%1\n").arg(firstPos/2 + 1));
		ret.append(tuto.mid(8, 8).toHex());
		ret.append("\n");
		ret.append(tuto.mid(16, 4).toHex());
		ret.append("\n");
	} else {
		ret = TutFile::parseScripts(tutID);
	}

	return ret;
}

int TutFileStandard::akaoID(int tutID) const
{
	if(isTut(tutID))	return -1;

	const QByteArray &data = this->data(tutID);
	if(data.size() < 6)		return -1;
	quint16 id;
	memcpy(&id, data.constData() + 4, 2);

	return id;
}

void TutFileStandard::setAkaoID(int tutID, quint16 akaoID)
{
	if(isTut(tutID))	return;

	dataRef(tutID).replace(4, 2, (char *)&akaoID, 2);
	setModified(true);
}
