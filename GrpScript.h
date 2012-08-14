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
#ifndef DEF_GRPSCRIPT
#define DEF_GRPSCRIPT

#include <QtGui>
#include "FF7Text.h"
#include "Script.h"

class GrpScript
{	
public:
	GrpScript();
	GrpScript(const QString &name);
	virtual ~GrpScript();

	void addScript();
	bool addScript(const QByteArray &script, bool explodeInit=true);
	// void replaceScript(int row, QByteArray script=QByteArray());

	QString getName() const;
	QString getRealName() const;
	void setName(const QString &name);
	int size() const;
	Script *getScript(quint8 scriptID) const;
	QByteArray getRealScript(quint8 scriptID) const;
	void getBgParams(QHash<quint8, quint8> &paramActifs) const;
	void getBgMove(qint16 z[2], qint16 *x=0, qint16 *y=0) const;
	int getTypeID();
	void setType();
	QString getType();
	QColor getTypeColor();
	QString getScriptName(quint8 scriptID);

	bool rechercherOpcode(int opcode, int &scriptID, int &opcodeID) const;
	bool rechercherVar(quint8 bank, quint8 adress, int value, int &scriptID, int &opcodeID) const;
	QList<FF7Var> searchAllVars() const;
	bool rechercherExec(quint8 group, quint8 script, int &scriptID, int &opcodeID) const;
	bool rechercherTexte(const QRegExp &texte, int &scriptID, int &opcodeID) const;
	bool rechercherOpcodeP(int opCode, int &scriptID, int &opcodeID) const;
	bool rechercherVarP(quint8 bank, quint8 adress, int value, int &scriptID, int &opcodeID) const;
	bool rechercherExecP(quint8 group, quint8 script, int &scriptID, int &opcodeID) const;
	bool rechercherTexteP(const QRegExp &texte, int &scriptID, int &opcodeID) const;
	void listUsedTexts(QSet<quint8> &usedTexts) const;
	void listUsedTuts(QSet<quint8> &usedTuts) const;
	void shiftTextIds(int textId, int steps=1);
	void shiftTutIds(int tutId, int steps=1);
	void listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const;
private:
	QString name;
	QList<Script *> scripts;

	qint16 character;
	bool animation;
	bool location;
	bool director;
	
};

#endif
