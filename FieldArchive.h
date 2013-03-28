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

class FieldArchive : public QObject, FieldArchiveIOObserver
{
	Q_OBJECT

public:
	enum Sorting {
		SortByName, SortByMapId
	};

	FieldArchive();
	FieldArchive(const QString &path, bool isDirectory=false);
	virtual ~FieldArchive();

	FieldArchiveIO::ErrorCode open();
	FieldArchiveIO::ErrorCode save(const QString &path=QString());
	void close();

	int size() const;
	Field *field(quint32 id, bool open=true, bool dontOptimize=false);
	TutFile *tut(const QString &name);
	const QMap<QString, TutFile *> &tuts() const;
	void addField(Field *field);
	void addTut(const QString &name);

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

	FieldArchiveIO *io() const;

	bool observerWasCanceled() {
		return false;
	}
	void setObserverMaximum(unsigned int max) {
		emit nbFilesChanged(max);
	}
	void setObserverValue(int value) {
		QApplication::processEvents();
		emit progress(value);
	}
signals:
	void progress(int);
	void nbFilesChanged(int);
//	void fileChanged(const QString &path);
//	void directoryChanged(const QString &path);
	
private:
	bool searchIterators(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &end, int fieldID, Sorting sorting) const;
	bool searchIteratorsP(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &end, int fieldID, Sorting sorting) const;
	bool openField(Field *field, bool dontOptimize=false);
	void setSaved();

	QList<Field *> fileList;
	QMultiMap<QString, int> fieldsSortByName;
	QMultiMap<QString, int> fieldsSortByMapId;
	QMap<QString, TutFile *> _tuts;

	FieldArchiveIO *_io;
	// QFileSystemWatcher fileWatcher;
};

#endif
