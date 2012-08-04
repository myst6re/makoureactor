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
#ifndef DEF_SCRIPT
#define DEF_SCRIPT

#include <QtGui>
#include "FF7Text.h"
#include "Commande.h"

class Script
{
public:
	Script();
	explicit Script(const QByteArray &script);
	virtual ~Script();

	void openScript(const QByteArray &script);
	int size() const;
	bool isEmpty() const;
	Commande *getCommande(quint16 commandeID);
	bool isVoid() const;
	QByteArray toByteArray() const;
	void setCommande(quint16 commandeID, const QByteArray &commande);
	void delCommande(quint16 commandeID);
	Commande *removeCommande(quint16 commandeID);
	void insertCommande(quint16 commandeID, const QByteArray &commande);
	void insertCommande(quint16 commandeID, Commande *commande);
	bool moveCommande(quint16 commandeID, bool direction);
	void shiftTextIds(int textId, int steps=1);
	void shiftTutIds(int tutId, int steps=1);

	bool rechercherOpCode(quint8 opCode, int &commandeID) const;
	bool rechercherVar(quint8 bank, quint8 adress, int value, int &commandeID) const;
	QList<int> searchAllVars() const;
	bool rechercherExec(quint8 group, quint8 script, int &commandeID) const;
	bool rechercherTexte(const QRegExp &texte, int &commandeID) const;
	bool rechercherOpCodeP(quint8 opCode, int &commandeID) const;
	bool rechercherVarP(quint8 bank, quint8 adress, int value, int &commandeID) const;
	bool rechercherExecP(quint8 group, quint8 script, int &commandeID) const;
	bool rechercherTexteP(const QRegExp &texte, int &commandeID) const;
	void listUsedTexts(QSet<quint8> &usedTexts) const;
	void listUsedTuts(QSet<quint8> &usedTuts) const;
	void listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const;

	void lecture(QTreeWidget *zoneScript);
	void setExpandedItems(const QList<Commande *> &expandedItems);

	static int posReturn(const QByteArray &script);
private:
	static QPixmap &posNumber(int num, const QPixmap &fontPixmap, QPixmap &wordPixmap);
	void shiftJumps(int commandID, int shift);
	void shiftJumpsSwap(Commande *commande, int shift);
	int posOfCommand(int commandID) const;

	QList<Commande *> commandes;
	QList<quint16> indent;
	QList<Commande *> expandedItems;
};

#endif
