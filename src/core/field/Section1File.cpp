/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#include "Section1File.h"
#include "Field.h"
#include "core/Config.h"
#include "core/FF7Font.h"

Section1File::Section1File(Field *field) :
	FieldPart(field), _scale(0), _version(0)
{
}

void Section1File::clear()
{
	_grpScripts.clear();
	_texts.clear();
	_author.clear();

	setOpen(false);
}

void Section1File::initEmpty()
{
	clear();

	_author = "makou";
	_scale = 512;
	_version = 0x0502;

	_texts.append(FF7Text(QObject::tr("Map name"), false));
	_texts.append(FF7Text(QObject::tr("Hello world!"), false));

	QList<Script> scripts;
	QList<Opcode> opcodes;

	opcodes.append(OpcodeMPNAM());
	opcodes.append(OpcodeRET());
	scripts.append(Script(opcodes));

	QSize s = FF7Font::calcSize(_texts.at(1).data());

	opcodes.clear();
	OpcodeWINDOW opcodeWINDOW;
	opcodeWINDOW.targetX = 0;
	opcodeWINDOW.targetY = 0;
	opcodeWINDOW.width = quint16(s.width());
	opcodeWINDOW.height = quint16(s.height());
	opcodeWINDOW.windowID = 0;
	opcodes.append(opcodeWINDOW);
	OpcodeMESSAGE opcodeMESSAGE;
	opcodeMESSAGE.textID = 1;
	opcodeMESSAGE.windowID = 0;
	opcodes.append(opcodeMESSAGE);
	opcodes.append(OpcodeRET());
	scripts.append(Script(opcodes));

	opcodes.clear();
	opcodes.append(OpcodeRET());
	scripts.append(Script(opcodes));

	_grpScripts.append(GrpScript("dic", scripts));

	QMap<quint8, QString> chars;
	chars.insert(0, "cloud");
	chars.insert(2, "tifa");
	chars.insert(8, "cid");

	QMapIterator<quint8, QString> it(chars);
	quint8 modelID = 0;

	while (it.hasNext()) {
		it.next();

		scripts.clear();

		opcodes.clear();
		OpcodeCHAR_ opcodeCHAR;
		opcodeCHAR.object3DID = modelID;
		opcodes.append(opcodeCHAR);
		OpcodePC opcodePC;
		opcodePC.charID = it.key();
		opcodes.append(opcodePC);
		opcodes.append(OpcodeRET());
		scripts.append(Script(opcodes));

		opcodes.clear();
		opcodes.append(OpcodeRET());
		scripts.append(Script(opcodes));

		opcodes.clear();
		opcodes.append(OpcodeRET());
		scripts.append(Script(opcodes));

		_grpScripts.append(GrpScript(it.value(), scripts));

		modelID += 1;
	}
}

bool Section1File::open()
{
	return open(field()->sectionData(Field::Scripts));
}

