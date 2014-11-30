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
#include "core/FF7Text.h"
#include "core/Var.h"
#include "Data.h"
#include "core/Config.h"
#include "Field.h"

const char *Opcode::operators[OPERATORS_SIZE] = {
	"==", "!=", ">", "<", ">=", "<=", "&", "^", "|", "bitON", "bitOFF"
};

Opcode::Opcode()
{
}

Opcode::~Opcode()
{
}

QByteArray Opcode::toByteArray() const
{
	return QByteArray()
			.append((char)id())
			.append(params());
}

int Opcode::subParam(int cur, int paramSize) const
{
	QByteArray p = params();
	int value, sizeBA;

	if(paramSize%8 !=0)
		sizeBA = paramSize/8+1;
	else
		sizeBA = paramSize/8;

	memcpy(&value, p.constData() + cur/8, sizeBA);
	return (value >> ((sizeBA*8-cur%8)-paramSize)) & ((int)pow(2, paramSize)-1);
}

bool Opcode::searchVar(quint8 bank, quint8 adress, Operation op, int value) const
{
	// TODO: compare var with var
	const bool noValue = value > 0xFFFF;
	QList<FF7Var> vars;

	getVariables(vars);

	switch (op) {
	case Assign:
		// Compare value if provided
		if(!noValue) {
			if(id() == SETBYTE) {
				OpcodeSETBYTE *setbyte = (OpcodeSETBYTE *)this;
				if(B1(setbyte->banks) == bank && setbyte->var == adress
						&& B2(setbyte->banks) == 0 && setbyte->value == value)
					return true;
			}
			if(id() == SETWORD) {
				OpcodeSETWORD *setword = (OpcodeSETWORD *)this;
				if(B1(setword->banks) == bank && setword->var == adress
						&& B2(setword->banks) == 0 && setword->value == (quint16)value)
					return true;
			}
		} else {
			// Every write vars
			foreach(const FF7Var &var, vars) {
				if(var.bank == bank && var.adress == adress
						&& var.write == true && var.size != FF7Var::Bit) {
					return true;
				}
			}
		}
		return false;
	case BitAssign:
		if(id() == BITON
				|| id() == BITOFF
				|| id() == BITXOR) {
			OpcodeBitOperation *bitOperation = (OpcodeBitOperation *)this;
			if(B1(bitOperation->banks) == bank && bitOperation->var == adress
					&& (noValue || (B2(bitOperation->banks) == 0 && bitOperation->position == value))) {
				return true;
			}
		}
		return false;
	case Compare:
	case BitCompare:
		if(id() == IFUB
				|| id() == IFUBL
				|| id() == IFSW
				|| id() == IFSWL
				|| id() == IFUW
				|| id() == IFUWL) {
			OpcodeIf *opcodeIf = (OpcodeIf *)this;
			if(((op == Compare && opcodeIf->oper < quint8(BitAnd))
				|| (op == BitCompare && opcodeIf->oper >= quint8(BitAnd)))
					&& B1(opcodeIf->banks) == bank && opcodeIf->value1 == adress
					&& (noValue || (B2(opcodeIf->banks) == 0 && opcodeIf->value2 == value))) {
				return true;
			}
		}
		return false;
	default:
		foreach(const FF7Var &var, vars) {
			if(var.bank == bank && var.adress == adress)
				return true;
		}
		return false;
	}
}

bool Opcode::searchExec(quint8 group, quint8 script) const
{
	if(id() == REQ || id() == REQSW || id() == REQEW) {
		OpcodeExec *exec = (OpcodeExec *)this;
		return exec->groupID == group && exec->scriptID == script;
	}
	return false;
}

bool Opcode::searchMapJump(quint16 fieldID) const
{
	if(id() == MAPJUMP) {
		return ((OpcodeMAPJUMP *)this)->fieldID == fieldID;
	}
	if(id() == MINIGAME) {
		return ((OpcodeMINIGAME *)this)->fieldID == fieldID;
	}
	return false;
}

bool Opcode::searchTextInScripts(const QRegExp &text, const Section1File *scriptsAndTexts) const
{
	qint16 textID = getTextID();
	return textID != -1
			&& textID < scriptsAndTexts->textCount()
			&& scriptsAndTexts->text(textID).contains(text);
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

void Opcode::shiftGroupIds(int groupId, int steps)
{
	int groupID = getGroupID();
	if(groupID != -1 && groupID > groupId) {
		setGroupID(groupID + steps);
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

void Opcode::swapGroupIds(int groupId1, int groupId2)
{
	int groupID = getGroupID();
	if(groupID == groupId1) {
		setGroupID(groupId2);
	} else if(groupID == groupId2) {
		setGroupID(groupId1);
	}
}

void Opcode::listWindows(int groupID, int scriptID, int opcodeID, QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const
{
	int windowID = getWindowID();
	if(windowID != -1) {
		FF7Window win = FF7Window();
		win.groupID = groupID;
		win.scriptID = scriptID;
		win.opcodeID = opcodeID;
		if(getWindow(win)) {
			win.type = id();
			windows.insert((groupID << 16) | (scriptID << 8) | windowID, win);
		}
		int textID = getTextID();
		if(textID != -1) {
			text2win.insert(textID, (groupID << 16) | (scriptID << 8) | windowID);
		}
	}
}

void Opcode::listModelPositions(QList<FF7Position> &positions) const
{
	Q_UNUSED(positions);
}

bool Opcode::linePosition(FF7Position position[2]) const
{
	Q_UNUSED(position);
	return false;
}

void Opcode::backgroundParams(QHash<quint8, quint8> &enabledParams) const
{
	quint8 param, state;

	if(id() == BGON) { //show bg parameter
		OpcodeBGON *bgon = (OpcodeBGON *)this;
		if(bgon->banks == 0) {
			param = bgon->paramID;
			state = 1 << bgon->stateID;
			if(enabledParams.contains(param))
				state |= enabledParams.value(param);
			enabledParams.insert(param, state);
		}
	}/*else if(id() == BGOFF) { //hide bg parameter
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

void Opcode::backgroundMove(qint16 z[2], qint16 *x, qint16 *y) const
{
	if(id() == BGPDH) { //Move Background Z
		OpcodeBGPDH *bgpdh = (OpcodeBGPDH *)this;
		if(bgpdh->banks==0 && bgpdh->layerID>1 && bgpdh->layerID<4) { //No var
			z[bgpdh->layerID-2] = bgpdh->targetZ;
		}
	} else if(x && y && id() == BGSCR) { // Animate Background X Y
		OpcodeBGSCR *bgscr = (OpcodeBGSCR *)this;
		if(bgscr->banks==0 && bgscr->layerID>1 && bgscr->layerID<4) { //No var
			x[bgscr->layerID-2] = bgscr->targetX;
			y[bgscr->layerID-2] = bgscr->targetY;
		}
	}
}

QString Opcode::_script(quint8 param, Section1File *scriptsAndTexts)
{
	if(param < scriptsAndTexts->grpScriptCount())
		return scriptsAndTexts->grpScript(param)->name() + QObject::tr(" (n°%1)").arg(param);
	return QObject::tr("? (n°%1)").arg(param);
}

QString Opcode::_text(quint8 textID, Section1File *scriptsAndTexts)
{
	if(textID < scriptsAndTexts->textCount()) {
		QString t = scriptsAndTexts->text(textID).text(Config::value("jp_txt", false).toBool(), true).simplified();
		if(t.size() > 70)
			t = t.left(35) % QString("...") % t.right(35);
		return "\"" + t + "\"";
	}
	return QObject::tr("(Pas de texte)");
}

QString Opcode::_item(quint16 itemID, quint8 bank)
{
	if(bank > 0) {
		return QObject::tr("n°%1").arg(_bank(itemID & 0xFF, bank));
	}

	if(itemID < 128) {
		if(!Data::item_names.isEmpty() && itemID < Data::item_names.size())
			return Data::item_names.at(itemID);
	} else if(itemID < 256) {
		if(!Data::weapon_names.isEmpty() && itemID-128 < Data::weapon_names.size())
			return Data::weapon_names.at(itemID-128);
	} else if(itemID < 288) {
		if(!Data::armor_names.isEmpty() && itemID-256 < Data::armor_names.size())
			return Data::armor_names.at(itemID-256);
	} else if(itemID < 320) {
		if(!Data::accessory_names.isEmpty() && itemID-288 < Data::accessory_names.size())
			return Data::accessory_names.at(itemID-288);
	}
	return QObject::tr("n°%1").arg(itemID);
}

QString Opcode::_materia(quint8 materiaID, quint8 bank)
{
	if(bank > 0) {
		return QObject::tr("n°%1").arg(_bank(materiaID, bank));
	}

	if(materiaID < Data::materia_names.size())
		return Data::materia_names.at(materiaID);
	return QObject::tr("n°%1").arg(materiaID);
}

QString Opcode::_field(quint16 fieldID)
{
	if(fieldID < Data::field_names.size())
		return QObject::tr("%1 (n°%2)")
				.arg(Data::field_names.at(fieldID))
				.arg(fieldID);
	return QObject::tr("n°%1").arg(fieldID);
}

QString Opcode::_movie(quint8 movieID)
{
	// Movie names are different according to the current disc
	QString dflt = QObject::tr("n°%1").arg(movieID);
	QStringList cds;
	cds << Data::movie_names_cd1.value(movieID, dflt)
		<< Data::movie_names_cd2.value(movieID, dflt)
		<< Data::movie_names_cd3.value(movieID, dflt);

	QStringList out;
	for(int discID = 0 ; discID < 3 ; ++discID) {
		out.append(QObject::tr("%1 (disque %2)").arg(cds.at(discID)).arg(discID + 1));
	}

	return out.join(", ");
}

/* QString Opcode::_objet3D(quint8 objet3D_ID)
{
	if(objet3D_ID < Data::currentCharNames.size())
		return QString("%1 (%2)").arg(Data::currentCharNames.at(objet3D_ID), Data::currentHrcNames.at(objet3D_ID));
	return QObject::tr("n°%1").arg(objet3D_ID);
} */

QString Opcode::_akao(quint8 akaoOp)
{
	switch(akaoOp) {
	case 0x28:	return QObject::tr("Jouer un effet sonore sur le canal 1");
	case 0x29:	return QObject::tr("Jouer un effet sonore sur le canal 2");
	case 0x2A:	return QObject::tr("Jouer un effet sonore sur le canal 3");
	case 0x2B:	return QObject::tr("Jouer un effet sonore sur le canal 4");
	case 0x98:	return QObject::tr("Reprendre la musique et les effets sonores");
	case 0x99:	return QObject::tr("Mettre en pause la musique et les effets sonores");
	case 0x9A:	return QObject::tr("Reprendre uniquement la musique");
	case 0x9B:	return QObject::tr("Mettre en pause uniquement la musique");
	case 0x9C:	return QObject::tr("Reprendre uniquement les effets sonores");
	case 0x9D:	return QObject::tr("Mettre en pause uniquement les effets sonores");
	case 0xA0:	return QObject::tr("Contrôle du volume (canal 1)");
	case 0xA1:	return QObject::tr("Contrôle du volume (canal 2)");
	case 0xA2:	return QObject::tr("Contrôle du volume (canal 3)");
	case 0xA3:	return QObject::tr("Contrôle du volume (canal 4)");
	case 0xA4:	return QObject::tr("Volume transitions (canal 1)");
	case 0xA5:	return QObject::tr("Volume transitions (canal 2)");
	case 0xA6:	return QObject::tr("Volume transitions (canal 3)");
	case 0xA7:	return QObject::tr("Volume transitions (canal 4)");
	case 0xA8:	return QObject::tr("Contrôle spatial (canal 1)");
	case 0xA9:	return QObject::tr("Contrôle spatial (canal 2)");
	case 0xAA:	return QObject::tr("Contrôle spatial (canal 3)");
	case 0xAB:	return QObject::tr("Contrôle spatial (canal 4)");
	case 0xAC:	return QObject::tr("Transitions spatiales (canal 1)");
	case 0xAD:	return QObject::tr("Transitions spatiales (canal 2)");
	case 0xAE:	return QObject::tr("Transitions spatiales (canal 3)");
	case 0xAF:	return QObject::tr("Transitions spatiales (canal 4)");
	case 0xB0:	return QObject::tr("Contrôle du tempo (canal 1)");
	case 0xB1:	return QObject::tr("Contrôle du tempo (canal 2)");
	case 0xB2:	return QObject::tr("Contrôle du tempo (canal 3)");
	case 0xB3:	return QObject::tr("Contrôle du tempo (canal 4)");
	case 0xB4:	return QObject::tr("Tempo transitions (canal 1)");
	case 0xB5:	return QObject::tr("Tempo transitions (canal 2)");
	case 0xB6:	return QObject::tr("Tempo transitions (canal 3)");
	case 0xB7:	return QObject::tr("Tempo transitions (canal 4)");
	case 0xB8:	return QObject::tr("Contrôle du volume sur tous les canaux");
	case 0xB9:	return QObject::tr("Volume transitions sur tous les canaux");
	case 0xBA:	return QObject::tr("Contrôle spatial sur tous les canaux");
	case 0xBB:	return QObject::tr("Transitions spatiales sur tous les canaux");
	case 0xBC:	return QObject::tr("Contrôle du tempo sur tous les canaux");
	case 0xBD:	return QObject::tr("Tempo transitions sur tous les canaux");
	case 0xC0:	return QObject::tr("Modifier le volume de la musique");
	case 0xC1:	return QObject::tr("Transition sur le volume de la musique");
	case 0xC2:	return QObject::tr("Attenuation du volume de la musique");
	case 0xC8:	return QObject::tr("Modifier la spatialisation de la musique");
	case 0xC9:	return QObject::tr("Transition sur la spatialisation de la musique");
	case 0xCA:	return QObject::tr("Attenuation de la spatialisation de la musique");
	case 0xD0:	return QObject::tr("Modifier le tempo de la musique");
	case 0xD1:	return QObject::tr("Transition sur le tempo de la musique");
	case 0xD2:	return QObject::tr("Attenuation du tempo de la musique");
	case 0xF0:	return QObject::tr("Arrêter la musique");
	case 0xF1:	return QObject::tr("Arrêter les effets sonores");
	default:	return QObject::tr("AKAO : %1?").arg(akaoOp);
	}
}

QString Opcode::_bank(quint8 adress, quint8 bank)
{
	if(!Var::name(bank, adress).isEmpty()) {
		return Var::name(bank, adress);
	}
	if(bank == 0) {
		return QString("?");
	}
	return QString("Var[%1][%2]").arg(bank).arg(adress);
}

QString Opcode::_var(int value, quint8 bank)
{
	if(bank > 0)
		return _bank(value & 0xFF, bank);
	return QString::number(value);
}

QString Opcode::_var(int value, quint8 bank1, quint8 bank2)
{
	if(bank1 > 0 || bank2 > 0) {
		QString ret = _var(value & 0xFFFF, bank1);
		if(bank2 != 0 || ((value >> 16) & 0xFFFF) != 0) {
			ret += QString(" + %1 * 65536").arg(_var((value >> 16) & 0xFFFF, bank2));
		}
		return ret;
	}
	return QString::number(value);
}

QString Opcode::_var(int value, quint8 bank1, quint8 bank2, quint8 bank3)
{
	if(bank1 > 0 || bank2 > 0 || bank3 > 0)
		return QObject::tr("%1 et %2 et %3")
				.arg(_bank(value & 0xFF, bank1))
				.arg(_bank((value >> 8) & 0xFF, bank2))
				.arg(_bank((value >> 16) & 0xFF, bank3));
	return QString::number(value);
}

QString Opcode::character(quint8 persoID)
{
	if(persoID < Data::char_names.size())	return Data::char_names.at(persoID);
	if(persoID >= 254)						return QObject::tr("(Vide)");
	if(persoID >= 100)						return Data::char_names.last();
	return QString("%1?").arg(persoID);
}

QString Opcode::_windowCorner(quint8 param, quint8 bank)
{
	if(bank > 0) {
		return _bank(param, bank);
	}

	switch(param) {
	case 0:		return QObject::tr("haut gauche");
	case 1:		return QObject::tr("bas gauche");
	case 2:		return QObject::tr("haut droit");
	case 3:		return QObject::tr("bas droit");
	default:	return QString("%1?").arg(param);
	}
}

QString Opcode::_sensRotation(quint8 param)
{
	switch(param) {
	case 1:		return QObject::tr("inverse");
	case 2:		return QObject::tr("inverse");
	default:	return QObject::tr("normal");
	}
}

QString Opcode::_operateur(quint8 param)
{
	if(param < OPERATORS_SIZE)
		return operators[param];
	return QString("%1?").arg(param);
}

OpcodeUnknown::OpcodeUnknown(quint8 id, const QByteArray &params)
{
	_id = id;
	unknown = params;
}

OpcodeUnknown::OpcodeUnknown(quint8 id, const char *params, int size)
{
	_id = id;
	setParams(params, size);
}

int OpcodeUnknown::id() const
{
	return _id;
}

quint8 OpcodeUnknown::size() const
{
	return unknown.size() + 1;
}

QString OpcodeUnknown::toString(Field *) const
{
	return QObject::tr("? (id=%1)")
			.arg(_id);
}

void OpcodeUnknown::setParams(const char *params, int size)
{
	unknown = QByteArray(params, size);
}

QByteArray OpcodeUnknown::params() const
{
	return unknown;
}

OpcodeRET::OpcodeRET()
{
}

QString OpcodeRET::toString(Field *) const
{
	return QObject::tr("Retourner");
}

OpcodeExec::OpcodeExec(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeExec::setParams(const char *params, int)
{
	groupID = params[0];
	scriptID = params[1] & 0x1F;
	priority = (params[1] >> 5) & 7;
}

QByteArray OpcodeExec::params() const
{
	return QByteArray()
			.append((char)groupID)
			.append(char((scriptID & 0x1F) | ((priority & 7) << 5)));
}

OpcodeREQ::OpcodeREQ(const char *params, int size) :
	OpcodeExec(params, size)
{
}

OpcodeREQ::OpcodeREQ(const OpcodeExec &op) :
	OpcodeExec(op)
{
}

QString OpcodeREQ::toString(Field *field) const
{
	return QObject::tr("Exécuter le script n°%3 du groupe externe %1 (priorité %2/6) - Seulement si le script n'est pas déjà en cours d'exécution")
			.arg(_script(groupID, field->scriptsAndTexts()))
			.arg(priority)
			.arg(scriptID);
}

OpcodeREQSW::OpcodeREQSW(const char *params, int size) :
	OpcodeExec(params, size)
{
}

OpcodeREQSW::OpcodeREQSW(const OpcodeExec &op) :
	OpcodeExec(op)
{
}

QString OpcodeREQSW::toString(Field *field) const
{
	return QObject::tr("Exécuter le script n°%3 du groupe externe %1 (priorité %2/6)")
			.arg(_script(groupID, field->scriptsAndTexts()))
			.arg(priority)
			.arg(scriptID);
}

OpcodeREQEW::OpcodeREQEW(const char *params, int size) :
	OpcodeExec(params, size)
{
}

OpcodeREQEW::OpcodeREQEW(const OpcodeExec &op) :
	OpcodeExec(op)
{
}

QString OpcodeREQEW::toString(Field *field) const
{
	return QObject::tr("Exécuter le script n°%3 du groupe %1 (priorité %2/6) - Attend la fin de l'exécution pour continuer")
			.arg(_script(groupID, field->scriptsAndTexts()))
			.arg(priority)
			.arg(scriptID);
}

OpcodeExecChar::OpcodeExecChar(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeExecChar::setParams(const char *params, int)
{
	partyID = params[0];
	scriptID = params[1] & 0x1F;
	priority = (params[1] >> 5) & 7;
}

QByteArray OpcodeExecChar::params() const
{
	return QByteArray()
			.append((char)partyID)
			.append(char((scriptID & 0x1F) | ((priority & 7) << 5)));
}

OpcodePREQ::OpcodePREQ(const char *params, int size) :
	OpcodeExecChar(params, size)
{
}

OpcodePREQ::OpcodePREQ(const OpcodeExecChar &op) :
	OpcodeExecChar(op)
{
}

QString OpcodePREQ::toString(Field *) const
{
	return QObject::tr("Exécuter le script n°%3 du groupe externe lié au personnage n°%1 de l'équipe (priorité %2/6) - Seulement si le script n'est pas déjà en cours d'exécution")
			.arg(partyID)
			.arg(priority)
			.arg(scriptID);
}

OpcodePRQSW::OpcodePRQSW(const char *params, int size) :
	OpcodeExecChar(params, size)
{
}

OpcodePRQSW::OpcodePRQSW(const OpcodeExecChar &op) :
	OpcodeExecChar(op)
{
}

QString OpcodePRQSW::toString(Field *) const
{
	return QObject::tr("Exécuter le script n°%3 du groupe externe lié au personnage n°%1 de l'équipe (priorité %2/6)")
			.arg(partyID)
			.arg(priority)
			.arg(scriptID);
}

OpcodePRQEW::OpcodePRQEW(const char *params, int size) :
	OpcodeExecChar(params, size)
{
}

OpcodePRQEW::OpcodePRQEW(const OpcodeExecChar &op) :
	OpcodeExecChar(op)
{
}

QString OpcodePRQEW::toString(Field *) const
{
	return QObject::tr("Exécuter le script n°%3 du groupe lié au personnage n°%1 de l'équipe (priorité %2/6) - Attend la fin de l'exécution pour continuer")
			.arg(partyID)
			.arg(priority)
			.arg(scriptID);
}

OpcodeRETTO::OpcodeRETTO(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeRETTO::setParams(const char *params, int)
{
	scriptID = params[0] & 0x1F;
	priority = (params[0] >> 5) & 7;
}

QString OpcodeRETTO::toString(Field *) const
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

OpcodeJOIN::OpcodeJOIN(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeJOIN::setParams(const char *params, int)
{
	speed = params[0];
}

QString OpcodeJOIN::toString(Field *) const
{
	return QObject::tr("Rassembler les membres de l'équipe dans le personnage jouable (vitesse=%1)")
			.arg(speed);
}

QByteArray OpcodeJOIN::params() const
{
	return QByteArray()
			.append(speed);
}

OpcodeSPLIT::OpcodeSPLIT(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSPLIT::setParams(const char *params, int)
{
	memcpy(banks, params, 3);

	memcpy(&targetX1, params + 3, 2); // bank 1
	memcpy(&targetY1, params + 5, 2); // bank 2
	direction1 = params[7]; // bank 3
	memcpy(&targetX2, params + 8, 2); // bank 4
	memcpy(&targetY2, params + 10, 2); // bank 5
	direction2 = params[12]; // bank 6
	speed = params[13];
}

QString OpcodeSPLIT::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), targetX1 & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY1 & 0xFF, FF7Var::SignedWord));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), direction1 & 0xFF, FF7Var::Byte));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), targetX2 & 0xFF, FF7Var::SignedWord));
	if(B1(banks[2]) != 0)
		vars.append(FF7Var(B1(banks[2]), targetY2 & 0xFF, FF7Var::SignedWord));
	if(B2(banks[2]) != 0)
		vars.append(FF7Var(B2(banks[2]), direction2 & 0xFF, FF7Var::Byte));
}

