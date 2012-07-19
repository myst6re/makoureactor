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
#include "Commande.h"

Commande::Commande(const QByteArray &commande, int pos)
	: pos(pos)
{
	setCommande(commande);
}

Commande::Commande() : opcode(0)
{
}

quint8 Commande::size() const { return params.size() + 1; }

const quint8 &Commande::getOpcode() const { return opcode; }

QByteArray &Commande::getParams() { return params; }

const QByteArray &Commande::getConstParams() const { return params; }

QByteArray Commande::toByteArray() const { return QByteArray((char)opcode + params); }

quint16 Commande::getIndent() const
{
	switch(opcode)
	{
	case 0x14:	return pos+5+(quint8)params.at(4);
	case 0x15:	return pos+5+_toInt(params.mid(4,2));
	case 0x16:
	case 0x18:	return pos+7+(quint8)params.at(6);
	case 0x17:
	case 0x19:	return pos+7+_toInt(params.mid(6,2));
	case 0x30:
	case 0x31:
	case 0x32:	return pos+3+(quint8)params.at(2);
	case 0xcb:
	case 0xcc:	return pos+2+(quint8)params.at(1);
	}
	return 0;
}

quint16 Commande::getPos() const { return pos; }

void Commande::setCommande(const QByteArray &commande)
{
	this->opcode = commande.size()<1 ? 0 : (quint8)commande.at(0);
	this->params = commande.mid(1);
}

void Commande::setPos(quint16 pos) { this->pos = pos; }

bool Commande::isVoid() const
{
	return opcode == 0 || (opcode >= 0x10 && opcode <= 0x13);
}

int Commande::subParam(int cur, int paramSize) const
{
	int value, tailleBA;
	
	if(paramSize%8 !=0)
		tailleBA = paramSize/8+1;
	else
		tailleBA = paramSize/8;
	
	memcpy(&value, params.mid(cur/8, tailleBA), tailleBA);
	return (value >> ((tailleBA*8-cur%8)-paramSize)) & ((int)pow(2, paramSize)-1);
}

bool Commande::rechercherVar(quint8 bank, quint8 adress, int value) const
{
	if(value!=65536) {
		if(this->opcode==0x80
			&& B1(params.at(0)) == bank && (quint8)params.at(1) == adress
			&& B2(params.at(0)) == 0 && (quint8)params.at(2) == value)
		{
			return true;
		}
		if(this->opcode==0x81
			&& B1(params.at(0)) == bank && (quint8)params.at(1) == adress
			&& B2(params.at(0)) == 0 && _toInt(params.mid(2,2)) == (quint16)value)
		{
			return true;
		}
		return false;
	}

	switch(this->opcode)
	{
	case 0x09:
		if((B1(params.at(0)) == bank && (quint8)params.at(3) == adress)
		   || (B2(params.at(0)) == bank && (quint8)params.at(5) == adress)
		   || (B1(params.at(1)) == bank && (quint8)params.at(7) == adress)
		   || (B2(params.at(1)) == bank && (quint8)params.at(8) == adress)
		   || (B1(params.at(2)) == bank && (quint8)params.at(10) == adress)
		   || (B2(params.at(2)) == bank && (quint8)params.at(12) == adress))
			return true;
		break;
	case 0xD3:
		if((B1(params.at(0)) == bank && (quint8)params.at(3) == adress)
		   || (B2(params.at(0)) == bank && (quint8)params.at(5) == adress)
		   || (B1(params.at(1)) == bank && (quint8)params.at(7) == adress)
		   || (B2(params.at(1)) == bank && (quint8)params.at(9) == adress)
		   || (B1(params.at(2)) == bank && (quint8)params.at(11) == adress)
		   || (B2(params.at(2)) == bank && (quint8)params.at(13) == adress))
			return true;
		break;
	case 0x56:case 0x57:
				if((B1(params.at(0)) == bank && (quint8)params.at(2) == adress)
				   || (B2(params.at(0)) == bank && (quint8)params.at(3) == adress)
				   || (B1(params.at(1)) == bank && (quint8)params.at(4) == adress)
				   || (B2(params.at(1)) == bank && (quint8)params.at(5) == adress))
					return true;
		break;
	case 0x75:case 0xC1:
				if((B1(params.at(0)) == bank && (quint8)params.at(3) == adress)
				   || (B2(params.at(0)) == bank && (quint8)params.at(4) == adress)
				   || (B1(params.at(1)) == bank && (quint8)params.at(5) == adress)
				   || (B2(params.at(1)) == bank && (quint8)params.at(6) == adress))
					return true;
		break;
	case 0xA5:case 0xD4:case 0xD5:
				if((B1(params.at(0)) == bank && (quint8)params.at(2) == adress)
				   || (B2(params.at(0)) == bank && (quint8)params.at(4) == adress)
				   || (B1(params.at(1)) == bank && (quint8)params.at(6) == adress)
				   || (B2(params.at(1)) == bank && (quint8)params.at(8) == adress))
					return true;
		break;
	case 0x0A:case 0x0B:
				if((B1(params.at(0)) == bank && (quint8)params.at(2) == adress)
				   || (B2(params.at(0)) == bank && (quint8)params.at(3) == adress)
				   || (B1(params.at(1)) == bank && (quint8)params.at(4) == adress))
					return true;
		break;
	case 0xA6:case 0xA7:case 0xC0:
				if((B1(params.at(0)) == bank && (quint8)params.at(2) == adress)
				   || (B2(params.at(0)) == bank && (quint8)params.at(4) == adress)
				   || (B1(params.at(1)) == bank && (quint8)params.at(6) == adress))
					return true;
		break;
	case 0x14:case 0x15:case 0x3B:case 0x76:case 0x77:
	case 0x78:case 0x79:case 0x80:case 0x81:case 0x82:
	case 0x83:case 0x84:case 0x85:case 0x86:case 0x87:
	case 0x88:case 0x89:case 0x8a:case 0x8b:case 0x8c:
	case 0x8d:case 0x8e:case 0x8f:case 0x90:case 0x91:
	case 0x92:case 0x93:case 0x94:case 0x9a:case 0x9b:
	case 0xE0:case 0xE1:
		if((B1(params.at(0)) == bank && (quint8)params.at(1) == adress)
		   || (B2(params.at(0)) == bank && (quint8)params.at(2) == adress))
			return true;
		break;
	case 0x16:case 0x17:case 0x18:case 0x58:case 0x59:
	case 0x5A:case 0x64:case 0x6A:case 0xA8:case 0xA9:
	case 0xAD:case 0xF1:
		if((B1(params.at(0)) == bank && (quint8)params.at(1) == adress)
		   || (B2(params.at(0)) == bank && (quint8)params.at(3) == adress))
			return true;
		break;
	case 0x2D:
		if((B1(params.at(0)) == bank && (quint8)params.at(2) == adress)
		   || (B2(params.at(0)) == bank && (quint8)params.at(4) == adress))
			return true;
		break;
	case 0xB8:case 0x9C:
				if((B1(params.at(0)) == bank && (quint8)params.at(2) == adress)
				   || (B2(params.at(0)) == bank && (quint8)params.at(3) == adress))
					return true;
		break;
	case 0x2C:case 0x37:case 0x39:case 0x3A:
				if(B1(params.at(0)) == bank && (quint8)params.at(2) == adress)
					return true;
		break;
	case 0x23:case 0x63:case 0x6E:case 0x6F:case 0x70:
	case 0x7A:case 0x7B:case 0x7C:case 0x7D:case 0x7F:
	case 0x95:case 0x96:case 0x97:case 0x98:case 0x99:
	case 0xB2:case 0xB3:case 0xB4:case 0xB5:case 0xBD:
	case 0xC5:case 0xC6:case 0xD6:case 0xD7:case 0xE2:
	case 0xE3:case 0xE4:case 0xFA:case 0xFE:
		if(B2(params.at(0)) == bank && (quint8)params.at(1) == adress)
			return true;
		break;
	case 0x49:case 0x73:case 0x74:case 0xB7:case 0xB9:
				if(B2(params.at(0)) == bank && (quint8)params.at(2) == adress)
					return true;
		break;
	case 0x41:case 0x42:
				if(B2(params.at(0)) == bank && (quint8)params.at(3) == adress)
					return true;
		break;
	case 0x48:
		if(B2(params.at(0)) == bank && (quint8)params.at(5) == adress)
			return true;
		break;
	}
	return false;
}

