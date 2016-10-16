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
#ifndef DEF_FIELDARCHIVE
#define DEF_FIELDARCHIVE

//#define DEBUG_FUNCTIONS

#include <QtCore>
#include "FieldArchiveIO.h"
#include "Field.h"

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
	quint16 fieldID;
	explicit SearchFieldQuery(quint16 fieldID) :
		fieldID(fieldID) {}
};

struct SearchTextQuery : public SearchQuery
{
	QRegExp text;
	explicit SearchTextQuery(QRegExp text) :
		text(text) {}
};

struct SearchIn
{
	virtual void reset()=0;
	virtual void toEnd()=0;
};

struct SearchInScript : public SearchIn
{
	int &groupID, &scriptID, &opcodeID;

	SearchInScript(int &groupID, int &scriptID, int &opcodeID) :
		groupID(groupID), scriptID(scriptID), opcodeID(opcodeID)
	{}

	void reset() {
		groupID = scriptID = opcodeID = 0;
	}

	void toEnd() {
		groupID = scriptID = opcodeID = 2147483647;
	}
};

struct SearchInText : public SearchIn
{
	int &textID, &from, &size, &index;

	SearchInText(int &textID, int &from, int &size, int &index) :
		textID(textID), from(from), size(size), index(index)
	{}

	void reset() {
		textID = 0;
		from = -1;
	}

	void toEnd() {
		textID = 2147483647;
		from = -1;
	}
};

class FieldArchive;

class FieldArchiveIterator : public QListIterator<Field *>
{
	friend class FieldArchive;
public:
	explicit FieldArchiveIterator(const FieldArchive &archive);
	Field *next(bool open=true);
	Field *peekNext(bool open=true) const;
	Field *peekPrevious(bool open=true) const;
	Field *previous(bool open=true);
private:
	static Field *openField(Field *field, bool open=true);
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
	inline int size() const {
		return fileList.size();
	}
	int indexOfField(const QString &name) const;
	const Field *field(quint32 id) const;
	Field *field(quint32 id, bool open=true);
	const Field *field(const QString &name) const;
	Field *field(const QString &name, bool open=true);
	void appendField(Field *field);
	FieldArchiveIO::ErrorCode addField(Field *field, const QString &fileName, int &fieldId);
	void removeField(quint32 id);

	bool isAllOpened() const;
	bool isModified() const;
	QList<FF7Var> searchAllVars(QMap<FF7Var, QSet<QString> > &fieldNames);
#ifdef DEBUG_FUNCTIONS
	void validateAsk();
	void validateOneLineSize();
	void printAkaos(const QString &filename);
	void printModelLoaders(const QString &filename, bool generic = true);
	void printTexts(const QString &filename);
	void compareTexts(FieldArchive *other);
	void printScripts(const QString &filename);
	void printScriptsDirs(const QString &filename);
	void diffScripts();
	static bool printBackgroundTiles(Field *field, const QString &filename, bool uniformize = false);
	void searchBackgroundZ();
	void searchAll();// research & debug function
#endif
	bool find(bool (*predicate)(Field *, SearchQuery *, SearchIn *),
			  SearchQuery *toSearch, int &fieldID, SearchIn *searchIn,
			  Sorting sorting, SearchScope scope);
	bool findLast(bool (*predicate)(Field *, SearchQuery *, SearchIn *),
				  SearchQuery *toSearch, int &fieldID, SearchIn *searchIn,
				  Sorting sorting, SearchScope scope);
	bool searchOpcode(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchExec(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchMapJump(quint16 _field, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchTextInScripts(const QRegExp &text, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchText(const QRegExp &text, int &fieldID, int &textID, int &from, int &size, Sorting sorting, SearchScope scope);
	bool searchOpcodeP(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchExecP(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchMapJumpP(quint16 _field, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchTextInScriptsP(const QRegExp &text, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchTextP(const QRegExp &text, int &fieldID, int &textID, int &from, int &index, int &size, Sorting sorting, SearchScope scope);
	bool replaceText(const QRegExp &search, const QString &after, int fieldID, int textID, int from);

	bool compileScripts(int &fieldID, int &groupID, int &scriptID, int &opcodeID, QString &errorStr);
	void removeBattles();
	void removeTexts();
	void cleanTexts();

	bool exportation(const QList<int> &selectedFields, const QString &directory,
					 bool overwrite, const QMap<ExportType, QString> &toExport);
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
	void updateFieldLists(Field *field, int fieldID);
	bool searchIterators(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &end, int fieldID, Sorting sorting, SearchScope scope) const;
	bool searchIteratorsP(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &end, int fieldID, Sorting sorting, SearchScope scope) const;
	static bool openField(Field *field);

	QList<Field *> fileList;
	QMultiMap<QString, int> fieldsSortByName;
	QMultiMap<QString, int> fieldsSortByMapId;

	FieldArchiveIO *_io;
	ArchiveObserver *_observer;
	// QFileSystemWatcher fileWatcher;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FieldArchive::ExportTypes)

#endif
