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
#include "FieldArchiveIOPS.h"
#include "FieldPS.h"
#include "FieldPSDemo.h"
#include "FieldArchivePS.h"
#include "../GZIP.h"
#include "Data.h"

QByteArray FieldArchiveIOPS::mimDataCache;
QByteArray FieldArchiveIOPS::modelDataCache;
Field *FieldArchiveIOPS::mimCache=0;
Field *FieldArchiveIOPS::modelCache=0;

FieldArchiveIOPS::FieldArchiveIOPS(FieldArchivePS *fieldArchive) :
	FieldArchiveIO(fieldArchive)
{
}

QByteArray FieldArchiveIOPS::mimData(Field *field, bool unlzs)
{
	// use data from the cache
	if (unlzs && mimDataIsCached(field)) {
		return mimDataCache;
	}

	QByteArray data = mimData2(field, unlzs);

	// put decompressed data in the cache
	if (unlzs && !data.isEmpty()) {
		mimCache = field;
		mimDataCache = data;
	}
	return data;
}

QByteArray FieldArchiveIOPS::modelData(Field *field, bool unlzs)
{
	// use data from the cache
	if (unlzs && modelDataIsCached(field)) {
		return modelDataCache;
	}

	QByteArray data = modelData2(field, unlzs);

	// put decompressed data in the cache
	if (unlzs && !data.isEmpty()) {
		modelCache = field;
		modelDataCache = data;
	}
	return data;
}

bool FieldArchiveIOPS::mimDataIsCached(Field *field)
{
	return mimCache && mimCache == field;
}

bool FieldArchiveIOPS::modelDataIsCached(Field *field)
{
	return modelCache && modelCache == field;
}

void FieldArchiveIOPS::clearCachedData()
{
	mimCache = 0;
	modelCache = 0;
	mimDataCache.clear();
	modelDataCache.clear();
	FieldArchiveIO::clearCachedData();
}

FieldArchivePS *FieldArchiveIOPS::fieldArchive()
{
	return static_cast<FieldArchivePS *>(FieldArchiveIO::fieldArchive());
}

FieldArchiveIOPSFile::FieldArchiveIOPSFile(const QString &path, FieldArchivePS *fieldArchive) :
	FieldArchiveIOPS(fieldArchive), fic(path)
{
}

QString FieldArchiveIOPSFile::path() const
{
	return fic.fileName();
}

Archive *FieldArchiveIOPSFile::device()
{
	return nullptr;
}

QByteArray FieldArchiveIOPSFile::fieldData2(Field *field, const QString &extension, bool unlzs)
{
	Q_UNUSED(field)
	QString path;

	if (extension != "DAT") {
		path = fic.fileName().left(fic.fileName().lastIndexOf('.')) + "." + extension;
	}

	return fileData(path, unlzs);
}

QByteArray FieldArchiveIOPSFile::mimData2(Field *field, bool unlzs)
{
	Q_UNUSED(field)
	return fileData(fic.fileName().left(fic.fileName().lastIndexOf('.')) + ".MIM", unlzs);
}

QByteArray FieldArchiveIOPSFile::modelData2(Field *field, bool unlzs)
{
	Q_UNUSED(field)
	return fileData(fic.fileName().left(fic.fileName().lastIndexOf('.')) + ".BSX", unlzs);
}

QByteArray FieldArchiveIOPSFile::fileData2(const QString &fileName)
{
	if (!fileName.isEmpty() && fileName != fic.fileName()) {
		QFile f(fileName);
		if (!f.open(QIODevice::ReadOnly)) {
			return QByteArray();
		}
		QByteArray d = f.readAll();
		f.close();

		return d;
	}

	if (!fic.isOpen() && !fic.open(QIODevice::ReadOnly))		return QByteArray();
	fic.reset();
	QByteArray data = fic.readAll();
	fic.close();

	return data;
}