QList<int> Commande::searchAllVars()
{
	QList<int> vars;

	switch(this->opcode)
	{
	case 0x09:
		vars.append(B1(params.at(0)) | (params.at(3) << 4));
		vars.append(B2(params.at(0)) | (params.at(5) << 4));
		vars.append(B1(params.at(1)) | (params.at(7) << 4));
		vars.append(B2(params.at(1)) | (params.at(8) << 4));
		vars.append(B1(params.at(2)) | (params.at(10) << 4));
		vars.append(B2(params.at(2)) | (params.at(12) << 4));
		return vars;
	case 0xD3:
		vars.append(B1(params.at(0)) | (params.at(3) << 4));
		vars.append(B2(params.at(0)) | (params.at(5) << 4));
		vars.append(B1(params.at(1)) | (params.at(7) << 4));
		vars.append(B2(params.at(1)) | (params.at(9) << 4));
		vars.append(B1(params.at(2)) | (params.at(11) << 4));
		vars.append(B2(params.at(2)) | (params.at(13) << 4));
		return vars;
	case 0x56:case 0x57:
		vars.append(B1(params.at(0)) | (params.at(2) << 4));
		vars.append(B2(params.at(0)) | (params.at(3) << 4));
		vars.append(B1(params.at(1)) | (params.at(4) << 4));
		vars.append(B2(params.at(1)) | (params.at(5) << 4));
		return vars;
	case 0x75:case 0xC1:
		vars.append(B1(params.at(0)) | (params.at(3) << 4));
		vars.append(B2(params.at(0)) | (params.at(4) << 4));
		vars.append(B1(params.at(1)) | (params.at(5) << 4));
		vars.append(B2(params.at(1)) | (params.at(6) << 4));
		return vars;
	case 0xA5:case 0xD4:case 0xD5:
		vars.append(B1(params.at(0)) | (params.at(2) << 4));
		vars.append(B2(params.at(0)) | (params.at(4) << 4));
		vars.append(B1(params.at(1)) | (params.at(6) << 4));
		vars.append(B2(params.at(1)) | (params.at(8) << 4));
		return vars;
	case 0x0A:case 0x0B:
		vars.append(B1(params.at(0)) | (params.at(2) << 4));
		vars.append(B2(params.at(0)) | (params.at(3) << 4));
		vars.append(B1(params.at(1)) | (params.at(4) << 4));
		return vars;
	case 0xA6:case 0xA7:case 0xC0:
		vars.append(B1(params.at(0)) | (params.at(2) << 4));
		vars.append(B2(params.at(0)) | (params.at(4) << 4));
		vars.append(B1(params.at(1)) | (params.at(6) << 4));
		return vars;
	case 0x14:case 0x15:case 0x3B:case 0x76:case 0x77:
	case 0x78:case 0x79:case 0x80:case 0x81:case 0x82:
	case 0x83:case 0x84:case 0x85:case 0x86:case 0x87:
	case 0x88:case 0x89:case 0x8a:case 0x8b:case 0x8c:
	case 0x8d:case 0x8e:case 0x8f:case 0x90:case 0x91:
	case 0x92:case 0x93:case 0x94:case 0x9a:case 0x9b:
	case 0xE0:case 0xE1:
		vars.append(B1(params.at(0)) | (params.at(1) << 4));
		vars.append(B2(params.at(0)) | (params.at(2) << 4));
		return vars;
	case 0x16:case 0x17:case 0x18:case 0x58:case 0x59:
	case 0x5A:case 0x64:case 0x6A:case 0xA8:case 0xA9:
	case 0xAD:case 0xF1:
		vars.append(B1(params.at(0)) | (params.at(1) << 4));
		vars.append(B2(params.at(0)) | (params.at(3) << 4));
		return vars;
	case 0x2D:
		vars.append(B1(params.at(0)) | (params.at(2) << 4));
		vars.append(B2(params.at(0)) | (params.at(4) << 4));
		return vars;
	case 0xB8:case 0x9C:
		vars.append(B1(params.at(0)) | (params.at(2) << 4));
		vars.append(B2(params.at(0)) | (params.at(3) << 4));
		return vars;
	case 0x2C:case 0x37:case 0x39:case 0x3A:
		vars.append(B1(params.at(0)) | (params.at(2) << 4));
		return vars;
	case 0x23:case 0x63:case 0x6E:case 0x6F:case 0x70:
	case 0x7A:case 0x7B:case 0x7C:case 0x7D:case 0x7F:
	case 0x95:case 0x96:case 0x97:case 0x98:case 0x99:
	case 0xB2:case 0xB3:case 0xB4:case 0xB5:case 0xBD:
	case 0xC5:case 0xC6:case 0xD6:case 0xD7:case 0xE2:
	case 0xE3:case 0xE4:case 0xFA:case 0xFE:
		vars.append(B2(params.at(0)) | (params.at(1) << 4));
		return vars;
	case 0x49:case 0x73:case 0x74:case 0xB7:case 0xB9:
		vars.append(B2(params.at(0)) | (params.at(2) << 4));
		return vars;
	case 0x41:case 0x42:
		vars.append(B2(params.at(0)) | (params.at(3) << 4));
		return vars;
	case 0x48:
		vars.append(B2(params.at(0)) | (params.at(5) << 4));
		return vars;
	default:
		return vars;
	}
}

bool Commande::rechercherExec(quint8 group, quint8 script) const
{
	return (opcode==0x01 || opcode==0x02 || opcode==0x03)
			&& (quint8)params.at(0) == group && (quint8)(params.at(1) & 0x1F) == script;
}

bool Commande::rechercherTexte(const QRegExp &texte) const
{
	qint16 textID = -1;
	switch(this->opcode)
	{
	case 0x0F:	if((quint8)params.at(0)==0xFD)	textID = (quint8)params.at(2);	break;
	case 0x40:	textID = (quint8)params.at(1);	break;
	case 0x43:	textID = (quint8)params.at(0);	break;
	case 0x48:	textID = (quint8)params.at(2);	break;
	}
	return textID != -1 && textID < Data::currentTextes->size() && Data::currentTextes->at(textID)->search(texte);
}

void Commande::listUsedTexts(QSet<quint8> &usedTexts) const
{
	switch(opcode)
	{
	case 0x0F://SPECIAL
		if((quint8)params.at(0)==0xFD)
			usedTexts.insert(params.at(2));
		break;
	case 0x40://MESSAGE
		usedTexts.insert(params.at(1));
		break;
	case 0x43://MAP NAME
		usedTexts.insert(params.at(0));
		break;
	case 0x48://ASK
		usedTexts.insert(params.at(2));
		break;
	}
}

void Commande::listUsedTuts(QSet<quint8> &usedTuts) const
{
	switch(opcode)
	{
	case 0x21://TUTOR
		usedTuts.insert(params.at(0));
		break;
	}
}

void Commande::shiftTextIds(int textId, int steps)
{
	switch(opcode)
	{
	case 0x0F://SPECIAL
		if((quint8)params.at(0)==0xFD && (quint8)params.at(2)>textId) {
			params[2] = (quint8)params.at(2) + steps;
		}
		break;
	case 0x40://MESSAGE
		if((quint8)params.at(1)>textId) {
			params[1] = (quint8)params.at(1) + steps;
		}
		break;
	case 0x43://MAP NAME
		if((quint8)params.at(0)>textId) {
			params[0] = (quint8)params.at(0) + steps;
		}
		break;
	case 0x48://ASK
		if((quint8)params.at(2)>textId) {
			params[2] = (quint8)params.at(2) + steps;
		}
		break;
	}
}

void Commande::shiftTutIds(int tutId, int steps)
{
	switch(opcode)
	{
	case 0x21://TUTOR
		if((quint8)params.at(0)>tutId) {
			params[0] = (quint8)params.at(0) + steps;
		}
		break;
	}
}

void Commande::listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const
{
	FF7Window win;
	const char *params = this->params.constData();

	memset(&win, 0, sizeof(FF7Window));
	win.type = opcode;

	switch(opcode)
	{
	case 0x2F: // WSIZW
	case 0x50: // WINDOW
		memcpy(&win.x, &params[1], 2);
		memcpy(&win.y, &params[3], 2);
		memcpy(&win.w, &params[5], 2);
		memcpy(&win.h, &params[7], 2);

		windows.insert(params[0], win);// winID
		break;
	case 0x51: // WMOVE
		memcpy(&win.w, &params[1], 2);
		memcpy(&win.h, &params[3], 2);

		windows.insert(params[0], win);// winID
		break;
	case 0x40: // MESSAGE
		text2win.insert(params[1], params[0]);// textID, winID
		break;
	}
}

QString Commande::toString() const
{
	QString ret;
	quint8 depart = opcode == 0x28;
	for(quint8 i=depart ; i<params.size()-1 ; ++i)
		ret += QString("%1,").arg((quint8)params.at(i));
	if(params.size()-depart>0) ret += QString("%1").arg((quint8)params.at(params.size()-1));
	return ret;
}

QString Commande::_script(quint8 param)
{
	if(param < Data::currentGrpScriptNames.size())
		return Data::currentGrpScriptNames.at(param) + QObject::tr(" (n°%1)").arg(param);
	return QObject::tr("? (n°%1)").arg(param);
}

QString Commande::_text(quint8 textID)
{
	if(textID < Data::currentTextes->size())
		return "\"" + Data::currentTextes->at(textID)->getShortText(Config::value("jp_txt", false).toBool()) + "\"";
	return QObject::tr("(Pas de texte)");
}

