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

#include <QtCore>
#include "FF7Text.h"
#include "Opcode.h"

class Script
{
public:
	Script();
	explicit Script(const QByteArray &script);
	virtual ~Script();

	bool openScript(const QByteArray &script);
	int size() const;
	bool isEmpty() const;
	bool isValid() const;
	Opcode *getOpcode(quint16 opcodeID) const;
	const QList<Opcode *> &getOpcodes() const;
	bool isVoid() const;
	QByteArray toByteArray() const;
	void setOpcode(quint16 opcodeID, Opcode *opcode);
	void delOpcode(quint16 opcodeID);
	Opcode *removeOpcode(quint16 opcodeID);
	void insertOpcode(quint16 opcodeID, Opcode *opcode);
	bool moveOpcode(quint16 opcodeID, bool direction);
	void shiftTextIds(int textId, int steps=1);
	void shiftTutIds(int tutId, int steps=1);

	bool searchOpcode(int opcode, int &opcodeID) const;
	bool searchVar(quint8 bank, quint8 adress, int value, int &opcodeID) const;
	QList<FF7Var> searchAllVars() const;
	bool searchExec(quint8 group, quint8 script, int &opcodeID) const;
	bool searchMapJump(quint16 field, int &opcodeID) const;
	bool searchText(const QRegExp &text, int &opcodeID) const;
	bool searchOpcodeP(int opcode, int &opcodeID) const;
	bool searchVarP(quint8 bank, quint8 adress, int value, int &opcodeID) const;
	bool searchExecP(quint8 group, quint8 script, int &opcodeID) const;
	bool searchMapJumpP(quint16 field, int &opcodeID) const;
	bool searchTextP(const QRegExp &text, int &opcodeID) const;
	void listUsedTexts(QSet<quint8> &usedTexts) const;
	void listUsedTuts(QSet<quint8> &usedTuts) const;
	void listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const;
	void getBgParams(QHash<quint8, quint8> &paramActifs) const;
	void getBgMove(qint16 z[2], qint16 *x, qint16 *y) const;

	const QList<Opcode *> &getExpandedItems() const;
	void setExpandedItems(const QList<Opcode *> &expandedItems);

	static int posReturn(const QByteArray &script);
	static Opcode *createOpcode(const QByteArray &script, int pos=0);
	static Opcode *copyOpcode(Opcode *opcode);
private:
	QList<Opcode *> opcodes;
	QList<Opcode *> expandedItems;

	bool valid;
};

#endif
