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
#include "Opcode.h"

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
#include "Opcode.h"

Opcode::Opcode()
{
}

QByteArray Opcode::params() const
{
	return QByteArray();
}

quint8 Opcode::size() const
{
	return Opcode::length[id()];
}

bool Opcode::hasParams() const
{
	return size() > 1;
}

const QString &Opcode::name() const
{
	return Opcode::names[id()];
}

void Opcode::setParams(const QByteArray &)
{
}

QByteArray Opcode::toByteArray() const
{
	return QByteArray()
			.append((char)id())
			.append(params());
}

bool Opcode::isJump() const
{
	return false;
}

bool Opcode::isLabel() const
{
	return false;
}

int Opcode::subParam(int cur, int paramSize) const
{
	QByteArray p = params();
	int value, sizeBA;

	if(paramSize%8 !=0)
		sizeBA = paramSize/8+1;
	else
		sizeBA = paramSize/8;

	memcpy(&value, p.mid(cur/8, sizeBA), sizeBA);
	return (value >> ((sizeBA*8-cur%8)-paramSize)) & ((int)pow(2, paramSize)-1);
}

bool Opcode::isVoid() const
{
	return false;
}

int Opcode::getTextID() const
{
	return -1;
}

void Opcode::setTextID(quint8)
{
}

int Opcode::getTutoID() const
{
	return -1;
}

void Opcode::setTutoID(quint8)
{
}

int Opcode::getWindowID() const
{
	return -1;
}

void Opcode::setWindowID(quint8)
{
}

bool Opcode::getWindow(FF7Window &) const
{
	return false;
}

void Opcode::setWindow(const FF7Window &)
{
}

void Opcode::getVariables(QList<FF7Var> &) const
{
}

bool Opcode::rechercherVar(quint8 bank, quint8 adress, int value) const
{
	if(value != 65536) {
		if(id()==0x80) {
			OpcodeSETBYTE *setbyte = (OpcodeSETBYTE *)this;
			if(B1(setbyte->banks) == bank && setbyte->var == adress
					&& B2(setbyte->banks) == bank && setbyte->value == value)
				return true;
		}
		if(id()==0x81) {
			OpcodeSETWORD *setword = (OpcodeSETWORD *)this;
			if(B1(setword->banks) == bank && setword->var == adress
					&& B2(setword->banks) == bank && setword->value == (quint16)value)
				return true;
		}
		return false;
	}

	QList<FF7Var> vars;

	getVariables(vars);

	foreach(const FF7Var &var, vars) {
		if(var.bank == bank && var.adress == adress)
			return true;
	}
	return false;
}

bool Opcode::rechercherExec(quint8 group, quint8 script) const
{
	if(id()==0x01 || id()==0x02 || id()==0x03) {
		OpcodeExec *exec = (OpcodeExec *)this;
		return exec->groupID == group && exec->scriptID == script;
	}
	return false;
}

bool Opcode::rechercherTexte(const QRegExp &texte) const
{
	qint16 textID = getTextID();
	return textID != -1
			&& textID < Data::currentTextes->size()
			&& Data::currentTextes->at(textID)->search(texte);
}

void Opcode::listUsedTexts(QSet<quint8> &usedTexts) const
{
	int textID = getTextID();
	if(textID != -1) {
		usedTexts.insert(textID);
	}
}

void Opcode::listUsedTuts(QSet<quint8> &usedTuts) const
{
	int tutoID = getTutoID();
	if(tutoID != -1) {
		usedTuts.insert(tutoID);
	}
}

void Opcode::shiftTextIds(int textId, int steps)
{
	int textID = getTextID();
	if(textID != -1 && textID > textId) {
		setTextID(textID + steps);
	}
}

void Opcode::shiftTutIds(int tutId, int steps)
{
	int tutoID = getTutoID();
	if(tutoID != -1 && tutoID > tutId) {
		setTutoID(tutoID + steps);
	}
}

void Opcode::listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const
{
	int windowID = getWindowID();
	if(windowID != -1) {
		FF7Window win;
		memset(&win, 0, sizeof(FF7Window));
		win.type = id();

		if(getWindow(win)) {
			windows.insert(windowID, win);
		}
		int textID = getTextID();
		if(textID != -1) {
			text2win.insert(textID, windowID);
		}
	}
}

void Opcode::getBgParams(QHash<quint8, quint8> &enabledParams) const
{
	quint8 param, state;

	if(id()==0xE0)//show bg parameter
	{
		OpcodeBGON *bgon = (OpcodeBGON *)this;
		if(bgon->banks == 0) {
			param = bgon->paramID;
			state = 1 << bgon->stateID;
			if(enabledParams.contains(param))
				state |= enabledParams.value(param);
			enabledParams.insert(param, state);
		}
	}
	/*else if(id()==0xE1)//hide bg parameter
	{
		OpcodeBGOFF *bgoff = (OpcodeBGOFF *)this;
		if(bgoff->banks == 0) {
			param = bgoff->paramID;
			state = 1 << bgoff->stateID;
			if(enabledParams.contains(param))
				state = (state & enabledParams.value(param)) ^ enabledParams.value(param);
			enabledParams.insert(param, state);
		}
	}*/
}

void Opcode::getBgMove(qint16 z[2], qint16 *x, qint16 *y) const
{
	if(id()==0x2C)//Move Background Z
	{
		OpcodeBGPDH *bgpdh = (OpcodeBGPDH *)this;
		if(bgpdh->banks==0 && bgpdh->layerID>1 && bgpdh->layerID<4)//No var
		{
			z[bgpdh->layerID-2] = bgpdh->targetZ;
		}
	}
	else if(x && y && id()==0x2D)// Animate Background X Y
	{
		OpcodeBGSCR *bgscr = (OpcodeBGSCR *)this;
		if(bgscr->banks==0 && bgscr->layerID>1 && bgscr->layerID<4)//No var
		{
			x[bgscr->layerID-2] = bgscr->targetX;
			y[bgscr->layerID-2] = bgscr->targetY;
		}
	}
}

QString Opcode::_script(quint8 param)
{
	if(param < Data::currentGrpScriptNames.size())
		return Data::currentGrpScriptNames.at(param) + QObject::tr(" (n°%1)").arg(param);
	return QObject::tr("? (n°%1)").arg(param);
}

QString Opcode::_text(quint8 textID)
{
	if(textID < Data::currentTextes->size())
		return "\"" + Data::currentTextes->at(textID)->getShortText(Config::value("jp_txt", false).toBool()) + "\"";
	return QObject::tr("(Pas de texte)");
}

QString Opcode::_item(quint16 itemID, quint8 bank)
{
	if(bank > 0)	return QObject::tr("n°%1").arg(_bank(itemID & 0xFF, bank));

	if(itemID < 128){
		if(!Data::item_names.isEmpty() && itemID < Data::item_names.size())	return Data::item_names.at(itemID);
	}else if(itemID < 256){
		if(!Data::weapon_names.isEmpty() && itemID-128 < Data::weapon_names.size())		return Data::weapon_names.at(itemID-128);
	}else if(itemID < 288){
		if(!Data::armor_names.isEmpty() && itemID-256 < Data::armor_names.size())		return Data::armor_names.at(itemID-256);
	}else if(itemID < 320){
		if(!Data::accessory_names.isEmpty() && itemID-288 < Data::accessory_names.size())	return Data::accessory_names.at(itemID-288);
	}return QObject::tr("n°%1").arg(itemID);
}

QString Opcode::_materia(quint8 materiaID, quint8 bank)
{
	if(bank > 0)	return QObject::tr("n°%1").arg(_bank(materiaID, bank));

	if(materiaID < Data::materia_names.size())	return Data::materia_names.at(materiaID);
	return QObject::tr("n°%1").arg(materiaID);
}

QString Opcode::_field(quint16 fieldID)
{
	if(fieldID < Data::field_names.size())	return Data::field_names.at(fieldID)+QObject::tr(" (n°%1)").arg(fieldID);
	return QObject::tr("n°%1").arg(fieldID);
}

QString Opcode::_movie(quint8 movieID)
{
	if(movieID < Data::movie_names.size() && Data::movie_names.at(movieID)!="")	return Data::movie_names.at(movieID);
	return QObject::tr("n°%1").arg(movieID);
}

/* QString Opcode::_objet3D(quint8 objet3D_ID)
{
	if(objet3D_ID < Data::currentCharNames.size())	return QString("%1 (%2)").arg(Data::currentCharNames.at(objet3D_ID), Data::currentHrcNames.at(objet3D_ID));
	return QObject::tr("n°%1").arg(objet3D_ID);
} */

QString Opcode::_bank(quint8 adress, quint8 bank)
{
	if(Var::name(bank, adress)!="")	return Var::name(bank, adress);
	if(bank==0)		return QString("?");
	return QString("Var[%1][%2]").arg(bank).arg(adress);
}

QString Opcode::_var(int value, quint8 bank)
{
	if(bank > 0)	return _bank(value & 0xFF, bank);
	return QString::number(value);
}

QString Opcode::_var(int value, quint8 bank1, quint8 bank2)
{
	if(bank1 > 0 || bank2 > 0) {
		QString ret = _var(value & 0xFFFF, bank1);
		if(bank2 != 0 || ((value >> 16) & 0xFFFF) != 0) {
			ret += QString(" + ") + _var((value >> 16) & 0xFFFF, bank2) + QString(" * 65536 ");
		}
		return ret;
	}
	return QString::number(value);
}

QString Opcode::_var(int value, quint8 bank1, quint8 bank2, quint8 bank3)
{
	if(bank1 > 0 || bank2 > 0 || bank3 > 0)
		return _bank(value & 0xFF, bank1) + QObject::tr(" et ") + _bank((value >> 8) & 0xFF, bank2) + QObject::tr(" et ") + _bank((value >> 16) & 0xFF, bank3);
	return QString::number(value);
}

QString Opcode::_personnage(quint8 persoID)
{
	if(persoID < Data::char_names.size())	return Data::char_names.at(persoID);
	if(persoID >= 254)						return QObject::tr("(Vide)");
	if(persoID >= 100)						return Data::char_names.last();
	return QString("%1?").arg(persoID);
}

QString Opcode::_windowCorner(quint8 param, quint8 bank)
{
	if(bank>0)	return _bank(param, bank);

	switch(param)
	{
	case 0:		return QObject::tr("haut gauche");
	case 1:		return QObject::tr("bas gauche");
	case 2:		return QObject::tr("haut droit");
	case 3:		return QObject::tr("bas droit");
	default:	return QString("%1?").arg(param);
	}
}

QString Opcode::_sensRotation(quint8 param)
{
	switch(param)
	{
	case 1:		return QObject::tr("inverse");
	case 2:		return QObject::tr("inverse");
	default:	return QObject::tr("normal");
	}
}

QString Opcode::_operateur(quint8 param)
{
	if(param < Data::operateur_names.size())
		return Data::operateur_names.at(param);
	return QString("%1?").arg(param);
}

OpcodeUnknown::OpcodeUnknown(quint8 id, const QByteArray &params)
{
	_id = id;
	unknown = params;
}

int OpcodeUnknown::id() const
{
	return _id;
}

quint8 OpcodeUnknown::size() const
{
	return unknown.size() + 1;
}

QString OpcodeUnknown::toString() const
{
	return QObject::tr("? (id=%1)")
			.arg(_id);
}

void OpcodeUnknown::setParams(const QByteArray &params)
{
	unknown = params;
}

QByteArray OpcodeUnknown::params() const
{
	return unknown;
}

OpcodeRET::OpcodeRET()
{
}

QString OpcodeRET::toString() const
{
	return QObject::tr("Retourner");
}

OpcodeExec::OpcodeExec(const QByteArray &params)
{
	setParams(params);
}

void OpcodeExec::setParams(const QByteArray &params)
{
	groupID = params.at(0);
	scriptID = params.at(1) & 0x1F;
	priority = (params.at(1) >> 5) & 7;
}

QByteArray OpcodeExec::params() const
{
	return QByteArray()
			.append((char)groupID)
			.append(char((scriptID & 0x1F) | ((priority & 7) << 5)));
}

OpcodeREQ::OpcodeREQ(const QByteArray &params) :
	OpcodeExec(params)
{
}

QString OpcodeREQ::toString() const
{
	return QObject::tr("Exécuter le script n°%3 du groupe externe %1 (priorité %2/6) - Seulement si le script n'est pas déjà en cours d'exécution")
			.arg(_script(groupID))
			.arg(priority)
			.arg(scriptID);
}

OpcodeREQSW::OpcodeREQSW(const QByteArray &params) :
	OpcodeExec(params)
{
}

QString OpcodeREQSW::toString() const
{
	return QObject::tr("Exécuter le script n°%3 du groupe externe %1 (priorité %2/6)")
			.arg(_script(groupID))
			.arg(priority)
			.arg(scriptID);
}

OpcodeREQEW::OpcodeREQEW(const QByteArray &params) :
	OpcodeExec(params)
{
}

QString OpcodeREQEW::toString() const
{
	return QObject::tr("Exécuter le script n°%3 du groupe %1 (priorité %2/6) - Attend la fin de l'exécution pour continuer")
			.arg(_script(groupID))
			.arg(priority)
			.arg(scriptID);
}

OpcodeExecChar::OpcodeExecChar(const QByteArray &params)
{
	setParams(params);
}

void OpcodeExecChar::setParams(const QByteArray &params)
{
	partyID = params.at(0);
	scriptID = params.at(1) & 0x1F;
	priority = (params.at(1) >> 5) & 7;
}

QByteArray OpcodeExecChar::params() const
{
	return QByteArray()
			.append((char)partyID)
			.append(char((scriptID & 0x1F) | ((priority & 7) << 5)));
}

OpcodePREQ::OpcodePREQ(const QByteArray &params) :
	OpcodeExecChar(params)
{
}

QString OpcodePREQ::toString() const
{
	return QObject::tr("Exécuter le script n°%3 du groupe externe lié au personnage n°%1 de l'équipe (priorité %2/6) - Seulement si le script n'est pas déjà en cours d'exécution")
			.arg(partyID)
			.arg(priority)
			.arg(scriptID);
}

OpcodePRQSW::OpcodePRQSW(const QByteArray &params) :
	OpcodeExecChar(params)
{
}

QString OpcodePRQSW::toString() const
{
	return QObject::tr("Exécuter le script n°%3 du groupe externe lié au personnage n°%1 de l'équipe (priorité %2/6)")
			.arg(partyID)
			.arg(priority)
			.arg(scriptID);
}

OpcodePRQEW::OpcodePRQEW(const QByteArray &params) :
	OpcodeExecChar(params)
{
}

QString OpcodePRQEW::toString() const
{
	return QObject::tr("Exécuter le script n°%3 du groupe lié au personnage n°%1 de l'équipe (priorité %2/6) - Attend la fin de l'exécution pour continuer")
			.arg(partyID)
			.arg(priority)
			.arg(scriptID);
}

OpcodeRETTO::OpcodeRETTO(const QByteArray &params)
{
	setParams(params);
}

void OpcodeRETTO::setParams(const QByteArray &params)
{
	scriptID = params.at(0) & 0x1F;//ScriptID sur 5 bits
	priority = (params.at(0) >> 5) & 7;//Priorité sur 3 bits
}

QString OpcodeRETTO::toString() const
{
	return QObject::tr("Retourner et exécuter le script n°%2 du groupe appelant (priorité %1/6)")
			.arg(priority)
			.arg(scriptID);
}

QByteArray OpcodeRETTO::params() const
{
	return QByteArray()
			.append(char((scriptID & 0x1F) | ((priority & 7) << 5)));
}

OpcodeJOIN::OpcodeJOIN(const QByteArray &params)
{
	setParams(params);
}

void OpcodeJOIN::setParams(const QByteArray &params)
{
	speed = params.at(0);
}

QString OpcodeJOIN::toString() const
{
	return QObject::tr("Rassembler les membres de l'équipe dans le personnage jouable (vitesse=%1)")
			.arg(speed);
}

QByteArray OpcodeJOIN::params() const
{
	return QByteArray()
			.append(speed);
}

OpcodeSPLIT::OpcodeSPLIT(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSPLIT::setParams(const QByteArray &params)
{
	const char *constParams = params.constData();
	memcpy(banks, constParams, 3);

	memcpy(&targetX1, &constParams[3], 2); // bank 1
	memcpy(&targetY1, &constParams[5], 2); // bank 2
	direction1 = params.at(7); // bank 3
	memcpy(&targetX2, &constParams[8], 2); // bank 4
	memcpy(&targetY2, &constParams[10], 2); // bank 5
	direction2 = params.at(12); // bank 6
	speed = params.at(13);
}

QString OpcodeSPLIT::toString() const
{
	return QObject::tr("Faire sortir les membres de l'équipe à partir du personnage jouable (perso 1 : X=%1, Y=%2, dir=%3 ; perso 2 : X=%4, Y=%5, dir=%6) (vitesse %7)")
			.arg(_var(targetX1, B1(banks[0])))
			.arg(_var(targetY1, B2(banks[0])))
			.arg(_var(direction1, B1(banks[1])))
			.arg(_var(targetX2, B2(banks[1])))
			.arg(_var(targetY2, B1(banks[2])))
			.arg(_var(direction2, B2(banks[2])))
			.arg(speed);
}

QByteArray OpcodeSPLIT::params() const
{
	return QByteArray()
			.append((char *)&banks, 3)
			.append((char *)&targetX1, 2)
			.append((char *)&targetY1, 2)
			.append((char)direction1)
			.append((char *)&targetX2, 2)
			.append((char *)&targetY2, 2)
			.append((char)direction2)
			.append((char)speed);
}

void OpcodeSPLIT::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), targetX1 & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY1 & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), direction1 & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), targetX2 & 0xFF));
	if(B1(banks[2]) != 0)
		vars.append(FF7Var(B1(banks[2]), targetY2 & 0xFF));
	if(B2(banks[2]) != 0)
		vars.append(FF7Var(B2(banks[2]), direction2 & 0xFF));
}

OpcodePartyE::OpcodePartyE(const QByteArray &params)
{
	setParams(params);
}

void OpcodePartyE::setParams(const QByteArray &params)
{
	const char *constParams = params.constData();
	memcpy(banks, constParams, 2);

	party1 = params.at(2); // bank 1
	party2 = params.at(3); // bank 2
	party3 = params.at(4); // bank 3 -checked-
}

QByteArray OpcodePartyE::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)party1)
			.append((char)party2)
			.append((char)party3);
}

void OpcodePartyE::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), party1));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), party2));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), party3));
}

OpcodeSPTYE::OpcodeSPTYE(const QByteArray &params) :
	OpcodePartyE(params)
{
}

QString OpcodeSPTYE::toString() const
{
	return QObject::tr("Sauvegarder les membres de l'équipe : %1 | %2 | %3")
			.arg(_var(party1, B1(banks[0])))
			.arg(_var(party2, B2(banks[0])))
			.arg(_var(party3, B1(banks[1])));
}

OpcodeGTPYE::OpcodeGTPYE(const QByteArray &params) :
	OpcodePartyE(params)
{
}

QString OpcodeGTPYE::toString() const
{
	return QObject::tr("Récupérer les membres de l'équipe : %1 | %2 | %3")
			.arg(_var(party1, B1(banks[0])))
			.arg(_var(party2, B2(banks[0])))
			.arg(_var(party3, B1(banks[1])));
}

OpcodeDSKCG::OpcodeDSKCG(const QByteArray &params)
{
	setParams(params);
}

void OpcodeDSKCG::setParams(const QByteArray &params)
{
	diskID = params.at(0);
}

QString OpcodeDSKCG::toString() const
{
	return QObject::tr("Demander le CD %1")
			.arg(diskID);
}

QByteArray OpcodeDSKCG::params() const
{
	return QByteArray().append((char)diskID);
}

OpcodeSPECIALARROW::OpcodeSPECIALARROW(const QByteArray &params)
{
	setParams(params);
}

quint8 OpcodeSPECIALARROW::size() const
{
	return 2;
}

void OpcodeSPECIALARROW::setParams(const QByteArray &params)
{
	hide = params.at(0);// Boolean
}

QString OpcodeSPECIALARROW::toString() const
{
	return QObject::tr("%1 le curseur main")
			.arg(hide == 0 ? QObject::tr("Afficher") : QObject::tr("Ne pas afficher"));
}

QByteArray OpcodeSPECIALARROW::params() const
{
	return QByteArray().append((char)hide);
}

OpcodeSPECIALPNAME::OpcodeSPECIALPNAME(const QByteArray &params)
{
	setParams(params);
}

quint8 OpcodeSPECIALPNAME::size() const
{
	return 2;
}

void OpcodeSPECIALPNAME::setParams(const QByteArray &params)
{
	unknown = params.at(0);
}

QString OpcodeSPECIALPNAME::toString() const
{
	return QObject::tr("PNAME - Désactiver le menu de droite (%1)")
			.arg(unknown);
}

QByteArray OpcodeSPECIALPNAME::params() const
{
	return QByteArray().append((char)unknown);
}

OpcodeSPECIALGMSPD::OpcodeSPECIALGMSPD(const QByteArray &params)
{
	setParams(params);
}

quint8 OpcodeSPECIALGMSPD::size() const
{
	return 2;
}

void OpcodeSPECIALGMSPD::setParams(const QByteArray &params)
{
	speed = params.at(0);
}

QString OpcodeSPECIALGMSPD::toString() const
{
	return QObject::tr("Modifier la vitesse de jeu (%1)")
			.arg(speed);
}

QByteArray OpcodeSPECIALGMSPD::params() const
{
	return QByteArray().append((char)speed);
}

OpcodeSPECIALSMSPD::OpcodeSPECIALSMSPD(const QByteArray &params)
{
	setParams(params);
}

quint8 OpcodeSPECIALSMSPD::size() const
{
	return 3;
}

void OpcodeSPECIALSMSPD::setParams(const QByteArray &params)
{
	unknown = params.at(0);
	speed = params.at(1);
}

