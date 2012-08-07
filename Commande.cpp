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

quint8 Commande::id() const
{
	return opcode;
}

QString Commande::toString() const
{
	return traduction();
}

QByteArray Commande::params() const
{
	return _params;
}

quint8 Commande::size() const { return params().size() + 1; }

QByteArray &Commande::getParams() { return _params; }

const QByteArray &Commande::getConstParams() const { return _params; }

QByteArray Commande::toByteArray() const
{
	return QByteArray()
			.append((char)id())
			.append(params());
}

quint16 Commande::getIndent() const
{
	switch(opcode)
	{
	case 0x14:	return pos+5+(quint8)params().at(4);
	case 0x15:	return pos+5+_toInt(params().mid(4,2));
	case 0x16:
	case 0x18:	return pos+7+(quint8)params().at(6);
	case 0x17:
	case 0x19:	return pos+7+_toInt(params().mid(6,2));
	case 0x30:
	case 0x31:
	case 0x32:	return pos+3+(quint8)params().at(2);
	case 0xcb:
	case 0xcc:	return pos+2+(quint8)params().at(1);
	}
	return 0;
}

quint16 Commande::getPos() const { return pos; }

void Commande::setCommande(const QByteArray &commande)
{
	this->opcode = commande.size()<1 ? 0 : (quint8)commande.at(0);
	this->_params = commande.mid(1);
}

void Commande::setPos(quint16 pos) { this->pos = pos; }

bool Commande::isVoid() const
{
	return id() == 0 || (id() >= 0x10 && id() <= 0x13);
}

int Commande::subParam(int cur, int paramSize) const
{
	int value, tailleBA;
	
	if(paramSize%8 !=0)
		tailleBA = paramSize/8+1;
	else
		tailleBA = paramSize/8;
	
	memcpy(&value, params().mid(cur/8, tailleBA), tailleBA);
	return (value >> ((tailleBA*8-cur%8)-paramSize)) & ((int)pow(2, paramSize)-1);
}

bool Commande::rechercherVar(quint8 bank, quint8 adress, int value) const
{
	if(value!=65536) {
		if(this->opcode==0x80
			&& B1(_params.at(0)) == bank && (quint8)_params.at(1) == adress
			&& B2(_params.at(0)) == 0 && (quint8)_params.at(2) == value)
		{
			return true;
		}
		if(this->opcode==0x81
			&& B1(_params.at(0)) == bank && (quint8)_params.at(1) == adress
			&& B2(_params.at(0)) == 0 && _toInt(_params.mid(2,2)) == (quint16)value)
		{
			return true;
		}
		return false;
	}

	switch(this->opcode)
	{
	case 0x09:
		if((B1(_params.at(0)) == bank && (quint8)_params.at(3) == adress)
		   || (B2(_params.at(0)) == bank && (quint8)_params.at(5) == adress)
		   || (B1(_params.at(1)) == bank && (quint8)_params.at(7) == adress)
		   || (B2(_params.at(1)) == bank && (quint8)_params.at(8) == adress)
		   || (B1(_params.at(2)) == bank && (quint8)_params.at(10) == adress)
		   || (B2(_params.at(2)) == bank && (quint8)_params.at(12) == adress))
			return true;
		break;
	case 0xD3:
		if((B1(_params.at(0)) == bank && (quint8)_params.at(3) == adress)
		   || (B2(_params.at(0)) == bank && (quint8)_params.at(5) == adress)
		   || (B1(_params.at(1)) == bank && (quint8)_params.at(7) == adress)
		   || (B2(_params.at(1)) == bank && (quint8)_params.at(9) == adress)
		   || (B1(_params.at(2)) == bank && (quint8)_params.at(11) == adress)
		   || (B2(_params.at(2)) == bank && (quint8)_params.at(13) == adress))
			return true;
		break;
	case 0x56:case 0x57:
				if((B1(_params.at(0)) == bank && (quint8)_params.at(2) == adress)
				   || (B2(_params.at(0)) == bank && (quint8)_params.at(3) == adress)
				   || (B1(_params.at(1)) == bank && (quint8)_params.at(4) == adress)
				   || (B2(_params.at(1)) == bank && (quint8)_params.at(5) == adress))
					return true;
		break;
	case 0x75:case 0xC1:
				if((B1(_params.at(0)) == bank && (quint8)_params.at(3) == adress)
				   || (B2(_params.at(0)) == bank && (quint8)_params.at(4) == adress)
				   || (B1(_params.at(1)) == bank && (quint8)_params.at(5) == adress)
				   || (B2(_params.at(1)) == bank && (quint8)_params.at(6) == adress))
					return true;
		break;
	case 0xA5:case 0xD4:case 0xD5:
				if((B1(_params.at(0)) == bank && (quint8)_params.at(2) == adress)
				   || (B2(_params.at(0)) == bank && (quint8)_params.at(4) == adress)
				   || (B1(_params.at(1)) == bank && (quint8)_params.at(6) == adress)
				   || (B2(_params.at(1)) == bank && (quint8)_params.at(8) == adress))
					return true;
		break;
	case 0x0A:case 0x0B:
				if((B1(_params.at(0)) == bank && (quint8)_params.at(2) == adress)
				   || (B2(_params.at(0)) == bank && (quint8)_params.at(3) == adress)
				   || (B1(_params.at(1)) == bank && (quint8)_params.at(4) == adress))
					return true;
		break;
	case 0xA6:case 0xA7:case 0xC0:
				if((B1(_params.at(0)) == bank && (quint8)_params.at(2) == adress)
				   || (B2(_params.at(0)) == bank && (quint8)_params.at(4) == adress)
				   || (B1(_params.at(1)) == bank && (quint8)_params.at(6) == adress))
					return true;
		break;
	case 0x14:case 0x15:case 0x3B:case 0x76:case 0x77:
	case 0x78:case 0x79:case 0x80:case 0x81:case 0x82:
	case 0x83:case 0x84:case 0x85:case 0x86:case 0x87:
	case 0x88:case 0x89:case 0x8a:case 0x8b:case 0x8c:
	case 0x8d:case 0x8e:case 0x8f:case 0x90:case 0x91:
	case 0x92:case 0x93:case 0x94:case 0x9a:case 0x9b:
	case 0xE0:case 0xE1:
		if((B1(_params.at(0)) == bank && (quint8)_params.at(1) == adress)
		   || (B2(_params.at(0)) == bank && (quint8)_params.at(2) == adress))
			return true;
		break;
	case 0x16:case 0x17:case 0x18:case 0x58:case 0x59:
	case 0x5A:case 0x64:case 0x6A:case 0xA8:case 0xA9:
	case 0xAD:case 0xF1:
		if((B1(_params.at(0)) == bank && (quint8)_params.at(1) == adress)
		   || (B2(_params.at(0)) == bank && (quint8)_params.at(3) == adress))
			return true;
		break;
	case 0x2D:
		if((B1(_params.at(0)) == bank && (quint8)_params.at(2) == adress)
		   || (B2(_params.at(0)) == bank && (quint8)_params.at(4) == adress))
			return true;
		break;
	case 0xB8:case 0x9C:
				if((B1(_params.at(0)) == bank && (quint8)_params.at(2) == adress)
				   || (B2(_params.at(0)) == bank && (quint8)_params.at(3) == adress))
					return true;
		break;
	case 0x2C:case 0x37:case 0x39:case 0x3A:
				if(B1(_params.at(0)) == bank && (quint8)_params.at(2) == adress)
					return true;
		break;
	case 0x23:case 0x63:case 0x6E:case 0x6F:case 0x70:
	case 0x7A:case 0x7B:case 0x7C:case 0x7D:case 0x7F:
	case 0x95:case 0x96:case 0x97:case 0x98:case 0x99:
	case 0xB2:case 0xB3:case 0xB4:case 0xB5:case 0xBD:
	case 0xC5:case 0xC6:case 0xD6:case 0xD7:case 0xE2:
	case 0xE3:case 0xE4:case 0xFA:case 0xFE:
		if(B2(_params.at(0)) == bank && (quint8)_params.at(1) == adress)
			return true;
		break;
	case 0x49:case 0x73:case 0x74:case 0xB7:case 0xB9:
				if(B2(_params.at(0)) == bank && (quint8)_params.at(2) == adress)
					return true;
		break;
	case 0x41:case 0x42:
				if(B2(_params.at(0)) == bank && (quint8)_params.at(3) == adress)
					return true;
		break;
	case 0x48:
		if(B2(_params.at(0)) == bank && (quint8)_params.at(5) == adress)
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
		vars.append(B1(_params.at(0)) | (_params.at(3) << 4));
		vars.append(B2(_params.at(0)) | (_params.at(5) << 4));
		vars.append(B1(_params.at(1)) | (_params.at(7) << 4));
		vars.append(B2(_params.at(1)) | (_params.at(8) << 4));
		vars.append(B1(_params.at(2)) | (_params.at(10) << 4));
		vars.append(B2(_params.at(2)) | (_params.at(12) << 4));
		return vars;
	case 0xD3:
		vars.append(B1(_params.at(0)) | (_params.at(3) << 4));
		vars.append(B2(_params.at(0)) | (_params.at(5) << 4));
		vars.append(B1(_params.at(1)) | (_params.at(7) << 4));
		vars.append(B2(_params.at(1)) | (_params.at(9) << 4));
		vars.append(B1(_params.at(2)) | (_params.at(11) << 4));
		vars.append(B2(_params.at(2)) | (_params.at(13) << 4));
		return vars;
	case 0x56:case 0x57:
		vars.append(B1(_params.at(0)) | (_params.at(2) << 4));
		vars.append(B2(_params.at(0)) | (_params.at(3) << 4));
		vars.append(B1(_params.at(1)) | (_params.at(4) << 4));
		vars.append(B2(_params.at(1)) | (_params.at(5) << 4));
		return vars;
	case 0x75:case 0xC1:
		vars.append(B1(_params.at(0)) | (_params.at(3) << 4));
		vars.append(B2(_params.at(0)) | (_params.at(4) << 4));
		vars.append(B1(_params.at(1)) | (_params.at(5) << 4));
		vars.append(B2(_params.at(1)) | (_params.at(6) << 4));
		return vars;
	case 0xA5:case 0xD4:case 0xD5:
		vars.append(B1(_params.at(0)) | (_params.at(2) << 4));
		vars.append(B2(_params.at(0)) | (_params.at(4) << 4));
		vars.append(B1(_params.at(1)) | (_params.at(6) << 4));
		vars.append(B2(_params.at(1)) | (_params.at(8) << 4));
		return vars;
	case 0x0A:case 0x0B:
		vars.append(B1(_params.at(0)) | (_params.at(2) << 4));
		vars.append(B2(_params.at(0)) | (_params.at(3) << 4));
		vars.append(B1(_params.at(1)) | (_params.at(4) << 4));
		return vars;
	case 0xA6:case 0xA7:case 0xC0:
		vars.append(B1(_params.at(0)) | (_params.at(2) << 4));
		vars.append(B2(_params.at(0)) | (_params.at(4) << 4));
		vars.append(B1(_params.at(1)) | (_params.at(6) << 4));
		return vars;
	case 0x14:case 0x15:case 0x3B:case 0x76:case 0x77:
	case 0x78:case 0x79:case 0x80:case 0x81:case 0x82:
	case 0x83:case 0x84:case 0x85:case 0x86:case 0x87:
	case 0x88:case 0x89:case 0x8a:case 0x8b:case 0x8c:
	case 0x8d:case 0x8e:case 0x8f:case 0x90:case 0x91:
	case 0x92:case 0x93:case 0x94:case 0x9a:case 0x9b:
	case 0xE0:case 0xE1:
		vars.append(B1(_params.at(0)) | (_params.at(1) << 4));
		vars.append(B2(_params.at(0)) | (_params.at(2) << 4));
		return vars;
	case 0x16:case 0x17:case 0x18:case 0x58:case 0x59:
	case 0x5A:case 0x64:case 0x6A:case 0xA8:case 0xA9:
	case 0xAD:case 0xF1:
		vars.append(B1(_params.at(0)) | (_params.at(1) << 4));
		vars.append(B2(_params.at(0)) | (_params.at(3) << 4));
		return vars;
	case 0x2D:
		vars.append(B1(_params.at(0)) | (_params.at(2) << 4));
		vars.append(B2(_params.at(0)) | (_params.at(4) << 4));
		return vars;
	case 0xB8:case 0x9C:
		vars.append(B1(_params.at(0)) | (_params.at(2) << 4));
		vars.append(B2(_params.at(0)) | (_params.at(3) << 4));
		return vars;
	case 0x2C:case 0x37:case 0x39:case 0x3A:
		vars.append(B1(_params.at(0)) | (_params.at(2) << 4));
		return vars;
	case 0x23:case 0x63:case 0x6E:case 0x6F:case 0x70:
	case 0x7A:case 0x7B:case 0x7C:case 0x7D:case 0x7F:
	case 0x95:case 0x96:case 0x97:case 0x98:case 0x99:
	case 0xB2:case 0xB3:case 0xB4:case 0xB5:case 0xBD:
	case 0xC5:case 0xC6:case 0xD6:case 0xD7:case 0xE2:
	case 0xE3:case 0xE4:case 0xFA:case 0xFE:
		vars.append(B2(_params.at(0)) | (_params.at(1) << 4));
		return vars;
	case 0x49:case 0x73:case 0x74:case 0xB7:case 0xB9:
		vars.append(B2(_params.at(0)) | (_params.at(2) << 4));
		return vars;
	case 0x41:case 0x42:
		vars.append(B2(_params.at(0)) | (_params.at(3) << 4));
		return vars;
	case 0x48:
		vars.append(B2(_params.at(0)) | (_params.at(5) << 4));
		return vars;
	default:
		return vars;
	}
}

