/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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
#include "Script.h"
#include "Section1File.h"

Script::Script() :
	valid(true)
{
}

Script::Script(const QList<Opcode> &opcodes) :
	_opcodes(opcodes), valid(true)
{
}

Script::Script(const char *script, qsizetype size)
{
	valid = openScript(script, size);
}

bool Script::openScript(const char *script, qsizetype scriptSize)
{
	qsizetype pos = 0, opcodeID = 0;
	QList<qsizetype> positions;
	QMultiMap<qsizetype, qsizetype> labelPositions;

	// Collect label positions
	while (pos < scriptSize) {
		Opcode op(script + pos, scriptSize - pos);
		_opcodes.append(op);
		positions.append(pos);
		if (op.isJump()) {
			labelPositions.insert(pos + op.jump(), opcodeID);
		}
		pos += op.size();
		++opcodeID;
	}
	positions.append(pos);

	QList<qsizetype> labelPositionsKeys = labelPositions.uniqueKeys();

	// Rely labels to jump opcodes
	quint16 labelNumber = 1;
	for (qsizetype jump: labelPositionsKeys) {
		qsizetype index = positions.indexOf(jump);
		const auto IDS = labelPositions.values(jump);
		for (qsizetype opcodeID : IDS) {
			_opcodes[opcodeID].setLabel(labelNumber);
			_opcodes[opcodeID].setBadJump(
			            index < 0
			            ? (jump < 0
			               ? BadJumpError::BeforeScript
			               : (jump > 65535 ? BadJumpError::AfterScript : BadJumpError::InsideInstruction))
			            : BadJumpError::Ok);
			if (_opcodes[opcodeID].badJump() != BadJumpError::Ok) {
				qWarning() << "Script::openScript" << "bad jump" << _opcodes[opcodeID].badJump();
			}
		}
		labelNumber += 1;
	}

	// Insert labels
	for (qsizetype i = labelPositionsKeys.size() - 1; i >= 0; --i) {
		qsizetype jump = labelPositionsKeys.at(i);
		qsizetype index = positions.indexOf(jump);
		if (index > -1) {
			OpcodeLABEL label;
			label._label = quint16(i) + 1;
			_opcodes.insert(index, label);
		}
	}

	return true;
}

Script Script::splitScriptAtReturn()
{
	int gotoLabel = -1;
	int opcodeID = 0;

	for (const Opcode &opcode : std::as_const(_opcodes)) {
		if (opcode.id() == OpcodeKey::LABEL) {
			if (gotoLabel != -1 && opcode.op().opcodeLABEL._label == quint32(gotoLabel)) {
				gotoLabel = -1;
			}
		} else if (gotoLabel == -1) {
			if (opcode.isJump() && !opcode.isBackJump()) {
				gotoLabel = opcode.label();
			} else if (opcode.id() == OpcodeKey::RET || opcode.id() == OpcodeKey::RETTO) {
				++opcodeID;
				break;
			}
		}
		++opcodeID;
	}

	Script s(_opcodes.mid(opcodeID));
	qsizetype size = _opcodes.size();
	for ( ; opcodeID < size; ++opcodeID) {
		_opcodes.removeLast();
	}

	return s;
}

qsizetype Script::size() const
{
	return _opcodes.size();
}

bool Script::isEmpty() const
{
	return _opcodes.isEmpty();
}

bool Script::isValid() const
{
	return valid;
}

Opcode &Script::opcode(qsizetype opcodeID)
{
	return _opcodes[opcodeID];
}

const Opcode &Script::opcode(qsizetype opcodeID) const
{
	return _opcodes.at(opcodeID);
}

QList<Opcode> &Script::opcodes()
{
	return _opcodes;
}

const QList<Opcode> &Script::opcodes() const
{
	return _opcodes;
}

