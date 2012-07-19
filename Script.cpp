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
#include "Script.h"

Script::Script()
{
}

Script::Script(const QByteArray &script)
{
	quint16 pos = 0, scriptSize = script.size(), longueur;
	quint8 opcode;
	while(pos < scriptSize)
	{
		longueur = 0;
		switch(opcode = (quint8)script.at(pos))
		{
		case 0x0F://SPECIAL
			switch((quint8)script.at(pos+1))
			{
			case 0xF5:case 0xF6:case 0xF7:case 0xFB:case 0xFC:
						longueur = 1;
				break;
			case 0xF8:case 0xFD:
						longueur = 2;
				break;
			}
			break;
		case 0x28://KAWAI
			longueur = (quint8)script.at(pos+1);
			break;
		}
		longueur += Opcode::length[opcode];
		commandes.append(new Commande(script.mid(pos, longueur), pos));
		pos += longueur;
	}
}

Script::~Script()
{
	foreach(Commande *commande, commandes)	delete commande;
}

int Script::posReturn(const QByteArray &script)
{
	quint16 pos=0, longueur, param16;
	int scriptSize = script.size();
	quint8 clef;
	
	if(scriptSize <= 0)	return -1;
	
	while(pos < scriptSize)
	{
		longueur = 0;
		switch(clef = (quint8)script.at(pos))
		{
		case 0x10://Saut
			pos += (quint8)script.at(pos+1) - 1;
			break;
		case 0x11://Saut
			memcpy(&param16, script.mid(pos+1,2), 2);
			pos += param16 - 1;
			break;
		case 0x14://If -> Saut
			pos += (quint8)script.at(pos+5) - 1;
			break;
		case 0x15://If -> Saut
			memcpy(&param16, script.mid(pos+5,2), 2);
			pos += param16 - 1;
			break;
		case 0x16:case 0x18://If -> Saut
					pos += (quint8)script.at(pos+7) - 1;
			break;
		case 0x17:case 0x19://If -> Saut
					memcpy(&param16, script.mid(pos+7,2), 2);
			pos += param16 - 1;
			break;
		case 0x0F://SPECIAL
			switch((quint8)script.at(pos+1))
			{
			case 0xF5:case 0xF6:case 0xF7:case 0xFB:case 0xFC:
						longueur = 1;
				break;
			case 0xF8:case 0xFD:
						longueur = 2;
				break;
			}
			break;
		case 0x28://KAWAI
			longueur = (quint8)script.at(pos+1);
			break;
		}
		pos += Opcode::length[clef] + longueur;
		if(clef == 0x00 || clef == 0x07)	break;
	}
	
	return pos;
}

int Script::size() const
{
	return commandes.size();
}

bool Script::isEmpty() const
{
	return commandes.isEmpty();
}

Commande *Script::getCommande(quint16 commandeID)
{
	// if(commandeID >= commandes.size())	return NULL;
	return commandes.at(commandeID);
}

QByteArray Script::toByteArray() const
{
	QByteArray ret;
	foreach(Commande *commande, commandes)
		ret.append(commande->toByteArray());
	return ret;
}

bool Script::isVoid() const
{
	foreach(Commande *commande, commandes)
		if(!commande->isVoid())	return false;
	return true;
}

void Script::setCommande(quint16 commandeID, const QByteArray &commande)
{
	Commande *curCommand = commandes.at(commandeID);
	int oldSize = curCommand->size();
	curCommand->setCommande(commande);
	shiftJumps(commandeID, curCommand->size()-oldSize);
}

void Script::delCommande(quint16 commandeID)
{
	int oldSize = commandes.at(commandeID)->size();
	delete commandes.takeAt(commandeID);
	shiftJumps(commandeID, -oldSize);
}

Commande *Script::removeCommande(quint16 commandeID)
{
	int oldSize = commandes.at(commandeID)->size();
	Commande *commande = commandes.takeAt(commandeID);
	shiftJumps(commandeID, -oldSize);
	return commande;
}

