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
#include "GrpScript.h"

GrpScript::GrpScript()
	: name(QString()), character(-1), animation(false), location(false), director(false)
{
	addScript(QByteArray(2, '\x0'));
	for(int i=1 ; i<32 ; i++)	addScript();
}

GrpScript::GrpScript(const QString &name)
	: name(name), character(-1), animation(false), location(false), director(false)
{
}

GrpScript::~GrpScript()
{
	foreach(Script *script, scripts)	delete script;
}

void GrpScript::addScript()
{
	if(scripts.isEmpty())	scripts.append(new Script());
	scripts.append(new Script());
}

bool GrpScript::addScript(const QByteArray &script, bool explodeInit)
{
	Script *s;

	if(explodeInit && scripts.isEmpty())
	{
		quint16 pos = Script::posReturn(script);
		s = new Script(script.left(pos));
		if(!s->isValid()) {
			delete s;
			return false;
		}
		scripts.append(s);

		s = new Script(script.mid(pos));
		if(!s->isValid()) {
			delete s;
			return false;
		}
		scripts.append(s);
	}
	else {
		s = new Script(script);
		if(!s->isValid()) {
			delete s;
			return false;
		}
		scripts.append(s);
	}

	return true;
}

/* void GrpScript::replaceScript(int row, QByteArray script)
{
	scripts.replace(row, new Script(script));
} */

void GrpScript::setType()
{
	if(scripts.size()<1)	return;
	character = -1;
	director = animation = location = false;
	
	Script *firstScript = scripts.first();
	
	foreach(Opcode *opcode, firstScript->getOpcodes())
	{
		switch((Opcode::Keys)opcode->id())
		{
		case Opcode::PC://Definition du personnage
			character = ((OpcodePC *)opcode)->charID;
			return;
		case Opcode::CHAR://Definition du modèle 3D
			character = 0xFF;
			break;
		case Opcode::LINE://definition d'une zone
			if(character==-1)	location = true;
			return;
		case Opcode::BGPDH:case Opcode::BGSCR:case Opcode::BGON:
		case Opcode::BGOFF:case Opcode::BGROL:case Opcode::BGROL2:
		case Opcode::BGCLR://bg paramètres
			if(character==-1)	animation = true;
			return;
		case Opcode::MPNAM://mapname
			if(character==-1)	director = true;
			return;
		default:
			break;
		}
	}
}

void GrpScript::getBgParams(QHash<quint8, quint8> &paramActifs) const
{
	if(scripts.size()<1)	return;

	scripts.first()->getBgParams(paramActifs);
}

void GrpScript::getBgMove(qint16 z[2], qint16 *x, qint16 *y) const
{
	if(scripts.size()<2)	return;

	foreach(Script *script, scripts) {
		script->getBgMove(z, x, y);
	}
}

QString GrpScript::getName() const
{
	return name.isEmpty() ? QObject::tr("Sans nom") : name;
}

void GrpScript::setName(const QString &name)
{
	this->name = name;
}

const QString &GrpScript::getRealName() const
{
	return name;
}

int GrpScript::size() const
{
	return scripts.size();
}

Script *GrpScript::getScript(quint8 scriptID) const
{
	return scripts.at(scriptID);
}

QByteArray GrpScript::toByteArray(quint8 scriptID) const
{
	if(scriptID == 0)
	{
		if(!scripts.at(0)->isEmpty())		return scripts.at(0)->toByteArray()+scripts.at(1)->toByteArray();
		return QByteArray();
	}
	return scripts.at(scriptID+1)->toByteArray();
}

int GrpScript::getTypeID()
{
	setType();
	if(character != -1)		return 1;
	if(location)			return 2;
	if(animation)			return 3;
	if(director)			return 4;
	return 0;
}

QString GrpScript::getType()
{
	switch(getTypeID())
	{
	case 1:
		if(character == 0xFF)	return QObject::tr("Objet 3D");
		return QString("%1").arg(Opcode::_personnage(character));
	case 2:	return QObject::tr("Zone");
	case 3:	return QObject::tr("Animation");
	case 4:	return QObject::tr("Main");
	}
	return "";
}

QColor GrpScript::getTypeColor()
{
	switch(getTypeID())
	{
	case 1:	return QColor(0x00,0x66,0xcc);
	case 2:	return QColor(0x00,0xcc,0x66);
	case 3:	return QColor(0xcc,0x66,0x00);
	case 4:	return QColor(0x66,0x00,0xcc);
	}
	return QColor(0x00,0x00,0x00);
}

QString GrpScript::getScriptName(quint8 scriptID)
{
	int type = getTypeID();
	
	switch(scriptID)
	{
	case 0:	return QObject::tr("S0 - Init");
	case 1:	return QObject::tr("S0 - Main");
	case 2:
		if(type == 1)	return QObject::tr("S1 - Parler");
		if(type == 2)	return QObject::tr("S1 - [OK]");
		break;
	case 3:
		if(type == 1)	return QObject::tr("S2 - Toucher");
		if(type == 2)	return QObject::tr("S2 - Bouger");
		break;
	case 4:
		if(type == 2)	return QObject::tr("S3 - Bouger");
		break;
	case 5:
		if(type == 2)	return QObject::tr("S4 - Aller");
		break;
	case 6:
		if(type == 2)	return QObject::tr("S5 - Aller 1x");
		break;
	case 7:
		if(type == 2)	return QObject::tr("S6 - Partir");
		break;
	}
	
	return QObject::tr("Script %1").arg(scriptID-1);
}