bool Script::compile(int &opcodeID, QString &errorStr)
{
	qint32 pos = 0;
	QHash<quint16, qint32> labelPositions; // Each label is unique

	// Search labels
	opcodeID = 0;
	for (const Opcode &opcode : std::as_const(_opcodes)) {
		if (opcode.id() == OpcodeKey::LABEL) {
			if (!labelPositions.contains(opcode.op().opcodeLABEL._label)) {
				labelPositions.insert(opcode.op().opcodeLABEL._label, pos);
			} else {
				errorStr = QObject::tr("Label %1 is declared several times.")
				           .arg(opcode.op().opcodeLABEL._label);
				return false;
			}
		} else {
			pos += opcode.size();
		}

		++opcodeID;
	}

	if (!labelPositions.isEmpty()) {
		qDebug() << "compile" << labelPositions;
	}

	// Convert jump opcodes
	bool wasSizeModified = false;
	do {
		wasSizeModified = false;
		pos = 0;
		opcodeID = 0;
		for (Opcode &opcode : _opcodes) {
			if (opcode.id() == OpcodeKey::LABEL) {
				if (labelPositions.value(opcode.op().opcodeLABEL._label) != pos) {
					qDebug() << "compile update position label" << opcode.op().opcodeLABEL._label << pos;
				}
				// Update position, in case we converted an opcode to long
				labelPositions.insert(opcode.op().opcodeLABEL._label, pos);
			} else {
				if (opcode.isJump()) {
					quint16 label = quint16(opcode.label());
					if (labelPositions.contains(label)) {
						qint32 jump = labelPositions.value(label) - pos;
						quint8 oldSize = opcode.size();
						qDebug() << "compile set jump" << "label" << label << "old jump" << opcode.jump() << "new jump" << jump << "old size" << oldSize << "pos" << pos;

						if (jump != opcode.jump() && opcode.setJump(jump)) { // Can convert to long jump, which change the size
							// size of opcode modified!
							wasSizeModified = oldSize != opcode.size();
							qDebug() << "wasSizeModified" << wasSizeModified << "new size" << opcode.size();
						}
					} else {
						errorStr = QObject::tr("Label %1 not found.")
								   .arg(label);
						return false;
					}
					
				}

				pos += opcode.size();
			}

			++opcodeID;
		}
		if (wasSizeModified) {
			qDebug() << "here we go again";
		}
	} while (wasSizeModified); // Retry to check if jump are well dimensioned until no changes happen

	// Look for jump errors
	pos = 0;
	opcodeID = 0;
	for (Opcode &opcode : _opcodes) {
		if (opcode.isJump()) {
			switch (opcode.badJump()) {
			case BadJumpError::Ok:
				break;
			case BadJumpError::InsideInstruction:
				errorStr = QObject::tr("Invalid jump, you must point to a valid label.");
				return false;
			case BadJumpError::ImpossibleBackward:
				errorStr = QObject::tr("The label %1 is unreachable because it is located before the opcode.").arg(opcode.label());
				return false;
			case BadJumpError::ImpossibleLong:
				errorStr = QObject::tr("Label %1 is unreachable, please bring this instruction closer.").arg(opcode.label());
				return false;
			case BadJumpError::AfterScript:
				errorStr = QObject::tr("Invalid jump to the label %1 which is after the maximum possible size of the script.").arg(opcode.label());
				return false;
			case BadJumpError::BeforeScript:
				errorStr = QObject::tr("Invalid jump to before the script.");
				return false;
			}
		}
		pos += opcode.size();
		++opcodeID;
	}

	if (pos > 65535) {
		errorStr = QObject::tr("Script too big, it should not exceed 65535 bytes. Actual size: %1.").arg(pos);
		return false;
	}

	return true;
}

QByteArray Script::toByteArray() const
{
	QByteArray ret;

	for (const Opcode &opcode : _opcodes) {
		ret.append(opcode.toByteArray());
	}

	return ret;
}

bool Script::isVoid() const
{
	for (const Opcode &opcode : _opcodes) {
		if (!opcode.isVoid()) {
			return false;
		}
	}
	return true;
}

void Script::setOpcode(qsizetype opcodeID, const Opcode &opcode)
{
	_opcodes.replace(opcodeID, opcode);
}