void FieldArchiveIOPSFile::close()
{
	fic.close();
	FieldArchiveIO::close();
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSFile::open2(ArchiveObserver *observer)
{
	Q_UNUSED(observer)

	QString name = this->name();
	fieldArchive()->appendField(new FieldPS(name.left(name.lastIndexOf('.')), this));

	// Open field
	FieldArchiveIterator it(*(fieldArchive()));
	Field *field = it.next();

	return field && field->isOpen() ? Ok : Invalid;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSFile::save2(const QString &path0, ArchiveObserver *observer)
{
	Q_UNUSED(observer)
	QString path = path0.isNull() ? fic.fileName() : path0;

	FieldArchiveIterator it(*(fieldArchive()));

	if (!it.hasNext()) {
		return FieldNotFound;
	}

	Field *field = it.next(false);
	if (field && field->isOpen() && field->isModified()) {
		qint8 err = field->save(path, true);
		if (err == 2)	return ErrorOpening;
		if (err == 1)	return Invalid;
		if (err != 0)	return NotImplemented;
	}

	return Ok;
}

FieldArchiveIOPSIso::FieldArchiveIOPSIso(const QString &path, FieldArchivePS *fieldArchive) :
	FieldArchiveIOPS(fieldArchive), iso(path), isoFieldDirectory(0)
{
}

QString FieldArchiveIOPSIso::path() const
{
	return iso.fileName();
}

Archive *FieldArchiveIOPSIso::device()
{
	return nullptr;
}

QByteArray FieldArchiveIOPSIso::fieldData2(Field *field, const QString &extension, bool unlzs)
{
	if (extension.isEmpty()) {
		return QByteArray();
	}
	return fileData(QString("%1.%2").arg(field->name().toUpper(), extension.toUpper()), unlzs);
}

QByteArray FieldArchiveIOPSIso::mimData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".MIM", unlzs);
}

QByteArray FieldArchiveIOPSIso::modelData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".BSX", unlzs);
}

QByteArray FieldArchiveIOPSIso::fileData2(const QString &fileName)
{
	if (isoFieldDirectory) {
		IsoFile *file = isoFieldDirectory->file(fileName.toUpper());
		if (file) {
			return file->data();
		}
	}
	return QByteArray();
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSIso::openIso()
{
	if (!iso.isOpen() && !iso.open(QIODevice::ReadOnly)) {
		return ErrorOpening;
	}

	isoFieldDirectory = iso.fieldDirectory();
	if (isoFieldDirectory == nullptr) {
		return FieldNotFound;
	}

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSIso::open2(ArchiveObserver *observer)
{
	ErrorCode error = openIso();
	if (error != Ok) {
		return error;
	}


	iso.maplist();

	QList<IsoFile *> files = isoFieldDirectory->files();

	if (observer)	observer->setObserverMaximum(files.size());
	bool isDemo = iso.isDemo();

	// QTime t;t.start();

	int i=0;
	for (IsoFile *file : qAsConst(files)) {
		if (observer) {
			if (observer->observerWasCanceled()) {
				return Aborted;
			}
			observer->setObserverValue(i);
		}

		if (isDemo) {
			if (file->name().endsWith(".ATE")) {
				QString name = file->name().mid(file->name().lastIndexOf('/')+1);
				fieldArchive()->appendField(new FieldPSDemo(name.left(name.lastIndexOf('.')), this));
			}
		} else if (file->name().endsWith(".DAT") && !file->name().startsWith("WM")) {
			QString name = file->name().mid(file->name().lastIndexOf('/')+1);
			fieldArchive()->appendField(new FieldPS(name.left(name.lastIndexOf('.')), this));
		}
	}
	// qDebug("Ouverture : %d ms", t.elapsed());

	Data::windowBin = WindowBinFile();

	if (!Data::windowBin.open(iso.windowBinData())) {
		qWarning() << "Cannot open window.bin";
	}

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSIso::save2(const QString &path0, ArchiveObserver *observer)
{
	if (iso.isDemo()) {
		return NotImplemented;
	}

	QString path = path0.isNull() ? iso.fileName() : path0;

	bool saveAs = QFileInfo(path) != QFileInfo(iso.io());

	if (observer)	observer->setObserverMaximum(100);

	// FIELD/*.DAT

	FieldArchiveIterator it(*(fieldArchive()));
	while (it.hasNext()) {
		if (observer && observer->observerWasCanceled()) {
			return Aborted;
		}
		Field *field = it.next(false);
		if (field && field->isOpen() && field->isModified()) {
			IsoFile *isoField = isoFieldDirectory->file(field->name().toUpper() + ".DAT");
			if (isoField == nullptr) {
				continue;
			}

			QByteArray newData;

			if (field->save(newData, true)) {
				isoField->setModifiedFile(newData);
			} else {
				return Invalid;
			}
		}
	}

	IsoArchive isoTemp(path % ".makoutemp");
	if (!isoTemp.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
		return ErrorOpening;
	}

	if (!iso.pack(&isoTemp, observer, isoFieldDirectory)) {
		if (observer && observer->observerWasCanceled()) {
			return Aborted;
		}
		return Invalid;
	}

	// End

	iso.close();
	isoTemp.close();

	if (saveAs) {
		iso.setFileName(path);
	}
	// Move the temporary file
	if (QFile::exists(path) && !QFile::remove(path)) {
		bool removed = false;

		while (observer->observerRetry(QObject::tr("Cannot remove destination archive"))) {
			if (QFile::remove(path)) {
				removed = true;
				break;
			}
		}

		if (!removed) {
			QFile::remove(isoTemp.fileName());
			openIso(); // Reopen
			return ErrorRemoving;
		}
	}
	if (!QFile::rename(isoTemp.fileName(), path)) {
		bool renamed = false;

		while (observer->observerRetry(QObject::tr("Cannot rename temporary file to destination path"))) {
			if (QFile::rename(isoTemp.fileName(), path)) {
				renamed = true;
				break;
			}
		}

		if (!renamed) {
			return ErrorRenaming;
		}
	}

	ErrorCode error = openIso();
	if (error != Ok) {
		return error;
	}

	return Ok;
}

FieldArchiveIOPSDir::FieldArchiveIOPSDir(const QString &path, FieldArchivePS *fieldArchive) :
	FieldArchiveIOPS(fieldArchive), dir(path)
{
}

QString FieldArchiveIOPSDir::path() const
{
	return dir.path();
}

Archive *FieldArchiveIOPSDir::device()
{
	return nullptr;
}

QByteArray FieldArchiveIOPSDir::fieldData2(Field *field, const QString &extension, bool unlzs)
{
	if (extension.isEmpty()) {
		return QByteArray();
	}
	return fileData(QString("%1.%2").arg(field->name().toUpper(), extension.toUpper()), unlzs);
}

QByteArray FieldArchiveIOPSDir::mimData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".MIM", unlzs);
}

QByteArray FieldArchiveIOPSDir::modelData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".BSX", unlzs);
}

