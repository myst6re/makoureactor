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

#include <QtCore>
#include <QColor>
#include "Script.h"

class ScriptsIterator : public QListIterator<Script *>
{
public:
	inline ScriptsIterator(const QList<Script *> &list)
		: QListIterator<Script *>(list), _opcodesIt(0) {}
	ScriptsIterator(const ScriptsIterator &other);
	virtual ~ScriptsIterator();

	Script * const &next();
	Script * const &previous();

	/* There is no hasNextOpcode() function
	 * nextOpcode() can return NULL
	 */
	Opcode *nextOpcode();
	Opcode *previousOpcode();
private:
	OpcodesIterator *_opcodesIt;
};

class GrpScript
{	
public:
	enum Type {
		NoType, Model, Location, Animation, Director
	};

	GrpScript();
	explicit GrpScript(const QString &name);
	GrpScript(const GrpScript &other);
	virtual ~GrpScript();

	static GrpScript *createGroupModel(quint8 modelID, int charID = -1);

	bool setScript(int row, const QByteArray &script);
	inline void setScript(int row, Script *script) {
		_scripts.replace(row, script);
	}

	QString name() const;
	inline const QString &realName() const {
		return _name;
	}
	inline void setName(const QString &name) {
		_name = name;
	}
	inline int size() const {
		return _scripts.size();
	}
	inline Script *script(quint8 scriptID) const {
		return _scripts.value(scriptID);
	}
	inline const QList<Script *> &scripts() const {
		return _scripts;
	}
	QByteArray toByteArray(quint8 scriptID) const;
	void backgroundParams(QHash<quint8, quint8> &paramActifs) const;
	void backgroundMove(qint16 z[2], qint16 *x=0, qint16 *y=0) const;
	Type typeID();
	QString type();
	inline qint16 character() const {
		return _character;
	}
	QColor typeColor();
	QString scriptName(quint8 scriptID);

	bool searchOpcode(int opcode, int &scriptID, int &opcodeID) const;
	bool searchVar(quint8 bank, quint8 adress, Opcode::Operation op, int value, int &scriptID, int &opcodeID) const;
	void searchAllVars(QList<FF7Var> &vars) const;
	bool searchExec(quint8 group, quint8 script, int &scriptID, int &opcodeID) const;
	bool searchMapJump(quint16 mapJump, int &scriptID, int &opcodeID) const;
	bool searchTextInScripts(const QRegExp &text, int &scriptID, int &opcodeID, const Section1File *scriptsAndTexts) const;
	bool searchP(int &scriptID, int &opcodeID) const;
	bool searchOpcodeP(int opCode, int &scriptID, int &opcodeID) const;
	bool searchVarP(quint8 bank, quint8 adress, Opcode::Operation op, int value, int &scriptID, int &opcodeID) const;
	bool searchExecP(quint8 group, quint8 script, int &scriptID, int &opcodeID) const;
	bool searchMapJumpP(quint16 field, int &scriptID, int &opcodeID) const;
	bool searchTextInScriptsP(const QRegExp &text, int &scriptID, int &opcodeID, const Section1File *scriptsAndTexts) const;
	void listUsedTexts(QSet<quint8> &usedTexts) const;
	void listUsedTuts(QSet<quint8> &usedTuts) const;
	void shiftGroupIds(int groupId, int steps=1);
	void shiftTextIds(int textId, int steps=1);
	void shiftTutIds(int tutId, int steps=1);
	void swapGroupIds(int groupId1, int groupId2);
	void setWindow(const FF7Window &win);
	void listWindows(int groupID, QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const;
	void listModelPositions(QList<FF7Position> &positions) const;
	bool linePosition(FF7Position position[2]) const;
	bool compile(int &scriptID, int &opcodeID, QString &errorStr);
	bool removeTexts();

	QString toString(Field *field) const;
private:
	void addScript();
	bool addScript(const QByteArray &script, bool explodeInit = true);
	void setType();
	bool search(int &scriptID, int &opcodeID) const;

	QString _name;
	QList<Script *> _scripts;

	qint16 _character;
	bool animation;
	bool location;
	bool director;
	
};

#endif