bool Commande::rechercherExec(quint8 group, quint8 script) const
{
	return (opcode==0x01 || opcode==0x02 || opcode==0x03)
			&& (quint8)_params.at(0) == group && (quint8)(_params.at(1) & 0x1F) == script;
}

bool Commande::rechercherTexte(const QRegExp &texte) const
{
	qint16 textID = -1;
	switch(this->opcode)
	{
	case 0x0F:	if((quint8)_params.at(0)==0xFD)	textID = (quint8)_params.at(2);	break;
	case 0x40:	textID = (quint8)_params.at(1);	break;
	case 0x43:	textID = (quint8)_params.at(0);	break;
	case 0x48:	textID = (quint8)_params.at(2);	break;
	}
	return textID != -1 && textID < Data::currentTextes->size() && Data::currentTextes->at(textID)->search(texte);
}

void Commande::listUsedTexts(QSet<quint8> &usedTexts) const
{
	switch(opcode)
	{
	case 0x0F://SPECIAL
		if((quint8)_params.at(0)==0xFD)
			usedTexts.insert(_params.at(2));
		break;
	case 0x40://MESSAGE
		usedTexts.insert(_params.at(1));
		break;
	case 0x43://MAP NAME
		usedTexts.insert(_params.at(0));
		break;
	case 0x48://ASK
		usedTexts.insert(_params.at(2));
		break;
	}
}

void Commande::listUsedTuts(QSet<quint8> &usedTuts) const
{
	switch(opcode)
	{
	case 0x21://TUTOR
		usedTuts.insert(_params.at(0));
		break;
	}
}

void Commande::shiftTextIds(int textId, int steps)
{
	switch(opcode)
	{
	case 0x0F://SPECIAL
		if((quint8)_params.at(0)==0xFD && (quint8)_params.at(2)>textId) {
			_params[2] = (quint8)_params.at(2) + steps;
		}
		break;
	case 0x40://MESSAGE
		if((quint8)_params.at(1)>textId) {
			_params[1] = (quint8)_params.at(1) + steps;
		}
		break;
	case 0x43://MAP NAME
		if((quint8)_params.at(0)>textId) {
			_params[0] = (quint8)_params.at(0) + steps;
		}
		break;
	case 0x48://ASK
		if((quint8)_params.at(2)>textId) {
			_params[2] = (quint8)_params.at(2) + steps;
		}
		break;
	}
}

void Commande::shiftTutIds(int tutId, int steps)
{
	switch(opcode)
	{
	case 0x21://TUTOR
		if((quint8)_params.at(0)>tutId) {
			_params[0] = (quint8)_params.at(0) + steps;
		}
		break;
	}
}

void Commande::listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const
{
	FF7Window win;
	const char *_params = this->_params.constData();

	memset(&win, 0, sizeof(FF7Window));
	win.type = opcode;

	switch(opcode)
	{
	case 0x2F: // WSIZW
	case 0x50: // WINDOW
		memcpy(&win.x, &_params[1], 2);
		memcpy(&win.y, &_params[3], 2);
		memcpy(&win.w, &_params[5], 2);
		memcpy(&win.h, &_params[7], 2);

		windows.insert(_params[0], win);// winID
		break;
	case 0x51: // WMOVE
		memcpy(&win.w, &_params[1], 2);
		memcpy(&win.h, &_params[3], 2);

		windows.insert(_params[0], win);// winID
		break;
	case 0x40: // MESSAGE
		text2win.insert(_params[1], _params[0]);// textID, winID
		break;
	}
}

QString Commande::toString2() const
{
	QString ret;
	quint8 depart = opcode == 0x28;
	for(quint8 i=depart ; i<_params.size()-1 ; ++i)
		ret += QString("%1,").arg((quint8)_params.at(i));
	if(_params.size()-depart>0) ret += QString("%1").arg((quint8)_params.at(_params.size()-1));
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
	return _item(_toInt(param), bank);
}

QString Commande::_item(quint16 itemID, quint8 bank)
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

QString Commande::_materia(const QByteArray &param, quint8 bank)
{
	return _materia(_toInt(param), bank);
}

QString Commande::_materia(quint8 materiaID, quint8 bank)
{
	if(bank > 0)	return QObject::tr("n°%1").arg(_bank(materiaID, bank));

	if(materiaID < Data::materia_names.size())	return Data::materia_names.at(materiaID);
	return QObject::tr("n°%1").arg(materiaID);
}

QString Commande::_field(const QByteArray &param)
{
	return _field(_toInt(param));
}