OpcodePartyE::OpcodePartyE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePartyE::setParams(const char *params, int)
{
	memcpy(banks, params, 2);

	party1 = params[2]; // bank 1
	party2 = params[3]; // bank 2
	party3 = params[4]; // bank 3 -checked-
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
		vars.append(FF7Var(B1(banks[0]), party1, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), party2, FF7Var::Byte));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), party3, FF7Var::Byte));
}

OpcodeSPTYE::OpcodeSPTYE(const char *params, int size) :
	OpcodePartyE(params, size)
{
}

OpcodeSPTYE::OpcodeSPTYE(const OpcodePartyE &op) :
	OpcodePartyE(op)
{
}

QString OpcodeSPTYE::toString(Field *) const
{
	return QObject::tr("Sauvegarder les membres de l'équipe : %1 | %2 | %3")
			.arg(_var(party1, B1(banks[0])))
			.arg(_var(party2, B2(banks[0])))
			.arg(_var(party3, B1(banks[1])));
}

OpcodeGTPYE::OpcodeGTPYE(const char *params, int size) :
	OpcodePartyE(params, size)
{
}

OpcodeGTPYE::OpcodeGTPYE(const OpcodePartyE &op) :
	OpcodePartyE(op)
{
}

QString OpcodeGTPYE::toString(Field *) const
{
	return QObject::tr("Récupérer les membres de l'équipe : %1 | %2 | %3")
			.arg(_var(party1, B1(banks[0])))
			.arg(_var(party2, B2(banks[0])))
			.arg(_var(party3, B1(banks[1])));
}

void OpcodeGTPYE::getVariables(QList<FF7Var> &vars) const
{
	QList<FF7Var> partyVars;
	OpcodePartyE::getVariables(partyVars);
	for(int i = 0 ; i < partyVars.size() ; ++i) {
		partyVars[i].write = true;
	}
	vars.append(partyVars);
}

OpcodeDSKCG::OpcodeDSKCG(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeDSKCG::setParams(const char *params, int)
{
	diskID = params[0];
}

QString OpcodeDSKCG::toString(Field *) const
{
	return QObject::tr("Demander le CD %1")
			.arg(diskID);
}

QByteArray OpcodeDSKCG::params() const
{
	return QByteArray().append((char)diskID);
}

OpcodeSPECIALARROW::OpcodeSPECIALARROW(const char *params, int size)
{
	setParams(params, size);
}

quint8 OpcodeSPECIALARROW::size() const
{
	return 2;
}

void OpcodeSPECIALARROW::setParams(const char *params, int)
{
	hide = params[0];// Boolean
}

QString OpcodeSPECIALARROW::toString(Field *) const
{
	return QObject::tr("%1 le curseur main")
			.arg(hide == 0 ? QObject::tr("Afficher") : QObject::tr("Ne pas afficher"));
}

QByteArray OpcodeSPECIALARROW::params() const
{
	return QByteArray().append((char)hide);
}

OpcodeSPECIALPNAME::OpcodeSPECIALPNAME(const char *params, int size)
{
	setParams(params, size);
}

quint8 OpcodeSPECIALPNAME::size() const
{
	return 2;
}

void OpcodeSPECIALPNAME::setParams(const char *params, int size)
{
	Q_UNUSED(size)
	unknown = params[0];
}

QString OpcodeSPECIALPNAME::toString(Field *) const
{
	return QObject::tr("PNAME - Désactiver le menu de droite (%1)")
			.arg(unknown);
}

QByteArray OpcodeSPECIALPNAME::params() const
{
	return QByteArray().append((char)unknown);
}

OpcodeSPECIALGMSPD::OpcodeSPECIALGMSPD(const char *params, int size)
{
	setParams(params, size);
}

quint8 OpcodeSPECIALGMSPD::size() const
{
	return 2;
}

void OpcodeSPECIALGMSPD::setParams(const char *params, int)
{
	speed = params[0];
}

QString OpcodeSPECIALGMSPD::toString(Field *) const
{
	return QObject::tr("Modifier la vitesse de jeu (%1)")
			.arg(speed);
}

QByteArray OpcodeSPECIALGMSPD::params() const
{
	return QByteArray().append((char)speed);
}

OpcodeSPECIALSMSPD::OpcodeSPECIALSMSPD(const char *params, int size)
{
	setParams(params, size);
}

quint8 OpcodeSPECIALSMSPD::size() const
{
	return 3;
}

void OpcodeSPECIALSMSPD::setParams(const char *params, int)
{
	unknown = params[0];
	speed = params[1];
}

QString OpcodeSPECIALSMSPD::toString(Field *) const
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

QString OpcodeSPECIALFLMAT::toString(Field *) const
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

QString OpcodeSPECIALFLITM::toString(Field *) const
{
	return QObject::tr("Remplir l'inventaire par tous les objets en quantité maximum");
}


OpcodeSPECIALBTLCK::OpcodeSPECIALBTLCK(const char *params, int size)
{
	setParams(params, size);
}

quint8 OpcodeSPECIALBTLCK::size() const
{
	return 2;
}

void OpcodeSPECIALBTLCK::setParams(const char *params, int)
{
	lock = params[0]; // Boolean
}

QString OpcodeSPECIALBTLCK::toString(Field *) const
{
	return QObject::tr("%1 les combats")
			.arg(lock == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeSPECIALBTLCK::params() const
{
	return QByteArray().append((char)lock);
}

OpcodeSPECIALMVLCK::OpcodeSPECIALMVLCK(const char *params, int size)
{
	setParams(params, size);
}

quint8 OpcodeSPECIALMVLCK::size() const
{
	return 2;
}

void OpcodeSPECIALMVLCK::setParams(const char *params, int)
{
	lock = params[0]; // Boolean
}

QString OpcodeSPECIALMVLCK::toString(Field *) const
{
	return QObject::tr("%1 les cinématiques")
			.arg(lock == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeSPECIALMVLCK::params() const
{
	return QByteArray().append((char)lock);
}

OpcodeSPECIALSPCNM::OpcodeSPECIALSPCNM(const char *params, int size)
{
	setParams(params, size);
}

quint8 OpcodeSPECIALSPCNM::size() const
{
	return 3;
}

void OpcodeSPECIALSPCNM::setParams(const char *params, int)
{
	charID = params[0];
	textID = params[1];
}

QString OpcodeSPECIALSPCNM::toString(Field *field) const
{
	return QObject::tr("Changer le nom de %1 par le texte %2")
			.arg(character(charID))
			.arg(_text(textID, field->scriptsAndTexts()));
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

QString OpcodeSPECIALRSGLB::toString(Field *) const
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

QString OpcodeSPECIALCLITM::toString(Field *) const
{
	return QObject::tr("Supprimer tous les objets de l'inventaire");
}

OpcodeSPECIAL::OpcodeSPECIAL(const char *params, int size) :
	opcode(0)
{
	setParams(params, size);
}

OpcodeSPECIAL::OpcodeSPECIAL(const OpcodeSPECIAL &other) :
	Opcode(other), opcode(0)
{
	// Realloc opcode attribute
	setParams(other.params().constData(), other.params().size());
}

OpcodeSPECIAL::~OpcodeSPECIAL()
{
	delete opcode;
}

quint8 OpcodeSPECIAL::size() const
{
	return 1 + opcode->size();
}

QString OpcodeSPECIAL::toString(Field *field) const
{
	return QObject::tr("SPECIAL - ") + opcode->toString(field);
}

void OpcodeSPECIAL::setParams(const char *params, int size)
{
	if(opcode)		delete opcode;

	switch((quint8)params[0])
	{
	case 0xF5:	opcode = new OpcodeSPECIALARROW(params + 1, size - 1);
		break;
	case 0xF6:	opcode = new OpcodeSPECIALPNAME(params + 1, size - 1);
		break;
	case 0xF7:	opcode = new OpcodeSPECIALGMSPD(params + 1, size - 1);
		break;
	case 0xF8:	opcode = new OpcodeSPECIALSMSPD(params + 1, size - 1);
		break;
	case 0xF9:	opcode = new OpcodeSPECIALFLMAT();
		break;
	case 0xFA:	opcode = new OpcodeSPECIALFLITM();
		break;
	case 0xFB:	opcode = new OpcodeSPECIALBTLCK(params + 1, size - 1);
		break;
	case 0xFC:	opcode = new OpcodeSPECIALMVLCK(params + 1, size - 1);
		break;
	case 0xFD:	opcode = new OpcodeSPECIALSPCNM(params + 1, size - 1);
		break;
	case 0xFE:	opcode = new OpcodeSPECIALRSGLB();
		break;
	case 0xFF:	opcode = new OpcodeSPECIALCLITM();
		break;
	default:
		qWarning() << "unknown opcode SPECIAL type" << (quint8)params[0];
		opcode = new OpcodeUnknown(params[0]);
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
	_jump(0), _label(0), _badJump(false)
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
	_badJump = false;
}

bool OpcodeJump::isBadJump() const
{
	return _badJump;
}

void OpcodeJump::setBadJump(bool badJump)
{
	_badJump = badJump;
}

quint32 OpcodeJump::maxJump() const
{
	if(isLongJump()) {
		return 65535 + jumpPosData();
	} else {
		return 255 + jumpPosData();
	}
}

OpcodeLabel::OpcodeLabel(quint32 label) :
	_label(label)
{
}

QString OpcodeLabel::toString(Field *) const
{
	return QObject::tr("Label %1")
			.arg(_label);
}

quint32 OpcodeLabel::label() const
{
	return _label;
}

void OpcodeLabel::setLabel(quint32 label)
{
	_label = label;
}

OpcodeJMPF::OpcodeJMPF(const char *params, int size) :
	OpcodeJump()
{
	setParams(params, size);
}

OpcodeJMPF::OpcodeJMPF(const OpcodeJump &op) :
	OpcodeJump(op)
{
}

void OpcodeJMPF::setParams(const char *params, int)
{
	_jump = (quint8)params[0] + jumpPosData();
}

QString OpcodeJMPF::toString(Field *) const
{
	return _badJump ? QObject::tr("Avancer de %n octet(s)", "With plural", _jump)
					: QObject::tr("Aller au label %1")
					  .arg(_label);
}

QByteArray OpcodeJMPF::params() const
{
	return QByteArray().append(char(_jump - jumpPosData()));
}

OpcodeJMPFL::OpcodeJMPFL(const char *params, int size) :
	OpcodeJump()
{
	setParams(params, size);
}

OpcodeJMPFL::OpcodeJMPFL(const OpcodeJump &op) :
	OpcodeJump(op)
{
}

void OpcodeJMPFL::setParams(const char *params, int)
{
	quint16 jump;
	memcpy(&jump, params, 2);

	_jump = jump + jumpPosData();
}

QString OpcodeJMPFL::toString(Field *) const
{
	return _badJump ? QObject::tr("Avancer de %n octet(s)", "With plural", _jump)
					: QObject::tr("Aller au label %1")
					  .arg(_label);
}

QByteArray OpcodeJMPFL::params() const
{
	quint16 jump = _jump - jumpPosData();
	return QByteArray().append((char *)&jump, 2);
}

OpcodeJMPB::OpcodeJMPB(const char *params, int size) :
	OpcodeJump()
{
	setParams(params, size);
}

OpcodeJMPB::OpcodeJMPB(const OpcodeJump &op) :
	OpcodeJump(op)
{
}

void OpcodeJMPB::setParams(const char *params, int)
{
	_jump = -(quint8)params[0];
}

QString OpcodeJMPB::toString(Field *) const
{
	return _badJump ? QObject::tr("Reculer de %n octet(s)", "With plural", -_jump)
					: QObject::tr("Aller au label %1")
					  .arg(_label);
}

QByteArray OpcodeJMPB::params() const
{
	return QByteArray().append(char(-_jump));
}

OpcodeJMPBL::OpcodeJMPBL(const char *params, int size) :
	OpcodeJump()
{
	setParams(params, size);
}

OpcodeJMPBL::OpcodeJMPBL(const OpcodeJump &op) :
	OpcodeJump(op)
{
}

void OpcodeJMPBL::setParams(const char *params, int)
{
	quint16 jump;
	memcpy(&jump, params, 2);

	_jump = -jump;
}

QString OpcodeJMPBL::toString(Field *) const
{
	return _badJump ? QObject::tr("Reculer de %n octet(s)", "With plural", -_jump)
					: QObject::tr("Aller au label %1")
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

	if(B1(banks) != 0) {
		Operator op = Operator(oper);
		vars.append(FF7Var(B1(banks), value1 & 0xFF,
						   op == BitAnd || op == BitXOr || op == BitOr
						   || op == BitOn || op == BitOff
						   ? FF7Var::Bit : FF7Var::Byte));
	}
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), value2 & 0xFF, FF7Var::Byte));
}

OpcodeIFUB::OpcodeIFUB(const char *params, int size)
{
	setParams(params, size);
}

OpcodeIFUB::OpcodeIFUB(const OpcodeIf &op) :
	OpcodeIf(op)
{
}

void OpcodeIFUB::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	value1 = (quint8)params[1]; // bank 1
	value2 = (quint8)params[2]; // bank 2
	oper = (quint8)params[3];
	_jump = (quint8)params[4] + jumpPosData();
}

QString OpcodeIFUB::toString(Field *) const
{
	return QObject::tr("Si %1 %3 %2 (%4)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_badJump
				 ? QObject::tr("avancer de %n octet(s) sinon", "With plural", _jump)
				 : QObject::tr("aller au label %1 sinon").arg(_label));
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

OpcodeIFUBL::OpcodeIFUBL(const char *params, int size)
{
	setParams(params, size);
}

OpcodeIFUBL::OpcodeIFUBL(const OpcodeIf &op) :
	OpcodeIf(op)
{
}

void OpcodeIFUBL::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	value1 = (quint8)params[1]; // bank 1
	value2 = (quint8)params[2]; // bank 2
	oper = (quint8)params[3];
	quint16 jump;
	memcpy(&jump, params + 4, 2);
	_jump = jump + jumpPosData();
}

QString OpcodeIFUBL::toString(Field *) const
{
	return QObject::tr("Si %1 %3 %2 (%4)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_badJump
				 ? QObject::tr("avancer de %n octet(s) sinon", "With plural", _jump)
				 : QObject::tr("aller au label %1 sinon").arg(_label));
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

OpcodeIFSW::OpcodeIFSW(const char *params, int size)
{
	setParams(params, size);
}

OpcodeIFSW::OpcodeIFSW(const OpcodeIf &op) :
	OpcodeIf(op)
{
}

void OpcodeIFSW::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	qint16 v1, v2;
	memcpy(&v1, params + 1, 2);
	memcpy(&v2, params + 3, 2);
	value1 = v1;
	value2 = v2;
	oper = (quint8)params[5];
	_jump = (quint8)params[6] + jumpPosData();
}

QString OpcodeIFSW::toString(Field *) const
{
	return QObject::tr("Si %1 %3 %2 (%4)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_badJump
				 ? QObject::tr("avancer de %n octet(s) sinon", "With plural", _jump)
				 : QObject::tr("aller au label %1 sinon").arg(_label));
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

void OpcodeIFSW::getVariables(QList<FF7Var> &vars) const
{
	QList<FF7Var> ifVars;
	OpcodeIf::getVariables(ifVars);
	for (int i = 0 ; i < ifVars.size() ; ++i) {
		ifVars[i].size = FF7Var::SignedWord;
	}
	vars.append(ifVars);
}

OpcodeIFSWL::OpcodeIFSWL(const char *params, int size)
{
	setParams(params, size);
}

OpcodeIFSWL::OpcodeIFSWL(const OpcodeIf &op) :
	OpcodeIf(op)
{
}

void OpcodeIFSWL::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	qint16 v1, v2;
	memcpy(&v1, params + 1, 2);
	memcpy(&v2, params + 3, 2);
	value1 = v1;
	value2 = v2;
	oper = (quint8)params[5];
	quint16 jump;
	memcpy(&jump, params + 6, 2);
	_jump = jump + jumpPosData();
}

QString OpcodeIFSWL::toString(Field *) const
{
	return QObject::tr("Si %1 %3 %2 (%4)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_badJump
				 ? QObject::tr("avancer de %n octet(s) sinon", "With plural", _jump)
				 : QObject::tr("aller au label %1 sinon").arg(_label));
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

void OpcodeIFSWL::getVariables(QList<FF7Var> &vars) const
{
	QList<FF7Var> ifVars;
	OpcodeIf::getVariables(ifVars);
	for (int i = 0 ; i < ifVars.size() ; ++i) {
		ifVars[i].size = FF7Var::SignedWord;
	}
	vars.append(ifVars);
}

OpcodeIFUW::OpcodeIFUW(const char *params, int size)
{
	setParams(params, size);
}

OpcodeIFUW::OpcodeIFUW(const OpcodeIf &op) :
	OpcodeIf(op)
{
}

void OpcodeIFUW::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	quint16 v1, v2;
	memcpy(&v1, params + 1, 2);
	memcpy(&v2, params + 3, 2);
	value1 = v1;
	value2 = v2;
	oper = (quint8)params[5];
	_jump = (quint8)params[6] + jumpPosData();
}

QString OpcodeIFUW::toString(Field *) const
{
	return QObject::tr("Si %1 %3 %2 (%4)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_badJump
				 ? QObject::tr("avancer de %n octet(s) sinon", "With plural", _jump)
				 : QObject::tr("aller au label %1 sinon").arg(_label));
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

void OpcodeIFUW::getVariables(QList<FF7Var> &vars) const
{
	QList<FF7Var> ifVars;
	OpcodeIf::getVariables(ifVars);
	for (int i = 0 ; i < ifVars.size() ; ++i) {
		ifVars[i].size = FF7Var::Word;
	}
	vars.append(ifVars);
}

OpcodeIFUWL::OpcodeIFUWL(const char *params, int size)
{
	setParams(params, size);
}

OpcodeIFUWL::OpcodeIFUWL(const OpcodeIf &op) :
	OpcodeIf(op)
{
}

void OpcodeIFUWL::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	quint16 v1, v2;
	memcpy(&v1, params + 1, 2);
	memcpy(&v2, params + 3, 2);
	value1 = v1;
	value2 = v2;
	oper = (quint8)params[5];
	quint16 jump;
	memcpy(&jump, params + 6, 2);
	_jump = jump + jumpPosData();
}

QString OpcodeIFUWL::toString(Field *) const
{
	return QObject::tr("Si %1 %3 %2 (%4)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_badJump
				 ? QObject::tr("avancer de %n octet(s) sinon", "With plural", _jump)
				 : QObject::tr("aller au label %1 sinon").arg(_label));
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

void OpcodeIFUWL::getVariables(QList<FF7Var> &vars) const
{
	QList<FF7Var> ifVars;
	OpcodeIf::getVariables(ifVars);
	for (int i = 0 ; i < ifVars.size() ; ++i) {
		ifVars[i].size = FF7Var::Word;
	}
	vars.append(ifVars);
}

OpcodeMINIGAME::OpcodeMINIGAME(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMINIGAME::setParams(const char *params, int)
{
	memcpy(&fieldID, params, 2);
	memcpy(&targetX, params + 2, 2);
	memcpy(&targetY, params + 4, 2);
	memcpy(&targetI, params + 6, 2);
	minigameParam = (quint8)params[8];
	minigameID = (quint8)params[9];
}

QString OpcodeMINIGAME::toString(Field *) const
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

	return QObject::tr("Lancer un mini-jeu : %5 (Après le jeu aller à l'écran %1 (X=%2, Y=%3, triangle id=%4))")
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

OpcodeTUTOR::OpcodeTUTOR(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeTUTOR::setParams(const char *params, int)
{
	tutoID = (quint8)params[0];
}

QString OpcodeTUTOR::toString(Field *) const
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

OpcodeBTMD2::OpcodeBTMD2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBTMD2::setParams(const char *params, int)
{
	memcpy(&battleMode, params, 4);
}

QString OpcodeBTMD2::toString(Field *) const
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
			case 6:		modes.append(QObject::tr("Active l'arène de combat du Gold Saucer"));									break;
			case 7:		modes.append(QObject::tr("Ne pas afficher d'écran de récompense"));						break;
			case 8:		modes.append(QObject::tr("Les personnages ne font pas leur animation de victoire"));	break;
			case 16:	modes.append(QObject::tr("Désactiver Game Over"));										break;
			default:	modes.append(QString("%1?").arg(i));													break;
			}
		}
	}

	return QObject::tr("Mode de combat : %1").arg(modes.isEmpty() ? QObject::tr("Aucun") : modes.join(", "));
}

QByteArray OpcodeBTMD2::params() const
{
	return QByteArray().append((char *)&battleMode, 4);
}

OpcodeBTRLD::OpcodeBTRLD(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBTRLD::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	var = (quint8)params[1]; // bank 2
}

QString OpcodeBTRLD::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), var, FF7Var::Word, true));
}