QString OpcodeSPECIALSMSPD::toString() const
{
	return QObject::tr("Modifier la vitesse des messages (%2) | %1 |")
			.arg(unknown)
			.arg(speed);
}

QByteArray OpcodeSPECIALSMSPD::params() const
{
	return QByteArray()
			.append((char)unknown)
			.append((char)speed);
}

OpcodeSPECIALFLMAT::OpcodeSPECIALFLMAT()
{
}

quint8 OpcodeSPECIALFLMAT::size() const
{
	return 1;
}

QString OpcodeSPECIALFLMAT::toString() const
{
	return QObject::tr("Remplir le menu matéria de toutes les matérias en quantité maximum");
}

OpcodeSPECIALFLITM::OpcodeSPECIALFLITM()
{
}

quint8 OpcodeSPECIALFLITM::size() const
{
	return 1;
}

QString OpcodeSPECIALFLITM::toString() const
{
	return QObject::tr("Remplir l'inventaire par tous les objets en quantité maximum");
}


OpcodeSPECIALBTLCK::OpcodeSPECIALBTLCK(const QByteArray &params)
{
	setParams(params);
}

quint8 OpcodeSPECIALBTLCK::size() const
{
	return 2;
}

void OpcodeSPECIALBTLCK::setParams(const QByteArray &params)
{
	lock = params.at(0); // Boolean
}

QString OpcodeSPECIALBTLCK::toString() const
{
	return QObject::tr("%1 les combats")
			.arg(lock == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeSPECIALBTLCK::params() const
{
	return QByteArray().append((char)lock);
}

OpcodeSPECIALMVLCK::OpcodeSPECIALMVLCK(const QByteArray &params)
{
	setParams(params);
}

quint8 OpcodeSPECIALMVLCK::size() const
{
	return 2;
}

void OpcodeSPECIALMVLCK::setParams(const QByteArray &params)
{
	lock = params.at(0); // Boolean
}

QString OpcodeSPECIALMVLCK::toString() const
{
	return QObject::tr("%1 les cinématiques")
			.arg(lock == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeSPECIALMVLCK::params() const
{
	return QByteArray().append((char)lock);
}

OpcodeSPECIALSPCNM::OpcodeSPECIALSPCNM(const QByteArray &params)
{
	setParams(params);
}

quint8 OpcodeSPECIALSPCNM::size() const
{
	return 3;
}

void OpcodeSPECIALSPCNM::setParams(const QByteArray &params)
{
	charID = params.at(0);
	textID = params.at(1);
}

QString OpcodeSPECIALSPCNM::toString() const
{
	return QObject::tr("Changer le nom de %1 par le texte %2")
			.arg(_personnage(charID))
			.arg(_text(textID));
}

QByteArray OpcodeSPECIALSPCNM::params() const
{
	return QByteArray()
			.append((char)charID)
			.append((char)textID);
}

int OpcodeSPECIALSPCNM::getTextID() const
{
	return textID;
}

void OpcodeSPECIALSPCNM::setTextID(quint8 textID)
{
	this->textID = textID;
}

OpcodeSPECIALRSGLB::OpcodeSPECIALRSGLB()
{
}

quint8 OpcodeSPECIALRSGLB::size() const
{
	return 1;
}

QString OpcodeSPECIALRSGLB::toString() const
{
	return QObject::tr("Met le temps à 0, débloque le menu \"PHS\" et \"Sauvegarder\". Nouvelle équipe : Clad | (Vide) | (Vide)");
}

OpcodeSPECIALCLITM::OpcodeSPECIALCLITM()
{
}

quint8 OpcodeSPECIALCLITM::size() const
{
	return 1;
}

QString OpcodeSPECIALCLITM::toString() const
{
	return QObject::tr("Supprimer tous les objets de l'inventaire");
}

OpcodeSPECIAL::OpcodeSPECIAL(const QByteArray &params) :
	opcode(0)
{
	setParams(params);
}

OpcodeSPECIAL::~OpcodeSPECIAL()
{
	delete opcode;
}

quint8 OpcodeSPECIAL::size() const
{
	return 1 + opcode->size();
}

QString OpcodeSPECIAL::toString() const
{
	return QObject::tr("SPECIAL - ") + opcode->toString();
}

void OpcodeSPECIAL::setParams(const QByteArray &params)
{
	if(opcode)		delete opcode;

	switch((quint8)params.at(0))
	{
	case 0xF5:	opcode = new OpcodeSPECIALARROW(params.mid(1));
		break;
	case 0xF6:	opcode = new OpcodeSPECIALPNAME(params.mid(1));
		break;
	case 0xF7:	opcode = new OpcodeSPECIALGMSPD(params.mid(1));
		break;
	case 0xF8:	opcode = new OpcodeSPECIALSMSPD(params.mid(1));
		break;
	case 0xF9:	opcode = new OpcodeSPECIALFLMAT();
		break;
	case 0xFA:	opcode = new OpcodeSPECIALFLITM();
		break;
	case 0xFB:	opcode = new OpcodeSPECIALBTLCK(params.mid(1));
		break;
	case 0xFC:	opcode = new OpcodeSPECIALMVLCK(params.mid(1));
		break;
	case 0xFD:	opcode = new OpcodeSPECIALSPCNM(params.mid(1));
		break;
	case 0xFE:	opcode = new OpcodeSPECIALRSGLB();
		break;
	case 0xFF:	opcode = new OpcodeSPECIALCLITM();
		break;
	default:	opcode = new OpcodeUnknown(params.at(0));
		break;
	}
}

QByteArray OpcodeSPECIAL::params() const
{
	return opcode->toByteArray();
}

int OpcodeSPECIAL::getTextID() const
{
	return opcode->getTextID();
}

void OpcodeSPECIAL::setTextID(quint8 textID)
{
	opcode->setTextID(textID);
}

OpcodeJump::OpcodeJump() :
	_jump(0), _label(0)
{
}

qint32 OpcodeJump::jump() const
{
	return _jump;
}

void OpcodeJump::setJump(qint32 jump)
{
	_jump = jump;
}

quint32 OpcodeJump::label() const
{
	return _label;
}

void OpcodeJump::setLabel(quint32 label)
{
	_label = label;
}

bool OpcodeJump::isJump() const
{
	return true;
}

OpcodeLabel::OpcodeLabel(quint32 label) :
	_label(label)
{
}

QString OpcodeLabel::toString() const
{
	return QObject::tr("Label %1")
			.arg(_label);
}

bool OpcodeLabel::isLabel() const
{
	return true;
}

quint32 OpcodeLabel::label() const
{
	return _label;
}

void OpcodeLabel::setLabel(quint32 label)
{
	_label = label;
}

OpcodeJMPF::OpcodeJMPF(const QByteArray &params) :
	OpcodeJump()
{
	setParams(params);
}

OpcodeJMPF::OpcodeJMPF(const OpcodeJump &op) :
	OpcodeJump(op)
{
}

void OpcodeJMPF::setParams(const QByteArray &params)
{
	_jump = (quint8)params.at(0) + jumpPosData();
}

QString OpcodeJMPF::toString() const
{
	return QObject::tr("Aller au label %1")
			.arg(_label);
}

QByteArray OpcodeJMPF::params() const
{
	return QByteArray().append(char(_jump - jumpPosData()));
}

OpcodeJMPFL::OpcodeJMPFL(const QByteArray &params) :
	OpcodeJump()
{
	setParams(params);
}

OpcodeJMPFL::OpcodeJMPFL(const OpcodeJump &op) :
	OpcodeJump(op)
{
}

void OpcodeJMPFL::setParams(const QByteArray &params)
{
	quint16 jump;
	memcpy(&jump, params.constData(), 2);

	_jump = jump + jumpPosData();
}

QString OpcodeJMPFL::toString() const
{
	return QObject::tr("Aller au label %1")
			.arg(_label);
}

QByteArray OpcodeJMPFL::params() const
{
	quint16 jump = _jump - jumpPosData();
	return QByteArray().append((char *)&jump, 2);
}

OpcodeJMPB::OpcodeJMPB(const QByteArray &params) :
	OpcodeJump()
{
	setParams(params);
}

OpcodeJMPB::OpcodeJMPB(const OpcodeJump &op) :
	OpcodeJump(op)
{
}

void OpcodeJMPB::setParams(const QByteArray &params)
{
	_jump = -(quint8)params.at(0);
}

QString OpcodeJMPB::toString() const
{
	return QObject::tr("Aller au label %1")
			.arg(int(_label));
}

QByteArray OpcodeJMPB::params() const
{
	return QByteArray().append(char(-_jump));
}

OpcodeJMPBL::OpcodeJMPBL(const QByteArray &params) :
	OpcodeJump()
{
	setParams(params);
}

OpcodeJMPBL::OpcodeJMPBL(const OpcodeJump &op) :
	OpcodeJump(op)
{
}

void OpcodeJMPBL::setParams(const QByteArray &params)
{
	quint16 jump;
	memcpy(&jump, params.constData(), 2);

	_jump = -jump;
}

QString OpcodeJMPBL::toString() const
{
	return QObject::tr("Aller au label %1")
			.arg(_label);
}

QByteArray OpcodeJMPBL::params() const
{
	quint16 jump = -_jump;
	return QByteArray().append((char *)&jump, 2);
}

OpcodeIf::OpcodeIf() :
	OpcodeJump(), banks(0), value1(0), value2(0), oper(0)
{
}

OpcodeIf::OpcodeIf(const OpcodeJump &op) :
	OpcodeJump(op), banks(0), value1(0), value2(0), oper(0)
{
}

void OpcodeIf::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), value1 & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), value2 & 0xFF));
}

OpcodeIFUB::OpcodeIFUB(const QByteArray &params) :
	OpcodeIf()
{
	setParams(params);
}

void OpcodeIFUB::setParams(const QByteArray &params)
{
	banks = (quint8)params.at(0);
	value1 = (quint8)params.at(1); // bank 1
	value2 = (quint8)params.at(2); // bank 2
	oper = (quint8)params.at(3);
	_jump = (quint8)params.at(4) + jumpPosData();
}

QString OpcodeIFUB::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller au label %4 sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label)
			.arg(oper==9 || oper==10 ? ")" : "");//mini hack ")"
}

QByteArray OpcodeIFUB::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)value1)
			.append((char)value2)
			.append((char)oper)
			.append(char(_jump - jumpPosData()));
}

OpcodeIFUBL::OpcodeIFUBL(const QByteArray &params)
{
	setParams(params);
}

OpcodeIFUBL::OpcodeIFUBL(const OpcodeIFUB &op) :
	OpcodeIf(op)
{
}

void OpcodeIFUBL::setParams(const QByteArray &params)
{
	const char *constParams = params.constData();
	banks = (quint8)params.at(0);
	value1 = (quint8)params.at(1); // bank 1
	value2 = (quint8)params.at(2); // bank 2
	oper = (quint8)params.at(3);
	quint16 jump;
	memcpy(&jump, &constParams[4], 2);
	_jump = jump + jumpPosData();
}

QString OpcodeIFUBL::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller au label %4 sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label)
			.arg(oper==9 || oper==10 ? ")" : "");//mini hack ")"
}

QByteArray OpcodeIFUBL::params() const
{
	quint16 jump = _jump - jumpPosData();
	return QByteArray()
			.append((char)banks)
			.append((char)value1)
			.append((char)value2)
			.append((char)oper)
			.append((char *)&jump, 2);
}

OpcodeIFSW::OpcodeIFSW(const QByteArray &params)
{
	setParams(params);
}

void OpcodeIFSW::setParams(const QByteArray &params)
{
	const char *constParams = params.constData();
	banks = (quint8)params.at(0);
	qint16 v1, v2;
	memcpy(&v1, &constParams[1], 2);
	memcpy(&v2, &constParams[3], 2);
	value1 = v1;
	value2 = v2;
	oper = (quint8)params.at(5);
	_jump = (quint8)params.at(6) + jumpPosData();
}

QString OpcodeIFSW::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller au label %4 sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label)
			.arg(oper==9 || oper==10 ? ")" : "");//mini hack ")"
}

QByteArray OpcodeIFSW::params() const
{
	qint16 v1=value1, v2=value2;
	return QByteArray()
			.append((char)banks)
			.append((char *)&v1, 2)
			.append((char *)&v2, 2)
			.append((char)oper)
			.append(char(_jump - jumpPosData()));
}

OpcodeIFSWL::OpcodeIFSWL(const QByteArray &params)
{
	setParams(params);
}

OpcodeIFSWL::OpcodeIFSWL(const OpcodeIFSW &op) :
	OpcodeIf(op)
{
}

void OpcodeIFSWL::setParams(const QByteArray &params)
{
	const char *constParams = params.constData();
	banks = (quint8)params.at(0);
	qint16 v1, v2;
	memcpy(&v1, &constParams[1], 2);
	memcpy(&v2, &constParams[3], 2);
	value1 = v1;
	value2 = v2;
	oper = (quint8)params.at(5);
	quint16 jump;
	memcpy(&jump, &constParams[6], 2);
	_jump = jump + jumpPosData();
}

QString OpcodeIFSWL::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller au label %4 sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label)
			.arg(oper==9 || oper==10 ? ")" : "");//mini hack ")"
}

QByteArray OpcodeIFSWL::params() const
{
	qint16 v1=value1, v2=value2;
	quint16 jump = _jump - jumpPosData();
	return QByteArray()
			.append((char)banks)
			.append((char *)&v1, 2)
			.append((char *)&v2, 2)
			.append((char)oper)
			.append((char *)&jump, 2);
}

OpcodeIFUW::OpcodeIFUW(const QByteArray &params)
{
	setParams(params);
}

void OpcodeIFUW::setParams(const QByteArray &params)
{
	const char *constParams = params.constData();
	banks = (quint8)params.at(0);
	quint16 v1, v2;
	memcpy(&v1, &constParams[1], 2);
	memcpy(&v2, &constParams[3], 2);
	value1 = v1;
	value2 = v2;
	oper = (quint8)params.at(5);
	_jump = (quint8)params.at(6) + jumpPosData();
}

QString OpcodeIFUW::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller au label %4 sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label)
			.arg(oper==9 || oper==10 ? ")" : "");//mini hack ")"
}

QByteArray OpcodeIFUW::params() const
{
	quint16 v1=value1, v2=value2;
	return QByteArray()
			.append((char)banks)
			.append((char *)&v1, 2)
			.append((char *)&v2, 2)
			.append((char)oper)
			.append(char(_jump - jumpPosData()));
}

OpcodeIFUWL::OpcodeIFUWL(const QByteArray &params)
{
	setParams(params);
}


OpcodeIFUWL::OpcodeIFUWL(const OpcodeIFUW &op) :
	OpcodeIf(op)
{
}

void OpcodeIFUWL::setParams(const QByteArray &params)
{
	const char *constParams = params.constData();
	banks = (quint8)params.at(0);
	quint16 v1, v2;
	memcpy(&v1, &constParams[1], 2);
	memcpy(&v2, &constParams[3], 2);
	value1 = v1;
	value2 = v2;
	oper = (quint8)params.at(5);
	quint16 jump;
	memcpy(&jump, &constParams[6], 2);
	_jump = jump + jumpPosData();
}

QString OpcodeIFUWL::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller au label %4 sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label)
			.arg(oper==9 || oper==10 ? ")" : "");//mini hack ")"
}

QByteArray OpcodeIFUWL::params() const
{
	quint16 v1=value1, v2=value2;
	quint16 jump = _jump - jumpPosData();
	return QByteArray()
			.append((char)banks)
			.append((char *)&v1, 2)
			.append((char *)&v2, 2)
			.append((char)oper)
			.append((char *)&jump, 2);
}

OpcodeMINIGAME::OpcodeMINIGAME(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMINIGAME::setParams(const QByteArray &params)
{
	const char *constParams = params.constData();
	memcpy(&fieldID, constParams, 2);
	memcpy(&targetX, &constParams[2], 2);
	memcpy(&targetY, &constParams[4], 2);
	memcpy(&targetI, &constParams[6], 2);
	minigameParam = (quint8)params.at(8);
	minigameID = (quint8)params.at(9);
}

QString OpcodeMINIGAME::toString() const
{
	QString miniGame;
	switch(minigameID)
	{
	case 0x00:		miniGame = QObject::tr("Course de moto (paramètre %1)").arg(minigameParam);break;
	case 0x01:		miniGame = QObject::tr("Course de chocobo (paramètre %1)").arg(minigameParam);break;
	case 0x02:		miniGame = QObject::tr("Descente en snowboard -mode normal- (paramètre %1)").arg(minigameParam);break;
	case 0x03:		miniGame = QObject::tr("Fort Condor (paramètre %1)").arg(minigameParam);break;
	case 0x04:		miniGame = QObject::tr("Sous-marin (paramètre %1)").arg(minigameParam);break;
	case 0x05:		miniGame = QObject::tr("Speed Square (paramètre %1)").arg(minigameParam);break;
	case 0x06:		miniGame = QObject::tr("Descente en snowboard -mode Gold Saucer- (paramètre %1)").arg(minigameParam);break;
	default:		miniGame = QObject::tr("%1? (paramètre %2)").arg(minigameID).arg(minigameParam);break;
	}

	return QObject::tr("Lancer un mini-jeu : %5 (Après le jeu aller à l'écran %1 (X=%2, Y=%3, polygone id=%4))")
			.arg(_field(fieldID))
			.arg(targetX)
			.arg(targetY)
			.arg(targetI)
			.arg(miniGame);
}

QByteArray OpcodeMINIGAME::params() const
{
	return QByteArray()
			.append((char *)&fieldID, 2)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&targetI, 2)
			.append((char)minigameParam)
			.append((char)minigameID);
}

OpcodeTUTOR::OpcodeTUTOR(const QByteArray &params)
{
	setParams(params);
}

void OpcodeTUTOR::setParams(const QByteArray &params)
{
	tutoID = (quint8)params.at(0);
}

QString OpcodeTUTOR::toString() const
{
	return QObject::tr("Lancer le tutoriel n°%1")
			.arg(tutoID);
}

QByteArray OpcodeTUTOR::params() const
{
	return QByteArray().append((char)tutoID);
}

int OpcodeTUTOR::getTutoID() const
{
	return tutoID;
}

void OpcodeTUTOR::setTutoID(quint8 tutoID)
{
	this->tutoID = tutoID;
}

OpcodeBTMD2::OpcodeBTMD2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBTMD2::setParams(const QByteArray &params)
{
	memcpy(&battleMode, params.constData(), 4);
}

QString OpcodeBTMD2::toString() const
{
	QStringList modes;
	for(quint8 i=0 ; i<32 ; ++i)
	{
		if((battleMode >> i) & 1)
		{
			switch(i)
			{
			case 1:		modes.append(QObject::tr("Compte à rebours"));											break;
			case 2:		modes.append(QObject::tr("Attaque préventive"));										break;
			case 3:		modes.append(QObject::tr("Impossible de fuir"));										break;
			case 5:		modes.append(QObject::tr("Ne pas jouer Fanfare"));										break;
			case 6:		modes.append(QObject::tr("Active la battle arena"));									break;
			case 7:		modes.append(QObject::tr("Ne pas afficher d'écran de récompense"));						break;
			case 8:		modes.append(QObject::tr("Les personnages ne font pas leur animation de victoire"));	break;
			case 23:	modes.append(QObject::tr("Désactiver Game Over"));										break;
			default:	modes.append(QString("%1?").arg(i));													break;
			}
		}
	}

	return QObject::tr("Mode de combat : %1").arg(modes.join(", "));
}

QByteArray OpcodeBTMD2::params() const
{
	return QByteArray().append((char *)&battleMode, 4);
}

OpcodeBTRLD::OpcodeBTRLD(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBTRLD::setParams(const QByteArray &params)
{
	banks = (quint8)params.at(0);
	var = (quint8)params.at(1); // bank 2
}

QString OpcodeBTRLD::toString() const
{
	return QObject::tr("Stocker le résultat du dernier combat dans %1")
			.arg(_bank(var, B2(banks)));
}

QByteArray OpcodeBTRLD::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)var);
}

void OpcodeBTRLD::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), var));
}

OpcodeWAIT::OpcodeWAIT(const QByteArray &params)
{
	setParams(params);
}

void OpcodeWAIT::setParams(const QByteArray &params)
{
	memcpy(&frameCount, params.constData(), 2);
}

QString OpcodeWAIT::toString() const
{
	return QObject::tr("Attendre %1 img")
			.arg(frameCount);
}

QByteArray OpcodeWAIT::params() const
{
	return QByteArray().append((char *)&frameCount, 2);
}

OpcodeNFADE::OpcodeNFADE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeNFADE::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 2);
	unknown1 = params.at(2);
	r = params.at(3); // bank 1 ?
	g = params.at(4); // bank 2 ?
	b = params.at(5); // bank 3 ?
	unknown2 = params.at(6);
	unknown3 = params.at(7);
}

QString OpcodeNFADE::toString() const
{
	return QObject::tr("Voiler l'écran avec la couleur RVB(%1, %2, %3)")
			.arg(_var(r, B1(banks[0])))
			.arg(_var(g, B2(banks[0])))
			.arg(_var(b, B1(banks[1])));
}

QByteArray OpcodeNFADE::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)unknown1)
			.append((char)r)
			.append((char)g)
			.append((char)b)
			.append((char)unknown2)
			.append((char)unknown3);
}

void OpcodeNFADE::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), r & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), g & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), b & 0xFF));
}