bool Section1File::open(const QByteArray &data)
{
	quint16 version, posTexts;
	int cur;
	qsizetype dataSize = data.size();
	const char *constData = data.constData();
	bool isDemo;

	if (dataSize < 32) {
		qWarning() << "Section1File::open data too short" << dataSize;
		return false;
	}

	memcpy(&version, constData, 2);

	_version = version;

	isDemo = version == 0x0301; // Check version format

	memcpy(&posTexts, constData + 4, 2); // posTexts (and end of scripts)
	if (quint32(dataSize) < posTexts || posTexts < 32) {
		qWarning() << "Section1File::open out of range posTexts" << posTexts << dataSize;
		return false;
	}

	clear();

	/* ---------- SCRIPTS ---------- */

	quint32 posAKAO = 0;
	quint16 nbAKAO, pos;
	quint8 emptyGrps = 0, nbScripts = quint8(data.at(2));

	// quint8 count3DModel = quint8(data.at(3));
	memcpy(&nbAKAO, constData + 6, 2); // nbAKAO

	if (isDemo) {
		_scale = 0; // FIXME: better value?
		cur = 8;
	} else {
		memcpy(&_scale, constData + 8, 2);
		cur = 16;
	}
	const char *author = constData + cur;
	_author = QString::fromLatin1(author, qsizetype(qstrnlen(author, 8)));
	//QString name2 = data.mid(cur + 8, 8);
	cur += 16;

	int posScripts = cur + 8 * nbScripts + 4 * nbAKAO;

	if (posTexts < posScripts + 64 * nbScripts) {
		qWarning() << "Section1File::open out of range posScripts" << posTexts << posScripts << nbScripts << dataSize;
		return false;
	}

	quint16 positions[33];
	const quint8 scriptCount = isDemo ? 16 : 32;

	if (nbAKAO > 0) {
		//INTERGRITY TEST
		//		QString out;
		//		bool pasok = false;
		//		for (int i=0; i<nbAKAO; ++i) {
		//			memcpy(&posAKAO, &constData[cur+8*nbScripts+i*4], 4);
		//			out.append(QString("%1 %2 %3 (%4)\n").arg(i).arg(posAKAO).arg(QString(data.mid(posAKAO, 4))).arg(QString(data.mid(posAKAO-4, 8).toHex())));
		//			if (data.mid(posAKAO, 4) != "AKAO" && data.at(posAKAO) != '\x12') {
		//				pasok = true;
		//			}
		//		}
		//		if (pasok) {
		//			qDebug() << out;
		//		}

		memcpy(&posAKAO, constData + cur + 8 * nbScripts, 4); // posAKAO
	} else {
		posAKAO = quint32(dataSize);
	}

	// On the Android version, posTexts can be after posAKAO
	quint16 posAfterScripts = quint16(qMin(posAKAO, quint32(posTexts)));

	for (quint8 i = 0; i < nbScripts; ++i) {
		const char *grpName = constData + cur + 8 * i;
		GrpScript grpScript(QString::fromLatin1(grpName, qsizetype(qstrnlen(grpName, 8))));

		if (emptyGrps > 1) {
			emptyGrps--;
		} else {
			// Listing start offsets
			memcpy(positions, constData + posScripts + scriptCount * 2 * i, scriptCount * 2);

			// Add offset at the end
			if (i == nbScripts - 1) {
				positions[scriptCount] = posAfterScripts;
			} else {
				memcpy(&pos, constData + posScripts + scriptCount * 2 * (i + 1), 2);

				if (pos > positions[scriptCount - 1]) {
					positions[scriptCount] = pos;
				} else {
					emptyGrps = 1;
					while (pos <= positions[scriptCount - 1] && i+emptyGrps<nbScripts-1) {
						memcpy(&pos, constData + posScripts + scriptCount * 2 * (i + emptyGrps + 1), 2);
						emptyGrps++;
					}
					if (i + emptyGrps == nbScripts) {
						positions[scriptCount] = posAfterScripts;
					} else {
						positions[scriptCount] = pos;
					}
				}
			}

			quint8 scriptID = 0;
			for (quint8 j = 0; j < scriptCount; ++j) {
				if (positions[j + 1] > positions[j]) {
					Script script(constData + positions[j], positions[j + 1] - positions[j]);
					if (!script.isValid()) {
						qWarning() << "Section1File::open invalid script" << i << j;
						return false;
					}
					if (scriptID == 0) {
						Script scriptMain = script.splitScriptAtReturn();
						grpScript.setScript(0, script); // S0 - Init
						grpScript.setScript(1, scriptMain); // S0 - Main
					} else {
						grpScript.setScript(scriptID + 1, script);
					}
					scriptID = j + 1;
				}
			}
		}
		
		_grpScripts.append(grpScript);
	}

	qsizetype sizeTextSection;

	/* ---------- TEXTS ---------- */
	if (posAKAO >= posTexts) {
		sizeTextSection = posAKAO - posTexts;
	} else {
		sizeTextSection = dataSize - posTexts;
	}

	if (sizeTextSection > 4) { // If there are texts
		quint16 posBeg, posEnd, textCount;
		if (dataSize < posTexts + 2) {
			qWarning() << "Section1File::open invalid posTexts 2" << posTexts << dataSize;
			return false;
		}
		memcpy(&posBeg, constData + posTexts + 2, 2);
		if (posBeg > 0) {
			textCount = posBeg / 2 - 1;

			for (quint32 i = 1; i < textCount; ++i) {
				memcpy(&posEnd, constData + posTexts + 2 + i*2, 2);

				if (dataSize < posTexts + posEnd) {
					qWarning() << "Section1File::open invalid posFin" << posTexts << posEnd << dataSize;
					break;
				}

				// FIXME: possible hidden data between 0xFF and posEnd - posBeg
				_texts.append(FF7Text(data.mid(posTexts + posBeg, posEnd - posBeg)));
				posBeg = posEnd;
			}
			if (dataSize < sizeTextSection) {
				qWarning() << "Section1File::open invalid sizeTextSection" << sizeTextSection << dataSize;
				return false;
			}
			// FIXME: possible hidden data between 0xFF and posEnd - posBeg
			_texts.append(FF7Text(data.mid(posTexts + posBeg, sizeTextSection - posBeg)));
		}
	}

	setOpen(true);

	return true;
}

