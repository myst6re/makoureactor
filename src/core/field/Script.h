/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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
#pragma once

#include <QtCore>
#include "Opcode.h"

class Script
{
public:
	enum MoveDirection {
		Up, Down
	};

	Script();
	explicit Script(const QList<Opcode> &opcodes);
	Script(const char *script, qsizetype size);

	bool openScript(const char *script, qsizetype size);
	qsizetype size() const;
	bool isEmpty() const;
	bool isValid() const;
	Opcode &opcode(qsizetype opcodeID);
	const Opcode &opcode(qsizetype opcodeID) const;
	QList<Opcode> &opcodes();
	const QList<Opcode> &opcodes() const;
	bool isVoid() const;
	bool compile(int &opcodeID, QString &errorStr);
	QByteArray toByteArray() const;
	inline QByteArray serialize() const {
		return toByteArray();
	}
	void setOpcode(qsizetype opcodeID, const Opcode &opcode);
	void removeOpcode(qsizetype opcodeID);
	void insertOpcode(qsizetype opcodeID, const Opcode &opcode);
	bool moveOpcode(qsizetype opcodeID, MoveDirection direction);
	void shiftGroupIds(quint8 groupId, qint16 steps = 1);
	void shiftTextIds(quint8 textId, qint16 steps = 1);
	void shiftTutIds(quint8 tutId, qint16 steps = 1);
	void shiftPalIds(quint8 palId, qint16 steps = 1);
	void swapGroupIds(quint8 groupId1, quint8 groupId2);
	void setWindow(const FF7Window &win);
	quint32 opcodePositionInBytes(qsizetype opcodeID) const;

	bool searchOpcode(int opcode, int &opcodeID) const;
	bool searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &opcodeID) const;
	void searchAllVars(QList<FF7Var> &vars) const;
	bool searchExec(quint8 group, quint8 script, int &opcodeID) const;
	bool searchMapJump(quint16 map, int &opcodeID) const;
	bool searchTextInScripts(const QRegularExpression &text, int &opcodeID, const Section1File *scriptsAndTexts) const;
	bool searchOpcodeP(int opcode, int &opcodeID) const;
	bool searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &opcodeID) const;
	bool searchExecP(quint8 group, quint8 script, int &opcodeID) const;
	bool searchMapJumpP(quint16 map, int &opcodeID) const;
	bool searchTextInScriptsP(const QRegularExpression &text, int &opcodeID, const Section1File *scriptsAndTexts) const;
	void listUsedTexts(QSet<quint8> &usedTexts) const;
	void listUsedTuts(QSet<quint8> &usedTuts) const;
	void listWindows(int groupID, int scriptID, QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const;
	void listWindows(int groupID, int scriptID, int textID, QList<FF7Window> &windows, int winID = -1) const;
	void listModelPositions(QList<FF7Position> &positions) const;
	bool linePosition(FF7Position position[2]) const;
	void backgroundParams(QHash<quint8, quint8> &enabledParams) const;
	void backgroundMove(qint16 z[2], qint16 *x, qint16 *y) const;
	bool removeTexts();

	Script splitScriptAtReturn();

	QString toString(const Section1File *scriptsAndTexts) const;
private:
	QList<Opcode> _opcodes;
	QString lastError;

	bool valid;
};

QDataStream &operator<<(QDataStream &stream, const QList<Opcode> &script);
QDataStream &operator>>(QDataStream &stream, QList<Opcode> &script);