QString Commande::_field(quint16 fieldID)
{
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

QString Commande::_var(int value, quint8 bank)
{
	if(bank > 0)	return _bank(value & 0xFF, bank);
	return QString::number(value);
}

QString Commande::_var(int value, quint8 bank1, quint8 bank2)
{
	if(bank > 0 || bank2 > 0)
		return _var(value & 0xFFFF, bank1) + QString(" + ") + _var((value >> 16) & 0xFFFF, bank2) + QString(" * 65536 ");
	return QString::number(value);
}

QString Commande::_var(int value, quint8 bank1, quint8 bank2, quint8 bank3)
{
	if(bank > 0 || bank2 > 0 || bank3 > 0)
		return _bank(value & 0xFF, bank1) + QObject::tr(" et ") + _bank((value >> 8) & 0xFF, bank2) + QObject::tr(" et ") + _bank((value >> 16) & 0xFF, bank3);
	return QString::number(value);
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
	switch((quint8)_params.at(0))
	{
	case 0xF5:
		return QObject::tr("%1 le curseur main").arg((quint8)_params.at(1) == 0 ? QObject::tr("Afficher") : QObject::tr("Ne pas afficher"));//Booleen
	case 0xF6:
		return QObject::tr("PNAME - Désactiver le menu de droite (%1)").arg((quint8)_params.at(1));//DEBUG
	case 0xF7:
		return QObject::tr("GMSPD | %1 |").arg((quint8)_params.at(1));//DEBUG
	case 0xF8:
		return QObject::tr("Modifier la vitesse des messages (%2) | %1 |")
				.arg((quint8)_params.at(1))//DEBUG
				.arg((quint8)_params.at(2));//Vitesse
	case 0xF9:
		return QObject::tr("Remplir le menu matéria de toutes les matérias en quantité maximum");
	case 0xFA:
		return QObject::tr("Remplir l'inventaire par tous les objets en quantité maximum");
	case 0xFB:
		return QObject::tr("%1 les combats").arg((quint8)_params.at(1) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booleen
	case 0xFC:
		return QObject::tr("%1 les cinématiques").arg((quint8)_params.at(1) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booleen
	case 0xFD:
		return QObject::tr("Changer le nom de %1 par le texte %2")
				.arg(_personnage(_params.at(1)))//Personnage ID
				.arg(_text(_params.at(2)));//Texte ID
	case 0xFE:
		return QObject::tr("Met le temps à 0, débloque le menu \"PHS\" et \"Sauvegarder\". Nouvelle équipe : Clad | (Vide) | (Vide)");
	case 0xFF:
		return QObject::tr("Supprimer tous les objets de l'inventaire");
	default:
		return QString("%1?").arg((quint8)_params.at(0),0,16);
	}
}

QString Commande::_kawai() const
{
	if(_params.size()>1)
	{
		switch((quint8)_params.at(1))
		{
		case 0x00:
			return "EYETX";
		case 0x01:
			return QObject::tr("%1 transparence").arg(_params.size() > 2 && (quint8)_params.at(2) == 0 ? QObject::tr("Désactiver") : QObject::tr("Activer"));
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
			return QString("%1?").arg((quint8)_params.at(1),0,16);
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
				.arg(_script((quint8)_params.at(0)))//GroupID
				.arg(((quint8)_params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)_params.at(1) & 0x1F);//ScriptID sur 5 bits

	case 0x02:
		return QObject::tr("Exécuter le script n°%3 du groupe %1 (priorité %2/6)")
				.arg(_script((quint8)_params.at(0)))//GroupID
				.arg(((quint8)_params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)_params.at(1) & 0x1F);//ScriptID sur 5 bits

	case 0x03:
		return QObject::tr("Exécuter le script n°%3 du groupe externe %1 (priorité %2/6) - Attend la fin de l'exécution pour continuer")
				.arg(_script((quint8)_params.at(0)))//GroupID
				.arg(((quint8)_params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)_params.at(1) & 0x1F);//ScriptID sur 5 bits
		
	case 0x04:
		return QObject::tr("Exécuter le script n°%3 du groupe lié au personnage n°%1 de l'équipe (priorité %2/6) - Seulement si le script n'est pas déjà en cours d'exécution")
				.arg((quint8)_params.at(0))//ID personnage d'équipe sur 8 bits
				.arg(((quint8)_params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)_params.at(1) & 0x1F);//ScriptID sur 5 bits
		
	case 0x05:
		return QObject::tr("Exécuter le script n°%3 du groupe lié au personnage n°%1 de l'équipe (priorité %2/6)")
				.arg((quint8)_params.at(0))//ID personnage d'équipe sur 8 bits
				.arg(((quint8)_params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)_params.at(1) & 0x1F);//ScriptID sur 5 bits
		
	case 0x06:
		return QObject::tr("Exécuter le script n°%3 du groupe lié au personnage n°%1 de l'équipe (priorité %2/6) - Attend la fin de l'exécution pour continuer")
				.arg((quint8)_params.at(0))//ID personnage d'équipe sur 8 bits
				.arg(((quint8)_params.at(1) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)_params.at(1) & 0x1F);//ScriptID sur 5 bits
		
	case 0x07:
		return QObject::tr("Return et exécuter le script n°%2 du groupe appelant (priorité %1/6)")
				.arg(((quint8)_params.at(0) >> 5) & 7)//Priorité sur 3 bits
				.arg((quint8)_params.at(0) & 0x1F);//ScriptID sur 5 bits
		
	case 0x08:
		return QObject::tr("Rassembler les membres de l'équipe dans le personnage jouable (vitesse=%1)")
				.arg((quint8)_params.at(0));//Vitesse
		
	case 0x09:
		return QObject::tr("Faire sortir les membres de l'équipe à partir du personnage jouable (perso 1 : X=%1, Y=%2, dir=%3 ; perso 2 : X=%4, Y=%5, dir=%6) (vitesse %7)")
				.arg(_svar(_params.mid(3,2), B1(_params.at(0))))//Cible X1 (bank 1)
				.arg(_svar(_params.mid(5,2), B2(_params.at(0))))//Cible Y1 (bank 2)
				.arg(_var(_params.mid(7,1), B1(_params.at(1))))//Direction 1 (bank 3)
				.arg(_svar(_params.mid(8,2), B2(_params.at(1))))//Cible X2 (bank 4)
				.arg(_svar(_params.mid(10,2), B1(_params.at(2))))//Cible Y2 (bank 5)
				.arg(_var(_params.mid(12,1), B2(_params.at(2))))//Direction 2 (bank 6)
				.arg((quint8)_params.at(13));//Vitesse
		
	case 0x0A:
		return QObject::tr("Sauvegarder les membres de l'équipe : %1 | %2 | %3")
				.arg(_var(_params.mid(2,1), B1(_params.at(0))))//party 1 (bank 1)
				.arg(_var(_params.mid(3,1), B2(_params.at(0))))//party 2 (bank 2)
				.arg(_var(_params.mid(4,1), B1(_params.at(1))));//party 3 (bank 3) -vérifié-
		
	case 0x0B:
		return QObject::tr("Récupérer les membres de l'équipe : %1 | %2 | %3")
				.arg(_var(_params.mid(2,1), B1(_params.at(0))))//party 1 (bank 1)
				.arg(_var(_params.mid(3,1), B2(_params.at(0))))//party 2 (bank 2)
				.arg(_var(_params.mid(4,1), B1(_params.at(1))));//party 3 (bank 3) -vérifié-
		
	case 0x0E:
		return QObject::tr("Demander le CD %1")
				.arg((quint8)_params.at(0));//num CD
		
	case 0x0F:
		return QObject::tr("SPECIAL - ") + _special();
		
	case 0x10:
		return QObject::tr("Aller à l'octet %1 du script")
				.arg(pos+1+(quint8)_params.at(0));//Saut court
		
	case 0x11:
		return QObject::tr("Aller à l'octet %1 du script")
				.arg(pos+1+_toSInt(_params.mid(0,2)));//Saut long
		
	case 0x12:
		return QObject::tr("Aller à l'octet %1 du script")
				.arg((int)(pos-(quint8)_params.at(0)));//Saut court en arrière
		
	case 0x13:
		return QObject::tr("Aller à l'octet %1 du script")
				.arg((int)(pos-_toInt(_params.mid(0,2))));//Saut long en arrière
		
	case 0x14://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_var(_params.mid(1,1), B1(_params.at(0))))//Valeur courte (bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))))//Valeur courte (bank 2)
				.arg(_operateur(_params.at(3)))//Opérateur
				.arg(pos+5+(quint8)_params.at(4))//Saut court
				.arg((quint8)_params.at(3)==9 || (quint8)_params.at(3)==10 ? ")" : "");//mini hack ")"
		
	case 0x15://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_var(_params.mid(1,1), B1(_params.at(0))))//Valeur courte (bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))))//Valeur courte (bank 2)
				.arg(_operateur(_params.at(3)))//Opérateur
				.arg(pos+5+_toInt(_params.mid(4,2)))//Saut long
				.arg((quint8)_params.at(3)==9 || (quint8)_params.at(3)==10 ? ")" : "");//mini hack ")"
		
	case 0x16://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_svar(_params.mid(1,2), B1(_params.at(0))))//Valeur signée (bank 1)
				.arg(_svar(_params.mid(3,2), B2(_params.at(0))))//Valeur signée (bank 2)
				.arg(_operateur(_params.at(5)))//Opérateur
				.arg(pos+7+(quint8)_params.at(6))//Saut court
				.arg((quint8)_params.at(5)==9 || (quint8)_params.at(5)==10 ? ")" : "");//mini hack ")"
		
	case 0x17://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_svar(_params.mid(1,2), B1(_params.at(0))))//Valeur signée (bank 1)
				.arg(_svar(_params.mid(3,2), B2(_params.at(0))))//Valeur signée (bank 2)
				.arg(_operateur(_params.at(5)))//Opérateur
				.arg(pos+7+_toInt(_params.mid(6,2)))//Saut long
				.arg((quint8)_params.at(5)==9 || (quint8)_params.at(5)==10 ? ")" : "");//mini hack ")"
		
	case 0x18://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_var(_params.mid(1,2), B1(_params.at(0))))//Valeur (bank 1)
				.arg(_var(_params.mid(3,2), B2(_params.at(0))))//Valeur (bank 2)
				.arg(_operateur(_params.at(5)))//Opérateur
				.arg(pos+7+(quint8)_params.at(6))//Saut court
				.arg((quint8)_params.at(5)==9 || (quint8)_params.at(5)==10 ? ")" : "");//mini hack ")"
		
	case 0x19://Indenter
		return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
				.arg(_var(_params.mid(1,2), B1(_params.at(0))))//Valeur (bank 1)
				.arg(_var(_params.mid(3,2), B2(_params.at(0))))//Valeur (bank 2)
				.arg(_operateur(_params.at(5)))//Opérateur
				.arg(pos+7+_toInt(_params.mid(6,2)))//Saut long
				.arg((quint8)_params.at(5)==9 || (quint8)_params.at(5)==10 ? ")" : "");//mini hack ")"
		
	case 0x20:
		return QObject::tr("Lancer un mini-jeu : %5 (Après le jeu aller à l'écran %1 (X=%2, Y=%3, polygone id=%4))")
				.arg(_field(_params.mid(0,2)))//id décor
				.arg(_toSInt(_params.mid(2,2)))//cible X
				.arg(_toSInt(_params.mid(4,2)))//cible Y
				.arg(_toInt(_params.mid(6,2)))//polygone id
				.arg(_miniGame(_params.at(9), _params.at(8)));//Mini-jeu
		
	case 0x21:
		return QObject::tr("Lancer le tutoriel n°%1")
				.arg((quint8)_params.at(0));//id tutoriel
		
	case 0x22:
		return QObject::tr("Mode de combat : %1")
				.arg(_battleMode(_toInt(_params.mid(0,4))));//battleMode
		
	case 0x23:
		return QObject::tr("Stocker le résultat du dernier combat dans %1")
				.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)
		
	case 0x24:
		return QObject::tr("Attendre %1 img")
				.arg(_toInt(_params.mid(0,2)));//Valeur
		
	case 0x25:
		return QObject::tr("Voiler l'écran avec la couleur RVB(%1, %2, %3)")
				.arg(_var(_params.mid(3,1), B1(_params.at(0))))//Rouge (bank 1 ???)
				.arg(_var(_params.mid(4,1), B2(_params.at(0))))//Vert (bank 2 ???)
				.arg(_var(_params.mid(5,1), B1(_params.at(1))));//Bleu (bank 3 ???)
		// .arg((quint8)_params.at(6));//Vitesse
		//Type
		//Adjust
		
	case 0x26:
		return QObject::tr("L'objet 3D cligne des yeux : %1")
				.arg((quint8)_params.at(0) == 0 ? QObject::tr("ON") : QObject::tr("OFF"));//booleen
		
	case 0x27:
		return QObject::tr("BGMOVIE : %1")
				.arg((quint8)_params.at(0) == 0 ? QObject::tr("ON") : QObject::tr("OFF"));//booleen
		
	case 0x28:
		return QObject::tr("Filtre graphique sur l'objet 3D - ") + _kawai();//KAWAI
		
	case 0x29:
		return QObject::tr("Attendre la fin de l'exécution du filtre graphique");
		
	case 0x2A:
		return QObject::tr("Déplacer l'objet 3D vers le membre n°%1 de l'équipe")
				.arg((quint8)_params.at(0));//Membre d'équipe
		
	case 0x2B:
		return QObject::tr("SLIP : %1")
				.arg((quint8)_params.at(0) == 0 ? QObject::tr("ON") : QObject::tr("OFF"));
		
	case 0x2C:
		return QObject::tr("Déplacer la couche %1 du décor (Z=%2)")
				.arg((quint8)_params.at(1))//Couche ID
				.arg(_svar(_params.mid(2,2), B2((quint8)_params.at(0))));//Cible Z (bank 2 ???)
		
	case 0x2D:
		return QObject::tr("Animer la couche %1 du décor (horizontalement=%2, verticalement=%3)")
				.arg((quint8)_params.at(1))//Couche ID
				.arg(_svar(_params.mid(2,2), B1(_params.at(0))))//Cible X (bank 1)
				.arg(_svar(_params.mid(4,2), B2(_params.at(0))));//Cible Y (bank 2)
		
	case 0x2E:
		return QObject::tr("WCLS (fenêtre n°%1)")
				.arg((quint8)_params.at(0));//Fenêtre ID
		
	case 0x2F:
		return QObject::tr("Redimensionner fenêtre n°%1 (X=%2, Y=%3, largeur=%4, hauteur=%5)")
				.arg((quint8)_params.at(0))//Fenêtre ID
				.arg(_toInt(_params.mid(1,2)))//Cible X
				.arg(_toInt(_params.mid(3,2)))//Cible Y
				.arg(_toInt(_params.mid(5,2)))//Largeur
				.arg(_toInt(_params.mid(7,2)));//Hauteur
		
	case 0x30://Indenter
		return QObject::tr("Si appuie sur la touche %1 (aller à l'octet %2 du script sinon)")
				.arg(_key(_toInt(_params.mid(0,2))))//touches
				.arg(pos+3+(quint8)_params.at(2));//Saut court
		
	case 0x31://Indenter
		return QObject::tr("Si appuie sur la touche %1 une fois (aller à l'octet %2 du script sinon)")
				.arg(_key(_toInt(_params.mid(0,2))))//touches
				.arg(pos+3+(quint8)_params.at(2));//Saut court
		
	case 0x32://Indenter
		return QObject::tr("Si relache la touche %1 pour la première fois (aller à l'octet %2 du script sinon)")
				.arg(_key(_toInt(_params.mid(0,2))))//touches
				.arg(pos+3+(quint8)_params.at(2));//Saut court
		
	case 0x33:
		return QObject::tr("%1 les déplacements du personnage jouable")
				.arg((quint8)_params.at(0) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booleen
		
	case 0x34:
		return QObject::tr("Tourner instantanément l'objet 3D vers le membre de l'équipe n°%1")
				.arg((quint8)_params.at(0));//Party ID
		
	case 0x35:
		return QObject::tr("Tourner l'objet 3D vers le membre de l'équipe n°%1 (Vitesse=%2, SensRotation=%3)")
				.arg((quint8)_params.at(0))//Party ID
				.arg((quint8)_params.at(1))//Vitesse
				.arg(_sensRotation((quint8)_params.at(2)));//Sens de rotation
		
	case 0x36:
		return QObject::tr("%2 dans fenêtre n°%1 (gauche=%3, haut=%4)")
				.arg((quint8)_params.at(0))//Fenêtre ID
				.arg(_windowNum((quint8)_params.at(1)))//Affichage numérique
				.arg((quint8)_params.at(2))//Margin-left
				.arg((quint8)_params.at(3));//Margin-top
		
	case 0x37:
		return QObject::tr("Affecter %2 dans la fenêtre n°%1 et afficher %3 chiffres")
				.arg((quint8)_params.at(1))//Fenêtre ID
				.arg(_lvar(_params.mid(2,4), B1(_params.at(0)), B2(_params.at(0))))//Valeur (bank 1 et 2)
				.arg((quint8)_params.at(6));//nombre de chiffres à afficher
		
	case 0x38:
		return QObject::tr("Affecter une valeur au compte à rebours (H=%1, M=%2, S=%3)")
				.arg(_var(_params.mid(2,1), B1(_params.at(0))))//Valeur (bank 1 ???)
				.arg(_var(_params.mid(3,1), B2(_params.at(0))))//Valeur (bank 2 ???)
				.arg(_var(_params.mid(4,1), B1(_params.at(1))));//Valeur (bank 3 ???)
		
	case 0x39:
		return QObject::tr("Ajouter %1 gils à l'équipe")
				.arg(_lvar(_params.mid(1,4), B1(_params.at(0)), B2(_params.at(0))));//Valeur (bank 1 et 2)
		
	case 0x3A:
		return QObject::tr("Retirer %1 gils à l'équipe")
				.arg(_lvar(_params.mid(1,4), B1(_params.at(0)), B2(_params.at(0))));//Valeur (bank 1 et 2)
		
	case 0x3B:
		return QObject::tr("Copier le nombre de Gils dans %1 et %2")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_bank(_params.at(2), B2(_params.at(0))));//(bank 2)
		
	case 0x3C:case 0x3D:case 0x3F:
		return QObject::tr("Redonne les HP/MP aux membres de l'équipe");
		
	case 0x3E:
		return QObject::tr("Redonne les HP/MP à tous et soigne les troubles de statut");
		
	case 0x40:
		return QObject::tr("Afficher message %2 dans la fenêtre n°%1")
				.arg((quint8)_params.at(0))//Fenêtre ID
				.arg(_text(_params.at(1)));//Texte
		
	case 0x41:
		return QObject::tr("Affecter %3 à la variable n°%2 dans la fenêtre n°%1")
				.arg((quint8)_params.at(1))//Fenêtre ID
				.arg((quint8)_params.at(2))//Win variable ID
				.arg(_var(_params.mid(3,1), B2(_params.at(0))));//Valeur (bank 2)
		
	case 0x42:
		return QObject::tr("Affecter %3 à la variable n°%2 dans la fenêtre n°%1")
				.arg((quint8)_params.at(1))//Fenêtre ID
				.arg((quint8)_params.at(2))//Win variable ID
				.arg(_var(_params.mid(3,2), B2(_params.at(0))));//Valeur (bank 2)
		
	case 0x43:
		return QObject::tr("Afficher %1 dans le menu")
				.arg(_text(_params.at(0)));//Texte
		
	case 0x45:
		return QObject::tr("Augmenter de %2 MPs le membre n°%1 de l'équipe")
				.arg((quint8)_params.at(1))//Party ID
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2 ???)
		
	case 0x47:
		return QObject::tr("Diminuer de %2 MPs le membre n°%1 de l'équipe")
				.arg((quint8)_params.at(1))//Party ID
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2 ???)
		
	case 0x48:
		return QObject::tr("Poser question %2 dans la fenêtre n°%1 (et mettre la réponse sélectionnée dans %5) première ligne=%3, dernière ligne=%4")
				.arg((quint8)_params.at(1))//Fenêtre ID
				.arg(_text(_params.at(2)))//Texte
				.arg((quint8)_params.at(3))//première ligne
				.arg((quint8)_params.at(4))//dernière ligne
				.arg(_bank(_params.at(5), B2(_params.at(0))));//(bank 2)
		
	case 0x49:
		return QObject::tr("Afficher menu %1")
				.arg(_menu(_params.at(1), _var(_params.mid(2,1), B2(_params.at(0)))));//Menu ID, Valeur (bank 2)
		
	case 0x4A:
		return QObject::tr("%1 l'accès aux menus")
				.arg((quint8)_params.at(0) == 0 ? QObject::tr("Permettre") : QObject::tr("Interdire"));//booleen
		
	case 0x4B:
		return QObject::tr("Choisir la battle table : %1")
				.arg((quint8)_params.at(0));//battle table ID
		
	case 0x4D:
		return QObject::tr("Augmenter de %2 HPs le membre n°%1 de l'équipe")
				.arg((quint8)_params.at(1))//Party ID
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2 ???)
		
	case 0x4F:
		return QObject::tr("Diminuer de %2 HPs le membre n°%1 de l'équipe")
				.arg((quint8)_params.at(1))//Party ID
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2 ???)
		
	case 0x50:
		return QObject::tr("Créer la fenêtre n°%1 (X=%2, Y=%3, largeur=%4, hauteur=%5)")
				.arg((quint8)_params.at(0))//Fenêtre ID
				.arg(_toInt(_params.mid(1,2)))//Cible X
				.arg(_toInt(_params.mid(3,2)))//Cible Y
				.arg(_toInt(_params.mid(5,2)))//Largeur
				.arg(_toInt(_params.mid(7,2)));//Hauteur
		
	case 0x51:
		return QObject::tr("Déplacer la fenêtre n°%1 (X=%2, Y=%3)")
				.arg((quint8)_params.at(0))//Fenêtre ID
				.arg(_toInt(_params.mid(1,2)))//Cible X
				.arg(_toInt(_params.mid(3,2)));//Cible Y
		
	case 0x52:
		return QObject::tr("Décoration de la fenêtre n°%1 : %2 (%3 la fermeture de la fenêtre par le joueur)")
				.arg((quint8)_params.at(0))//Fenêtre ID
				.arg(_windowType((quint8)_params.at(1)))//Mode
				.arg((quint8)_params.at(2) == 0 ? QObject::tr("autoriser") : QObject::tr("empêcher"));//booleen
		
	case 0x53:
		return QObject::tr("Remettre la fenêtre n°%1 à zéro")
				.arg((quint8)_params.at(0));//Fenêtre ID
		
	case 0x54:
		return QObject::tr("Fermer la fenêtre n°%1")
				.arg((quint8)_params.at(0));//Fenêtre ID
		
	case 0x55:
		return QObject::tr("Configurer le nombre de lignes de texte à %2 dans la fenêtre n°%1")
				.arg((quint8)_params.at(0))//Fenêtre ID
				.arg((quint8)_params.at(1));//Ligne
		
	case 0x56:
		return QObject::tr("Obtenir la couleur du côté %1 des fenêtres et en stocker les composantes dans %2 (R), %3 (V) et %4 (B)")
				.arg(_windowCorner(_params.at(2), B1(_params.at(0))))//Côté (bank 1)
				.arg(_var(_params.mid(3,1), B2(_params.at(0))))//Rouge (bank 2)
				.arg(_var(_params.mid(4,1), B1(_params.at(1))))//Vert (bank 3)
				.arg(_var(_params.mid(5,1), B2(_params.at(1))));//Bleu (bank 4)
		
	case 0x57:
		return QObject::tr("Changer la couleur du côté %1 des fenêtres : RVB(%2, %3, %4)")
				.arg(_windowCorner(_params.at(2), B1(_params.at(0))))//Côté (bank 1)
				.arg(_var(_params.mid(3,1), B2(_params.at(0))))//Rouge (bank 2)
				.arg(_var(_params.mid(4,1), B1(_params.at(1))))//Vert (bank 3)
				.arg(_var(_params.mid(5,1), B2(_params.at(1))));//Bleu (bank 4)
		
	case 0x58:
		return QObject::tr("Ajouter %2 objet(s) %1 dans l'inventaire")
				.arg(_item(_params.mid(1,2), B1(_params.at(0))))//Item ID (bank 1)
				.arg(_var(_params.mid(3,1), B2(_params.at(0))));//Quantité (bank 2)
		
	case 0x59:
		return QObject::tr("Supprimer %2 objet(s) %1 dans l'inventaire")
				.arg(_item(_params.mid(1,2), B1(_params.at(0))))//Item ID (bank 1)
				.arg(_var(_params.mid(3,1), B2(_params.at(0))));//Quantité (bank 2)
		
	case 0x5A:
		return QObject::tr("%2 = quantité d'objets %1 dans l'inventaire")
				.arg(_item(_params.mid(1,2), B1(_params.at(0))))//Item ID (bank 1)
				.arg(_bank(_params.at(3), B2(_params.at(0))));//(bank 2)
		
		case 0x5B:
		return QObject::tr("Ajouter la matéria %1 dans l'inventaire (AP=%2)")
				.arg(_materia(_params.mid(2,1), B1(_params.at(0))))//Materia ID (bank 1)
				.arg(_lvar(_params.mid(3,3), B2(_params.at(0)), B1(_params.at(1)), B2(_params.at(1))));//Materia AP (bank 2, 3 et 4 ???)
		
		case 0x5C:
		return QObject::tr("Supprimer %3 matéria(s) %1 dans l'inventaire (AP=%2)")
				.arg(_materia(_params.mid(2,1), B1(_params.at(0))))//Materia ID (bank 1)
				.arg(_lvar(_params.mid(3,3), B2(_params.at(0)), B1(_params.at(1)), B2(_params.at(1))))//Materia AP (bank 2, 3 et 4 ???)
				.arg((quint8)_params.at(6));//Quantité
		
		case 0x5D:
		return QObject::tr("%4 = quantité de matéria %1 dans l'inventaire (AP=%2, ???=%3)")
				.arg(_materia(_params.mid(3,1), B1(_params.at(0))))//Materia ID (bank 1)
				.arg(_lvar(_params.mid(4,3), B2(_params.at(0)), B1(_params.at(1)), B2(_params.at(1))))//Materia AP (bank 2, 3 et 4 ???)
				.arg((quint8)_params.at(7))//TODO ??
				.arg(_bank(_params.at(8), B2(_params.at(2))));//(bank 5)
		
		case 0x5E:
		return QObject::tr("Secouer l'écran (nbOscillations=%1, Amplitude=%2, vitesse=%3)")
				.arg((quint8)_params.at(2))//nbOscillations???
				.arg((quint8)_params.at(5))//Amplitude
				.arg((quint8)_params.at(6));//Vitesse
		
		case 0x5F:
		return QObject::tr("Ne rien faire...");
		
		case 0x60:
		return QObject::tr("Aller à l'écran %1 (X=%2, Y=%3, polygone id=%4, direction=%5)")
				.arg(_field(_params.mid(0,2)))//id décor
				.arg(_toSInt(_params.mid(2,2)))//cible X
				.arg(_toSInt(_params.mid(4,2)))//cible Y
				.arg(_toInt(_params.mid(6,2)))//polygone id
				.arg((quint8)_params.at(8));//direction

		case 0x61:
		return QObject::tr("SCRLO");
		
		case 0x62:
		return QObject::tr("SCRLC");
		
		case 0x63:
		return QObject::tr("Centrer sur le groupe %2 (vitesse=%1, type=%3)")
				.arg(_var(_params.mid(1,2), B2(_params.at(0))))//Vitesse (bank 2)
				.arg(_script((quint8)_params.at(3)))//GroupID
				.arg((quint8)_params.at(4));//ScrollType
		
		case 0x64:
		return QObject::tr("Centrer sur zone (X=%1, Y=%2)")
				.arg(_svar(_params.mid(1,2), B1(_params.at(0))))//Cible X (bank 1)
				.arg(_svar(_params.mid(3,2), B2(_params.at(0))));//Cible Y (bank 2)
		
		case 0x65:
		return QObject::tr("Centrer sur le personnage jouable");
		
		case 0x66:
		return QObject::tr("Centrer sur zone (X=%1, Y=%2, vitesse=%3)")
				.arg(_svar(_params.mid(2,2), B1(_params.at(0))))//Cible X (bank 1 ???)
				.arg(_svar(_params.mid(4,2), B2(_params.at(0))))//Cible Y (bank 2 ???)
				.arg(_var(_params.mid(6,2), B2(_params.at(1))));//Vitesse (bank 4 ???)
		
		case 0x67:
		return QObject::tr("Attendre la fin du dernier centrage pour continuer");
		
		case 0x68:
		return QObject::tr("Centrer sur zone (X=%1, Y=%2, vitesse=%3)")
				.arg(_svar(_params.mid(2,2), B1(_params.at(0))))//Cible X (bank 1 ???)
				.arg(_svar(_params.mid(4,2), B2(_params.at(0))))//Cible Y (bank 2 ???)
				.arg(_var(_params.mid(6,2), B2(_params.at(1))));//Vitesse (bank 4 ???)
		
		case 0x69:
		return QObject::tr("MPDSP : %1")
				.arg((quint8)_params.at(0));//booléen ?
		
		case 0x6A:
		return QObject::tr("Centrer sur Zone (?=%1, ?=%2, ?=%3)")
				.arg(_svar(_params.mid(1,2), B1(_params.at(0))))//??? (bank 1)
				.arg(_svar(_params.mid(3,2), B2(_params.at(0))))//??? (bank 2)
				.arg((quint8)_params.at(5));//???
		
		case 0x6B:
		return QObject::tr("Voiler l'écran avec la couleur RVB(%1, %2, %3) (vitesse=%4, type=%5, adjust=%6)")
				.arg(_var(_params.mid(2,1), B1(_params.at(0))))//Rouge (bank 1 ???)
				.arg(_var(_params.mid(3,1), B2(_params.at(0))))//Vert (bank 2 ???)
				.arg(_var(_params.mid(4,1), B2(_params.at(1))))//Bleu (bank 4 ???)
				.arg((quint8)_params.at(5))//Vitesse
				.arg((quint8)_params.at(6))//Type
				.arg((quint8)_params.at(7));//Adjust
		
		case 0x6C:
		return QObject::tr("Attendre la fin du voilage de l'écran pour continuer");
		
		case 0x6D:
		return QObject::tr("%1 le polygone n°%2")
				.arg((quint8)_params.at(2) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"))//booléen
				.arg(_toInt(_params.mid(0,2)));//Polygone ID
		
		case 0x6E:
		return QObject::tr("Stocker l'id du décor précédent dans %1")
				.arg(_bank(_params.at(1), B2(_params.at(0))));//Décor ID (bank 2)
		
		case 0x6F:
		return QObject::tr("Centrer sur le personnage n°%2 de l'équipe actuelle (vitesse=%1 img, type=%3)")
				.arg(_var(_params.mid(1,2), B2(_params.at(0))))//Vitesse (bank 2) ???
				.arg((quint8)_params.at(3))//party ID
				.arg((quint8)_params.at(4));//Type de Scroll
		
		case 0x70:
		return QObject::tr("Commencer le combat n°%1")
				.arg(_var(_params.mid(1,2), B2(_params.at(0))));//battle ID (bank 2) ???
		
		case 0x71:
		return QObject::tr("%1 les combats aléatoires")
				.arg((quint8)_params.at(0) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//booléen
		
		case 0x72:
		return QObject::tr("BTLMD");
		
		case 0x73:
		return QObject::tr("Obtenir la direction du personnage n°%1 de l'équipe actuelle et la stocker dans %2")
				.arg((quint8)_params.at(1))//Party ID
				.arg(_bank(_params.at(2), B2(_params.at(0))));//(bank 2)
		
		case 0x74:
		return QObject::tr("Obtenir l'id du personnage n°%1 de l'équipe actuelle et le stocker dans %2")
				.arg((quint8)_params.at(1))//Party ID
				.arg(_bank(_params.at(2), B2(_params.at(0))));//(bank 2)
		
		case 0x75:
		return QObject::tr("Obtenir les coordonnées du personnage n°%1 de l'équipe actuelle (stocker : X dans %2, Y dans %3, Z dans %4 et l'id dans %5)")
				.arg((quint8)_params.at(2))//Party ID
				.arg(_bank(_params.at(3), B1(_params.at(0))))//(bank 1)
				.arg(_bank(_params.at(4), B2(_params.at(0))))//(bank 2)
				.arg(_bank(_params.at(5), B1(_params.at(1))))//(bank 3)
				.arg(_bank(_params.at(6), B2(_params.at(1))));//(bank 4)
		
		case 0x76:
		return QObject::tr("%1 = %1 + %2 (8 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x77:
		return QObject::tr("%1 = %1 + %2 (16 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x78:
		return QObject::tr("%1 = %1 - %2 (8 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x79:
		return QObject::tr("%1 = %1 - %2 (16 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x7A:
		return QObject::tr("%1 = %1 + 1 (8 bits)")
				.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)
		
		case 0x7B:
		return QObject::tr("%1 = %1 + 1 (16 bits)")
				.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)
		
		case 0x7C:
		return QObject::tr("%1 = %1 - 1 (8 bits)")
				.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)
		
		case 0x7D:
		return QObject::tr("%1 = %1 - 1 (16 bits)")
				.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)
		
		case 0x7E:
		return QObject::tr("%1 la possibilité de parler à l'objet 3D")
				.arg((quint8)_params.at(0) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booléen
		
		case 0x7F:
		return QObject::tr("Seed Random Generator : %1")
				.arg(_var(_params.mid(1,1), B2(_params.at(0))));//Valeur (bank 2)

		case 0x80:
		return QObject::tr("%1 = %2 (8 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x81:
		return QObject::tr("%1 = %2 (16 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x82:
		return QObject::tr("Mettre le bit %2 à 1 dans %1")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Position (bank 2)
		
		case 0x83:
		return QObject::tr("Mettre le bit %2 à 0 dans %1")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Position (bank 2)
		
		case 0x84:
		return QObject::tr("Inverser la valeur du bit %2 dans %1")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Position (bank 2)
		
		case 0x85:
		return QObject::tr("%1 = %1 + %2 (8 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x86:
		return QObject::tr("%1 = %1 + %2 (16 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x87:
		return QObject::tr("%1 = %1 - %2 (8 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x88:
		return QObject::tr("%1 = %1 - %2 (16 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x89:
		return QObject::tr("%1 = %1 * %2 (8 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x8A:
		return QObject::tr("%1 = %1 * %2 (16 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x8B:
		return QObject::tr("%1 = %1 / %2 (8 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x8C:
		return QObject::tr("%1 = %1 / %2 (16 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x8D:
		return QObject::tr("%1 = %1 mod %2 (8 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x8E:
		return QObject::tr("%1 = %1 mod %2 (16 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x8F:
		return QObject::tr("%1 = %1 & %2 (8 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x90:
		return QObject::tr("%1 = %1 & %2 (16 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x91:
		return QObject::tr("%1 = %1 | %2 (8 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x92:
		return QObject::tr("%1 = %1 | %2 (16 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x93:
		return QObject::tr("%1 = %1 ^ %2 (8 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x94:
		return QObject::tr("%1 = %1 ^ %2 (16 bits)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x95:
		return QObject::tr("%1 = %1 + 1 (8 bits)")
				.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)
		
		case 0x96:
		return QObject::tr("%1 = %1 + 1 (16 bits)")
				.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)
		
		case 0x97:
		return QObject::tr("%1 = %1 - 1 (8 bits)")
				.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)
		
		case 0x98:
		return QObject::tr("%1 = %1 - 1 (16 bits)")
				.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)
		
		case 0x99:
		return QObject::tr("Affecter une valeur aléatoire à %1 (8 bits)")
				.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)
		
		case 0x9A:
		return QObject::tr("%1 = %2 & 0xFF (low byte)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,1), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x9B:
		return QObject::tr("%1 = (%2 >> 8) & 0xFF (high byte)")
				.arg(_bank(_params.at(1), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(2,2), B2(_params.at(0))));//Valeur (bank 2)
		
		case 0x9C:
		return QObject::tr("%1 = (%2 & 0xFF) | ((%3 & 0xFF) << 8)")
				.arg(_bank(_params.at(2), B1(_params.at(0))))//(bank 1)
				.arg(_var(_params.mid(3,1), B2(_params.at(0))))//Valeur (bank 2)
				.arg(_var(_params.mid(4,1), B2(_params.at(1))));//Valeur (bank 4 ???)
		
		case 0x9D:
		return QObject::tr("SETX");
		
		case 0x9E:
		return QObject::tr("GETX");
		
		case 0x9F:
		return QObject::tr("SEARCHX");
		
		case 0xA0:
		return QObject::tr("L'objet 3D est jouable et c'est %1")
				.arg(_personnage(_params.at(0)));//Personnage ID
		
		case 0xA1:
		return QObject::tr("Ce groupe est un objet 3D (id=%1)")
				.arg((quint8)_params.at(0));//objet 3D ID
		
		case 0xA2:
		return QObject::tr("Joue l'animation %1 de l'objet 3D (vitesse=%2)")
				.arg((quint8)_params.at(0))//animation ID
				.arg((quint8)_params.at(1));//vitesse
		
		case 0xA3:
		return QObject::tr("Joue l'animation %1 de l'objet 3D et retourne à l'état précédent (vitesse=%2)")
				.arg((quint8)_params.at(0))//animation ID
				.arg((quint8)_params.at(1));//vitesse
		
		case 0xA4:
		return QObject::tr("%1 l'objet 3D")
				.arg((quint8)_params.at(0) == 0 ? QObject::tr("Cacher") : QObject::tr("Afficher"));//animation ID
		
		case 0xA5:
		return QObject::tr("Place l'objet 3D (X=%1, Y=%2, Z=%3, polygone id=%4)")
				.arg(_svar(_params.mid(2,2), B1(_params.at(0))))//Cible X (bank 1)
				.arg(_svar(_params.mid(4,2), B2(_params.at(0))))//Cible Y (bank 2)
				.arg(_svar(_params.mid(6,2), B1(_params.at(1))))//Cible Z (bank 3)
				.arg(_var(_params.mid(8,2), B2(_params.at(1))));//Polygone ID (bank 4)
		
		case 0xA6:
		return QObject::tr("Place l'objet 3D (X=%1, Y=%2, polygone id=%4)")
				.arg(_svar(_params.mid(2,2), B1(_params.at(0))))//Cible X (bank 1)
				.arg(_svar(_params.mid(4,2), B2(_params.at(0))))//Cible Y (bank 2)
				.arg(_var(_params.mid(6,2), B1(_params.at(1))));//Polygone ID (bank 3)
		
		case 0xA7:
		return QObject::tr("Place l'objet 3D (X=%1, Y=%2, Z=%3)")
				.arg(_svar(_params.mid(2,2), B1(_params.at(0))))//Cible X (bank 1)
				.arg(_svar(_params.mid(4,2), B2(_params.at(0))))//Cible Y (bank 2)
				.arg(_svar(_params.mid(6,2), B1(_params.at(1))));//Cible Z (bank 3)
		
		case 0xA8:
		return QObject::tr("Déplace l'objet 3D (X=%1, Y=%2)")
				.arg(_svar(_params.mid(1,2), B1(_params.at(0))))//Cible X (bank 1)
				.arg(_svar(_params.mid(3,2), B2(_params.at(0))));//Cible Y (bank 2)
		
		case 0xA9:
		return QObject::tr("Déplace l'objet 3D sans animation (X=%1, Y=%2)")
				.arg(_svar(_params.mid(1,2), B1(_params.at(0))))//Cible X (bank 1)
				.arg(_svar(_params.mid(3,2), B2(_params.at(0))));//Cible Y (bank 2)
		
		case 0xAA:
		return QObject::tr("Déplace l'objet 3D vers le groupe %1")
				.arg(_script((quint8)_params.at(0)));//GroupID
		
		case 0xAB:
		return QObject::tr("Rotation de l'objet 3D vers le groupe %1 (vitesse=%2, SensRotation=%3)")
				.arg(_script((quint8)_params.at(0)))//GroupID
				.arg(_sensRotation((quint8)_params.at(1)))//Sens de rotation
				.arg((quint8)_params.at(2));//Vitesse
		
		case 0xAC:
		return QObject::tr("Attendre que l'animation soit terminée pour continuer");
		
		case 0xAD:
		return QObject::tr("Déplace l'objet 3D sans animation (X=%1, Y=%2)")
				.arg(_svar(_params.mid(1,2), B1(_params.at(0))))//Cible X (bank 1)
				.arg(_svar(_params.mid(3,2), B2(_params.at(0))));//Cible Y (bank 2)
		
		case 0xAE:
		return QObject::tr("Joue l'animation %1 de l'objet 3D et retourne à l'état précédent (vitesse=%2)")
				.arg((quint8)_params.at(0))//animation ID
				.arg((quint8)_params.at(1));//vitesse
		
		case 0xAF:
		return QObject::tr("Joue l'animation %1 de l'objet 3D (vitesse=%2)")
				.arg((quint8)_params.at(0))//animation ID
				.arg((quint8)_params.at(1));//vitesse
		
		case 0xB0:
		return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D et retourne à l'état précédent (première img=%2, dernière img=%3, vitesse=%4)")
				.arg((quint8)_params.at(0))//animation ID
				.arg((quint8)_params.at(1))//Première img
				.arg((quint8)_params.at(2))//Dernière img
				.arg((quint8)_params.at(3));//vitesse
		
		case 0xB1:
		return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D (première img=%2, dernière img=%3, vitesse=%4)")
				.arg((quint8)_params.at(0))//animation ID
				.arg((quint8)_params.at(1))//Première img
				.arg((quint8)_params.at(2))//Dernière img
				.arg((quint8)_params.at(3));//vitesse
		
		case 0xB2:
		return QObject::tr("Configurer la vitesse des déplacements de l'objet 3D : %1")
				.arg(_var(_params.mid(1,2), B2(_params.at(0))));//Vitesse (bank 2)
		
		case 0xB3:
		return QObject::tr("Mettre l'objet 3D dans la direction : %1")
				.arg(_var(_params.mid(1,1), B2(_params.at(0))));//Direction (bank 2)
		
		case 0xB4:
		return QObject::tr("Rotation (direction=%1, nbTours=%2, vitesse=%3, ?=%4)")
				.arg(_var(_params.mid(1,1), B2(_params.at(0))))//Final Rotation (bank 2)
				.arg((quint8)_params.at(2))//nbTours
				.arg((quint8)_params.at(3))//Vitesse
				.arg((quint8)_params.at(4));//?
		
		case 0xB5:
		return QObject::tr("Rotation inversée (direction=%1, nbTours=%2, vitesse=%3, ?=%4)")
				.arg(_var(_params.mid(1,1), B2(_params.at(0))))//Final Rotation (bank 2)
				.arg((quint8)_params.at(2))//nbTours
				.arg((quint8)_params.at(3))//Vitesse
				.arg((quint8)_params.at(4));//?
		
		case 0xB6:
		return QObject::tr("Mettre l'objet 3D en direction du groupe %1")
				.arg(_script((quint8)_params.at(0)));//GroupID
		
		case 0xB7:
		return QObject::tr("Stocker dans %2 la direction du groupe %1")
				.arg(_script((quint8)_params.at(1)))//GroupID
				.arg(_bank(_params.at(2), B2(_params.at(0))));//(Bank 2)
		
		case 0xB8:
		return QObject::tr("Stocker dans %2 et %3 la position X et Y du groupe %1")
				.arg(_script((quint8)_params.at(1)))//GroupID
				.arg(_bank(_params.at(2), B1(_params.at(0))))//(Bank 1)
				.arg(_bank(_params.at(3), B2(_params.at(0))));//(Bank 2)
		
		case 0xB9:
		return QObject::tr("Stocker dans %2 le polygone id du groupe %1")
				.arg(_script((quint8)_params.at(1)))//GroupID
				.arg(_bank(_params.at(2), B2(_params.at(0))));//(Bank 2)
		
		case 0xBA:
		return QObject::tr("Joue l'animation %1 de l'objet 3D (vitesse=%2)")
				.arg((quint8)_params.at(0))//animation ID
				.arg((quint8)_params.at(1));//vitesse
		
		case 0xBB:
		return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D et retourne à l'état précédent (première img=%2, dernière img=%3, vitesse=%4)")
				.arg((quint8)_params.at(0))//animation ID
				.arg((quint8)_params.at(1))//Première img
				.arg((quint8)_params.at(2))//Dernière img
				.arg((quint8)_params.at(3));//vitesse
		
		case 0xBC:
		return QObject::tr("Joue partiellement l'animation %1 de l'objet 3D (première img=%2, dernière img=%3, vitesse=%4)")
				.arg((quint8)_params.at(0))//animation ID
				.arg((quint8)_params.at(1))//Première img
				.arg((quint8)_params.at(2))//Dernière img
				.arg((quint8)_params.at(3));//vitesse
		
		case 0xBD:
		return QObject::tr("Configurer la vitesse des animations de l'objet 3D : %1")
				.arg(_var(_params.mid(1,2), B2(_params.at(0))));//Vitesse (bank 2)
		
		case 0xBF:
		return QObject::tr("Prendre le contrôle du groupe %1")
				.arg(_script((quint8)_params.at(0)));//GroupID
		
		case 0xC0:
		return QObject::tr("Faire sauter un personnage (X=%1, Y=%2, polygone id=%3, hauteur=%4)")
				.arg(_svar(_params.mid(2,2), B1(_params.at(0))))//Cible X (bank 1)
				.arg(_svar(_params.mid(4,2), B2(_params.at(0))))//Cible Y (bank 2)
				.arg(_var(_params.mid(6,2), B1(_params.at(1))))//Polygone id (bank 3)
				.arg(_svar(_params.mid(8,2), B2(_params.at(1))));//Height (bank 4)
		
		case 0xC1:
		return QObject::tr("Stocker la position du groupe %1 dans des variables (%2=X, %3=Y, %4=Z, %5=polygone id)")
				.arg(_script((quint8)_params.at(2)))//GroupID
				.arg(_bank(_params.at(3), B1(_params.at(0))))//(bank 1)
				.arg(_bank(_params.at(4), B2(_params.at(0))))//(bank 2)
				.arg(_bank(_params.at(5), B1(_params.at(1))))//(bank 3)
				.arg(_bank(_params.at(6), B2(_params.at(1))));//(bank 4)
		
		case 0xC2:
		return QObject::tr("Monter une échelle avec l'animation %6 (X=%1, Y=%2, Z=%3, polygone id=%4, sens=%5, direction=%7, vitesse=%8)")
				.arg(_svar(_params.mid(2,2), B1(_params.at(0))))//Cible X (bank 1)
				.arg(_svar(_params.mid(4,2), B2(_params.at(0))))//Cible Y (bank 2)
				.arg(_svar(_params.mid(6,2), B1(_params.at(1))))//Cible Z (bank 3)
				.arg(_var(_params.mid(8,2), B2(_params.at(1))))//Polygone id (bank 4)
				.arg((quint8)_params.at(10))//Sens montee/descente
				.arg((quint8)_params.at(11))//Animation id
				.arg((quint8)_params.at(12))//Direction
				.arg((quint8)_params.at(13));//Vitesse
		
		case 0xC3:
		return QObject::tr("Offset Object (mouvement=%1, X=%2, Y=%3, Z=%4, vitesse=%5)")
				.arg((quint8)_params.at(2))//Mouvement type
				.arg(_svar(_params.mid(3,2), B1(_params.at(0))))//Cible X (bank 1)
				.arg(_svar(_params.mid(5,2), B2(_params.at(0))))//Cible Y (bank 2)
				.arg(_svar(_params.mid(7,2), B1(_params.at(1))))//Cible Z (bank 3)
				.arg(_var(_params.mid(9,2), B2(_params.at(1))));//Vitesse (bank 4)
		
		case 0xC4:
		return QObject::tr("Attendre la fin de l'exécution de l'Offset Object pour continuer");
		
		case 0xC5:
		return QObject::tr("Modifier la distance nécessaire pour parler avec l'objet 3D : %1")
				.arg(_var(_params.mid(1,1), B2(_params.at(0))));//Distance (bank 2)
		
		case 0xC6:
		return QObject::tr("Modifier la distance nécessaire pour toucher l'objet 3D : %1")
				.arg(_var(_params.mid(1,1), B2(_params.at(0))));//Distance (bank 2)
		
		case 0xC7:
		return QObject::tr("%1 la possibilité de toucher l'objet 3D")
				.arg((quint8)_params.at(0) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booléen
		
		case 0xC8:
		return QObject::tr("Ajouter %1 à l'équipe actuelle")
				.arg(_personnage(_params.at(0)));//Personnage ID
		
		case 0xC9:
		return QObject::tr("Retirer %1 de l'équipe actuelle")
				.arg(_personnage(_params.at(0)));//Personnage ID
		
		case 0xCA:
		return QObject::tr("Nouvelle équipe : %1 | %2 | %3")
				.arg(_personnage(_params.at(0)))//Personnage ID
				.arg(_personnage(_params.at(1)))//Personnage ID
				.arg(_personnage(_params.at(2)));//Personnage ID
		
		case 0xCB://Indenter
			return QObject::tr("Si %1 est dans l'équipe actuelle (aller à l'octet %2 sinon)")
					.arg(_personnage(_params.at(0)))//Personnage ID
					.arg(pos+2+(quint8)_params.at(1));//Saut court

		case 0xCC://Indenter
			return QObject::tr("Si %1 existe (aller à l'octet %2 sinon)")
					.arg(_personnage(_params.at(0)))//Personnage ID
					.arg(pos+2+(quint8)_params.at(1));//Saut court

		case 0xCD:
			return QObject::tr("%2 %1")
					.arg((quint8)_params.at(0) == 0 ? QObject::tr("n'existe plus") : QObject::tr("existe"))//Booléen
					.arg(_personnage(_params.at(1)));//Personnage ID

		case 0xCE:
			return QObject::tr("Bloque %1 dans le menu PHS")
					.arg(_personnage(_params.at(0)));//Personnage ID

		case 0xCF:
			return QObject::tr("Débloque %1 dans le menu PHS")
					.arg(_personnage(_params.at(0)));//Personnage ID

		case 0xD0:
			return QObject::tr("Définit la zone (X1=%1, Y1=%2, Z1=%3, X2=%4, Y2=%5, Z2=%6)")
					.arg(_toSInt(_params.mid(0,2)))//Cible X1
					.arg(_toSInt(_params.mid(2,2)))//Cible Y1
					.arg(_toSInt(_params.mid(4,2)))//Cible Z1
					.arg(_toSInt(_params.mid(6,2)))//Cible X2
					.arg(_toSInt(_params.mid(8,2)))//Cible Y2
					.arg(_toSInt(_params.mid(10,2)));//Cible Z2

		case 0xD1:
			return QObject::tr("%1 la zone")
					.arg((quint8)_params.at(0) == 0 ? QObject::tr("Effacer") : QObject::tr("Tracer"));//Booléen

		case 0xD2:
			return QObject::tr("%1 les changements de décor par le joueur")
					.arg((quint8)_params.at(0) == 0 ? QObject::tr("Autoriser") : QObject::tr("Empêcher"));//Booléen

		case 0xD3:
			return QObject::tr("Redimensionner la zone (X1=%1, Y1=%2, Z1=%3, X2=%4, Y2=%5, Z2=%6)")
					.arg(_svar(_params.mid(3,2), B1(_params.at(0))))//Cible X1 (Bank 1)
					.arg(_svar(_params.mid(5,2), B2(_params.at(0))))//Cible Y1 (Bank 2)
					.arg(_svar(_params.mid(7,2), B1(_params.at(1))))//Cible Z1 (Bank 3)
					.arg(_svar(_params.mid(9,2), B2(_params.at(1))))//Cible X2 (Bank 4)
					.arg(_svar(_params.mid(11,2), B1(_params.at(2))))//Cible Y2 (Bank 5)
					.arg(_svar(_params.mid(13,2), B2(_params.at(2))));//Cible Z2 (Bank 6)

		case 0xD4:
			return QObject::tr("%4 = Sinus(%1) %2 %3")
					.arg(_var(_params.mid(2,2), B1(_params.at(0))))//Variable (bank 1)
					.arg(_var(_params.mid(4,2), B2(_params.at(0))))//Variable (bank 2)
					.arg(_var(_params.mid(6,2), B1(_params.at(1))))//Variable (bank 3)
					.arg(_bank(_params.at(8), B2(_params.at(1))));//Variable (bank 4)

		case 0xD5:
			return QObject::tr("%4 = Cosinus(%1) %2 %3")
					.arg(_var(_params.mid(2,2), B1(_params.at(0))))//Variable (bank 1)
					.arg(_var(_params.mid(4,2), B2(_params.at(0))))//Variable (bank 2)
					.arg(_var(_params.mid(6,2), B1(_params.at(1))))//Variable (bank 3)
					.arg(_bank(_params.at(8), B2(_params.at(1))));//Variable (bank 4)

		case 0xD6:
			return QObject::tr("Modifier la distance nécessaire pour parler avec l'objet 3D : %1")
					.arg(_var(_params.mid(1,2), B2(_params.at(0))));//Distance (bank 2)

		case 0xD7:
			return QObject::tr("Modifier la distance nécessaire pour toucher l'objet 3D : %1")
					.arg(_var(_params.mid(1,2), B2(_params.at(0))));//Distance (bank 2)

		case 0xD8:
			return QObject::tr("Commencer à charger l'écran %1")
					.arg(_field(_params.mid(0,2)));//id décor

		case 0xD9:
			return QObject::tr("PMJMP2");//TODO renommer

		case 0xDA:
			return QObject::tr("AKAO2");
			//TODO parametres

		case 0xDB:
			return QObject::tr("%1 rotation")
					.arg((quint8)_params.at(0) == 0 ? QObject::tr("Activer") : QObject::tr("Désactiver"));//Booléen

		case 0xDC:
			return QObject::tr("Jouer animation n°%1 pour '%3' (vitesse=%2)")
					.arg((quint8)_params.at(0))//animation ID
					.arg((quint8)_params.at(1))//vitesse
					.arg((quint8)_params.at(2)==0 ? QObject::tr("rester immobile") : ((quint8)_params.at(2)==1 ? QObject::tr("marcher") : QObject::tr("courir")));//Stand Walk Run

		case 0xDD:
			return QObject::tr("Stoppe l'animation de l'objet 3D");

		case 0xDE:
			return QObject::tr("Attendre que la rotation soit terminée pour continuer");

		case 0xDF:
			return QObject::tr("MPPAL");
			//TODO parametres

		case 0xE0:
			return QObject::tr("Afficher l'état n°%2 du paramètre n°%1")
					.arg(_var(_params.mid(1,1), B1(_params.at(0))))//Paramètre (bank 1)
					.arg(_var(_params.mid(2,1), B2(_params.at(0))));//État (bank 2)

		case 0xE1:
			return QObject::tr("Cacher l'état n°%2 du paramètre n°%1")
					.arg(_var(_params.mid(1,1), B1(_params.at(0))))//Paramètre (bank 1)
					.arg(_var(_params.mid(2,1), B2(_params.at(0))));//État (bank 2)

		case 0xE2:
			return QObject::tr("Afficher l'état suivant du paramètre n°%1")
					.arg(_var(_params.mid(1,1), B2(_params.at(0))));//Paramètre (bank 2)

		case 0xE3:
			return QObject::tr("Afficher l'état précédent du paramètre n°%1")
					.arg(_var(_params.mid(1,1), B2(_params.at(0))));//Paramètre (bank 2)

		case 0xE4:
			return QObject::tr("Cacher paramètre n°%1")
					.arg(_var(_params.mid(1,1), B2(_params.at(0))));//Paramètre (bank 2)

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
					.arg(_var(_params.mid(5,1), B1(_params.at(1))))
					.arg(_var(_params.mid(6,1), B2(_params.at(1))))
					.arg(_var(_params.mid(7,1), B1(_params.at(2))));
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
					.arg((quint8)_params.at(0));//Musique ID

		case 0xF1:
			return QObject::tr("Jouer son n°%1 (position=%2/127)")
					.arg(_var(_params.mid(1,2), B1(_params.at(0))))//Sound ID (bank 1)
					.arg(_var(_params.mid(3,1), B2(_params.at(0))));//Position (bank 2)

		case 0xF2:
			return QObject::tr("AKAO");
			//TODO parametres

		case 0xF3:
			return QObject::tr("MUSVT (musique n°%1)")
					.arg((quint8)_params.at(0));//Musique ID

		case 0xF4:
			return QObject::tr("MUSVM (musique n°%1)")
					.arg((quint8)_params.at(0));//Musique ID

		case 0xF5:
			return QObject::tr("%1 musique")
					.arg((quint8)_params.at(0) == 0 ? QObject::tr("Déverrouiller") : QObject::tr("Verrouiller", "test"));

		case 0xF6:
			return QObject::tr("Choisir musique n°%1 comme musique de combat")
					.arg((quint8)_params.at(0));//Musique ID

		case 0xF7:
			return QObject::tr("CHMPH");
			//TODO parametres

		case 0xF8:
			return QObject::tr("Choisir prochaine cinématique : %1")
					.arg(_movie(_params.at(0)));

		case 0xF9:
			return QObject::tr("Jouer la cinématique choisie");

		case 0xFA:
			return QObject::tr("Stocker Movie frame dans %1")
					.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)

		case 0xFB:
			return QObject::tr("Camera Movie : %1")
					.arg((quint8)_params.at(0) == 0 ? QObject::tr("ON") : QObject::tr("OFF"));//booléen

		case 0xFC:
			return QObject::tr("FMUSC");
			//TODO parametres

		case 0xFD:
			return QObject::tr("CMUSC");
			//TODO parametres

		case 0xFE:
			return QObject::tr("Si la musique est jouée mettre %1 à 1")
					.arg(_bank(_params.at(1), B2(_params.at(0))));//(bank 2)

		case 0xFF:
			return QObject::tr("Game Over");

		default:
			return "?";
		}
}

OpcodeEmpty::OpcodeEmpty(quint8 id) :
	Commande()
{
	_id = id;
}

quint8 OpcodeEmpty::id() const
{
	return _id;
}

QString OpcodeEmpty::toString() const
{
	return QObject::tr("? (id=%1)")
			.arg(_id);
}

OpcodeRET::OpcodeRET() :
	Commande()
{
}

QString OpcodeRET::toString() const
{
	return QObject::tr("Retourner");
}

OpcodeExec::OpcodeExec(const QByteArray &params)
{
	groupID = params.at(0);//GroupID sur 1 octet
	scriptID = params.at(1) & 0x1F;//ScriptID sur 5 bits
	priority = (params.at(1) >> 5) & 7;//Priorité sur 3 bits
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
	return QObject::tr("Exécuter le script n°%3 du groupe %1 (priorité %2/6) - Seulement si le script n'est pas déjà en cours d'exécution")
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
	return QObject::tr("Exécuter le script n°%3 du groupe %1 (priorité %2/6)")
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
	return QObject::tr("Exécuter le script n°%3 du groupe externe %1 (priorité %2/6) - Attend la fin de l'exécution pour continuer")
			.arg(_script(groupID))
			.arg(priority)
			.arg(scriptID);
}

OpcodeExecChar::OpcodeExecChar(const QByteArray &params) :
	Commande()
{
	charID = params.at(0);//charID sur 1 octet
	scriptID = params.at(1) & 0x1F;//ScriptID sur 5 bits
	priority = (params.at(1) >> 5) & 7;//Priorité sur 3 bits
}

QByteArray OpcodeExecChar::params() const
{
	return QByteArray()
			.append((char)charID)
			.append(char((scriptID & 0x1F) | ((priority & 7) << 5)));
}

OpcodePREQ::OpcodePREQ(const QByteArray &params) :
	OpcodeExecChar(params)
{
}

QString OpcodePREQ::toString() const
{
	return QObject::tr("Exécuter le script n°%3 du groupe lié au personnage n°%1 de l'équipe (priorité %2/6) - Seulement si le script n'est pas déjà en cours d'exécution")
			.arg(charID)
			.arg(priority)
			.arg(scriptID);
}

OpcodePRQSW::OpcodePRQSW(const QByteArray &params) :
	OpcodeExecChar(params)
{
}

QString OpcodePRQSW::toString() const
{
	return QObject::tr("Exécuter le script n°%3 du groupe lié au personnage n°%1 de l'équipe (priorité %2/6)")
			.arg(charID)
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
			.arg(charID)
			.arg(priority)
			.arg(scriptID);
}

OpcodeRETTO::OpcodeRETTO(const QByteArray &params) :
	Commande()
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

OpcodeJOIN::OpcodeJOIN(const QByteArray &params) :
	Commande()
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

OpcodeSPLIT::OpcodeSPLIT(const QByteArray &params) :
	Commande()
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

OpcodePartyE::OpcodePartyE(const QByteArray &params) :
	Commande()
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

OpcodeDSKCG::OpcodeDSKCG(const QByteArray &params) :
	Commande()
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

OpcodeSPECIALARROW::OpcodeSPECIALARROW(const QByteArray &params) :
	Commande()
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

OpcodeSPECIALPNAME::OpcodeSPECIALPNAME(const QByteArray &params) :
	Commande()
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

OpcodeSPECIALGMSPD::OpcodeSPECIALGMSPD(const QByteArray &params) :
	Commande()
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

OpcodeSPECIALSMSPD::OpcodeSPECIALSMSPD(const QByteArray &params) :
	Commande()
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

OpcodeSPECIALFLMAT::OpcodeSPECIALFLMAT() :
	Commande()
{
}

QString OpcodeSPECIALFLMAT::toString() const
{
	return QObject::tr("Remplir le menu matéria de toutes les matérias en quantité maximum");
}

OpcodeSPECIALFLITM::OpcodeSPECIALFLITM() :
	Commande()
{
}

QString OpcodeSPECIALFLITM::toString() const
{
	return QObject::tr("Remplir l'inventaire par tous les objets en quantité maximum");
}


OpcodeSPECIALBTLCK::OpcodeSPECIALBTLCK(const QByteArray &params) :
	Commande()
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

OpcodeSPECIALMVLCK::OpcodeSPECIALMVLCK(const QByteArray &params) :
	Commande()
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

OpcodeSPECIALSPCNM::OpcodeSPECIALSPCNM(const QByteArray &params) :
	Commande()
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

OpcodeSPECIALRSGLB::OpcodeSPECIALRSGLB() :
	Commande()
{
}

QString OpcodeSPECIALRSGLB::toString() const
{
	return QObject::tr("Met le temps à 0, débloque le menu \"PHS\" et \"Sauvegarder\". Nouvelle équipe : Clad | (Vide) | (Vide)");
}

OpcodeSPECIALCLITM::OpcodeSPECIALCLITM() :
	Commande()
{
}

QString OpcodeSPECIALCLITM::toString() const
{
	return QObject::tr("Supprimer tous les objets de l'inventaire");
}

OpcodeSPECIAL::OpcodeSPECIAL(const QByteArray &params) :
	Commande()
{
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
	default:	opcode = new OpcodeEmpty(params.at(0));
		break;
	}
}

OpcodeSPECIAL::~OpcodeSPECIAL()
{
	delete opcode;
}

QString OpcodeSPECIAL::toString() const
{
	return QObject::tr("SPECIAL - ") + opcode->toString();
}

QByteArray OpcodeSPECIAL::params() const
{
	return opcode->toByteArray();
}

OpcodeJump::OpcodeJump() :
	Commande(), _label(0), _jump(0)
{
}

void OpcodeJump::setLabel(quint32 label)
{
	_label = label;
}

quint16 OpcodeJump::jump() const
{
	return _jump;
}

void OpcodeJump::setJump(quint16 jump)
{
	_jump = jump;
}

OpcodeJMPF::OpcodeJMPF(const QByteArray &params) :
	OpcodeJump()
{
	_jump = (quint8)params.at(0) + 1;
}

QString OpcodeJMPF::toString() const
{
	return QObject::tr("Aller à l'octet %1 du script")
			.arg(_label+_jump);
}

QByteArray OpcodeJMPF::params() const
{
	return QByteArray().append(char(_jump - 1));
}

OpcodeJMPFL::OpcodeJMPFL(const QByteArray &params) :
	OpcodeJump()
{
	quint16 jump;
	memcpy(&jump, params.constData(), 2);

	_jump = jump + 1;
}

QString OpcodeJMPFL::toString() const
{
	return QObject::tr("Aller à l'octet %1 du script")
			.arg(_label+_jump);
}

QByteArray OpcodeJMPFL::params() const
{
	quint16 jump = _jump - 1;
	return QByteArray().append((char *)&jump, 2);
}

OpcodeJMPB::OpcodeJMPB(const QByteArray &params) :
	OpcodeJump()
{
	_jump = -(quint8)params.at(0);
}

QString OpcodeJMPB::toString() const
{
	return QObject::tr("Aller à l'octet %1 du script")
			.arg(int(_label+_jump));
}

QByteArray OpcodeJMPB::params() const
{
	return QByteArray().append(char(-_jump));
}

OpcodeJMPBL::OpcodeJMPBL(const QByteArray &params) :
	OpcodeJump()
{
	quint16 jump;
	memcpy(&jump, params.constData(), 2);

	_jump = -jump;
}

QString OpcodeJMPBL::toString() const
{
	return QObject::tr("Aller à l'octet %1 du script")
			.arg(_label+_jump);
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

OpcodeIFUB::OpcodeIFUB(const QByteArray &params) :
	OpcodeIf()
{
	banks = params.at(0);
	value1 = params.at(1); // bank 1
	value2 = params.at(2); // bank 2
	oper = params.at(3);
	_jump = params.at(4) + 5;
}

QString OpcodeIFUB::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label + _jump)
			.arg(oper==9 || oper==10 ? ")" : "");//mini hack ")"
}

QByteArray OpcodeIFUB::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)value1)
			.append((char)value2)
			.append((char)oper)
			.append(char(_jump - 5));
}

OpcodeIFUBL::OpcodeIFUBL(const QByteArray &params)
{
	const char *constParams = params.constData();
	banks = params.at(0);
	value1 = params.at(1); // bank 1
	value2 = params.at(2); // bank 2
	oper = params.at(3);
	quint16 jump;
	memcpy(&jump, &constParams[4], 2);
	_jump = jump + 5;
}

QString OpcodeIFUBL::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label + _jump)
			.arg(oper==9 || oper==10 ? ")" : "");//mini hack ")"
}

QByteArray OpcodeIFUBL::params() const
{
	quint16 jump = _jump - 5;
	return QByteArray()
			.append((char)banks)
			.append((char)value1)
			.append((char)value2)
			.append((char)oper)
			.append((char *)&jump, 2);
}

OpcodeIFSW::OpcodeIFSW(const QByteArray &params)
{
	const char *constParams = params.constData();
	banks = params.at(0);
	qint16 v1, v2;
	memcpy(&v1, &constParams[1], 2);
	memcpy(&v2, &constParams[3], 2);
	value1 = v1;
	value2 = v2;
	oper = params.at(5);
	_jump = params.at(6) + 7;
}

QString OpcodeIFSW::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label + _jump)
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
			.append(char(_jump - 7));
}

OpcodeIFSWL::OpcodeIFSWL(const QByteArray &params)
{
	const char *constParams = params.constData();
	banks = params.at(0);
	qint16 v1, v2;
	memcpy(&v1, &constParams[1], 2);
	memcpy(&v2, &constParams[3], 2);
	value1 = v1;
	value2 = v2;
	oper = params.at(5);
	quint16 jump;
	memcpy(&jump, &constParams[6], 2);
	_jump = jump + 7;
}

QString OpcodeIFSWL::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label + _jump)
			.arg(oper==9 || oper==10 ? ")" : "");//mini hack ")"
}

QByteArray OpcodeIFSWL::params() const
{
	qint16 v1=value1, v2=value2;
	quint16 jump = _jump - 7;
	return QByteArray()
			.append((char)banks)
			.append((char *)&v1, 2)
			.append((char *)&v2, 2)
			.append((char)oper)
			.append((char *)&jump, 2);
}

OpcodeIFUW::OpcodeIFUW(const QByteArray &params)
{
	const char *constParams = params.constData();
	banks = params.at(0);
	quint16 v1, v2;
	memcpy(&v1, &constParams[1], 2);
	memcpy(&v2, &constParams[3], 2);
	value1 = v1;
	value2 = v2;
	oper = params.at(5);
	_jump = params.at(6) + 7;
}

QString OpcodeIFUW::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label + _jump)
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
			.append(char(_jump - 7));
}

OpcodeIFUWL::OpcodeIFUWL(const QByteArray &params)
{
	const char *constParams = params.constData();
	banks = params.at(0);
	quint16 v1, v2;
	memcpy(&v1, &constParams[1], 2);
	memcpy(&v2, &constParams[3], 2);
	value1 = v1;
	value2 = v2;
	oper = params.at(5);
	quint16 jump;
	memcpy(&jump, &constParams[6], 2);
	_jump = jump + 7;
}

QString OpcodeIFUWL::toString() const
{
	return QObject::tr("Si %1 %3 %2%5 (aller à l'octet %4 du script sinon)")
			.arg(_var(value1, B1(banks)))
			.arg(_var(value2, B2(banks)))
			.arg(_operateur(oper))
			.arg(_label + _jump)
			.arg(oper==9 || oper==10 ? ")" : "");//mini hack ")"
}

QByteArray OpcodeIFUWL::params() const
{
	quint16 v1=value1, v2=value2;
	quint16 jump = _jump - 7;
	return QByteArray()
			.append((char)banks)
			.append((char *)&v1, 2)
			.append((char *)&v2, 2)
			.append((char)oper)
			.append((char *)&jump, 2);
}

OpcodeMINIGAME::OpcodeMINIGAME(const QByteArray &params)
{
	const char *constParams = params.constData();
	memcpy(&fieldID, constParams, 2);
	memcpy(&targetX, &constParams[2], 2);
	memcpy(&targetY, &constParams[4], 2);
	memcpy(&targetI, &constParams[6], 2);
	minigameParam = params.at(8);
	minigameID = params.at(9);
}

QString OpcodeMINIGAME::toString() const
{
	return QObject::tr("Lancer un mini-jeu : %5 (Après le jeu aller à l'écran %1 (X=%2, Y=%3, polygone id=%4))")
			.arg(_field(fieldID))
			.arg(targetX)
			.arg(targetY)
			.arg(targetI)
			.arg(_miniGame(minigameID, minigameParam));
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
	tutoID = params.at(0);
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

OpcodeBTMD2::OpcodeBTMD2(const QByteArray &params)
{
	memcpy(&battleMode, params.constData(), 2);
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
	banks = params.at(0);
	var = params.at(1); // bank 2
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

OpcodeWAIT::OpcodeWAIT(const QByteArray &params)
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

OpcodeBLINK::OpcodeBLINK(const QByteArray &params)
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

OpcodeKAWAIUnknown::OpcodeKAWAIUnknown(const QByteArray &params) :
	data(params)
{
}

QString OpcodeKAWAIUnknown::toString() const
{
	return QObject::tr("Opération inconnue");
}

QByteArray OpcodeKAWAIUnknown::params() const
{
	return data;
}

OpcodeKAWAIEYETX::OpcodeKAWAIEYETX(const QByteArray &params) :
	OpcodeKAWAIUnknown(params)
{
}

QString OpcodeKAWAIEYETX::toString() const
{
	return QObject::tr("EYETX");
}

OpcodeKAWAITRNSP::OpcodeKAWAITRNSP(const QByteArray &params)
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
	OpcodeKAWAIUnknown(params)
{
}

QString OpcodeKAWAIAMBNT::toString() const
{
	return QObject::tr("AMBNT");
}

OpcodeKAWAILIGHT::OpcodeKAWAILIGHT(const QByteArray &params) :
	OpcodeKAWAIUnknown(params)
{
}

QString OpcodeKAWAILIGHT::toString() const
{
	return QObject::tr("LIGHT");
}

OpcodeKAWAISBOBJ::OpcodeKAWAISBOBJ(const QByteArray &params) :
	OpcodeKAWAIUnknown(params)
{
}

QString OpcodeKAWAISBOBJ::toString() const
{
	return QObject::tr("SBOBJ");
}

OpcodeKAWAISHINE::OpcodeKAWAISHINE(const QByteArray &params) :
	OpcodeKAWAIUnknown(params)
{
}

QString OpcodeKAWAISHINE::toString() const
{
	return QObject::tr("SHINE");
}

OpcodeKAWAIRESET::OpcodeKAWAIRESET(const QByteArray &params) :
	OpcodeKAWAIUnknown(params)
{
}

QString OpcodeKAWAIRESET::toString() const
{
	return QObject::tr("RESET");
}

OpcodeKAWAI::OpcodeKAWAI(const QByteArray &params)
{
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
	default:	opcode = new OpcodeKAWAIUnknown(params.mid(2));
		break;
	}
}

OpcodeKAWAI::~OpcodeKAWAI()
{
	delete opcode;
}

QString OpcodeKAWAI::toString() const
{
	return QObject::tr("Filtre graphique sur l'objet 3D - %1")
			.arg(opcode->toString());
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

OpcodeBGSCR::OpcodeBGSCR(const QByteArray &params)
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

OpcodeWCLS::OpcodeWCLS(const QByteArray &params)
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

OpcodeWSIZW::OpcodeWSIZW(const QByteArray &params)
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

OpcodeIfKey::OpcodeIfKey(const QByteArray &params)
{
	memcpy(&keys, params.constData(), 2);
	_jump = params.at(2) + 3;
}

QByteArray OpcodeIfKey::params() const
{
	return QByteArray()
			.append((char *)&keys, 2)
			.append(char(_jump - 3));
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
	return QObject::tr("Si appuie sur la touche %1 (aller à l'octet %2 du script sinon)")
			.arg(keyString())
			.arg(_label + _jump);
}

OpcodeIFKEYON::OpcodeIFKEYON(const QByteArray &params) :
	OpcodeIfKey(params)
{
}

QString OpcodeIFKEYON::toString() const
{
	return QObject::tr("Si appuie sur la touche %1 une fois (aller à l'octet %2 du script sinon)")
			.arg(keyString())
			.arg(_label + _jump);
}

OpcodeIFKEYOFF::OpcodeIFKEYOFF(const QByteArray &params) :
	OpcodeIfKey(params)
{
}

QString OpcodeIFKEYOFF::toString() const
{
	return QObject::tr("Si relache la touche %1 pour la première fois (aller à l'octet %2 du script sinon)")
			.arg(keyString())
			.arg(_label + _jump);
}

OpcodeUC::OpcodeUC(const QByteArray &params)
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
	windowID = params.at(0);
	displayType = params.at(1);
	marginLeft = params.at(2);
	marginTop = params.at(3);
}

QString OpcodeWSPCL::toString() const
{
	return QObject::tr("%2 dans fenêtre n°%1 (gauche=%3, haut=%4)")
			.arg(windowID)
			.arg(_windowNum(displayType))
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

OpcodeWNUMB::OpcodeWNUMB(const QByteArray &params)
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
			.append((char)windowID)
			.append((char)windowID)
			.append((char *)&value, 4)
			.append((char)digitCount);
}

OpcodeSTTIM::OpcodeSTTIM(const QByteArray &params)
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

OpcodeGOLD::OpcodeGOLD(const QByteArray &params)
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

OpcodeGOLDu::OpcodeGOLDu(const QByteArray &params) :
	OpcodeGOLD(params)
{
}

QString OpcodeGOLDu::toString() const
{
	return QObject::tr("Ajouter %1 gils à l'équipe")
			.arg(_lvar(value, B1(banks), B2(banks)));
}

OpcodeGOLDd::OpcodeGOLDd(const QByteArray &params) :
	OpcodeGOLD(params)
{
}

QString OpcodeGOLDd::toString() const
{
	return QObject::tr("Retirer %1 gils à l'équipe")
			.arg(_lvar(value, B1(banks), B2(banks)));
}

OpcodeCHGLD::OpcodeCHGLD(const QByteArray &params)
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

OpcodeMPARA::OpcodeMPARA(const QByteArray &params)
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

OpcodeMPRA2::OpcodeMPRA2(const QByteArray &params)
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

OpcodeMPNAM::OpcodeMPNAM(const QByteArray &params)
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

OpcodeMP::OpcodeMP(const QByteArray &params)
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

OpcodeMENU::OpcodeMENU(const QByteArray &params)
{
	banks = params.at(0);
	menuID = params.at(1);
	param = params.at(2); // bank 2
}

QString OpcodeMENU::menu(const QString &param)
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

OpcodeMENU2::OpcodeMENU2(const QByteArray &params)
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
	banks = params.at(0);
	partyID = params.at(1);
	memcpy(&value, &(params.constData()[2]), 2); // bank 2??
}

QByteArray OpcodeHP::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)partyID)
			.append((char *)&value);
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

OpcodeWMOVE::OpcodeWMOVE(const QByteArray &params)
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

OpcodeWMODE::OpcodeWMODE(const QByteArray &params)
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

	return QObject::tr("Déplacer la fenêtre n°%1 (X=%2, Y=%3)")
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

OpcodeWREST::OpcodeWREST(const QByteArray &params)
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

OpcodeWCLSE::OpcodeWCLSE(const QByteArray &params)
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

OpcodeWROW::OpcodeWROW(const QByteArray &params)
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

OpcodeGWCOL::OpcodeGWCOL(const QByteArray &params)
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
			.arg(_var(varR, B2(banks[0])))
			.arg(_var(varG, B1(banks[1])))
			.arg(_var(varB, B2(banks[1])));
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

OpcodeSWCOL::OpcodeSWCOL(const QByteArray &params)
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

OpcodeSTITM::OpcodeSTITM(const QByteArray &params)
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

OpcodeDLITM::OpcodeDLITM(const QByteArray &params)
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

OpcodeCKITM::OpcodeCKITM(const QByteArray &params)
{
	banks = params.at(0);
	memcpy(&itemID, &(params.constData()[1]), 2); // bank 1
	varQuantity = params.at(3); // bank 2
}

QString OpcodeCKITM::toString() const
{
	return QObject::tr("%2 = quantité d'objets %1 dans l'inventaire")
			.arg(_item(itemID, B1(banks)))
			.arg(_var(varQuantity, B2(banks)));
}

QByteArray OpcodeCKITM::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char *)&itemID, 2)
			.append((char)varQuantity);
}

