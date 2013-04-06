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
#ifndef SECTION1FILE_H
#define SECTION1FILE_H

#include <QtCore>
#include "GrpScript.h"
#include "../FF7Text.h"
#include "TutFile.h"

class Section1File
{
public:
	Section1File();
	virtual ~Section1File();
	void clear();
	bool open(const QByteArray &data);
	QByteArray save(const QByteArray &data) const;
	bool isOpen() const;
	bool isModified() const;
	void setModified(bool modified);

	int modelID(quint8 grpScriptID) const;
	void bgParamAndBgMove(QHash<quint8, quint8> &paramActifs, qint16 *z=0, qint16 *x=0, qint16 *y=0) const;

	const QList<GrpScript *> &grpScripts() const;
	GrpScript *grpScript(int groupID) const;
	int grpScriptCount() const;
	void insertGrpScript(int row);
	void insertGrpScript(int row, GrpScript *grpScript);
	void deleteGrpScript(int row);
	void removeGrpScript(int row);
	bool moveGrpScript(int row, bool direction);

	void searchAllVars(QList<FF7Var> &vars) const;
	bool searchOpcode(int opcode, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchVar(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchExec(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchMapJump(quint16 field, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchTextInScripts(const QRegExp &text, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchText(const QRegExp &text, int &textID, int &from, int &size) const;
	bool searchOpcodeP(int opcode, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchVarP(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchExecP(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchMapJumpP(quint16 mapJump, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchTextInScriptsP(const QRegExp &text, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchTextP(const QRegExp &text, int &textID, int &from, int &index, int &size) const;
	void setWindow(const FF7Window &win);
	void listWindows(QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const;

	void shiftTutIds(int row, int shift);

	QList<FF7Text *> *texts();
	int textCount() const;
	FF7Text *text(int textID) const;
	void insertText(int row);
	void deleteText(int row);
	QSet<quint8> listUsedTexts() const;
	QSet<quint8> listUsedTuts() const;

	const QString &author() const;
	void setAuthor(const QString &author);

	quint16 scale() const;
	void setScale(quint16 scale);

	TutFile *tut() const;
	void setTut(TutFile *tut);
private:
	bool modified, opened;

	QString _author;
	quint16 _scale;
	// quint8 nbObjets3D;

	QList<GrpScript *> _grpScripts;
	QList<FF7Text *> _texts;
	TutFile *_tut;
};

#endif // SECTION1FILE_H