QByteArray Section1File::save() const
{
	TutFileStandard *tut = field()->tutosAndSounds();
	QByteArray grpScriptNames, positionsScripts, positionsAKAO, allScripts, realScript, positionsTexts, allTexts, allAKAOs;
	quint32 newPosAKAOs, pos32, newPosTexts32;
	quint16 newPosScripts, newPosTexts, newNbAKAO, pos;
	quint8 newNbGrpScripts;

	//nbGrpScripts = (quint8)data.at(2);//nbGrpScripts
	newNbGrpScripts = _grpScripts.size();
	//memcpy(&posTexts, constData + 4, 2);//posTexts (and end of the scripts section)

	newNbAKAO = tut->size(); // 255 maximum

	newPosScripts = 32 + newNbGrpScripts * 72 + newNbAKAO * 4;
	pos32 = newPosScripts;

	// Creation newPosScripts + scripts
	quint8 count3DModel = 0;
	for (const GrpScript &grpScript : _grpScripts) {
		grpScriptNames.append(grpScript.realName().toLatin1().leftJustified(8, '\x00', true));
		for (quint8 j = 0; j < 32; ++j) {
			realScript = grpScript.toByteArray(j);
			if (!realScript.isEmpty()) {
				pos32 = newPosScripts + allScripts.size();
			}
			if (pos32 > 65535) {
				qWarning() << "Section1File::save script size overflow";
				return QByteArray();
			}
			pos = quint16(pos32);
			positionsScripts.append((char *)&pos, 2);
			allScripts.append(realScript);
		}
		if (grpScript.type() == GrpScript::Model) {
			++count3DModel;
		}
	}

	// Creation new positions Texts
	newPosTexts32 = newPosScripts + allScripts.size();
	if (newPosTexts32 > 65535) {
		qWarning() << "Section1File::save script size overflow";
		return QByteArray();
	}
	newPosTexts = quint16(newPosTexts32);

	quint16 newNbText = textCount();

	for (const FF7Text &text : texts()) {
		pos32 = 2 + newNbText * 2 + allTexts.size();
		if (pos32 > 65535) {
			qWarning() << "Section1File::save script + text size overflow";
			return QByteArray();
		}
		pos = quint16(pos32);
		positionsTexts.append((char *)&pos, 2);
		allTexts.append(text.data());
		allTexts.append('\xff');// end of text
	}

	// Word padding
	int scriptsAndTextsSize = positionsScripts.size() + allScripts.size() + 2 + positionsTexts.size() + allTexts.size();
	if (scriptsAndTextsSize % 4 != 0 && tut->size() > 0) {
		allTexts.append(QByteArray(4 - scriptsAndTextsSize % 4, '\0'));
	}

	newPosAKAOs = newPosTexts + (2 + newNbText*2 + allTexts.size());

	allAKAOs = tut->save(positionsAKAO, newPosAKAOs);

	QByteArray mapauthor = _author.toLatin1().leftJustified(8, '\0', true);
	mapauthor[7] = '\0';
	QByteArray mapname = field()->name().toLatin1().leftJustified(8, '\0', true);
	mapname[7] = '\0';

	return QByteArray()
	    .append((char *)&_version, 2) // Version
	    .append(char(newNbGrpScripts)) // nbGrpScripts
	    .append(char(count3DModel)) // nb3DObjects
	    .append((char *)&newPosTexts, 2) // PosTexts
	    .append((char *)&newNbAKAO, 2) // AKAO count
	    .append((char *)&_scale, 2)
	    .append(_empty.leftJustified(6, '\0', true)) // Empty
	    .append(mapauthor) // mapAuthor
	    .append(mapname) // mapName
	    .append(grpScriptNames) // Names of grpScripts
	    .append(positionsAKAO) // PosAKAO
	    .append(positionsScripts) // PosScripts
	    .append(allScripts) // Scripts
	    .append((char *)&newNbText, 2) // nbTexts
	    .append(positionsTexts) // positionsTexts
	    .append(allTexts) // Texts
	    .append(allAKAOs); // AKAO / tutos
}

