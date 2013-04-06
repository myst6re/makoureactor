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
#ifndef FIELDARCHIVEIO_H
#define FIELDARCHIVEIO_H

#include <QtCore>

class FieldArchive;
class Field;

struct FieldArchiveIOObserver
{
	FieldArchiveIOObserver() {}
	virtual bool observerWasCanceled() const=0;
	virtual void setObserverMaximum(unsigned int max)=0;
	virtual void setObserverValue(int value)=0;
};

class FieldArchiveIO
{
public:
	enum ErrorCode {
		Ok, FieldNotFound, ErrorOpening, ErrorOpeningTemp, ErrorRemoving, ErrorRenaming, ErrorCopying, Invalid, NotImplemented
	};
	enum Type {
		Lgp, File, Iso, Dir
	};

	FieldArchiveIO(FieldArchive *fieldArchive);
	virtual ~FieldArchiveIO();
	virtual bool isPS() const=0;
	inline bool isPC() const { return !isPS(); }

	QByteArray fieldData(Field *field, bool unlzs=true);
	QByteArray fileData(const QString &fileName, bool unlzs=true, bool isLzsFile=true);

	bool fieldDataIsCached(Field *field) const;
	virtual void clearCachedData();

	virtual void close();
	ErrorCode open(FieldArchiveIOObserver *observer=0);
	ErrorCode save(const QString &path=QString(), FieldArchiveIOObserver *observer=0);

	virtual QString path() const=0;
	QString directory() const;
	QString name() const;
	virtual inline bool hasName() const { return true; }

	virtual Type type() const=0;

	virtual void *device()=0;
protected:
	virtual QByteArray fieldData2(Field *field, bool unlzs)=0;
	virtual QByteArray fileData2(const QString &fileName)=0;

	virtual ErrorCode open2(FieldArchiveIOObserver *observer)=0;
	virtual ErrorCode save2(const QString &path, FieldArchiveIOObserver *observer)=0;
	FieldArchive *fieldArchive();
private:
	FieldArchive *_fieldArchive;
	static QByteArray fieldDataCache, mimDataCache, modelDataCache;
	static Field *fieldCache, *mimCache, *modelCache;
};

#endif // FIELDARCHIVEIO_H