OpcodeWAIT::OpcodeWAIT(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeWAIT::setParams(const char *params, int)
{
	memcpy(&frameCount, params, 2);
}

QString OpcodeWAIT::toString(Field *) const
{
	return QObject::tr("Attendre %1 img")
			.arg(frameCount);
}

QByteArray OpcodeWAIT::params() const
{
	return QByteArray().append((char *)&frameCount, 2);
}

OpcodeNFADE::OpcodeNFADE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeNFADE::setParams(const char *params, int)
{
	memcpy(banks, params, 2);
	unknown1 = params[2];
	r = params[3]; // bank 1
	g = params[4]; // bank 2
	b = params[5]; // bank 3
	unknown2 = params[6]; // bank 4
	unknown3 = params[7]; // Unused?
}

QString OpcodeNFADE::toString(Field *) const
{
	return QObject::tr("Voiler l'écran avec la couleur RVB(%2, %3, %4) (u1=%1, u2=%5)")
			.arg(unknown1)
			.arg(_var(r, B1(banks[0])))
			.arg(_var(g, B2(banks[0])))
			.arg(_var(b, B1(banks[1])))
			.arg(_var(unknown2, B2(banks[1])));
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
		vars.append(FF7Var(B1(banks[0]), r & 0xFF, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), g & 0xFF, FF7Var::Byte));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), b & 0xFF, FF7Var::Byte));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), unknown2 & 0xFF, FF7Var::Byte)); // FIXME: byte?
}

OpcodeBLINK::OpcodeBLINK(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBLINK::setParams(const char *params, int)
{
	closed = params[0]; // boolean
}

QString OpcodeBLINK::toString(Field *) const
{
	return QObject::tr("L'objet 3D cligne des yeux : %1")
			.arg(closed == 0 ? QObject::tr("ON") : QObject::tr("OFF"));
}

QByteArray OpcodeBLINK::params() const
{
	return QByteArray().append((char)closed);
}

OpcodeBGMOVIE::OpcodeBGMOVIE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBGMOVIE::setParams(const char *params, int)
{
	disabled = params[0]; // boolean
}

QString OpcodeBGMOVIE::toString(Field *) const
{
	return QObject::tr("BGMOVIE : %1")
			.arg(disabled == 0 ? QObject::tr("ON") : QObject::tr("OFF"));
}

QByteArray OpcodeBGMOVIE::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodeKAWAIEYETX::OpcodeKAWAIEYETX(const char *params, int size)
{
	setParams(params, size);
}

quint8 OpcodeKAWAIEYETX::size() const
{
	return 5 + data.size();
}

void OpcodeKAWAIEYETX::setParams(const char *params, int size)
{
	eyeID1 = (quint8)params[0];
	eyeID2 = (quint8)params[1];
	mouthID = (quint8)params[2];
	objectID = (quint8)params[3];
	data = QByteArray(params + 4, size - 4);
}

QString OpcodeKAWAIEYETX::toString(Field *) const
{
	return QObject::tr("Changer l'état de la texture des yeux ou de la bouche (oeil 1=%1, oeil 2=%2, bouche=%3, ID objet 3D=%4)")
			.arg(eyeID1)
			.arg(eyeID2)
			.arg(mouthID)
			.arg(objectID);
}

QByteArray OpcodeKAWAIEYETX::params() const
{
	return QByteArray()
			.append((char)eyeID1)
			.append((char)eyeID2)
			.append((char)mouthID)
			.append((char)objectID)
			.append(data);
}

OpcodeKAWAITRNSP::OpcodeKAWAITRNSP(const char *params, int size)
{
	setParams(params, size);
}

quint8 OpcodeKAWAITRNSP::size() const
{
	return 2 + data.size();
}

void OpcodeKAWAITRNSP::setParams(const char *params, int size)
{
	enableTransparency = (quint8)params[0];
	data = QByteArray(params + 1, size - 1);
}

QString OpcodeKAWAITRNSP::toString(Field *) const
{
	return QObject::tr("%1 transparence")
			.arg(enableTransparency == 0 ? QObject::tr("Désactiver") : QObject::tr("Activer"));
}

QByteArray OpcodeKAWAITRNSP::params() const
{
	return QByteArray()
			.append((char)enableTransparency)
			.append(data);
}

OpcodeKAWAIAMBNT::OpcodeKAWAIAMBNT(const char *params, int size)
{
	setParams(params, size);
}

quint8 OpcodeKAWAIAMBNT::size() const
{
	return 8 + data.size();
}

void OpcodeKAWAIAMBNT::setParams(const char *params, int size)
{
	r1 = (quint8)params[0];
	r2 = (quint8)params[1];
	g1 = (quint8)params[2];
	g2 = (quint8)params[3];
	b1 = (quint8)params[4];
	b2 = (quint8)params[5];
	flags = (quint8)params[6];
	data = QByteArray(params + 7, size - 7);
}

QString OpcodeKAWAIAMBNT::toString(Field *) const
{
	return QObject::tr("Change la couleur ambiante du modèle : RVB(%1, %2, %3) RVB(%4, %5, %6) (flags=%7)")
			.arg(r1)
			.arg(g1)
			.arg(b1)
			.arg(r2)
			.arg(g2)
			.arg(b2)
			.arg(flags);
}

QByteArray OpcodeKAWAIAMBNT::params() const
{
	return QByteArray()
			.append((char)r1)
			.append((char)r2)
			.append((char)g1)
			.append((char)g2)
			.append((char)b1)
			.append((char)b2)
			.append((char)flags)
			.append(data);
}

OpcodeKAWAIUNKNOWN4::OpcodeKAWAIUNKNOWN4(const char *params, int size) :
	OpcodeUnknown(0x04, params, size)
{
}

QString OpcodeKAWAIUNKNOWN4::toString(Field *) const
{
	return QObject::tr("UNKNOWN4");
}

OpcodeKAWAILIGHT::OpcodeKAWAILIGHT(const char *params, int size) :
	OpcodeUnknown(0x06, params, size)
{
}

QString OpcodeKAWAILIGHT::toString(Field *) const
{
	return QObject::tr("LIGHT");
}

OpcodeKAWAIUNKNOWN7::OpcodeKAWAIUNKNOWN7(const char *params, int size) :
	OpcodeUnknown(0x07, params, size)
{
}

QString OpcodeKAWAIUNKNOWN7::toString(Field *) const
{
	return QObject::tr("UNKNOWN7");
}

OpcodeKAWAIUNKNOWN8::OpcodeKAWAIUNKNOWN8(const char *params, int size) :
	OpcodeUnknown(0x08, params, size)
{
}

QString OpcodeKAWAIUNKNOWN8::toString(Field *) const
{
	return QObject::tr("UNKNOWN8");
}

OpcodeKAWAIUNKNOWN9::OpcodeKAWAIUNKNOWN9(const char *params, int size) :
	OpcodeUnknown(0x09, params, size)
{
}

QString OpcodeKAWAIUNKNOWN9::toString(Field *) const
{
	return QObject::tr("UNKNOWN9");
}

OpcodeKAWAISBOBJ::OpcodeKAWAISBOBJ(const char *params, int size) :
	OpcodeUnknown(0x0A, params, size)
{
}

QString OpcodeKAWAISBOBJ::toString(Field *) const
{
	return QObject::tr("SBOBJ");
}

OpcodeKAWAIUNKNOWNB::OpcodeKAWAIUNKNOWNB(const char *params, int size) :
	OpcodeUnknown(0x0B, params, size)
{
}

QString OpcodeKAWAIUNKNOWNB::toString(Field *) const
{
	return QObject::tr("UNKNOWNB");
}

OpcodeKAWAIUNKNOWNC::OpcodeKAWAIUNKNOWNC(const char *params, int size) :
	OpcodeUnknown(0x0C, params, size)
{
}

QString OpcodeKAWAIUNKNOWNC::toString(Field *) const
{
	return QObject::tr("UNKNOWNC");
}

OpcodeKAWAISHINE::OpcodeKAWAISHINE(const char *params, int size) :
	OpcodeUnknown(0x0D, params, size)
{
}

QString OpcodeKAWAISHINE::toString(Field *) const
{
	return QObject::tr("SHINE");
}

OpcodeKAWAIRESET::OpcodeKAWAIRESET(const char *params, int size) :
	OpcodeUnknown(0xFF, params, size)
{
}

QString OpcodeKAWAIRESET::toString(Field *) const
{
	return QObject::tr("RESET");
}

OpcodeKAWAI::OpcodeKAWAI(const char *params, int size) :
	opcode(0)
{
	setParams(params, size);
}

OpcodeKAWAI::OpcodeKAWAI(const OpcodeKAWAI &other) :
	Opcode(other), opcode(0)
{
	// Realloc opcode attribute
	setParams(other.params().constData(), other.params().size());
}

OpcodeKAWAI::~OpcodeKAWAI()
{
	delete opcode;
}

quint8 OpcodeKAWAI::size() const
{
	return opcode->size() + 2;
}

QString OpcodeKAWAI::toString(Field *field) const
{
	return QObject::tr("Filtre graphique sur l'objet 3D - %1")
			.arg(opcode->toString(field));
}

void OpcodeKAWAI::setParams(const char *params, int size)
{
	if(opcode)		delete opcode;

//	size = params[0];
	switch((quint8)params[1])
	{
	case 0x00:	opcode = new OpcodeKAWAIEYETX(params + 2, size - 2);
		break;
	case 0x01:	opcode = new OpcodeKAWAITRNSP(params + 2, size - 2);
		break;
	case 0x02:	opcode = new OpcodeKAWAIAMBNT(params + 2, size - 2);
		break;
	case 0x04:	opcode = new OpcodeKAWAIUNKNOWN4(params + 2, size - 2);
		break;
	case 0x06:	opcode = new OpcodeKAWAILIGHT(params + 2, size - 2);
		break;
	case 0x07:	opcode = new OpcodeKAWAIUNKNOWN7(params + 2, size - 2);
		break;
	case 0x08:	opcode = new OpcodeKAWAIUNKNOWN8(params + 2, size - 2);
		break;
	case 0x09:	opcode = new OpcodeKAWAIUNKNOWN9(params + 2, size - 2);
		break;
	case 0x0A:	opcode = new OpcodeKAWAISBOBJ(params + 2, size - 2);
		break;
	case 0x0B:	opcode = new OpcodeKAWAIUNKNOWNB(params + 2, size - 2);
		break;
	case 0x0C:	opcode = new OpcodeKAWAIUNKNOWNC(params + 2, size - 2);
		break;
	case 0x0D:	opcode = new OpcodeKAWAISHINE(params + 2, size - 2);
		break;
	case 0xFF:	opcode = new OpcodeKAWAIRESET(params + 2, size - 2);
		break;
	default:
		qWarning() << "unknown opcode KAWAI type" << (quint8)params[1];
		opcode = new OpcodeUnknown((quint8)params[1], params + 2, size - 2);
		break;
	}
}

QByteArray OpcodeKAWAI::params() const
{
	return QByteArray()
			.append(size())
			.append(opcode->toByteArray());
}

OpcodeKAWIW::OpcodeKAWIW()
{
}

QString OpcodeKAWIW::toString(Field *) const
{
	return QObject::tr("Attendre la fin de l'exécution du filtre graphique");
}

OpcodePMOVA::OpcodePMOVA(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePMOVA::setParams(const char *params, int)
{
	partyID = params[0];
}

QString OpcodePMOVA::toString(Field *) const
{
	return QObject::tr("Déplacer l'objet 3D vers le membre n°%1 de l'équipe")
			.arg(partyID);
}

QByteArray OpcodePMOVA::params() const
{
	return QByteArray().append((char)partyID);
}

OpcodeSLIP::OpcodeSLIP(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSLIP::setParams(const char *params, int)
{
	off = params[0]; // Boolean
}

QString OpcodeSLIP::toString(Field *) const
{
	return QObject::tr("SLIP : %1")
			.arg(off == 0 ? QObject::tr("ON") : QObject::tr("OFF"));
}

QByteArray OpcodeSLIP::params() const
{
	return QByteArray().append((char)off);
}

OpcodeBGPDH::OpcodeBGPDH(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBGPDH::setParams(const char *params, int)
{
	banks = params[0];
	layerID = params[1];
	memcpy(&targetZ, params + 2, 2); // bank 2 ???
}

QString OpcodeBGPDH::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), targetZ & 0xFF, FF7Var::SignedWord));
}

OpcodeBGSCR::OpcodeBGSCR(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBGSCR::setParams(const char *params, int)
{
	banks = params[0];
	layerID = params[1];
	memcpy(&targetX, params + 2, 2); // bank 1
	memcpy(&targetY, params + 4, 2); // bank 2
}

QString OpcodeBGSCR::toString(Field *) const
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
		vars.append(FF7Var(B1(banks), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), targetY & 0xFF, FF7Var::SignedWord));
}