void Script::insertCommande(quint16 commandeID, const QByteArray &commande)
{
	insertCommande(commandeID, new Commande(commande));
}

void Script::insertCommande(quint16 commandeID, Commande *commande)
{
	commandes.insert(commandeID, commande);
	shiftJumps(commandeID, commande->size());
}

bool Script::moveCommande(quint16 commandeID, bool direction)
{
	if(commandeID >= commandes.size())	return false;

	Commande *commande;
	
	if(direction) // DOWN
	{
		if(commandeID == commandes.size()-1)	return false;
		commande = removeCommande(commandeID);
		insertCommande(commandeID+1, commande);
		shiftJumpsSwap(commande, -commandes.at(commandeID)->size());
	}
	else // UP
	{
		if(commandeID == 0)	return false;
		commande = removeCommande(commandeID);
		insertCommande(commandeID-1, commande);
		shiftJumpsSwap(commande, commandes.at(commandeID)->size());
	}
	return true;
}

bool Script::rechercherOpCode(quint8 opCode, int &commandeID) const
{
	if(commandeID < 0)	commandeID = 0;

	int nbCommandes = commandes.size();
	while(commandeID < nbCommandes)
	{
		if(opCode == commandes.at(commandeID)->getOpcode())	return true;
		++commandeID;
		// qDebug() << "SCRIPT_commandeID " << commandeID;
	}
	return false;
}

bool Script::rechercherVar(quint8 bank, quint8 adress, int value, int &commandeID) const
{
	if(commandeID < 0)	commandeID = 0;

	int nbCommandes = commandes.size();
	while(commandeID < nbCommandes)
	{
		if(commandes.at(commandeID)->rechercherVar(bank, adress, value))	return true;
		++commandeID;
	}
	return false;
}

QList<int> Script::searchAllVars() const
{
	QList<int> vars;

	foreach(Commande *commande, commandes) {
		vars.append(commande->searchAllVars());
	}

	return vars;
}

bool Script::rechercherExec(quint8 group, quint8 script, int &commandeID) const
{
	if(commandeID < 0)	commandeID = 0;

	int nbCommandes = commandes.size();
	while(commandeID < nbCommandes)
	{
		if(commandes.at(commandeID)->rechercherExec(group, script))	return true;
		++commandeID;
	}
	return false;
}

bool Script::rechercherTexte(const QRegExp &texte, int &commandeID) const
{
	if(commandeID < 0)	commandeID = 0;

	int nbCommandes = commandes.size();
	while(commandeID < nbCommandes)
	{
		if(commandes.at(commandeID)->rechercherTexte(texte))	return true;
		++commandeID;
	}
	return false;
}

bool Script::rechercherOpCodeP(quint8 opCode, int &commandeID) const
{
	if(commandeID >= commandes.size())	commandeID = commandes.size()-1;
	
	while(commandeID >= 0)
	{
		if(opCode == commandes.at(commandeID)->getOpcode())	return true;
		--commandeID;
	}

	return false;
}

bool Script::rechercherVarP(quint8 bank, quint8 adress, int value, int &commandeID) const
{
	if(commandeID >= commandes.size())	commandeID = commandes.size()-1;
	
	while(commandeID >= 0)
	{
		if(commandes.at(commandeID)->rechercherVar(bank, adress, value))	return true;
		--commandeID;
	}

	return false;
}

bool Script::rechercherExecP(quint8 group, quint8 script, int &commandeID) const
{
	if(commandeID >= commandes.size())	commandeID = commandes.size()-1;

	while(commandeID >= 0)
	{
		if(commandes.at(commandeID)->rechercherExec(group, script))	return true;
		--commandeID;
	}

	return false;
}

