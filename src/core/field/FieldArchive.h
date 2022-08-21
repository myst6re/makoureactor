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

//#define DEBUG_FUNCTIONS

#include <QtCore>
#include "FieldArchiveIO.h"
#include "Field.h"
#include <PsfFile.h>

struct SearchQuery
{
};

struct SearchOpcodeQuery : public SearchQuery
{
	int opcode;
	explicit SearchOpcodeQuery(int opcode) :
		opcode(opcode) {}
};

struct SearchVarQuery : public SearchQuery
{
	quint8 bank;
	quint16 address;
	Opcode::Operation op;
	int value;
	SearchVarQuery(quint8 bank, quint16 address, Opcode::Operation op, int value) :
		bank(bank), address(address), op(op), value(value) {}
};

struct SearchExecQuery : public SearchQuery
{
	quint8 group, script;
	SearchExecQuery(quint8 group, quint8 script) :
		group(group), script(script) {}
};

struct SearchFieldQuery : public SearchQuery
{
	int mapID;
	explicit SearchFieldQuery(int mapID) :
		mapID(mapID) {}
};

struct SearchTextQuery : public SearchQuery
{
	QRegularExpression text;
	explicit SearchTextQuery(QRegularExpression text) :
		text(text) {}
};

struct SearchIn
{
	virtual ~SearchIn();
	virtual void reset()=0;
	virtual void toEnd()=0;
};

struct SearchInScript : public SearchIn
{
	int &groupID, &scriptID, &opcodeID;

	SearchInScript(int &groupID, int &scriptID, int &opcodeID) :
		groupID(groupID), scriptID(scriptID), opcodeID(opcodeID)
	{}

	void reset() override;
	void toEnd() override;
};

struct SearchInText : public SearchIn
{
	int &textID;
	qsizetype &from, &size, &index;

	SearchInText(int &textID, qsizetype &from, qsizetype &size, qsizetype &index) :
		textID(textID), from(from), size(size), index(index)
	{}

	void reset() override;
	void toEnd() override;
};

class FieldArchive;

class FieldArchiveIterator : public QMapIterator<int, Field *>
{
	friend class FieldArchive;
public:
	explicit FieldArchiveIterator(const FieldArchive &archive);
	bool seek(int mapId);
	Field *next(bool open=true, bool dontOptimize=false);
	Field *peekNext(bool open=true, bool dontOptimize=false) const;
	Field *peekPrevious(bool open=true, bool dontOptimize=false) const;
	Field *previous(bool open=true, bool dontOptimize=false);
	inline int mapId() const {
		return key();
	}
private:
	static Field *openField(Field *field, bool open=true, bool dontOptimize=false);
	QMap<int, Field *> mapList;
};

class FieldArchive
{
	friend class FieldArchiveIterator;
public:
	enum Sorting {
		SortByName, SortByMapId
	};

	enum SearchScope {
		GlobalScope, FieldScope, GrpScriptScope, ScriptScope, TextScope
	};

	enum ExportType {
		Fields, Backgrounds, Akaos, Texts
	};
	Q_DECLARE_FLAGS(ExportTypes, ExportType)

	FieldArchive();
	explicit FieldArchive(FieldArchiveIO *io);
	virtual ~FieldArchive();
	virtual bool isPC() const=0;
	inline bool isPS() const { return !isPC(); }

	FieldArchiveIO::ErrorCode open();
	FieldArchiveIO::ErrorCode save(const QString &path=QString());
	void close();

	virtual void clear();
	inline qsizetype size() const {
		return fileList.size();
	}
	const Field *field(int mapId) const;
	Field *field(int mapId, bool open=true, bool dontOptimize=false);
	const Field *field(const QString &name) const;
	Field *field(const QString &name, bool open=true, bool dontOptimize=false);
	int appendField(Field *field);
	void addNewField(Field *field, int &mapID);
	void delField(int id);

	bool isAllOpened() const;
	bool isModified() const;
	QList<FF7Var> searchAllVars(QMap<FF7Var, QSet<QString> > &fieldNames);
#ifdef DEBUG_FUNCTIONS
	void validateAsk();
	void validateOneLineSize();
	void printAkaos(const QString &filename);
	void printModelLoaders(const QString &filename, bool generic = true);
	void printTexts(const QString &filename, bool usedTexts = false);
	void printTextsDir(const QString &dirname, bool usedTexts = false);
	void compareTexts(FieldArchive *other);
	void printScripts(const QString &filename);
	void printScriptsDirs(const QString &filename);
	void diffScripts();
	bool printBackgroundTiles(bool uniformize = false, bool fromUnusedPCSection = false);
	void printBackgroundZ();
	void searchAll();// research & debug function
#endif
	bool find(bool (*predicate)(Field *, SearchQuery *, SearchIn *),
			  SearchQuery *toSearch, int &mapID, SearchIn *searchIn,
			  Sorting sorting, SearchScope scope);
	bool findLast(bool (*predicate)(Field *, SearchQuery *, SearchIn *),
				  SearchQuery *toSearch, int &mapID, SearchIn *searchIn,
				  Sorting sorting, SearchScope scope);
	bool searchOpcode(int opcode, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchExec(quint8 group, quint8 script, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchMapJump(int map, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchTextInScripts(const QRegularExpression &text, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchText(const QRegularExpression &text, int &mapID, int &textID, qsizetype &from, qsizetype &size, Sorting sorting, SearchScope scope);
	bool searchOpcodeP(int opcode, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchExecP(quint8 group, quint8 script, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchMapJumpP(int map, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchTextInScriptsP(const QRegularExpression &text, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchTextP(const QRegularExpression &text, int &mapID, int &textID, qsizetype &from, qsizetype &index, qsizetype &size, Sorting sorting, SearchScope scope);
	bool replaceText(const QRegularExpression &search, const QString &after, int mapID, int textID, int from);

	bool compileScripts(int &mapID, int &groupID, int &scriptID, int &opcodeID, QString &errorStr);
	void removeBattles();
	void removeTexts();
	void cleanTexts();
	void autosizeTextWindows();

	bool exportation(const QList<int> &selectedFields, const QString &directory,
					 bool overwrite, const QMap<ExportType, QString> &toExport,
	                 PsfTags *tags = nullptr);
	bool importation(const QList<int> &selectedFields, const QString &directory,
					 const QMap<Field::FieldSection, QString> &toImport);

	virtual FieldArchiveIO *io() const;
	inline void setObserver(ArchiveObserver *observer) {
		_observer = observer;
	}

protected:
	virtual void setSaved();
	void setIO(FieldArchiveIO *io);
	inline ArchiveObserver *observer() const {
		return _observer;
	}
private:
	int indexOfField(const QString &name) const;
	void updateFieldLists(Field *field, int fieldID);
	static bool openField(Field *field, bool dontOptimize=false);

	QMap<int, Field *> fileList;
	QMultiMap<QString, int> fieldsSortByName;

	FieldArchiveIO *_io;
	ArchiveObserver *_observer;
	// QFileSystemWatcher fileWatcher;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FieldArchive::ExportTypes)