OpcodeWCLS::OpcodeWCLS(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeWCLS::setParams(const char *params, int)
{
	windowID = params[0];
}

QString OpcodeWCLS::toString(Field *) const
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

OpcodeWindow::OpcodeWindow(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeWindow::setParams(const char *params, int)
{
	windowID = params[0];
	memcpy(&targetX, params + 1, 2);
	memcpy(&targetY, params + 3, 2);
	memcpy(&width, params + 5, 2);
	memcpy(&height, params + 7, 2);
}

QByteArray OpcodeWindow::params() const
{
	return QByteArray()
			.append((char)windowID)
			.append((char *)&targetX, 2)
			.append((char *)&targetY, 2)
			.append((char *)&width, 2)
			.append((char *)&height, 2);
}

int OpcodeWindow::getWindowID() const
{
	return windowID;
}

void OpcodeWindow::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

bool OpcodeWindow::getWindow(FF7Window &window) const
{
	window.x = targetX;
	window.y = targetY;
	window.w = width;
	window.h = height;
	return true;
}

void OpcodeWindow::setWindow(const FF7Window &window)
{
	targetX = window.x;
	targetY = window.y;
	width = window.w;
	height = window.h;
}

OpcodeWSIZW::OpcodeWSIZW(const char *params, int size) :
	OpcodeWindow(params, size)
{
}

OpcodeWSIZW::OpcodeWSIZW(const OpcodeWindow &op) :
	OpcodeWindow(op)
{
}

QString OpcodeWSIZW::toString(Field *) const
{
	return QObject::tr("Redimensionner fenêtre n°%1 (X=%2, Y=%3, largeur=%4, hauteur=%5)")
			.arg(windowID)
			.arg(targetX)
			.arg(targetY)
			.arg(width)
			.arg(height);
}

OpcodeIfKey::OpcodeIfKey(const char *params, int size)
{
	setParams(params, size);
}

OpcodeIfKey::OpcodeIfKey(const OpcodeJump &op) :
	OpcodeJump(op)
{
}

void OpcodeIfKey::setParams(const char *params, int)
{
	memcpy(&keys, params, 2);
	_jump = (quint8)params[2] + jumpPosData();
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
	if(ret.isEmpty()) {
		return QObject::tr("(aucune touche)");
	}
	return ret.join(QObject::tr(" ou "));
}

OpcodeIFKEY::OpcodeIFKEY(const char *params, int size) :
	OpcodeIfKey(params, size)
{
}

OpcodeIFKEY::OpcodeIFKEY(const OpcodeIfKey &op) :
	OpcodeIfKey(op)
{
}

QString OpcodeIFKEY::toString(Field *) const
{
	return QObject::tr("Si appuie sur la touche %1 (%2)")
			.arg(keyString())
			.arg(_badJump
				 ? QObject::tr("avancer de %n octet(s) sinon", "With plural", _jump)
				 : QObject::tr("aller au label %1 sinon").arg(_label));
}

OpcodeIFKEYON::OpcodeIFKEYON(const char *params, int size) :
	OpcodeIfKey(params, size)
{
}

OpcodeIFKEYON::OpcodeIFKEYON(const OpcodeIfKey &op) :
	OpcodeIfKey(op)
{
}

QString OpcodeIFKEYON::toString(Field *) const
{
	return QObject::tr("Si appuie sur la touche %1 une fois (%2)")
			.arg(keyString())
			.arg(_badJump
				 ? QObject::tr("avancer de %n octet(s) sinon", "With plural", _jump)
				 : QObject::tr("aller au label %1 sinon").arg(_label));
}

OpcodeIFKEYOFF::OpcodeIFKEYOFF(const char *params, int size) :
	OpcodeIfKey(params, size)
{
}

OpcodeIFKEYOFF::OpcodeIFKEYOFF(const OpcodeIfKey &op) :
	OpcodeIfKey(op)
{
}

QString OpcodeIFKEYOFF::toString(Field *) const
{
	return QObject::tr("Si relache la touche %1 pour la première fois (%2)")
			.arg(keyString())
			.arg(_badJump
				 ? QObject::tr("avancer de %n octet(s) sinon", "With plural", _jump)
				 : QObject::tr("aller au label %1 sinon").arg(_label));
}

OpcodeUC::OpcodeUC(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeUC::setParams(const char *params, int)
{
	disabled = params[0];
}

QString OpcodeUC::toString(Field *) const
{
	return QObject::tr("%1 les déplacements du personnage jouable")
			.arg(disabled == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeUC::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodePDIRA::OpcodePDIRA(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePDIRA::setParams(const char *params, int)
{
	partyID = params[0];
}

QString OpcodePDIRA::toString(Field *) const
{
	return QObject::tr("Tourner instantanément l'objet 3D vers le membre de l'équipe n°%1")
			.arg(partyID);
}

QByteArray OpcodePDIRA::params() const
{
	return QByteArray().append((char)partyID);
}

OpcodePTURA::OpcodePTURA(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePTURA::setParams(const char *params, int)
{
	partyID = params[0];
	speed = params[1];
	directionRotation = params[2];
}

QString OpcodePTURA::toString(Field *) const
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

OpcodeWSPCL::OpcodeWSPCL(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeWSPCL::setParams(const char *params, int)
{
	windowID = params[0];
	displayType = params[1];
	marginLeft = params[2];
	marginTop = params[3];
}

QString OpcodeWSPCL::toString(Field *) const
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

OpcodeWNUMB::OpcodeWNUMB(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeWNUMB::setParams(const char *params, int)
{
	banks = params[0];
	windowID = params[1];
	memcpy(&value, params + 2, 4);// bank 1 and 2
	digitCount = params[6];
}

QString OpcodeWNUMB::toString(Field *) const
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
		vars.append(FF7Var(B1(banks), value & 0xFF, FF7Var::Word));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), (value >> 16) & 0xFF, FF7Var::Word));
}

OpcodeSTTIM::OpcodeSTTIM(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSTTIM::setParams(const char *params, int)
{
	banks[0] = params[0];
	banks[1] = params[1];
	h = params[2];// bank 1??
	m = params[3];// bank 2??
	s = params[4];// bank 3??
}

QString OpcodeSTTIM::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), h, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), m, FF7Var::Byte));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), s, FF7Var::Byte));
}

OpcodeGOLD::OpcodeGOLD(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeGOLD::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&value, params + 1, 4);// bank 1 and 2
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
		vars.append(FF7Var(B1(banks), value & 0xFF, FF7Var::Word));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), (value >> 16) & 0xFF, FF7Var::Word));
}

OpcodeGOLDu::OpcodeGOLDu(const char *params, int size) :
	OpcodeGOLD(params, size)
{
}

OpcodeGOLDu::OpcodeGOLDu(const OpcodeGOLD &op) :
	OpcodeGOLD(op)
{
}

QString OpcodeGOLDu::toString(Field *) const
{
	return QObject::tr("Ajouter %1 gils à l'équipe")
			.arg(_var(value, B1(banks), B2(banks)));
}

OpcodeGOLDd::OpcodeGOLDd(const char *params, int size) :
	OpcodeGOLD(params, size)
{
}

OpcodeGOLDd::OpcodeGOLDd(const OpcodeGOLD &op) :
	OpcodeGOLD(op)
{
}

QString OpcodeGOLDd::toString(Field *) const
{
	return QObject::tr("Retirer %1 gils à l'équipe")
			.arg(_var(value, B1(banks), B2(banks)));
}

OpcodeCHGLD::OpcodeCHGLD(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCHGLD::setParams(const char *params, int)
{
	banks = params[0];
	var1 = params[1]; // bank 1
	var2 = params[2]; // bank 2
}

QString OpcodeCHGLD::toString(Field *) const
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
		vars.append(FF7Var(B1(banks), var1, FF7Var::Word, true));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), var2, FF7Var::Word, true));
}

OpcodeHMPMAX1::OpcodeHMPMAX1()
{
}

QString OpcodeHMPMAX1::toString(Field *) const
{
	return QObject::tr("Redonne les HP/MP aux membres de l'équipe");
}

OpcodeHMPMAX2::OpcodeHMPMAX2()
{
}

QString OpcodeHMPMAX2::toString(Field *) const
{
	return QObject::tr("Redonne les HP/MP aux membres de l'équipe");
}

OpcodeMHMMX::OpcodeMHMMX()
{
}

QString OpcodeMHMMX::toString(Field *) const
{
	return QObject::tr("Redonne les HP/MP à tous et soigne les troubles de statut");
}

OpcodeHMPMAX3::OpcodeHMPMAX3()
{
}

QString OpcodeHMPMAX3::toString(Field *) const
{
	return QObject::tr("Redonne les HP/MP aux membres de l'équipe");
}

OpcodeMESSAGE::OpcodeMESSAGE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMESSAGE::setParams(const char *params, int)
{
	windowID = params[0];
	textID = params[1];
}

QString OpcodeMESSAGE::toString(Field *field) const
{
	return QObject::tr("Afficher message %2 dans la fenêtre n°%1")
			.arg(windowID)
			.arg(_text(textID, field->scriptsAndTexts()));
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

OpcodeMPARA::OpcodeMPARA(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMPARA::setParams(const char *params, int)
{
	banks = params[0];
	windowID = params[1];
	windowVarID = params[2];
	value = params[3]; // bank 2
}

QString OpcodeMPARA::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), value, FF7Var::Byte));
}

OpcodeMPRA2::OpcodeMPRA2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMPRA2::setParams(const char *params, int)
{
	banks = params[0];
	windowID = params[1];
	windowVarID = params[2];
	memcpy(&value, params + 3, 2); // bank 2
}

QString OpcodeMPRA2::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), value & 0xFF, FF7Var::Word));
}

OpcodeMPNAM::OpcodeMPNAM(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMPNAM::setParams(const char *params, int)
{
	textID = params[0];
}

QString OpcodeMPNAM::toString(Field *field) const
{
	return QObject::tr("Afficher %1 dans le menu")
			.arg(_text(textID, field->scriptsAndTexts()));
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

OpcodeHPMP::OpcodeHPMP(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeHPMP::setParams(const char *params, int)
{
	banks = params[0];
	partyID = params[1];
	memcpy(&value, params + 2, 2); // bank 2??
}

QByteArray OpcodeHPMP::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)partyID)
			.append((char *)&value, 2);
}

void OpcodeHPMP::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), value & 0xFF, FF7Var::Word));
}

OpcodeMPu::OpcodeMPu(const char *params, int size) :
	OpcodeHPMP(params, size)
{
}

OpcodeMPu::OpcodeMPu(const OpcodeHPMP &op) :
	OpcodeHPMP(op)
{
}

QString OpcodeMPu::toString(Field *) const
{
	return QObject::tr("Augmenter de %2 MPs le membre n°%1 de l'équipe")
			.arg(partyID)
			.arg(_var(value, B2(banks)));
}

OpcodeMPd::OpcodeMPd(const char *params, int size) :
	OpcodeHPMP(params, size)
{
}

OpcodeMPd::OpcodeMPd(const OpcodeHPMP &op) :
	OpcodeHPMP(op)
{
}

QString OpcodeMPd::toString(Field *) const
{
	return QObject::tr("Diminuer de %2 MPs le membre n°%1 de l'équipe")
			.arg(partyID)
			.arg(_var(value, B2(banks)));
}

OpcodeASK::OpcodeASK(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeASK::setParams(const char *params, int)
{
	banks = params[0];
	windowID = params[1];
	textID = params[2];
	firstLine = params[3];
	lastLine = params[4];
	varAnswer = params[5]; // bank 2
}

QString OpcodeASK::toString(Field *field) const
{
	return QObject::tr("Poser question %2 dans la fenêtre n°%1 (et mettre la réponse sélectionnée dans %5) première ligne=%3, dernière ligne=%4")
			.arg(windowID)
			.arg(_text(textID, field->scriptsAndTexts()))
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
		vars.append(FF7Var(B2(banks), varAnswer, FF7Var::Byte, true));
}

OpcodeMENU::OpcodeMENU(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMENU::setParams(const char *params, int)
{
	banks = params[0];
	menuID = params[1];
	param = params[2]; // bank 2
}

QString OpcodeMENU::menu(const QString &param) const
{
	switch(menuID)
	{
	case 1:		return QObject::tr("Fermer le programme (paramètre %1)").arg(param);
	case 2:		return QObject::tr("Encount Error (paramètre %1)").arg(param);
	case 5:		return QObject::tr("Crédits de ff7 (paramètre %1)").arg(param);
	case 6:		return QObject::tr("Changer nom de %1").arg(character(this->param));//Parameter: char id
	case 7:		return QObject::tr("Changer l'équipe (paramètre %1)").arg(param);
	case 8:		return QObject::tr("magasin n°%1").arg(param);//Parameter: shop id
	case 9:		return QObject::tr("principal (paramètre %1)").arg(param);
	case 12:	return QObject::tr("course de moto (paramètre %1)").arg(param);
	case 14:	return QObject::tr("Sauvegarde (paramètre %1)").arg(param);
	case 15:	return QObject::tr("Effacer toutes les matérias (paramètre %1)").arg(param);
	case 16:	return QObject::tr("Rétablir toutes les matérias (paramètre %1)").arg(param);
	case 17:	return QObject::tr("Effacer la matéria de %1").arg(character(this->param));//Parameter: char id
	case 18:	return QObject::tr("Effacer les matérias de Clad (paramètre %1)").arg(param);
	case 19:	return QObject::tr("Rétablir les matérias de Clad (paramètre %1)").arg(param);
	case 20:	return QObject::tr("? (paramètre %1)").arg(param);// TODO
	case 21:	return QObject::tr("HPs à 1 (paramètre %1)").arg(param);
	case 22:	return QObject::tr("? (paramètre %1)").arg(param);// TODO
	case 23:	return QObject::tr("maitre ? (paramètre %1)").arg(param);
	case 24:	return QObject::tr("? (paramètre %1)").arg(param);
	case 25:	return QObject::tr("? (paramètre %1)").arg(param);
	default:	return QObject::tr("%1? (paramètre %2)").arg(menuID).arg(param);
	}
}

QString OpcodeMENU::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), param, FF7Var::Byte));
}

OpcodeMENU2::OpcodeMENU2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMENU2::setParams(const char *params, int)
{
	disabled = params[0];
}

QString OpcodeMENU2::toString(Field *) const
{
	return QObject::tr("%1 l'accès aux menus")
			.arg(disabled == 0 ? QObject::tr("Permettre") : QObject::tr("Interdire"));
}

QByteArray OpcodeMENU2::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodeBTLTB::OpcodeBTLTB(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBTLTB::setParams(const char *params, int)
{
	battleTableID = params[0];
}

QString OpcodeBTLTB::toString(Field *) const
{
	return QObject::tr("Choisir la battle table : %1")
			.arg(battleTableID);
}

QByteArray OpcodeBTLTB::params() const
{
	return QByteArray().append((char)battleTableID);
}

OpcodeHPu::OpcodeHPu(const char *params, int size) :
	OpcodeHPMP(params, size)
{
}

OpcodeHPu::OpcodeHPu(const OpcodeHPMP &op) :
	OpcodeHPMP(op)
{
}

QString OpcodeHPu::toString(Field *) const
{
	return QObject::tr("Augmenter de %2 HPs le membre n°%1 de l'équipe")
			.arg(partyID)
			.arg(_var(value, B2(banks)));
}

OpcodeHPd::OpcodeHPd(const char *params, int size) :
	OpcodeHPMP(params, size)
{
}

OpcodeHPd::OpcodeHPd(const OpcodeHPMP &op) :
	OpcodeHPMP(op)
{
}

QString OpcodeHPd::toString(Field *) const
{
	return QObject::tr("Diminuer de %2 HPs le membre n°%1 de l'équipe")
			.arg(partyID)
			.arg(_var(value, B2(banks)));
}

OpcodeWINDOW::OpcodeWINDOW(const char *params, int size) :
	OpcodeWindow(params, size)
{
}

OpcodeWINDOW::OpcodeWINDOW(const OpcodeWindow &op) :
	OpcodeWindow(op)
{
}

QString OpcodeWINDOW::toString(Field *) const
{
	return QObject::tr("Créer la fenêtre n°%1 (X=%2, Y=%3, largeur=%4, hauteur=%5)")
			.arg(windowID)
			.arg(targetX)
			.arg(targetY)
			.arg(width)
			.arg(height);
}

OpcodeWMOVE::OpcodeWMOVE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeWMOVE::setParams(const char *params, int)
{
	windowID = params[0];
	memcpy(&relativeX, params + 1, 2);
	memcpy(&relativeY, params + 3, 2);
}

QString OpcodeWMOVE::toString(Field *) const
{
	return QObject::tr("Déplacer la fenêtre n°%1 (déplacement : X=%2, Y=%3)")
			.arg(windowID)
			.arg(relativeX)
			.arg(relativeY);
}

QByteArray OpcodeWMOVE::params() const
{
	return QByteArray()
			.append((char)windowID)
			.append((char *)&relativeX, 2)
			.append((char *)&relativeY, 2);
}

int OpcodeWMOVE::getWindowID() const
{
	return windowID;
}

void OpcodeWMOVE::setWindowID(quint8 windowID)
{
	this->windowID = windowID;
}

OpcodeWMODE::OpcodeWMODE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeWMODE::setParams(const char *params, int)
{
	windowID = params[0];
	mode = params[1];
	preventClose = params[2]; // boolean
}

QString OpcodeWMODE::toString(Field *) const
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

OpcodeWREST::OpcodeWREST(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeWREST::setParams(const char *params, int)
{
	windowID = params[0];
}

QString OpcodeWREST::toString(Field *) const
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

bool OpcodeWREST::getWindow(FF7Window &window) const
{
	window.x = 8;
	window.y = 149;
	window.w = 304;
	window.h = 74;
	return true;
}

void OpcodeWREST::setWindow(const FF7Window &)
{
}

OpcodeWCLSE::OpcodeWCLSE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeWCLSE::setParams(const char *params, int)
{
	windowID = params[0];
}

QString OpcodeWCLSE::toString(Field *) const
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

OpcodeWROW::OpcodeWROW(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeWROW::setParams(const char *params, int)
{
	windowID = params[0];
	rowCount = params[1];
}

QString OpcodeWROW::toString(Field *) const
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

OpcodeWCOL::OpcodeWCOL(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeWCOL::setParams(const char *params, int)
{
	banks[0] = params[0];
	banks[1] = params[1];
	corner = params[2]; // bank 1
	r = params[3]; // bank 2
	g = params[4]; // bank 3
	b = params[5]; // bank 4
}

QByteArray OpcodeWCOL::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append((char)corner)
			.append((char)r)
			.append((char)g)
			.append((char)b);
}

OpcodeGWCOL::OpcodeGWCOL(const char *params, int size) :
	OpcodeWCOL(params, size)
{
}

OpcodeGWCOL::OpcodeGWCOL(const OpcodeWCOL &op) :
	OpcodeWCOL(op)
{
}

QString OpcodeGWCOL::toString(Field *) const
{
	return QObject::tr("Obtenir la couleur du côté %1 des fenêtres et en stocker les composantes dans %2 (R), %3 (V) et %4 (B)")
			.arg(_windowCorner(corner, B1(banks[0])))
			.arg(_bank(r, B2(banks[0])))
			.arg(_bank(g, B1(banks[1])))
			.arg(_bank(b, B2(banks[1])));
}

void OpcodeGWCOL::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), corner, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), r, FF7Var::Byte, true));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), g, FF7Var::Byte, true));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), b, FF7Var::Byte, true));
}

OpcodeSWCOL::OpcodeSWCOL(const char *params, int size) :
	OpcodeWCOL(params, size)
{
	setParams(params, size);
}

OpcodeSWCOL::OpcodeSWCOL(const OpcodeWCOL &op) :
	OpcodeWCOL(op)
{
}