OpcodeSMTRA::OpcodeSMTRA(const QByteArray &params)
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

OpcodeDMTRA::OpcodeDMTRA(const QByteArray &params)
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

OpcodeCMTRA::OpcodeCMTRA(const QByteArray &params)
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
	return QObject::tr("%4 = quantité de matéria %1 dans l'inventaire (AP=%2, ???=%3)")
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

OpcodeSHAKE::OpcodeSHAKE(const QByteArray &params)
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
	memcpy(&fieldID, params.constData(), 2);
	memcpy(&targetX, &(params.constData()[2]), 2);
	memcpy(&targetY, &(params.constData()[4]), 2);
	memcpy(&targetI, &(params.constData()[6]), 2);
	direction = params.at(8);
}

QString OpcodeMAPJUMP::toString() const
{
	return QObject::tr("Aller à l'écran %1 (X=%2, Y=%3, polygone id=%4, direction=%5)")
			.arg(fieldID)
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

OpcodeSCR2D::OpcodeSCR2D(const QByteArray &params)
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

OpcodeSCRCC::OpcodeSCRCC()
{
}

QString OpcodeSCRCC::toString() const
{
	return QObject::tr("Centrer sur le personnage jouable");
}

OpcodeSCR2DC::OpcodeSCR2DC(const QByteArray &params)
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

OpcodeSCRLW::OpcodeSCRLW()
{
}

QString OpcodeSCRLW::toString() const
{
	return QObject::tr("Attendre la fin du dernier centrage pour continuer");
}

OpcodeSCR2DL::OpcodeSCR2DL(const QByteArray &params)
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

OpcodeMPDSP::OpcodeMPDSP(const QByteArray &params)
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

OpcodeFADE::OpcodeFADE(const QByteArray &params)
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

OpcodeFADEW::OpcodeFADEW()
{
}

QString OpcodeFADEW::toString() const
{
	return QObject::tr("Attendre la fin du voilage de l'écran pour continuer");
}

OpcodeIDLCK::OpcodeIDLCK(const QByteArray &params)
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

OpcodeSCRLP::OpcodeSCRLP(const QByteArray &params)
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

OpcodeBATTLE::OpcodeBATTLE(const QByteArray &params)
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

OpcodeBTLON::OpcodeBTLON(const QByteArray &params)
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

OpcodeGETPC::OpcodeGETPC(const QByteArray &params)
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

OpcodePXYZI::OpcodePXYZI(const QByteArray &params)
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

OpcodeOperation::OpcodeOperation(const QByteArray &params)
{
	banks = params.at(0);
	var = params.at(1); // bank 1
	value = params.at(2); // bank 2
}

QByteArray OpcodeOperation::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)var)
			.append((char)value);
}

