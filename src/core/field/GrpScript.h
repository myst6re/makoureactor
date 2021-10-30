/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#include <QColor>
#include "Script.h"

#define SCRIPTS_SIZE 33

class GrpScript
{	
public:
	enum Type {
		NoType, Model, Location, Animation, Director
	};

	explicit GrpScript(const QList<Script> &scripts = QList<Script>());
	explicit GrpScript(const QString &name, const QList<Script> &scripts = QList<Script>());

	static GrpScript createGroupModel(quint8 modelID, qint16 charID = -1);

	void setScript(int row, const Script &script);

	QString name() const;
	inline const QString &realName() const {
		return _name;
	}
	inline void setName(const QString &name) {
		_name = name;
	}
	inline const Script &script(quint8 scriptID) const {
		return _scripts.at(scriptID);
	}
	inline Script &script(quint8 scriptID) {
		return _scripts[scriptID];
	}
	const QVarLengthArray<Script> &scripts() const {
		return _scripts;
	}
	QList<Script> scriptToList() const;
	QByteArray toByteArray(quint8 scriptID) const;
	void backgroundParams(QHash<quint8, quint8> &paramActifs) const;
	void backgroundMove(qint16 z[2], qint16 *x = nullptr, qint16 *y = nullptr) const;
	inline GrpScript::Type type() const {
		return _type;
	}
	QString typeString() const;
	inline qint16 character() const {
		return _character;
	}
	QColor typeColor() const;
	QString scriptName(quint8 scriptID) const;

	bool searchOpcode(int opcode, int &scriptID, int &opcodeID) const;
	bool searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &scriptID, int &opcodeID) const;
	void searchAllVars(QList<FF7Var> &vars) const;
	bool searchExec(quint8 group, quint8 script, int &scriptID, int &opcodeID) const;
	bool searchMapJump(quint16 mapJump, int &scriptID, int &opcodeID) const;
	bool searchTextInScripts(const QRegularExpression &text, int &scriptID, int &opcodeID, const Section1File *scriptsAndTexts) const;
	bool searchP(int &scriptID, int &opcodeID) const;
	bool searchOpcodeP(int opCode, int &scriptID, int &opcodeID) const;
	bool searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &scriptID, int &opcodeID) const;
	bool searchExecP(quint8 group, quint8 script, int &scriptID, int &opcodeID) const;
	bool searchMapJumpP(quint16 field, int &scriptID, int &opcodeID) const;
	bool searchTextInScriptsP(const QRegularExpression &text, int &scriptID, int &opcodeID, const Section1File *scriptsAndTexts) const;
	void listUsedTexts(QSet<quint8> &usedTexts) const;
	void listUsedTuts(QSet<quint8> &usedTuts) const;
	void shiftGroupIds(int groupId, int steps = 1);
	void shiftTextIds(int textId, int steps = 1);
	void shiftTutIds(int tutId, int steps = 1);
	void swapGroupIds(int groupId1, int groupId2);
	void setWindow(const FF7Window &win);
	void listWindows(int groupID, QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const;
	void listWindows(int groupID, int textID, QList<FF7Window> &windows) const;
	void listModelPositions(QList<FF7Position> &positions) const;
	bool linePosition(FF7Position position[2]) const;
	bool compile(int &scriptID, int &opcodeID, QString &errorStr);
	bool removeTexts();

	QString toString(Section1File *scriptsAndTexts) const;
private:
	void detectType();
	bool search(int &scriptID, int &opcodeID) const;

	QString _name;
	QVarLengthArray<Script> _scripts;

	Type _type;
	qint16 _character;
};

QDataStream &operator<<(QDataStream &stream, const QList<GrpScript> &scripts);
QDataStream &operator>>(QDataStream &stream, QList<GrpScript> &scripts);
QDataStream &operator<<(QDataStream &stream, const QList<Script> &scripts);
QDataStream &operator>>(QDataStream &stream, QList<Script> &scripts);