void Script::removeOpcode(qsizetype opcodeID)
{
	_opcodes.removeAt(opcodeID);
}

void Script::insertOpcode(qsizetype opcodeID, const Opcode &opcode)
{
	_opcodes.insert(opcodeID, opcode);
}

bool Script::moveOpcode(qsizetype opcodeID, MoveDirection direction)
{
	if (opcodeID >= _opcodes.size()) {
		return false;
	}
	
	if (direction == Down) {
		if (opcodeID == _opcodes.size() - 1) {
			return false;
		}
		_opcodes.swapItemsAt(opcodeID, opcodeID + 1);
	} else {
		if (opcodeID == 0) {
			return false;
		}
		_opcodes.swapItemsAt(opcodeID, opcodeID - 1);
	}
	return true;
}

bool Script::searchOpcode(int opcode, int &opcodeID) const
{
	if (opcodeID < 0) {
		opcodeID = 0;
	}

	if (opcodeID >= _opcodes.size()) {
		return false;
	}

	if ((opcode & 0xFFFF) == _opcodes.at(opcodeID).id()) {
		return true;
	}

	return searchOpcode(opcode, ++opcodeID);
}

bool Script::searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &opcodeID) const
{
	if (opcodeID < 0) {
		opcodeID = 0;
	}

	if (opcodeID >= _opcodes.size()) {
		return false;
	}

	if (_opcodes.at(opcodeID).searchVar(bank, address, op, value)) {
		return true;
	}

	return searchVar(bank, address, op, value, ++opcodeID);
}

void Script::searchAllVars(QList<FF7Var> &vars) const
{
	for (const Opcode &opcode : _opcodes) {
		opcode.variables(vars);
	}
}

bool Script::searchExec(quint8 group, quint8 script, int &opcodeID) const
{
	if (opcodeID < 0) {
		opcodeID = 0;
	}

	if (opcodeID >= _opcodes.size()) {
		return false;
	}

	const Opcode &op = _opcodes.at(opcodeID);

	if (op.groupID() == group && op.scriptID() == script) {
		return true;
	}

	return searchExec(group, script, ++opcodeID);
}

bool Script::searchMapJump(quint16 map, int &opcodeID) const
{
	if (opcodeID < 0) {
		opcodeID = 0;
	}

	if (opcodeID >= _opcodes.size()) {
		return false;
	}

	if (_opcodes.at(opcodeID).mapID() == map) {
		return true;
	}

	return searchMapJump(map, ++opcodeID);
}

bool Script::searchTextInScripts(const QRegularExpression &text, int &opcodeID, const Section1File *scriptsAndTexts) const
{
	if (opcodeID < 0) {
		opcodeID = 0;
	}

	if (opcodeID >= _opcodes.size()) {
		return false;
	}

	qint16 textID = _opcodes.at(opcodeID).textID();

	if (textID >= 0 && textID < scriptsAndTexts->textCount() && scriptsAndTexts->text(textID).contains(text)) {
		return true;
	}

	return searchTextInScripts(text, ++opcodeID, scriptsAndTexts);
}

bool Script::searchOpcodeP(int opcode, int &opcodeID) const
{
	if (opcodeID >= _opcodes.size()) {
		opcodeID = _opcodes.size() - 1;
	}

	if (opcodeID < 0) {
		return false;
	}

	if ((opcode & 0xFFFF) == _opcodes.at(opcodeID).id()) {
		return true;
	}

	return searchOpcodeP(opcode, --opcodeID);
}

bool Script::searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &opcodeID) const
{
	if (opcodeID >= _opcodes.size()) {
		opcodeID = _opcodes.size() - 1;
	}

	if (opcodeID < 0) {
		return false;
	}

	if (_opcodes.at(opcodeID).searchVar(bank, address, op, value)) {
		return true;
	}

	return searchVarP(bank, address, op, value, --opcodeID);
}