QString OpcodeSWCOL::toString(Field *) const
{
	return QObject::tr("Changer la couleur du côté %1 des fenêtres : RVB(%2, %3, %4)")
			.arg(_windowCorner(corner, B1(banks[0])))
			.arg(_var(r, B2(banks[0])))
			.arg(_var(g, B1(banks[1])))
			.arg(_var(b, B2(banks[1])));
}

void OpcodeSWCOL::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), corner, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), r, FF7Var::Byte));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), g, FF7Var::Byte));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), b, FF7Var::Byte));
}

OpcodeItem::OpcodeItem(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeItem::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&itemID, params + 1, 2); // bank 1
	quantity = params[3]; // bank 2
}

QByteArray OpcodeItem::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&itemID, 2)
			.append((char)quantity);
}

void OpcodeItem::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), itemID & 0xFF, FF7Var::Word));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), quantity, FF7Var::Byte));
}

OpcodeSTITM::OpcodeSTITM(const char *params, int size) :
	OpcodeItem(params, size)
{
	setParams(params, size);
}

OpcodeSTITM::OpcodeSTITM(const OpcodeItem &op) :
	OpcodeItem(op)
{
}

QString OpcodeSTITM::toString(Field *) const
{
	return QObject::tr("Ajouter %2 objet(s) %1 dans l'inventaire")
			.arg(_item(itemID, B1(banks)))
			.arg(_var(quantity, B2(banks)));
}

OpcodeDLITM::OpcodeDLITM(const char *params, int size) :
	OpcodeItem(params, size)
{
}

OpcodeDLITM::OpcodeDLITM(const OpcodeItem &op) :
	OpcodeItem(op)
{
}

QString OpcodeDLITM::toString(Field *) const
{
	return QObject::tr("Supprimer %2 objet(s) %1 dans l'inventaire")
			.arg(_item(itemID, B1(banks)))
			.arg(_var(quantity, B2(banks)));
}

OpcodeCKITM::OpcodeCKITM(const char *params, int size) :
	OpcodeItem(params, size)
{
}

OpcodeCKITM::OpcodeCKITM(const OpcodeItem &op) :
	OpcodeItem(op)
{
}

QString OpcodeCKITM::toString(Field *) const
{
	return QObject::tr("%2 = quantité d'objets %1 dans l'inventaire")
			.arg(_item(itemID, B1(banks)))
			.arg(_bank(quantity, B2(banks)));
}

void OpcodeCKITM::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), itemID & 0xFF, FF7Var::Word));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), quantity, FF7Var::Byte, true));
}

OpcodeSMTRA::OpcodeSMTRA(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSMTRA::setParams(const char *params, int)
{
	banks[0] = params[0];
	banks[1] = params[1];
	materiaID = params[2]; // bank 1
	APCount = 0;
	memcpy(&APCount, params + 3, 3); // bank 2, bank 3, bank 4
}

QString OpcodeSMTRA::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), materiaID, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), APCount & 0xFF, FF7Var::Word));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), (APCount >> 8) & 0xFF, FF7Var::Word));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), (APCount >> 16) & 0xFF, FF7Var::Word)); // FIXME: word?
}

OpcodeDMTRA::OpcodeDMTRA(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeDMTRA::setParams(const char *params, int)
{
	banks[0] = params[0];
	banks[1] = params[1];
	materiaID = params[2]; // bank 1
	APCount = 0;
	memcpy(&APCount, params + 3, 3); // bank 2, bank 3, bank 4
	quantity = params[6];
}

QString OpcodeDMTRA::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), materiaID, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), APCount & 0xFF, FF7Var::Word));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), (APCount >> 8) & 0xFF, FF7Var::Word));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), (APCount >> 16) & 0xFF, FF7Var::Word)); // FIXME: word?
}

OpcodeCMTRA::OpcodeCMTRA(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCMTRA::setParams(const char *params, int)
{
	banks[0] = params[0];
	banks[1] = params[1];
	banks[2] = params[2];
	materiaID = params[3]; // bank 1
	APCount = 0;
	memcpy(&APCount, params + 4, 3); // bank 2, bank 3, bank 4
	unknown = params[7];
	varQuantity = params[8]; // bank 5
}

QString OpcodeCMTRA::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), materiaID, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), APCount & 0xFF, FF7Var::Word));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), (APCount >> 8) & 0xFF, FF7Var::Word));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), (APCount >> 16) & 0xFF, FF7Var::Word)); // FIXME: word?
	if(B2(banks[2]) != 0)
		vars.append(FF7Var(B2(banks[2]), varQuantity, FF7Var::Byte, true)); // FIXME: word?
}

OpcodeSHAKE::OpcodeSHAKE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSHAKE::setParams(const char *params, int)
{
	unknown1 = params[0];
	unknown2 = params[1];
	shakeCount = params[2];
	unknown3 = params[3];
	unknown4 = params[4];
	amplitude = params[5];
	speed = params[6];
}

QString OpcodeSHAKE::toString(Field *) const
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

QString OpcodeNOP::toString(Field *) const
{
	return QObject::tr("Ne rien faire...");
}

OpcodeMAPJUMP::OpcodeMAPJUMP(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMAPJUMP::setParams(const char *params, int)
{
	memcpy(&fieldID, params, 2);
	memcpy(&targetX, params + 2, 2);
	memcpy(&targetY, params + 4, 2);
	memcpy(&targetI, params + 6, 2);
	direction = params[8];
}

QString OpcodeMAPJUMP::toString(Field *) const
{
	return QObject::tr("Aller à l'écran %1 (X=%2, Y=%3, triangle id=%4, direction=%5)")
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

OpcodeSCRLO::OpcodeSCRLO(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSCRLO::setParams(const char *params, int)
{
	unknown = params[0];
}

QString OpcodeSCRLO::toString(Field *) const
{
	return QObject::tr("SCRLO (?=%1)")
			.arg(unknown);
}

QByteArray OpcodeSCRLO::params() const
{
	return QByteArray().append((char)unknown);
}

OpcodeSCRLC::OpcodeSCRLC(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSCRLC::setParams(const char *params, int)
{
	memcpy(&unknown, params, 4);
}

QString OpcodeSCRLC::toString(Field *) const
{
	return QObject::tr("SCRLC (?=%1)")
			.arg(unknown);
}

QByteArray OpcodeSCRLC::params() const
{
	return QByteArray().append((char *)&unknown, 4);
}

OpcodeSCRLA::OpcodeSCRLA(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSCRLA::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&speed, params + 1, 2); // bank 2
	groupID = params[3];
	scrollType = params[4];
}

QString OpcodeSCRLA::toString(Field *field) const
{
	return QObject::tr("Centrer sur le groupe %2 (vitesse=%1, type=%3)")
			.arg(_var(speed, B2(banks)))
			.arg(_script(groupID, field->scriptsAndTexts()))
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
		vars.append(FF7Var(B2(banks), speed & 0xFF, FF7Var::Word));
}

OpcodeSCR2D::OpcodeSCR2D(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSCR2D::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&targetX, params + 1, 2); // bank 1
	memcpy(&targetY, params + 3, 2); // bank 2
}

QString OpcodeSCR2D::toString(Field *) const
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
		vars.append(FF7Var(B1(banks), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), targetY & 0xFF, FF7Var::SignedWord));
}

OpcodeSCRCC::OpcodeSCRCC()
{
}

QString OpcodeSCRCC::toString(Field *) const
{
	return QObject::tr("Centrer sur le personnage jouable");
}

OpcodeSCR2DC::OpcodeSCR2DC(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSCR2DC::setParams(const char *params, int)
{
	banks[0] = params[0];
	banks[1] = params[1];
	memcpy(&targetX, params + 2, 2); // bank 1
	memcpy(&targetY, params + 4, 2); // bank 2
	memcpy(&speed, params + 6, 2); // bank 4
}

QString OpcodeSCR2DC::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF, FF7Var::SignedWord));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), speed & 0xFF, FF7Var::Word));
}

OpcodeSCRLW::OpcodeSCRLW()
{
}

QString OpcodeSCRLW::toString(Field *) const
{
	return QObject::tr("Attendre la fin du dernier centrage pour continuer");
}

OpcodeSCR2DL::OpcodeSCR2DL(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSCR2DL::setParams(const char *params, int)
{
	banks[0] = params[0];
	banks[1] = params[1];
	memcpy(&targetX, params + 2, 2); // bank 1
	memcpy(&targetY, params + 4, 2); // bank 2
	memcpy(&speed, params + 6, 2); // bank 4
}

QString OpcodeSCR2DL::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF, FF7Var::SignedWord));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), speed & 0xFF, FF7Var::Word));
}

OpcodeMPDSP::OpcodeMPDSP(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMPDSP::setParams(const char *params, int)
{
	unknown = params[0];
}

QString OpcodeMPDSP::toString(Field *) const
{
	return QObject::tr("MPDSP : %1")
			.arg(unknown);
}

QByteArray OpcodeMPDSP::params() const
{
	return QByteArray().append((char)unknown);
}

OpcodeVWOFT::OpcodeVWOFT(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeVWOFT::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&unknown1, params + 1, 2); // bank 1
	memcpy(&unknown2, params + 3, 2); // bank 2
	unknown3 = params[5];
}

QString OpcodeVWOFT::toString(Field *) const
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
		vars.append(FF7Var(B1(banks), unknown1 & 0xFF, FF7Var::Word));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), unknown2 & 0xFF, FF7Var::Word));
}

OpcodeFADE::OpcodeFADE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeFADE::setParams(const char *params, int)
{
	banks[0] = params[0];
	banks[1] = params[1];
	r = params[2]; // bank 1
	g = params[3]; // bank 2
	b = params[4]; // bank 4
	speed = params[5];
	fadeType = params[6];
	adjust = params[7];
}

QString OpcodeFADE::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), r, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), g, FF7Var::Byte));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), b, FF7Var::Byte));
}

OpcodeFADEW::OpcodeFADEW()
{
}

QString OpcodeFADEW::toString(Field *) const
{
	return QObject::tr("Attendre la fin du voilage de l'écran pour continuer");
}

OpcodeIDLCK::OpcodeIDLCK(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeIDLCK::setParams(const char *params, int)
{
	memcpy(&triangleID, params, 2);
	locked = params[2]; // boolean
}

QString OpcodeIDLCK::toString(Field *) const
{
	return QObject::tr("%2 le triangle n°%1")
			.arg(triangleID)
			.arg(locked == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeIDLCK::params() const
{
	return QByteArray()
			.append((char *)&triangleID, 2)
			.append((char)locked);
}

OpcodeLSTMP::OpcodeLSTMP(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeLSTMP::setParams(const char *params, int)
{
	banks = params[0];
	var = params[1];
}

QString OpcodeLSTMP::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), var, FF7Var::Word, true));
}

OpcodeSCRLP::OpcodeSCRLP(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSCRLP::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&speed, params + 1, 2); // bank 2
	partyID = params[3];
	scrollType = params[4];
}

QString OpcodeSCRLP::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), speed & 0xFF, FF7Var::Word));
}

OpcodeBATTLE::OpcodeBATTLE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBATTLE::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&battleID, params + 1, 2); // bank 2
}

QString OpcodeBATTLE::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), battleID & 0xFF, FF7Var::Word));
}

OpcodeBTLON::OpcodeBTLON(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBTLON::setParams(const char *params, int)
{
	disabled = params[0];
}

QString OpcodeBTLON::toString(Field *) const
{
	return QObject::tr("%1 les combats aléatoires")
			.arg(disabled == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeBTLON::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodeBTLMD::OpcodeBTLMD(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBTLMD::setParams(const char *params, int)
{
	memcpy(&battleMode, params, 2);
}

QString OpcodeBTLMD::toString(Field *) const
{
	QStringList modes;
	for(quint8 i=0 ; i<16 ; ++i)
	{
		if((battleMode >> i) & 1)
		{
			switch(i)
			{
			case 1:		modes.append(QObject::tr("Compte à rebours"));							break;
			case 2:		modes.append(QObject::tr("Attaque préventive"));						break;
			case 3:		modes.append(QObject::tr("Impossible de fuir"));						break;
			case 5:		modes.append(QObject::tr("Ne pas jouer Fanfare"));						break;
			case 6:		modes.append(QObject::tr("Active l'arène de combat du Gold Saucer"));	break;
			case 7:		modes.append(QObject::tr("Ne pas afficher d'écran de récompense"));		break;
			case 8:		modes.append(QObject::tr("Désactiver Game Over"));						break;
			default:	modes.append(QString("%1?").arg(i));									break;
			}
		}
	}

	return QObject::tr("Mode de combat : %1").arg(modes.isEmpty() ? QObject::tr("Aucun") : modes.join(", "));
}

QByteArray OpcodeBTLMD::params() const
{
	return QByteArray().append((char *)&battleMode, 2);
}

OpcodePGTDR::OpcodePGTDR(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePGTDR::setParams(const char *params, int)
{
	banks = params[0];
	partyID = params[1];
	varDir = params[2]; // bank 2
}

QString OpcodePGTDR::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), varDir, FF7Var::Byte, true));
}

OpcodeGETPC::OpcodeGETPC(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeGETPC::setParams(const char *params, int)
{
	banks = params[0];
	partyID = params[1];
	varPC = params[2]; // bank 2
}

QString OpcodeGETPC::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), varPC, FF7Var::Byte, true));
}

OpcodePXYZI::OpcodePXYZI(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePXYZI::setParams(const char *params, int)
{
	banks[0] = params[0];
	banks[1] = params[1];
	partyID = params[2];
	varX = params[3]; // bank 1
	varY = params[4]; // bank 2
	varZ = params[5]; // bank 3
	varI = params[6]; // bank 4
}

QString OpcodePXYZI::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), varX, FF7Var::SignedWord, true));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), varY, FF7Var::SignedWord, true));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), varZ, FF7Var::SignedWord, true));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), varI, FF7Var::Word, true));
}

OpcodeBinaryOperation::OpcodeBinaryOperation()
{
}

OpcodeOperation::OpcodeOperation(const char *params, int size)
{
	setParams(params, size);
}

OpcodeOperation::OpcodeOperation(const OpcodeBinaryOperation &op) :
	OpcodeBinaryOperation(op)
{
}

void OpcodeOperation::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	var = (quint8)params[1]; // bank 1
	value = (quint8)params[2]; // bank 2
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
		vars.append(FF7Var(B1(banks), var, FF7Var::Byte, true));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), value & 0xFF));
}

OpcodeOperation2::OpcodeOperation2(const char *params, int size)
{
	setParams(params, size);
}

OpcodeOperation2::OpcodeOperation2(const OpcodeBinaryOperation &op) :
	OpcodeBinaryOperation(op)
{
}

void OpcodeOperation2::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	var = (quint8)params[1];
	value = 0;
	memcpy(&value, params + 2, 2);
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
		vars.append(FF7Var(B1(banks), var, FF7Var::Word, true));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), value & 0xFF, FF7Var::Word));
}

OpcodeUnaryOperation::OpcodeUnaryOperation(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeUnaryOperation::setParams(const char *params, int)
{
	banks = params[0];
	var = params[1]; // bank 2
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
		vars.append(FF7Var(B2(banks), var, FF7Var::Byte, true));
}

OpcodeUnaryOperation2::OpcodeUnaryOperation2(const char *params, int size) :
	OpcodeUnaryOperation(params, size)
{
}

OpcodeUnaryOperation2::OpcodeUnaryOperation2(const OpcodeUnaryOperation &other) :
	OpcodeUnaryOperation(other)
{
}

void OpcodeUnaryOperation2::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), var, FF7Var::Word, true));
}