bool Script::rechercherTexteP(const QRegExp &texte, int &commandeID) const
{
	if(commandeID >= commandes.size())	commandeID = commandes.size()-1;
	
	while(commandeID >= 0)
	{
		if(commandes.at(commandeID)->rechercherTexte(texte))	return true;
		--commandeID;
	}

	return false;
}

void Script::listUsedTexts(QSet<quint8> &usedTexts) const
{
	foreach(Commande *commande, commandes)
		commande->listUsedTexts(usedTexts);
}

void Script::listUsedTuts(QSet<quint8> &usedTuts) const
{
	foreach(Commande *commande, commandes)
		commande->listUsedTuts(usedTuts);
}

void Script::shiftTextIds(int textId, int steps)
{
	foreach(Commande *commande, commandes)
		commande->shiftTextIds(textId, steps);
}

void Script::shiftTutIds(int tutId, int steps)
{
	foreach(Commande *commande, commandes)
		commande->shiftTutIds(tutId, steps);
}

void Script::listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const
{
	foreach(Commande *commande, commandes)
		commande->listWindows(windows, text2win);
}

int Script::posOfCommand(int commandID) const
{
	int currentCommandID=0, pos=0;

	foreach(Commande *commande, commandes)
	{
		if(commandID==currentCommandID)
		{
			return pos;
		}
		++currentCommandID;
		pos += commande->size();
	}

	return pos;
}