bool Section1File::exporter(QIODevice *device, ExportFormat format)
{
	bool jp = Config::value("jp_txt", false).toBool();

	switch (format) {
	case TXTText: {
		if (!device->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
			return false;
		}
		int i=0;
		for (const FF7Text &text : texts()) {
			device->write(QString("---TEXT%1---\n%2\n")
						  .arg(i++, 3, 10, QChar('0'))
						  .arg(text.text(jp))
						  .toUtf8());
		}
		device->close();
		return true;
	}
	case XMLText: {
		if (!device->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			return false;
		}
		QXmlStreamWriter stream(device);
		stream.setAutoFormatting(true);
		stream.writeStartDocument();
		stream.writeStartElement("field");
		stream.writeAttribute("name", field()->name());
		stream.writeStartElement("texts");
		int id=0;
		for (const FF7Text &text : texts()) {
			stream.writeStartElement("text");
			stream.writeAttribute("id", QString::number(id));
			stream.writeCharacters(text.text(jp));
			stream.writeEndElement(); // /text
			++id;
		}

		stream.writeEndElement(); // /texts
		stream.writeEndElement(); // /field
		stream.writeEndDocument();
		device->close();
		return true;
	}
	}

	return false;
}

bool Section1File::importer(QIODevice *device, ExportFormat format)
{
	Q_UNUSED(format)
	//TODO
	// bool jp = Config::value("jp_txt", false).toBool();
	bool start = false, field = false, texts = false;

	QXmlStreamReader stream(device);

	while (!stream.atEnd()) {
		QXmlStreamReader::TokenType type = stream.readNext();
		if (!start && type == QXmlStreamReader::StartDocument) {
			start = true;
		} else if (start && !field && type == QXmlStreamReader::StartElement
				  && stream.name() == QLatin1String("field")) {
			field = true;
		} else if (field && !texts && type == QXmlStreamReader::StartElement
				  && stream.name() == QLatin1String("texts")) {
			texts = true;
		} else if (texts && type == QXmlStreamReader::StartElement
				  && stream.name() == QLatin1String("text")) {
//			stream.attributes().value("id");
		}
	}

	return stream.hasError();
}

bool Section1File::isModified() const
{
	TutFileStandard *tut = field()->tutosAndSounds();
	return FieldPart::isModified() || (tut && tut->isModified());
}

int Section1File::modelID(quint8 grpScriptID) const
{
	if (_grpScripts.at(grpScriptID).type() != GrpScript::Model) {
		return -1;
	}

	int ID = 0;

	for (int i = 0; i < grpScriptID; ++i) {
		if (_grpScripts.at(i).type() == GrpScript::Model) {
			++ID;
		}
	}

	return ID;
}

void Section1File::bgParamAndBgMove(QHash<quint8, quint8> &paramActifs, qint16 *z, qint16 *x, qint16 *y) const
{
	for (const GrpScript &grpScript : _grpScripts) {
		grpScript.backgroundParams(paramActifs);
		if (z) {
			grpScript.backgroundMove(z, x, y);
		}
	}
}