OpcodeBLINK::OpcodeBLINK(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBLINK::setParams(const QByteArray &params)
{
	closed = params.at(0); // boolean
}

QString OpcodeBLINK::toString() const
{
	return QObject::tr("L'objet 3D cligne des yeux : %1")
			.arg(closed == 0 ? QObject::tr("ON") : QObject::tr("OFF"));
}

QByteArray OpcodeBLINK::params() const
{
	return QByteArray().append((char)closed);
}

OpcodeBGMOVIE::OpcodeBGMOVIE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBGMOVIE::setParams(const QByteArray &params)
{
	disabled = params.at(0); // boolean
}

QString OpcodeBGMOVIE::toString() const
{
	return QObject::tr("BGMOVIE : %1")
			.arg(disabled == 0 ? QObject::tr("ON") : QObject::tr("OFF"));
}

QByteArray OpcodeBGMOVIE::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodeKAWAIEYETX::OpcodeKAWAIEYETX(const QByteArray &params) :
	OpcodeUnknown(0x00, params)
{
}

QString OpcodeKAWAIEYETX::toString() const
{
	return QObject::tr("EYETX : Changer l'état de la texture des yeux ou de la bouche");
}

OpcodeKAWAITRNSP::OpcodeKAWAITRNSP(const QByteArray &params)
{
	setParams(params);
}

quint8 OpcodeKAWAITRNSP::size() const
{
	return 2 + data.size();
}

void OpcodeKAWAITRNSP::setParams(const QByteArray &params)
{
	enableTransparency = params.at(0);
	data = params.mid(1);
}

QString OpcodeKAWAITRNSP::toString() const
{
	return QObject::tr("%1 transparence")
			.arg(enableTransparency == 0 ? QObject::tr("Désactiver") : QObject::tr("Activer"));
}

QByteArray OpcodeKAWAITRNSP::params() const
{
	return QByteArray()
			.append(enableTransparency)
			.append(data);
}

OpcodeKAWAIAMBNT::OpcodeKAWAIAMBNT(const QByteArray &params) :
	OpcodeUnknown(0x02, params)
{
}

QString OpcodeKAWAIAMBNT::toString() const
{
	return QObject::tr("AMBNT");
}

OpcodeKAWAILIGHT::OpcodeKAWAILIGHT(const QByteArray &params) :
	OpcodeUnknown(0x06, params)
{
}

QString OpcodeKAWAILIGHT::toString() const
{
	return QObject::tr("LIGHT");
}

OpcodeKAWAISBOBJ::OpcodeKAWAISBOBJ(const QByteArray &params) :
	OpcodeUnknown(0x0A, params)
{
}

QString OpcodeKAWAISBOBJ::toString() const
{
	return QObject::tr("SBOBJ");
}

OpcodeKAWAISHINE::OpcodeKAWAISHINE(const QByteArray &params) :
	OpcodeUnknown(0x0D, params)
{
}

QString OpcodeKAWAISHINE::toString() const
{
	return QObject::tr("SHINE");
}

OpcodeKAWAIRESET::OpcodeKAWAIRESET(const QByteArray &params) :
	OpcodeUnknown(0xFF, params)
{
}

QString OpcodeKAWAIRESET::toString() const
{
	return QObject::tr("RESET");
}

OpcodeKAWAI::OpcodeKAWAI(const QByteArray &params) :
	opcode(0)
{
	setParams(params);
}

OpcodeKAWAI::~OpcodeKAWAI()
{
	delete opcode;
}

quint8 OpcodeKAWAI::size() const
{
	return opcode->size() + 2;
}

QString OpcodeKAWAI::toString() const
{
	return QObject::tr("Filtre graphique sur l'objet 3D - %1")
			.arg(opcode->toString());
}

void OpcodeKAWAI::setParams(const QByteArray &params)
{
	if(opcode)		delete opcode;

//	size = params.at(0);
	switch((quint8)params.at(1))
	{
	case 0x00:	opcode = new OpcodeKAWAIEYETX(params.mid(2));
		break;
	case 0x01:	opcode = new OpcodeKAWAITRNSP(params.mid(2));
		break;
	case 0x02:	opcode = new OpcodeKAWAIAMBNT(params.mid(2));
		break;
	case 0x06:	opcode = new OpcodeKAWAILIGHT(params.mid(2));
		break;
	case 0x0A:	opcode = new OpcodeKAWAISBOBJ(params.mid(2));
		break;
	case 0x0D:	opcode = new OpcodeKAWAISHINE(params.mid(2));
		break;
	case 0xFF:	opcode = new OpcodeKAWAIRESET(params.mid(2));
		break;
	default:	opcode = new OpcodeUnknown((quint8)params.at(1), params.mid(2));
		break;
	}
}

QByteArray OpcodeKAWAI::params() const
{
	quint8 size = 2 + opcode->size();
	return QByteArray()
			.append((char)size)
			.append(opcode->toByteArray());
}

OpcodeKAWIW::OpcodeKAWIW()
{
}

QString OpcodeKAWIW::toString() const
{
	return QObject::tr("Attendre la fin de l'exécution du filtre graphique");
}

OpcodePMOVA::OpcodePMOVA(const QByteArray &params)
{
	setParams(params);
}

void OpcodePMOVA::setParams(const QByteArray &params)
{
	partyID = params.at(0);
}

QString OpcodePMOVA::toString() const
{
	return QObject::tr("Déplacer l'objet 3D vers le membre n°%1 de l'équipe")
			.arg(partyID);
}

QByteArray OpcodePMOVA::params() const
{
	return QByteArray().append((char)partyID);
}

OpcodeSLIP::OpcodeSLIP(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSLIP::setParams(const QByteArray &params)
{
	off = params.at(0); // Boolean
}

QString OpcodeSLIP::toString() const
{
	return QObject::tr("SLIP : %1")
			.arg(off == 0 ? QObject::tr("ON") : QObject::tr("OFF"));
}

QByteArray OpcodeSLIP::params() const
{
	return QByteArray().append((char)off);
}

OpcodeBGPDH::OpcodeBGPDH(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBGPDH::setParams(const QByteArray &params)
{
	banks = params.at(0);
	layerID = params.at(1);
	memcpy(&targetZ, &(params.constData()[2]), 2); // bank 2 ???
}

QString OpcodeBGPDH::toString() const
{
	return QObject::tr("Déplacer la couche %1 du décor (Z=%2)")
			.arg(layerID)
			.arg(_var(targetZ, B2(banks)));
}

QByteArray OpcodeBGPDH::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)layerID)
			.append((char *)&targetZ, 2);
}

void OpcodeBGPDH::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), targetZ & 0xFF));
}

OpcodeBGSCR::OpcodeBGSCR(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBGSCR::setParams(const QByteArray &params)
{
	banks = params.at(0);
	layerID = params.at(1);
	memcpy(&targetX, &(params.constData()[2]), 2); // bank 1
	memcpy(&targetY, &(params.constData()[4]), 2); // bank 2
}

QString OpcodeBGSCR::toString() const
{
	return QObject::tr("Animer la couche %1 du décor (horizontalement=%2, verticalement=%3)")
			.arg(layerID)
			.arg(_var(targetX, B1(banks)))
			.arg(_var(targetY, B2(banks)));
}

QByteArray OpcodeBGSCR::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)layerID)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2);
}

void OpcodeBGSCR::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), targetX & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), targetY & 0xFF));
}

OpcodeWCLS::OpcodeWCLS(const QByteArray &params)
{
	setParams(params);
}

void OpcodeWCLS::setParams(const QByteArray &params)
{
	windowID = params.at(0);
}

QString OpcodeWCLS::toString() const
{
	return QObject::tr("WCLS (fenêtre n°%1)")
			.arg(windowID);
}

QByteArray OpcodeWCLS::params() const
{
	return QByteArray().append((char)windowID);
}

int OpcodeWCLS::getWindowID() const
{
	return windowID;
}

void OpcodeWCLS::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

OpcodeWSIZW::OpcodeWSIZW(const QByteArray &params)
{
	setParams(params);
}

void OpcodeWSIZW::setParams(const QByteArray &params)
{
	windowID = params.at(0);
	memcpy(&targetX, &(params.constData()[1]), 2);
	memcpy(&targetY, &(params.constData()[3]), 2);
	memcpy(&width, &(params.constData()[5]), 2);
	memcpy(&height, &(params.constData()[7]), 2);
}

QString OpcodeWSIZW::toString() const
{
	return QObject::tr("Redimensionner fenêtre n°%1 (X=%2, Y=%3, largeur=%4, hauteur=%5)")
			.arg(windowID)
			.arg(targetX)
			.arg(targetY)
			.arg(width)
			.arg(height);
}

QByteArray OpcodeWSIZW::params() const
{
	return QByteArray()
			.append((char)windowID)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&width, 2)
			.append((char *)&height, 2);
}

int OpcodeWSIZW::getWindowID() const
{
	return windowID;
}

void OpcodeWSIZW::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

bool OpcodeWSIZW::getWindow(FF7Window &window) const
{
	window.x = targetX;
	window.y = targetY;
	window.w = width;
	window.h = height;

	return true;
}

void OpcodeWSIZW::setWindow(const FF7Window &window)
{
	targetX = window.x;
	targetY = window.y;
	width = window.w;
	height = window.h;
}

OpcodeIfKey::OpcodeIfKey(const QByteArray &params)
{
	setParams(params);
}

void OpcodeIfKey::setParams(const QByteArray &params)
{
	memcpy(&keys, params.constData(), 2);
	_jump = (quint8)params.at(2) + jumpPosData();
}

QByteArray OpcodeIfKey::params() const
{
	return QByteArray()
			.append((char *)&keys, 2)
			.append(char(_jump - jumpPosData()));
}

QString OpcodeIfKey::keyString() const
{
	QStringList ret;
	for(quint8 i=0 ; i<16 ; ++i) {
		if((keys >> i) & 1) {
			ret.append(Data::key_names.at(i));
		}
	}
	return ret.join(QObject::tr(" ou "));
}

OpcodeIFKEY::OpcodeIFKEY(const QByteArray &params) :
	OpcodeIfKey(params)
{
}

QString OpcodeIFKEY::toString() const
{
	return QObject::tr("Si appuie sur la touche %1 (aller au label %2 sinon)")
			.arg(keyString())
			.arg(_label);
}

OpcodeIFKEYON::OpcodeIFKEYON(const QByteArray &params) :
	OpcodeIfKey(params)
{
}

QString OpcodeIFKEYON::toString() const
{
	return QObject::tr("Si appuie sur la touche %1 une fois (aller au label %2 sinon)")
			.arg(keyString())
			.arg(_label);
}

OpcodeIFKEYOFF::OpcodeIFKEYOFF(const QByteArray &params) :
	OpcodeIfKey(params)
{
}

QString OpcodeIFKEYOFF::toString() const
{
	return QObject::tr("Si relache la touche %1 pour la première fois (aller au label %2 sinon)")
			.arg(keyString())
			.arg(_label);
}

OpcodeUC::OpcodeUC(const QByteArray &params)
{
	setParams(params);
}

void OpcodeUC::setParams(const QByteArray &params)
{
	disabled = params.at(0);
}

QString OpcodeUC::toString() const
{
	return QObject::tr("%1 les déplacements du personnage jouable")
			.arg(disabled == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeUC::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodePDIRA::OpcodePDIRA(const QByteArray &params)
{
	setParams(params);
}

void OpcodePDIRA::setParams(const QByteArray &params)
{
	partyID = params.at(0);
}

QString OpcodePDIRA::toString() const
{
	return QObject::tr("Tourner instantanément l'objet 3D vers le membre de l'équipe n°%1")
			.arg(partyID);
}

QByteArray OpcodePDIRA::params() const
{
	return QByteArray().append((char)partyID);
}

OpcodePTURA::OpcodePTURA(const QByteArray &params)
{
	setParams(params);
}

void OpcodePTURA::setParams(const QByteArray &params)
{
	partyID = params.at(0);
	speed = params.at(1);
	directionRotation = params.at(2);
}

QString OpcodePTURA::toString() const
{
	return QObject::tr("Tourner l'objet 3D vers le membre de l'équipe n°%1 (Vitesse=%2, SensRotation=%3)")
			.arg(partyID)
			.arg(speed)
			.arg(_sensRotation(directionRotation));
}

QByteArray OpcodePTURA::params() const
{
	return QByteArray()
			.append((char)partyID)
			.append((char)speed)
			.append((char)directionRotation);
}

OpcodeWSPCL::OpcodeWSPCL(const QByteArray &params)
{
	setParams(params);
}

void OpcodeWSPCL::setParams(const QByteArray &params)
{
	windowID = params.at(0);
	displayType = params.at(1);
	marginLeft = params.at(2);
	marginTop = params.at(3);
}

QString OpcodeWSPCL::toString() const
{
	QString windowNum;
	switch(displayType)
	{
	case 0x00:		windowNum = QObject::tr("(vide)");					break;
	case 0x01:		windowNum = QObject::tr("Horloge");					break;
	case 0x02:		windowNum = QObject::tr("Affichage numérique");		break;
	default:		windowNum = QString("%1?").arg(displayType);		break;
	}

	return QObject::tr("%2 dans fenêtre n°%1 (gauche=%3, haut=%4)")
			.arg(windowID)
			.arg(windowNum)
			.arg(marginLeft)
			.arg(marginTop);
}

QByteArray OpcodeWSPCL::params() const
{
	return QByteArray()
			.append((char)windowID)
			.append((char)displayType)
			.append((char)marginLeft)
			.append((char)marginTop);
}

int OpcodeWSPCL::getWindowID() const
{
	return windowID;
}

void OpcodeWSPCL::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

OpcodeWNUMB::OpcodeWNUMB(const QByteArray &params)
{
	setParams(params);
}

void OpcodeWNUMB::setParams(const QByteArray &params)
{
	banks = params.at(0);
	windowID = params.at(1);
	memcpy(&value, &(params.constData()[2]), 4);// bank 1 and 2
	digitCount = params.at(6);
}

QString OpcodeWNUMB::toString() const
{
	return QObject::tr("Affecter %2 dans la fenêtre n°%1 et afficher %3 chiffres")
			.arg(windowID)
			.arg(_var(value, B1(banks), B2(banks)))
			.arg(digitCount);
}

QByteArray OpcodeWNUMB::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)windowID)
			.append((char *)&value, 4)
			.append((char)digitCount);
}

int OpcodeWNUMB::getWindowID() const
{
	return windowID;
}

void OpcodeWNUMB::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

void OpcodeWNUMB::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), value & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), (value >> 16) & 0xFF));
}

OpcodeSTTIM::OpcodeSTTIM(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSTTIM::setParams(const QByteArray &params)
{
	banks[0] = params.at(0);
	banks[1] = params.at(1);
	h = params.at(2);// bank 1??
	m = params.at(3);// bank 2??
	s = params.at(4);// bank 3??
}

QString OpcodeSTTIM::toString() const
{
	return QObject::tr("Affecter une valeur au compte à rebours (H=%1, M=%2, S=%3)")
			.arg(_var(h, B1(banks[0])))
			.arg(_var(m, B2(banks[0])))
			.arg(_var(s, B1(banks[1])));
}

QByteArray OpcodeSTTIM::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)h)
			.append((char)m)
			.append((char)s);
}

void OpcodeSTTIM::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), h));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), m));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), s));
}

OpcodeGOLD::OpcodeGOLD(const QByteArray &params)
{
	setParams(params);
}

void OpcodeGOLD::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&value, &(params.constData()[1]), 4);// bank 1 and 2
}

QByteArray OpcodeGOLD::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&value, 4);
}

void OpcodeGOLD::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), value & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), (value >> 16) & 0xFF));
}

OpcodeGOLDu::OpcodeGOLDu(const QByteArray &params) :
	OpcodeGOLD(params)
{
}

QString OpcodeGOLDu::toString() const
{
	return QObject::tr("Ajouter %1 gils à l'équipe")
			.arg(_var(value, B1(banks), B2(banks)));
}

OpcodeGOLDd::OpcodeGOLDd(const QByteArray &params) :
	OpcodeGOLD(params)
{
}

QString OpcodeGOLDd::toString() const
{
	return QObject::tr("Retirer %1 gils à l'équipe")
			.arg(_var(value, B1(banks), B2(banks)));
}

OpcodeCHGLD::OpcodeCHGLD(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCHGLD::setParams(const QByteArray &params)
{
	banks = params.at(0);
	var1 = params.at(1); // bank 1
	var2 = params.at(2); // bank 2
}

QString OpcodeCHGLD::toString() const
{
	return QObject::tr("Copier le nombre de Gils dans %1 et %2")
			.arg(_bank(var1, B1(banks)))
			.arg(_bank(var2, B2(banks)));
}

QByteArray OpcodeCHGLD::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)var1)
			.append((char)var2);
}

void OpcodeCHGLD::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), var1));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), var2));
}

OpcodeHMPMAX1::OpcodeHMPMAX1()
{
}

QString OpcodeHMPMAX1::toString() const
{
	return QObject::tr("Redonne les HP/MP aux membres de l'équipe");
}

OpcodeHMPMAX2::OpcodeHMPMAX2()
{
}

QString OpcodeHMPMAX2::toString() const
{
	return QObject::tr("Redonne les HP/MP aux membres de l'équipe");
}

OpcodeMHMMX::OpcodeMHMMX()
{
}

QString OpcodeMHMMX::toString() const
{
	return QObject::tr("Redonne les HP/MP à tous et soigne les troubles de statut");
}

OpcodeHMPMAX3::OpcodeHMPMAX3()
{
}

QString OpcodeHMPMAX3::toString() const
{
	return QObject::tr("Redonne les HP/MP aux membres de l'équipe");
}

OpcodeMESSAGE::OpcodeMESSAGE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMESSAGE::setParams(const QByteArray &params)
{
	windowID = params.at(0);
	textID = params.at(1);
}

QString OpcodeMESSAGE::toString() const
{
	return QObject::tr("Afficher message %2 dans la fenêtre n°%1")
			.arg(windowID)
			.arg(_text(textID));
}

QByteArray OpcodeMESSAGE::params() const
{
	return QByteArray()
			.append((char)windowID)
			.append((char)textID);
}

int OpcodeMESSAGE::getWindowID() const
{
	return windowID;
}

void OpcodeMESSAGE::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

int OpcodeMESSAGE::getTextID() const
{
	return textID;
}

void OpcodeMESSAGE::setTextID(quint8 textID)
{
	this->textID = textID;
}

OpcodeMPARA::OpcodeMPARA(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMPARA::setParams(const QByteArray &params)
{
	banks = params.at(0);
	windowID = params.at(1);
	windowVarID = params.at(2);
	value = params.at(3); // bank 2
}

QString OpcodeMPARA::toString() const
{
	return QObject::tr("Affecter %3 à la variable n°%2 dans la fenêtre n°%1")
			.arg(windowID)
			.arg(windowVarID)
			.arg(_var(value, B2(banks)));
}

QByteArray OpcodeMPARA::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)windowID)
			.append((char)windowVarID)
			.append((char)value);
}

int OpcodeMPARA::getWindowID() const
{
	return windowID;
}

void OpcodeMPARA::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

void OpcodeMPARA::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), value));
}

OpcodeMPRA2::OpcodeMPRA2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMPRA2::setParams(const QByteArray &params)
{
	banks = params.at(0);
	windowID = params.at(1);
	windowVarID = params.at(2);
	memcpy(&value, &(params.constData()[3]), 2); // bank 2
}

QString OpcodeMPRA2::toString() const
{
	return QObject::tr("Affecter %3 à la variable n°%2 dans la fenêtre n°%1")
			.arg(windowID)
			.arg(windowVarID)
			.arg(_var(value, B2(banks)));
}

QByteArray OpcodeMPRA2::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)windowID)
			.append((char)windowVarID)
			.append((char *)&value, 2);
}

int OpcodeMPRA2::getWindowID() const
{
	return windowID;
}

void OpcodeMPRA2::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

void OpcodeMPRA2::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), value & 0xFF));
}

OpcodeMPNAM::OpcodeMPNAM(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMPNAM::setParams(const QByteArray &params)
{
	textID = params.at(0);
}

QString OpcodeMPNAM::toString() const
{
	return QObject::tr("Afficher %1 dans le menu")
			.arg(_text(textID));
}

QByteArray OpcodeMPNAM::params() const
{
	return QByteArray()
			.append((char)textID);
}

int OpcodeMPNAM::getTextID() const
{
	return textID;
}

void OpcodeMPNAM::setTextID(quint8 textID)
{
	this->textID = textID;
}

OpcodeMP::OpcodeMP(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMP::setParams(const QByteArray &params)
{
	banks = params.at(0);
	partyID = params.at(1);
	memcpy(&value, &(params.constData()[2]), 2); // bank 2??
}

QByteArray OpcodeMP::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)partyID)
			.append((char *)&value, 2);
}

void OpcodeMP::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), value & 0xFF));
}

OpcodeMPu::OpcodeMPu(const QByteArray &params) :
	OpcodeMP(params)
{
}

QString OpcodeMPu::toString() const
{
	return QObject::tr("Augmenter de %2 MPs le membre n°%1 de l'équipe")
			.arg(partyID)
			.arg(_var(value, B2(banks)));
}

OpcodeMPd::OpcodeMPd(const QByteArray &params) :
	OpcodeMP(params)
{
}

QString OpcodeMPd::toString() const
{
	return QObject::tr("Diminuer de %2 MPs le membre n°%1 de l'équipe")
			.arg(partyID)
			.arg(_var(value, B2(banks)));
}

OpcodeASK::OpcodeASK(const QByteArray &params)
{
	setParams(params);
}

void OpcodeASK::setParams(const QByteArray &params)
{
	banks = params.at(0);
	windowID = params.at(1);
	textID = params.at(2);
	firstLine = params.at(3);
	lastLine = params.at(4);
	varAnswer = params.at(5); // bank 2
}

QString OpcodeASK::toString() const
{
	return QObject::tr("Poser question %2 dans la fenêtre n°%1 (et mettre la réponse sélectionnée dans %5) première ligne=%3, dernière ligne=%4")
			.arg(windowID)
			.arg(_text(textID))
			.arg(firstLine)
			.arg(lastLine)
			.arg(_bank(varAnswer, B2(banks)));
}

QByteArray OpcodeASK::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)windowID)
			.append((char)textID)
			.append((char)firstLine)
			.append((char)lastLine)
			.append((char)varAnswer);
}

int OpcodeASK::getWindowID() const
{
	return windowID;
}

void OpcodeASK::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

int OpcodeASK::getTextID() const
{
	return textID;
}

void OpcodeASK::setTextID(quint8 textID)
{
	this->textID = textID;
}

void OpcodeASK::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), varAnswer));
}