QString Commande::_item(const QByteArray &param, quint8 bank)
{
	if(bank > 0)	return QObject::tr("n°%1").arg(_bank(param.at(0), bank));
	int itemID = _toInt(param);
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

QString Commande::_materia(const QByteArray &param, quint8 bank)
{
	if(bank > 0)	return QObject::tr("n°%1").arg(_bank(param.at(0), bank));
	int materiaID = _toInt(param);
	if(materiaID < Data::materia_names.size())	return Data::materia_names.at(materiaID);
	return QObject::tr("n°%1").arg(materiaID);
}

QString Commande::_field(const QByteArray &param)
{
	int fieldID = _toInt(param);
	if(fieldID < Data::field_names.size())	return Data::field_names.at(fieldID)+QObject::tr(" (n°%1)").arg(fieldID);
	return QObject::tr("n°%1").arg(fieldID);
}

QString Commande::_movie(quint8 movieID)
{
	if(movieID < Data::movie_names.size() && Data::movie_names.at(movieID)!="")	return Data::movie_names.at(movieID);
	return QObject::tr("n°%1").arg(movieID);
}

/* QString Commande::_objet3D(quint8 objet3D_ID)
{
	if(objet3D_ID < Data::currentCharNames.size())	return QString("%1 (%2)").arg(Data::currentCharNames.at(objet3D_ID), Data::currentHrcNames.at(objet3D_ID));
	return QObject::tr("n°%1").arg(objet3D_ID);
} */

QString Commande::_bank(quint8 adress, quint8 bank)
{
	if(Var::name(bank, adress)!="")	return Var::name(bank, adress);
	if(bank==0)		return QString("?");
	return QString("Var[%1][%2]").arg(bank).arg(adress);
}

QString Commande::_var(const QByteArray &param, quint8 bank)
{
	if(bank > 0)	return _bank(param.at(0), bank);
	return QString("%1").arg(_toInt(param));
}

QString Commande::_lvar(const QByteArray &param, quint8 bank1, quint8 bank2)
{
	if(bank1 > 0 || bank2 > 0)	return _var(param.mid(0,2), bank1) + QString(" + ") + _var(param.mid(2,2), bank2) + QString(" * 65536 ");
	return QString("%1").arg(_toInt(param));
}

QString Commande::_lvar(const QByteArray &param, quint8 bank1, quint8 bank2, quint8 bank3)
{
	if(bank1 > 0 || bank2 > 0 || bank3 > 0)	return _bank(param.at(0), bank1) + QObject::tr(" et ") + _bank(param.at(1), bank2) + QObject::tr(" et ") + _bank(param.at(2), bank3);
	return QString("%1").arg(_toInt(param));
}

QString Commande::_svar(const QByteArray &param, quint8 bank)
{
	if(bank > 0)	return _bank(param.at(0), bank);
	return QString("%1").arg(_toSInt(param));
}

quint32 Commande::_toInt(const QByteArray &param)
{	
	quint32 valeur=0;
	memcpy(&valeur, param, param.size());
	return valeur;
}

qint16 Commande::_toSInt(const QByteArray &param)
{	
	qint16 valeur=0;
	memcpy(&valeur, param, 2);
	return valeur;
}

QString Commande::_personnage(quint8 persoID)
{
	if(persoID < Data::char_names.size())	return Data::char_names.at(persoID);
	if(persoID >= 254)						return QObject::tr("(Vide)");
	if(persoID >= 100)						return Data::char_names.last();
	return QString("%1?").arg(persoID);
}

QString Commande::_miniGame(quint8 ID, quint8 param)
{
	switch(ID)
	{
	case 0x00:		return QObject::tr("Course de moto (paramètre %1)").arg(param);
	case 0x01:		return QObject::tr("Course de chocobo (paramètre %1)").arg(param);
	case 0x02:		return QObject::tr("Descente en snowboard -mode normal- (paramètre %1)").arg(param);
	case 0x03:		return QObject::tr("Fort Condor (paramètre %1)").arg(param);
	case 0x04:		return QObject::tr("Sous-marin (paramètre %1)").arg(param);
	case 0x05:		return QObject::tr("Speed Square (paramètre %1)").arg(param);
	case 0x06:		return QObject::tr("Descente en snowboard -mode Gold Saucer- (paramètre %1)").arg(param);
	default:		return QObject::tr("%1? (paramètre %2)").arg(ID).arg(param);
	}
}

QString Commande::_menu(quint8 ID, const QString &param)
{
	switch(ID)
	{
	case 1:		return QObject::tr("Fermer le programme (paramètre %1)").arg(param);
	case 2:		return QObject::tr("Encount Error (paramètre %1)").arg(param);
	case 5:		return QObject::tr("Crédits de ff7 (paramètre %1)").arg(param);
	case 6:		return QObject::tr("Changer nom de %1").arg(_personnage(param.toInt()));//Paramètre : personnage id
	case 7:		return QObject::tr("Changer l'équipe (paramètre %1)").arg(param);
	case 8:		return QObject::tr("magasin n°%1").arg(param);//Paramètre : magasin id
	case 9:		return QObject::tr("principal (paramètre %1)").arg(param);
	case 12:	return QObject::tr("course de moto (paramètre %1)").arg(param);
	case 14:	return QObject::tr("Sauvegarde (paramètre %1)").arg(param);
	case 15:	return QObject::tr("Effacer toutes les matérias (paramètre %1)").arg(param);
	case 16:	return QObject::tr("Rétablir toutes les matérias (paramètre %1)").arg(param);
	case 17:	return QObject::tr("Effacer la Matéria de %1").arg(_personnage(param.toInt()));
	case 18:	return QObject::tr("Effacer les matérias de Clad (paramètre %1)").arg(param);
	case 19:	return QObject::tr("Rétablir les matérias de Clad (paramètre %1)").arg(param);
	case 20:	return QObject::tr("? (paramètre %1)").arg(param);
	case 21:	return QObject::tr("HPs à 1 (paramètre %1)").arg(param);
	case 22:	return QObject::tr("? (paramètre %1)").arg(param);
	case 23:	return QObject::tr("maitre ? (paramètre %1)").arg(param);
	case 24:	return QObject::tr("? (paramètre %1)").arg(param);
	case 25:	return QObject::tr("? (paramètre %1)").arg(param);
	default:	return QObject::tr("%1? (paramètre %2)").arg(ID).arg(param);
	}
}

QString Commande::_windowType(quint8 param)
{
	switch(param)
	{
	case 0x00:		return QObject::tr("Normale");
	case 0x01:		return QObject::tr("Pas de fond, ni de bordure");
	case 0x02:		return QObject::tr("Fond semi-transparent");
	default:		return QString("%1?").arg(param);
	}
}

QString Commande::_windowNum(quint8 param)
{
	switch(param)
	{
	case 0x00:		return QObject::tr("(vide)");
	case 0x01:		return QObject::tr("Horloge");
	case 0x02:		return QObject::tr("Affichage numérique");
	default:		return QString("%1?").arg(param);
	}
}

QString Commande::_windowCorner(quint8 param, quint8 bank)
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

QString Commande::_sensRotation(quint8 param)
{
	switch(param)
	{
	case 1:		return QObject::tr("inverse");
	case 2:		return QObject::tr("inverse");
	default:	return QObject::tr("normal");
	}
}

QString Commande::_operateur(quint8 param)
{
	if(param < Data::operateur_names.size())
		return Data::operateur_names.at(param);
	return QString("%1?").arg(param);
}

QString Commande::_key(quint16 param)
{
	QString ret = "";
	for(quint8 i=0 ; i<16 ; ++i)
	{
		if((param >> i) & 1)
		{
			if(!ret.isEmpty())	ret.append(QObject::tr(" ou "));
			ret.append(Data::key_names.at(i));
		}
	}
	return ret;
}

QString Commande::_battleMode(quint32 param)
{
	QString ret = "";
	for(quint8 i=0 ; i<32 ; ++i)
	{
		if((param >> i) & 1)
		{
			if(!ret.isEmpty())	ret.append(", ");
			switch(i)
			{
			case 1:		ret.append(QObject::tr("Compte à rebours"));											break;
			case 2:		ret.append(QObject::tr("Attaque préventive"));											break;
			case 3:		ret.append(QObject::tr("Impossible de fuir"));											break;
			case 5:		ret.append(QObject::tr("Ne pas jouer Fanfare"));										break;
			case 6:		ret.append(QObject::tr("Active la battle arena"));										break;
			case 7:		ret.append(QObject::tr("Ne pas afficher d'écran de récompense"));						break;
			case 8:		ret.append(QObject::tr("Les personnages ne font pas leur animation de victoire"));		break;
			case 23:	ret.append(QObject::tr("Désactiver Game Over"));										break;
			default:	ret.append(QString("%1?").arg(i));														break;
			}
		}
	}
	return ret;
}

QString Commande::_special() const
{
	switch((quint8)params.at(0))
	{
	case 0xF5:
		return QObject::tr("%1 le curseur main").arg((quint8)params.at(1) == 0 ? QObject::tr("Afficher") : QObject::tr("Ne pas afficher"));//Booleen
	case 0xF6:
		return QObject::tr("PNAME - Désactiver le menu de droite (%1)").arg((quint8)params.at(1));//DEBUG
	case 0xF7:
		return QObject::tr("GMSPD | %1 |").arg((quint8)params.at(1));//DEBUG
	case 0xF8:
		return QObject::tr("Modifier la vitesse des messages (%2) | %1 |")
				.arg((quint8)params.at(1))//DEBUG
				.arg((quint8)params.at(2));//Vitesse
	case 0xF9:
		return QObject::tr("Remplir le menu matéria de toutes les matérias en quantité maximum");
	case 0xFA:
		return QObject::tr("Remplir l'inventaire par tous les objets en quantité maximum");
	case 0xFB:
		return QObject::tr("%1 les combats").arg((quint8)params.at(1) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booleen
	case 0xFC:
		return QObject::tr("%1 les cinématiques").arg((quint8)params.at(1) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booleen
	case 0xFD:
		return QObject::tr("Changer le nom de %1 par le texte %2")
				.arg(_personnage(params.at(1)))//Personnage ID
				.arg(_text(params.at(2)));//Texte ID
	case 0xFE:
		return QObject::tr("Met le temps à 0, débloque le menu \"PHS\" et \"Sauvegarder\". Nouvelle équipe : Clad | (Vide) | (Vide)");
	case 0xFF:
		return QObject::tr("Supprimer tous les objets de l'inventaire");
	default:
		return QString("%1?").arg((quint8)params.at(0),0,16);
	}
}

QString Commande::_kawai() const
{
	if(params.size()>1)
	{
		switch((quint8)params.at(1))
		{
		case 0x00:
			return "EYETX";
		case 0x01:
			return QObject::tr("%1 transparence").arg(params.size() > 2 && (quint8)params.at(2) == 0 ? QObject::tr("Désactiver") : QObject::tr("Activer"));
		case 0x02:
			return "AMBNT";
			//0x04
		case 0x06:
			return "LIGHT";
			//0x07
			//0x08
			//0x09
		case 0x0A:
			return "SBOBJ";
			//0x0B
			//0x0C
		case 0x0D:
			return "SHINE";
		case 0xFF:
			return "RESET";
		default:
			return QString("%1?").arg((quint8)params.at(1),0,16);
		}
	}
	return QObject::tr("Err.");
}

QString Commande::traduction() const
{
	switch(opcode)
	{
	case 0x00:
		return "Return";
		
	case 0x01:
		return QObject::tr("Exécuter le script n°%3 du groupe %1 (priorité %2/6) - Seulement si le script n'est pas déjà en cours d'exécution")
				.arg(_script((quint8)params.at(0)))//GroupID
				.arg(((quint8)params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)params.at(1) & 0x1F);//ScriptID sur 5 bits

	case 0x02:
		return QObject::tr("Exécuter le script n°%3 du groupe %1 (priorité %2/6)")
				.arg(_script((quint8)params.at(0)))//GroupID
				.arg(((quint8)params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)params.at(1) & 0x1F);//ScriptID sur 5 bits

	case 0x03:
		return QObject::tr("Exécuter le script n°%3 du groupe externe %1 (priorité %2/6) - Attend la fin de l'exécution pour continuer")
				.arg(_script((quint8)params.at(0)))//GroupID
				.arg(((quint8)params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)params.at(1) & 0x1F);//ScriptID sur 5 bits
		
	case 0x04:
		return QObject::tr("Exécuter le script n°%3 du groupe lié au personnage n°%1 de l'équipe (priorité %2/6) - Seulement si le script n'est pas déjà en cours d'exécution")
				.arg((quint8)params.at(0))//ID personnage d'équipe sur 8 bits
				.arg(((quint8)params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)params.at(1) & 0x1F);//ScriptID sur 5 bits
		
	case 0x05:
		return QObject::tr("Exécuter le script n°%3 du groupe lié au personnage n°%1 de l'équipe (priorité %2/6)")
				.arg((quint8)params.at(0))//ID personnage d'équipe sur 8 bits
				.arg(((quint8)params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)params.at(1) & 0x1F);//ScriptID sur 5 bits
		
	case 0x06:
		return QObject::tr("Exécuter le script n°%3 du groupe lié au personnage n°%1 de l'équipe (priorité %2/6) - Attend la fin de l'exécution pour continuer")
				.arg((quint8)params.at(0))//ID personnage d'équipe sur 8 bits
				.arg(((quint8)params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)params.at(1) & 0x1F);//ScriptID sur 5 bits
		
	case 0x07:
		return QObject::tr("Return et exécuter le script n°%2 du groupe appelant (priorité %1/6)")
				.arg(((quint8)params.at(0) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)params.at(0) & 0x1F);//ScriptID sur 5 bits
		
	case 0x08:
		return QObject::tr("Rassembler les membres de l'équipe dans le personnage jouable (vitesse=%1)")
				.arg((quint8)params.at(0));//Vitesse
		
	case 0x09:
		return QObject::tr("Faire sortir les membres de l'équipe à partir du personnage jouable (perso 1 : X=%1, Y=%2, dir=%3 ; perso 2 : X=%4, Y=%5, dir=%6) (vitesse %7)")
				.arg(_svar(params.mid(3,2), B1(params.at(0))))//Cible X1 (bank 1)
				.arg(_svar(params.mid(5,2), B2(params.at(0))))//Cible Y1 (bank 2)
				.arg(_var(params.mid(7,1), B1(params.at(1))))//Direction 1 (bank 3)
				.arg(_svar(params.mid(8,2), B2(params.at(1))))//Cible X2 (bank 4)
				.arg(_svar(params.mid(10,2), B1(params.at(2))))//Cible Y2 (bank 5)
				.arg(_var(params.mid(12,1), B2(params.at(2))))//Direction 2 (bank 6)
				.arg((quint8)params.at(13));//Vitesse
		
	case 0x0A:
		return QObject::tr("Sauvegarder les membres de l'équipe : %1 | %2 | %3")
				.arg(_var(params.mid(2,1), B1(params.at(0))))//party 1 (bank 1)
				.arg(_var(params.mid(3,1), B2(params.at(0))))//party 2 (bank 2)
				.arg(_var(params.mid(4,1), B1(params.at(1))));//party 3 (bank 3) -vérifié-
		
	case 0x0B:
		return QObject::tr("Récupérer les membres de l'équipe : %1 | %2 | %3")
				.arg(_var(params.mid(2,1), B1(params.at(0))))//party 1 (bank 1)
				.arg(_var(params.mid(3,1), B2(params.at(0))))//party 2 (bank 2)
				.arg(_var(params.mid(4,1), B1(params.at(1))));//party 3 (bank 3) -vérifié-
		
	case 0x0E:
		return QObject::tr("Demander le CD %1")
				.arg((quint8)params.at(0));//num CD
		
	case 0x0F:
		return QObject::tr("SPECIAL - ") + _special();
		
	case 0x10:
		return QObject::tr("Aller à l'octet %1 du script")
				.arg(pos+1+(quint8)params.at(0));//Saut court
		
	case 0x11:
		return QObject::tr("Aller à l'octet %1 du script")
				.arg(pos+1+_toSInt(params.mid(0,2)));//Saut long
		
	case 0x12:
		return QObject::tr("Aller à l'octet %1 du script")
				.arg((int)(pos-(quint8)params.at(0)));//Saut court en arrière
		
	case 0x13:
		return QObject::tr("Aller à l'octet %1 du script")
				.arg((int)(pos-_toInt(params.mid(0,2))));//Saut long en arrière
		
	case 0x14://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_var(params.mid(1,1), B1(params.at(0))))//Valeur courte (bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))))//Valeur courte (bank 2)
				.arg(_operateur(params.at(3)))//Opérateur
				.arg(pos+5+(quint8)params.at(4))//Saut court
				.arg((quint8)params.at(3)==9 || (quint8)params.at(3)==10 ? ")" : "");//mini hack ")"
		
	case 0x15://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_var(params.mid(1,1), B1(params.at(0))))//Valeur courte (bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))))//Valeur courte (bank 2)
				.arg(_operateur(params.at(3)))//Opérateur
				.arg(pos+5+_toInt(params.mid(4,2)))//Saut long
				.arg((quint8)params.at(3)==9 || (quint8)params.at(3)==10 ? ")" : "");//mini hack ")"
		
	case 0x16://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_svar(params.mid(1,2), B1(params.at(0))))//Valeur signée (bank 1)
				.arg(_svar(params.mid(3,2), B2(params.at(0))))//Valeur signée (bank 2)
				.arg(_operateur(params.at(5)))//Opérateur
				.arg(pos+7+(quint8)params.at(6))//Saut court
				.arg((quint8)params.at(5)==9 || (quint8)params.at(5)==10 ? ")" : "");//mini hack ")"
		
	case 0x17://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_svar(params.mid(1,2), B1(params.at(0))))//Valeur signée (bank 1)
				.arg(_svar(params.mid(3,2), B2(params.at(0))))//Valeur signée (bank 2)
				.arg(_operateur(params.at(5)))//Opérateur
				.arg(pos+7+_toInt(params.mid(6,2)))//Saut long
				.arg((quint8)params.at(5)==9 || (quint8)params.at(5)==10 ? ")" : "");//mini hack ")"
		
	case 0x18://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_var(params.mid(1,2), B1(params.at(0))))//Valeur (bank 1)
				.arg(_var(params.mid(3,2), B2(params.at(0))))//Valeur (bank 2)
				.arg(_operateur(params.at(5)))//Opérateur
				.arg(pos+7+(quint8)params.at(6))//Saut court
				.arg((quint8)params.at(5)==9 || (quint8)params.at(5)==10 ? ")" : "");//mini hack ")"
		
	case 0x19://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_var(params.mid(1,2), B1(params.at(0))))//Valeur (bank 1)
				.arg(_var(params.mid(3,2), B2(params.at(0))))//Valeur (bank 2)
				.arg(_operateur(params.at(5)))//Opérateur
				.arg(pos+7+_toInt(params.mid(6,2)))//Saut long
				.arg((quint8)params.at(5)==9 || (quint8)params.at(5)==10 ? ")" : "");//mini hack ")"
		
	case 0x20:
		return QObject::tr("Lancer un mini-jeu : %5 (Après le jeu aller à l'écran %1 (X=%2, Y=%3, polygone id=%4))")
				.arg(_field(params.mid(0,2)))//id décor
				.arg(_toSInt(params.mid(2,2)))//cible X
				.arg(_toSInt(params.mid(4,2)))//cible Y
				.arg(_toInt(params.mid(6,2)))//polygone id
				.arg(_miniGame(params.at(9), params.at(8)));//Mini-jeu
		
	case 0x21:
		return QObject::tr("Lancer le tutoriel n°%1")
				.arg((quint8)params.at(0));//id tutoriel
		
	case 0x22:
		return QObject::tr("Mode de combat : %1")
				.arg(_battleMode(_toInt(params.mid(0,4))));//battleMode
		
	case 0x23:
		return QObject::tr("Stocker le résultat du dernier combat dans %1")
				.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)
		
	case 0x24:
		return QObject::tr("Attendre %1 img")
				.arg(_toInt(params.mid(0,2)));//Valeur
		
	case 0x25:
		return QObject::tr("Voiler l'écran avec la couleur RVB(%1, %2, %3)")
				.arg(_var(params.mid(3,1), B1(params.at(0))))//Rouge (bank 1 ???)
				.arg(_var(params.mid(4,1), B2(params.at(0))))//Vert (bank 2 ???)
				.arg(_var(params.mid(5,1), B1(params.at(1))));//Bleu (bank 3 ???)
		// .arg((quint8)params.at(6));//Vitesse
		//Type
		//Adjust
		
	case 0x26:
		return QObject::tr("L'objet 3D cligne des yeux : %1")
				.arg((quint8)params.at(0) == 0 ? QObject::tr("ON") : QObject::tr("OFF"));//booleen
		
	case 0x27:
		return QObject::tr("BGMOVIE : %1")
				.arg((quint8)params.at(0) == 0 ? QObject::tr("ON") : QObject::tr("OFF"));//booleen
		
	case 0x28:
		return QObject::tr("Filtre graphique sur l'objet 3D - ") + _kawai();//KAWAI
		
	case 0x29:
		return QObject::tr("Attendre la fin de l'exécution du filtre graphique");
		
	case 0x2A:
		return QObject::tr("Déplacer l'objet 3D vers le membre n°%1 de l'équipe")
				.arg((quint8)params.at(0));//Membre d'équipe
		
	case 0x2B:
		return QObject::tr("SLIP : %1")
				.arg((quint8)params.at(0) == 0 ? QObject::tr("ON") : QObject::tr("OFF"));
		
	case 0x2C:
		return QObject::tr("Déplacer la couche %1 du décor (Z=%2)")
				.arg((quint8)params.at(1))//Couche ID
				.arg(_svar(params.mid(2,2), B2((quint8)params.at(0))));//Cible Z (bank 2 ???)
		
	case 0x2D:
		return QObject::tr("Animer la couche %1 du décor (horizontalement=%2, verticalement=%3)")
				.arg((quint8)params.at(1))//Couche ID
				.arg(_svar(params.mid(2,2), B1(params.at(0))))//Cible X (bank 1)
				.arg(_svar(params.mid(4,2), B2(params.at(0))));//Cible Y (bank 2)
		
	case 0x2E:
		return QObject::tr("WCLS (fenêtre n°%1)")
				.arg((quint8)params.at(0));//Fenêtre ID
		
	case 0x2F:
		return QObject::tr("Redimensionner fenêtre n°%1 (X=%2, Y=%3, largeur=%4, hauteur=%5)")
				.arg((quint8)params.at(0))//Fenêtre ID
				.arg(_toInt(params.mid(1,2)))//Cible X
				.arg(_toInt(params.mid(3,2)))//Cible Y
				.arg(_toInt(params.mid(5,2)))//Largeur
				.arg(_toInt(params.mid(7,2)));//Hauteur
		
	case 0x30://Indenter
		return QObject::tr("Si appuie sur la touche %1 (aller à l'octet %2 du script sinon)")
				.arg(_key(_toInt(params.mid(0,2))))//touches
				.arg(pos+3+(quint8)params.at(2));//Saut court
		
	case 0x31://Indenter
		return QObject::tr("Si appuie sur la touche %1 une fois (aller à l'octet %2 du script sinon)")
				.arg(_key(_toInt(params.mid(0,2))))//touches
				.arg(pos+3+(quint8)params.at(2));//Saut court
		
	case 0x32://Indenter
		return QObject::tr("Si relache la touche %1 pour la première fois (aller à l'octet %2 du script sinon)")
				.arg(_key(_toInt(params.mid(0,2))))//touches
				.arg(pos+3+(quint8)params.at(2));//Saut court
		
	case 0x33:
		return QObject::tr("%1 les déplacements du personnage jouable")
				.arg((quint8)params.at(0) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booleen
		
	case 0x34:
		return QObject::tr("Tourner instantanément l'objet 3D vers le membre de l'équipe n°%1")
				.arg((quint8)params.at(0));//Party ID
		
	case 0x35:
		return QObject::tr("Tourner l'objet 3D vers le membre de l'équipe n°%1 (Vitesse=%2, SensRotation=%3)")
				.arg((quint8)params.at(0))//Party ID
				.arg((quint8)params.at(1))//Vitesse
				.arg(_sensRotation((quint8)params.at(2)));//Sens de rotation
		
	case 0x36:
		return QObject::tr("%2 dans fenêtre n°%1 (gauche=%3, haut=%4)")
				.arg((quint8)params.at(0))//Fenêtre ID
				.arg(_windowNum((quint8)params.at(1)))//Affichage numérique
				.arg((quint8)params.at(2))//Margin-left
				.arg((quint8)params.at(3));//Margin-top
		
	case 0x37:
		return QObject::tr("Affecter %2 dans la fenêtre n°%1 et afficher %3 chiffres")
				.arg((quint8)params.at(1))//Fenêtre ID
				.arg(_lvar(params.mid(2,4), B1(params.at(0)), B2(params.at(0))))//Valeur (bank 1 et 2)
				.arg((quint8)params.at(6));//nombre de chiffres à afficher
		
	case 0x38:
		return QObject::tr("Affecter une valeur au compte à rebours (H=%1, M=%2, S=%3)")
				.arg(_var(params.mid(2,1), B1(params.at(0))))//Valeur (bank 1 ???)
				.arg(_var(params.mid(3,1), B2(params.at(0))))//Valeur (bank 2 ???)
				.arg(_var(params.mid(4,1), B1(params.at(1))));//Valeur (bank 3 ???)
		
	case 0x39:
		return QObject::tr("Ajouter %1 gils à l'équipe")
				.arg(_lvar(params.mid(1,4), B1(params.at(0)), B2(params.at(0))));//Valeur (bank 1 et 2)
		
	case 0x3A:
		return QObject::tr("Retirer %1 gils à l'équipe")
				.arg(_lvar(params.mid(1,4), B1(params.at(0)), B2(params.at(0))));//Valeur (bank 1 et 2)
		
	case 0x3B:
		return QObject::tr("Copier le nombre de Gils dans %1 et %2")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_bank(params.at(2), B2(params.at(0))));//(bank 2)
		
	case 0x3C:case 0x3D:case 0x3F:
		return QObject::tr("Redonne les HP/MP aux membres de l'équipe");
		
	case 0x3E:
		return QObject::tr("Redonne les HP/MP à tous et soigne les troubles de statut");
		
	case 0x40:
		return QObject::tr("Afficher message %2 dans la fenêtre n°%1")
				.arg((quint8)params.at(0))//Fenêtre ID
				.arg(_text(params.at(1)));//Texte
		
	case 0x41:
		return QObject::tr("Affecter %3 à la variable n°%2 dans la fenêtre n°%1")
				.arg((quint8)params.at(1))//Fenêtre ID
				.arg((quint8)params.at(2))//Win variable ID
				.arg(_var(params.mid(3,1), B2(params.at(0))));//Valeur (bank 2)
		
	case 0x42:
		return QObject::tr("Affecter %3 à la variable n°%2 dans la fenêtre n°%1")
				.arg((quint8)params.at(1))//Fenêtre ID
				.arg((quint8)params.at(2))//Win variable ID
				.arg(_var(params.mid(3,2), B2(params.at(0))));//Valeur (bank 2)
		
	case 0x43:
		return QObject::tr("Afficher %1 dans le menu")
				.arg(_text(params.at(0)));//Texte
		
	case 0x45:
		return QObject::tr("Augmenter de %2 MPs le membre n°%1 de l'équipe")
				.arg((quint8)params.at(1))//Party ID
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2 ???)
		
	case 0x47:
		return QObject::tr("Diminuer de %2 MPs le membre n°%1 de l'équipe")
				.arg((quint8)params.at(1))//Party ID
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2 ???)
		
	case 0x48:
		return QObject::tr("Poser question %2 dans la fenêtre n°%1 (et mettre la réponse sélectionnée dans %5) première ligne=%3, dernière ligne=%4")
				.arg((quint8)params.at(1))//Fenêtre ID
				.arg(_text(params.at(2)))//Texte
				.arg((quint8)params.at(3))//première ligne
				.arg((quint8)params.at(4))//dernière ligne
				.arg(_bank(params.at(5), B2(params.at(0))));//(bank 2)
		
	case 0x49:
		return QObject::tr("Afficher menu %1")
				.arg(_menu(params.at(1), _var(params.mid(2,1), B2(params.at(0)))));//Menu ID, Valeur (bank 2)
		
	case 0x4A:
		return QObject::tr("%1 l'accès aux menus")
				.arg((quint8)params.at(0) == 0 ? QObject::tr("Permettre") : QObject::tr("Interdire"));//booleen
		
	case 0x4B:
		return QObject::tr("Choisir la battle table : %1")
				.arg((quint8)params.at(0));//battle table ID
		
	case 0x4D:
		return QObject::tr("Augmenter de %2 HPs le membre n°%1 de l'équipe")
				.arg((quint8)params.at(1))//Party ID
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2 ???)
		
	case 0x4F:
		return QObject::tr("Diminuer de %2 HPs le membre n°%1 de l'équipe")
				.arg((quint8)params.at(1))//Party ID
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2 ???)
		
	case 0x50:
		return QObject::tr("Créer la fenêtre n°%1 (X=%2, Y=%3, largeur=%4, hauteur=%5)")
				.arg((quint8)params.at(0))//Fenêtre ID
				.arg(_toInt(params.mid(1,2)))//Cible X
				.arg(_toInt(params.mid(3,2)))//Cible Y
				.arg(_toInt(params.mid(5,2)))//Largeur
				.arg(_toInt(params.mid(7,2)));//Hauteur
		
	case 0x51:
		return QObject::tr("Déplacer la fenêtre n°%1 (X=%2, Y=%3)")
				.arg((quint8)params.at(0))//Fenêtre ID
				.arg(_toInt(params.mid(1,2)))//Cible X
				.arg(_toInt(params.mid(3,2)));//Cible Y
		
	case 0x52:
		return QObject::tr("Décoration de la fenêtre n°%1 : %2 (%3 la fermeture de la fenêtre par le joueur)")
				.arg((quint8)params.at(0))//Fenêtre ID
				.arg(_windowType((quint8)params.at(1)))//Mode
				.arg((quint8)params.at(2) == 0 ? QObject::tr("autoriser") : QObject::tr("empêcher"));//booleen
		
	case 0x53:
		return QObject::tr("Remettre la fenêtre n°%1 à zéro")
				.arg((quint8)params.at(0));//Fenêtre ID
		
	case 0x54:
		return QObject::tr("Fermer la fenêtre n°%1")
				.arg((quint8)params.at(0));//Fenêtre ID
		
	case 0x55:
		return QObject::tr("Configurer le nombre de lignes de texte à %2 dans la fenêtre n°%1")
				.arg((quint8)params.at(0))//Fenêtre ID
				.arg((quint8)params.at(1));//Ligne
		
	case 0x56:
		return QObject::tr("Obtenir la couleur du côté %1 des fenêtres et en stocker les composantes dans %2 (R), %3 (V) et %4 (B)")
				.arg(_windowCorner(params.at(2), B1(params.at(0))))//Côté (bank 1)
				.arg(_var(params.mid(3,1), B2(params.at(0))))//Rouge (bank 2)
				.arg(_var(params.mid(4,1), B1(params.at(1))))//Vert (bank 3)
				.arg(_var(params.mid(5,1), B2(params.at(1))));//Bleu (bank 4)
		
	case 0x57:
		return QObject::tr("Changer la couleur du côté %1 des fenêtres : RVB(%2, %3, %4)")
				.arg(_windowCorner(params.at(2), B1(params.at(0))))//Côté (bank 1)
				.arg(_var(params.mid(3,1), B2(params.at(0))))//Rouge (bank 2)
				.arg(_var(params.mid(4,1), B1(params.at(1))))//Vert (bank 3)
				.arg(_var(params.mid(5,1), B2(params.at(1))));//Bleu (bank 4)
		
	case 0x58:
		return QObject::tr("Ajouter %2 objet(s) %1 dans l'inventaire")
				.arg(_item(params.mid(1,2), B1(params.at(0))))//Item ID (bank 1)
				.arg(_var(params.mid(3,1), B2(params.at(0))));//Quantité (bank 2)
		
	case 0x59:
		return QObject::tr("Supprimer %2 objet(s) %1 dans l'inventaire")
				.arg(_item(params.mid(1,2), B1(params.at(0))))//Item ID (bank 1)
				.arg(_var(params.mid(3,1), B2(params.at(0))));//Quantité (bank 2)
		
	case 0x5A:
		return QObject::tr("%2 = quantité d'objets %1 dans l'inventaire")
				.arg(_item(params.mid(1,2), B1(params.at(0))))//Item ID (bank 1)
				.arg(_bank(params.at(3), B2(params.at(0))));//(bank 2)
		
		case 0x5B:
		return QObject::tr("Ajouter la matéria %1 dans l'inventaire (AP=%2)")
				.arg(_materia(params.mid(2,1), B1(params.at(0))))//Materia ID (bank 1)
				.arg(_lvar(params.mid(3,3), B2(params.at(0)), B1(params.at(1)), B2(params.at(1))));//Materia AP (bank 2, 3 et 4 ???)
		
		case 0x5C:
		return QObject::tr("Supprimer %3 matéria(s) %1 dans l'inventaire (AP=%2)")
				.arg(_materia(params.mid(2,1), B1(params.at(0))))//Materia ID (bank 1)
				.arg(_lvar(params.mid(3,3), B2(params.at(0)), B1(params.at(1)), B2(params.at(1))))//Materia AP (bank 2, 3 et 4 ???)
				.arg((quint8)params.at(6));//Quantité
		
		case 0x5D:
		return QObject::tr("%4 = quantité de matéria %1 dans l'inventaire (AP=%2, ???=%3)")
				.arg(_materia(params.mid(3,1), B1(params.at(0))))//Materia ID (bank 1)
				.arg(_lvar(params.mid(4,3), B2(params.at(0)), B1(params.at(1)), B2(params.at(1))))//Materia AP (bank 2, 3 et 4 ???)
				.arg((quint8)params.at(7))//TODO ??
				.arg(_bank(params.at(8), B2(params.at(2))));//(bank 5)
		
		case 0x5E:
		return QObject::tr("Secouer l'écran (nbOscillations=%1, Amplitude=%2, vitesse=%3)")
				.arg((quint8)params.at(2))//nbOscillations???
				.arg((quint8)params.at(5))//Amplitude
				.arg((quint8)params.at(6));//Vitesse
		
		case 0x5F:
		return QObject::tr("Ne rien faire...");
		
		case 0x60:
		return QObject::tr("Aller à l'écran %1 (X=%2, Y=%3, polygone id=%4, direction=%5)")
				.arg(_field(params.mid(0,2)))//id décor
				.arg(_toSInt(params.mid(2,2)))//cible X
				.arg(_toSInt(params.mid(4,2)))//cible Y
				.arg(_toInt(params.mid(6,2)))//polygone id
				.arg((quint8)params.at(8));//direction

		case 0x61:
		return QObject::tr("SCRLO");
		
		case 0x62:
		return QObject::tr("SCRLC");
		
		case 0x63:
		return QObject::tr("Centrer sur le groupe %2 (vitesse=%1, type=%3)")
				.arg(_var(params.mid(1,2), B2(params.at(0))))//Vitesse (bank 2)
				.arg(_script((quint8)params.at(3)))//GroupID
				.arg((quint8)params.at(4));//ScrollType
		
		case 0x64:
		return QObject::tr("Centrer sur zone (X=%1, Y=%2)")
				.arg(_svar(params.mid(1,2), B1(params.at(0))))//Cible X (bank 1)
				.arg(_svar(params.mid(3,2), B2(params.at(0))));//Cible Y (bank 2)
		
		case 0x65:
		return QObject::tr("Centrer sur le personnage jouable");
		
		case 0x66:
		return QObject::tr("Centrer sur zone (X=%1, Y=%2, vitesse=%3)")
				.arg(_svar(params.mid(2,2), B1(params.at(0))))//Cible X (bank 1 ???)
				.arg(_svar(params.mid(4,2), B2(params.at(0))))//Cible Y (bank 2 ???)
				.arg(_var(params.mid(6,2), B2(params.at(1))));//Vitesse (bank 4 ???)
		
		case 0x67:
		return QObject::tr("Attendre la fin du dernier centrage pour continuer");
		
		case 0x68:
		return QObject::tr("Centrer sur zone (X=%1, Y=%2, vitesse=%3)")
				.arg(_svar(params.mid(2,2), B1(params.at(0))))//Cible X (bank 1 ???)
				.arg(_svar(params.mid(4,2), B2(params.at(0))))//Cible Y (bank 2 ???)
				.arg(_var(params.mid(6,2), B2(params.at(1))));//Vitesse (bank 4 ???)
		
		case 0x69:
		return QObject::tr("MPDSP : %1")
				.arg((quint8)params.at(0));//booléen ?
		
		case 0x6A:
		return QObject::tr("Centrer sur Zone (?=%1, ?=%2, ?=%3)")
				.arg(_svar(params.mid(1,2), B1(params.at(0))))//??? (bank 1)
				.arg(_svar(params.mid(3,2), B2(params.at(0))))//??? (bank 2)
				.arg((quint8)params.at(5));//???
		
		case 0x6B:
		return QObject::tr("Voiler l'écran avec la couleur RVB(%1, %2, %3) (vitesse=%4, type=%5, adjust=%6)")
				.arg(_var(params.mid(2,1), B1(params.at(0))))//Rouge (bank 1 ???)
				.arg(_var(params.mid(3,1), B2(params.at(0))))//Vert (bank 2 ???)
				.arg(_var(params.mid(4,1), B2(params.at(1))))//Bleu (bank 4 ???)
				.arg((quint8)params.at(5))//Vitesse
				.arg((quint8)params.at(6))//Type
				.arg((quint8)params.at(7));//Adjust
		
		case 0x6C:
		return QObject::tr("Attendre la fin du voilage de l'écran pour continuer");
		
		case 0x6D:
		return QObject::tr("%1 le polygone n°%2")
				.arg((quint8)params.at(2) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"))//booléen
				.arg(_toInt(params.mid(0,2)));//Polygone ID
		
		case 0x6E:
		return QObject::tr("Stocker l'id du décor précédent dans %1")
				.arg(_bank(params.at(1), B2(params.at(0))));//Décor ID (bank 2)
		
		case 0x6F:
		return QObject::tr("Centrer sur le personnage n°%2 de l'équipe actuelle (vitesse=%1 img, type=%3)")
				.arg(_var(params.mid(1,2), B2(params.at(0))))//Vitesse (bank 2) ???
				.arg((quint8)params.at(3))//party ID
				.arg((quint8)params.at(4));//Type de Scroll
		
		case 0x70:
		return QObject::tr("Commencer le combat n°%1")
				.arg(_var(params.mid(1,2), B2(params.at(0))));//battle ID (bank 2) ???
		
		case 0x71:
		return QObject::tr("%1 les combats aléatoires")
				.arg((quint8)params.at(0) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//booléen
		
		case 0x72:
		return QObject::tr("BTLMD");
		
		case 0x73:
		return QObject::tr("Obtenir la direction du personnage n°%1 de l'équipe actuelle et la stocker dans %2")
				.arg((quint8)params.at(1))//Party ID
				.arg(_bank(params.at(2), B2(params.at(0))));//(bank 2)
		
		case 0x74:
		return QObject::tr("Obtenir l'id du personnage n°%1 de l'équipe actuelle et le stocker dans %2")
				.arg((quint8)params.at(1))//Party ID
				.arg(_bank(params.at(2), B2(params.at(0))));//(bank 2)
		
		case 0x75:
		return QObject::tr("Obtenir les coordonnées du personnage n°%1 de l'équipe actuelle (stocker : X dans %2, Y dans %3, Z dans %4 et l'id dans %5)")
				.arg((quint8)params.at(2))//Party ID
				.arg(_bank(params.at(3), B1(params.at(0))))//(bank 1)
				.arg(_bank(params.at(4), B2(params.at(0))))//(bank 2)
				.arg(_bank(params.at(5), B1(params.at(1))))//(bank 3)
				.arg(_bank(params.at(6), B2(params.at(1))));//(bank 4)
		
		case 0x76:
		return QObject::tr("%1 = %1 + %2 (8 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x77:
		return QObject::tr("%1 = %1 + %2 (16 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x78:
		return QObject::tr("%1 = %1 - %2 (8 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x79:
		return QObject::tr("%1 = %1 - %2 (16 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x7A:
		return QObject::tr("%1 = %1 + 1 (8 bits)")
				.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)
		
		case 0x7B:
		return QObject::tr("%1 = %1 + 1 (16 bits)")
				.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)
		
		case 0x7C:
		return QObject::tr("%1 = %1 - 1 (8 bits)")
				.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)
		
		case 0x7D:
		return QObject::tr("%1 = %1 - 1 (16 bits)")
				.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)
		
		case 0x7E:
		return QObject::tr("%1 la possibilité de parler à l'objet 3D")
				.arg((quint8)params.at(0) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booléen
		
		case 0x7F:
		return QObject::tr("Seed Random Generator : %1")
				.arg(_var(params.mid(1,1), B2(params.at(0))));//Valeur (bank 2)

		case 0x80:
		return QObject::tr("%1 = %2 (8 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x81:
		return QObject::tr("%1 = %2 (16 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x82:
		return QObject::tr("Mettre le bit %2 à 1 dans %1")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Position (bank 2)
		
		case 0x83:
		return QObject::tr("Mettre le bit %2 à 0 dans %1")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Position (bank 2)
		
		case 0x84:
		return QObject::tr("Inverser la valeur du bit %2 dans %1")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Position (bank 2)
		
		case 0x85:
		return QObject::tr("%1 = %1 + %2 (8 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x86:
		return QObject::tr("%1 = %1 + %2 (16 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x87:
		return QObject::tr("%1 = %1 - %2 (8 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x88:
		return QObject::tr("%1 = %1 - %2 (16 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x89:
		return QObject::tr("%1 = %1 * %2 (8 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x8A:
		return QObject::tr("%1 = %1 * %2 (16 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x8B:
		return QObject::tr("%1 = %1 / %2 (8 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x8C:
		return QObject::tr("%1 = %1 / %2 (16 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x8D:
		return QObject::tr("%1 = %1 mod %2 (8 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x8E:
		return QObject::tr("%1 = %1 mod %2 (16 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x8F:
		return QObject::tr("%1 = %1 & %2 (8 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x90:
		return QObject::tr("%1 = %1 & %2 (16 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x91:
		return QObject::tr("%1 = %1 | %2 (8 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x92:
		return QObject::tr("%1 = %1 | %2 (16 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x93:
		return QObject::tr("%1 = %1 ^ %2 (8 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x94:
		return QObject::tr("%1 = %1 ^ %2 (16 bits)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x95:
		return QObject::tr("%1 = %1 + 1 (8 bits)")
				.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)
		
		case 0x96:
		return QObject::tr("%1 = %1 + 1 (16 bits)")
				.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)
		
		case 0x97:
		return QObject::tr("%1 = %1 - 1 (8 bits)")
				.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)
		
		case 0x98:
		return QObject::tr("%1 = %1 - 1 (16 bits)")
				.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)
		
		case 0x99:
		return QObject::tr("Affecter une valeur aléatoire à %1 (8 bits)")
				.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)
		
		case 0x9A:
		return QObject::tr("%1 = %2 & 0xFF (low byte)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,1), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x9B:
		return QObject::tr("%1 = (%2 >> 8) & 0xFF (high byte)")
				.arg(_bank(params.at(1), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(2,2), B2(params.at(0))));//Valeur (bank 2)
		
		case 0x9C:
		return QObject::tr("%1 = (%2 & 0xFF) | ((%3 & 0xFF) << 8)")
				.arg(_bank(params.at(2), B1(params.at(0))))//(bank 1)
				.arg(_var(params.mid(3,1), B2(params.at(0))))//Valeur (bank 2)
				.arg(_var(params.mid(4,1), B2(params.at(1))));//Valeur (bank 4 ???)
		
		case 0x9D:
		return QObject::tr("SETX");
		
		case 0x9E:
		return QObject::tr("GETX");
		
		case 0x9F:
		return QObject::tr("SEARCHX");
		
		case 0xA0:
		return QObject::tr("L'objet 3D est jouable et c'est %1")
				.arg(_personnage(params.at(0)));//Personnage ID
		
		case 0xA1:
		return QObject::tr("Ce groupe est un objet 3D (id=%1)")
				.arg((quint8)params.at(0));//objet 3D ID
		
		case 0xA2:
		return QObject::tr("Joue l'animation %1 de l'objet 3D (vitesse=%2)")
				.arg((quint8)params.at(0))//animation ID
				.arg((quint8)params.at(1));//vitesse
		
		case 0xA3:
		return QObject::tr("Joue l'animation %1 de l'objet 3D et retourne à l'état précédent (vitesse=%2)")
				.arg((quint8)params.at(0))//animation ID
				.arg((quint8)params.at(1));//vitesse
		
		case 0xA4:
		return QObject::tr("%1 l'objet 3D")
				.arg((quint8)params.at(0) == 0 ? QObject::tr("Cacher") : QObject::tr("Afficher"));//animation ID
		
		case 0xA5:
		return QObject::tr("Place l'objet 3D (X=%1, Y=%2, Z=%3, polygone id=%4)")
				.arg(_svar(params.mid(2,2), B1(params.at(0))))//Cible X (bank 1)
				.arg(_svar(params.mid(4,2), B2(params.at(0))))//Cible Y (bank 2)
				.arg(_svar(params.mid(6,2), B1(params.at(1))))//Cible Z (bank 3)
				.arg(_var(params.mid(8,2), B2(params.at(1))));//Polygone ID (bank 4)
		
		case 0xA6:
		return QObject::tr("Place l'objet 3D (X=%1, Y=%2, polygone id=%4)")
				.arg(_svar(params.mid(2,2), B1(params.at(0))))//Cible X (bank 1)
				.arg(_svar(params.mid(4,2), B2(params.at(0))))//Cible Y (bank 2)
				.arg(_var(params.mid(6,2), B1(params.at(1))));//Polygone ID (bank 3)
		
		case 0xA7:
		return QObject::tr("Place l'objet 3D (X=%1, Y=%2, Z=%3)")
				.arg(_svar(params.mid(2,2), B1(params.at(0))))//Cible X (bank 1)
				.arg(_svar(params.mid(4,2), B2(params.at(0))))//Cible Y (bank 2)
				.arg(_svar(params.mid(6,2), B1(params.at(1))));//Cible Z (bank 3)
		
		case 0xA8:
		return QObject::tr("Déplace l'objet 3D (X=%1, Y=%2)")
				.arg(_svar(params.mid(1,2), B1(params.at(0))))//Cible X (bank 1)
				.arg(_svar(params.mid(3,2), B2(params.at(0))));//Cible Y (bank 2)
		
		case 0xA9:
		return QObject::tr("Déplace l'objet 3D sans animation (X=%1, Y=%2)")
				.arg(_svar(params.mid(1,2), B1(params.at(0))))//Cible X (bank 1)
				.arg(_svar(params.mid(3,2), B2(params.at(0))));//Cible Y (bank 2)
		
		case 0xAA:
		return QObject::tr("Déplace l'objet 3D vers le groupe %1")
				.arg(_script((quint8)params.at(0)));//GroupID
		
		case 0xAB:
		return QObject::tr("Rotation de l'objet 3D vers le groupe %1 (vitesse=%2, SensRotation=%3)")
				.arg(_script((quint8)params.at(0)))//GroupID
				.arg(_sensRotation((quint8)params.at(1)))//Sens de rotation
				.arg((quint8)params.at(2));//Vitesse
		
		case 0xAC:
		return QObject::tr("Attendre que l'animation soit terminée pour continuer");
		
		case 0xAD:
		return QObject::tr("Déplace l'objet 3D sans animation (X=%1, Y=%2)")
				.arg(_svar(params.mid(1,2), B1(params.at(0))))//Cible X (bank 1)
				.arg(_svar(params.mid(3,2), B2(params.at(0))));//Cible Y (bank 2)
		
		case 0xAE:
		return QObject::tr("Joue l'animation %1 de l'objet 3D et retourne à l'état précédent (vitesse=%2)")
				.arg((quint8)params.at(0))//animation ID
				.arg((quint8)params.at(1));//vitesse
		
		case 0xAF:
		return QObject::tr("Joue l'animation %1 de l'objet 3D (vitesse=%2)")
				.arg((quint8)params.at(0))//animation ID
				.arg((quint8)params.at(1));//vitesse
		
		case 0xB0:
		return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D et retourne à l'état précédent (première img=%2, dernière img=%3, vitesse=%4)")
				.arg((quint8)params.at(0))//animation ID
				.arg((quint8)params.at(1))//Première img
				.arg((quint8)params.at(2))//Dernière img
				.arg((quint8)params.at(3));//vitesse
		
		case 0xB1:
		return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D (première img=%2, dernière img=%3, vitesse=%4)")
				.arg((quint8)params.at(0))//animation ID
				.arg((quint8)params.at(1))//Première img
				.arg((quint8)params.at(2))//Dernière img
				.arg((quint8)params.at(3));//vitesse
		
		case 0xB2:
		return QObject::tr("Configurer la vitesse des déplacements de l'objet 3D : %1")
				.arg(_var(params.mid(1,2), B2(params.at(0))));//Vitesse (bank 2)
		
		case 0xB3:
		return QObject::tr("Mettre l'objet 3D dans la direction : %1")
				.arg(_var(params.mid(1,1), B2(params.at(0))));//Direction (bank 2)
		
		case 0xB4:
		return QObject::tr("Rotation (direction=%1, nbTours=%2, vitesse=%3, ?=%4)")
				.arg(_var(params.mid(1,1), B2(params.at(0))))//Final Rotation (bank 2)
				.arg((quint8)params.at(2))//nbTours
				.arg((quint8)params.at(3))//Vitesse
				.arg((quint8)params.at(4));//?
		
		case 0xB5:
		return QObject::tr("Rotation inversée (direction=%1, nbTours=%2, vitesse=%3, ?=%4)")
				.arg(_var(params.mid(1,1), B2(params.at(0))))//Final Rotation (bank 2)
				.arg((quint8)params.at(2))//nbTours
				.arg((quint8)params.at(3))//Vitesse
				.arg((quint8)params.at(4));//?
		
		case 0xB6:
		return QObject::tr("Mettre l'objet 3D en direction du groupe %1")
				.arg(_script((quint8)params.at(0)));//GroupID
		
		case 0xB7:
		return QObject::tr("Stocker dans %2 la direction du groupe %1")
				.arg(_script((quint8)params.at(1)))//GroupID
				.arg(_bank(params.at(2), B2(params.at(0))));//(Bank 2)
		
		case 0xB8:
		return QObject::tr("Stocker dans %2 et %3 la position X et Y du groupe %1")
				.arg(_script((quint8)params.at(1)))//GroupID
				.arg(_bank(params.at(2), B1(params.at(0))))//(Bank 1)
				.arg(_bank(params.at(3), B2(params.at(0))));//(Bank 2)
		
		case 0xB9:
		return QObject::tr("Stocker dans %2 le polygone id du groupe %1")
				.arg(_script((quint8)params.at(1)))//GroupID
				.arg(_bank(params.at(2), B2(params.at(0))));//(Bank 2)
		
		case 0xBA:
		return QObject::tr("Joue l'animation %1 de l'objet 3D (vitesse=%2)")
				.arg((quint8)params.at(0))//animation ID
				.arg((quint8)params.at(1));//vitesse
		
		case 0xBB:
		return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D et retourne à l'état précédent (première img=%2, dernière img=%3, vitesse=%4)")
				.arg((quint8)params.at(0))//animation ID
				.arg((quint8)params.at(1))//Première img
				.arg((quint8)params.at(2))//Dernière img
				.arg((quint8)params.at(3));//vitesse
		
		case 0xBC:
		return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D (première img=%2, dernière img=%3, vitesse=%4)")
				.arg((quint8)params.at(0))//animation ID
				.arg((quint8)params.at(1))//Première img
				.arg((quint8)params.at(2))//Dernière img
				.arg((quint8)params.at(3));//vitesse
		
		case 0xBD:
		return QObject::tr("Configurer la vitesse des animations de l'objet 3D : %1")
				.arg(_var(params.mid(1,2), B2(params.at(0))));//Vitesse (bank 2)
		
		case 0xBF:
		return QObject::tr("Prendre le contrôle du groupe %1")
				.arg(_script((quint8)params.at(0)));//GroupID
		
		case 0xC0:
		return QObject::tr("Faire sauter un personnage (X=%1, Y=%2, polygone id=%3, hauteur=%4)")
				.arg(_svar(params.mid(2,2), B1(params.at(0))))//Cible X (bank 1)
				.arg(_svar(params.mid(4,2), B2(params.at(0))))//Cible Y (bank 2)
				.arg(_var(params.mid(6,2), B1(params.at(1))))//Polygone id (bank 3)
				.arg(_svar(params.mid(8,2), B2(params.at(1))));//Height (bank 4)
		
		case 0xC1:
		return QObject::tr("Stocker la position du groupe %1 dans des variables (%2=X, %3=Y, %4=Z, %5=polygone id)")
				.arg(_script((quint8)params.at(2)))//GroupID
				.arg(_bank(params.at(3), B1(params.at(0))))//(bank 1)
				.arg(_bank(params.at(4), B2(params.at(0))))//(bank 2)
				.arg(_bank(params.at(5), B1(params.at(1))))//(bank 3)
				.arg(_bank(params.at(6), B2(params.at(1))));//(bank 4)
		
		case 0xC2:
		return QObject::tr("Monter une échelle avec l'animation %6 (X=%1, Y=%2, Z=%3, polygone id=%4, sens=%5, direction=%7, vitesse=%8)")
				.arg(_svar(params.mid(2,2), B1(params.at(0))))//Cible X (bank 1)
				.arg(_svar(params.mid(4,2), B2(params.at(0))))//Cible Y (bank 2)
				.arg(_svar(params.mid(6,2), B1(params.at(1))))//Cible Z (bank 3)
				.arg(_var(params.mid(8,2), B2(params.at(1))))//Polygone id (bank 4)
				.arg((quint8)params.at(10))//Sens montee/descente
				.arg((quint8)params.at(11))//Animation id
				.arg((quint8)params.at(12))//Direction
				.arg((quint8)params.at(13));//Vitesse
		
		case 0xC3:
		return QObject::tr("Offset Object (mouvement=%1, X=%2, Y=%3, Z=%4, vitesse=%5)")
				.arg((quint8)params.at(2))//Mouvement type
				.arg(_svar(params.mid(3,2), B1(params.at(0))))//Cible X (bank 1)
				.arg(_svar(params.mid(5,2), B2(params.at(0))))//Cible Y (bank 2)
				.arg(_svar(params.mid(7,2), B1(params.at(1))))//Cible Z (bank 3)
				.arg(_var(params.mid(9,2), B2(params.at(1))));//Vitesse (bank 4)
		
		case 0xC4:
		return QObject::tr("Attendre la fin de l'exécution de l'Offset Object pour continuer");
		
		case 0xC5:
		return QObject::tr("Modifier la distance nécessaire pour parler avec l'objet 3D : %1")
				.arg(_var(params.mid(1,1), B2(params.at(0))));//Distance (bank 2)
		
		case 0xC6:
		return QObject::tr("Modifier la distance nécessaire pour toucher l'objet 3D : %1")
				.arg(_var(params.mid(1,1), B2(params.at(0))));//Distance (bank 2)
		
		case 0xC7:
		return QObject::tr("%1 la possibilité de toucher l'objet 3D")
				.arg((quint8)params.at(0) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booléen
		
		case 0xC8:
		return QObject::tr("Ajouter %1 à l'équipe actuelle")
				.arg(_personnage(params.at(0)));//Personnage ID
		
		case 0xC9:
		return QObject::tr("Retirer %1 de l'équipe actuelle")
				.arg(_personnage(params.at(0)));//Personnage ID
		
		case 0xCA:
		return QObject::tr("Nouvelle équipe : %1 | %2 | %3")
				.arg(_personnage(params.at(0)))//Personnage ID
				.arg(_personnage(params.at(1)))//Personnage ID
				.arg(_personnage(params.at(2)));//Personnage ID
		
		case 0xCB://Indenter
			return QObject::tr("Si %1 est dans l'équipe actuelle (aller à l'octet %2 sinon)")
					.arg(_personnage(params.at(0)))//Personnage ID
					.arg(pos+2+(quint8)params.at(1));//Saut court

		case 0xCC://Indenter
			return QObject::tr("Si %1 existe (aller à l'octet %2 sinon)")
					.arg(_personnage(params.at(0)))//Personnage ID
					.arg(pos+2+(quint8)params.at(1));//Saut court

		case 0xCD:
			return QObject::tr("%2 %1")
					.arg((quint8)params.at(0) == 0 ? QObject::tr("n'existe plus") : QObject::tr("existe"))//Booléen
					.arg(_personnage(params.at(1)));//Personnage ID

		case 0xCE:
			return QObject::tr("Bloque %1 dans le menu PHS")
					.arg(_personnage(params.at(0)));//Personnage ID

		case 0xCF:
			return QObject::tr("Débloque %1 dans le menu PHS")
					.arg(_personnage(params.at(0)));//Personnage ID

		case 0xD0:
			return QObject::tr("Définit la zone (X1=%1, Y1=%2, Z1=%3, X2=%4, Y2=%5, Z2=%6)")
					.arg(_toSInt(params.mid(0,2)))//Cible X1
					.arg(_toSInt(params.mid(2,2)))//Cible Y1
					.arg(_toSInt(params.mid(4,2)))//Cible Z1
					.arg(_toSInt(params.mid(6,2)))//Cible X2
					.arg(_toSInt(params.mid(8,2)))//Cible Y2
					.arg(_toSInt(params.mid(10,2)));//Cible Z2

		case 0xD1:
			return QObject::tr("%1 la zone")
					.arg((quint8)params.at(0) == 0 ? QObject::tr("Effacer") : QObject::tr("Tracer"));//Booléen

		case 0xD2:
			return QObject::tr("%1 les changements de décor par le joueur")
					.arg((quint8)params.at(0) == 0 ? QObject::tr("Autoriser") : QObject::tr("Empêcher"));//Booléen

		case 0xD3:
			return QObject::tr("Redimensionner la zone (X1=%1, Y1=%2, Z1=%3, X2=%4, Y2=%5, Z2=%6)")
					.arg(_svar(params.mid(3,2), B1(params.at(0))))//Cible X1 (Bank 1)
					.arg(_svar(params.mid(5,2), B2(params.at(0))))//Cible Y1 (Bank 2)
					.arg(_svar(params.mid(7,2), B1(params.at(1))))//Cible Z1 (Bank 3)
					.arg(_svar(params.mid(9,2), B2(params.at(1))))//Cible X2 (Bank 4)
					.arg(_svar(params.mid(11,2), B1(params.at(2))))//Cible Y2 (Bank 5)
					.arg(_svar(params.mid(13,2), B2(params.at(2))));//Cible Z2 (Bank 6)

		case 0xD4:
			return QObject::tr("%4 = Sinus(%1) %2 %3")
					.arg(_var(params.mid(2,2), B1(params.at(0))))//Variable (bank 1)
					.arg(_var(params.mid(4,2), B2(params.at(0))))//Variable (bank 2)
					.arg(_var(params.mid(6,2), B1(params.at(1))))//Variable (bank 3)
					.arg(_bank(params.at(8), B2(params.at(1))));//Variable (bank 4)

		case 0xD5:
			return QObject::tr("%4 = Cosinus(%1) %2 %3")
					.arg(_var(params.mid(2,2), B1(params.at(0))))//Variable (bank 1)
					.arg(_var(params.mid(4,2), B2(params.at(0))))//Variable (bank 2)
					.arg(_var(params.mid(6,2), B1(params.at(1))))//Variable (bank 3)
					.arg(_bank(params.at(8), B2(params.at(1))));//Variable (bank 4)

		case 0xD6:
			return QObject::tr("Modifier la distance nécessaire pour parler avec l'objet 3D : %1")
					.arg(_var(params.mid(1,2), B2(params.at(0))));//Distance (bank 2)

		case 0xD7:
			return QObject::tr("Modifier la distance nécessaire pour toucher l'objet 3D : %1")
					.arg(_var(params.mid(1,2), B2(params.at(0))));//Distance (bank 2)

		case 0xD8:
			return QObject::tr("Commencer à charger l'écran %1")
					.arg(_field(params.mid(0,2)));//id décor

		case 0xD9:
			return QObject::tr("PMJMP2");//TODO renommer

		case 0xDA:
			return QObject::tr("AKAO2");
			//TODO parametres

		case 0xDB:
			return QObject::tr("%1 rotation")
					.arg((quint8)params.at(0) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booléen

		case 0xDC:
			return QObject::tr("Jouer animation n°%1 pour '%3' (vitesse=%2)")
					.arg((quint8)params.at(0))//animation ID
					.arg((quint8)params.at(1))//vitesse
					.arg((quint8)params.at(2)==0 ? QObject::tr("rester immobile") : ((quint8)params.at(2)==1 ? QObject::tr("marcher") : QObject::tr("courir")));//Stand Walk Run

		case 0xDD:
			return QObject::tr("Stoppe l'animation de l'objet 3D");

		case 0xDE:
			return QObject::tr("Attendre que la rotation soit terminée pour continuer");

		case 0xDF:
			return QObject::tr("MPPAL");
			//TODO parametres

		case 0xE0:
			return QObject::tr("Afficher l'état n°%2 du paramètre n°%1")
					.arg(_var(params.mid(1,1), B1(params.at(0))))//Paramètre (bank 1)
					.arg(_var(params.mid(2,1), B2(params.at(0))));//État (bank 2)

		case 0xE1:
			return QObject::tr("Cacher l'état n°%2 du paramètre n°%1")
					.arg(_var(params.mid(1,1), B1(params.at(0))))//Paramètre (bank 1)
					.arg(_var(params.mid(2,1), B2(params.at(0))));//État (bank 2)

		case 0xE2:
			return QObject::tr("Afficher l'état suivant du paramètre n°%1")
					.arg(_var(params.mid(1,1), B2(params.at(0))));//Paramètre (bank 2)

		case 0xE3:
			return QObject::tr("Afficher l'état précédent du paramètre n°%1")
					.arg(_var(params.mid(1,1), B2(params.at(0))));//Paramètre (bank 2)

		case 0xE4:
			return QObject::tr("Cacher paramètre n°%1")
					.arg(_var(params.mid(1,1), B2(params.at(0))));//Paramètre (bank 2)

		case 0xE5:
			return QObject::tr("STPAL");
			//TODO parametres

		case 0xE6:
			return QObject::tr("LDPAL");
			//TODO parametres

		case 0xE7:
			return QObject::tr("CPPAL");
			//TODO parametres

		case 0xE8:
			return QObject::tr("RTPAL");
			//TODO parametres

		case 0xE9:
			return QObject::tr("ADPAL");
			//TODO parametres

		case 0xEA:
			return QObject::tr("MPPAL2 (R=%1, V=%2, B=%3)")
					.arg(_var(params.mid(5,1), B1(params.at(1))))
					.arg(_var(params.mid(6,1), B2(params.at(1))))
					.arg(_var(params.mid(7,1), B1(params.at(2))));
			//TODO parametres

		case 0xEB:
			return QObject::tr("STPLS");
			//TODO parametres

		case 0xEC:
			return QObject::tr("LDPLS");
			//TODO parametres

		case 0xED:
			return QObject::tr("CPPAL2");
			//TODO parametres

		case 0xEE:
			return QObject::tr("RTPAL2");
			//TODO parametres

		case 0xEF:
			return QObject::tr("ADPAL2");
			//TODO parametres

		case 0xF0:
			return QObject::tr("Jouer musique n°%1")
					.arg((quint8)params.at(0));//Musique ID

		case 0xF1:
			return QObject::tr("Jouer son n°%1 (position=%2/127)")
					.arg(_var(params.mid(1,2), B1(params.at(0))))//Sound ID (bank 1)
					.arg(_var(params.mid(3,1), B2(params.at(0))));//Position (bank 2)

		case 0xF2:
			return QObject::tr("AKAO");
			//TODO parametres

		case 0xF3:
			return QObject::tr("MUSVT (musique n°%1)")
					.arg((quint8)params.at(0));//Musique ID

		case 0xF4:
			return QObject::tr("MUSVM (musique n°%1)")
					.arg((quint8)params.at(0));//Musique ID

		case 0xF5:
			return QObject::tr("%1 musique")
					.arg((quint8)params.at(0) == 0 ? QObject::tr("Déverrouiller") : QObject::tr("Verrouiller"));

		case 0xF6:
			return QObject::tr("Choisir musique n°%1 comme musique de combat")
					.arg((quint8)params.at(0));//Musique ID

		case 0xF7:
			return QObject::tr("CHMPH");
			//TODO parametres

		case 0xF8:
			return QObject::tr("Choisir prochaine cinématique : %1")
					.arg(_movie(params.at(0)));

		case 0xF9:
			return QObject::tr("Jouer la cinématique choisie");

		case 0xFA:
			return QObject::tr("Stocker Movie frame dans %1")
					.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)

		case 0xFB:
			return QObject::tr("Camera Movie : %1")
					.arg((quint8)params.at(0) == 0 ? QObject::tr("ON") : QObject::tr("OFF"));//booléen

		case 0xFC:
			return QObject::tr("FMUSC");
			//TODO parametres

		case 0xFD:
			return QObject::tr("CMUSC");
			//TODO parametres

		case 0xFE:
			return QObject::tr("Si la musique est jouée mettre %1 à 1")
					.arg(_bank(params.at(1), B2(params.at(0))));//(bank 2)

		case 0xFF:
			return QObject::tr("Game Over");

		default:
			return "?";
		}
}
