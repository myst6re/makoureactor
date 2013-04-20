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

	FieldArchive();
	explicit FieldArchive(FieldArchiveIO *io);
	virtual ~FieldArchive();
	virtual bool isPC() const=0;
	inline bool isPS() { return !isPC(); }

	FieldArchiveIO::ErrorCode open(FieldArchiveIOObserver *observer=0);
	FieldArchiveIO::ErrorCode save(const QString &path=QString(), FieldArchiveIOObserver *observer=0);
	void close();

	virtual void clear();
	int size() const;
	int indexOfField(const QString &name) const;
	Field *field(quint32 id, bool open=true, bool dontOptimize=false);
	void addField(Field *field);
	void removeField(quint32 id);

	bool isAllOpened();
	QList<FF7Var> searchAllVars();
	void searchAll();// research & debug function
	bool searchOpcode(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting);
	bool searchVar(quint8 bank, quint8 adress, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting);
	bool searchExec(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting);
	bool searchMapJump(quint16 _field, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting);
	bool searchTextInScripts(const QRegExp &text, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting);
	bool searchText(const QRegExp &text, int &fieldID, int &textID, int &from, int &size, Sorting sorting);
	bool searchOpcodeP(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting);
	bool searchVarP(quint8 bank, quint8 adress, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting);
	bool searchExecP(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting);
	bool searchMapJumpP(quint16 _field, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting);
	bool searchTextInScriptsP(const QRegExp &text, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting);
	bool searchTextP(const QRegExp &text, int &fieldID, int &textID, int &from, int &index, int &size, Sorting sorting);

	bool exportation(const QList<int> &selectedFields, const QString &directory,
					 bool overwrite, const QMap<Field::FieldSection, QString> &toExport, FieldArchiveIOObserver *observer);
	bool importation(const QList<int> &selectedFields, const QString &directory,
					 const QMap<Field::FieldSection, QString> &toImport,
					 FieldArchiveIOObserver *observer);

	virtual FieldArchiveIO *io() const;
protected:
	virtual void setSaved();
	void setIO(FieldArchiveIO *io);
private:
	bool searchIterators(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &end, int fieldID, Sorting sorting) const;
	bool searchIteratorsP(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &end, int fieldID, Sorting sorting) const;
	bool openField(Field *field, bool dontOptimize=false);

	QList<Field *> fileList;
	QMultiMap<QString, int> fieldsSortByName;
	QMultiMap<QString, int> fieldsSortByMapId;

	FieldArchiveIO *_io;
	// QFileSystemWatcher fileWatcher;
};

#endif