OpcodeMENU::OpcodeMENU(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMENU::setParams(const QByteArray &params)
{
	banks = params.at(0);
	menuID = params.at(1);
	param = params.at(2); // bank 2
}

QString OpcodeMENU::menu(const QString &param) const
{
	switch(menuID)
	{
	case 1:		return QObject::tr("Fermer le programme (paramètre %1)").arg(param);
	case 2:		return QObject::tr("Encount Error (paramètre %1)").arg(param);
	case 5:		return QObject::tr("Crédits de ff7 (paramètre %1)").arg(param);
	case 6:		return QObject::tr("Changer nom de %1").arg(_personnage(this->param));//Paramètre : personnage id
	case 7:		return QObject::tr("Changer l'équipe (paramètre %1)").arg(param);
	case 8:		return QObject::tr("magasin n°%1").arg(param);//Paramètre : magasin id
	case 9:		return QObject::tr("principal (paramètre %1)").arg(param);
	case 12:	return QObject::tr("course de moto (paramètre %1)").arg(param);
	case 14:	return QObject::tr("Sauvegarde (paramètre %1)").arg(param);
	case 15:	return QObject::tr("Effacer toutes les matérias (paramètre %1)").arg(param);
	case 16:	return QObject::tr("Rétablir toutes les matérias (paramètre %1)").arg(param);
	case 17:	return QObject::tr("Effacer la Matéria de %1").arg(_personnage(this->param));
	case 18:	return QObject::tr("Effacer les matérias de Clad (paramètre %1)").arg(param);
	case 19:	return QObject::tr("Rétablir les matérias de Clad (paramètre %1)").arg(param);
	case 20:	return QObject::tr("? (paramètre %1)").arg(param);
	case 21:	return QObject::tr("HPs à 1 (paramètre %1)").arg(param);
	case 22:	return QObject::tr("? (paramètre %1)").arg(param);
	case 23:	return QObject::tr("maitre ? (paramètre %1)").arg(param);
	case 24:	return QObject::tr("? (paramètre %1)").arg(param);
	case 25:	return QObject::tr("? (paramètre %1)").arg(param);
	default:	return QObject::tr("%1? (paramètre %2)").arg(menuID).arg(param);
	}
}

QString OpcodeMENU::toString() const
{
	return QObject::tr("Afficher menu %1")
			.arg(menu(_var(param, B2(banks))));
}

QByteArray OpcodeMENU::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)menuID)
			.append((char)param);
}

void OpcodeMENU::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), param));
}

OpcodeMENU2::OpcodeMENU2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMENU2::setParams(const QByteArray &params)
{
	disabled = params.at(0);
}

QString OpcodeMENU2::toString() const
{
	return QObject::tr("%1 l'accès aux menus")
			.arg(disabled == 0 ? QObject::tr("Permettre") : QObject::tr("Interdire"));
}

QByteArray OpcodeMENU2::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodeBTLTB::OpcodeBTLTB(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBTLTB::setParams(const QByteArray &params)
{
	battleTableID = params.at(0);
}

QString OpcodeBTLTB::toString() const
{
	return QObject::tr("Choisir la battle table : %1")
			.arg(battleTableID);
}

QByteArray OpcodeBTLTB::params() const
{
	return QByteArray().append((char)battleTableID);
}

OpcodeHP::OpcodeHP(const QByteArray &params)
{
	setParams(params);
}

void OpcodeHP::setParams(const QByteArray &params)
{
	banks = params.at(0);
	partyID = params.at(1);
	memcpy(&value, &(params.constData()[2]), 2); // bank 2??
}

QByteArray OpcodeHP::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)partyID)
			.append((char *)&value, 2);
}

void OpcodeHP::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), value));
}

OpcodeHPu::OpcodeHPu(const QByteArray &params) :
	OpcodeHP(params)
{
}

QString OpcodeHPu::toString() const
{
	return QObject::tr("Augmenter de %2 HPs le membre n°%1 de l'équipe")
			.arg(partyID)
			.arg(_var(value, B2(banks)));
}

OpcodeHPd::OpcodeHPd(const QByteArray &params) :
	OpcodeHP(params)
{
}

QString OpcodeHPd::toString() const
{
	return QObject::tr("Diminuer de %2 HPs le membre n°%1 de l'équipe")
			.arg(partyID)
			.arg(_var(value, B2(banks)));
}

OpcodeWINDOW::OpcodeWINDOW(const QByteArray &params)
{
	setParams(params);
}

void OpcodeWINDOW::setParams(const QByteArray &params)
{
	windowID = params.at(0);
	memcpy(&targetX, &(params.constData()[1]), 2);
	memcpy(&targetY, &(params.constData()[3]), 2);
	memcpy(&width, &(params.constData()[5]), 2);
	memcpy(&height, &(params.constData()[7]), 2);
}

QString OpcodeWINDOW::toString() const
{
	return QObject::tr("Créer la fenêtre n°%1 (X=%2, Y=%3, largeur=%4, hauteur=%5)")
			.arg(windowID)
			.arg(targetX)
			.arg(targetY)
			.arg(width)
			.arg(height);
}

QByteArray OpcodeWINDOW::params() const
{
	return QByteArray()
			.append((char)windowID)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&width, 2)
			.append((char *)&height, 2);
}

int OpcodeWINDOW::getWindowID() const
{
	return windowID;
}

void OpcodeWINDOW::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

bool OpcodeWINDOW::getWindow(FF7Window &window) const
{
	window.x = targetX;
	window.y = targetY;
	window.w = width;
	window.h = height;
	return true;
}

void OpcodeWINDOW::setWindow(const FF7Window &window)
{
	targetX = window.x;
	targetY = window.y;
	width = window.w;
	height = window.h;
}

OpcodeWMOVE::OpcodeWMOVE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeWMOVE::setParams(const QByteArray &params)
{
	windowID = params.at(0);
	memcpy(&targetX, &(params.constData()[1]), 2);
	memcpy(&targetY, &(params.constData()[3]), 2);
}

QString OpcodeWMOVE::toString() const
{
	return QObject::tr("Déplacer la fenêtre n°%1 (X=%2, Y=%3)")
			.arg(windowID)
			.arg(targetX)
			.arg(targetY);
}

QByteArray OpcodeWMOVE::params() const
{
	return QByteArray()
			.append((char)windowID)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2);
}

int OpcodeWMOVE::getWindowID() const
{
	return windowID;
}

void OpcodeWMOVE::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

bool OpcodeWMOVE::getWindow(FF7Window &window) const
{
	window.x = targetX;
	window.y = targetY;
	return true;
}

void OpcodeWMOVE::setWindow(const FF7Window &window)
{
	targetX = window.x;
	targetY = window.y;
}

OpcodeWMODE::OpcodeWMODE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeWMODE::setParams(const QByteArray &params)
{
	windowID = params.at(0);
	mode = params.at(1);
	preventClose = params.at(2); // boolean
}

QString OpcodeWMODE::toString() const
{
	QString typeStr;
	switch(mode)
	{
	case 0x00:		typeStr = QObject::tr("Normale");						break;
	case 0x01:		typeStr = QObject::tr("Pas de fond, ni de bordure");	break;
	case 0x02:		typeStr = QObject::tr("Fond semi-transparent");			break;
	default:		typeStr = QString("%1?").arg(mode);
	}

	return QObject::tr("Décoration de la fenêtre n°%1 : %2 (%3 la fermeture de la fenêtre par le joueur)")
			.arg(windowID)
			.arg(typeStr)
			.arg(preventClose == 0 ? QObject::tr("autoriser") : QObject::tr("empêcher"));
}

QByteArray OpcodeWMODE::params() const
{
	return QByteArray()
			.append((char)windowID)
			.append((char)mode)
			.append((char)preventClose);
}

int OpcodeWMODE::getWindowID() const
{
	return windowID;
}

void OpcodeWMODE::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

OpcodeWREST::OpcodeWREST(const QByteArray &params)
{
	setParams(params);
}

void OpcodeWREST::setParams(const QByteArray &params)
{
	windowID = params.at(0);
}

QString OpcodeWREST::toString() const
{
	return QObject::tr("Remettre la fenêtre n°%1 à zéro")
			.arg(windowID);
}

QByteArray OpcodeWREST::params() const
{
	return QByteArray().append((char)windowID);
}

int OpcodeWREST::getWindowID() const
{
	return windowID;
}

void OpcodeWREST::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

OpcodeWCLSE::OpcodeWCLSE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeWCLSE::setParams(const QByteArray &params)
{
	windowID = params.at(0);
}

QString OpcodeWCLSE::toString() const
{
	return QObject::tr("Fermer la fenêtre n°%1")
			.arg(windowID);
}

QByteArray OpcodeWCLSE::params() const
{
	return QByteArray().append((char)windowID);
}

int OpcodeWCLSE::getWindowID() const
{
	return windowID;
}

void OpcodeWCLSE::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

OpcodeWROW::OpcodeWROW(const QByteArray &params)
{
	setParams(params);
}

void OpcodeWROW::setParams(const QByteArray &params)
{
	windowID = params.at(0);
	rowCount = params.at(1);
}

QString OpcodeWROW::toString() const
{
	return QObject::tr("Configurer le nombre de lignes de texte à %2 dans la fenêtre n°%1")
			.arg(windowID)
			.arg(rowCount);
}

QByteArray OpcodeWROW::params() const
{
	return QByteArray()
			.append((char)windowID)
			.append((char)rowCount);
}

int OpcodeWROW::getWindowID() const
{
	return windowID;
}

void OpcodeWROW::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

OpcodeGWCOL::OpcodeGWCOL(const QByteArray &params)
{
	setParams(params);
}

void OpcodeGWCOL::setParams(const QByteArray &params)
{
	banks[0] = params.at(0);
	banks[1] = params.at(1);
	corner = params.at(2); // bank 1
	varR = params.at(3); // bank 2
	varG = params.at(4); // bank 3
	varB = params.at(5); // bank 4
}

QString OpcodeGWCOL::toString() const
{
	return QObject::tr("Obtenir la couleur du côté %1 des fenêtres et en stocker les composantes dans %2 (R), %3 (V) et %4 (B)")
			.arg(_windowCorner(corner, B1(banks[0])))
			.arg(_bank(varR, B2(banks[0])))
			.arg(_bank(varG, B1(banks[1])))
			.arg(_bank(varB, B2(banks[1])));
}

QByteArray OpcodeGWCOL::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)corner)
			.append((char)varR)
			.append((char)varG)
			.append((char)varB);
}

void OpcodeGWCOL::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), corner));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), varR));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), varG));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), varB));
}

OpcodeSWCOL::OpcodeSWCOL(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSWCOL::setParams(const QByteArray &params)
{
	banks[0] = params.at(0);
	banks[1] = params.at(1);
	corner = params.at(2); // bank 1
	r = params.at(3); // bank 2
	g = params.at(4); // bank 3
	b = params.at(5); // bank 4
}

QString OpcodeSWCOL::toString() const
{
	return QObject::tr("Changer la couleur du côté %1 des fenêtres : RVB(%2, %3, %4)")
			.arg(_windowCorner(corner, B1(banks[0])))
			.arg(_var(r, B2(banks[0])))
			.arg(_var(g, B1(banks[1])))
			.arg(_var(b, B2(banks[1])));
}

QByteArray OpcodeSWCOL::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)corner)
			.append((char)r)
			.append((char)g)
			.append((char)b);
}

void OpcodeSWCOL::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), corner));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), r));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), g));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), b));
}

OpcodeSTITM::OpcodeSTITM(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSTITM::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&itemID, &(params.constData()[1]), 2); // bank 1
	quantity = params.at(3); // bank 2
}

QString OpcodeSTITM::toString() const
{
	return QObject::tr("Ajouter %2 objet(s) %1 dans l'inventaire")
			.arg(_item(itemID, B1(banks)))
			.arg(_var(quantity, B2(banks)));
}

QByteArray OpcodeSTITM::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&itemID, 2)
			.append((char)quantity);
}

void OpcodeSTITM::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), itemID & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), quantity));
}

OpcodeDLITM::OpcodeDLITM(const QByteArray &params)
{
	setParams(params);
}

void OpcodeDLITM::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&itemID, &(params.constData()[1]), 2); // bank 1
	quantity = params.at(3); // bank 2
}

QString OpcodeDLITM::toString() const
{
	return QObject::tr("Supprimer %2 objet(s) %1 dans l'inventaire")
			.arg(_item(itemID, B1(banks)))
			.arg(_var(quantity, B2(banks)));
}

QByteArray OpcodeDLITM::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&itemID, 2)
			.append((char)quantity);
}

void OpcodeDLITM::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), itemID & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), quantity));
}

OpcodeCKITM::OpcodeCKITM(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCKITM::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&itemID, &(params.constData()[1]), 2); // bank 1
	varQuantity = params.at(3); // bank 2
}

QString OpcodeCKITM::toString() const
{
	return QObject::tr("%2 = quantité d'objets %1 dans l'inventaire")
			.arg(_item(itemID, B1(banks)))
			.arg(_bank(varQuantity, B2(banks)));
}

QByteArray OpcodeCKITM::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&itemID, 2)
			.append((char)varQuantity);
}

void OpcodeCKITM::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), itemID & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), varQuantity));
}

OpcodeSMTRA::OpcodeSMTRA(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSMTRA::setParams(const QByteArray &params)
{
	banks[0] = params.at(0);
	banks[1] = params.at(1);
	materiaID = params.at(2); // bank 1
	APCount = 0;
	memcpy(&APCount, &(params.constData()[3]), 3); // bank 2, bank 3, bank 4
}

QString OpcodeSMTRA::toString() const
{
	return QObject::tr("Ajouter la matéria %1 dans l'inventaire (AP=%2)")
			.arg(_materia(materiaID, B1(banks[0])))
			.arg(_var(APCount, B2(banks[0]), B1(banks[1]), B2(banks[1])));
}

QByteArray OpcodeSMTRA::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)materiaID)
			.append((char *)&APCount, 3);
}

void OpcodeSMTRA::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), materiaID));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), APCount & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), (APCount >> 8) & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), (APCount >> 16) & 0xFF));
}

OpcodeDMTRA::OpcodeDMTRA(const QByteArray &params)
{
	setParams(params);
}

void OpcodeDMTRA::setParams(const QByteArray &params)
{
	banks[0] = params.at(0);
	banks[1] = params.at(1);
	materiaID = params.at(2); // bank 1
	APCount = 0;
	memcpy(&APCount, &(params.constData()[3]), 3); // bank 2, bank 3, bank 4
	quantity = params.at(6);
}

QString OpcodeDMTRA::toString() const
{
	return QObject::tr("Supprimer %3 matéria(s) %1 dans l'inventaire (AP=%2)")
			.arg(_materia(materiaID, B1(banks[0])))
			.arg(_var(APCount, B2(banks[0]), B1(banks[1]), B2(banks[1])))
			.arg(quantity);
}

QByteArray OpcodeDMTRA::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)materiaID)
			.append((char *)&APCount, 3)
			.append((char)quantity);
}

void OpcodeDMTRA::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), materiaID));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), APCount & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), (APCount >> 8) & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), (APCount >> 16) & 0xFF));
}

OpcodeCMTRA::OpcodeCMTRA(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCMTRA::setParams(const QByteArray &params)
{
	banks[0] = params.at(0);
	banks[1] = params.at(1);
	banks[2] = params.at(2);
	materiaID = params.at(3); // bank 1
	APCount = 0;
	memcpy(&APCount, &(params.constData()[4]), 3); // bank 2, bank 3, bank 4
	unknown = params.at(7);
	varQuantity = params.at(8); // bank 5
}

QString OpcodeCMTRA::toString() const
{
	return QObject::tr("%4 = quantité de matéria %1 dans l'inventaire (AP=%2, ?=%3)")
			.arg(_materia(materiaID, B1(banks[0])))
			.arg(_var(APCount, B2(banks[0]), B1(banks[1]), B2(banks[1])))
			.arg(unknown)
			.arg(_bank(varQuantity, B2(banks[2])));
}

QByteArray OpcodeCMTRA::params() const
{
	return QByteArray()
			.append((char *)&banks, 3)
			.append((char)materiaID)
			.append((char *)&APCount, 3)
			.append((char)unknown)
			.append((char)varQuantity);
}

void OpcodeCMTRA::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), materiaID));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), APCount & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), (APCount >> 8) & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), (APCount >> 16) & 0xFF));
	if(B2(banks[2]) != 0)
		vars.append(FF7Var(B2(banks[2]), varQuantity));
}

OpcodeSHAKE::OpcodeSHAKE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSHAKE::setParams(const QByteArray &params)
{
	unknown1 = params.at(0);
	unknown2 = params.at(1);
	shakeCount = params.at(2);
	unknown3 = params.at(3);
	unknown4 = params.at(4);
	amplitude = params.at(5);
	speed = params.at(6);
}

QString OpcodeSHAKE::toString() const
{
	return QObject::tr("Secouer l'écran (nbOscillations=%1, Amplitude=%2, vitesse=%3)")
			.arg(shakeCount)
			.arg(amplitude)
			.arg(speed);
}

QByteArray OpcodeSHAKE::params() const
{
	return QByteArray()
			.append((char)unknown1)
			.append((char)unknown2)
			.append((char)shakeCount)
			.append((char)unknown3)
			.append((char)unknown4)
			.append((char)amplitude)
			.append((char)speed);
}

OpcodeNOP::OpcodeNOP()
{
}

QString OpcodeNOP::toString() const
{
	return QObject::tr("Ne rien faire...");
}

OpcodeMAPJUMP::OpcodeMAPJUMP(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMAPJUMP::setParams(const QByteArray &params)
{
	memcpy(&fieldID, params.constData(), 2);
	memcpy(&targetX, &(params.constData()[2]), 2);
	memcpy(&targetY, &(params.constData()[4]), 2);
	memcpy(&targetI, &(params.constData()[6]), 2);
	direction = params.at(8);
}

QString OpcodeMAPJUMP::toString() const
{
	return QObject::tr("Aller à l'écran %1 (X=%2, Y=%3, polygone id=%4, direction=%5)")
			.arg(_field(fieldID))
			.arg(targetX)
			.arg(targetY)
			.arg(targetI)
			.arg(direction);
}

QByteArray OpcodeMAPJUMP::params() const
{
	return QByteArray()
			.append((char *)&fieldID, 2)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&targetI, 2)
			.append((char)direction);
}

OpcodeSCRLO::OpcodeSCRLO(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSCRLO::setParams(const QByteArray &params)
{
	unknown = params.at(0);
}

QString OpcodeSCRLO::toString() const
{
	return QObject::tr("SCRLO (?=%1)")
			.arg(unknown);
}

QByteArray OpcodeSCRLO::params() const
{
	return QByteArray().append((char)unknown);
}

OpcodeSCRLC::OpcodeSCRLC(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSCRLC::setParams(const QByteArray &params)
{
	memcpy(&unknown, params.constData(), 4);
}

QString OpcodeSCRLC::toString() const
{
	return QObject::tr("SCRLC (?=%1)")
			.arg(unknown);
}

QByteArray OpcodeSCRLC::params() const
{
	return QByteArray().append((char *)&unknown, 4);
}

OpcodeSCRLA::OpcodeSCRLA(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSCRLA::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&speed, &(params.constData()[1]), 2); // bank 2
	groupID = params.at(3);
	scrollType = params.at(4);
}

QString OpcodeSCRLA::toString() const
{
	return QObject::tr("Centrer sur le groupe %2 (vitesse=%1, type=%3)")
			.arg(_var(speed, B2(banks)))
			.arg(_script(groupID))
			.arg(scrollType);
}

QByteArray OpcodeSCRLA::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&speed, 2)
			.append((char)groupID)
			.append((char)scrollType);
}

void OpcodeSCRLA::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), speed & 0xFF));
}

OpcodeSCR2D::OpcodeSCR2D(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSCR2D::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&targetX, &(params.constData()[1]), 2); // bank 1
	memcpy(&targetY, &(params.constData()[3]), 2); // bank 2
}

QString OpcodeSCR2D::toString() const
{
	return QObject::tr("Centrer sur zone (X=%1, Y=%2)")
			.arg(_var(targetX, B1(banks)))
			.arg(_var(targetY, B2(banks)));
}

QByteArray OpcodeSCR2D::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2);
}

void OpcodeSCR2D::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), targetX & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), targetY & 0xFF));
}

OpcodeSCRCC::OpcodeSCRCC()
{
}

QString OpcodeSCRCC::toString() const
{
	return QObject::tr("Centrer sur le personnage jouable");
}

OpcodeSCR2DC::OpcodeSCR2DC(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSCR2DC::setParams(const QByteArray &params)
{
	banks[0] = params.at(0);
	banks[1] = params.at(1);
	memcpy(&targetX, &(params.constData()[2]), 2); // bank 1
	memcpy(&targetY, &(params.constData()[4]), 2); // bank 2
	memcpy(&speed, &(params.constData()[6]), 2); // bank 4
}

QString OpcodeSCR2DC::toString() const
{
	return QObject::tr("Centrer sur zone (X=%1, Y=%2, vitesse=%3)")
			.arg(_var(targetX, B1(banks[0])))
			.arg(_var(targetY, B2(banks[0])))
			.arg(_var(speed, B2(banks[1])));
}

QByteArray OpcodeSCR2DC::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&speed, 2);
}

void OpcodeSCR2DC::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), speed & 0xFF));
}

OpcodeSCRLW::OpcodeSCRLW()
{
}

QString OpcodeSCRLW::toString() const
{
	return QObject::tr("Attendre la fin du dernier centrage pour continuer");
}

OpcodeSCR2DL::OpcodeSCR2DL(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSCR2DL::setParams(const QByteArray &params)
{
	banks[0] = params.at(0);
	banks[1] = params.at(1);
	memcpy(&targetX, &(params.constData()[2]), 2); // bank 1
	memcpy(&targetY, &(params.constData()[4]), 2); // bank 2
	memcpy(&speed, &(params.constData()[6]), 2); // bank 4
}

QString OpcodeSCR2DL::toString() const
{
	return QObject::tr("Centrer sur zone (X=%1, Y=%2, vitesse=%3)")
			.arg(_var(targetX, B1(banks[0])))
			.arg(_var(targetY, B2(banks[0])))
			.arg(_var(speed, B2(banks[1])));
}

QByteArray OpcodeSCR2DL::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&speed, 2);
}

