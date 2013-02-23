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
#include "QLockedFile.h"
#include "Field.h"
#include "IsoArchive.h"
#include "Lgp.h"

class FieldIO : public QIODevice
{
public:
	FieldIO(Field *field, QObject *parent=0);
	virtual void close();
protected:
	virtual qint64 readData(char *data, qint64 maxSize);
private:
	qint64 writeData(const char *, qint64) { return -1; }
	bool setCache();
	Field *_field;
	QByteArray _cache;
};

class FieldArchive : public QObject, IsoControl, LgpObserver
{
	Q_OBJECT

public:
	enum Sorting {
		SortByName, SortByMapId
	};

	enum ErrorCode {
		Ok, FieldNotFound, ErrorOpening, ErrorOpeningTemp, ErrorRemoving, Invalid, NotImplemented
	};

	FieldArchive();
	FieldArchive(const QString &path, bool isDirectory=false);
	virtual ~FieldArchive();

	int size() const;
	Field *field(quint32 id, bool open=true, bool dontOptimize=false);
	QByteArray getFieldData(Field *field, bool unlzs=true);
	QByteArray getMimData(Field *field, bool unlzs=true);
	QByteArray getModelData(Field *field, bool unlzs=true);
	QByteArray getFileData(const QString &fileName, bool unlzs=true);
	TutFile *getTut(const QString &name);
	bool fieldDataIsCached(Field *field) const;
	bool mimDataIsCached(Field *field) const;
	bool modelDataIsCached(Field *field) const;
	void clearCachedData();

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

	void close();
	ErrorCode open();
	ErrorCode save(QString path=QString());

	QString path() const;
	QString chemin() const;
	QString name() const;

	bool isDatFile() const;
	bool isDirectory() const;
	bool isLgp() const;
	bool isIso() const;

	Lgp *lgp() const;

	void setIsoOut(int value) {
		QApplication::processEvents();
		emit progress(value);
	}
	void setObserverValue(int value) {
		QApplication::processEvents();
		emit progress(value);
	}
	void setObserverMaximum(unsigned int max) {
		emit nbFilesChanged(max);
	}
	bool observerWasCanceled() {
		return false;
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
	QByteArray updateFieldBin(const QByteArray &data, IsoDirectory *fieldDirectory);
	
	QList<Field *> fileList;
	QMultiMap<QString, int> fieldsSortByName;
	QMultiMap<QString, int> fieldsSortByMapId;
	QLockedFile *fic;
	Lgp *_lgp;
	QDir *dir;
	IsoArchive *iso;
	IsoDirectory *isoFieldDirectory;
	QMap<QString, TutFile *> tuts;
	bool isDat;
	static QByteArray fieldDataCache, mimDataCache, modelDataCache;
	static Field *fieldCache, *mimCache, *modelCache;
	// QFileSystemWatcher fileWatcher;
};

#endif