bool GrpScript::search(int &scriptID, int &opcodeID) const
{
	if(scriptID < 0)
		opcodeID = scriptID = 0;
	if(scriptID >= scripts.size())
		return false;
	return true;
}

bool GrpScript::searchOpcode(int opcode, int &scriptID, int &opcodeID) const
{
	if(!search(scriptID, opcodeID))
		return false;
	if(scripts.at(scriptID)->searchOpcode(opcode, opcodeID))
		return true;

	return searchOpcode(opcode, ++scriptID, opcodeID = 0);
}

bool GrpScript::searchVar(quint8 bank, quint8 adress, int value, int &scriptID, int &opcodeID) const
{
	if(!search(scriptID, opcodeID))
		return false;
	if(scripts.at(scriptID)->searchVar(bank, adress, value, opcodeID))
		return true;

	return searchVar(bank, adress, value, ++scriptID, opcodeID = 0);
}

void GrpScript::searchAllVars(QList<FF7Var> &vars) const
{
	foreach(Script *script, scripts) {
		script->searchAllVars(vars);
	}
}

bool GrpScript::searchExec(quint8 group, quint8 script, int &scriptID, int &opcodeID) const
{
	if(!search(scriptID, opcodeID))
		return false;
	if(scripts.at(scriptID)->searchExec(group, script, opcodeID))
		return true;

	return searchExec(group, script, ++scriptID, opcodeID = 0);
}

bool GrpScript::searchMapJump(quint16 field, int &scriptID, int &opcodeID) const
{
	if(!search(scriptID, opcodeID))
		return false;
	if(scripts.at(scriptID)->searchMapJump(field, opcodeID))
		return true;

	return searchMapJump(field, ++scriptID, opcodeID = 0);
}

bool GrpScript::searchTextInScripts(const QRegExp &text, int &scriptID, int &opcodeID) const
{
	if(scriptID < 0)
		opcodeID = scriptID = 0;
	if(scriptID >= scripts.size())
		return false;
	if(scripts.at(scriptID)->searchTextInScripts(text, opcodeID))
		return true;

	return searchTextInScripts(text, ++scriptID, opcodeID = 0);
}

bool GrpScript::searchP(int &scriptID, int &opcodeID) const
{
	if(scriptID >= scripts.size()) {
		scriptID = scripts.size()-1;
		opcodeID = 2147483647;
	}
	if(scriptID < 0)
		return false;
	return true;
}

bool GrpScript::searchOpcodeP(int opcode, int &scriptID, int &opcodeID) const
{
	if(!searchP(scriptID, opcodeID))
		return false;
	if(scripts.at(scriptID)->searchOpcodeP(opcode, opcodeID))
		return true;

	return searchOpcodeP(opcode, --scriptID, opcodeID = 2147483647);
}

bool GrpScript::searchVarP(quint8 bank, quint8 adress, int value, int &scriptID, int &opcodeID) const
{
	if(!searchP(scriptID, opcodeID))
		return false;
	if(scripts.at(scriptID)->searchVarP(bank, adress, value, opcodeID))
		return true;

	return searchVarP(bank, adress, value, --scriptID, opcodeID = 2147483647);
}

bool GrpScript::searchExecP(quint8 group, quint8 script, int &scriptID, int &opcodeID) const
{
	if(!searchP(scriptID, opcodeID))
		return false;
	if(scripts.at(scriptID)->searchExecP(group, script, opcodeID))
		return true;

	return searchExecP(group, script, --scriptID, opcodeID = 2147483647);
}

bool GrpScript::searchMapJumpP(quint16 field, int &scriptID, int &opcodeID) const
{
	if(!searchP(scriptID, opcodeID))
		return false;
	if(scripts.at(scriptID)->searchMapJumpP(field, opcodeID))
		return true;

	return searchMapJumpP(field, --scriptID, opcodeID = 2147483647);
}

bool GrpScript::searchTextInScriptsP(const QRegExp &text, int &scriptID, int &opcodeID) const
{
	if(!searchP(scriptID, opcodeID))
		return false;
	if(scripts.at(scriptID)->searchTextInScriptsP(text, opcodeID))
		return true;

	return searchTextInScriptsP(text, --scriptID, opcodeID = 2147483647);
}

void GrpScript::listUsedTexts(QSet<quint8> &usedTexts) const
{
	foreach(Script *script, scripts)
		script->listUsedTexts(usedTexts);
}

void GrpScript::listUsedTuts(QSet<quint8> &usedTuts) const
{
	foreach(Script *script, scripts)
		script->listUsedTuts(usedTuts);
}

void GrpScript::shiftTextIds(int textId, int steps)
{
	foreach(Script *script, scripts)
		script->shiftTextIds(textId, steps);
}

void GrpScript::shiftTutIds(int tutId, int steps)
{
	foreach(Script *script, scripts)
		script->shiftTutIds(tutId, steps);
}

void GrpScript::listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const
{
	foreach(Script *script, scripts)
		script->listWindows(windows, text2win);
}