const QList<GrpScript> &Section1File::grpScripts() const
{
	return _grpScripts;
}

const GrpScript &Section1File::grpScript(int groupID) const
{
	return _grpScripts.at(groupID);
}

GrpScript &Section1File::grpScript(int groupID)
{
	return _grpScripts[groupID];
}

qsizetype Section1File::grpScriptCount() const
{
	return _grpScripts.size();
}

bool Section1File::insertGrpScript(int row, const GrpScript &grpScript)
{
	if (grpScriptCount() < maxGrpScriptCount()) {
		_grpScripts.insert(row, grpScript);
		for (GrpScript &grpScript : _grpScripts) {
			grpScript.shiftGroupIds(row - 1, +1);
		}
		setModified(true);
		return true;
	}
	return false;
}

void Section1File::removeGrpScript(int row)
{
	if (row < _grpScripts.size()) {
		_grpScripts.removeAt(row);
		for (GrpScript &grpScript : _grpScripts) {
			grpScript.shiftGroupIds(row, -1);
		}
		setModified(true);
	}
}

bool Section1File::moveGrpScript(int row, bool direction)
{
	if (row >= _grpScripts.size()) {
		return false;
	}

	if (direction) { // down
		if (row == _grpScripts.size() - 1) {
			return false;
		}
		_grpScripts.swapItemsAt(row, row + 1);
		for (GrpScript &grpScript : _grpScripts) {
			grpScript.swapGroupIds(row, row + 1);
		}
	} else { // up
		if (row == 0) {
			return false;
		}
		_grpScripts.swapItemsAt(row, row - 1);
		for (GrpScript &grpScript : _grpScripts) {
			grpScript.swapGroupIds(row, row - 1);
		}
	}
	setModified(true);

	return true;
}

void Section1File::searchAllVars(QList<FF7Var> &vars) const
{
	for (const GrpScript &group : _grpScripts) {
		group.searchAllVars(vars);
	}
}

bool Section1File::searchOpcode(int opcode, int &groupID, int &scriptID, int &opcodeID) const
{
	if (groupID < 0) {
		groupID = scriptID = opcodeID = 0;
	}
	if (groupID >= _grpScripts.size()) {
		return false;
	}
	if (_grpScripts.at(groupID).searchOpcode(opcode, scriptID, opcodeID)) {
		return true;
	}

	scriptID = 0;
	opcodeID = 0;
	return searchOpcode(opcode, ++groupID, scriptID, opcodeID);
}

bool Section1File::searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &groupID, int &scriptID, int &opcodeID) const
{
	if (groupID < 0) {
		groupID = scriptID = opcodeID = 0;
	}
	if (groupID >= _grpScripts.size()) {
		return false;
	}
	if (_grpScripts.at(groupID).searchVar(bank, address, op, value, scriptID, opcodeID)) {
		return true;
	}

	scriptID = 0;
	opcodeID = 0;
	return searchVar(bank, address, op, value, ++groupID, scriptID , opcodeID);
}