OpcodeOperation2::OpcodeOperation2(const QByteArray &params)
{
	banks = params.at(0);
	var = params.at(1);
	memcpy(&value, &(params.constData()[2]), 2);
}

QByteArray OpcodeOperation2::params() const
{
	return QByteArray()
			.append((char)banks)
			.append((char)var)
			.append((char *)&value, 2);
}

OpcodeUnaryOperation::OpcodeUnaryOperation(const QByteArray &params)
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
	OpcodeOperation(params)
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

OpcodeRDMSD::OpcodeRDMSD(const QByteArray &params) :
	OpcodeUnaryOperation(params)
{
}

QString OpcodeRDMSD::toString() const
{
	return QObject::tr("Seed Random Generator : %1")
			.arg(_var(var, B2(banks)));
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
	OpcodeOperation(params)
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
	OpcodeOperation(params)
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
	OpcodeOperation(params)
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
	OpcodeOperation(params)
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
	OpcodeOperation(params)
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
	OpcodeOperation(params)
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
	OpcodeOperation(params)
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

OpcodeSETX::OpcodeSETX(const QByteArray &params)
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
	charID = params.at(0);
}

QString OpcodePC::toString() const
{
	return QObject::tr("L'objet 3D est jouable et c'est %1")
			.arg(_personnage(charID));
}