bool Script::searchExecP(quint8 group, quint8 script, int &opcodeID) const
{
	if (opcodeID >= _opcodes.size()) {
		opcodeID = _opcodes.size() - 1;
	}

	if (opcodeID < 0) {
		return false;
	}

	Opcode op = _opcodes.at(opcodeID);
	if (op.groupID() == group && op.scriptID() == script) {
		return true;
	}

	return searchExecP(group, script, --opcodeID);
}

bool Script::searchMapJumpP(quint16 map, int &opcodeID) const
{
	if (opcodeID >= _opcodes.size()) {
		opcodeID = _opcodes.size() - 1;
	}

	if (opcodeID < 0) {
		return false;
	}

	if (_opcodes.at(opcodeID).mapID() == map) {
		return true;
	}

	return searchMapJumpP(map, --opcodeID);
}

bool Script::searchTextInScriptsP(const QRegularExpression &text, int &opcodeID, const Section1File *scriptsAndTexts) const
{
	if (opcodeID >= _opcodes.size()) {
		opcodeID = _opcodes.size() - 1;
	}

	if (opcodeID < 0) {
		return false;
	}
	
	qint16 textID = _opcodes.at(opcodeID).textID();

	if (textID >= 0 && textID < scriptsAndTexts->textCount() && scriptsAndTexts->text(textID).contains(text)) {
		return true;
	}

	return searchTextInScriptsP(text, --opcodeID, scriptsAndTexts);
}

void Script::listUsedTexts(QSet<quint8> &usedTexts) const
{
	for (const Opcode &opcode : _opcodes) {
		qint16 textID = opcode.textID();
		if (textID >= 0) {
			usedTexts.insert(quint8(textID));
		}
	}
}

void Script::listUsedTuts(QSet<quint8> &usedTuts) const
{
	for (const Opcode &opcode : _opcodes) {
		qint16 tutoID = opcode.tutoID();
		if (tutoID >= 0) {
			usedTuts.insert(quint8(tutoID));
		}
	}
}

void Script::shiftGroupIds(quint8 groupId, qint16 steps)
{
	for (Opcode &opcode : _opcodes) {
		qint16 groupID = opcode.groupID();
		if (groupID >= 0 && groupID > groupId) {
			opcode.setGroupID(quint8(std::min(groupID + steps, 255)));
		}
	}
}

void Script::shiftTextIds(quint8 textId, qint16 steps)
{
	for (Opcode &opcode : _opcodes) {
		qint16 textID = opcode.textID();
		if (textID >= 0 && textID > textId) {
			opcode.setTextID(quint8(std::min(textID + steps, 255)));
		}
	}
}

void Script::shiftTutIds(quint8 tutoId, qint16 steps)
{
	for (Opcode &opcode : _opcodes) {
		qint16 tutoID = opcode.tutoID();
		if (tutoID >= 0 && tutoID > tutoId) {
			opcode.setTutoID(quint8(std::min(tutoID + steps, 255)));
		}
	}
}

void Script::shiftPalIds(quint8 palId, qint16 steps)
{
	for (Opcode &opcode : _opcodes) {
		qint16 paletteID = opcode.paletteID();
		if (paletteID >= 0 && paletteID > palId) {
			opcode.setPaletteID(quint8(std::min(paletteID + steps, 255)));
		}
	}
}

void Script::swapGroupIds(quint8 groupId1, quint8 groupId2)
{
	for (Opcode &opcode : _opcodes) {
		qint16 groupID = opcode.groupID();
		if (groupID == groupId1) {
			opcode.setGroupID(groupId2);
		} else if (groupID == groupId2) {
			opcode.setGroupID(groupId1);
		}
	}
}

void Script::setWindow(const FF7Window &win)
{
	if (win.opcodeID < _opcodes.size()) {
		_opcodes[win.opcodeID].setWindow(win);
	}
}

quint32 Script::opcodePositionInBytes(qsizetype opcodeID) const
{
	quint32 pos = 0;
	qsizetype i = 0;
	for (const Opcode &op : _opcodes) {
		if (i == opcodeID) {
			return pos;
		}
		pos += op.size();
		++i;
	}
	return pos;
}