void OpcodeSCR2DL::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), speed & 0xFF));
}

OpcodeMPDSP::OpcodeMPDSP(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMPDSP::setParams(const QByteArray &params)
{
	unknown = params.at(0);
}

QString OpcodeMPDSP::toString() const
{
	return QObject::tr("MPDSP : %1")
			.arg(unknown);
}

QByteArray OpcodeMPDSP::params() const
{
	return QByteArray().append((char)unknown);
}

OpcodeVWOFT::OpcodeVWOFT(const QByteArray &params)
{
	setParams(params);
}

void OpcodeVWOFT::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&unknown1, &(params.constData()[1]), 2); // bank 1
	memcpy(&unknown2, &(params.constData()[3]), 2); // bank 2
	unknown3 = params.at(5);
}

QString OpcodeVWOFT::toString() const
{
	return QObject::tr("Centrer sur Zone (?=%1, ?=%2, ?=%3)")
			.arg(_var(unknown1, B1(banks)))
			.arg(_var(unknown2, B2(banks)))
			.arg(unknown3);
}

QByteArray OpcodeVWOFT::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&unknown1, 2)
			.append((char *)&unknown2, 2)
			.append((char)unknown3);
}

void OpcodeVWOFT::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), unknown1));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), unknown2));
}

OpcodeFADE::OpcodeFADE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeFADE::setParams(const QByteArray &params)
{
	banks[0] = params.at(0);
	banks[1] = params.at(1);
	r = params.at(2); // bank 1
	g = params.at(3); // bank 2
	b = params.at(4); // bank 4
	speed = params.at(5);
	fadeType = params.at(6);
	adjust = params.at(7);
}

QString OpcodeFADE::toString() const
{
	return QObject::tr("Voiler l'écran avec la couleur RVB(%1, %2, %3) (vitesse=%4, type=%5, adjust=%6)")
			.arg(_var(r, B1(banks[0])))
			.arg(_var(g, B2(banks[0])))
			.arg(_var(b, B2(banks[1])))
			.arg(speed)
			.arg(fadeType)
			.arg(adjust);
}

QByteArray OpcodeFADE::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)r)
			.append((char)g)
			.append((char)b)
			.append((char)speed)
			.append((char)fadeType)
			.append((char)adjust);
}

void OpcodeFADE::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), r));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), g));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), b));
}

OpcodeFADEW::OpcodeFADEW()
{
}

QString OpcodeFADEW::toString() const
{
	return QObject::tr("Attendre la fin du voilage de l'écran pour continuer");
}

OpcodeIDLCK::OpcodeIDLCK(const QByteArray &params)
{
	setParams(params);
}

void OpcodeIDLCK::setParams(const QByteArray &params)
{
	memcpy(&triangleID, params.constData(), 2);
	locked = params.at(2); // boolean
}

QString OpcodeIDLCK::toString() const
{
	return QObject::tr("%1 le polygone n°%2")
			.arg(triangleID)
			.arg(locked == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeIDLCK::params() const
{
	return QByteArray()
			.append((char *)&triangleID, 2)
			.append((char)locked);
}

OpcodeLSTMP::OpcodeLSTMP(const QByteArray &params)
{
	setParams(params);
}

void OpcodeLSTMP::setParams(const QByteArray &params)
{
	banks = params.at(0);
	var = params.at(1);
}

QString OpcodeLSTMP::toString() const
{
	return QObject::tr("Stocker l'id de l'écran précédent dans %1")
			.arg(_bank(var, B2(banks)));
}

QByteArray OpcodeLSTMP::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)var);
}

void OpcodeLSTMP::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), var));
}

OpcodeSCRLP::OpcodeSCRLP(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSCRLP::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&speed, &(params.constData()[1]), 2); // bank 2
	partyID = params.at(3);
	scrollType = params.at(4);
}

QString OpcodeSCRLP::toString() const
{
	return QObject::tr("Centrer sur le personnage n°%2 de l'équipe actuelle (vitesse=%1 img, type=%3)")
			.arg(_var(speed, B2(banks)))
			.arg(partyID)
			.arg(scrollType);
}

QByteArray OpcodeSCRLP::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&speed, 2)
			.append((char)partyID)
			.append((char)scrollType);
}

void OpcodeSCRLP::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), speed & 0xFF));
}

OpcodeBATTLE::OpcodeBATTLE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBATTLE::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&battleID, &(params.constData()[1]), 2); // bank 2
}

QString OpcodeBATTLE::toString() const
{
	return QObject::tr("Commencer le combat n°%1")
			.arg(_var(battleID, B2(banks)));
}

QByteArray OpcodeBATTLE::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&battleID, 2);
}

void OpcodeBATTLE::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), battleID & 0xFF));
}

OpcodeBTLON::OpcodeBTLON(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBTLON::setParams(const QByteArray &params)
{
	disabled = params.at(0);
}

QString OpcodeBTLON::toString() const
{
	return QObject::tr("%1 les combats aléatoires")
			.arg(disabled == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeBTLON::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodeBTLMD::OpcodeBTLMD(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBTLMD::setParams(const QByteArray &params)
{
	memcpy(&unknown, params.constData(), 2);
}

QString OpcodeBTLMD::toString() const
{
	return QObject::tr("BTLMD (?=%1)")
			.arg(unknown);
}

QByteArray OpcodeBTLMD::params() const
{
	return QByteArray().append((char *)&unknown, 2);
}

OpcodePGTDR::OpcodePGTDR(const QByteArray &params)
{
	setParams(params);
}

void OpcodePGTDR::setParams(const QByteArray &params)
{
	banks = params.at(0);
	partyID = params.at(1);
	varDir = params.at(2); // bank 2
}

QString OpcodePGTDR::toString() const
{
	return QObject::tr("Obtenir la direction du personnage n°%1 de l'équipe actuelle et la stocker dans %2")
			.arg(partyID)
			.arg(_bank(varDir, B2(banks)));
}

QByteArray OpcodePGTDR::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)partyID)
			.append((char)varDir);
}

void OpcodePGTDR::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), varDir));
}

OpcodeGETPC::OpcodeGETPC(const QByteArray &params)
{
	setParams(params);
}

void OpcodeGETPC::setParams(const QByteArray &params)
{
	banks = params.at(0);
	partyID = params.at(1);
	varPC = params.at(2); // bank 2
}

QString OpcodeGETPC::toString() const
{
	return QObject::tr("Obtenir l'id du personnage n°%1 de l'équipe actuelle et le stocker dans %2")
			.arg(partyID)
			.arg(_bank(varPC, B2(banks)));
}

QByteArray OpcodeGETPC::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)partyID)
			.append((char)varPC);
}

void OpcodeGETPC::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), varPC));
}

OpcodePXYZI::OpcodePXYZI(const QByteArray &params)
{
	setParams(params);
}

void OpcodePXYZI::setParams(const QByteArray &params)
{
	banks[0] = params.at(0);
	banks[1] = params.at(1);
	partyID = params.at(2);
	varX = params.at(3); // bank 1
	varY = params.at(4); // bank 2
	varZ = params.at(5); // bank 3
	varI = params.at(6); // bank 4
}

QString OpcodePXYZI::toString() const
{
	return QObject::tr("Obtenir les coordonnées du personnage n°%1 de l'équipe actuelle (stocker : X dans %2, Y dans %3, Z dans %4 et l'id dans %5)")
			.arg(partyID)
			.arg(_bank(varX, B1(banks[0])))
			.arg(_bank(varY, B2(banks[0])))
			.arg(_bank(varZ, B1(banks[1])))
			.arg(_bank(varI, B2(banks[1])));
}

QByteArray OpcodePXYZI::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)partyID)
			.append((char)varX)
			.append((char)varY)
			.append((char)varZ)
			.append((char)varI);
}

void OpcodePXYZI::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), varX));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), varY));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), varZ));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), varI));
}

OpcodeBinaryOperation::OpcodeBinaryOperation()
{
}

OpcodeOperation::OpcodeOperation(const QByteArray &params)
{
	setParams(params);
}

void OpcodeOperation::setParams(const QByteArray &params)
{
	banks = (quint8)params.at(0);
	var = (quint8)params.at(1); // bank 1
	value = (quint8)params.at(2); // bank 2
}

QByteArray OpcodeOperation::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)var)
			.append((char)(value & 0xFF));
}

void OpcodeOperation::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), var));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), value & 0xFF));
}

OpcodeOperation2::OpcodeOperation2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeOperation2::setParams(const QByteArray &params)
{
	banks = (quint8)params.at(0);
	var = (quint8)params.at(1);
	value = 0;
	memcpy(&value, &(params.constData()[2]), 2);
}

QByteArray OpcodeOperation2::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)var)
			.append((char *)&value, 2);
}

void OpcodeOperation2::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), var));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), value & 0xFF));
}

OpcodeUnaryOperation::OpcodeUnaryOperation(const QByteArray &params)
{
	setParams(params);
}

void OpcodeUnaryOperation::setParams(const QByteArray &params)
{
	banks = params.at(0);
	var = params.at(1); // bank 2
}

QByteArray OpcodeUnaryOperation::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)var);
}

void OpcodeUnaryOperation::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), var));
}

OpcodePLUSX::OpcodePLUSX(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodePLUSX::toString() const
{
	return QObject::tr("%1 = %1 + %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodePLUS2X::OpcodePLUS2X(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodePLUS2X::toString() const
{
	return QObject::tr("%1 = %1 + %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMINUSX::OpcodeMINUSX(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodeMINUSX::toString() const
{
	return QObject::tr("%1 = %1 - %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMINUS2X::OpcodeMINUS2X(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodeMINUS2X::toString() const
{
	return QObject::tr("%1 = %1 - %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeINCX::OpcodeINCX(const QByteArray &params) :
	OpcodeUnaryOperation(params)
{
}

QString OpcodeINCX::toString() const
{
	return QObject::tr("%1 = %1 + 1 (8 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeINC2X::OpcodeINC2X(const QByteArray &params) :
	OpcodeUnaryOperation(params)
{
}

QString OpcodeINC2X::toString() const
{
	return QObject::tr("%1 = %1 + 1 (16 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeDECX::OpcodeDECX(const QByteArray &params) :
	OpcodeUnaryOperation(params)
{
}

QString OpcodeDECX::toString() const
{
	return QObject::tr("%1 = %1 - 1 (8 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeDEC2X::OpcodeDEC2X(const QByteArray &params) :
	OpcodeUnaryOperation(params)
{
}

QString OpcodeDEC2X::toString() const
{
	return QObject::tr("%1 = %1 - 1 (16 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeTLKON::OpcodeTLKON(const QByteArray &params)
{
	setParams(params);
}

void OpcodeTLKON::setParams(const QByteArray &params)
{
	disabled = params.at(0); // boolean
}

QString OpcodeTLKON::toString() const
{
	return QObject::tr("%1 la possibilité de parler à l'objet 3D")
			.arg(disabled == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeTLKON::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodeRDMSD::OpcodeRDMSD(const QByteArray &params)
{
	setParams(params);
}

void OpcodeRDMSD::setParams(const QByteArray &params)
{
	banks = params.at(0);
	value = params.at(1); // bank 2
}

QString OpcodeRDMSD::toString() const
{
	return QObject::tr("Seed Random Generator : %1")
			.arg(_var(value, B2(banks)));
}

QByteArray OpcodeRDMSD::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)value);
}

OpcodeSETBYTE::OpcodeSETBYTE(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodeSETBYTE::toString() const
{
	return QObject::tr("%1 = %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeSETWORD::OpcodeSETWORD(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodeSETWORD::toString() const
{
	return QObject::tr("%1 = %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeBitOperation::OpcodeBitOperation(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBitOperation::setParams(const QByteArray &params)
{
	banks = params.at(0);
	var = params.at(1); // bank 1
	position = params.at(2); // bank 2
}

QByteArray OpcodeBitOperation::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)var)
			.append((char)position);
}

void OpcodeBitOperation::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), var));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), position));
}

OpcodeBITON::OpcodeBITON(const QByteArray &params) :
	OpcodeBitOperation(params)
{
}

QString OpcodeBITON::toString() const
{
	return QObject::tr("Mettre le bit %2 à 1 dans %1")
			.arg(_bank(var, B1(banks)))
			.arg(_var(position, B2(banks)));
}

OpcodeBITOFF::OpcodeBITOFF(const QByteArray &params) :
	OpcodeBitOperation(params)
{
}

QString OpcodeBITOFF::toString() const
{
	return QObject::tr("Mettre le bit %2 à 0 dans %1")
			.arg(_bank(var, B1(banks)))
			.arg(_var(position, B2(banks)));
}

OpcodeBITXOR::OpcodeBITXOR(const QByteArray &params) :
	OpcodeBitOperation(params)
{
}

QString OpcodeBITXOR::toString() const
{
	return QObject::tr("Inverser la valeur du bit %2 dans %1")
			.arg(_bank(var, B1(banks)))
			.arg(_var(position, B2(banks)));
}

OpcodePLUS::OpcodePLUS(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodePLUS::toString() const
{
	return QObject::tr("%1 = %1 + %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodePLUS2::OpcodePLUS2(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodePLUS2::toString() const
{
	return QObject::tr("%1 = %1 + %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMINUS::OpcodeMINUS(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodeMINUS::toString() const
{
	return QObject::tr("%1 = %1 - %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMINUS2::OpcodeMINUS2(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodeMINUS2::toString() const
{
	return QObject::tr("%1 = %1 - %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMUL::OpcodeMUL(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodeMUL::toString() const
{
	return QObject::tr("%1 = %1 * %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMUL2::OpcodeMUL2(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodeMUL2::toString() const
{
	return QObject::tr("%1 = %1 * %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeDIV::OpcodeDIV(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodeDIV::toString() const
{
	return QObject::tr("%1 = %1 / %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeDIV2::OpcodeDIV2(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodeDIV2::toString() const
{
	return QObject::tr("%1 = %1 / %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMOD::OpcodeMOD(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodeMOD::toString() const
{
	return QObject::tr("%1 = %1 mod %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMOD2::OpcodeMOD2(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodeMOD2::toString() const
{
	return QObject::tr("%1 = %1 mod %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeAND::OpcodeAND(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodeAND::toString() const
{
	return QObject::tr("%1 = %1 & %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeAND2::OpcodeAND2(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodeAND2::toString() const
{
	return QObject::tr("%1 = %1 & %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeOR::OpcodeOR(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodeOR::toString() const
{
	return QObject::tr("%1 = %1 | %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeOR2::OpcodeOR2(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodeOR2::toString() const
{
	return QObject::tr("%1 = %1 | %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeXOR::OpcodeXOR(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodeXOR::toString() const
{
	return QObject::tr("%1 = %1 ^ %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeXOR2::OpcodeXOR2(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodeXOR2::toString() const
{
	return QObject::tr("%1 = %1 ^ %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeINC::OpcodeINC(const QByteArray &params) :
	OpcodeUnaryOperation(params)
{
}

QString OpcodeINC::toString() const
{
	return QObject::tr("%1 = %1 + 1 (8 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeINC2::OpcodeINC2(const QByteArray &params) :
	OpcodeUnaryOperation(params)
{
}

QString OpcodeINC2::toString() const
{
	return QObject::tr("%1 = %1 + 1 (16 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeDEC::OpcodeDEC(const QByteArray &params) :
	OpcodeUnaryOperation(params)
{
}

QString OpcodeDEC::toString() const
{
	return QObject::tr("%1 = %1 - 1 (8 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeDEC2::OpcodeDEC2(const QByteArray &params) :
	OpcodeUnaryOperation(params)
{
}

QString OpcodeDEC2::toString() const
{
	return QObject::tr("%1 = %1 - 1 (16 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeRANDOM::OpcodeRANDOM(const QByteArray &params) :
	OpcodeUnaryOperation(params)
{
}

QString OpcodeRANDOM::toString() const
{
	return QObject::tr("Affecter une valeur aléatoire à %1 (8 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeLBYTE::OpcodeLBYTE(const QByteArray &params) :
	OpcodeOperation(params)
{
}

QString OpcodeLBYTE::toString() const
{
	return QObject::tr("%1 = %2 & 0xFF (low byte)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeHBYTE::OpcodeHBYTE(const QByteArray &params) :
	OpcodeOperation2(params)
{
}

QString OpcodeHBYTE::toString() const
{
	return QObject::tr("%1 = (%2 >> 8) & 0xFF (high byte)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

Opcode2BYTE::Opcode2BYTE(const QByteArray &params)
{
	setParams(params);
}

void Opcode2BYTE::setParams(const QByteArray &params)
{
	banks[0] = params.at(0);
	banks[1] = params.at(1);
	var = params.at(2); // bank 1
	value1 = params.at(3); // bank 2
	value2 = params.at(4); // bank 4
}

QString Opcode2BYTE::toString() const
{
	return QObject::tr("%1 = (%2 & 0xFF) | ((%3 & 0xFF) << 8)")
			.arg(_bank(var, B1(banks[0])))
			.arg(_var(value1, B2(banks[0])))
			.arg(_var(value2, B2(banks[1])));
}

QByteArray Opcode2BYTE::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)var)
			.append((char)value1)
			.append((char)value2);
}

void Opcode2BYTE::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), var));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), value1));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), value2));
}

OpcodeSETX::OpcodeSETX(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSETX::setParams(const QByteArray &params)
{
	memcpy(&unknown, params.constData(), 6);
}

QString OpcodeSETX::toString() const
{
	return QObject::tr("SETX");
}

QByteArray OpcodeSETX::params() const
{
	return QByteArray().append((char *)&unknown, 6);
}

OpcodeGETX::OpcodeGETX(const QByteArray &params)
{
	setParams(params);
}

void OpcodeGETX::setParams(const QByteArray &params)
{
	memcpy(&unknown, params.constData(), 6);
}

QString OpcodeGETX::toString() const
{
	return QObject::tr("GETX");
}

QByteArray OpcodeGETX::params() const
{
	return QByteArray().append((char *)&unknown, 6);
}

OpcodeSEARCHX::OpcodeSEARCHX(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSEARCHX::setParams(const QByteArray &params)
{
	memcpy(&unknown, params.constData(), 10);
}

QString OpcodeSEARCHX::toString() const
{
	return QObject::tr("SEARCHX");
}

QByteArray OpcodeSEARCHX::params() const
{
	return QByteArray().append((char *)&unknown, 10);
}

OpcodePC::OpcodePC(const QByteArray &params)
{
	setParams(params);
}

void OpcodePC::setParams(const QByteArray &params)
{
	charID = params.at(0);
}

QString OpcodePC::toString() const
{
	return QObject::tr("L'objet 3D est jouable et c'est %1")
			.arg(_personnage(charID));
}

QByteArray OpcodePC::params() const
{
	return QByteArray().append((char)charID);
}

OpcodeCHAR::OpcodeCHAR(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCHAR::setParams(const QByteArray &params)
{
	objectID = params.at(0);
}

QString OpcodeCHAR::toString() const
{
	return QObject::tr("Ce groupe est un objet 3D (id=%1)")
			.arg(objectID);
}

QByteArray OpcodeCHAR::params() const
{
	return QByteArray().append((char)objectID);
}

OpcodeDFANM::OpcodeDFANM(const QByteArray &params)
{
	setParams(params);
}

void OpcodeDFANM::setParams(const QByteArray &params)
{
	animID = params.at(0);
	speed = params.at(1);
}

QString OpcodeDFANM::toString() const
{
	return QObject::tr("Joue l'animation %1 de l'objet 3D (vitesse=%2)")
			.arg(animID)
			.arg(speed);
}

QByteArray OpcodeDFANM::params() const
{
	return QByteArray()
			.append((char)animID)
			.append((char)speed);
}

OpcodeANIME1::OpcodeANIME1(const QByteArray &params)
{
	setParams(params);
}

void OpcodeANIME1::setParams(const QByteArray &params)
{
	animID = params.at(0);
	speed = params.at(1);
}

QString OpcodeANIME1::toString() const
{
	return QObject::tr("Joue l'animation %1 de l'objet 3D et retourne à l'état précédent (vitesse=%2)")
			.arg(animID)
			.arg(speed);
}

QByteArray OpcodeANIME1::params() const
{
	return QByteArray()
			.append((char)animID)
			.append((char)speed);
}

OpcodeVISI::OpcodeVISI(const QByteArray &params)
{
	setParams(params);
}

void OpcodeVISI::setParams(const QByteArray &params)
{
	show = params.at(0);
}

QString OpcodeVISI::toString() const
{
	return QObject::tr("%1 l'objet 3D")
			.arg(show == 0 ? QObject::tr("Cacher") : QObject::tr("Afficher"));
}

QByteArray OpcodeVISI::params() const
{
	return QByteArray()
			.append((char)show);
}

OpcodeXYZI::OpcodeXYZI(const QByteArray &params)
{
	setParams(params);
}

void OpcodeXYZI::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 2);
	memcpy(&targetX, &(params.constData()[2]), 2); // bank 1
	memcpy(&targetY, &(params.constData()[4]), 2); // bank 2
	memcpy(&targetZ, &(params.constData()[6]), 2); // bank 3
	memcpy(&targetI, &(params.constData()[8]), 2); // bank 4
}

QString OpcodeXYZI::toString() const
{
	return QObject::tr("Place l'objet 3D (X=%1, Y=%2, Z=%3, polygone id=%4)")
			.arg(_var(targetX, B1(banks[0])))
			.arg(_var(targetY, B2(banks[0])))
			.arg(_var(targetZ, B1(banks[1])))
			.arg(_var(targetI, B2(banks[1])));
}

QByteArray OpcodeXYZI::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&targetZ, 2)
			.append((char *)&targetI, 2);
}

void OpcodeXYZI::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetZ & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), targetI & 0xFF));
}

OpcodeXYI::OpcodeXYI(const QByteArray &params)
{
	setParams(params);
}

void OpcodeXYI::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 2);
	memcpy(&targetX, &(params.constData()[2]), 2); // bank 1
	memcpy(&targetY, &(params.constData()[4]), 2); // bank 2
	memcpy(&targetI, &(params.constData()[6]), 2); // bank 3
}

QString OpcodeXYI::toString() const
{
	return QObject::tr("Place l'objet 3D (X=%1, Y=%2, polygone id=%4)")
			.arg(_var(targetX, B1(banks[0])))
			.arg(_var(targetY, B2(banks[0])))
			.arg(_var(targetI, B1(banks[1])));
}

QByteArray OpcodeXYI::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&targetI, 2);
}

void OpcodeXYI::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetI & 0xFF));
}

OpcodeXYZ::OpcodeXYZ(const QByteArray &params)
{
	setParams(params);
}

void OpcodeXYZ::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 2);
	memcpy(&targetX, &(params.constData()[2]), 2); // bank 1
	memcpy(&targetY, &(params.constData()[4]), 2); // bank 2
	memcpy(&targetZ, &(params.constData()[6]), 2); // bank 3
}

QString OpcodeXYZ::toString() const
{
	return QObject::tr("Place l'objet 3D (X=%1, Y=%2, Z=%3)")
			.arg(_var(targetX, B1(banks[0])))
			.arg(_var(targetY, B2(banks[0])))
			.arg(_var(targetZ, B1(banks[1])));
}

QByteArray OpcodeXYZ::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&targetZ, 2);
}

void OpcodeXYZ::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetZ & 0xFF));
}

OpcodeMOVE::OpcodeMOVE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMOVE::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&targetX, &(params.constData()[1]), 2); // bank 1
	memcpy(&targetY, &(params.constData()[3]), 2); // bank 2
}

QString OpcodeMOVE::toString() const
{
	return QObject::tr("Déplace l'objet 3D (X=%1, Y=%2)")
			.arg(_var(targetX, B1(banks)))
			.arg(_var(targetY, B2(banks)));
}

QByteArray OpcodeMOVE::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2);
}

void OpcodeMOVE::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), targetX & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), targetY & 0xFF));
}

OpcodeCMOVE::OpcodeCMOVE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCMOVE::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&targetX, &(params.constData()[1]), 2); // bank 1
	memcpy(&targetY, &(params.constData()[3]), 2); // bank 2
}

QString OpcodeCMOVE::toString() const
{
	return QObject::tr("Déplace l'objet 3D sans animation (X=%1, Y=%2)")
			.arg(_var(targetX, B1(banks)))
			.arg(_var(targetY, B2(banks)));
}

QByteArray OpcodeCMOVE::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2);
}

void OpcodeCMOVE::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), targetX & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), targetY & 0xFF));
}

OpcodeMOVA::OpcodeMOVA(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMOVA::setParams(const QByteArray &params)
{
	groupID = params.at(0);
}

QString OpcodeMOVA::toString() const
{
	return QObject::tr("Déplace l'objet 3D vers le groupe %1")
			.arg(_script(groupID));
}

QByteArray OpcodeMOVA::params() const
{
	return QByteArray()
			.append((char)groupID);
}

OpcodeTURA::OpcodeTURA(const QByteArray &params)
{
	setParams(params);
}

void OpcodeTURA::setParams(const QByteArray &params)
{
	groupID = params.at(0);
	directionRotation = params.at(1);
	speed = params.at(2);
}

QString OpcodeTURA::toString() const
{
	return QObject::tr("Rotation de l'objet 3D vers le groupe %1 (vitesse=%2, SensRotation=%3)")
			.arg(_script(groupID))
			.arg(_sensRotation(directionRotation))
			.arg(speed);
}

QByteArray OpcodeTURA::params() const
{
	return QByteArray()
			.append((char)groupID)
			.append((char)directionRotation)
			.append((char)speed);
}

OpcodeANIMW::OpcodeANIMW()
{
}

QString OpcodeANIMW::toString() const
{
	return QObject::tr("Attendre que l'animation soit terminée pour continuer");
}

OpcodeFMOVE::OpcodeFMOVE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeFMOVE::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&targetX, &(params.constData()[1]), 2);
	memcpy(&targetY, &(params.constData()[3]), 2);
}

QString OpcodeFMOVE::toString() const
{
	return QObject::tr("Déplace l'objet 3D sans animation (X=%1, Y=%2)")
			.arg(_var(targetX, B1(banks)))
			.arg(_var(targetY, B2(banks)));
}

QByteArray OpcodeFMOVE::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2);
}

void OpcodeFMOVE::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), targetX & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), targetY & 0xFF));
}

