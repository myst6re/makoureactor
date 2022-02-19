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

#include <QLockedFile.h>
#include <Lgp.h>
#include "FieldArchiveIO.h"

class FieldArchivePC;

class FieldArchiveIOPC : public FieldArchiveIO
{
public:
	explicit FieldArchiveIOPC(FieldArchivePC *fieldArchive);
	inline bool isPS() const override { return false; }

	// Removing virtual
	inline void clearCachedData() override {
		return FieldArchiveIO::clearCachedData();
	}
protected:
	FieldArchivePC *fieldArchive();
};

class FieldArchiveIOPCLgp : public FieldArchiveIOPC, ArchiveObserver
{
public:
	FieldArchiveIOPCLgp(const QString &path, FieldArchivePC *fieldArchive);
	inline Type type() const override { return Lgp; }

	void close() override;
	ErrorCode addField(const QString &fileName, const QString &name) override;

	QString path() const override;

	Archive *device() override;

	void setObserverValue(int value) override {
		if(observer)	observer->setObserverValue(value);
	}
	void setObserverMaximum(unsigned int max) override {
		if(observer)	observer->setObserverMaximum(max);
	}
	bool observerWasCanceled() const override {
		return observer && observer->observerWasCanceled();
	}
	bool observerRetry(const QString &message) override {
		return observer && observer->observerRetry(message);
	}
private:
	QByteArray fieldData2(Field *field, const QString &extension, bool unlzs) override;
	QByteArray fileData2(const QString &fileName) override;

	ErrorCode open2(ArchiveObserver *observer) override;
	ErrorCode save2(const QString &path, ArchiveObserver *observer) override;

	::Lgp _lgp;
	ArchiveObserver *observer;
};

class FieldArchiveIOPCFile : public FieldArchiveIOPC
{
public:
	FieldArchiveIOPCFile(const QString &path, FieldArchivePC *fieldArchive);
	inline Type type() const override { return File; }

	void close() override;

	QString path() const override;

	Archive *device() override;
private:
	QByteArray fieldData2(Field *field, const QString &extension, bool unlzs) override;
	QByteArray fileData2(const QString &fileName) override;

	ErrorCode open2(ArchiveObserver *observer) override;
	ErrorCode save2(const QString &path, ArchiveObserver *observer) override;

	QLockedFile fic;
};

class FieldArchiveIOPCDir : public FieldArchiveIOPC
{
public:
	FieldArchiveIOPCDir(const QString &path, FieldArchivePC *fieldArchive);
	inline Type type() const override { return Dir; }

	QString path() const override;
	inline bool hasName() const override { return false; }

	Archive *device() override;
private:
	QByteArray fieldData2(Field *field, const QString &extension, bool unlzs) override;
	QByteArray fileData2(const QString &fileName) override;

	ErrorCode open2(ArchiveObserver *observer) override;
	ErrorCode save2(const QString &path, ArchiveObserver *observer) override;

	QDir dir;
};
