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
#include "TutFile.h"

TutFile::TutFile()
	: _isOpen(false), _isModified(false), tutType(false)
{
}

TutFile::TutFile(const QByteArray &data, bool tutType)
	: _isOpen(false), _isModified(false)
{
	open(data, tutType);
}

bool TutFile::isOpen() const
{
	return _isOpen;
}

bool TutFile::isModified() const
{
	return _isModified;
}

void TutFile::setModified(bool modified)
{
	_isModified = modified;
}

bool TutFile::open(const QByteArray &data, bool tutType)
{
	const char *constData = data.constData();
	QList<quint32> positions;
	quint32 dataSize = data.size();

	this->tutType = tutType;

	if(!tutType) {
		quint32 posAKAO, posAKAOList;
		quint16 nbAKAO;
		quint8 nbEntity;

		if(dataSize <= 8)
			return false;

		nbEntity = constData[2];
		memcpy(&nbAKAO, &constData[6], 2);

		posAKAOList = 32+nbEntity*8;

		if(dataSize <= posAKAOList+nbAKAO*4)
			return false;

		for(int i=0 ; i<nbAKAO ; ++i) {
			memcpy(&posAKAO, &constData[posAKAOList+i*4], 4);
			positions.append(posAKAO);
		}

		positions.append(dataSize);
	} else {
		quint16 posTut;
		for(int i=0 ; i<9 ; ++i) {
			memcpy(&posTut, &constData[i*2], 2);
			if(posTut < 18)		posTut = 18;
			positions.append(posTut);
		}

		positions.append(dataSize);
	}

	tutos.clear();
	for(int i=0 ; i<positions.size()-1 ; ++i) {
		if(positions.at(i) < dataSize && positions.at(i) < positions.at(i+1)) {
			tutos.append(data.mid(positions.at(i), positions.at(i+1)-positions.at(i)));
		}
	}

	_isOpen = true;

	return true;
}

QByteArray TutFile::save(QByteArray &toc, quint32 firstPos) const
{
	quint32 pos;
	QByteArray ret;

	toc.clear();

	if(!tutType) {
		foreach(const QByteArray &tuto, tutos) {
			pos = firstPos + ret.size();
			toc.append((char *)&pos, 4);
			ret.append(tuto);
		}
	} else {
		for(int i=0 ; i<9 ; ++i) {
			if(i < tutos.size()) {
				pos = 18 + ret.size();
				ret.append(tutos.at(i));
			} else {
				pos = 0xffff;
			}
			toc.append((char *)&pos, 2);
		}
	}

	return ret;
}

int TutFile::size() const
{
	return tutos.size();
}

bool TutFile::hasTut() const
{
	int size = this->size();
	for(int i=0 ; i<size ; ++i) {
		if(isTut(i))	return true;
	}
	return false;
}

bool TutFile::isTut(int tutID) const
{
	return tutos.value(tutID).left(4) != "AKAO" && tutos.value(tutID).left(3) != "KAO";// PC bug
}

void TutFile::removeTut(int tutID)
{
	tutos.removeAt(tutID);
	_isModified = true;
}

bool TutFile::insertTut(int tutID)
{
	if((tutType && size()<9) || (!tutType && size()<256)) {
		tutos.insert(tutID, QByteArray("\x11", 1));
		_isModified = true;
		return true;
	}
	return false;
}

bool TutFile::insertAkao(int tutID, const QString &akaoPath)
{
	if(!tutType) {
		QFile f(akaoPath);
		if(f.open(QIODevice::ReadOnly)) {
			tutos.insert(tutID, f.readAll());
			_isModified = true;
			f.close();
			return true;
		}
	}
	return false;
}

const QByteArray &TutFile::data(int tutID) const
{
	return tutos.at(tutID);
}

void TutFile::setData(int tutID, const QByteArray &data)
{
	tutos.replace(tutID, data);
	_isModified = true;
}