bool Section1File::searchExec(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const
{
	if (groupID < 0) {
		groupID = scriptID = opcodeID = 0;
	}
	if (groupID >= _grpScripts.size()) {
		return false;
	}
	if (_grpScripts.at(groupID).searchExec(group, script, scriptID, opcodeID)) {
		return true;
	}

	scriptID = 0;
	opcodeID = 0;
	return searchExec(group, script, ++groupID, scriptID, opcodeID);
}

bool Section1File::searchMapJump(quint16 map, int &groupID, int &scriptID, int &opcodeID) const
{
	if (groupID < 0) {
		groupID = scriptID = opcodeID = 0;
	}
	if (groupID >= _grpScripts.size()) {
		return false;
	}
	if (_grpScripts.at(groupID).searchMapJump(map, scriptID, opcodeID)) {
		return true;
	}

	scriptID = 0;
	opcodeID = 0;
	return searchMapJump(map, ++groupID, scriptID, opcodeID);
}

bool Section1File::searchTextInScripts(const QRegularExpression &text, int &groupID, int &scriptID, int &opcodeID) const
{
	if (groupID < 0) {
		groupID = scriptID = opcodeID = 0;
	}
	if (groupID >= _grpScripts.size()) {
		return false;
	}
	if (_grpScripts.at(groupID).searchTextInScripts(text, scriptID, opcodeID, this)) {
		return true;
	}

	scriptID = 0;
	opcodeID = 0;
	return searchTextInScripts(text, ++groupID, scriptID, opcodeID);
}

bool Section1File::searchText(const QRegularExpression &text, int &textID, qsizetype &from, qsizetype &size) const
{
	if (textID < 0) {
		textID = 0;
	}
	if (textID >= textCount()) {
		return false;
	}
	from = this->text(textID).indexOf(text, from, size);
	if (from != -1) {
		return true;
	}

	from = 0;
	return searchText(text, ++textID, from, size);
}

bool Section1File::searchOpcodeP(int opcode, int &groupID, int &scriptID, int &opcodeID) const
{
	if (groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size() - 1;
		scriptID = opcodeID = 2147483647;
	}
	if (groupID < 0) {
		return false;
	}
	if (_grpScripts.at(groupID).searchOpcodeP(opcode, scriptID, opcodeID)) {
		return true;
	}

	groupID -= 1;
	scriptID = 2147483647;
	opcodeID = 2147483647;
	return searchOpcodeP(opcode, groupID, scriptID, opcodeID);
}

bool Section1File::searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &groupID, int &scriptID, int &opcodeID) const
{
	if (groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size() - 1;
		scriptID = opcodeID = 2147483647;
	}
	if (groupID < 0) {
		return false;
	}
	if (_grpScripts.at(groupID).searchVarP(bank, address, op, value, scriptID, opcodeID)) {
		return true;
	}

	groupID -= 1;
	scriptID = 2147483647;
	opcodeID = 2147483647;
	return searchVarP(bank, address, op, value, groupID, scriptID, opcodeID);
}

