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
#include "../Config.h"
#include "../FF7Text.h"

TutFile::TutFile(Field *field) :
	FieldPart(field)
{
}

bool TutFile::open(const QByteArray &data)
{
	QList<quint32> positions;
	quint32 dataSize = data.size();

	positions = openPositions(data);
	if(positions.isEmpty()) {
		return false;
	}

	tutos.clear();
	for(int i=0 ; i<positions.size()-1 ; ++i) {
		if(positions.at(i) < dataSize && positions.at(i) < positions.at(i+1)) {
			QByteArray t = data.mid(positions.at(i), positions.at(i+1)-positions.at(i));
//			if(t.startsWith("KAO")) {
//				t.prepend('A'); // PC bug
//			}
//			qDebug() << i << t.left(16).toHex();
			tutos.append(t);
		}
	}

	setOpen(true);

	return true;
}

void TutFile::clear()
{
	tutos.clear();
}

int TutFile::size() const
{
	return tutos.size();
}

void TutFile::removeTut(int tutID)
{
	tutos.removeAt(tutID);
	setModified(true);
}

bool TutFile::insertTut(int tutID)
{
	return insertData(tutID, QByteArray("\x11", 1));
}

const QByteArray &TutFile::data(int tutID) const
{
	return tutos.at(tutID);
}

QByteArray &TutFile::dataRef(int tutID)
{
	return tutos[tutID];
}

void TutFile::setData(int tutID, const QByteArray &data)
{
	tutos.replace(tutID, data);
	setModified(true);
}

bool TutFile::insertData(int tutID, const QByteArray &data)
{
	if(size() < maxTutCount()) {
		tutos.insert(tutID, data);
		setModified(true);
		return true;
	}
	return false;
}

bool TutFile::insertData(int tutID, const QString &path)
{
	QFile f(path);
	if(f.open(QIODevice::ReadOnly)) {
		insertData(tutID, f.readAll());
		f.close();
		return true;
	}

	return false;
}

const QList<QByteArray> &TutFile::dataList() const
{
	return tutos;
}

bool TutFile::isTut(int tutID) const
{
	Q_UNUSED(tutID)
	return true;
}

QString TutFile::parseScripts(int tutID) const
{
	const QByteArray &tuto = data(tutID);
	const char *constTuto = tuto.constData();
	QString ret;
	QByteArray textData;
	bool jp = Config::value("jp_txt", false).toBool();
	quint8 clef;
	int i=0, size = tuto.size(), endOfText;

	while(i < size)
	{
		switch(clef = tuto.at(i++))
		{
		case 0x00:
			if(i + 2 >= size) {
				return ret;
			}
			quint16 u;
			memcpy(&u, constTuto + i, 2);
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

			if(endOfText != -1) {
				textData = tuto.mid(i, endOfText-i);
			} else {
				textData = tuto.mid(i); // FIXME: this can break the bijection
			}

			ret.append(QString("TEXT(%1)")
					   .arg(FF7Text(textData).text(jp)));

			if(endOfText != -1) {
				if(endOfText + 1 > i) {
					i = endOfText + 1;
				}
			} else {
				return ret;
			}
			break;
		case 0x11:	ret.append("{FINISH}");		break;
		case 0x12:
			if(i + 4 >= size) {
				return ret;
			}
			quint16 x, y;
			memcpy(&x, constTuto + i, 2);
			memcpy(&y, constTuto + i+2, 2);
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

	return ret;
}

bool TutFile::parseText(int tutID, const QString &tuto)
{
	if(!isTut(tutID))	return false;

	QStringList lines = tuto.split('\n', QString::SkipEmptyParts), params;
	QByteArray ret;
	bool ok, multilineText = false;
	int value;
	bool jp = Config::value("jp_txt", false).toBool();

	foreach(QString line, lines) {
		if(multilineText) {
			if(line.endsWith(")")) {
				line.chop(1);// remove ")"
				multilineText = false;
			}
			ret.append(FF7Text(line, jp).data());
			if(!multilineText) {
				ret.append('\xff');
			}
		} else if(line.startsWith("MOVE(") && line.endsWith(")")) {
			ret.append('\x12');
			line.chop(1);// remove ")"
			params = line.mid(5).split(',');
			if(params.size()>=2) {
				value = params.first().toInt(&ok);
				if(ok) {
					ret.append((char *)&value, 2);
				} else {
					ret.append("\x00\x00", 2);
					//return false;
				}
				value = params.at(1).toInt(&ok);
				if(ok) {
					ret.append((char *)&value, 2);
				} else {
					ret.append("\x00\x00", 2);
					//return false;
				}
			} else {
				ret.append("\x00\x00\x00\x00", 4);
				//return false;
			}
		} else if(line.startsWith("PAUSE(") && line.endsWith(")")) {
			ret.append('\x00');
			line.chop(1);// remove ")"

			value = line.mid(6).toInt(&ok);
			if(ok) {
				ret.append((char *)&value, 2);
			} else {
				ret.append("\x00\x00", 2);
				//return false;
			}
		} else if(line.startsWith("TEXT(")) {
			if(line.endsWith(")")) {
				line.chop(1);// remove ")"
				multilineText = false;
			} else {
				multilineText = true;
			}

			ret.append('\x10');
			ret.append(FF7Text(line.mid(5), jp).data());

			if(!multilineText) {
				ret.append('\xff');
			}
		}
		else if(line=="{FINISH}") {		ret.append('\x11'); }
		else if(line=="{NOP}") {		ret.append('\xff'); }
		else if(line=="[UP]") {			ret.append('\x02'); }
		else if(line=="[DOWN]") {		ret.append('\x03'); }
		else if(line=="[LEFT]") {		ret.append('\x04'); }
		else if(line=="[RIGHT]") {		ret.append('\x05'); }
		else if(line=="[MENU]") {		ret.append('\x06'); }
		else if(line=="[CANCEL]") {		ret.append('\x07'); }
		else if(line=="[CHANGE]") {		ret.append('\x08'); }
		else if(line=="[OK]") {			ret.append('\x09'); }
		else if(line=="[R1]") {			ret.append('\x0a'); }
		else if(line=="[R2]") {			ret.append('\x0b'); }
		else if(line=="[L1]") {			ret.append('\x0c'); }
		else if(line=="[L2]") {			ret.append('\x0d'); }
		else if(line=="[START]") {		ret.append('\x0e'); }
		else if(line=="[SELECT]") {		ret.append('\x0f'); }
		else if(line.startsWith("[") && line.endsWith("]")) {
			value = line.mid(1,2).toInt(&ok);
			if(ok) {
				ret.append((char)value);
			}
		} else { // Compatibility Makou Reactor <= v1.6
			ret.append('\x10');
			ret.append(FF7Text(line, jp).data()).append('\xff');
		}
	}

	if(tutos.at(tutID) != ret) {
		tutos.replace(tutID, ret);

		setModified(true);
	}
	return true;
}
