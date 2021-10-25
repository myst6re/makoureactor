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
#include "GrpScript.h"
#include "../FF7Text.h"
#include "../../Data.h"

GrpScript::GrpScript() :
    _scripts(SCRIPTS_SIZE), _type(NoType), _character(-1)
{
	_scripts[0] = Script(QList<OpcodeBox>() << OpcodeBox(new OpcodeRET()));
	_scripts[1] = Script(QList<OpcodeBox>() << OpcodeBox(new OpcodeRET()));
}

GrpScript::GrpScript(const QString &name, const QList<Script> &scripts) :
    _name(name), _scripts(SCRIPTS_SIZE), _type(NoType), _character(-1)
{
	qsizetype size = qMin(scripts.size(), SCRIPTS_SIZE);
	for (qint8 i = 0; i < size; ++i) {
		_scripts[i] = scripts.at(i);
	}
}

GrpScript GrpScript::createGroupModel(quint8 modelID, qint16 charID)
{
	GrpScript group;
	QList<OpcodeBox> initOps;
	initOps.append(OpcodeBox(new OpcodeCHAR(modelID)));
	if (charID >= 0) {
		initOps.append(OpcodeBox(new OpcodePC(quint8(charID))));
	}

	group.setScript(0, Script(initOps));

	return group;
}

void GrpScript::setScript(int row, const Script &script)
{
	_scripts[row] = script;

	if (row == 0) {
		detectType();
	}
}

void GrpScript::detectType()
{
	_character = -1;
	_type = NoType;
	
	const Script &firstScript = _scripts.at(0);
	
	for (const OpcodeBox &opcode : firstScript.opcodes()) {
		switch (opcode.id()) {
		case Opcode::PC: // Character definition
			_character = opcode.cast<OpcodePC>().charID;
			_type = Model;
			return;
		case Opcode::CHAR:// 3D Model definition
			_character = 0x100;
			_type = Model;
			break;
		case Opcode::LINE:// Wakmesh line definition
			if (_type == NoType) {
				_type = Location;
			}
			return;
		case Opcode::BGPDH:case Opcode::BGSCR:case Opcode::BGON:
		case Opcode::BGOFF:case Opcode::BGROL:case Opcode::BGROL2:
		case Opcode::BGCLR:// Background parameter
			if (_type == NoType) {
				_type = Animation;
			}
			return;
		case Opcode::MPNAM:// Map name
			if (_type == NoType) {
				_type = Director;
			}
			return;
		default:
			break;
		}
	}
}

void GrpScript::backgroundParams(QHash<quint8, quint8> &paramActifs) const
{
	_scripts.first().backgroundParams(paramActifs);
}

void GrpScript::backgroundMove(qint16 z[2], qint16 *x, qint16 *y) const
{
	for (const Script &script : _scripts) {
		script.backgroundMove(z, x, y);
	}
}

QString GrpScript::name() const
{
	return _name.isEmpty() ? QObject::tr("Untitled") : _name;
}

QList<Script> GrpScript::scriptToList() const
{
	QList<Script> ret;
	ret.reserve(33);

	for (const Script &script : _scripts) {
		ret.append(script);
	}

	return ret;
}

QByteArray GrpScript::toByteArray(quint8 scriptID) const
{
	if (scriptID == 0) {
		if (!_scripts.at(0).isEmpty()) {
			return _scripts.at(0).toByteArray() + _scripts.at(1).toByteArray();
		}
		return QByteArray();
	}
	if (scriptID + 1 < SCRIPTS_SIZE) {
		return _scripts.at(scriptID + 1).toByteArray();
	}
	return QByteArray();
}

QString GrpScript::typeString() const
{
	switch (type()) {
	case Model:
		if (_character == 0x100) {
			return QObject::tr("Field model");
		}
		return QString("%1").arg(Opcode::character(quint8(_character)));
	case Location:
		return QObject::tr("Line");
	case Animation:
		return QObject::tr("Animation");
	case Director:
		return QObject::tr("Main");
	default:
		return QString();
	}
}

QColor GrpScript::typeColor() const
{
	switch (type()) {
	case Model:
		return Data::color(Data::ColorBlueForeground);
	case Location:
		return Data::color(Data::ColorGreenForeground);
	case Animation:
		return Data::color(Data::ColorRedForeground);
	case Director:
		return Data::color(Data::ColorPurpleForeground);
	default:
		return QColor();
	}
}