void Script::shiftJumps(int commandID, int shift)
{
	if(shift == 0)	return;

	int currentCommandID=0, pos=posOfCommand(commandID), currentPos=0, jump;

	foreach(Commande *commande, commandes)
	{
		if(currentCommandID < commandID)
		{
			switch(commande->getOpcode())
			{
			case 0x10:// JMPF (Jump)
				jump = commande->getConstParams().at(0);
				if(currentPos + 1 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[0] = jump;
//						qDebug() << "JMPF" << (jump-shift) << jump;
					} else if(jump >= 0 && jump < 65536)// -> JMPFL
					{
						commande->setCommande(QByteArray().append('\x11').append((char *)&jump, 2));
//						qDebug() << "JMPF -> JMPFL" << (jump-shift) << jump;
					}
				}
				break;
			case 0x11:// JMPFL (Long Jump)
				jump=0;
				memcpy(&jump, commande->getConstParams().mid(0,2).constData(), 2);
				if(currentPos + 1 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 65536) {
						memcpy(commande->getParams().data(), &jump, 2);
//						qDebug() << "JMPFL" << (jump-shift) << jump;
					}
				}
				break;
			case 0x14:// IFUB (Jump)
				jump = commande->getConstParams().at(4);
				if(currentPos + 5 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[4] = jump;
//						qDebug() << "IFUB" << (jump-shift) << jump;
					}
					else if(jump >= 0 && jump < 65536)// -> IFUBL
					{
						commande->setCommande(QByteArray().append('\x15').append(commande->getConstParams().left(4)).append((char *)&jump, 2));
//						qDebug() << "IFUB -> IFUBL" << (jump-shift) << jump;
					}
				}
				break;
			case 0x15:// IFUBL (Long Jump)
				jump=0;
				memcpy(&jump, commande->getConstParams().mid(4,2).constData(), 2);
				if(currentPos + 5 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 65536) {
						memcpy(commande->getParams().data() + 4, &jump, 2);
//						qDebug() << "IFUBL" << (jump-shift) << jump;
					}
				}
				break;
			case 0x16:// IFSW (Jump)
			case 0x18:// IFUW (Jump)
				jump = commande->getConstParams().at(6);
				if(currentPos + 7 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[6] = jump;
//						qDebug() << (commande->getOpcode()==0x16 ? "IFSW" : "IFUW") << (jump-shift) << jump;
					} else if(jump >= 0 && jump < 65536)// -> IFSWL/IFUWL
					{
						commande->setCommande(QByteArray().append(commande->getOpcode()==0x16 ? '\x17' : '\x19').append(commande->getConstParams().left(6)).append((char *)&jump, 2));
//						qDebug() << (commande->getOpcode()==0x16 ? "IFSW -> IFSWL" : "IFUW -> IFUWL") << (jump-shift) << jump;
					}
				}
				break;
			case 0x17:// IFSWL (Long Jump)
			case 0x19:// IFUWL (Long Jump)
				jump=0;
				memcpy(&jump, commande->getConstParams().mid(6,2).constData(), 2);
				if(currentPos + 7 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 65536) {
						memcpy(commande->getParams().data() + 6, &jump, 2);
//						qDebug() << (commande->getOpcode()==0x17 ? "IFSWL" : "IFUWL") << (jump-shift) << jump;
					}
				}
				break;
			case 0x30:// IFKEY (Jump)
			case 0x31:// IFKEYON (Jump)
			case 0x32:// IFKEYOFF (Jump)
				jump = commande->getConstParams().at(2);
				if(currentPos + 3 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[2] = jump;
//						qDebug() << (commande->getOpcode()==0x30 ? "IFKEY" : (commande->getOpcode()==0x31 ? "IFKEYON" : "IFKEYOFF")) << (jump-shift) << jump;
					}
				}
				break;
			case 0xCB:// IFPRTYQ (Jump)
			case 0xCC:// IFMEMBQ (Jump)
				jump = commande->getConstParams().at(1);
				if(currentPos + 2 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[1] = jump;
//						qDebug() << (commande->getOpcode()==0xCB ? "IFPRTYQ" : "IFMEMBQ") << (jump-shift) << jump;
					}
				}
				break;
			}
		}
		else
		{
			switch(commande->getOpcode())
			{
			case 0x12:// JMPB (Jump Forward)
				jump = commande->getConstParams().at(0);
				if(currentPos - jump > pos) {
					jump -= shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[0] = jump;
//						qDebug() << "JMPB" << (jump-shift) << jump;
					}
					else if(jump >= 0 && jump < 65536)// -> JMPBL
					{
						commande->setCommande(QByteArray().append('\x13').append((char *)&jump, 2));
//						qDebug() << "JMPB -> JMPBL" << (jump-shift) << jump;
					}
				}
				break;
			case 0x13:// JMPBL (Long Jump Forward)
				jump=0;
				memcpy(&jump, commande->getConstParams().mid(0,2).constData(), 2);
				if(currentPos - jump > pos) {
					jump -= shift;
					if(jump >= 0 && jump < 65536) {
						memcpy(commande->getParams().data(), &jump, 2);
//						qDebug() << "JMPBL" << (jump-shift) << jump;
					}
				}
				break;
			}
		}
		++currentCommandID;
		currentPos += commande->size();
	}
}

void Script::shiftJumpsSwap(Commande *commande, int shift)
{
	if(shift == 0)	return;

	int jump;

	switch(commande->getOpcode())
	{
	case 0x10:// JMPF (Jump)
		jump = commande->getConstParams().at(0) + shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[0] = jump;
//			qDebug() << "JMPF" << (jump-shift) << jump;
		} else if(jump >= 0 && jump < 65536)// -> JMPFL
		{
			commande->setCommande(QByteArray().append('\x11').append((char *)&jump, 2));
//			qDebug() << "JMPF -> JMPFL" << (jump-shift) << jump;
		}
		break;
	case 0x11:// JMPFL (Long Jump)
		jump=0;
		memcpy(&jump, commande->getConstParams().mid(0,2).constData(), 2);
		jump += shift;
		if(jump >= 0 && jump < 65536) {
			memcpy(commande->getParams().data(), &jump, 2);
//			qDebug() << "JMPFL" << (jump-shift) << jump;
		}
		break;
	case 0x12:// JMPB (Jump Forward)
		jump = commande->getConstParams().at(0) - shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[0] = jump;
//			qDebug() << "JMPB" << (jump-shift) << jump;
		}
		else if(jump >= 0 && jump < 65536)// -> JMPBL
		{
			commande->setCommande(QByteArray().append('\x13').append((char *)&jump, 2));
//			qDebug() << "JMPB -> JMPBL" << (jump-shift) << jump;
		}
		break;
	case 0x13:// JMPBL (Long Jump Forward)
		jump=0;
		memcpy(&jump, commande->getConstParams().mid(0,2).constData(), 2);
		jump -= shift;
		if(jump >= 0 && jump < 65536) {
			memcpy(commande->getParams().data(), &jump, 2);
//			qDebug() << "JMPBL" << (jump-shift) << jump;
		}
		break;
	case 0x14:// IFUB (Jump)
		jump = commande->getConstParams().at(4) + shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[4] = jump;
