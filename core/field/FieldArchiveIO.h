#ifndef FIELDARCHIVEIO_H
#define FIELDARCHIVEIO_H

#include <QtCore>
#include "QLockedFile.h"
#include "IsoArchive.h"
#include "Lgp.h"

class FieldArchive;
class FieldArchivePS;
class FieldArchivePC;
class Field;

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

	QByteArray fieldData(Field *field, bool unlzs=true);
	QByteArray fileData(const QString &fileName, bool unlzs=true);

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

class FieldArchiveIOPC : public FieldArchiveIO
{
public:
	FieldArchiveIOPC(FieldArchive *fieldArchive) :
		FieldArchiveIO(fieldArchive) {}
	// Removing virtual
	inline void clearCachedData() {
		return FieldArchiveIO::clearCachedData();
	}
protected:
	FieldArchivePC *fieldArchive() {
		return (FieldArchivePC *)FieldArchiveIO::fieldArchive();
	}
};

class FieldArchiveIOPS : public FieldArchiveIO
{
public:
	FieldArchiveIOPS(FieldArchive *fieldArchive) :
		FieldArchiveIO(fieldArchive) {}

	QByteArray mimData(Field *field, bool unlzs=true);
	QByteArray modelData(Field *field, bool unlzs=true);

	bool mimDataIsCached(Field *field) const;
	bool modelDataIsCached(Field *field) const;
	void clearCachedData();
protected:
	virtual QByteArray mimData2(Field *field, bool unlzs)=0;
	virtual QByteArray modelData2(Field *field, bool unlzs)=0;

	FieldArchivePS *fieldArchive() {
		return (FieldArchivePS *)FieldArchiveIO::fieldArchive();
	}
private:
	static QByteArray mimDataCache, modelDataCache;
	static Field *mimCache, *modelCache;
};

class FieldArchiveIOLgp : public FieldArchiveIOPC, LgpObserver
{
public:
	FieldArchiveIOLgp(const QString &path, FieldArchive *fieldArchive);
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

class FieldArchiveIOFile : public FieldArchiveIOPS
{
public:
	FieldArchiveIOFile(const QString &path, FieldArchive *fieldArchive);
	inline Type type() const { return File; }

	void close();
	ErrorCode save(const QString &path=QString());

	QString path() const;

	void *device();
private:
	QByteArray fieldData2(Field *field, bool unlzs);
	QByteArray mimData2(Field *field, bool unlzs);
	QByteArray modelData2(Field *field, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(FieldArchiveIOObserver *observer);
	ErrorCode save2(const QString &path, FieldArchiveIOObserver *observer);

	QLockedFile fic;
};

class FieldArchiveIOIso : public FieldArchiveIOPS, IsoControl
{
public:
	FieldArchiveIOIso(const QString &path, FieldArchive *fieldArchive);
	inline Type type() const { return Iso; }

	ErrorCode open(QList<Field *> &fields);
	ErrorCode save(const QString &path=QString());

	QString path() const;

	void *device();

	void setIsoOut(int value) {
		if(observer)	observer->setObserverValue(value);
	}
private:
	QByteArray fieldData2(Field *field, bool unlzs);
	QByteArray mimData2(Field *field, bool unlzs);
	QByteArray modelData2(Field *field, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(FieldArchiveIOObserver *observer);
	ErrorCode save2(const QString &path, FieldArchiveIOObserver *observer);

	static QByteArray updateFieldBin(const QByteArray &data, IsoDirectory *fieldDirectory);

	IsoArchive iso;
	IsoDirectory *isoFieldDirectory;
	FieldArchiveIOObserver *observer;
};

class FieldArchiveIODir : public FieldArchiveIOPS
{
public:
	FieldArchiveIODir(const QString &path, FieldArchive *fieldArchive);
	inline Type type() const { return Dir; }

	ErrorCode open(QList<Field *> &fields);
	ErrorCode save(const QString &path=QString());

	QString path() const;
	inline bool hasName() const { return false; }

	void *device();
private:
	QByteArray fieldData2(Field *field, bool unlzs);
	QByteArray mimData2(Field *field, bool unlzs);
	QByteArray modelData2(Field *field, bool unlzs);
	QByteArray fileData2(const QString &fileName);

	ErrorCode open2(FieldArchiveIOObserver *observer);
	ErrorCode save2(const QString &path, FieldArchiveIOObserver *observer);

	QDir dir;
};

#endif // FIELDARCHIVEIO_H