OpcodePLUSX::OpcodePLUSX(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodePLUSX::OpcodePLUSX(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodePLUSX::toString(Field *) const
{
	return QObject::tr("%1 = %1 + %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodePLUS2X::OpcodePLUS2X(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodePLUS2X::OpcodePLUS2X(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodePLUS2X::toString(Field *) const
{
	return QObject::tr("%1 = %1 + %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMINUSX::OpcodeMINUSX(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodeMINUSX::OpcodeMINUSX(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodeMINUSX::toString(Field *) const
{
	return QObject::tr("%1 = %1 - %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMINUS2X::OpcodeMINUS2X(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodeMINUS2X::OpcodeMINUS2X(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodeMINUS2X::toString(Field *) const
{
	return QObject::tr("%1 = %1 - %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeINCX::OpcodeINCX(const char *params, int size) :
	OpcodeUnaryOperation(params, size)
{
}


OpcodeINCX::OpcodeINCX(const OpcodeUnaryOperation &op) :
	OpcodeUnaryOperation(op)
{
}

QString OpcodeINCX::toString(Field *) const
{
	return QObject::tr("%1 = %1 + 1 (8 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeINC2X::OpcodeINC2X(const char *params, int size) :
	OpcodeUnaryOperation2(params, size)
{
}

OpcodeINC2X::OpcodeINC2X(const OpcodeUnaryOperation &op) :
	OpcodeUnaryOperation2(op)
{
}

QString OpcodeINC2X::toString(Field *) const
{
	return QObject::tr("%1 = %1 + 1 (16 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeDECX::OpcodeDECX(const char *params, int size) :
	OpcodeUnaryOperation(params, size)
{
}

OpcodeDECX::OpcodeDECX(const OpcodeUnaryOperation &op) :
	OpcodeUnaryOperation(op)
{
}

QString OpcodeDECX::toString(Field *) const
{
	return QObject::tr("%1 = %1 - 1 (8 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeDEC2X::OpcodeDEC2X(const char *params, int size) :
	OpcodeUnaryOperation2(params, size)
{
}

OpcodeDEC2X::OpcodeDEC2X(const OpcodeUnaryOperation &op) :
	OpcodeUnaryOperation2(op)
{
}

QString OpcodeDEC2X::toString(Field *) const
{
	return QObject::tr("%1 = %1 - 1 (16 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeTLKON::OpcodeTLKON(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeTLKON::setParams(const char *params, int)
{
	disabled = params[0]; // boolean
}

QString OpcodeTLKON::toString(Field *) const
{
	return QObject::tr("%1 la possibilité de parler à l'objet 3D")
			.arg(disabled == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeTLKON::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodeRDMSD::OpcodeRDMSD(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeRDMSD::setParams(const char *params, int)
{
	banks = params[0];
	value = params[1]; // bank 2
}

QString OpcodeRDMSD::toString(Field *) const
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

OpcodeSETBYTE::OpcodeSETBYTE(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodeSETBYTE::OpcodeSETBYTE(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodeSETBYTE::toString(Field *) const
{
	return QObject::tr("%1 = %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeSETWORD::OpcodeSETWORD(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodeSETWORD::OpcodeSETWORD(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodeSETWORD::toString(Field *) const
{
	return QObject::tr("%1 = %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeBitOperation::OpcodeBitOperation(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBitOperation::setParams(const char *params, int)
{
	banks = params[0];
	var = params[1]; // bank 1
	position = params[2]; // bank 2
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
		vars.append(FF7Var(B1(banks), var, FF7Var::Bit, true));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), position, FF7Var::Byte));
}

OpcodeBITON::OpcodeBITON(const char *params, int size) :
	OpcodeBitOperation(params, size)
{
}

OpcodeBITON::OpcodeBITON(const OpcodeBitOperation &op) :
	OpcodeBitOperation(op)
{
}

QString OpcodeBITON::toString(Field *) const
{
	return QObject::tr("Mettre le bit %2 à 1 dans %1")
			.arg(_bank(var, B1(banks)))
			.arg(_var(position, B2(banks)));
}

OpcodeBITOFF::OpcodeBITOFF(const char *params, int size) :
	OpcodeBitOperation(params, size)
{
}

OpcodeBITOFF::OpcodeBITOFF(const OpcodeBitOperation &op) :
	OpcodeBitOperation(op)
{
}

QString OpcodeBITOFF::toString(Field *) const
{
	return QObject::tr("Mettre le bit %2 à 0 dans %1")
			.arg(_bank(var, B1(banks)))
			.arg(_var(position, B2(banks)));
}

OpcodeBITXOR::OpcodeBITXOR(const char *params, int size) :
	OpcodeBitOperation(params, size)
{
}

OpcodeBITXOR::OpcodeBITXOR(const OpcodeBitOperation &op) :
	OpcodeBitOperation(op)
{
}

QString OpcodeBITXOR::toString(Field *) const
{
	return QObject::tr("Inverser la valeur du bit %2 dans %1")
			.arg(_bank(var, B1(banks)))
			.arg(_var(position, B2(banks)));
}

OpcodePLUS::OpcodePLUS(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodePLUS::OpcodePLUS(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodePLUS::toString(Field *) const
{
	return QObject::tr("%1 = %1 + %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodePLUS2::OpcodePLUS2(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodePLUS2::OpcodePLUS2(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodePLUS2::toString(Field *) const
{
	return QObject::tr("%1 = %1 + %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMINUS::OpcodeMINUS(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodeMINUS::OpcodeMINUS(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodeMINUS::toString(Field *) const
{
	return QObject::tr("%1 = %1 - %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMINUS2::OpcodeMINUS2(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodeMINUS2::OpcodeMINUS2(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodeMINUS2::toString(Field *) const
{
	return QObject::tr("%1 = %1 - %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMUL::OpcodeMUL(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodeMUL::OpcodeMUL(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodeMUL::toString(Field *) const
{
	return QObject::tr("%1 = %1 * %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMUL2::OpcodeMUL2(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodeMUL2::OpcodeMUL2(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodeMUL2::toString(Field *) const
{
	return QObject::tr("%1 = %1 * %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeDIV::OpcodeDIV(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodeDIV::OpcodeDIV(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodeDIV::toString(Field *) const
{
	return QObject::tr("%1 = %1 / %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeDIV2::OpcodeDIV2(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodeDIV2::OpcodeDIV2(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodeDIV2::toString(Field *) const
{
	return QObject::tr("%1 = %1 / %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMOD::OpcodeMOD(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodeMOD::OpcodeMOD(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodeMOD::toString(Field *) const
{
	return QObject::tr("%1 = %1 mod %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeMOD2::OpcodeMOD2(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodeMOD2::OpcodeMOD2(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodeMOD2::toString(Field *) const
{
	return QObject::tr("%1 = %1 mod %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeAND::OpcodeAND(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodeAND::OpcodeAND(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodeAND::toString(Field *) const
{
	return QObject::tr("%1 = %1 & %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeAND2::OpcodeAND2(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodeAND2::OpcodeAND2(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodeAND2::toString(Field *) const
{
	return QObject::tr("%1 = %1 & %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeOR::OpcodeOR(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodeOR::OpcodeOR(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodeOR::toString(Field *) const
{
	return QObject::tr("%1 = %1 | %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeOR2::OpcodeOR2(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodeOR2::OpcodeOR2(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodeOR2::toString(Field *) const
{
	return QObject::tr("%1 = %1 | %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeXOR::OpcodeXOR(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodeXOR::OpcodeXOR(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodeXOR::toString(Field *) const
{
	return QObject::tr("%1 = %1 ^ %2 (8 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeXOR2::OpcodeXOR2(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodeXOR2::OpcodeXOR2(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodeXOR2::toString(Field *) const
{
	return QObject::tr("%1 = %1 ^ %2 (16 bits)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeINC::OpcodeINC(const char *params, int size) :
	OpcodeUnaryOperation(params, size)
{
}

OpcodeINC::OpcodeINC(const OpcodeUnaryOperation &op) :
	OpcodeUnaryOperation(op)
{
}

QString OpcodeINC::toString(Field *) const
{
	return QObject::tr("%1 = %1 + 1 (8 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeINC2::OpcodeINC2(const char *params, int size) :
	OpcodeUnaryOperation2(params, size)
{
}

OpcodeINC2::OpcodeINC2(const OpcodeUnaryOperation &op) :
	OpcodeUnaryOperation2(op)
{
}

QString OpcodeINC2::toString(Field *) const
{
	return QObject::tr("%1 = %1 + 1 (16 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeDEC::OpcodeDEC(const char *params, int size) :
	OpcodeUnaryOperation(params, size)
{
}

OpcodeDEC::OpcodeDEC(const OpcodeUnaryOperation &op) :
	OpcodeUnaryOperation(op)
{
}

QString OpcodeDEC::toString(Field *) const
{
	return QObject::tr("%1 = %1 - 1 (8 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeDEC2::OpcodeDEC2(const char *params, int size) :
	OpcodeUnaryOperation2(params, size)
{
}

OpcodeDEC2::OpcodeDEC2(const OpcodeUnaryOperation &op) :
	OpcodeUnaryOperation2(op)
{
}

QString OpcodeDEC2::toString(Field *) const
{
	return QObject::tr("%1 = %1 - 1 (16 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeRANDOM::OpcodeRANDOM(const char *params, int size) :
	OpcodeUnaryOperation(params, size)
{
}

OpcodeRANDOM::OpcodeRANDOM(const OpcodeUnaryOperation &op) :
	OpcodeUnaryOperation(op)
{
}

QString OpcodeRANDOM::toString(Field *) const
{
	return QObject::tr("Affecter une valeur aléatoire à %1 (8 bits)")
			.arg(_bank(var, B2(banks)));
}

OpcodeLBYTE::OpcodeLBYTE(const char *params, int size) :
	OpcodeOperation(params, size)
{
}

OpcodeLBYTE::OpcodeLBYTE(const OpcodeBinaryOperation &op) :
	OpcodeOperation(op)
{
}

QString OpcodeLBYTE::toString(Field *) const
{
	return QObject::tr("%1 = %2 & 0xFF (low byte)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

OpcodeHBYTE::OpcodeHBYTE(const char *params, int size) :
	OpcodeOperation2(params, size)
{
}

OpcodeHBYTE::OpcodeHBYTE(const OpcodeBinaryOperation &op) :
	OpcodeOperation2(op)
{
}

QString OpcodeHBYTE::toString(Field *) const
{
	return QObject::tr("%1 = (%2 >> 8) & 0xFF (high byte)")
			.arg(_bank(var, B1(banks)))
			.arg(_var(value, B2(banks)));
}

Opcode2BYTE::Opcode2BYTE(const char *params, int size)
{
	setParams(params, size);
}

void Opcode2BYTE::setParams(const char *params, int)
{
	banks[0] = params[0];
	banks[1] = params[1];
	var = params[2]; // bank 1
	value1 = params[3]; // bank 2
	value2 = params[4]; // bank 4
}

QString Opcode2BYTE::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), var, FF7Var::Word, true));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), value1, FF7Var::Byte));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), value2, FF7Var::Byte));
}

OpcodeSETX::OpcodeSETX(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSETX::setParams(const char *params, int)
{
	memcpy(&unknown, params, 6);
}

QString OpcodeSETX::toString(Field *) const
{
	return QObject::tr("SETX %1")
			.arg(QString(QByteArray((char *)&unknown, 6).toHex()));
}

QByteArray OpcodeSETX::params() const
{
	return QByteArray().append((char *)&unknown, 6);
}

OpcodeGETX::OpcodeGETX(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeGETX::setParams(const char *params, int)
{
	memcpy(&unknown, params, 6);
}

QString OpcodeGETX::toString(Field *) const
{
	return QObject::tr("GETX %1")
			.arg(QString(QByteArray((char *)&unknown, 6).toHex()));
}

QByteArray OpcodeGETX::params() const
{
	return QByteArray().append((char *)&unknown, 6);
}

OpcodeSEARCHX::OpcodeSEARCHX(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSEARCHX::setParams(const char *params, int)
{
	memcpy(banks, params, 3);
	searchStart = (quint8)params[3];
	memcpy(&start, params + 4, 2); // bank 2
	memcpy(&end, params + 6, 2); // bank 3
	value = (quint8)params[8]; // bank 4
	varResult = (quint8)params[9]; // bank 6
}

QString OpcodeSEARCHX::toString(Field *) const
{
	return QObject::tr("Rechercher la valeur %5 dans la mémoire (bank=%1, début=%2+%3, fin=%2+%4) et mettre la position dans %6")
			.arg(B1(banks[0]))
			.arg(searchStart)
			.arg(_var(start, B2(banks[0])))
			.arg(_var(end, B1(banks[1])))
			.arg(_var(value, B2(banks[1])))
			.arg(_var(varResult, B2(banks[2])));
}

QByteArray OpcodeSEARCHX::params() const
{
	return QByteArray()
			.append((char *)banks, 3)
			.append((char)searchStart)
			.append((char *)&start, 2)
			.append((char *)&end, 2)
			.append((char)value)
			.append((char)varResult);
}

void OpcodeSEARCHX::getVariables(QList<FF7Var> &vars) const
{
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), start & 0xFF, FF7Var::Word));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), end & 0xFF, FF7Var::Word));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), value, FF7Var::Byte));
	if(B2(banks[2]) != 0)
		vars.append(FF7Var(B2(banks[2]), varResult, FF7Var::Byte));
}

OpcodePC::OpcodePC(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePC::setParams(const char *params, int)
{
	charID = params[0];
}

QString OpcodePC::toString(Field *) const
{
	return QObject::tr("L'objet 3D est jouable et c'est %1")
			.arg(character(charID));
}

QByteArray OpcodePC::params() const
{
	return QByteArray().append((char)charID);
}

OpcodeCHAR::OpcodeCHAR(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCHAR::setParams(const char *params, int)
{
	objectID = params[0];
}

QString OpcodeCHAR::toString(Field *) const
{
	return QObject::tr("Ce groupe est un objet 3D (id=%1)")
			.arg(objectID);
}

QByteArray OpcodeCHAR::params() const
{
	return QByteArray().append((char)objectID);
}

OpcodeDFANM::OpcodeDFANM(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeDFANM::setParams(const char *params, int)
{
	animID = params[0];
	speed = params[1];
}

QString OpcodeDFANM::toString(Field *) const
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

OpcodeANIME1::OpcodeANIME1(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeANIME1::setParams(const char *params, int)
{
	animID = params[0];
	speed = params[1];
}

QString OpcodeANIME1::toString(Field *) const
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

OpcodeVISI::OpcodeVISI(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeVISI::setParams(const char *params, int)
{
	show = params[0];
}

QString OpcodeVISI::toString(Field *) const
{
	return QObject::tr("%1 l'objet 3D")
			.arg(show == 0 ? QObject::tr("Cacher") : QObject::tr("Afficher"));
}

QByteArray OpcodeVISI::params() const
{
	return QByteArray()
			.append((char)show);
}

OpcodeXYZI::OpcodeXYZI(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeXYZI::setParams(const char *params, int)
{
	memcpy(banks, params, 2);
	memcpy(&targetX, params + 2, 2); // bank 1
	memcpy(&targetY, params + 4, 2); // bank 2
	memcpy(&targetZ, params + 6, 2); // bank 3
	memcpy(&targetI, params + 8, 2); // bank 4
}

QString OpcodeXYZI::toString(Field *) const
{
	return QObject::tr("Place l'objet 3D (X=%1, Y=%2, Z=%3, triangle id=%4)")
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
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF, FF7Var::SignedWord));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetZ & 0xFF, FF7Var::SignedWord));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), targetI & 0xFF, FF7Var::Word));
}

void OpcodeXYZI::listModelPositions(QList<FF7Position> &positions) const
{
	FF7Position pos = FF7Position();
	pos.x = targetX;
	pos.y = targetY;
	pos.z = targetZ;
	pos.id = targetI;
	pos.hasId = true;
	pos.hasZ = true;
	positions.append(pos);
}

OpcodeXYI::OpcodeXYI(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeXYI::setParams(const char *params, int)
{
	memcpy(banks, params, 2);
	memcpy(&targetX, params + 2, 2); // bank 1
	memcpy(&targetY, params + 4, 2); // bank 2
	memcpy(&targetI, params + 6, 2); // bank 3
}

QString OpcodeXYI::toString(Field *) const
{
	return QObject::tr("Place l'objet 3D (X=%1, Y=%2, triangle id=%4)")
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
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF, FF7Var::SignedWord));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetI & 0xFF, FF7Var::Word));
}

void OpcodeXYI::listModelPositions(QList<FF7Position> &positions) const
{
	FF7Position pos = FF7Position();
	pos.x = targetX;
	pos.y = targetY;
	pos.id = targetI;
	pos.hasId = true;
	pos.hasZ = false;
	positions.append(pos);
}

OpcodeXYZ::OpcodeXYZ(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeXYZ::setParams(const char *params, int)
{
	memcpy(banks, params, 2);
	memcpy(&targetX, params + 2, 2); // bank 1
	memcpy(&targetY, params + 4, 2); // bank 2
	memcpy(&targetZ, params + 6, 2); // bank 3
}

QString OpcodeXYZ::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF, FF7Var::SignedWord));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetZ & 0xFF, FF7Var::SignedWord));
}

void OpcodeXYZ::listModelPositions(QList<FF7Position> &positions) const
{
	FF7Position pos = FF7Position();
	pos.x = targetX;
	pos.y = targetY;
	pos.z = targetZ;
	pos.hasId = false;
	pos.hasZ = true;
	positions.append(pos);
}

OpcodeMOVE::OpcodeMOVE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMOVE::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&targetX, params + 1, 2); // bank 1
	memcpy(&targetY, params + 3, 2); // bank 2
}

QString OpcodeMOVE::toString(Field *) const
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
		vars.append(FF7Var(B1(banks), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), targetY & 0xFF, FF7Var::SignedWord));
}

OpcodeCMOVE::OpcodeCMOVE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCMOVE::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&targetX, params + 1, 2); // bank 1
	memcpy(&targetY, params + 3, 2); // bank 2
}

QString OpcodeCMOVE::toString(Field *) const
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
		vars.append(FF7Var(B1(banks), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), targetY & 0xFF, FF7Var::SignedWord));
}

OpcodeMOVA::OpcodeMOVA(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMOVA::setParams(const char *params, int)
{
	groupID = params[0];
}

QString OpcodeMOVA::toString(Field *field) const
{
	return QObject::tr("Déplace l'objet 3D vers le groupe %1")
			.arg(_script(groupID, field->scriptsAndTexts()));
}

QByteArray OpcodeMOVA::params() const
{
	return QByteArray()
			.append((char)groupID);
}

OpcodeTURA::OpcodeTURA(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeTURA::setParams(const char *params, int)
{
	groupID = params[0];
	directionRotation = params[1];
	speed = params[2];
}

QString OpcodeTURA::toString(Field *field) const
{
	return QObject::tr("Rotation de l'objet 3D vers le groupe %1 (vitesse=%2, SensRotation=%3)")
			.arg(_script(groupID, field->scriptsAndTexts()))
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

QString OpcodeANIMW::toString(Field *) const
{
	return QObject::tr("Attendre que l'animation soit terminée pour continuer");
}

OpcodeFMOVE::OpcodeFMOVE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeFMOVE::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&targetX, params + 1, 2);
	memcpy(&targetY, params + 3, 2);
}

QString OpcodeFMOVE::toString(Field *) const
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
		vars.append(FF7Var(B1(banks), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), targetY & 0xFF, FF7Var::SignedWord));
}

OpcodeANIME2::OpcodeANIME2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeANIME2::setParams(const char *params, int)
{
	animID = params[0];
	speed = params[1];
}

QString OpcodeANIME2::toString(Field *) const
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

OpcodeANIMX1::OpcodeANIMX1(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeANIMX1::setParams(const char *params, int)
{
	animID = params[0];
	speed = params[1];
}

QString OpcodeANIMX1::toString(Field *) const
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

OpcodeCANIM1::OpcodeCANIM1(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCANIM1::setParams(const char *params, int)
{
	animID = params[0];
	firstFrame = params[1];
	lastFrame = params[2];
	speed = params[3];
}

QString OpcodeCANIM1::toString(Field *) const
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

OpcodeCANMX1::OpcodeCANMX1(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCANMX1::setParams(const char *params, int)
{
	animID = params[0];
	firstFrame = params[1];
	lastFrame = params[2];
	speed = params[3];
}

QString OpcodeCANMX1::toString(Field *) const
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

OpcodeMSPED::OpcodeMSPED(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMSPED::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&speed, params + 1, 2); // bank 2
}

QString OpcodeMSPED::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), speed & 0xFF, FF7Var::Word));
}

OpcodeDIR::OpcodeDIR(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeDIR::setParams(const char *params, int)
{
	banks = params[0];
	direction = params[1]; // bank 2
}

QString OpcodeDIR::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), direction, FF7Var::Byte));
}

OpcodeTURNGEN::OpcodeTURNGEN(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeTURNGEN::setParams(const char *params, int)
{
	banks = params[0];
	direction = params[1]; // bank 2
	turnCount = params[2];
	speed = params[3];
	unknown = params[4];
}

QString OpcodeTURNGEN::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), direction, FF7Var::Byte));
}

OpcodeTURN::OpcodeTURN(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeTURN::setParams(const char *params, int)
{
	banks = params[0];
	direction = params[1]; // bank 2
	turnCount = params[2];
	speed = params[3];
	unknown = params[4];
}

QString OpcodeTURN::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), direction, FF7Var::Byte));
}

OpcodeDIRA::OpcodeDIRA(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeDIRA::setParams(const char *params, int)
{
	groupID = params[0];
}

QString OpcodeDIRA::toString(Field *field) const
{
	return QObject::tr("Mettre l'objet 3D en direction du groupe %1")
			.arg(_script(groupID, field->scriptsAndTexts()));
}

QByteArray OpcodeDIRA::params() const
{
	return QByteArray()
			.append((char)groupID);
}

OpcodeGETDIR::OpcodeGETDIR(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeGETDIR::setParams(const char *params, int)
{
	banks = params[0];
	groupID = params[1];
	varDir = params[2]; // bank 2
}

QString OpcodeGETDIR::toString(Field *field) const
{
	return QObject::tr("Stocker dans %2 la direction du groupe %1")
			.arg(_script(groupID, field->scriptsAndTexts()))
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
		vars.append(FF7Var(B2(banks), varDir, FF7Var::Byte, true));
}

OpcodeGETAXY::OpcodeGETAXY(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeGETAXY::setParams(const char *params, int)
{
	banks = params[0];
	groupID = params[1];
	varX = params[2]; // bank 1
	varY = params[3]; // bank 2
}

QString OpcodeGETAXY::toString(Field *field) const
{
	return QObject::tr("Stocker dans %2 et %3 la position X et Y du groupe %1")
			.arg(_script(groupID, field->scriptsAndTexts()))
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
		vars.append(FF7Var(B1(banks), varX, FF7Var::SignedWord, true));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), varY, FF7Var::SignedWord, true));
}

OpcodeGETAI::OpcodeGETAI(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeGETAI::setParams(const char *params, int)
{
	banks = params[0];
	groupID = params[1];
	varI = params[2]; // bank 2
}

QString OpcodeGETAI::toString(Field *field) const
{
	return QObject::tr("Stocker dans %2 le triangle id du groupe %1")
			.arg(_script(groupID, field->scriptsAndTexts()))
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
		vars.append(FF7Var(B2(banks), varI, FF7Var::Word, true));
}

OpcodeANIMX2::OpcodeANIMX2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeANIMX2::setParams(const char *params, int)
{
	animID = params[0];
	speed = params[1];
}

QString OpcodeANIMX2::toString(Field *) const
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

OpcodeCANIM2::OpcodeCANIM2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCANIM2::setParams(const char *params, int)
{
	animID = params[0];
	firstFrame = params[1];
	lastFrame = params[2];
	speed = params[3];
}

QString OpcodeCANIM2::toString(Field *) const
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

OpcodeCANMX2::OpcodeCANMX2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCANMX2::setParams(const char *params, int)
{
	animID = params[0];
	firstFrame = params[1];
	lastFrame = params[2];
	speed = params[3];
}

QString OpcodeCANMX2::toString(Field *) const
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

OpcodeASPED::OpcodeASPED(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeASPED::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&speed, params + 1, 2); // bank 2
}

QString OpcodeASPED::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), speed & 0xFF, FF7Var::Word));
}

OpcodeCC::OpcodeCC(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCC::setParams(const char *params, int)
{
	groupID = params[0];
}

QString OpcodeCC::toString(Field *field) const
{
	return QObject::tr("Prendre le contrôle du groupe %1")
			.arg(_script(groupID, field->scriptsAndTexts()));
}

QByteArray OpcodeCC::params() const
{
	return QByteArray()
			.append((char)groupID);
}

OpcodeJUMP::OpcodeJUMP(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeJUMP::setParams(const char *params, int)
{
	memcpy(banks, params, 2);
	memcpy(&targetX, params + 2, 2);
	memcpy(&targetY, params + 4, 2);
	memcpy(&targetI, params + 6, 2);
	memcpy(&height, params + 8, 2);
}

QString OpcodeJUMP::toString(Field *) const
{
	return QObject::tr("Faire sauter un personnage (X=%1, Y=%2, triangle id=%3, hauteur=%4)")
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
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF, FF7Var::SignedWord));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetI & 0xFF, FF7Var::Word));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), height & 0xFF, FF7Var::SignedWord));
}

OpcodeAXYZI::OpcodeAXYZI(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeAXYZI::setParams(const char *params, int)
{
	memcpy(banks, params, 2);
	groupID = params[2];
	varX = params[3]; // bank 1
	varY = params[4]; // bank 2
	varZ = params[5]; // bank 3
	varI = params[6]; // bank 4
}

QString OpcodeAXYZI::toString(Field *field) const
{
	return QObject::tr("Stocker la position du groupe %1 dans des variables (%2=X, %3=Y, %4=Z, %5=triangle id)")
			.arg(_script(groupID, field->scriptsAndTexts()))
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
		vars.append(FF7Var(B1(banks[0]), varX, FF7Var::SignedWord, true));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), varY, FF7Var::SignedWord, true));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), varZ, FF7Var::SignedWord, true));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), varI, FF7Var::Word, true));
}

OpcodeLADER::OpcodeLADER(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeLADER::setParams(const char *params, int)
{
	memcpy(banks, params, 2);
	memcpy(&targetX, params + 2, 2); // bank 1
	memcpy(&targetY, params + 4, 2); // bank 2
	memcpy(&targetZ, params + 6, 2); // bank 3
	memcpy(&targetI, params + 8, 2); // bank 4
	way = params[10];
	animID = params[11];
	direction = params[12];
	speed = params[13];
}

QString OpcodeLADER::toString(Field *) const
{
	return QObject::tr("Monter une échelle avec l'animation %6 (X=%1, Y=%2, Z=%3, triangle id=%4, sens=%5, direction=%7, vitesse=%8)")
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
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF, FF7Var::SignedWord));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetZ & 0xFF, FF7Var::SignedWord));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), targetI & 0xFF, FF7Var::Word));
}

OpcodeOFST::OpcodeOFST(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeOFST::setParams(const char *params, int)
{
	memcpy(banks, params, 2);
	moveType = params[2];
	memcpy(&targetX, params + 3, 2); // bank 1
	memcpy(&targetY, params + 5, 2); // bank 2
	memcpy(&targetZ, params + 7, 2); // bank 3
	memcpy(&speed, params + 9, 2); // bank 4
}

QString OpcodeOFST::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), targetX & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY & 0xFF, FF7Var::SignedWord));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetZ & 0xFF, FF7Var::SignedWord));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), speed & 0xFF, FF7Var::Word));
}