void Script::listWindows(int groupID, int scriptID, QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const
{
	int opcodeID = 0;
	for (const Opcode &opcode : _opcodes) {
		qint16 windowID = opcode.windowID();
		if (windowID >= 0 || opcode.id() == OpcodeKey::MPNAM) {
			FF7Window win = FF7Window();
			win.groupID = quint16(groupID);
			win.scriptID = quint16(scriptID);
			win.opcodeID = quint16(opcodeID);
			if (opcode.window(win)) {
				win.type = quint8(opcode.id());
				windows.insert(quint64((groupID << 16) | (scriptID << 8) | windowID), win);
			} else if (opcode.id() == OpcodeKey::WMODE && opcode.op().opcodeWMODE.mode == 0x01) {
				win.type = NOWIN;
				win.mode = 0x01;
				windows.insert(quint64((groupID << 16) | (scriptID << 8) | windowID), win);
			} else if (opcode.id() == OpcodeKey::MPNAM) {
				windowID = -1;
				win.type = OpcodeKey::MPNAM;
				windows.insert(quint64((groupID << 16) | (scriptID << 8) | windowID), win);
			}
			qint16 textID = opcode.textID();
			if (textID >= 0) {
				text2win.insert(quint8(textID), quint64((groupID << 16) | (scriptID << 8) | windowID));
			}
		}
	}
}

void Script::listWindows(int groupID, int scriptID, int textID, QList<FF7Window> &windows, int winID) const
{
	int opcodeID = 0;
	QMap<int, FF7Window> lastWinPerWindowID;
	for (const Opcode &opcode : _opcodes) {
		if (opcode.isJump() || (opcode.id() <= OpcodeKey::RETTO
		                         && opcode.id() != OpcodeKey::REQ
		                         && opcode.id() != OpcodeKey::PREQ)) {
			lastWinPerWindowID.clear();
		} else {
			FF7Window win = FF7Window();
			win.groupID = quint16(groupID);
			win.scriptID = quint16(scriptID);
			win.opcodeID = quint16(opcodeID);
			if (opcode.window(win)) {
				win.type = quint8(opcode.id());
				if (lastWinPerWindowID.contains(opcode.windowID())) {
					win.mode = lastWinPerWindowID.value(opcode.windowID()).mode;
					win.displayType = lastWinPerWindowID.value(opcode.windowID()).displayType;
					win.displayX = lastWinPerWindowID.value(opcode.windowID()).displayX;
					win.displayY = lastWinPerWindowID.value(opcode.windowID()).displayY;
				}
				lastWinPerWindowID.insert(opcode.windowID(), win);
			} else if (opcode.textID() == textID
			           && lastWinPerWindowID.contains(opcode.windowID())) {
				win = lastWinPerWindowID.value(opcode.windowID());
				if (win.type != 255 || win.mode == 0x01) {
					if (opcode.id() == OpcodeKey::ASK) {
						const OpcodeASK &opcodeAsk = opcode.op().opcodeASK;
						win.ask_first = opcodeAsk.firstLine;
						win.ask_last = opcodeAsk.lastLine;
						win.type = OpcodeKey::ASK;
					}
					if (winID < 0 || opcode.windowID() == winID) {
						windows.append(win);
					}
				}
			} else if (opcode.id() == OpcodeKey::WMODE) {
				if (lastWinPerWindowID.contains(opcode.windowID())) {
					win = lastWinPerWindowID.value(opcode.windowID());
				} else {
					win.type = 255;
				}
				win.mode = opcode.op().opcodeWMODE.mode;
				lastWinPerWindowID.insert(opcode.windowID(), win);
			} else if (opcode.id() == OpcodeKey::WSPCL) {
				if (lastWinPerWindowID.contains(opcode.windowID())) {
					win = lastWinPerWindowID.value(opcode.windowID());
				} else {
					win.type = 255;
				}
				win.displayType = opcode.op().opcodeWSPCL.displayType;
				win.displayX = opcode.op().opcodeWSPCL.marginLeft;
				win.displayY = opcode.op().opcodeWSPCL.marginTop;
				lastWinPerWindowID.insert(opcode.windowID(), win);
			} else if (opcode.id() == OpcodeKey::MPNAM && opcode.textID() == textID) {
				win.type = OpcodeKey::MPNAM;
				if (winID < 0) {
					windows.append(win);
				}
			}
		}

		opcodeID += 1;
	}
}

void Script::listModelPositions(QList<FF7Position> &positions) const
{
	for (const Opcode &opcode : _opcodes) {
		FF7Position pos;
		if (opcode.modelPosition(pos)) {
			positions.append(pos);
		}
	}
}

bool Script::linePosition(FF7Position position[2]) const
{
	for (const Opcode &opcode : _opcodes) {
		if (opcode.linePosition(position)) {
			return true;
		}
	}
	return false;
}

void Script::backgroundParams(QHash<quint8, quint8> &enabledParams) const
{
	for (const Opcode &opcode : _opcodes) {
		if (opcode.id() == OpcodeKey::BGON) { // Show background parameter
			const OpcodeBGON &bgon = opcode.op().opcodeBGON;
			if (bgon.banks == 0) {
				quint8 param = bgon.bgParamID;
				quint8 state = quint8(1 << bgon.bgStateID);
				if (enabledParams.contains(param)) {
					state |= enabledParams.value(param);
				}
				enabledParams.insert(param, state);
			}
		}/*else if (opcode.id() == BGOFF) { // Hide background parameter
			const OpcodeBGOFF &bgoff = opcode.op().opcodeBGOFF;
			if (bgoff.banks == 0) {
				quint8 param = bgoff.bgParamID;
				quint8 state = quint8(1 << bgoff.bgStateID);
				if (enabledParams.contains(param)) {
					state = (state & enabledParams.value(param)) ^ enabledParams.value(param);
				}
				enabledParams.insert(param, state);
			}
		}*/
	}
}

void Script::backgroundMove(qint16 z[2], qint16 *x, qint16 *y) const
{
	for (const Opcode &opcode : _opcodes) {
		if (opcode.id() == OpcodeKey::BGPDH) { // Move Background Z
			const OpcodeBGPDH &bgpdh = opcode.op().opcodeBGPDH;
			if (bgpdh.banks == 0 && bgpdh.layerID > 1 && bgpdh.layerID < 4) { // No var
				z[bgpdh.layerID - 2] = bgpdh.targetZ;
			}
		} else if (x && y && opcode.id() == OpcodeKey::BGSCR) { // Animate Background X Y
			const OpcodeBGSCR &bgscr = opcode.op().opcodeBGSCR;
			if (bgscr.banks == 0 && bgscr.layerID > 1 && bgscr.layerID < 4) { //No var
				x[bgscr.layerID - 2] = bgscr.targetX;
				y[bgscr.layerID - 2] = bgscr.targetY;
			}
		}
	}
}

bool Script::removeTexts()
{
	bool modified = false;
	qsizetype i = 0;
	for (const Opcode &opcode : std::as_const(_opcodes)) {
		if (opcode.id() != OpcodeKey::ASK
				&& opcode.id() != OpcodeKey::MPNAM
				&& opcode.textID() != -1) {
			_opcodes.removeAt(i);
			modified = true;
		}
		i += 1;
	}

	return modified;
}

QString Script::toString(const Section1File *scriptsAndTexts) const
{
	QString ret;

	for (const Opcode &opcode : _opcodes) {
		ret.append(opcode.toString(scriptsAndTexts));
		ret.append("\n");
	}

	return ret;
}

QDataStream &operator<<(QDataStream &stream, const QList<Opcode> &script)
{
	stream << script.size();

	for (const Opcode &opcode : script) {
		stream << opcode.serialize();
	}

	return stream;
}

QDataStream &operator>>(QDataStream &stream, QList<Opcode> &script)
{
	qsizetype size;
	stream >> size;

	for (qsizetype i = 0; i < size; ++i) {
		QByteArray data;
		stream >> data;

		script.append(Opcode::unserialize(data));
	}

	return stream;
}