QByteArray OpcodePC::params() const
{
	return QByteArray()
			.append((char)charID);
}

OpcodeCHAR::OpcodeCHAR(const QByteArray &params)
{
	objectID = params.at(0);
}

QString OpcodeCHAR::toString() const
{
	return QObject::tr("Ce groupe est un objet 3D (id=%1)")
			.arg(_personnage(objectID));
}

QByteArray OpcodeCHAR::params() const
{
	return QByteArray()
			.append((char)objectID);
}

OpcodeDFANM::OpcodeDFANM(const QByteArray &params)
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

OpcodeXYI::OpcodeXYI(const QByteArray &params)
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

OpcodeXYZ::OpcodeXYZ(const QByteArray &params)
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

OpcodeMOVE::OpcodeMOVE(const QByteArray &params)
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

OpcodeCMOVE::OpcodeCMOVE(const QByteArray &params)
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

OpcodeMOVA::OpcodeMOVA(const QByteArray &params)
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

OpcodeANIME2::OpcodeANIME2(const QByteArray &params)
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

OpcodeDIR::OpcodeDIR(const QByteArray &params)
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

OpcodeTURNGEN::OpcodeTURNGEN(const QByteArray &params)
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

OpcodeTURN::OpcodeTURN(const QByteArray &params)
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