OpcodeANIME2::OpcodeANIME2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeANIME2::setParams(const QByteArray &params)
{
	animID = params.at(0);
	speed = params.at(1);
}

QString OpcodeANIME2::toString() const
{
	return QObject::tr("Joue l'animation %1 de l'objet 3D et retourne à l'état précédent (vitesse=%2)")
			.arg(animID)
			.arg(speed);
}

QByteArray OpcodeANIME2::params() const
{
	return QByteArray()
			.append((char)animID)
			.append((char)speed);
}

OpcodeANIMX1::OpcodeANIMX1(const QByteArray &params)
{
	setParams(params);
}

void OpcodeANIMX1::setParams(const QByteArray &params)
{
	animID = params.at(0);
	speed = params.at(1);
}

QString OpcodeANIMX1::toString() const
{
	return QObject::tr("Joue l'animation %1 de l'objet 3D (vitesse=%2)")
			.arg(animID)
			.arg(speed);
}

QByteArray OpcodeANIMX1::params() const
{
	return QByteArray()
			.append((char)animID)
			.append((char)speed);
}

OpcodeCANIM1::OpcodeCANIM1(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCANIM1::setParams(const QByteArray &params)
{
	animID = params.at(0);
	firstFrame = params.at(1);
	lastFrame = params.at(2);
	speed = params.at(3);
}

QString OpcodeCANIM1::toString() const
{
	return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D et retourne à l'état précédent (première img=%2, dernière img=%3, vitesse=%4)")
			.arg(animID)
			.arg(firstFrame)
			.arg(lastFrame)
			.arg(speed);
}

QByteArray OpcodeCANIM1::params() const
{
	return QByteArray()
			.append((char)animID)
			.append((char)firstFrame)
			.append((char)lastFrame)
			.append((char)speed);
}

OpcodeCANMX1::OpcodeCANMX1(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCANMX1::setParams(const QByteArray &params)
{
	animID = params.at(0);
	firstFrame = params.at(1);
	lastFrame = params.at(2);
	speed = params.at(3);
}

QString OpcodeCANMX1::toString() const
{
	return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D (première img=%2, dernière img=%3, vitesse=%4)")
			.arg(animID)
			.arg(firstFrame)
			.arg(lastFrame)
			.arg(speed);
}

QByteArray OpcodeCANMX1::params() const
{
	return QByteArray()
			.append((char)animID)
			.append((char)firstFrame)
			.append((char)lastFrame)
			.append((char)speed);
}

OpcodeMSPED::OpcodeMSPED(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMSPED::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&speed, &(params.constData()[1]), 2); // bank 2
}

QString OpcodeMSPED::toString() const
{
	return QObject::tr("Configurer la vitesse des déplacements de l'objet 3D : %1")
			.arg(_var(speed, B2(banks)));
}

QByteArray OpcodeMSPED::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&speed, 2);
}

void OpcodeMSPED::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), speed & 0xFF));
}

OpcodeDIR::OpcodeDIR(const QByteArray &params)
{
	setParams(params);
}

void OpcodeDIR::setParams(const QByteArray &params)
{
	banks = params.at(0);
	direction = params.at(1); // bank 2
}

QString OpcodeDIR::toString() const
{
	return QObject::tr("Mettre l'objet 3D dans la direction : %1")
			.arg(_var(direction, B2(banks)));
}

QByteArray OpcodeDIR::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)direction);
}

void OpcodeDIR::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), direction));
}

OpcodeTURNGEN::OpcodeTURNGEN(const QByteArray &params)
{
	setParams(params);
}

void OpcodeTURNGEN::setParams(const QByteArray &params)
{
	banks = params.at(0);
	direction = params.at(1); // bank 2
	turnCount = params.at(2);
	speed = params.at(3);
	unknown = params.at(4);
}

QString OpcodeTURNGEN::toString() const
{
	return QObject::tr("Rotation (direction=%1, nbTours=%2, vitesse=%3, ?=%4)")
			.arg(_var(direction, B2(banks)))
			.arg(turnCount)
			.arg(speed)
			.arg(unknown);
}

QByteArray OpcodeTURNGEN::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)direction)
			.append((char)turnCount)
			.append((char)speed)
			.append((char)unknown);
}

void OpcodeTURNGEN::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), direction));
}

OpcodeTURN::OpcodeTURN(const QByteArray &params)
{
	setParams(params);
}

void OpcodeTURN::setParams(const QByteArray &params)
{
	banks = params.at(0);
	direction = params.at(1); // bank 2
	turnCount = params.at(2);
	speed = params.at(3);
	unknown = params.at(4);
}

QString OpcodeTURN::toString() const
{
	return QObject::tr("Rotation inversée (direction=%1, nbTours=%2, vitesse=%3, ?=%4)")
			.arg(_var(direction, B2(banks)))
			.arg(turnCount)
			.arg(speed)
			.arg(unknown);
}

QByteArray OpcodeTURN::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)direction)
			.append((char)turnCount)
			.append((char)speed)
			.append((char)unknown);
}

void OpcodeTURN::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), direction));
}

OpcodeDIRA::OpcodeDIRA(const QByteArray &params)
{
	setParams(params);
}

void OpcodeDIRA::setParams(const QByteArray &params)
{
	groupID = params.at(0);
}

QString OpcodeDIRA::toString() const
{
	return QObject::tr("Mettre l'objet 3D en direction du groupe %1")
			.arg(_script(groupID));
}

QByteArray OpcodeDIRA::params() const
{
	return QByteArray()
			.append((char)groupID);
}

OpcodeGETDIR::OpcodeGETDIR(const QByteArray &params)
{
	setParams(params);
}

void OpcodeGETDIR::setParams(const QByteArray &params)
{
	banks = params.at(0);
	groupID = params.at(1);
	varDir = params.at(2); // bank 2
}

QString OpcodeGETDIR::toString() const
{
	return QObject::tr("Stocker dans %2 la direction du groupe %1")
			.arg(_script(groupID))
			.arg(_bank(varDir, B2(banks)));
}

QByteArray OpcodeGETDIR::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)groupID)
			.append((char)varDir);
}

void OpcodeGETDIR::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), varDir));
}

OpcodeGETAXY::OpcodeGETAXY(const QByteArray &params)
{
	setParams(params);
}

void OpcodeGETAXY::setParams(const QByteArray &params)
{
	banks = params.at(0);
	groupID = params.at(1);
	varX = params.at(2); // bank 1
	varY = params.at(3); // bank 2
}

QString OpcodeGETAXY::toString() const
{
	return QObject::tr("Stocker dans %2 et %3 la position X et Y du groupe %1")
			.arg(_script(groupID))
			.arg(_bank(varX, B1(banks)))
			.arg(_bank(varY, B2(banks)));
}

QByteArray OpcodeGETAXY::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)groupID)
			.append((char)varX)
			.append((char)varY);
}

void OpcodeGETAXY::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), varX));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), varY));
}

OpcodeGETAI::OpcodeGETAI(const QByteArray &params)
{
	setParams(params);
}

void OpcodeGETAI::setParams(const QByteArray &params)
{
	banks = params.at(0);
	groupID = params.at(1);
	varI = params.at(2); // bank 2
}

QString OpcodeGETAI::toString() const
{
	return QObject::tr("Stocker dans %2 le polygone id du groupe %1")
			.arg(_script(groupID))
			.arg(_bank(varI, B2(banks)));
}

QByteArray OpcodeGETAI::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)groupID)
			.append((char)varI);
}

void OpcodeGETAI::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), varI));
}

OpcodeANIMX2::OpcodeANIMX2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeANIMX2::setParams(const QByteArray &params)
{
	animID = params.at(0);
	speed = params.at(1);
}

QString OpcodeANIMX2::toString() const
{
	return QObject::tr("Joue l'animation %1 de l'objet 3D (vitesse=%2)")
			.arg(animID)
			.arg(speed);
}

QByteArray OpcodeANIMX2::params() const
{
	return QByteArray()
			.append((char)animID)
			.append((char)speed);
}

OpcodeCANIM2::OpcodeCANIM2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCANIM2::setParams(const QByteArray &params)
{
	animID = params.at(0);
	firstFrame = params.at(1);
	lastFrame = params.at(2);
	speed = params.at(3);
}

QString OpcodeCANIM2::toString() const
{
	return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D et retourne à l'état précédent (première img=%2, dernière img=%3, vitesse=%4)")
			.arg(animID)
			.arg(firstFrame)
			.arg(lastFrame)
			.arg(speed);
}

QByteArray OpcodeCANIM2::params() const
{
	return QByteArray()
			.append((char)animID)
			.append((char)firstFrame)
			.append((char)lastFrame)
			.append((char)speed);
}

OpcodeCANMX2::OpcodeCANMX2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCANMX2::setParams(const QByteArray &params)
{
	animID = params.at(0);
	firstFrame = params.at(1);
	lastFrame = params.at(2);
	speed = params.at(3);
}

QString OpcodeCANMX2::toString() const
{
	return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D (première img=%2, dernière img=%3, vitesse=%4)")
			.arg(animID)
			.arg(firstFrame)
			.arg(lastFrame)
			.arg(speed);
}

QByteArray OpcodeCANMX2::params() const
{
	return QByteArray()
			.append((char)animID)
			.append((char)firstFrame)
			.append((char)lastFrame)
			.append((char)speed);
}

OpcodeASPED::OpcodeASPED(const QByteArray &params)
{
	setParams(params);
}

void OpcodeASPED::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&speed, &(params.constData()[1]), 2); // bank 2
}

QString OpcodeASPED::toString() const
{
	return QObject::tr("Configurer la vitesse des animations de l'objet 3D : %1")
			.arg(_var(speed, B2(banks)));
}

QByteArray OpcodeASPED::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&speed, 2);
}

void OpcodeASPED::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), speed));
}

OpcodeCC::OpcodeCC(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCC::setParams(const QByteArray &params)
{
	groupID = params.at(0);
}

QString OpcodeCC::toString() const
{
	return QObject::tr("Prendre le contrôle du groupe %1")
			.arg(_script(groupID));
}

QByteArray OpcodeCC::params() const
{
	return QByteArray()
			.append((char)groupID);
}

OpcodeJUMP::OpcodeJUMP(const QByteArray &params)
{
	setParams(params);
}

void OpcodeJUMP::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 2);
	memcpy(&targetX, &(params.constData()[2]), 2);
	memcpy(&targetY, &(params.constData()[4]), 2);
	memcpy(&targetI, &(params.constData()[6]), 2);
	memcpy(&height, &(params.constData()[8]), 2);
}

QString OpcodeJUMP::toString() const
{
	return QObject::tr("Faire sauter un personnage (X=%1, Y=%2, polygone id=%3, hauteur=%4)")
			.arg(_var(targetX, B1(banks[0])))
			.arg(_var(targetY, B2(banks[0])))
			.arg(_var(targetI, B1(banks[1])))
			.arg(_var(height, B2(banks[1])));
}

QByteArray OpcodeJUMP::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&targetI, 2)
			.append((char *)&height, 2);
}

void OpcodeJUMP::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), targetX));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetI));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), height));
}

OpcodeAXYZI::OpcodeAXYZI(const QByteArray &params)
{
	setParams(params);
}

void OpcodeAXYZI::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 2);
	groupID = params.at(2);
	varX = params.at(3); // bank 1
	varY = params.at(4); // bank 2
	varZ = params.at(5); // bank 3
	varI = params.at(6); // bank 4
}

QString OpcodeAXYZI::toString() const
{
	return QObject::tr("Stocker la position du groupe %1 dans des variables (%2=X, %3=Y, %4=Z, %5=polygone id)")
			.arg(_script(groupID))
			.arg(_bank(varX, B1(banks[0])))
			.arg(_bank(varY, B2(banks[0])))
			.arg(_bank(varZ, B1(banks[1])))
			.arg(_bank(varI, B2(banks[1])));
}

QByteArray OpcodeAXYZI::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)groupID)
			.append((char)varX)
			.append((char)varY)
			.append((char)varZ)
			.append((char)varI);
}

void OpcodeAXYZI::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), varX));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), varY));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), varZ));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), varI));
}

OpcodeLADER::OpcodeLADER(const QByteArray &params)
{
	setParams(params);
}

void OpcodeLADER::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 2);
	memcpy(&targetX, &(params.constData()[2]), 2); // bank 1
	memcpy(&targetY, &(params.constData()[4]), 2); // bank 2
	memcpy(&targetZ, &(params.constData()[6]), 2); // bank 3
	memcpy(&targetI, &(params.constData()[8]), 2); // bank 4
	way = params.at(10);
	animID = params.at(11);
	direction = params.at(12);
	speed = params.at(13);
}

QString OpcodeLADER::toString() const
{
	return QObject::tr("Monter une échelle avec l'animation %6 (X=%1, Y=%2, Z=%3, polygone id=%4, sens=%5, direction=%7, vitesse=%8)")
			.arg(_var(targetX, B1(banks[0])))
			.arg(_var(targetY, B2(banks[0])))
			.arg(_var(targetZ, B1(banks[1])))
			.arg(_var(targetI, B2(banks[1])))
			.arg(way)
			.arg(animID)
			.arg(direction)
			.arg(speed);
}

QByteArray OpcodeLADER::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&targetZ, 2)
			.append((char *)&targetI, 2)
			.append((char)way)
			.append((char)animID)
			.append((char)direction)
			.append((char)speed);
}

void OpcodeLADER::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetZ & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), targetI & 0xFF));
}

OpcodeOFST::OpcodeOFST(const QByteArray &params)
{
	setParams(params);
}

void OpcodeOFST::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 2);
	moveType = params.at(2);
	memcpy(&targetX, &(params.constData()[3]), 2); // bank 1
	memcpy(&targetY, &(params.constData()[5]), 2); // bank 2
	memcpy(&targetZ, &(params.constData()[7]), 2); // bank 3
	memcpy(&speed, &(params.constData()[9]), 2); // bank 4
}

QString OpcodeOFST::toString() const
{
	return QObject::tr("Offset Object (mouvement=%1, X=%2, Y=%3, Z=%4, vitesse=%5)")
			.arg(moveType)
			.arg(_var(targetX, B1(banks[0])))
			.arg(_var(targetY, B2(banks[0])))
			.arg(_var(targetZ, B1(banks[1])))
			.arg(_var(speed, B2(banks[1])));
}

QByteArray OpcodeOFST::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)moveType)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&targetZ, 2)
			.append((char *)&speed, 2);
}

void OpcodeOFST::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetZ & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), speed & 0xFF));
}

OpcodeOFSTW::OpcodeOFSTW()
{
}

QString OpcodeOFSTW::toString() const
{
	return QObject::tr("Attendre la fin de l'exécution de l'Offset Object pour continuer");
}

OpcodeTALKR::OpcodeTALKR(const QByteArray &params)
{
	setParams(params);
}

void OpcodeTALKR::setParams(const QByteArray &params)
{
	banks = params.at(0);
	distance = params.at(1); // bank 2
}

QString OpcodeTALKR::toString() const
{
	return QObject::tr("Modifier la distance nécessaire pour parler avec l'objet 3D : %1")
			.arg(_var(distance, B2(banks)));
}

QByteArray OpcodeTALKR::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)distance);
}

void OpcodeTALKR::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), distance));
}

OpcodeSLIDR::OpcodeSLIDR(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSLIDR::setParams(const QByteArray &params)
{
	banks = params.at(0);
	distance = params.at(1); // bank 2
}

QString OpcodeSLIDR::toString() const
{
	return QObject::tr("Modifier la distance nécessaire pour toucher l'objet 3D : %1")
			.arg(_var(distance, B2(banks)));
}

QByteArray OpcodeSLIDR::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)distance);
}

void OpcodeSLIDR::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), distance));
}

