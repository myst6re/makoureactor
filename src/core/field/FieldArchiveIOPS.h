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

#include <QLockedFile>
#include "FieldArchiveIO.h"
#include <IsoArchiveFF7>

class FieldArchivePS;

class FieldArchiveIOPS : public FieldArchiveIO
{
public:
	explicit FieldArchiveIOPS(FieldArchivePS *fieldArchive);
	inline bool isPS() const override { return true; }

	QByteArray mimData(Field *field, bool unlzs=true);
	QByteArray modelData(Field *field, bool unlzs=true);

	static bool mimDataIsCached(Field *field);
	static bool modelDataIsCached(Field *field);
	void clearCachedData() override;
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
	inline Type type() const override { return File; }

	void close() override;

	QString path() const override;

	Archive *device() override;
private:
	QByteArray fieldData2(Field *field, const QString &extension, bool unlzs) override;
	QByteArray mimData2(Field *field, bool unlzs) override;
	QByteArray modelData2(Field *field, bool unlzs) override;
	QByteArray fileData2(const QString &fileName) override;

	ErrorCode open2(ArchiveObserver *observer) override;
	ErrorCode save2(const QString &path, ArchiveObserver *observer) override;

	QLockedFile fic;
};

class FieldArchiveIOPSIso : public FieldArchiveIOPS
{
public:
	FieldArchiveIOPSIso(const QString &path, FieldArchivePS *fieldArchive);
	inline Type type() const override { return Iso; }

	QString path() const override;

	Archive *device() override;
private:
	QByteArray fieldData2(Field *field, const QString &extension, bool unlzs) override;
	QByteArray mimData2(Field *field, bool unlzs) override;
	QByteArray modelData2(Field *field, bool unlzs) override;
	QByteArray fileData2(const QString &fileName) override;

	ErrorCode openIso();
	ErrorCode open2(ArchiveObserver *observer) override;
	ErrorCode save2(const QString &path, ArchiveObserver *observer) override;

	IsoArchiveFF7 iso;
	IsoDirectory *isoFieldDirectory;
};

class FieldArchiveIOPSDir : public FieldArchiveIOPS
{
public:
	FieldArchiveIOPSDir(const QString &path, FieldArchivePS *fieldArchive);
	inline Type type() const override { return Dir; }

	QString path() const override;
	inline bool hasName() const override { return false; }

	Archive *device() override;
private:
	QByteArray fieldData2(Field *field, const QString &extension, bool unlzs) override;
	QByteArray mimData2(Field *field, bool unlzs) override;
	QByteArray modelData2(Field *field, bool unlzs) override;
	QByteArray fileData2(const QString &fileName) override;

	ErrorCode open2(ArchiveObserver *observer) override;
	ErrorCode save2(const QString &path, ArchiveObserver *observer) override;

	QDir dir;
};