//			qDebug() << "IFUB" << (jump-shift) << jump;
		}
		else if(jump >= 0 && jump < 65536)// -> IFUBL
		{
			commande->setCommande(QByteArray().append('\x15').append(commande->getConstParams().left(4)).append((char *)&jump, 2));
//			qDebug() << "IFUB -> IFUBL" << (jump-shift) << jump;
		}
		break;
	case 0x15:// IFUBL (Long Jump)
		jump=0;
		memcpy(&jump, commande->getConstParams().mid(4,2).constData(), 2);
		jump += shift;
		if(jump >= 0 && jump < 65536) {
			memcpy(commande->getParams().data() + 4, &jump, 2);
//			qDebug() << "IFUBL" << (jump-shift) << jump;
		}
		break;
	case 0x16:// IFSW (Jump)
	case 0x18:// IFUW (Jump)
		jump = commande->getConstParams().at(6) + shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[6] = jump;
//			qDebug() << (commande->getOpcode()==0x16 ? "IFSW" : "IFUW") << (jump-shift) << jump;
		} else if(jump >= 0 && jump < 65536)// -> IFSWL/IFUWL
		{
			commande->setCommande(QByteArray().append(commande->getOpcode()==0x16 ? '\x17' : '\x19').append(commande->getConstParams().left(6)).append((char *)&jump, 2));
//			qDebug() << (commande->getOpcode()==0x16 ? "IFSW -> IFSWL" : "IFUW -> IFUWL") << (jump-shift) << jump;
		}
		break;
	case 0x17:// IFSWL (Long Jump)
	case 0x19:// IFUWL (Long Jump)
		jump=0;
		memcpy(&jump, commande->getConstParams().mid(6,2).constData(), 2);
		jump += shift;
		if(jump >= 0 && jump < 65536) {
			memcpy(commande->getParams().data() + 6, &jump, 2);
//			qDebug() << (commande->getOpcode()==0x17 ? "IFSWL" : "IFUWL") << (jump-shift) << jump;
		}
		break;
	case 0x30:// IFKEY (Jump)
	case 0x31:// IFKEYON (Jump)
	case 0x32:// IFKEYOFF (Jump)
		jump = commande->getConstParams().at(2) + shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[2] = jump;
//			qDebug() << (commande->getOpcode()==0x30 ? "IFKEY" : (commande->getOpcode()==0x31 ? "IFKEYON" : "IFKEYOFF")) << (jump-shift) << jump;
		}
		break;
	case 0xCB:// IFPRTYQ (Jump)
	case 0xCC:// IFMEMBQ (Jump)
		jump = commande->getConstParams().at(1) + shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[1] = jump;
//			qDebug() << (commande->getOpcode()==0xCB ? "IFPRTYQ" : "IFMEMBQ") << (jump-shift) << jump;
		}
		break;
	}
}