OpcodeSOLID::OpcodeSOLID(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSOLID::setParams(const QByteArray &params)
{
	disabled = params.at(0);
}

QString OpcodeSOLID::toString() const
{
	return QObject::tr("%1 la possibilité de toucher l'objet 3D")
			.arg(disabled == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeSOLID::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodePRTYP::OpcodePRTYP(const QByteArray &params)
{
	setParams(params);
}

void OpcodePRTYP::setParams(const QByteArray &params)
{
	charID = params.at(0);
}

QString OpcodePRTYP::toString() const
{
	return QObject::tr("Ajouter %1 à l'équipe actuelle")
			.arg(_personnage(charID));
}

QByteArray OpcodePRTYP::params() const
{
	return QByteArray().append((char)charID);
}

OpcodePRTYM::OpcodePRTYM(const QByteArray &params)
{
	setParams(params);
}

void OpcodePRTYM::setParams(const QByteArray &params)
{
	charID = params.at(0);
}

QString OpcodePRTYM::toString() const
{
	return QObject::tr("Retirer %1 de l'équipe actuelle")
			.arg(_personnage(charID));
}

QByteArray OpcodePRTYM::params() const
{
	return QByteArray().append((char)charID);
}

OpcodePRTYE::OpcodePRTYE(const QByteArray &params)
{
	setParams(params);
}

void OpcodePRTYE::setParams(const QByteArray &params)
{
	memcpy(charID, params.constData(), 3);
}

QString OpcodePRTYE::toString() const
{
	return QObject::tr("Nouvelle équipe : %1 | %2 | %3")
			.arg(_personnage(charID[0]))
			.arg(_personnage(charID[1]))
			.arg(_personnage(charID[2]));
}

QByteArray OpcodePRTYE::params() const
{
	return QByteArray().append((char *)&charID, 3);
}

OpcodeIfQ::OpcodeIfQ(const QByteArray &params) :
	OpcodeJump()
{
	setParams(params);
}

void OpcodeIfQ::setParams(const QByteArray &params)
{
	charID = params.at(0);
	_jump = (quint8)params.at(1) + jumpPosData();
}

QByteArray OpcodeIfQ::params() const
{
	return QByteArray()
			.append((char)charID)
			.append(char(_jump - jumpPosData()));
}

OpcodeIFPRTYQ::OpcodeIFPRTYQ(const QByteArray &params) :
	OpcodeIfQ(params)
{
}

QString OpcodeIFPRTYQ::toString() const
{
	return QObject::tr("Si %1 est dans l'équipe actuelle (aller au label %2 sinon)")
			.arg(_personnage(charID))
			.arg(_label);
}

OpcodeIFMEMBQ::OpcodeIFMEMBQ(const QByteArray &params) :
	OpcodeIfQ(params)
{
}

QString OpcodeIFMEMBQ::toString() const
{
	return QObject::tr("Si %1 existe (aller au label %2 sinon)")
			.arg(_personnage(charID))
			.arg(_label);
}

OpcodeMMBUD::OpcodeMMBUD(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMMBUD::setParams(const QByteArray &params)
{
	exists = params.at(0); // boolean
	charID = params.at(1);
}

QString OpcodeMMBUD::toString() const
{
	return QObject::tr("%2 %1")
			.arg(exists == 0 ? QObject::tr("n'existe plus") : QObject::tr("existe"))
			.arg(_personnage(charID));
}

QByteArray OpcodeMMBUD::params() const
{
	return QByteArray()
			.append((char)exists)
			.append((char)charID);
}

OpcodeMMBLK::OpcodeMMBLK(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMMBLK::setParams(const QByteArray &params)
{
	charID = params.at(0);
}

QString OpcodeMMBLK::toString() const
{
	return QObject::tr("Bloque %1 dans le menu PHS")
			.arg(_personnage(charID));
}

QByteArray OpcodeMMBLK::params() const
{
	return QByteArray().append((char)charID);
}

OpcodeMMBUK::OpcodeMMBUK(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMMBUK::setParams(const QByteArray &params)
{
	charID = params.at(0);
}

QString OpcodeMMBUK::toString() const
{
	return QObject::tr("Débloque %1 dans le menu PHS")
			.arg(_personnage(charID));
}

QByteArray OpcodeMMBUK::params() const
{
	return QByteArray().append((char)charID);
}

OpcodeLINE::OpcodeLINE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeLINE::setParams(const QByteArray &params)
{
	memcpy(&targetX1, params.constData(), 2);
	memcpy(&targetY1, &(params.constData()[2]), 2);
	memcpy(&targetZ1, &(params.constData()[4]), 2);
	memcpy(&targetX2, &(params.constData()[6]), 2);
	memcpy(&targetY2, &(params.constData()[8]), 2);
	memcpy(&targetZ2, &(params.constData()[10]), 2);
}

QString OpcodeLINE::toString() const
{
	return QObject::tr("Définit la zone (X1=%1, Y1=%2, Z1=%3, X2=%4, Y2=%5, Z2=%6)")
			.arg(targetX1)
			.arg(targetY1)
			.arg(targetZ1)
			.arg(targetX2)
			.arg(targetY2)
			.arg(targetZ2);
}

QByteArray OpcodeLINE::params() const
{
	return QByteArray()
			.append((char *)&targetX1, 2)
			.append((char *)&targetY1, 2)
			.append((char *)&targetZ1, 2)
			.append((char *)&targetX2, 2)
			.append((char *)&targetY2, 2)
			.append((char *)&targetZ2, 2);
}

OpcodeLINON::OpcodeLINON(const QByteArray &params)
{
	setParams(params);
}

void OpcodeLINON::setParams(const QByteArray &params)
{
	enabled = params.at(0);
}

QString OpcodeLINON::toString() const
{
	return QObject::tr("%1 la zone")
			.arg(enabled == 0 ? QObject::tr("Effacer") : QObject::tr("Tracer"));
}

QByteArray OpcodeLINON::params() const
{
	return QByteArray().append((char)enabled);
}

OpcodeMPJPO::OpcodeMPJPO(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMPJPO::setParams(const QByteArray &params)
{
	prevent = params.at(0);
}

QString OpcodeMPJPO::toString() const
{
	return QObject::tr("%1 les changements de décor par le joueur")
			.arg(prevent == 0 ? QObject::tr("Autoriser") : QObject::tr("Empêcher"));
}

QByteArray OpcodeMPJPO::params() const
{
	return QByteArray().append((char)prevent);
}

OpcodeSLINE::OpcodeSLINE(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSLINE::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 3);
	memcpy(&targetX1, &(params.constData()[3]), 2); // bank 1
	memcpy(&targetY1, &(params.constData()[5]), 2); // bank 2
	memcpy(&targetZ1, &(params.constData()[7]), 2); // bank 3
	memcpy(&targetX2, &(params.constData()[9]), 2); // bank 4
	memcpy(&targetY2, &(params.constData()[11]), 2); // bank 5
	memcpy(&targetZ2, &(params.constData()[13]), 2); // bank 6
}

QString OpcodeSLINE::toString() const
{
	return QObject::tr("Redimensionner la zone (X1=%1, Y1=%2, Z1=%3, X2=%4, Y2=%5, Z2=%6)")
			.arg(_var(targetX1, B1(banks[0])))
			.arg(_var(targetY1, B2(banks[0])))
			.arg(_var(targetZ1, B1(banks[1])))
			.arg(_var(targetX2, B2(banks[1])))
			.arg(_var(targetY2, B1(banks[2])))
			.arg(_var(targetZ2, B2(banks[2])));
}

QByteArray OpcodeSLINE::params() const
{
	return QByteArray()
			.append((char *)&banks, 3)
			.append((char *)&targetX1, 2)
			.append((char *)&targetY1, 2)
			.append((char *)&targetZ1, 2)
			.append((char *)&targetX2, 2)
			.append((char *)&targetY2, 2)
			.append((char *)&targetZ2, 2);
}

void OpcodeSLINE::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), targetX1 & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY1 & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetZ1 & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), targetX2));
	if(B1(banks[2]) != 0)
		vars.append(FF7Var(B1(banks[2]), targetY2 & 0xFF));
	if(B2(banks[2]) != 0)
		vars.append(FF7Var(B2(banks[2]), targetZ2));
}

OpcodeSIN::OpcodeSIN(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSIN::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 2);
	memcpy(&value1, &(params.constData()[2]), 2); // bank 1
	memcpy(&value2, &(params.constData()[4]), 2); // bank 2
	memcpy(&value3, &(params.constData()[6]), 2); // bank 3
	var = params.at(8); // bank 4
}

QString OpcodeSIN::toString() const
{
	return QObject::tr("%4 = Sinus(%1) %2 %3")
			.arg(_var(value1, B1(banks[0])))
			.arg(_var(value2, B2(banks[0])))
			.arg(_var(value3, B1(banks[1])))
			.arg(_bank(var, B2(banks[1])));
}

QByteArray OpcodeSIN::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char *)&value1, 2)
			.append((char *)&value2, 2)
			.append((char *)&value3, 2)
			.append((char)var);
}

void OpcodeSIN::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), value1 & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), value2 & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), value3 & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), var));
}

OpcodeCOS::OpcodeCOS(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCOS::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 2);
	memcpy(&value1, &(params.constData()[2]), 2); // bank 1
	memcpy(&value2, &(params.constData()[4]), 2); // bank 2
	memcpy(&value3, &(params.constData()[6]), 2); // bank 3
	var = params.at(8); // bank 4
}

QString OpcodeCOS::toString() const
{
	return QObject::tr("%4 = Cosinus(%1) %2 %3")
			.arg(_var(value1, B1(banks[0])))
			.arg(_var(value2, B2(banks[0])))
			.arg(_var(value3, B1(banks[1])))
			.arg(_bank(var, B2(banks[1])));
}

QByteArray OpcodeCOS::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char *)&value1, 2)
			.append((char *)&value2, 2)
			.append((char *)&value3, 2)
			.append((char)var);
}

void OpcodeCOS::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), value1 & 0xFF));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), value2 & 0xFF));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), value3 & 0xFF));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), var));
}

OpcodeTLKR2::OpcodeTLKR2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeTLKR2::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&distance, &(params.constData()[1]), 2); // bank 2
}

QString OpcodeTLKR2::toString() const
{
	return QObject::tr("Modifier la distance nécessaire pour parler avec l'objet 3D : %1")
			.arg(_var(distance, B2(banks)));
}

QByteArray OpcodeTLKR2::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&distance, 2);
}

void OpcodeTLKR2::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), distance & 0xFF));
}

OpcodeSLDR2::OpcodeSLDR2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSLDR2::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&distance, &(params.constData()[1]), 2); // bank 2
}

QString OpcodeSLDR2::toString() const
{
	return QObject::tr("Modifier la distance nécessaire pour toucher l'objet 3D : %1")
			.arg(_var(distance, B2(banks)));
}

QByteArray OpcodeSLDR2::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&distance, 2);
}

void OpcodeSLDR2::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), distance & 0xFF));
}

OpcodePMJMP::OpcodePMJMP(const QByteArray &params)
{
	setParams(params);
}

void OpcodePMJMP::setParams(const QByteArray &params)
{
	memcpy(&fieldID, params.constData(), 2);
}

QString OpcodePMJMP::toString() const
{
	return QObject::tr("Commencer à charger l'écran %1")
			.arg(_field(fieldID));
}

QByteArray OpcodePMJMP::params() const
{
	return QByteArray().append((char *)&fieldID, 2);
}

OpcodePMJMP2::OpcodePMJMP2()
{
}

QString OpcodePMJMP2::toString() const
{
	return QObject::tr("PMJMP2");
}

OpcodeAKAO2::OpcodeAKAO2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeAKAO2::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 14);
}

QString OpcodeAKAO2::toString() const
{
	return QObject::tr("AKAO2");
}

QByteArray OpcodeAKAO2::params() const
{
	return QByteArray().append((char *)&unknown, 14);
}

OpcodeFCFIX::OpcodeFCFIX(const QByteArray &params)
{
	setParams(params);
}

void OpcodeFCFIX::setParams(const QByteArray &params)
{
	disabled = params.at(0);
}

QString OpcodeFCFIX::toString() const
{
	return QObject::tr("%1 rotation")
			.arg(disabled == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeFCFIX::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodeCCANM::OpcodeCCANM(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCCANM::setParams(const QByteArray &params)
{
	animID = params.at(0);
	speed = params.at(1);
	standWalkRun = params.at(2);
}

QString OpcodeCCANM::toString() const
{
	return QObject::tr("Jouer animation n°%1 pour '%3' (vitesse=%2)")
			.arg(animID)
			.arg(speed)
			.arg(standWalkRun == 0 ? QObject::tr("rester immobile")
								   : (standWalkRun == 1 ? QObject::tr("marcher")
														: QObject::tr("courir")));
}

QByteArray OpcodeCCANM::params() const
{
	return QByteArray()
			.append((char)animID)
			.append((char)speed)
			.append((char)standWalkRun);
}

OpcodeANIMB::OpcodeANIMB()
{
}

QString OpcodeANIMB::toString() const
{
	return QObject::tr("Stoppe l'animation de l'objet 3D");
}

OpcodeTURNW::OpcodeTURNW()
{
}

QString OpcodeTURNW::toString() const
{
	return QObject::tr("Attendre que la rotation soit terminée pour continuer");
}

OpcodeMPPAL::OpcodeMPPAL(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMPPAL::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 10);
}

QString OpcodeMPPAL::toString() const
{
	return QObject::tr("MPPAL");
}

QByteArray OpcodeMPPAL::params() const
{
	return QByteArray().append((char *)&unknown, 10);
}

OpcodeBGON::OpcodeBGON(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBGON::setParams(const QByteArray &params)
{
	banks = params.at(0);
	paramID = params.at(1); // bank 1
	stateID = params.at(2); // bank 2
}

QString OpcodeBGON::toString() const
{
	return QObject::tr("Afficher l'état n°%2 du paramètre n°%1")
			.arg(_var(paramID, B1(banks)))
			.arg(_var(stateID, B2(banks)));
}

QByteArray OpcodeBGON::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)paramID)
			.append((char)stateID);
}

void OpcodeBGON::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), paramID));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), stateID));
}

OpcodeBGOFF::OpcodeBGOFF(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBGOFF::setParams(const QByteArray &params)
{
	banks = params.at(0);
	paramID = params.at(1); // bank 1
	stateID = params.at(2); // bank 2
}

QString OpcodeBGOFF::toString() const
{
	return QObject::tr("Cacher l'état n°%2 du paramètre n°%1")
			.arg(_var(paramID, B1(banks)))
			.arg(_var(stateID, B2(banks)));
}

QByteArray OpcodeBGOFF::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)paramID)
			.append((char)stateID);
}

void OpcodeBGOFF::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), paramID));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), stateID));
}

OpcodeBGROL::OpcodeBGROL(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBGROL::setParams(const QByteArray &params)
{
	banks = params.at(0);
	paramID = params.at(1); // bank 2
}

QString OpcodeBGROL::toString() const
{
	return QObject::tr("Afficher l'état suivant du paramètre n°%1")
			.arg(_var(paramID, B2(banks)));
}

QByteArray OpcodeBGROL::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)paramID);
}

void OpcodeBGROL::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), paramID));
}

OpcodeBGROL2::OpcodeBGROL2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBGROL2::setParams(const QByteArray &params)
{
	banks = params.at(0);
	paramID = params.at(1); // bank 2
}

QString OpcodeBGROL2::toString() const
{
	return QObject::tr("Afficher l'état précédent du paramètre n°%1")
			.arg(_var(paramID, B2(banks)));
}

QByteArray OpcodeBGROL2::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)paramID);
}

void OpcodeBGROL2::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), paramID));
}

OpcodeBGCLR::OpcodeBGCLR(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBGCLR::setParams(const QByteArray &params)
{
	banks = params.at(0);
	paramID = params.at(1); // bank 2
}

QString OpcodeBGCLR::toString() const
{
	return QObject::tr("Cacher paramètre n°%1")
			.arg(_var(paramID, B2(banks)));
}

QByteArray OpcodeBGCLR::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)paramID);
}

void OpcodeBGCLR::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), paramID));
}

OpcodeSTPAL::OpcodeSTPAL(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSTPAL::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 4);
}

QString OpcodeSTPAL::toString() const
{
	return QObject::tr("STPAL");
}

QByteArray OpcodeSTPAL::params() const
{
	return QByteArray().append((char *)&unknown, 4);
}

OpcodeLDPAL::OpcodeLDPAL(const QByteArray &params)
{
	setParams(params);
}

void OpcodeLDPAL::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 4);
}

QString OpcodeLDPAL::toString() const
{
	return QObject::tr("LDPAL");
}

QByteArray OpcodeLDPAL::params() const
{
	return QByteArray().append((char *)&unknown, 4);
}

OpcodeCPPAL::OpcodeCPPAL(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCPPAL::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 4);
}

QString OpcodeCPPAL::toString() const
{
	return QObject::tr("CPPAL");
}

QByteArray OpcodeCPPAL::params() const
{
	return QByteArray().append((char *)&unknown, 4);
}

OpcodeRTPAL::OpcodeRTPAL(const QByteArray &params)
{
	setParams(params);
}

void OpcodeRTPAL::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 6);
}

QString OpcodeRTPAL::toString() const
{
	return QObject::tr("RTPAL");
}

QByteArray OpcodeRTPAL::params() const
{
	return QByteArray().append((char *)&unknown, 6);
}

OpcodeADPAL::OpcodeADPAL(const QByteArray &params)
{
	setParams(params);
}

void OpcodeADPAL::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 9);
}

QString OpcodeADPAL::toString() const
{
	return QObject::tr("ADPAL");
}

QByteArray OpcodeADPAL::params() const
{
	return QByteArray().append((char *)&unknown, 9);
}

OpcodeMPPAL2::OpcodeMPPAL2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMPPAL2::setParams(const QByteArray &params)
{
	memcpy(banks, params.constData(), 3);
	memcpy(unknown1, &(params.constData()[3]), 2);
	r = params.at(5); // bank 3
	g = params.at(6); // bank 4
	b = params.at(7); // bank 5
	unknown2 = params.at(8);
}

QString OpcodeMPPAL2::toString() const
{
	return QObject::tr("MPPAL2 (R=%1, V=%2, B=%3)")
			.arg(_var(r, B1(banks[1])))
			.arg(_var(g, B2(banks[1])))
			.arg(_var(b, B1(banks[2])));
}

QByteArray OpcodeMPPAL2::params() const
{
	return QByteArray()
			.append((char *)&banks, 3)
			.append((char *)&unknown1, 2)
			.append((char)r)
			.append((char)g)
			.append((char)b)
			.append((char)unknown2);
}

void OpcodeMPPAL2::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), r));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), g));
	if(B1(banks[2]) != 0)
		vars.append(FF7Var(B2(banks[1]), b));
}

OpcodeSTPLS::OpcodeSTPLS(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSTPLS::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 4);
}

QString OpcodeSTPLS::toString() const
{
	return QObject::tr("STPLS");
}

QByteArray OpcodeSTPLS::params() const
{
	return QByteArray().append((char *)&unknown, 4);
}

OpcodeLDPLS::OpcodeLDPLS(const QByteArray &params)
{
	setParams(params);
}

void OpcodeLDPLS::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 4);
}

QString OpcodeLDPLS::toString() const
{
	return QObject::tr("LDPLS");
}

QByteArray OpcodeLDPLS::params() const
{
	return QByteArray().append((char *)&unknown, 4);
}

OpcodeCPPAL2::OpcodeCPPAL2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCPPAL2::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 7);
}

QString OpcodeCPPAL2::toString() const
{
	return QObject::tr("CPPAL2");
}

QByteArray OpcodeCPPAL2::params() const
{
	return QByteArray().append((char *)&unknown, 7);
}

OpcodeRTPAL2::OpcodeRTPAL2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeRTPAL2::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 7);
}

QString OpcodeRTPAL2::toString() const
{
	return QObject::tr("RTPAL2");
}

QByteArray OpcodeRTPAL2::params() const
{
	return QByteArray().append((char *)&unknown, 7);
}

OpcodeADPAL2::OpcodeADPAL2(const QByteArray &params)
{
	setParams(params);
}

void OpcodeADPAL2::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 10);
}

QString OpcodeADPAL2::toString() const
{
	return QObject::tr("ADPAL2");
}

QByteArray OpcodeADPAL2::params() const
{
	return QByteArray().append((char *)&unknown, 10);
}

OpcodeMUSIC::OpcodeMUSIC(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMUSIC::setParams(const QByteArray &params)
{
	musicID = params.at(0);
}

QString OpcodeMUSIC::toString() const
{
	return QObject::tr("Jouer musique n°%1")
			.arg(musicID);
}

QByteArray OpcodeMUSIC::params() const
{
	return QByteArray().append((char)musicID);
}

OpcodeSOUND::OpcodeSOUND(const QByteArray &params)
{
	setParams(params);
}

void OpcodeSOUND::setParams(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&soundID, &(params.constData()[1]), 2); // bank 1
	position = params.at(3); // bank 2
}

QString OpcodeSOUND::toString() const
{
	return QObject::tr("Jouer son n°%1 (position=%2/127)")
			.arg(_var(soundID, B1(banks)))
			.arg(_var(position, B2(banks)));
}

QByteArray OpcodeSOUND::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&soundID, 2)
			.append((char)position);
}

void OpcodeSOUND::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), soundID & 0xFF));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), position));
}

OpcodeAKAO::OpcodeAKAO(const QByteArray &params)
{
	setParams(params);
}

void OpcodeAKAO::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 13);
}

QString OpcodeAKAO::toString() const
{
	return QObject::tr("AKAO");
}

QByteArray OpcodeAKAO::params() const
{
	return QByteArray().append((char *)&unknown, 13);
}

OpcodeMUSVT::OpcodeMUSVT(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMUSVT::setParams(const QByteArray &params)
{
	musicID = params.at(0);
}

QString OpcodeMUSVT::toString() const
{
	return QObject::tr("MUSVT (musique n°%1)")
			.arg(musicID);
}

QByteArray OpcodeMUSVT::params() const
{
	return QByteArray().append((char)musicID);
}

OpcodeMUSVM::OpcodeMUSVM(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMUSVM::setParams(const QByteArray &params)
{
	musicID = params.at(0);
}

QString OpcodeMUSVM::toString() const
{
	return QObject::tr("MUSVM (musique n°%1)")
			.arg(musicID);
}

QByteArray OpcodeMUSVM::params() const
{
	return QByteArray().append((char)musicID);
}

OpcodeMULCK::OpcodeMULCK(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMULCK::setParams(const QByteArray &params)
{
	locked = params.at(0);
}

QString OpcodeMULCK::toString() const
{
	return QObject::tr("%1 musique")
			.arg(locked == 0 ? QObject::tr("Déverrouiller") : QObject::tr("Verrouiller", "test"));
}

QByteArray OpcodeMULCK::params() const
{
	return QByteArray().append((char)locked);
}

OpcodeBMUSC::OpcodeBMUSC(const QByteArray &params)
{
	setParams(params);
}

void OpcodeBMUSC::setParams(const QByteArray &params)
{
	musicID = params.at(0);
}

QString OpcodeBMUSC::toString() const
{
	return QObject::tr("Choisir musique n°%1 comme musique de combat")
			.arg(musicID);
}

QByteArray OpcodeBMUSC::params() const
{
	return QByteArray().append((char)musicID);
}

OpcodeCHMPH::OpcodeCHMPH(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCHMPH::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 3);
}

QString OpcodeCHMPH::toString() const
{
	return QObject::tr("CHMPH");
}

QByteArray OpcodeCHMPH::params() const
{
	return QByteArray().append((char *)&unknown, 3);
}

OpcodePMVIE::OpcodePMVIE(const QByteArray &params)
{
	setParams(params);
}

void OpcodePMVIE::setParams(const QByteArray &params)
{
	movieID = params.at(0);
}

QString OpcodePMVIE::toString() const
{
	return QObject::tr("Choisir prochaine cinématique : %1")
			.arg(_movie(movieID));
}

QByteArray OpcodePMVIE::params() const
{
	return QByteArray().append((char)movieID);
}

OpcodeMOVIE::OpcodeMOVIE()
{
}

QString OpcodeMOVIE::toString() const
{
	return QObject::tr("Jouer la cinématique choisie");
}

OpcodeMVIEF::OpcodeMVIEF(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMVIEF::setParams(const QByteArray &params)
{
	banks = params.at(0);
	varCurMovieFrame = params.at(1);
}

QString OpcodeMVIEF::toString() const
{
	return QObject::tr("Stocker Movie frame dans %1")
			.arg(_bank(varCurMovieFrame, B2(banks)));
}

QByteArray OpcodeMVIEF::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)varCurMovieFrame);
}

