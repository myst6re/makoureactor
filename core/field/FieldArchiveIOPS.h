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
#ifndef FIELDARCHIVEIOPS_H
#define FIELDARCHIVEIOPS_H

#include "FieldArchiveIO.h"
#include "../QLockedFile.h"
#include "../IsoArchive.h"

class FieldArchivePS;

class FieldArchiveIOPS : public FieldArchiveIO
{
public:
	FieldArchiveIOPS(FieldArchivePS *fieldArchive);
	inline bool isPS() const { return true; }

	QByteArray mimData(Field *field, bool unlzs=true);
	QByteArray modelData(Field *field, bool unlzs=true);

	bool mimDataIsCached(Field *field) const;
	bool modelDataIsCached(Field *field) const;
	void clearCachedData();
protected:
	virtual QByteArray mimData2(Field *field, bool unlzs)=0;
	virtual QByteArray modelData2(Field *field, bool unlzs)=0;

	FieldArchivePS *fieldArchive();
private:
	static QByteArray mimDataCache, modelDataCache;
	static Field *mimCache, *modelCache;
};

class FieldArchiveIOPSFile : public FieldArchiveIOPS
{
public:
	FieldArchiveIOPSFile(const QString &path, FieldArchivePS *fieldArchive);
	inline Type type() const { return File; }

	void close();

	QString path() const;

	Archive *device();
private:
	QByteArray fieldData2(Field *field, bool unlzs);
	QByteArray mimData2(Field *field, bool unlzs);
	QByteArray modelData2(Field *field, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(ArchiveObserver *observer);
	ErrorCode save2(const QString &path, ArchiveObserver *observer);

	QLockedFile fic;
};

class FieldArchiveIOPSIso : public FieldArchiveIOPS, IsoControl
{
public:
	FieldArchiveIOPSIso(const QString &path, FieldArchivePS *fieldArchive);
	inline Type type() const { return Iso; }

	QString path() const;

	Archive *device();

	void setIsoOut(int value) {
		if(observer)	observer->setObserverValue(value);
	}
	bool wasCanceled() {
		return observer && observer->observerWasCanceled();
	}
private:
	QByteArray fieldData2(Field *field, bool unlzs);
	QByteArray mimData2(Field *field, bool unlzs);
	QByteArray modelData2(Field *field, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(ArchiveObserver *observer);
	ErrorCode save2(const QString &path, ArchiveObserver *observer);

	static QByteArray updateFieldBin(const QByteArray &data, IsoDirectory *fieldDirectory);

	IsoArchive iso;
	IsoDirectory *isoFieldDirectory;
	ArchiveObserver *observer;
};

class FieldArchiveIOPSDir : public FieldArchiveIOPS
{
public:
	FieldArchiveIOPSDir(const QString &path, FieldArchivePS *fieldArchive);
	inline Type type() const { return Dir; }

	QString path() const;
	inline bool hasName() const { return false; }

	Archive *device();
private:
	QByteArray fieldData2(Field *field, bool unlzs);
	QByteArray mimData2(Field *field, bool unlzs);
	QByteArray modelData2(Field *field, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(ArchiveObserver *observer);
	ErrorCode save2(const QString &path, ArchiveObserver *observer);

	QDir dir;
};

#endif // FIELDARCHIVEIOPS_H