QString GrpScript::scriptName(quint8 scriptID) const
{
	Type t = type();
	
	switch (scriptID) {
	case 0:
		return QObject::tr("S0 - Init");
	case 1:
		return QObject::tr("S0 - Main");
	case 2:
		if (t == Model) {
			return QObject::tr("S1 - Talk");
		}
		if (t == Location) {
			return QObject::tr("S1 - [OK]");
		}
		break;
	case 3:
		if (t == Model) {
			return QObject::tr("S2 - Contact");
		}
		if (t == Location) {
			return QObject::tr("S2 - Move");
		}
		break;
	case 4:
		if (t == Location) {
			return QObject::tr("S3 - Move");
		}
		break;
	case 5:
		if (t == Location) {
			return QObject::tr("S4 - Go");
		}
		break;
	case 6:
		if (t == Location) {
			return QObject::tr("S5 - Go 1x");
		}
		break;
	case 7:
		if (t == Location) {
			return QObject::tr("S6 - Go away");
		}
		break;
	}
	
	return QObject::tr("Script %1").arg(scriptID - 1);
}

bool GrpScript::search(int &scriptID, int &opcodeID) const
{
	if (scriptID < 0) {
		opcodeID = scriptID = 0;
	}
	return scriptID < SCRIPTS_SIZE;
}

bool GrpScript::searchOpcode(int opcode, int &scriptID, int &opcodeID) const
{
	if (!search(scriptID, opcodeID)) {
		return false;
	}
	if (_scripts.at(scriptID).searchOpcode(opcode, opcodeID)) {
		return true;
	}

	opcodeID = 0;
	return searchOpcode(opcode, ++scriptID, opcodeID);
}

bool GrpScript::searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &scriptID, int &opcodeID) const
{
	if (!search(scriptID, opcodeID)) {
		return false;
	}
	if (_scripts.at(scriptID).searchVar(bank, address, op, value, opcodeID)) {
		return true;
	}

	opcodeID = 0;
	return searchVar(bank, address, op, value, ++scriptID, opcodeID);
}

void GrpScript::searchAllVars(QList<FF7Var> &vars) const
{
	for (const Script &script : _scripts) {
		script.searchAllVars(vars);
	}
}

bool GrpScript::searchExec(quint8 group, quint8 script, int &scriptID, int &opcodeID) const
{
	if (!search(scriptID, opcodeID)) {
		return false;
	}
	if (_scripts.at(scriptID).searchExec(group, script, opcodeID)) {
		return true;
	}

	opcodeID = 0;
	return searchExec(group, script, ++scriptID, opcodeID);
}

bool GrpScript::searchMapJump(quint16 field, int &scriptID, int &opcodeID) const
{
	if (!search(scriptID, opcodeID)) {
		return false;
	}
	if (_scripts.at(scriptID).searchMapJump(field, opcodeID)) {
		return true;
	}

	opcodeID = 0;
	return searchMapJump(field, ++scriptID, opcodeID);
}

bool GrpScript::searchTextInScripts(const QRegularExpression &text, int &scriptID, int &opcodeID, const Section1File *scriptsAndTexts) const
{
	if (scriptID < 0) {
		opcodeID = scriptID = 0;
	}
	if (scriptID >= SCRIPTS_SIZE) {
		return false;
	}
	if (_scripts.at(scriptID).searchTextInScripts(text, opcodeID, scriptsAndTexts)) {
		return true;
	}

	opcodeID = 0;
	return searchTextInScripts(text, ++scriptID, opcodeID, scriptsAndTexts);
}

bool GrpScript::searchP(int &scriptID, int &opcodeID) const
{
	if (scriptID >= SCRIPTS_SIZE) {
		scriptID = SCRIPTS_SIZE - 1;
		opcodeID = 2147483647;
	}

	return scriptID >= 0;
}

bool GrpScript::searchOpcodeP(int opcode, int &scriptID, int &opcodeID) const
{
	if (!searchP(scriptID, opcodeID)) {
		return false;
	}
	if (_scripts.at(scriptID).searchOpcodeP(opcode, opcodeID)) {
		return true;
	}

	opcodeID = 2147483647;
	return searchOpcodeP(opcode, --scriptID, opcodeID);
}

bool GrpScript::searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &scriptID, int &opcodeID) const
{
	if (!searchP(scriptID, opcodeID)) {
		return false;
	}
	if (_scripts.at(scriptID).searchVarP(bank, address, op, value, opcodeID)) {
		return true;
	}

	opcodeID = 2147483647;
	return searchVarP(bank, address, op, value, --scriptID, opcodeID);
}

bool GrpScript::searchExecP(quint8 group, quint8 script, int &scriptID, int &opcodeID) const
{
	if (!searchP(scriptID, opcodeID)) {
		return false;
	}
	if (_scripts.at(scriptID).searchExecP(group, script, opcodeID)) {
		return true;
	}

	opcodeID = 2147483647;
	return searchExecP(group, script, --scriptID, opcodeID);
}