OpcodeDIRA::OpcodeDIRA(const QByteArray &params)
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

OpcodeGETAXY::OpcodeGETAXY(const QByteArray &params)
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

OpcodeGETAI::OpcodeGETAI(const QByteArray &params)
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

OpcodeANIMX2::OpcodeANIMX2(const QByteArray &params)
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

OpcodeCC::OpcodeCC(const QByteArray &params)
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

OpcodeAXYZI::OpcodeAXYZI(const QByteArray &params)
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

OpcodeLADER::OpcodeLADER(const QByteArray &params)
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

OpcodeOFST::OpcodeOFST(const QByteArray &params)
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

OpcodeOFSTW::OpcodeOFSTW()
{
}

QString OpcodeOFSTW::toString() const
{
	return QObject::tr("Attendre la fin de l'exécution de l'Offset Object pour continuer");
}

OpcodeTALKR::OpcodeTALKR(const QByteArray &params)
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

OpcodeSLIDR::OpcodeSLIDR(const QByteArray &params)
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

OpcodeSOLID::OpcodeSOLID(const QByteArray &params)
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

OpcodeIFPRTYQ::OpcodeIFPRTYQ(const QByteArray &params) :
	OpcodeJump()
{
	charID = params.at(0);
	_jump = params.at(1) + 2;
}