OpcodeOFSTW::OpcodeOFSTW()
{
}

QString OpcodeOFSTW::toString(Field *) const
{
	return QObject::tr("Attendre la fin de l'exécution de l'Offset Object pour continuer");
}

OpcodeTALKR::OpcodeTALKR(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeTALKR::setParams(const char *params, int)
{
	banks = params[0];
	distance = params[1]; // bank 2
}

QString OpcodeTALKR::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), distance, FF7Var::Byte));
}

OpcodeSLIDR::OpcodeSLIDR(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSLIDR::setParams(const char *params, int)
{
	banks = params[0];
	distance = params[1]; // bank 2
}

QString OpcodeSLIDR::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), distance, FF7Var::Byte));
}

OpcodeSOLID::OpcodeSOLID(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSOLID::setParams(const char *params, int)
{
	disabled = params[0];
}

QString OpcodeSOLID::toString(Field *) const
{
	return QObject::tr("%1 la possibilité de toucher l'objet 3D")
			.arg(disabled == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeSOLID::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodePRTYP::OpcodePRTYP(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePRTYP::setParams(const char *params, int)
{
	charID = params[0];
}

QString OpcodePRTYP::toString(Field *) const
{
	return QObject::tr("Ajouter %1 à l'équipe actuelle")
			.arg(character(charID));
}

QByteArray OpcodePRTYP::params() const
{
	return QByteArray().append((char)charID);
}

OpcodePRTYM::OpcodePRTYM(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePRTYM::setParams(const char *params, int)
{
	charID = params[0];
}

QString OpcodePRTYM::toString(Field *) const
{
	return QObject::tr("Retirer %1 de l'équipe actuelle")
			.arg(character(charID));
}

QByteArray OpcodePRTYM::params() const
{
	return QByteArray().append((char)charID);
}

OpcodePRTYE::OpcodePRTYE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePRTYE::setParams(const char *params, int)
{
	memcpy(charID, params, 3);
}

QString OpcodePRTYE::toString(Field *) const
{
	return QObject::tr("Nouvelle équipe : %1 | %2 | %3")
			.arg(character(charID[0]))
			.arg(character(charID[1]))
			.arg(character(charID[2]));
}

QByteArray OpcodePRTYE::params() const
{
	return QByteArray().append((char *)&charID, 3);
}

OpcodeIfQ::OpcodeIfQ(const char *params, int size) :
	OpcodeJump()
{
	setParams(params, size);
}

void OpcodeIfQ::setParams(const char *params, int)
{
	charID = params[0];
	_jump = (quint8)params[1] + jumpPosData();
}

QByteArray OpcodeIfQ::params() const
{
	return QByteArray()
			.append((char)charID)
			.append(char(_jump - jumpPosData()));
}

OpcodeIFPRTYQ::OpcodeIFPRTYQ(const char *params, int size) :
	OpcodeIfQ(params, size)
{
}

QString OpcodeIFPRTYQ::toString(Field *) const
{
	return QObject::tr("Si %1 est dans l'équipe actuelle (%2)")
			.arg(character(charID))
			.arg(_badJump
				 ? QObject::tr("avancer de %n octet(s) sinon", "With plural", _jump)
				 : QObject::tr("aller au label %1 sinon").arg(_label));
}

OpcodeIFMEMBQ::OpcodeIFMEMBQ(const char *params, int size) :
	OpcodeIfQ(params, size)
{
}

QString OpcodeIFMEMBQ::toString(Field *) const
{
	return QObject::tr("Si %1 existe (%2)")
			.arg(character(charID))
			.arg(_badJump
				 ? QObject::tr("avancer de %n octet(s) sinon", "With plural", _jump)
				 : QObject::tr("aller au label %1 sinon").arg(_label));
}

OpcodeMMBUD::OpcodeMMBUD(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMMBUD::setParams(const char *params, int)
{
	exists = params[0]; // boolean
	charID = params[1];
}

QString OpcodeMMBUD::toString(Field *) const
{
	return QObject::tr("%2 %1")
			.arg(exists == 0 ? QObject::tr("n'existe plus") : QObject::tr("existe"))
			.arg(character(charID));
}

QByteArray OpcodeMMBUD::params() const
{
	return QByteArray()
			.append((char)exists)
			.append((char)charID);
}

OpcodeMMBLK::OpcodeMMBLK(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMMBLK::setParams(const char *params, int)
{
	charID = params[0];
}

QString OpcodeMMBLK::toString(Field *) const
{
	return QObject::tr("Bloque %1 dans le menu PHS")
			.arg(character(charID));
}

QByteArray OpcodeMMBLK::params() const
{
	return QByteArray().append((char)charID);
}

OpcodeMMBUK::OpcodeMMBUK(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMMBUK::setParams(const char *params, int)
{
	charID = params[0];
}

QString OpcodeMMBUK::toString(Field *) const
{
	return QObject::tr("Débloque %1 dans le menu PHS")
			.arg(character(charID));
}

QByteArray OpcodeMMBUK::params() const
{
	return QByteArray().append((char)charID);
}

OpcodeLINE::OpcodeLINE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeLINE::setParams(const char *params, int)
{
	memcpy(&targetX1, params, 2);
	memcpy(&targetY1, params + 2, 2);
	memcpy(&targetZ1, params + 4, 2);
	memcpy(&targetX2, params + 6, 2);
	memcpy(&targetY2, params + 8, 2);
	memcpy(&targetZ2, params + 10, 2);
}

QString OpcodeLINE::toString(Field *) const
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

bool OpcodeLINE::linePosition(FF7Position position[2]) const
{
	position[0].x = targetX1;
	position[0].y = targetY1;
	position[0].z = targetZ1;
	position[1].x = targetX2;
	position[1].y = targetY2;
	position[1].z = targetZ2;
	position[0].hasZ = true;
	position[0].hasId = false;
	position[1].hasZ = true;
	position[1].hasId = false;

	return true;
}

OpcodeLINON::OpcodeLINON(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeLINON::setParams(const char *params, int)
{
	enabled = params[0];
}

QString OpcodeLINON::toString(Field *) const
{
	return QObject::tr("%1 la zone")
			.arg(enabled == 0 ? QObject::tr("Effacer") : QObject::tr("Tracer"));
}

QByteArray OpcodeLINON::params() const
{
	return QByteArray().append((char)enabled);
}

OpcodeMPJPO::OpcodeMPJPO(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMPJPO::setParams(const char *params, int)
{
	prevent = params[0];
}

QString OpcodeMPJPO::toString(Field *) const
{
	return QObject::tr("%1 les changements de décor par le joueur")
			.arg(prevent == 0 ? QObject::tr("Autoriser") : QObject::tr("Empêcher"));
}

QByteArray OpcodeMPJPO::params() const
{
	return QByteArray().append((char)prevent);
}

OpcodeSLINE::OpcodeSLINE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSLINE::setParams(const char *params, int)
{
	memcpy(banks, params, 3);
	memcpy(&targetX1, params + 3, 2); // bank 1
	memcpy(&targetY1, params + 5, 2); // bank 2
	memcpy(&targetZ1, params + 7, 2); // bank 3
	memcpy(&targetX2, params + 9, 2); // bank 4
	memcpy(&targetY2, params + 11, 2); // bank 5
	memcpy(&targetZ2, params + 13, 2); // bank 6
}

QString OpcodeSLINE::toString(Field *) const
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
		vars.append(FF7Var(B1(banks[0]), targetX1 & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), targetY1 & 0xFF, FF7Var::SignedWord));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), targetZ1 & 0xFF, FF7Var::SignedWord));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), targetX2 & 0xFF, FF7Var::SignedWord));
	if(B1(banks[2]) != 0)
		vars.append(FF7Var(B1(banks[2]), targetY2 & 0xFF, FF7Var::SignedWord));
	if(B2(banks[2]) != 0)
		vars.append(FF7Var(B2(banks[2]), targetZ2 & 0xFF, FF7Var::SignedWord));
}

OpcodeSIN::OpcodeSIN(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSIN::setParams(const char *params, int)
{
	memcpy(banks, params, 2);
	memcpy(&value1, params + 2, 2); // bank 1
	memcpy(&value2, params + 4, 2); // bank 2
	memcpy(&value3, params + 6, 2); // bank 3
	var = params[8]; // bank 4
}

QString OpcodeSIN::toString(Field *) const
{
	return QObject::tr("%4 = ((Sinus(%1) * %2) + %3) >> 12")
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
		vars.append(FF7Var(B1(banks[0]), value1 & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), value2 & 0xFF, FF7Var::SignedWord)); // FIXME: signed word?
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), value3 & 0xFF, FF7Var::SignedWord)); // FIXME: signed word?
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), var, FF7Var::SignedWord, true));
}

OpcodeCOS::OpcodeCOS(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCOS::setParams(const char *params, int)
{
	memcpy(banks, params, 2);
	memcpy(&value1, params + 2, 2); // bank 1
	memcpy(&value2, params + 4, 2); // bank 2
	memcpy(&value3, params + 6, 2); // bank 3
	var = params[8]; // bank 4
}

QString OpcodeCOS::toString(Field *) const
{
	return QObject::tr("%4 = ((Cosinus(%1) * %2) + %3) >> 12")
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
		vars.append(FF7Var(B1(banks[0]), value1 & 0xFF, FF7Var::SignedWord));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), value2 & 0xFF, FF7Var::SignedWord)); // FIXME: signed word?
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), value3 & 0xFF, FF7Var::SignedWord)); // FIXME: signed word?
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), var, FF7Var::SignedWord, true));
}

OpcodeTLKR2::OpcodeTLKR2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeTLKR2::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&distance, params + 1, 2); // bank 2
}

QString OpcodeTLKR2::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), distance & 0xFF, FF7Var::Word));
}

OpcodeSLDR2::OpcodeSLDR2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSLDR2::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&distance, params + 1, 2); // bank 2
}

QString OpcodeSLDR2::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), distance & 0xFF, FF7Var::Word));
}

OpcodePMJMP::OpcodePMJMP(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePMJMP::setParams(const char *params, int)
{
	memcpy(&fieldID, params, 2);
}

QString OpcodePMJMP::toString(Field *) const
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

QString OpcodePMJMP2::toString(Field *) const
{
	return QObject::tr("PMJMP2");
}

OpcodeAKAO2::OpcodeAKAO2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeAKAO2::setParams(const char *params, int)
{
	memcpy(banks, params, 3);
	opcode = (quint8)params[3];
	memcpy(&param1, params + 4, 2); // bank 1
	memcpy(&param2, params + 6, 2); // bank 2
	memcpy(&param3, params + 8, 2); // bank 3
	memcpy(&param4, params + 10, 2); // bank 4
	memcpy(&param5, params + 12, 2); // bank 6
}

QString OpcodeAKAO2::toString(Field *) const
{
	return QObject::tr("%1 (param1=%2, param2=%3, param3=%4, param4=%5, param5=%6)")
			.arg(_akao(opcode))
			.arg(_var(param1, B1(banks[0])))
			.arg(_var(param2, B2(banks[0])))
			.arg(_var(param3, B1(banks[1])))
			.arg(_var(param4, B2(banks[1])))
			.arg(_var(param5, B2(banks[2])));
}

QByteArray OpcodeAKAO2::params() const
{
	return QByteArray()
			.append((char *)&banks, 3)
			.append((char)opcode)
			.append((char *)&param1, 2)
			.append((char *)&param2, 2)
			.append((char *)&param3, 2)
			.append((char *)&param4, 2)
			.append((char *)&param5, 2);
}

void OpcodeAKAO2::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), param1 & 0xFF, FF7Var::Word));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), param2 & 0xFF, FF7Var::Word));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), param3 & 0xFF, FF7Var::Word));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), param4 & 0xFF, FF7Var::Word));
	if(B2(banks[2]) != 0)
		vars.append(FF7Var(B2(banks[2]), param5 & 0xFF, FF7Var::Word));
}

OpcodeFCFIX::OpcodeFCFIX(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeFCFIX::setParams(const char *params, int)
{
	disabled = params[0];
}

QString OpcodeFCFIX::toString(Field *) const
{
	return QObject::tr("%1 rotation")
			.arg(disabled == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));
}

QByteArray OpcodeFCFIX::params() const
{
	return QByteArray().append((char)disabled);
}

OpcodeCCANM::OpcodeCCANM(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCCANM::setParams(const char *params, int)
{
	animID = params[0];
	speed = params[1];
	standWalkRun = params[2];
}

QString OpcodeCCANM::toString(Field *) const
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

QString OpcodeANIMB::toString(Field *) const
{
	return QObject::tr("Stoppe l'animation de l'objet 3D");
}

OpcodeTURNW::OpcodeTURNW()
{
}

QString OpcodeTURNW::toString(Field *) const
{
	return QObject::tr("Attendre que la rotation soit terminée pour continuer");
}

OpcodeMPPAL::OpcodeMPPAL(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMPPAL::setParams(const char *params, int)
{
	memcpy(banks, params, 3);
	posSrc = (quint8)params[3];
	posDst = (quint8)params[4];
	start = (quint8)params[5]; // bank 1
	b = (quint8)params[6]; // bank 2
	g = (quint8)params[7]; // bank 3
	r = (quint8)params[8]; // bank 4
	colorCount = (quint8)params[9]; // bank 6
}

QString OpcodeMPPAL::toString(Field *) const
{
	return QObject::tr("Multiplier RVB(%6, %5, %4) sur les couleurs d'une palette (sourcePal=%1, ciblePal=%2, première couleur=%3, nombre de couleurs=%7+1)")
			.arg(posSrc)
			.arg(posDst)
			.arg(_var(start, B1(banks[0])))
			.arg(_var(b, B2(banks[0])))
			.arg(_var(g, B1(banks[1])))
			.arg(_var(r, B2(banks[1])))
			.arg(_var(colorCount, B2(banks[2])));
}

QByteArray OpcodeMPPAL::params() const
{
	return QByteArray()
			.append((char *)banks, 3)
			.append(char(posSrc))
			.append(char(posDst))
			.append(char(start))
			.append(char(b))
			.append(char(g))
			.append(char(r))
			.append(char(colorCount));
}

void OpcodeMPPAL::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), start, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), b, FF7Var::Byte));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), g, FF7Var::Byte));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), r, FF7Var::Byte));
	if(B2(banks[2]) != 0)
		vars.append(FF7Var(B2(banks[2]), colorCount, FF7Var::Byte));
}

