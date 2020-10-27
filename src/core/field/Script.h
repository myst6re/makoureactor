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
#include "Opcode.h"

typedef QListIterator<Opcode *> OpcodesIterator;

class Script
{
public:
	enum MoveDirection {
		Up, Down
	};

	Script();
	explicit Script(const QList<Opcode *> &opcodes);
	explicit Script(const QByteArray &script);
	Script(const QByteArray &script, int pos, int size);
	Script(const Script &other);
	virtual ~Script();

	bool openScript(const QByteArray &script, const int initPos, const int size);
	int size() const;
	bool isEmpty() const;
	bool isValid() const;
	Opcode *opcode(quint16 opcodeID) const;
	const QList<Opcode *> &opcodes() const;
	bool isVoid() const;
	bool compile(int &opcodeID, QString &errorStr);
	QByteArray toByteArray() const;
	inline QByteArray serialize() const {
		return toByteArray();
	}
	void setOpcode(quint16 opcodeID, Opcode *opcode);
	void delOpcode(quint16 opcodeID);
	Opcode *removeOpcode(quint16 opcodeID);
	void insertOpcode(quint16 opcodeID, Opcode *opcode);
	bool moveOpcode(quint16 opcodeID, MoveDirection direction);
	void shiftGroupIds(int groupId, int steps=1);
	void shiftTextIds(int textId, int steps=1);
	void shiftTutIds(int tutId, int steps=1);
	void swapGroupIds(int groupId1, int groupId2);
	void setWindow(const FF7Window &win);
	int opcodePositionInBytes(quint16 opcodeID);

	bool searchOpcode(int opcode, int &opcodeID) const;
	bool searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &opcodeID) const;
	void searchAllVars(QList<FF7Var> &vars) const;
	bool searchExec(quint8 group, quint8 script, int &opcodeID) const;
	bool searchMapJump(quint16 field, int &opcodeID) const;
	bool searchTextInScripts(const QRegExp &text, int &opcodeID, const Section1File *scriptsAndTexts) const;
	bool searchOpcodeP(int opcode, int &opcodeID) const;
	bool searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &opcodeID) const;
	bool searchExecP(quint8 group, quint8 script, int &opcodeID) const;
	bool searchMapJumpP(quint16 field, int &opcodeID) const;
	bool searchTextInScriptsP(const QRegExp &text, int &opcodeID, const Section1File *scriptsAndTexts) const;
	void listUsedTexts(QSet<quint8> &usedTexts) const;
	void listUsedTuts(QSet<quint8> &usedTuts) const;
	void listWindows(int groupID, int scriptID, QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const;
	void listWindows(int groupID, int scriptID, int textID, QList<FF7Window> &windows) const;
	void listModelPositions(QList<FF7Position> &positions) const;
	bool linePosition(FF7Position position[2]) const;
	void backgroundParams(QHash<quint8, quint8> &paramActifs) const;
	void backgroundMove(qint16 z[2], qint16 *x, qint16 *y) const;
	bool removeTexts();

	Script *splitScriptAtReturn();
	static Opcode *createOpcode(const QByteArray &script, int pos=0);
	static Opcode *copyOpcode(Opcode *opcode);

	QString toString(Field *field) const;
private:
	OpcodeJump *convertOpcodeJumpDirection(OpcodeJump *opcodeJump, bool *ok=nullptr) const;
	OpcodeJump *convertOpcodeJumpRangeToLong(OpcodeJump *opcodeJump, bool *ok=nullptr) const;
	bool canConvertOpcodeJumpRangeToLong(OpcodeJump *opcodeJump) const;
//	bool verifyOpcodeJumpRange(OpcodeJump *opcodeJump, QString &errorStr) const;
	QList<Opcode *> _opcodes;
	QString lastError;

	bool valid;
};

QDataStream &operator<<(QDataStream &stream, const QList<Opcode *> &script);
QDataStream &operator>>(QDataStream &stream, QList<Opcode *> &script);

#endif