void Script::lecture(QTreeWidget *zoneScript)
{
	indent.clear();
	QList<QTreeWidgetItem *> row;
	QTreeWidgetItem *parentItem = 0;
	QString description;
	quint8 clef;
	quint16 commandeID = 0, nbCommandes = commandes.size(), pos = 0;
	Commande *curCommande;
	QPixmap fontPixmap(":/images/chiffres.png");
	bool error = false;
	
	while(commandeID < nbCommandes)
	{
		curCommande = commandes.at(commandeID);
		curCommande->setPos(pos);
		
		while(!indent.isEmpty() && pos>=indent.last())
		{
			if(pos>indent.last())	error = true;
			indent.removeLast();
			parentItem = parentItem->parent();
		}
		
		description = curCommande->traduction();
		clef = curCommande->getOpcode();
		
		/* description += QString(" | %1 || ").arg(clef,0,16);
		for(quint8 cur=0 ; cur<curCommande->getConstParams().size() ; ++cur)
		{
			description += QString("%1 | ").arg((quint8)curCommande->getConstParams().at(cur));
		} */
		
		/* description += " | ";
		for(quint8 cur=0 ; cur<indent.size() ; ++cur)
		{
			description += QString("%1 | ").arg(indent.at(cur));
		} */ //DEBUG
		QTreeWidgetItem *item = new QTreeWidgetItem(parentItem, QStringList() << description << QString::number(commandeID));
		row.append(item);
		
		// pos += Data::opcode_length[clef];
		
		//ERREUR
		/* if(pos>script.size())
		{
			description += " " + QObject::tr("Erreur : dépassement") + "";
			item->setBackground(0, QColor(0xFF,0xCC,0xCC));
		} */
		QPixmap wordPixmap(26,11);
		item->setIcon(0, QIcon(posNumber(pos, fontPixmap, wordPixmap)));
		if((clef>=0x14 && clef<=0x19) || (clef>=0x30 && clef<=0x32) || clef==0xcb || clef==0xcc)
		{
			item->setForeground(0, QColor(0x00,0x66,0xcc));
			indent.append(curCommande->getIndent());
			parentItem = item;
		}
		else if(clef>=0x01 && clef<=0x07)
			item->setForeground(0, QColor(0xcc,0x66,0x00));
		else if(clef>=0x10 && clef<=0x13)
			item->setForeground(0, QColor(0x66,0xcc,0x00));
		else if(clef==0x00 || clef==0x07)
			item->setForeground(0, QColor(0x66,0x66,0x66));

		if(error)
		{
			item->setBackground(0, QColor(0xFF,0xCC,0xCC));
			error = false;
		}

		pos += curCommande->size();
		++commandeID;
	}

	zoneScript->addTopLevelItems(row);

	commandeID = 0;
	foreach(QTreeWidgetItem *item, row) {
		if(expandedItems.contains(commandes.at(commandeID))) {
			item->setExpanded(true);
		}
		++commandeID;
	}
}

void Script::setExpandedItems(const QList<Commande *> &expandedItems)
{
	this->expandedItems = expandedItems;
}

QPixmap &Script::posNumber(int num, const QPixmap &fontPixmap, QPixmap &wordPixmap)
{
	QString strNum = QString("%1").arg(num, 4, 10, QChar(' '));
	wordPixmap.fill(QColor(0,0,0,0));
	QPainter painter(&wordPixmap);
	
	if(strNum.at(0)!=' ')
		painter.drawTiledPixmap(1, 1, 5, 9, fontPixmap, 5*strNum.mid(0,1).toInt(), 0);
	if(strNum.at(1)!=' ')
		painter.drawTiledPixmap(7, 1, 5, 9, fontPixmap, 5*strNum.mid(1,1).toInt(), 0);
	if(strNum.at(2)!=' ')
		painter.drawTiledPixmap(13, 1, 5, 9, fontPixmap, 5*strNum.mid(2,1).toInt(), 0); 
	if(strNum.at(3)!=' ')
		painter.drawTiledPixmap(19, 1, 5, 9, fontPixmap, 5*strNum.mid(3,1).toInt(), 0);
	
	painter.end();
	return wordPixmap;
}