bool Section1File::searchExecP(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const
{
	if (groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size() - 1;
		scriptID = opcodeID = 2147483647;
	}
	if (groupID < 0) {
		return false;
	}
	if (_grpScripts.at(groupID).searchExecP(group, script, scriptID, opcodeID)) {
		return true;
	}

	groupID -= 1;
	scriptID = 2147483647;
	opcodeID = 2147483647;
	return searchExecP(group, script, groupID, scriptID, opcodeID);
}

bool Section1File::searchMapJumpP(quint16 map, int &groupID, int &scriptID, int &opcodeID) const
{
	if (groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size() - 1;
		scriptID = opcodeID = 2147483647;
	}
	if (groupID < 0) {
		return false;
	}
	if (_grpScripts.at(groupID).searchMapJumpP(map, scriptID, opcodeID)) {
		return true;
	}

	groupID -= 1;
	scriptID = 2147483647;
	opcodeID = 2147483647;
	return searchMapJumpP(map, groupID, scriptID, opcodeID);
}

bool Section1File::searchTextInScriptsP(const QRegularExpression &text, int &groupID, int &scriptID, int &opcodeID) const
{
	if (groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size() - 1;
		scriptID = opcodeID = 2147483647;
	}
	if (groupID < 0) {
		return false;
	}
	if (_grpScripts.at(groupID).searchTextInScriptsP(text, scriptID, opcodeID, this)) {
		return true;
	}

	groupID -= 1;
	scriptID = 2147483647;
	opcodeID = 2147483647;
	return searchTextInScriptsP(text, groupID, scriptID, opcodeID);
}

bool Section1File::searchTextP(const QRegularExpression &text, int &textID, qsizetype &from, qsizetype &index, qsizetype &size) const
{
	if (textID >= textCount()) {
		textID = textCount()-1;
		from = -1;
	}
	if (textID < 0) {
		return false;
	}
	index = this->text(textID).lastIndexOf(text, from, size);
	if (index != -1) {
		return true;
	}

	textID -= 1;
	from = -1;
	return searchTextP(text, textID, from, index, size);
}

bool Section1File::replaceText(const QRegularExpression &search, const QString &after, int textID, int from)
{
	bool jp = Config::value("jp_txt", false).toBool();
	FF7Text beforeT = text(textID);
	QString before = beforeT.text(jp);
	QRegularExpressionMatch match = search.match(before, from);

	if (match.capturedStart() == from) {
		before.replace(from, match.capturedLength(), after);
		setText(textID, FF7Text(before, jp));
		return true;
	}
	return false;
}

void Section1File::setWindow(const FF7Window &win)
{
	if (win.groupID < _grpScripts.size()) {
		_grpScripts[win.groupID].setWindow(win);
		setModified(true);
	}
}

void Section1File::listWindows(QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const
{
	int groupID = 0;
	for (const GrpScript &group : _grpScripts) {
		group.listWindows(groupID++, windows, text2win);
	}
}

void Section1File::listWindows(int textID, QList<FF7Window> &windows) const
{
	int groupID = 0;
	for (const GrpScript &group : _grpScripts) {
		group.listWindows(groupID++, textID, windows);
	}
}

void Section1File::listModelPositions(QMultiMap<int, FF7Position> &positions) const
{
	int modelId = 0;
	for (const GrpScript &group : _grpScripts) {
		if (group.type() == GrpScript::Model) {
			QList<FF7Position> pos;
			group.listModelPositions(pos);
			if (!pos.isEmpty()) {
				for (const FF7Position &position : qAsConst(pos)) {
					positions.insert(modelId, position);
				}
			}
			++modelId;
		}
	}
}

int Section1File::modelCount() const
{
	int modelId = 0;
	for (const GrpScript &group : grpScripts()) {
		if (group.type() == GrpScript::Model) {
			modelId++;
		}
	}

	return modelId;
}

void Section1File::linePosition(QMap<int, std::pair<FF7Position, FF7Position>> &positions) const
{
	int groupID = 0;
	for (const GrpScript &group : _grpScripts) {
		if (group.type() == GrpScript::Location) {
			FF7Position position[2] = { FF7Position(), FF7Position() };
			if (group.linePosition(position)) {
				positions.insert(groupID, std::pair<FF7Position, FF7Position>(position[0], position[1]));
			}
		}
		++groupID;
	}
}

bool Section1File::compileScripts(int &groupID, int &scriptID, int &opcodeID, QString &errorStr)
{
	groupID = 0;
	for (GrpScript &group : _grpScripts) {
		if (!group.compile(scriptID, opcodeID, errorStr)) {
			return false;
		}
		++groupID;
	}

	return true;
}

void Section1File::removeTexts()
{
	for (GrpScript &group : _grpScripts) {
		if (group.removeTexts()) {
			setModified(true);
		}
	}
}

void Section1File::cleanTexts()
{
	QSet<quint8> usedTexts = listUsedTexts();

	for (int textID = 0; textID < _texts.size(); ++textID) {
		if (!usedTexts.contains(quint8(textID))) {
			_texts[textID] = FF7Text();
			setModified(true);
		}
	}
}

void Section1File::autosizeTextWindows()
{
	QSet<quint8> textIDs = listUsedTexts();
	for (quint8 textID : textIDs) {
		if (textID >= _texts.size()) {
			continue;
		}
		QList<FF7Window> windows;
		listWindows(textID, windows);
		if (!windows.isEmpty()) {
			QSize size = FF7Font::calcSize(text(textID).data());
			for (FF7Window win : qAsConst(windows)) {
				if (win.displayType > 0) {
					continue; // TODO: estimate size for countdown and numerical display
				}
				bool isModified;
				isModified = win.w != size.width() || win.h != size.height();
				win.w = quint16(size.width());
				win.h = quint16(size.height());
				QPoint pos = win.realPos();
				isModified = isModified || win.x != pos.x() || win.y != pos.y();
				win.x = qint16(pos.x());
				win.y = qint16(pos.y());

				setWindow(win);
			}
		}
	}
}

//void Section1File::searchWindows() const
//{
//	int groupID=0;
//	for (const GrpScript &group : _grpScripts) {
//		const QList<Script> &scripts = group.scripts();
//		if (!scripts.isEmpty()) {
//			scripts.at(0)->searchWindows();
//			if (scripts.size() > 0) {
//				scripts.at(1)->searchWindows();

//				if (group.type() == GrpScript::Model) {
//					if (scripts.size() > 1) {
//						scripts.at(2)->searchWindows(); // talk
//					}
//					if (scripts.size() > 2) {
//						scripts.at(3)->searchWindows(); // touch
//					}
//				} else if (group.type() == GrpScript::Location) {
//					if (scripts.size() > 1) {
//						scripts.at(2)->searchWindows(); // talk
//					}
//					if (scripts.size() > 2) {
//						scripts.at(3)->searchWindows(); // touch
//					}
//					if (scripts.size() > 3) {
//						scripts.at(4)->searchWindows(); // move
//					}
//					if (scripts.size() > 4) {
//						scripts.at(5)->searchWindows(); // go
//					}
//					if (scripts.size() > 5) {
//						scripts.at(6)->searchWindows(); // go1
//					}
//					if (scripts.size() > 6) {
//						scripts.at(7)->searchWindows(); // leave
//					}
//				}

//			}
//		}
//	}
//		group.listWindows(groupID++, windows, text2win);
//}

const QList<FF7Text> &Section1File::texts() const
{
	return _texts;
}

qsizetype Section1File::textCount() const
{
	return _texts.size();
}

const FF7Text &Section1File::text(int textID) const
{
	return _texts.at(textID);
}

void Section1File::setText(int textID, const FF7Text &text)
{
	if (textID >= 0 && textID < _texts.size()) {
		_texts.replace(textID, text);
		setModified(true);
	}
}

bool Section1File::insertText(int textID, const FF7Text &text)
{
	if (textCount() < maxTextCount()) {
		_texts.insert(textID, text);
		for (GrpScript &grpScript : _grpScripts) {
			grpScript.shiftTextIds(textID - 1, +1);
		}
		setModified(true);
		return true;
	}
	return false;
}

void Section1File::deleteText(int textID)
{
	if (textID >= 0 && textID < _texts.size()) {
		_texts.removeAt(textID);
		for (GrpScript &grpScript : _grpScripts) {
			grpScript.shiftTextIds(textID, -1);
		}
		setModified(true);
	}
}

void Section1File::clearTexts()
{
	if (!_texts.isEmpty()) {
		_texts.clear();
		setModified(true);
	}
}

QSet<quint8> Section1File::listUsedTexts() const
{
	QSet<quint8> usedTexts;
	for (const GrpScript &grpScript : _grpScripts) {
		grpScript.listUsedTexts(usedTexts);
	}
	return usedTexts;
}

void Section1File::shiftTutIds(int row, int shift)
{
	for (GrpScript &grpScript : _grpScripts) {
		grpScript.shiftTutIds(row, shift);
	}
	setModified(true);
}

QSet<quint8> Section1File::listUsedTuts() const
{
	QSet<quint8> usedTuts;
	for (const GrpScript &grpScript : _grpScripts) {
		grpScript.listUsedTuts(usedTuts);
	}
	return usedTuts;
}

const QString &Section1File::author() const
{
	return _author;
}

void Section1File::setAuthor(const QString &author)
{
	_author = author;
	setModified(true);
}

quint16 Section1File::scale() const
{
	return _scale;
}

void Section1File::setScale(quint16 scale)
{
	_scale = scale;
	setModified(true);
}

qsizetype Section1File::availableBytesForScripts() const
{
	TutFileStandard *tut = field()->tutosAndSounds();
	int AKAOCount = tut && tut->isOpen() ? tut->size() : 0; // TODO: opens tut
	return 65535 - (32 + grpScriptCount() * 72 + AKAOCount * 4);
}