QString OpcodeIFPRTYQ::toString() const
{
	return QObject::tr("Si %1 est dans l'équipe actuelle (aller à l'octet %2 sinon)")
			.arg(_personnage(charID))
			.arg(_label + _jump);
}

QByteArray OpcodeIFPRTYQ::params() const
{
	return QByteArray()
			.append((char)charID)
			.append(char(_jump - 2));
}

OpcodeIFMEMBQ::OpcodeIFMEMBQ(const QByteArray &params) :
	OpcodeJump()
{
	charID = params.at(0);
	_jump = params.at(1) + 2;
}

QString OpcodeIFMEMBQ::toString() const
{
	return QObject::tr("Si %1 existe (aller à l'octet %2 sinon)")
			.arg(_personnage(charID))
			.arg(_label + _jump);
}

QByteArray OpcodeIFMEMBQ::params() const
{
	return QByteArray()
			.append((char)charID)
			.append(char(_jump - 2));
}

OpcodeMMBUD::OpcodeMMBUD(const QByteArray &params)
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

OpcodeSIN::OpcodeSIN(const QByteArray &params)
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

OpcodeCOS::OpcodeCOS(const QByteArray &params)
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

OpcodeTLKR2::OpcodeTLKR2(const QByteArray &params)
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

OpcodeSLDR2::OpcodeSLDR2(const QByteArray &params)
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

OpcodePMJMP::OpcodePMJMP(const QByteArray &params)
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

OpcodeBGOFF::OpcodeBGOFF(const QByteArray &params)
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

OpcodeBGROL::OpcodeBGROL(const QByteArray &params)
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

OpcodeBGROL2::OpcodeBGROL2(const QByteArray &params)
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

OpcodeBGCLR::OpcodeBGCLR(const QByteArray &params)
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

OpcodeSTPAL::OpcodeSTPAL(const QByteArray &params)
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

OpcodeSTPLS::OpcodeSTPLS(const QByteArray &params)
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

OpcodeAKAO::OpcodeAKAO(const QByteArray &params)
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

OpcodeMVCAM::OpcodeMVCAM(const QByteArray &params)
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

OpcodeGAMEOVER::OpcodeGAMEOVER()
{
}

QString OpcodeGAMEOVER::toString() const
{
	return QObject::tr("Game Over");
}
