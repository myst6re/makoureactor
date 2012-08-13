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

	qDebug() << name << "script" << scripts.size();

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
	
	quint16 nbCommandes = firstScript->size();
	
	for(quint16 i=0 ; i<nbCommandes ; ++i)
	{
		switch(firstScript->getCommande(i)->id())
		{
		case 0xA0://Definition du personnage
			character = ((OpcodePC *)firstScript->getCommande(i))->charID;
			return;
		case 0xA1://Definition du modèle 3D
			character = 0xFF;
			break;
		case 0xD0://definition d'une zone
			if(character==-1)	location = true;
			return;
		case 0x2C:case 0x2D:case 0xE0:case 0xE1:case 0xE2:case 0xE3:case 0xE4://bg paramètres
			if(character==-1)	animation = true;
			return;
		case 0x43://mapname
			if(character==-1)	director = true;
			return;
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

QString GrpScript::getRealName() const
{
	return name;
}

int GrpScript::size() const
{
	return scripts.size();
}

Script *GrpScript::getScript(quint8 scriptID) const
{
	if(scriptID >= scripts.size())	return NULL;
	return scripts.at(scriptID);
}

QByteArray GrpScript::getRealScript(quint8 scriptID) const
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
		return QString("%1").arg(Commande::_personnage(character));
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

bool GrpScript::rechercherOpCode(quint8 opCode, int &scriptID, int &commandeID) const
{
	if(scriptID < 0)	scriptID = 0;

	int nbScripts = scripts.size();
	
	while(scriptID < nbScripts)
	{
		if(scripts.at(scriptID)->rechercherOpCode(opCode, commandeID))	return true;
		++scriptID;
		commandeID = 0;
	}
	return false;
}

bool GrpScript::rechercherVar(quint8 bank, quint8 adress, int value, int &scriptID, int &commandeID) const
{
	if(scriptID < 0)	scriptID = 0;

	int nbScripts = scripts.size();
	
	while(scriptID < nbScripts)
	{
		if(scripts.at(scriptID)->rechercherVar(bank, adress, value, commandeID))	return true;
		++scriptID;
		commandeID = 0;
	}
	return false;
}

QList<FF7Var> GrpScript::searchAllVars() const
{
	QList<FF7Var> vars;

	foreach(Script *script, scripts) {
		vars.append(script->searchAllVars());
	}

	return vars;
}

bool GrpScript::rechercherExec(quint8 group, quint8 script, int &scriptID, int &commandeID) const
{
	if(scriptID < 0)	scriptID = 0;

	int nbScripts = scripts.size();

	while(scriptID < nbScripts)
	{
		if(scripts.at(scriptID)->rechercherExec(group, script, commandeID))	return true;
		++scriptID;
		commandeID = 0;
	}
	return false;
}

bool GrpScript::rechercherTexte(const QRegExp &texte, int &scriptID, int &commandeID) const
{
	if(scriptID < 0)	scriptID = 0;

	int nbScripts = scripts.size();
	
	while(scriptID < nbScripts)
	{
		if(scripts.at(scriptID)->rechercherTexte(texte, commandeID))	return true;
		++scriptID;
		commandeID = 0;
	}
	return false;
}

bool GrpScript::rechercherOpCodeP(quint8 opCode, int &scriptID, int &commandeID) const
{
	if(scriptID >= scripts.size())	scriptID = scripts.size()-1;

	while(scriptID >= 0)
	{
		if(scripts.at(scriptID)->rechercherOpCodeP(opCode, commandeID))	return true;
		--scriptID;
		if(scriptID >= 0) {
			commandeID = scripts.at(scriptID)->size()-1;
		}
	}

	return false;
}

bool GrpScript::rechercherVarP(quint8 bank, quint8 adress, int value, int &scriptID, int &commandeID) const
{
	if(scriptID >= scripts.size())	scriptID = scripts.size()-1;

	while(scriptID >= 0)
	{
		if(scripts.at(scriptID)->rechercherVarP(bank, adress, value, commandeID))	return true;
		--scriptID;
		if(scriptID >= 0)	commandeID = scripts.at(scriptID)->size()-1;
	}

	return false;
}

bool GrpScript::rechercherExecP(quint8 group, quint8 script, int &scriptID, int &commandeID) const
{
	if(scriptID >= scripts.size())	scriptID = scripts.size()-1;

	while(scriptID >= 0)
	{
		if(scripts.at(scriptID)->rechercherExecP(group, script, commandeID))	return true;
		--scriptID;
		if(scriptID >= 0)	commandeID = scripts.at(scriptID)->size()-1;
	}

	return false;
}

bool GrpScript::rechercherTexteP(const QRegExp &texte, int &scriptID, int &commandeID) const
{
	if(scriptID >= scripts.size())	scriptID = scripts.size()-1;

	while(scriptID >= 0)
	{
		if(scripts.at(scriptID)->rechercherTexteP(texte, commandeID))	return true;
		--scriptID;
		if(scriptID >= 0)	commandeID = scripts.at(scriptID)->size()-1;
	}

	return false;
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