void OpcodeMVIEF::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), varCurMovieFrame));
}

OpcodeMVCAM::OpcodeMVCAM(const QByteArray &params)
{
	setParams(params);
}

void OpcodeMVCAM::setParams(const QByteArray &params)
{
	movieCamID = params.at(0);
}

QString OpcodeMVCAM::toString() const
{
	return QObject::tr("Camera Movie : %1")
			.arg(movieCamID);
}

QByteArray OpcodeMVCAM::params() const
{
	return QByteArray().append((char)movieCamID);
}

OpcodeFMUSC::OpcodeFMUSC(const QByteArray &params)
{
	setParams(params);
}

void OpcodeFMUSC::setParams(const QByteArray &params)
{
	unknown = params.at(0);
}

QString OpcodeFMUSC::toString() const
{
	return QObject::tr("FMUSC (?=%1)")
			.arg(unknown);
}

QByteArray OpcodeFMUSC::params() const
{
	return QByteArray().append((char)unknown);
}

OpcodeCMUSC::OpcodeCMUSC(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCMUSC::setParams(const QByteArray &params)
{
	memcpy(unknown, params.constData(), 5);
}

QString OpcodeCMUSC::toString() const
{
	return QObject::tr("CMUSC");
}

QByteArray OpcodeCMUSC::params() const
{
	return QByteArray().append((char *)&unknown, 5);
}

OpcodeCHMST::OpcodeCHMST(const QByteArray &params)
{
	setParams(params);
}

void OpcodeCHMST::setParams(const QByteArray &params)
{
	banks = params.at(0);
	var = params.at(1); // bank 2
}

QString OpcodeCHMST::toString() const
{
	return QObject::tr("Si la musique est jouée mettre %1 à 1")
			.arg(_bank(var, B2(banks)));
}

QByteArray OpcodeCHMST::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)var);
}

void OpcodeCHMST::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), var));
}

OpcodeGAMEOVER::OpcodeGAMEOVER()
{
}

QString OpcodeGAMEOVER::toString() const
{
	return QObject::tr("Game Over");
}

const quint8 Opcode::length[257] =
{
/*00*//* RET */			1,
/*01*//* REQ */			3,
/*02*//* REQSW */		3,
/*03*//* REQEW */		3,
/*04*//* PREQ */		3,
/*05*//* PRQSW */		3,
/*06*//* PRQEW */		3,
/*07*//* RETTO */		2,
/*08*//* JOIN */		2,
/*09*//* SPLIT */		15,
/*0a*//* SPTYE */		6,
/*0b*//* GTPYE */		6,
/*0c*//*  */			1,
/*0d*//*  */			1,
/*0e*//* DSKCG */		2,
/*0f*//* SPECIAL */		2,
	
/*10*//* JMPF */		2,
/*11*//* JMPFL */		3,
/*12*//* JMPB */		2,
/*13*//* JMPBL */		3,
/*14*//* IFUB */		6,
/*15*//* IFUBL */		7,
/*16*//* IFSW */		8,
/*17*//* IFSWL */		9,
/*18*//* IFUW */		8,
/*19*//* IFUWL */		9,
/*1a*//*  */			1,
/*1b*//*  */			1,
/*1c*//*  */			1,
/*1d*//*  */			1,
/*1e*//*  */			1,
/*1f*//*  */			1,
	
/*20*//* MINIGAME */	11,
/*21*//* TUTOR */		2,
/*22*//* BTMD2 */		5,
/*23*//* BTRLD */		3,
/*24*//* WAIT */		3,
/*25*//* NFADE */		9,
/*26*//* BLINK */		2,
/*27*//* BGMOVIE */		2,
/*28*//* KAWAI */		0,
/*29*//* KAWIW */		1,
/*2a*//* PMOVA */		2,
/*2b*//* SLIP */		2,
/*2c*//* BGPDH */		5,
/*2d*//* BGSCR */		7,
/*2e*//* WCLS */		2,
/*2f*//* WSIZW */		10,

/*30*//* IFKEY */		4,
/*31*//* IFKEYON */		4,
/*32*//* IFKEYOFF */	4,
/*33*//* UC */			2,
/*34*//* PDIRA */		2,
/*35*//* PTURA */		4,
/*36*//* WSPCL */		5,
/*37*//* WNUMB */		8,
/*38*//* STTIM */		6,
/*39*//* GOLDu */		6,
/*3a*//* GOLDd */		6,
/*3b*//* CHGLD */		4,
/*3c*//* HMPMAX1 */		1,
/*3d*//* HMPMAX2 */		1,
/*3e*//* MHMMX */		1,
/*3f*//* HMPMAX3 */		1,

/*40*//* MESSAGE */		3,
/*41*//* MPARA */		5,
/*42*//* MPRA2 */		6,
/*43*//* MPNAM */		2,
/*44*//*  */			1,
/*45*//* MPu */			5,
/*46*//*  */			1,
/*47*//* MPd */			5,
/*48*//* ASK */			7,
/*49*//* MENU */		4,
/*4a*//* MENU2 */		2,
/*4b*//* BTLTB */		2,
/*4c*//*  */			1,
/*4d*//* HPu */			5,
/*4e*//*  */			1,
/*4f*//* HPd */			5,

/*50*//* WINDOW */		10,
/*51*//* WMOVE */		6,
/*52*//* WMODE */		4,
/*53*//* WREST */		2,
/*54*//* WCLSE */		2,
/*55*//* WROW */		3,
/*56*//* GWCOL */		7,
/*57*//* SWCOL */		7,
/*58*//* STITM */		5,
/*59*//* DLITM */		5,
/*5a*//* CKITM */		5,
/*5b*//* SMTRA */		7,
/*5c*//* DMTRA */		8,
/*5d*//* CMTRA */		10,
/*5e*//* SHAKE */		8,
/*5f*//* NOP */			1,

/*60*//* MAPJUMP */	10,
/*61*//* SCRLO */	2,
/*62*//* SCRLC */	5,
/*63*//* SCRLA */	6,
/*64*//* SCR2D */	6,
/*65*//* SCRCC */	1,
/*66*//* SCR2DC */	9,
/*67*//* SCRLW */	1,
/*68*//* SCR2DL */	9,
/*69*//* MPDSP */	2,
/*6a*//* VWOFT */	7,
/*6b*//* FADE */		9,
/*6c*//* FADEW */	1,
/*6d*//* IDLCK */	4,
/*6e*//* LSTMP */	3,
/*6f*//* SCRLP */		6,

/*70*//* BATTLE */	4,
/*71*//* BTLON */	2,
/*72*//* BTLMD */	3,
/*73*//* PGTDR */	4,
/*74*//* GETPC */	4,
/*75*//* PXYZI */	8,
/*76*//* PLUS! */		4,
/*77*//* PLUS2! */	5,
/*78*//* MINUS! */	4,
/*79*//* MINUS2! */	5,
/*7a*//* INC! */		3,
/*7b*//* INC2! */		3,
/*7c*//* DEC! */		3,
/*7d*//* DEC2! */		3,
/*7e*//* TLKON */	2,
/*7f*//* RDMSD */	3,

/*80*//* SETBYTE */	4,
/*81*//* SETWORD */	5,
/*82*//* BITON */	4,
/*83*//* BITOFF */	4,
/*84*//* BITXOR */	4,
/*85*//* PLUS */		4,
/*86*//* PLUS2 */	5,
/*87*//* MINUS */	4,
/*88*//* MINUS2 */	5,
/*89*//* MUL */		4,
/*8a*//* MUL2 */		5,
/*8b*//* DIV */		4,
/*8c*//* DIV2 */		5,
/*8d*//* MOD */		4,
/*8e*//* MOD2 */		5,
/*8f*//* AND */		4,

/*90*//* AND2 */		5,
/*91*//* OR */		4,
/*92*//* OR2 */		5,
/*93*//* XOR */		4,
/*94*//* XOR2 */		5,
/*95*//* INC */		3,
/*96*//* INC2 */		3,
/*97*//* DEC */		3,
/*98*//* DEC2 */		3,
/*99*//* RANDOM */	3,
/*9a*//* LBYTE */	4,
/*9b*//* HBYTE */	5,
/*9c*//* 2BYTE */	6,
/*9d*//* SETX */		7,
/*9e*//* GETX */		7,
/*9f*//* SEARCHX */	11,

/*a0*//* PC */		2,
/*a1*//* CHAR */		2,
/*a2*//* DFANM */	3,
/*a3*//* ANIME1 */	3,
/*a4*//* VISI */		2,
/*a5*//* XYZI */		11,
/*a6*//* XYI */		9,
/*a7*//* XYZ */		9,
/*a8*//* MOVE */		6,
/*a9*//* CMOVE */	6,
/*aa*//* MOVA */		2,
/*ab*//* TURA */		4,
/*ac*//* ANIMW */	1,
/*ad*//* FMOVE */	6,
/*ae*//* ANIME2 */	3,
/*af*//* ANIM!1 */	3,

/*b0*//* CANIM1 */	5,
/*b1*//* CANM!1 */	5,
/*b2*//* MSPED */	4,
/*b3*//* DIR */		3,
/*b4*//* TURNGEN */	6,
/*b5*//* TURN */		6,
/*b6*//* DIRA */		2,
/*b7*//* GETDIR */	4,
/*b8*//* GETAXY */	5,
/*b9*//* GETAI */	4,
/*ba*//* ANIM!2 */	3,
/*bb*//* CANIM2 */	5,
/*bc*//* CANM!2 */	5,
/*bd*//* ASPED */	4,
/*be*//*  */			1,
/*bf*//* CC */		2,

/*c0*//* JUMP */		11,
/*c1*//* AXYZI */	8,
/*c2*//* LADER */	15,
/*c3*//* OFST */		12,
/*c4*//* OFSTW */	1,
/*c5*//* TALKR */	3,
/*c6*//* SLIDR */	3,
/*c7*//* SOLID */	2,
/*c8*//* PRTYP */		2,
/*c9*//* PRTYM */	2,
/*ca*//* PRTYE */		4,
/*cb*//* IFPRTYQ */	3,
/*cc*//* IFMEMBQ */	3,
/*cd*//* MMBud */	3,
/*ce*//* MMBLK */	2,
/*cf*//* MMBUK */	2,

/*d0*//* LINE */		13,
/*d1*//* LINON */	2,
/*d2*//* MPJPO */	2,
/*d3*//* SLINE */	16,
/*d4*//* SIN */		10,
/*d5*//* COS */		10,
/*d6*//* TLKR2 */		4,
/*d7*//* SLDR2 */	4,
/*d8*//* PMJMP */	3,
/*d9*//* PMJMP2 */	1,
/*da*//* AKAO2 */	15,
/*db*//* FCFIX */		2,
/*dc*//* CCANM */	4,
/*dd*//* ANIMB */	1,
/*de*//* TURNW */	1,
/*df*//* MPPAL */		11,

/*e0*//* BGON */		4,
/*e1*//* BGOFF */	4,
/*e2*//* BGROL */	3,
/*e3*//* BGROL2 */	3,
/*e4*//* BGCLR */	3,
/*e5*//* STPAL */	5,
/*e6*//* LDPAL */	5,
/*e7*//* CPPAL */		5,
/*e8*//* RTPAL */		7,
/*e9*//* ADPAL */	10,
/*ea*//* MPPAL2 */	10,
/*eb*//* STPLS */		5,
/*ec*//* LDPLS */		5,
/*ed*//* CPPAL2 */	8,
/*ee*//* RTPAL2 */	8,
/*ef*//* ADPAL2 */	11,

/*f0*//* MUSIC */	2,
/*f1*//* SOUND */	5,
/*f2*//* AKAO */		14,
/*f3*//* MUSVT */	2,
/*f4*//* MUSVM */	2,
/*f5*//* MULCK */	2,
/*f6*//* BMUSC */	2,
/*f7*//* CHMPH */	4,
/*f8*//* PMVIE */	2,
/*f9*//* MOVIE */	1,
/*fa*//* MVIEF */	3,
/*fb*//* MVCAM */	2,
/*fc*//* FMUSC */	2,
/*fd*//* CMUSC */	6,
/*fe*//* CHMST */	3,
/*ff*//* GAMEOVER */	1,
/*100*//* LABEL */	0
};

const QString Opcode::names[257] =
{
/*00*/	"RET",
/*01*/	"REQ",
/*02*/	"REQSW",
/*03*/	"REQEW",
/*04*/	"PREQ",
/*05*/	"PRQSW",
/*06*/	"PRQEW",
/*07*/	"RETTO",
/*08*/	"JOIN",
/*09*/	"SPLIT",
/*0a*/	"SPTYE",
/*0b*/	"GTPYE",
/*0c*/	"Unknown1",
/*0d*/	"Unknown2",
/*0e*/	"DSKCG",
/*0f*/	"SPECIAL",
	
/*10*/	"JMPF",
/*11*/	"JMPFL",
/*12*/	"JMPB",
/*13*/	"JMPBL",
/*14*/	"IFUB",
/*15*/	"IFUBL",
/*16*/	"IFSW",
/*17*/	"IFSWL",
/*18*/	"IFUW",
/*19*/	"IFUWL",
/*1a*/	"Unknown3",
/*1b*/	"Unknown4",
/*1c*/	"Unknown5",
/*1d*/	"Unknown6",
/*1e*/	"Unknown7",
/*1f*/	"Unknown8",
	
/*20*/	"MINIGAME",
/*21*/	"TUTOR",
/*22*/	"BTMD2",
/*23*/	"BTRLD",
/*24*/	"WAIT",
/*25*/	"NFADE",
/*26*/	"BLINK",
/*27*/	"BGMOVIE",
/*28*/	"KAWAI",
/*29*/	"KAWIW",
/*2a*/	"PMOVA",
/*2b*/	"SLIP",
/*2c*/	"BGPDH",
/*2d*/	"BGSCR",
/*2e*/	"WCLS",
/*2f*/	"WSIZW",

/*30*/	"IFKEY",
/*31*/	"IFKEYON",
/*32*/	"IFKEYOFF",
/*33*/	"UC",
/*34*/	"PDIRA",
/*35*/	"PTURA",
/*36*/	"WSPCL",
/*37*/	"WNUMB",
/*38*/	"STTIM",
/*39*/	"GOLDu",
/*3a*/	"GOLDd",
/*3b*/	"CHGLD",
/*3c*/	"HMPMAX1",
/*3d*/	"HMPMAX2",
/*3e*/	"MHMMX",
/*3f*/	"HMPMAX3",

/*40*/	"MESSAGE",
/*41*/	"MPARA",
/*42*/	"MPRA2",
/*43*/	"MPNAM",
/*44*/	"Unknown9",
/*45*/	"MPu",
/*46*/	"Unknown10",
/*47*/	"MPd",
/*48*/	"ASK",
/*49*/	"MENU",
/*4a*/	"MENU2",
/*4b*/	"BTLTB",
/*4c*/	"Unknown11",
/*4d*/	"HPu",
/*4e*/	"Unknown12",
/*4f*/	"HPd",

/*50*/	"WINDOW",
/*51*/	"WMOVE",
/*52*/	"WMODE",
/*53*/	"WREST",
/*54*/	"WCLSE",
/*55*/	"WROW",
/*56*/	"GWCOL",
/*57*/	"SWCOL",
/*58*/	"STITM",
/*59*/	"DLITM",
/*5a*/	"CKITM",
/*5b*/	"SMTRA",
/*5c*/	"DMTRA",
/*5d*/	"CMTRA",
/*5e*/	"SHAKE",
/*5f*/	"NOP",

/*60*/	"MAPJUMP",
/*61*/	"SCRLO",
/*62*/	"SCRLC",
/*63*/	"SCRLA",
/*64*/	"SCR2D",
/*65*/	"SCRCC",
/*66*/	"SCR2DC",
/*67*/	"SCRLW",
/*68*/	"SCR2DL",
/*69*/	"MPDSP",
/*6a*/	"VWOFT",
/*6b*/	"FADE",
/*6c*/	"FADEW",
/*6d*/	"IDLCK",
/*6e*/	"LSTMP",
/*6f*/	"SCRLP",

/*70*/	"BATTLE",
/*71*/	"BTLON",
/*72*/	"BTLMD",
/*73*/	"PGTDR",
/*74*/	"GETPC",
/*75*/	"PXYZI",
/*76*/	"PLUS!",
/*77*/	"PLUS2!",
/*78*/	"MINUS!",
/*79*/	"MINUS2!",
/*7a*/	"INC!",
/*7b*/	"INC2!",
/*7c*/	"DEC!",
/*7d*/	"DEC2!",
/*7e*/	"TLKON",
/*7f*/	"RDMSD",

/*80*/	"SETBYTE",
/*81*/	"SETWORD",
/*82*/	"BITON",
/*83*/	"BITOFF",
/*84*/	"BITXOR",
/*85*/	"PLUS",
/*86*/	"PLUS2",
/*87*/	"MINUS",
/*88*/	"MINUS2",
/*89*/	"MUL",
/*8a*/	"MUL2",
/*8b*/	"DIV",
/*8c*/	"DIV2",
/*8d*/	"MOD",
/*8e*/	"MOD2",
/*8f*/	"AND",

/*90*/	"AND2",
/*91*/	"OR",
/*92*/	"OR2",
/*93*/	"XOR",
/*94*/	"XOR2",
/*95*/	"INC",
/*96*/	"INC2",
/*97*/	"DEC",
/*98*/	"DEC2",
/*99*/	"RANDOM",
/*9a*/	"LBYTE",
/*9b*/	"HBYTE",
/*9c*/	"2BYTE",
/*9d*/	"SETX",
/*9e*/	"GETX",
/*9f*/	"SEARCHX",

/*a0*/	"PC",
/*a1*/	"CHAR",
/*a2*/	"DFANM",
/*a3*/	"ANIME1",
/*a4*/	"VISI",
/*a5*/	"XYZI",
/*a6*/	"XYI",
/*a7*/	"XYZ",
/*a8*/	"MOVE",
/*a9*/	"CMOVE",
/*aa*/	"MOVA",
/*ab*/	"TURA",
/*ac*/	"ANIMW",
/*ad*/	"FMOVE",
/*ae*/	"ANIME2",
/*af*/	"ANIM!1",

/*b0*/	"CANIM1",
/*b1*/	"CANM!1",
/*b2*/	"MSPED",
/*b3*/	"DIR",
/*b4*/	"TURNGEN",
/*b5*/	"TURN",
/*b6*/	"DIRA",
/*b7*/	"GETDIR",
/*b8*/	"GETAXY",
/*b9*/	"GETAI",
/*ba*/	"ANIM!2",
/*bb*/	"CANIM2",
/*bc*/	"CANM!2",
/*bd*/	"ASPED",
/*be*/	"Unknown13",
/*bf*/	"CC",

/*c0*/	"JUMP",
/*c1*/	"AXYZI",
/*c2*/	"LADER",
/*c3*/	"OFST",
/*c4*/	"OFSTW",
/*c5*/	"TALKR",
/*c6*/	"SLIDR",
/*c7*/	"SOLID",
/*c8*/	"PRTYP",
/*c9*/	"PRTYM",
/*ca*/	"PRTYE",
/*cb*/	"IFPRTYQ",
/*cc*/	"IFMEMBQ",
/*cd*/	"MMBud",
/*ce*/	"MMBLK",
/*cf*/	"MMBUK",

/*d0*/	"LINE",
/*d1*/	"LINON",
/*d2*/	"MPJPO",
/*d3*/	"SLINE",
/*d4*/	"SIN",
/*d5*/	"COS",
/*d6*/	"TLKR2",
/*d7*/	"SLDR2",
/*d8*/	"PMJMP",
/*d9*/	"PMJMP2",
/*da*/	"AKAO2",
/*db*/	"FCFIX",
/*dc*/	"CCANM",
/*dd*/	"ANIMB",
/*de*/	"TURNW",
/*df*/	"MPPAL",

/*e0*/	"BGON",
/*e1*/	"BGOFF",
/*e2*/	"BGROL",
/*e3*/	"BGROL2",
/*e4*/	"BGCLR",
/*e5*/	"STPAL",
/*e6*/	"LDPAL",
/*e7*/	"CPPAL",
/*e8*/	"RTPAL",
/*e9*/	"ADPAL",
/*ea*/	"MPPAL2",
/*eb*/	"STPLS",
/*ec*/	"LDPLS",
/*ed*/	"CPPAL2",
/*ee*/	"RTPAL2",
/*ef*/	"ADPAL2",

/*f0*/	"MUSIC",
/*f1*/	"SOUND",
/*f2*/	"AKAO",
/*f3*/	"MUSVT",
/*f4*/	"MUSVM",
/*f5*/	"MULCK",
/*f6*/	"BMUSC",
/*f7*/	"CHMPH",
/*f8*/	"PMVIE",
/*f9*/	"MOVIE",
/*fa*/	"MVIEF",
/*fb*/	"MVCAM",
/*fc*/	"FMUSC",
/*fd*/	"CMUSC",
/*fe*/	"CHMST",
/*ff*/	"GAMEOVER",
/*100*/	"LABEL"
};