QByteArray FieldArchiveIOPSDir::fileData2(const QString &fileName)
{
	QByteArray data;

	QFile f(dir.filePath(fileName.toUpper()));
	if (!f.open(QIODevice::ReadOnly))	return QByteArray();
	data = f.readAll();
	f.close();

	return data;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSDir::open2(ArchiveObserver *observer)
{
	QStringList list;
	list.append("*.DAT");
	list = dir.entryList(list, QDir::Files | QDir::NoSymLinks);

	if (observer)	observer->setObserverMaximum(list.size());

	// QTime t;t.start();

	int i=0;
	for (const QString &name : qAsConst(list)) {
		if (observer) {
			if (observer->observerWasCanceled()) {
				return Aborted;
			}
			observer->setObserverValue(i++);
		}

		if (!name.startsWith("WM", Qt::CaseInsensitive)) {
			fieldArchive()->appendField(new FieldPS(name.left(name.lastIndexOf('.')), this));
		}
	}

	// qDebug("Opening: %d ms", t.elapsed());

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSDir::save2(const QString &path, ArchiveObserver *observer)
{
	bool saveAs;

	if (!path.isEmpty()) {
		saveAs = QDir::cleanPath(path) != QDir::cleanPath(dir.path());
	} else {
		saveAs = false;
	}

	FieldArchiveIterator it(*(fieldArchive()));
	quint32 i = 0;

	if (observer) {
		observer->setObserverMaximum(fieldArchive()->size());
	}

	while (it.hasNext()) {
		if (observer && observer->observerWasCanceled()) {
			return Aborted;
		}
		Field *field = it.next(false);
		QString datName = field->name().toUpper() + ".DAT",
		        datPath = dir.filePath(datName);
		if (field) {
			if (field->isOpen() && field->isModified()) {
				qint8 err = field->save(datPath, true);
				if (err == 2)	return ErrorOpening;
				if (err == 1)	return Invalid;
				if (err != 0)	return NotImplemented;
			} else if (saveAs) {
				QString dstPath = path + "/" + datName;
				if (!QFile::copy(datPath, dstPath)) {
					return ErrorCopying;
				}
			}
		}
		if (observer) {
			observer->setObserverValue(i++);
		}
	}

	return Ok;
}