QString TutFile::parseScripts(int tutID) const
{
	QByteArray tuto = tutos.value(tutID);
	const char *constTuto = tuto.constData();
	QString ret;
	bool jp = Config::value("jp_txt", false).toBool();

	if(isTut(tutID)) {
		quint8 clef;
		int i=0, size = tuto.size(), endOfText;

		while(i<size)
		{
			switch(clef = tuto.at(i++))
			{
			case 0x00:
				quint16 u;
				memcpy(&u, &constTuto[i], 2);
				ret.append(QString("PAUSE(%1)").arg(u));
				i+=2;
				break;
			case 0x02:	ret.append("[UP]");			break;
			case 0x03:	ret.append("[DOWN]");		break;
			case 0x04:	ret.append("[LEFT]");		break;
			case 0x05:	ret.append("[RIGHT]");		break;
			case 0x06:	ret.append("[MENU]");		break;
			case 0x07:	ret.append("[CANCEL]");		break;
			case 0x08:	ret.append("[CHANGE]");		break;
			case 0x09:	ret.append("[OK]");			break;
			case 0x0A:	ret.append("[R1]");			break;
			case 0x0B:	ret.append("[R2]");			break;
			case 0x0C:	ret.append("[L1]");			break;
			case 0x0D:	ret.append("[L2]");			break;
			case 0x0E:	ret.append("[START]");		break;
			case 0x0F:	ret.append("[SELECT]");		break;
			case 0x10:
				endOfText = tuto.indexOf('\xff', i);
				if(endOfText!=-1) {
					ret.append(FF7Text(tuto.mid(i, endOfText-i)).getText(jp));
					if(endOfText+1>i)
						i = endOfText+1;
				}
				else {
					ret.append(FF7Text(tuto.mid(i)).getText(jp));
					return ret;
				}
				break;
			case 0x11:	ret.append("{FINISH}");		break;
			case 0x12:
				quint16 x, y;
				memcpy(&x, &constTuto[i], 2);
				memcpy(&y, &constTuto[i+2], 2);
				ret.append(QString("MOVE(%1,%2)").arg(x).arg(y));
				i+=4;
				break;
			case 0xff:
				ret.append("{NOP}");
				break;
			default:
				ret.append(QString("[%1]").arg(clef,2,16,QChar('0')));
				break;
			}
			ret.append("\n");
		}

	} else {
		quint16 id, length, firstPos;
		memcpy(&id, &constTuto[4], 2);
		memcpy(&length, &constTuto[6], 2);
		memcpy(&firstPos, &constTuto[20], 2);

		ret = QObject::tr("totalLength=%4\nid=%1\nlength=%2\nnbCanaux=%3\n").arg(id).arg(length).arg(firstPos/2 + 1).arg(tuto.size());
		ret.append(tuto.mid(8, 8).toHex());
		ret.append("\n");
		ret.append(tuto.mid(16, 4).toHex());
		ret.append("\n");
	}

	return ret;
}

void TutFile::parseText(int tutID, const QString &tuto)
{
	if(!isTut(tutID))	return;

	QStringList lines = tuto.split('\n', QString::SkipEmptyParts), params;
	QByteArray ret;
	bool ok;
	int value;
	bool jp = Config::value("jp_txt", false).toBool();

	foreach(QString line, lines) {
		if(line.startsWith("MOVE(") && line.endsWith(")")) {
			ret.append('\x12');
			line.chop(1);// remove ")"
			params = line.mid(5).split(',');
			if(params.size()>=2) {
				value = params.first().toInt(&ok);
				if(ok) {
					ret.append((char *)&value, 2);
				}
				else {
					ret.append("\x00\x00", 2);
				}
				value = params.at(1).toInt(&ok);
				if(ok) {
					ret.append((char *)&value, 2);
				}
				else {
					ret.append("\x00\x00", 2);
				}
			}
			else {
				ret.append("\x00\x00\x00\x00", 4);
			}
		}
		else if(line.startsWith("PAUSE(") && line.endsWith(")")) {
			ret.append('\x00');
			line.chop(1);// remove ")"

			value = line.mid(6).toInt(&ok);
			if(ok) {
				ret.append((char *)&value, 2);
			}
			else {
				ret.append("\x00\x00", 2);
			}
		}
		else if(line=="{FINISH}") {			ret.append('\x11');}
		else if(line=="{NOP}") {			ret.append('\xff');}
		else if(line=="[UP]") {				ret.append('\x02');}
		else if(line=="[DOWN]") {			ret.append('\x03');}
		else if(line=="[LEFT]") {			ret.append('\x04');}
		else if(line=="[RIGHT]") {			ret.append('\x05');}
		else if(line=="[MENU]")	 {			ret.append('\x06');}
		else if(line=="[CANCEL]") {			ret.append('\x07');}
		else if(line=="[CHANGE]") {			ret.append('\x08');}
		else if(line=="[OK]")	 {			ret.append('\x09');}
		else if(line=="[R1]")	 {			ret.append('\x0a');}
		else if(line=="[R2]")	 {			ret.append('\x0b');}
		else if(line=="[L1]")	 {			ret.append('\x0c');}
		else if(line=="[L2]")	 {			ret.append('\x0d');}
		else if(line=="[START]") {			ret.append('\x0e');}
		else if(line=="[SELECT]")	 {		ret.append('\x0f');}
		else if(line.startsWith("[") && line.endsWith("]")) {
			value = line.mid(1,2).toInt(&ok);
			if(ok) {
				ret.append((char)value);
			}
		}
		else {
			ret.append('\x10');
			ret.append(FF7Text(line, jp).getData()).append('\xff');
		}
	}

	tutos.replace(tutID, ret);

	_isModified = true;
}

int TutFile::akaoID(int tutID) const
{
	if(isTut(tutID))	return -1;

	QByteArray data = tutos.value(tutID);
	if(data.size() < 6)		return -1;
	quint16 id;
	memcpy(&id, &(data.constData()[4]), 2);

	return id;
}

void TutFile::setAkaoID(int tutID, quint16 akaoID)
{
	if(isTut(tutID))	return;

	tutos[tutID].replace(4, 2, (char *)&akaoID, 2);
	_isModified = true;
}