OpcodeBGON::OpcodeBGON(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBGON::setParams(const char *params, int)
{
	banks = params[0];
	paramID = params[1]; // bank 1
	stateID = params[2]; // bank 2
}

QString OpcodeBGON::toString(Field *) const
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
		vars.append(FF7Var(B1(banks), paramID, FF7Var::Byte));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), stateID, FF7Var::Byte));
}

OpcodeBGOFF::OpcodeBGOFF(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBGOFF::setParams(const char *params, int)
{
	banks = params[0];
	paramID = params[1]; // bank 1
	stateID = params[2]; // bank 2
}

QString OpcodeBGOFF::toString(Field *) const
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
		vars.append(FF7Var(B1(banks), paramID, FF7Var::Byte));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), stateID, FF7Var::Byte));
}

OpcodeBGROL::OpcodeBGROL(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBGROL::setParams(const char *params, int)
{
	banks = params[0];
	paramID = params[1]; // bank 2
}

QString OpcodeBGROL::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), paramID, FF7Var::Byte));
}

OpcodeBGROL2::OpcodeBGROL2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBGROL2::setParams(const char *params, int)
{
	banks = params[0];
	paramID = params[1]; // bank 2
}

QString OpcodeBGROL2::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), paramID, FF7Var::Byte));
}

OpcodeBGCLR::OpcodeBGCLR(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBGCLR::setParams(const char *params, int)
{
	banks = params[0];
	paramID = params[1]; // bank 2
}

QString OpcodeBGCLR::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), paramID, FF7Var::Byte));
}

OpcodeSTPAL::OpcodeSTPAL(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSTPAL::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	palID = (quint8)params[1]; // bank 1
	position = (quint8)params[2]; // bank 2
	colorCount = (quint8)params[3];
}

QString OpcodeSTPAL::toString(Field *) const
{
	return QObject::tr("Charger la palette n°%1 à la position %2 (nombre de couleurs=%3)")
			.arg(_var(palID, B1(banks)))
			.arg(_var(position, B2(banks)))
			.arg(colorCount+1);
}

QByteArray OpcodeSTPAL::params() const
{
	return QByteArray()
			.append(char(banks))
			.append(char(palID))
			.append(char(position))
			.append(char(colorCount));
}

void OpcodeSTPAL::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), palID, FF7Var::Byte));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), position, FF7Var::Byte));
}

OpcodeLDPAL::OpcodeLDPAL(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeLDPAL::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	position = (quint8)params[1]; // bank 1
	palID = (quint8)params[2]; // bank 2
	colorCount = (quint8)params[3];
}

QString OpcodeLDPAL::toString(Field *) const
{
	return QObject::tr("Charger la position %1 dans la palette n°%2 (nombre de couleurs=%3)")
			.arg(_var(position, B1(banks)))
			.arg(_var(palID, B2(banks)))
			.arg(colorCount+1);
}

QByteArray OpcodeLDPAL::params() const
{
	return QByteArray()
			.append(char(banks))
			.append(char(position))
			.append(char(palID))
			.append(char(colorCount));
}

void OpcodeLDPAL::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), position, FF7Var::Byte));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), palID, FF7Var::Byte));
}

OpcodeCPPAL::OpcodeCPPAL(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCPPAL::setParams(const char *params, int)
{
	banks = (quint8)params[0];
	posSrc = (quint8)params[1]; // bank 1
	posDst = (quint8)params[2]; // bank 2
	colorCount = (quint8)params[3];
}

QString OpcodeCPPAL::toString(Field *) const
{
	return QObject::tr("Copier palette (sourcePal=%1, ciblePal=%2, nombre de couleurs=%3)")
			.arg(_var(posSrc, B1(banks)))
			.arg(_var(posDst, B2(banks)))
			.arg(colorCount+1);
}

QByteArray OpcodeCPPAL::params() const
{
	return QByteArray()
			.append(char(banks))
			.append(char(posSrc))
			.append(char(posDst))
			.append(char(colorCount));
}

void OpcodeCPPAL::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), posSrc, FF7Var::Byte));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), posDst, FF7Var::Byte));
}

OpcodeRTPAL::OpcodeRTPAL(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeRTPAL::setParams(const char *params, int)
{
	memcpy(banks, params, 2);
	posSrc = (quint8)params[2]; // bank 1
	posDst = (quint8)params[3]; // bank 2
	start = (quint8)params[4]; // bank 4
	end = (quint8)params[5];
}

QString OpcodeRTPAL::toString(Field *) const
{
	return QObject::tr("Copier un morceau de palette (sourcePal=%1, ciblePal=%2, première couleur=%3, nombre de couleurs=%4)")
			.arg(_var(posSrc, B1(banks[0])))
			.arg(_var(posDst, B2(banks[0])))
			.arg(_var(start, B2(banks[1])))
			.arg(end + 1);
}

QByteArray OpcodeRTPAL::params() const
{
	return QByteArray()
			.append((char *)&banks, 2)
			.append(char(posSrc))
			.append(char(posDst))
			.append(char(start))
			.append(char(end));
}

void OpcodeRTPAL::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), posSrc, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), posDst, FF7Var::Byte));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), start, FF7Var::Byte));
}

OpcodeADPAL::OpcodeADPAL(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeADPAL::setParams(const char *params, int)
{
	memcpy(&banks, params, 3);
	posSrc = (quint8)params[3]; // bank 1
	posDst = (quint8)params[4]; // bank 2
	b = (qint8)params[5]; // bank 3
	g = (qint8)params[6]; // bank 4
	r = (qint8)params[7]; // bank 5
	colorCount = (quint8)params[8];
}

QString OpcodeADPAL::toString(Field *) const
{
	return QObject::tr("Additionner RVB(%5, %4, %3) sur les couleurs d'une palette (sourcePal=%1, ciblePal=%2, nombre de couleurs=%6)")
			.arg(_var(posSrc, B1(banks[0])))
			.arg(_var(posDst, B2(banks[0])))
			.arg(_var(b, B1(banks[1])))
			.arg(_var(g, B2(banks[1])))
			.arg(_var(r, B1(banks[2])))
			.arg(colorCount+1);
}

QByteArray OpcodeADPAL::params() const
{
	return QByteArray()
			.append((char *)&banks, 3)
			.append(char(posSrc))
			.append(char(posDst))
			.append(char(b))
			.append(char(g))
			.append(char(r))
			.append(char(colorCount));
}

void OpcodeADPAL::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), posSrc, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), posDst, FF7Var::Byte));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), b, FF7Var::Byte));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), g, FF7Var::Byte));
	if(B1(banks[2]) != 0)
		vars.append(FF7Var(B1(banks[2]), r, FF7Var::Byte));
}

OpcodeMPPAL2::OpcodeMPPAL2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMPPAL2::setParams(const char *params, int)
{
	memcpy(&banks, params, 3);
	posSrc = (quint8)params[3]; // bank 1
	posDst = (quint8)params[4]; // bank 2
	b = (quint8)params[5]; // bank 3
	g = (quint8)params[6]; // bank 4
	r = (quint8)params[7]; // bank 5
	colorCount = (quint8)params[8];
}

QString OpcodeMPPAL2::toString(Field *) const
{
	return QObject::tr("Multiplier RVB(%5, %4, %3) sur les couleurs d'une palette (sourcePal=%1, ciblePal=%2, nombre de couleurs=%6)")
			.arg(_var(posSrc, B1(banks[0])))
			.arg(_var(posDst, B2(banks[0])))
			.arg(_var(b, B1(banks[1])))
			.arg(_var(g, B2(banks[1])))
			.arg(_var(r, B1(banks[2])))
			.arg(colorCount+1);
}

QByteArray OpcodeMPPAL2::params() const
{
	return QByteArray()
			.append((char *)&banks, 3)
			.append(char(posSrc))
			.append(char(posDst))
			.append(char(b))
			.append(char(g))
			.append(char(r))
			.append(char(colorCount));
}

void OpcodeMPPAL2::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), posSrc, FF7Var::Byte));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), posDst, FF7Var::Byte));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), b, FF7Var::Byte));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), g, FF7Var::Byte));
	if(B1(banks[2]) != 0)
		vars.append(FF7Var(B1(banks[2]), r, FF7Var::Byte));
}

OpcodeSTPLS::OpcodeSTPLS(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSTPLS::setParams(const char *params, int)
{
	palID = (quint8)params[0];
	posSrc = (quint8)params[1];
	start = (quint8)params[2];
	colorCount = (quint8)params[3];
}

QString OpcodeSTPLS::toString(Field *) const
{
	return QObject::tr("Charger la palette n°%1 à la position %2 (première couleur=%3, nombre de couleurs=%4)")
			.arg(palID)
			.arg(posSrc)
			.arg(start)
			.arg(colorCount+1);
}

QByteArray OpcodeSTPLS::params() const
{
	return QByteArray()
			.append(char(palID))
			.append(char(posSrc))
			.append(char(start))
			.append(char(colorCount));
}

OpcodeLDPLS::OpcodeLDPLS(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeLDPLS::setParams(const char *params, int)
{
	posSrc = (quint8)params[0];
	palID = (quint8)params[1];
	start = (quint8)params[2];
	colorCount = (quint8)params[3];
}

QString OpcodeLDPLS::toString(Field *) const
{
	return QObject::tr("Charger la position %1 dans la palette n°%2 (première couleur=%3, nombre de couleurs=%4)")
			.arg(posSrc)
			.arg(palID)
			.arg(start)
			.arg(colorCount+1);
}

QByteArray OpcodeLDPLS::params() const
{
	return QByteArray()
			.append(char(posSrc))
			.append(char(palID))
			.append(char(start))
			.append(char(colorCount));
}

OpcodeCPPAL2::OpcodeCPPAL2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCPPAL2::setParams(const char *params, int)
{
	memcpy(unknown, params, 7);
}

QString OpcodeCPPAL2::toString(Field *) const
{
	return QObject::tr("CPPAL2 %1")
			.arg(QString(QByteArray((char *)&unknown, 7).toHex()));
}

QByteArray OpcodeCPPAL2::params() const
{
	return QByteArray().append((char *)&unknown, 7);
}

OpcodeRTPAL2::OpcodeRTPAL2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeRTPAL2::setParams(const char *params, int)
{
	memcpy(unknown, params, 7);
}

QString OpcodeRTPAL2::toString(Field *) const
{
	return QObject::tr("RTPAL2 %1")
			.arg(QString(QByteArray((char *)&unknown, 7).toHex()));
}

QByteArray OpcodeRTPAL2::params() const
{
	return QByteArray().append((char *)&unknown, 7);
}

OpcodeADPAL2::OpcodeADPAL2(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeADPAL2::setParams(const char *params, int)
{
	memcpy(unknown, params, 10);
}

QString OpcodeADPAL2::toString(Field *) const
{
	return QObject::tr("ADPAL2 %1")
			.arg(QString(QByteArray((char *)&unknown, 10).toHex()));
}

QByteArray OpcodeADPAL2::params() const
{
	return QByteArray().append((char *)&unknown, 10);
}

OpcodeMUSIC::OpcodeMUSIC(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMUSIC::setParams(const char *params, int)
{
	musicID = params[0];
}

QString OpcodeMUSIC::toString(Field *) const
{
	return QObject::tr("Jouer musique n°%1")
			.arg(musicID);
}

QByteArray OpcodeMUSIC::params() const
{
	return QByteArray().append((char)musicID);
}

OpcodeSOUND::OpcodeSOUND(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeSOUND::setParams(const char *params, int)
{
	banks = params[0];
	memcpy(&soundID, params + 1, 2); // bank 1
	position = params[3]; // bank 2
}

QString OpcodeSOUND::toString(Field *) const
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
		vars.append(FF7Var(B1(banks), soundID & 0xFF, FF7Var::Word));
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), position, FF7Var::Byte));
}

OpcodeAKAO::OpcodeAKAO(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeAKAO::setParams(const char *params, int)
{
	memcpy(banks, params, 3);
	opcode = (quint8)params[3];
	param1 = (quint8)params[4]; // bank 1
	memcpy(&param2, params + 5, 2); // bank 2
	memcpy(&param3, params + 7, 2); // bank 3
	memcpy(&param4, params + 9, 2); // bank 4
	memcpy(&param5, params + 11, 2); // bank 6
}

QString OpcodeAKAO::toString(Field *) const
{
	return QObject::tr("%1 (param1=%2, param2=%3, param3=%4, param4=%5, param5=%6)")
			.arg(_akao(opcode))
			.arg(_var(param1, B1(banks[0])))
			.arg(_var(param2, B2(banks[0])))
			.arg(_var(param3, B1(banks[1])))
			.arg(_var(param4, B2(banks[1])))
			.arg(_var(param5, B2(banks[2])));
}

QByteArray OpcodeAKAO::params() const
{
	return QByteArray()
			.append((char *)&banks, 3)
			.append((char)opcode)
			.append((char)param1)
			.append((char *)&param2, 2)
			.append((char *)&param3, 2)
			.append((char *)&param4, 2)
			.append((char *)&param5, 2);
}

void OpcodeAKAO::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks[0]) != 0)
		vars.append(FF7Var(B1(banks[0]), param1 & 0xFF, FF7Var::Word));
	if(B2(banks[0]) != 0)
		vars.append(FF7Var(B2(banks[0]), param2 & 0xFF, FF7Var::Word));
	if(B1(banks[1]) != 0)
		vars.append(FF7Var(B1(banks[1]), param3 & 0xFF, FF7Var::Word));
	if(B2(banks[1]) != 0)
		vars.append(FF7Var(B2(banks[1]), param4 & 0xFF, FF7Var::Word));
	if(B2(banks[2]) != 0)
		vars.append(FF7Var(B2(banks[2]), param5 & 0xFF, FF7Var::Word));
}

OpcodeMUSVT::OpcodeMUSVT(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMUSVT::setParams(const char *params, int)
{
	musicID = params[0];
}

QString OpcodeMUSVT::toString(Field *) const
{
	return QObject::tr("MUSVT (musique n°%1)")
			.arg(musicID);
}

QByteArray OpcodeMUSVT::params() const
{
	return QByteArray().append((char)musicID);
}

OpcodeMUSVM::OpcodeMUSVM(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMUSVM::setParams(const char *params, int)
{
	musicID = params[0];
}

QString OpcodeMUSVM::toString(Field *) const
{
	return QObject::tr("MUSVM (musique n°%1)")
			.arg(musicID);
}

QByteArray OpcodeMUSVM::params() const
{
	return QByteArray().append((char)musicID);
}

OpcodeMULCK::OpcodeMULCK(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMULCK::setParams(const char *params, int)
{
	locked = params[0];
}

QString OpcodeMULCK::toString(Field *) const
{
	return QObject::tr("%1 musique")
			.arg(locked == 0 ? QObject::tr("Déverrouiller") : QObject::tr("Verrouiller", "test"));
}

QByteArray OpcodeMULCK::params() const
{
	return QByteArray().append((char)locked);
}

OpcodeBMUSC::OpcodeBMUSC(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeBMUSC::setParams(const char *params, int)
{
	musicID = params[0];
}

QString OpcodeBMUSC::toString(Field *) const
{
	return QObject::tr("Choisir musique n°%1 comme musique de combat")
			.arg(musicID);
}

QByteArray OpcodeBMUSC::params() const
{
	return QByteArray().append((char)musicID);
}

OpcodeCHMPH::OpcodeCHMPH(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCHMPH::setParams(const char *params, int)
{
	banks = params[0];
	var1 = params[1];
	var2 = params[2];
}

QString OpcodeCHMPH::toString(Field *) const
{
	return QObject::tr("CHMPH : Sauvegarder (inconnu) dans %1 et (inconnu) dans %2")
			.arg(_bank(var1, B1(banks)))
			.arg(_bank(var2, B2(banks)));
}

QByteArray OpcodeCHMPH::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)var1)
			.append((char)var2);
}

void OpcodeCHMPH::getVariables(QList<FF7Var> &vars) const
{
	if(B1(banks) != 0)
		vars.append(FF7Var(B1(banks), var1, FF7Var::Byte, true)); // FIXME: byte?
	if(B2(banks) != 0)
		vars.append(FF7Var(B2(banks), var2, FF7Var::Byte, true)); // FIXME: byte?
}

OpcodePMVIE::OpcodePMVIE(const char *params, int size)
{
	setParams(params, size);
}

void OpcodePMVIE::setParams(const char *params, int)
{
	movieID = params[0];
}

QString OpcodePMVIE::toString(Field *) const
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

QString OpcodeMOVIE::toString(Field *) const
{
	return QObject::tr("Jouer la cinématique choisie");
}

OpcodeMVIEF::OpcodeMVIEF(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMVIEF::setParams(const char *params, int)
{
	banks = params[0];
	varCurMovieFrame = params[1];
}

QString OpcodeMVIEF::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), varCurMovieFrame, FF7Var::Word, true));
}

OpcodeMVCAM::OpcodeMVCAM(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeMVCAM::setParams(const char *params, int)
{
	movieCamID = params[0];
}

QString OpcodeMVCAM::toString(Field *) const
{
	return QObject::tr("Camera Movie : %1")
			.arg(movieCamID);
}

QByteArray OpcodeMVCAM::params() const
{
	return QByteArray().append((char)movieCamID);
}

OpcodeFMUSC::OpcodeFMUSC(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeFMUSC::setParams(const char *params, int)
{
	unknown = params[0];
}

QString OpcodeFMUSC::toString(Field *) const
{
	return QObject::tr("FMUSC (?=%1)")
			.arg(unknown);
}

QByteArray OpcodeFMUSC::params() const
{
	return QByteArray().append((char)unknown);
}

OpcodeCMUSC::OpcodeCMUSC(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCMUSC::setParams(const char *params, int)
{
	musicID = params[0];
	unknown2 = params[1];
	unknown3 = params[2];
	unknown4 = params[3];
	unknown5 = params[4];
	unknown6 = params[5];
	unknown7 = params[6];
}

QString OpcodeCMUSC::toString(Field *) const
{
	return QObject::tr("CMUSC (musique n°%1, inconnu1=%2, inconnu2=%3, inconnu2=%4, inconnu4=%5, inconnu5=%6, inconnu6=%7)")
			.arg(musicID)
			.arg(unknown2)
			.arg(unknown3)
			.arg(unknown4)
			.arg(unknown5)
			.arg(unknown6)
			.arg(unknown7);
}

QByteArray OpcodeCMUSC::params() const
{
	return QByteArray()
			.append((char)musicID)
			.append((char)unknown2)
			.append((char)unknown3)
			.append((char)unknown4)
			.append((char)unknown5)
			.append((char)unknown6)
			.append((char)unknown7);
}

OpcodeCHMST::OpcodeCHMST(const char *params, int size)
{
	setParams(params, size);
}

void OpcodeCHMST::setParams(const char *params, int)
{
	banks = params[0];
	var = params[1]; // bank 2
}

QString OpcodeCHMST::toString(Field *) const
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
		vars.append(FF7Var(B2(banks), var, FF7Var::Byte, true));
}

OpcodeGAMEOVER::OpcodeGAMEOVER()
{
}

QString OpcodeGAMEOVER::toString(Field *) const
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
/*28*//* KAWAI */		2,
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
/*fd*//* CMUSC */	8,
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