bool GrpScript::searchMapJumpP(quint16 field, int &scriptID, int &opcodeID) const
{
	if (!searchP(scriptID, opcodeID)) {
		return false;
	}
	if (_scripts.at(scriptID).searchMapJumpP(field, opcodeID)) {
		return true;
	}

	opcodeID = 2147483647;
	return searchMapJumpP(field, --scriptID, opcodeID);
}

bool GrpScript::searchTextInScriptsP(const QRegularExpression &text, int &scriptID, int &opcodeID, const Section1File *scriptsAndTexts) const
{
	if (!searchP(scriptID, opcodeID)) {
		return false;
	}
	if (_scripts.at(scriptID).searchTextInScriptsP(text, opcodeID, scriptsAndTexts)) {
		return true;
	}

	opcodeID = 2147483647;
	return searchTextInScriptsP(text, --scriptID, opcodeID, scriptsAndTexts);
}

void GrpScript::listUsedTexts(QSet<quint8> &usedTexts) const
{
	for (const Script &script : _scripts) {
		script.listUsedTexts(usedTexts);
	}
}

void GrpScript::listUsedTuts(QSet<quint8> &usedTuts) const
{
	for (const Script &script : _scripts) {
		script.listUsedTuts(usedTuts);
	}
}

void GrpScript::shiftGroupIds(int groupId, int steps)
{
	for (Script &script : _scripts) {
		script.shiftGroupIds(groupId, steps);
	}
}

void GrpScript::shiftTextIds(int textId, int steps)
{
	for (Script &script : _scripts) {
		script.shiftTextIds(textId, steps);
	}
}

void GrpScript::shiftTutIds(int tutId, int steps)
{
	for (Script &script : _scripts) {
		script.shiftTutIds(tutId, steps);
	}
}

void GrpScript::swapGroupIds(int groupId1, int groupId2)
{
	for (Script &script : _scripts) {
		script.swapGroupIds(groupId1, groupId2);
	}
}

void GrpScript::setWindow(const FF7Window &win)
{
	if (win.scriptID < SCRIPTS_SIZE) {
		_scripts[win.scriptID].setWindow(win);
	}
}

void GrpScript::listWindows(int groupID, QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const
{
	int scriptID = 0;
	for (const Script &script : _scripts) {
		script.listWindows(groupID, scriptID++, windows, text2win);
	}
}

void GrpScript::listWindows(int groupID, int textID, QList<FF7Window> &windows) const
{
	int scriptID = 0;
	for (const Script &script : _scripts) {
		script.listWindows(groupID, scriptID++, textID, windows);
	}
}

void GrpScript::listModelPositions(QList<FF7Position> &positions) const
{
	_scripts.at(0).listModelPositions(positions);
	_scripts.at(1).listModelPositions(positions);
}

bool GrpScript::linePosition(FF7Position position[2]) const
{
	return _scripts.first().linePosition(position);
}

bool GrpScript::compile(int &scriptID, int &opcodeID, QString &errorStr)
{
	scriptID = 0;
	for (Script &script : _scripts) {
		if (!script.compile(opcodeID, errorStr)) {
			return false;
		}
		++scriptID;
	}

	return true;
}

bool GrpScript::removeTexts()
{
	bool modified = false;
	for (Script &script : _scripts) {
		if (script.removeTexts()) {
			modified = true;
		}
	}

	return modified;
}

QString GrpScript::toString(Field *field) const
{
	QString ret(QObject::tr("Group '%1':").arg(name()));
	int scriptID = 0;

	ret.append("\n");

	for (const Script &script : _scripts) {
		if (!script.isEmpty()) {
			ret.append("\t");
			ret.append(QObject::tr("Script '%1' :").arg(scriptID));
			ret.append("\n\t\t");
			ret.append(script.toString(field).split("\n").join("\n\t\t"));
			ret.append("\n");
		}
		scriptID++;
	}

	return ret;
}

QDataStream &operator<<(QDataStream &stream, const QList<GrpScript> &groups)
{
	stream << groups.size();

	for (const GrpScript &group : groups) {
		stream << group.name() << group.scriptToList();
	}

	return stream;
}

QDataStream &operator>>(QDataStream &stream, QList<GrpScript> &groups)
{
	int size;
	stream >> size;

	for (int i = 0; i < size; ++i) {
		QString name;
		QList<Script> scripts;
		stream >> name >> scripts;
		groups.append(GrpScript(name, scripts));
	}

	return stream;
}

QDataStream &operator<<(QDataStream &stream, const QList<Script> &scripts)
{
	stream << scripts.size();

	for (const Script &script : scripts) {
		stream << script.serialize();
	}

	return stream;
}

QDataStream &operator>>(QDataStream &stream, QList<Script> &scripts)
{
	int size;
	stream >> size;

	for (int i = 0; i < size; ++i) {
		QByteArray data;
		stream >> data;

		scripts.append(Script(data));
	}

	return stream;
}
