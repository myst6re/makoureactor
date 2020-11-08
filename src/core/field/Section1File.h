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
#include "FieldPart.h"
#include "GrpScript.h"
#include "../FF7Text.h"
#include "TutFileStandard.h"

class GrpScriptsIterator : public QListIterator<GrpScript *>
{
public:
	inline explicit GrpScriptsIterator(const QList<GrpScript *> &list)
		: QListIterator<GrpScript *>(list), _scriptsIt(0) {}
	GrpScriptsIterator(const GrpScriptsIterator &other);
	virtual ~GrpScriptsIterator();

	GrpScript * const &next();
	GrpScript * const &previous();

	/* There is no hasNextScript() function
	 * nextScript() can return nullptr
	 */
	Script *nextScript();
	Script *previousScript();

	/* There is no hasNextOpcode() function
	 * nextOpcode() can return nullptr
	 */
	Opcode *nextOpcode();
	Opcode *previousOpcode();
private:
	ScriptsIterator *_scriptsIt;
};

class Section1File : public FieldPart
{
public:
	enum ExportFormat {
		XMLText, TXTText
	};

	explicit Section1File(Field *field);
	Section1File(const Section1File &other);
	virtual ~Section1File();
	void clear();
	void initEmpty();
	bool open();
	bool open(const QByteArray &data);
	QByteArray save() const;
	bool exporter(QIODevice *device, ExportFormat format);
	bool importer(QIODevice *device, ExportFormat format);
	bool isModified() const;

	int modelID(quint8 grpScriptID) const;
	void bgParamAndBgMove(QHash<quint8, quint8> &paramActifs, qint16 *z=0, qint16 *x=0, qint16 *y=0) const;

	const QList<GrpScript *> &grpScripts() const;
	GrpScript *grpScript(int groupID) const;
	int grpScriptCount() const;
	inline static int maxGrpScriptCount() { return 256; }
	bool insertGrpScript(int row);
	bool insertGrpScript(int row, GrpScript *grpScript);
	void deleteGrpScript(int row);
	void removeGrpScript(int row);
	bool moveGrpScript(int row, bool direction);

	void searchAllVars(QList<FF7Var> &vars) const;
	bool searchOpcode(int opcode, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchExec(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchMapJump(quint16 field, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchTextInScripts(const QRegExp &text, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchText(const QRegExp &text, int &textID, int &from, int &size) const;
	bool searchOpcodeP(int opcode, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchExecP(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchMapJumpP(quint16 mapJump, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchTextInScriptsP(const QRegExp &text, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchTextP(const QRegExp &text, int &textID, int &from, int &index, int &size) const;
	void setWindow(const FF7Window &win);
	void listWindows(QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const;
	void listWindows(int textID, QList<FF7Window> &windows) const;
	void listModelPositions(QMultiMap<int, FF7Position> &positions) const;
	int modelCount() const;
	void linePosition(QMap<int, FF7Position *> &positions) const;

	void shiftTutIds(int row, int shift);
	bool compileScripts(int &groupID, int &scriptID, int &opcodeID, QString &errorStr);
	void removeTexts();
	void cleanTexts();
	void autosizeTextWindows();

	const QList<FF7Text> &texts() const;
	int textCount() const;
	inline static int maxTextCount() { return 256; }
	const FF7Text &text(int textID) const;
	void setText(int textID, const FF7Text &text);
	bool insertText(int textID, const FF7Text &text);
	bool replaceText(const QRegExp &search, const QString &after, int textID, int from);
	void deleteText(int textID);
	void clearTexts();
	QSet<quint8> listUsedTexts() const;
	QSet<quint8> listUsedTuts() const;

	const QString &author() const;
	void setAuthor(const QString &author);

	quint16 scale() const;
	void setScale(quint16 scale);

	int availableBytesForScripts() const;
private:
	QString _author;
	quint16 _scale;
	// quint8 nbObjets3D;
	quint16 _version;
	QByteArray _empty;

	QList<GrpScript *> _grpScripts;
	QList<FF7Text> _texts;
};

#endif // SECTION1FILE_H
