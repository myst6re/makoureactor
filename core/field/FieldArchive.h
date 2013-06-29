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

#include <QtCore>
#include "FieldArchiveIO.h"
#include "Field.h"

class FieldArchive
{
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
	inline bool isPS() { return !isPC(); }

	FieldArchiveIO::ErrorCode open();
	FieldArchiveIO::ErrorCode save(const QString &path=QString());
	void close();

	virtual void clear();
	inline int size() const {
		return fileList.size();
	}
	int indexOfField(const QString &name) const;
	const Field *field(quint32 id) const;
	Field *field(quint32 id, bool open=true, bool dontOptimize=false);
	void appendField(Field *field);
	void addField(Field *field);
	void removeField(quint32 id);

	bool isAllOpened() const;
	bool isModified() const;
	QList<FF7Var> searchAllVars();
	void searchAll();// research & debug function
	bool searchOpcode(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchVar(quint8 bank, quint8 adress, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchExec(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchMapJump(quint16 _field, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchTextInScripts(const QRegExp &text, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchText(const QRegExp &text, int &fieldID, int &textID, int &from, int &size, Sorting sorting, SearchScope scope);
	bool searchOpcodeP(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
	bool searchVarP(quint8 bank, quint8 adress, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope);
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
	inline void setObserver(FieldArchiveIOObserver *observer) {
		_observer = observer;
	}

protected:
	virtual void setSaved();
	void setIO(FieldArchiveIO *io);
	inline FieldArchiveIOObserver *observer() const {
		return _observer;
	}
private:
	void updateFieldLists(Field *field, int fieldID);
	bool searchIterators(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &end, int fieldID, Sorting sorting, SearchScope scope) const;
	bool searchIteratorsP(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &end, int fieldID, Sorting sorting, SearchScope scope) const;
	bool openField(Field *field, bool dontOptimize=false);

	QList<Field *> fileList;
	QMultiMap<QString, int> fieldsSortByName;
	QMultiMap<QString, int> fieldsSortByMapId;

	FieldArchiveIO *_io;
	FieldArchiveIOObserver *_observer;
	// QFileSystemWatcher fileWatcher;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FieldArchive::ExportTypes)

#endif
