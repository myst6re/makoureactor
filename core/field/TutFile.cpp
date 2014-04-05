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

TutFile::TutFile(const QList<QByteArray> &tutos) :
	FieldPart(0), tutos(tutos)
{
	setOpen(true);
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

void TutFile::removeTut(int tutID)
{
	tutos.removeAt(tutID);
	setModified(true);
}

bool TutFile::insertTut(int tutID)
{
	return insertData(tutID, QByteArray("\x11", 1));
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

QString TutFile::parseScripts(int tutID) const
{
	const QByteArray &tuto = data(tutID);
	const char *constTuto = tuto.constData();
	QString ret;
	QByteArray textData;
	bool jp = Config::value("jp_txt", false).toBool();
	quint8 key;
	int i=0, size = tuto.size(), endOfText;

	while(i < size) {
		switch(key = tuto.at(i++)) {
		case 0x00:
			if(i + 2 > size) {
				ret.append(parseScriptsUnknownString(key));
				ret.append("\n");
				if(i < size) {
					ret.append(parseScriptsUnknownString(tuto.at(i)));
					ret.append("\n");
				}
				return ret;
			}
			quint16 u;
			memcpy(&u, constTuto + i, 2);
			ret.append(QString("PAUSE(%1)").arg(u));
			i += 2;
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

			ret.append(QString("TEXT(\"%1\")")
					   .arg(FF7Text(textData).text(jp).replace('"', "\\\"")));

			if(endOfText != -1) {
				if(endOfText + 1 > i) {
					i = endOfText + 1;
				}
			} else {
				ret.append("\n");
				return ret;
			}
			break;
		case 0x11:	ret.append("{FINISH}");		break;
		case 0x12:
			if(i + 4 > size) {
				ret.append(parseScriptsUnknownString(key));
				ret.append("\n");
				if(i < size) {
					ret.append(parseScriptsUnknownString(tuto.at(i++)));
					ret.append("\n");
					if(i < size) {
						ret.append(parseScriptsUnknownString(tuto.at(i++)));
						ret.append("\n");
						if(i < size) {
							ret.append(parseScriptsUnknownString(tuto.at(i)));
							ret.append("\n");
						}
					}
				}
				return ret;
			}
			quint16 x, y;
			memcpy(&x, constTuto + i, 2);
			memcpy(&y, constTuto + i + 2, 2);
			ret.append(QString("MOVE(%1,%2)").arg(x).arg(y));
			i += 4;
			break;
		case 0xff:
			ret.append("{NOP}");
			break;
		default:
			ret.append(parseScriptsUnknownString(key));
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
	ushort value;
	bool jp = Config::value("jp_txt", false).toBool();

	foreach(QString line, lines) {
		QString rawLine = line;
		line = line.trimmed();
		if(multilineText) {
			if(rawLine.endsWith("\")") && !rawLine.endsWith("\\\")")) {
				rawLine.chop(2);// remove '")'
				multilineText = false;
			} else {
				rawLine.append("\n");
			}
			ret.append(FF7Text(rawLine, jp).data());
			if(!multilineText) {
				ret.append('\xff');
			}
		} else if(line.startsWith("MOVE(") && line.endsWith(")")) {
			ret.append('\x12');
			line.chop(1);// remove ")"
			params = line.mid(5).split(',');

			if(!params.isEmpty()) {
				value = params.first().trimmed().toUShort();
				ret.append((char *)&value, 2);

				if(params.size() >= 2) {
					value = params.at(1).trimmed().toUShort();
					ret.append((char *)&value, 2);
				} else {
					// Repeat first value
					ret.append((char *)&value, 2);
				}
			} else {
				ret.append("\x00\x00\x00\x00", 4);
			}
		} else if(line.startsWith("PAUSE(") && line.endsWith(")")) {
			ret.append('\x00');
			line.chop(1);// remove ")"

			value = line.mid(6).trimmed().toUShort();
			ret.append((char *)&value, 2);
		} else if(line.startsWith("TEXT(\"")) {
			if(line.endsWith("\")") && !line.endsWith("\\\")")) {
				line.chop(2);// remove '")'
				multilineText = false;
			} else {
				multilineText = true;
				line.append("\n");
			}

			ret.append('\x10');
			ret.append(FF7Text(line.mid(6).replace("\\\"", "\""), jp).data());

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
			value = line.mid(1, line.size() - 2).trimmed().toUShort(&ok, 16);
			if(ok) {
				if(value > 255) {
					value = 255;
				}
				ret.append((char)value);
			}
		} else { // Compatibility Makou Reactor <= v1.6
			ret.append('\x10');
			ret.append(FF7Text(rawLine, jp).data()).append('\xff');
		}
	}

	if(tutos.at(tutID) != ret) {
		tutos.replace(tutID, ret);

		setModified(true);
	}
	return true;
}

#include "TutFileStandard.h"

void TutFile::testParsing()
{
	// Pause
	QByteArray pauseTooShort("\x00\x01", 2);QString pauseTooShortResult = "[00]\n[01]\n";
	QByteArray pauseAlone("\x00\x01\x02", 3);QString pauseAloneResult = "PAUSE(513)\n";
	QByteArray pauseAndNop("\x00\x01\x02\xFF", 4);QString pauseAndNopResult = "PAUSE(513)\n{NOP}\n";
	QByteArray pauseBigValues("\x00\xFF\xFF", 3);QString pauseBigValuesResult = "PAUSE(65535)\n";
	// Keys
	QByteArray keys("\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F", 14);
	QString keysResult = "[UP]\n[DOWN]\n[LEFT]\n[RIGHT]\n[MENU]\n[CANCEL]\n[CHANGE]\n[OK]\n[R1]\n[R2]\n[L1]\n[L2]\n[START]\n[SELECT]\n";
	// Move
	QByteArray moveTooShort1("\x12\x01", 2);QString moveTooShort1Result = "[12]\n[01]\n";
	QByteArray moveTooShort2("\x12\x01\x02", 3);QString moveTooShort2Result = "[12]\n[01]\n[02]\n";
	QByteArray moveTooShort3("\x12\x01\x02\x03", 4);QString moveTooShort3Result = "[12]\n[01]\n[02]\n[03]\n";
	QByteArray moveAlone("\x12\x01\x02\x03\x04", 5);QString moveAloneResult = "MOVE(513,1027)\n";
	QByteArray moveAndNop("\x12\x01\x02\x03\x04\xFF", 6);QString moveAndNopResult = "MOVE(513,1027)\n{NOP}\n";
	QByteArray moveBigValues("\x12\xFF\xFF\xFF\xFF", 5);QString moveBigValuesResult = "MOVE(65535,65535)\n";
	// Text
	QByteArray textTooShort1("\x10", 1);QString textTooShort1Result("TEXT(\"\")\n");
	QByteArray textTooShort2("\x10", 1);textTooShort2 += FF7Text("A", false).data();QString textTooShort2Result("TEXT(\"A\")\n");
	QByteArray textTooShort3("\x10", 1);textTooShort3 += FF7Text("A", false).data() + QByteArray("\xFF");QString textTooShort3Result("TEXT(\"A\")\n");
	QByteArray textWithQuotes1("\x10", 1);textWithQuotes1 += FF7Text("A\"B", false).data() + QByteArray("\xFF");QString textWithQuotes1Result("TEXT(\"A\\\"B\")\n");
	QByteArray textWithQuotes2("\x10", 1);textWithQuotes2 += FF7Text("A\\\"B", false).data() + QByteArray("\xFF");QString textWithQuotes2Result("TEXT(\"A\\\\\"B\")\n");
	QByteArray textMultiLine1("\x10", 1);textMultiLine1 += FF7Text("A\nB", false).data() + QByteArray("\xFF");QString textMultiLine1Result("TEXT(\"A\nB\")\n");

	QList<QByteArray> tutos;
	tutos << pauseTooShort << pauseAlone <<
			 pauseAndNop << pauseBigValues <<
			 keys <<
			 moveTooShort1 << moveTooShort2 <<
			 moveTooShort3 << moveAlone <<
			 moveAndNop << moveBigValues <<
			 textTooShort1 << textTooShort2 <<
			 textTooShort3 << textWithQuotes1 <<
			 textWithQuotes2 << textMultiLine1;
	QList<QString> strings;
	strings << pauseTooShortResult << pauseAloneResult <<
			   pauseAndNopResult << pauseBigValuesResult <<
			   keysResult <<
			   moveTooShort1Result << moveTooShort2Result <<
			   moveTooShort3Result << moveAloneResult <<
			   moveAndNopResult << moveBigValuesResult <<
			   textTooShort1Result << textTooShort2Result <<
			   textTooShort3Result << textWithQuotes1Result <<
			   textWithQuotes2Result << textMultiLine1Result;

	TutFileStandard tut(tutos);
	qWarning() << "TutFile::testParsing Data to text tests";
	for(int i=0 ; i<tut.size() ; ++i) {
		if(strings.at(i) != tut.parseScripts(i)) {
			qWarning() << i << tut.parseScripts(i) << "<>" << strings.at(i) << tutos.at(i).toHex();
		} else {
			qWarning() << i << "OK";
		}
	}
	qWarning() << "TutFile::testParsing Text to data tests";
	for(int i=0 ; i<tut.size() ; ++i) {
		if(tut.parseText(i, strings.at(i)) && tutos.at(i) != tut.data(i)) {
			qWarning() << i << tutos.at(i).toHex() << "<>" << tut.data(i).toHex() << strings.at(i);
		} else {
			qWarning() << i << "OK";
		}
	}
}
