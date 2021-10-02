/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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

#include <QLockedFile.h>
#include "FieldArchiveIO.h"
#include "../Lgp.h"

class FieldArchivePC;

class FieldArchiveIOPC : public FieldArchiveIO
{
public:
	explicit FieldArchiveIOPC(FieldArchivePC *fieldArchive);
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
	ErrorCode addField(const QString &fileName, const QString &name);

	QString path() const;

	Archive *device();

	void setObserverValue(int value) {
		if(observer)	observer->setObserverValue(value);
	}
	void setObserverMaximum(unsigned int max) {
		if(observer)	observer->setObserverMaximum(max);
	}
	bool observerWasCanceled() const {
		return observer && observer->observerWasCanceled();
	}
	bool observerRetry(const QString &message) {
		return observer && observer->observerRetry(message);
	}
private:
	QByteArray fieldData2(Field *field, const QString &extension, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(ArchiveObserver *observer);
	ErrorCode save2(const QString &path, ArchiveObserver *observer);

	::Lgp _lgp;
	ArchiveObserver *observer;
};

class FieldArchiveIOPCFile : public FieldArchiveIOPC
{
public:
	FieldArchiveIOPCFile(const QString &path, FieldArchivePC *fieldArchive);
	inline Type type() const { return File; }

	void close();

	QString path() const;

	Archive *device();
private:
	QByteArray fieldData2(Field *field, const QString &extension, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(ArchiveObserver *observer);
	ErrorCode save2(const QString &path, ArchiveObserver *observer);

	QLockedFile fic;
};

class FieldArchiveIOPCDir : public FieldArchiveIOPC
{
public:
	FieldArchiveIOPCDir(const QString &path, FieldArchivePC *fieldArchive);
	inline Type type() const { return Dir; }

	QString path() const;
	inline bool hasName() const { return false; }

	Archive *device();
private:
	QByteArray fieldData2(Field *field, const QString &extension, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(ArchiveObserver *observer);
	ErrorCode save2(const QString &path, ArchiveObserver *observer);

	QDir dir;
};
