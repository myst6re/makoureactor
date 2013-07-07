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
#ifndef FIELDARCHIVEIOPC_H
#define FIELDARCHIVEIOPC_H

#include "FieldArchiveIO.h"
#include "../QLockedFile.h"
#include "../Lgp.h"

class FieldArchivePC;

class FieldArchiveIOPC : public FieldArchiveIO
{
public:
	FieldArchiveIOPC(FieldArchivePC *fieldArchive);
	inline bool isPS() const { return false; }

	// Removing virtual
	inline void clearCachedData() {
		return FieldArchiveIO::clearCachedData();
	}
protected:
	FieldArchivePC *fieldArchive();
};

class FieldArchiveIOPCLgp : public FieldArchiveIOPC, ArchiveObserver
{
public:
	FieldArchiveIOPCLgp(const QString &path, FieldArchivePC *fieldArchive);
	inline Type type() const { return Lgp; }

	void close();

	QString path() const;

	void *device();

	void setObserverValue(int value) {
		if(observer)	observer->setObserverValue(value);
	}
	void setObserverMaximum(unsigned int max) {
		if(observer)	observer->setObserverMaximum(max);
	}
	bool observerWasCanceled() const {
		return observer && observer->observerWasCanceled();
	}
private:
	QByteArray fieldData2(Field *field, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(FieldArchiveIOObserver *observer);
	ErrorCode save2(const QString &path, FieldArchiveIOObserver *observer);

	::Lgp _lgp;
	FieldArchiveIOObserver *observer;
};

class FieldArchiveIOPCFile : public FieldArchiveIOPC
{
public:
	FieldArchiveIOPCFile(const QString &path, FieldArchivePC *fieldArchive);
	inline Type type() const { return File; }

	void close();

	QString path() const;

	void *device();
private:
	QByteArray fieldData2(Field *field, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(FieldArchiveIOObserver *observer);
	ErrorCode save2(const QString &path, FieldArchiveIOObserver *observer);

	QLockedFile fic;
};

class FieldArchiveIOPCDir : public FieldArchiveIOPC
{
public:
	FieldArchiveIOPCDir(const QString &path, FieldArchivePC *fieldArchive);
	inline Type type() const { return Dir; }

	QString path() const;
	inline bool hasName() const { return false; }

	void *device();
private:
	QByteArray fieldData2(Field *field, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(FieldArchiveIOObserver *observer);
	ErrorCode save2(const QString &path, FieldArchiveIOObserver *observer);

	QDir dir;
};

#endif // FIELDARCHIVEIOPC_H
