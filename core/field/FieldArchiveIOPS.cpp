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
	if(unlzs && mimDataIsCached(field)) {
		return mimDataCache;
	}

	QByteArray data = mimData2(field, unlzs);

	// put decompressed data in the cache
	if(unlzs && !data.isEmpty()) {
		mimCache = field;
		mimDataCache = data;
	}
	return data;
}

QByteArray FieldArchiveIOPS::modelData(Field *field, bool unlzs)
{
	// use data from the cache
	if(unlzs && modelDataIsCached(field)) {
		return modelDataCache;
	}

	QByteArray data = modelData2(field, unlzs);

	// put decompressed data in the cache
	if(unlzs && !data.isEmpty()) {
		modelCache = field;
		modelDataCache = data;
	}
	return data;
}

bool FieldArchiveIOPS::mimDataIsCached(Field *field) const
{
	return mimCache && mimCache == field;
}

bool FieldArchiveIOPS::modelDataIsCached(Field *field) const
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
	return (FieldArchivePS *)FieldArchiveIO::fieldArchive();
}

FieldArchiveIOPSFile::FieldArchiveIOPSFile(const QString &path, FieldArchivePS *fieldArchive) :
	FieldArchiveIOPS(fieldArchive), fic(path)
{
}

QString FieldArchiveIOPSFile::path() const
{
	return fic.fileName();
}

void *FieldArchiveIOPSFile::device()
{
	return &fic;
}

QByteArray FieldArchiveIOPSFile::fieldData2(Field *field, bool unlzs)
{
	Q_UNUSED(field)
	return fileData(QString(), unlzs);
}

QByteArray FieldArchiveIOPSFile::mimData2(Field *field, bool unlzs)
{
	Q_UNUSED(field)
	Q_UNUSED(unlzs)
	return QByteArray();
}

QByteArray FieldArchiveIOPSFile::modelData2(Field *field, bool unlzs)
{
	Q_UNUSED(field)
	Q_UNUSED(unlzs)
	return QByteArray();
}

QByteArray FieldArchiveIOPSFile::fileData2(const QString &fileName)
{
	Q_UNUSED(fileName)
	if(!fic.isOpen() && !fic.open(QIODevice::ReadOnly))		return QByteArray();
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

FieldArchiveIO::ErrorCode FieldArchiveIOPSFile::open2(FieldArchiveIOObserver *observer)
{
	Q_UNUSED(observer)

	QString name = this->name();
	fieldArchive()->addField(new FieldPS(name.left(name.lastIndexOf('.')), this));

	return fieldArchive()->field(0) != NULL ? Ok : Invalid;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSFile::save2(const QString &path0, FieldArchiveIOObserver *observer)
{
	Q_UNUSED(observer)
	QString path = path0.isNull() ? fic.fileName() : path0;

	Field *field = fieldArchive()->field(0, false);
	if(field && field->isOpen() && field->isModified()) {
		qint8 err = field->save(path, true);
		if(err == 2)	return ErrorOpening;
		if(err == 1)	return Invalid;
		if(err != 0)	return NotImplemented;
	}

	return Ok;
}

FieldArchiveIOPSIso::FieldArchiveIOPSIso(const QString &path, FieldArchivePS *fieldArchive) :
	FieldArchiveIOPS(fieldArchive), iso(path)
{
}

QString FieldArchiveIOPSIso::path() const
{
	return iso.fileName();
}

void *FieldArchiveIOPSIso::device()
{
	return &iso;
}

QByteArray FieldArchiveIOPSIso::fieldData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".DAT", unlzs);
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
	return iso.file(isoFieldDirectory->file(fileName.toUpper()));
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSIso::open2(FieldArchiveIOObserver *observer)
{
	if(!iso.isOpen() && !iso.open(QIODevice::ReadOnly)) {
		return ErrorOpening;
	}

	isoFieldDirectory = iso.rootDirectory()->directory("FIELD");
	if(isoFieldDirectory == NULL)	return FieldNotFound;

	QList<IsoFile *> files = isoFieldDirectory->files();

	if(observer)	observer->setObserverMaximum(files.size());

	// QTime t;t.start();

	int i=0;
	foreach(IsoFile *file, files) {
		if(observer) {
			if(observer->observerWasCanceled()) {
				return Aborted;
			}
			observer->setObserverValue(i);
		}

		if(file->name().endsWith(".DAT") && !file->name().startsWith("WM")) {
			QString name = file->name().mid(file->name().lastIndexOf('/')+1);
			fieldArchive()->addField(new FieldPS(name.left(name.lastIndexOf('.')), this));
		}
	}
	// qDebug("Ouverture : %d ms", t.elapsed());

	Data::windowBin = WindowBinFile();

	if(!Data::windowBin.open(iso.file("INIT/WINDOW.BIN"))) {
		qWarning() << "Cannot open window.bin";
	}

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSIso::save2(const QString &path0, FieldArchiveIOObserver *observer)
{
	QString path = path0.isNull() ? iso.fileName() : path0;

	bool saveAs = QFileInfo(path) != QFileInfo(iso);

	IsoArchive isoTemp(path%".makoutemp");
	if(!isoTemp.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
		return ErrorOpening;
	}

	if(observer)	observer->setObserverMaximum(100);
	// Reset IsoControl
	baseEstimation = 0;
	estimation = 100;

	// FIELD/*.DAT

	for(int fieldID=0 ; fieldID<fieldArchive()->size() ; ++fieldID) {
		if(observer && observer->observerWasCanceled()) {
			return Aborted;
		}
		Field *field = fieldArchive()->field(fieldID, false);
		if(field && field->isOpen() && field->isModified()) {
			IsoFile *isoField = isoFieldDirectory->file(field->name().toUpper() + ".DAT");
			if(isoField == NULL) {
				continue;
			}

			QByteArray newData;

			if(field->save(newData, true)) {
				isoField->setData(newData);
			} else {
				return Invalid;
			}
		}
	}

	// FIELD/FIELD.BIN

	IsoFile *isoFieldBin = isoFieldDirectory->file("FIELD.BIN");
	if(isoFieldBin == NULL) {
		return Invalid;
	}

	QByteArray data = updateFieldBin(iso.file(isoFieldBin), isoFieldDirectory);
	if(data.isEmpty()) {
		return Invalid;
	}
	isoFieldBin->setData(data);

//		qDebug() << "start";

	this->observer = observer;
	if(!iso.pack(&isoTemp, this, isoFieldDirectory)) {
		if(wasCanceled()) {
			return Aborted;
		}
		this->observer = 0;
		return Invalid;
	}
	this->observer = 0;

	// End

	// Remove or close the old file
	if(!saveAs) {
		if(!iso.remove())	return ErrorRemoving;
	} else {
		iso.close();
		iso.setFileName(path);
	}
	// Move the temporary file
	if(QFile::exists(path) && !QFile::remove(path))		return ErrorRemoving;
	isoTemp.rename(path);

	iso.open(QIODevice::ReadOnly);

	// Clear "isModified" state
	iso.applyModifications(isoFieldDirectory);

	return Ok;
}

QByteArray FieldArchiveIOPSIso::updateFieldBin(const QByteArray &data, IsoDirectory *fieldDirectory)
{
	QByteArray header = data.left(8), ungzip;
	quint32 oldSectorStart, newSectorStart, oldSize, newSize;
	QMap<QByteArray, QByteArray> changementsFieldBin;
	QMap<QByteArray, QString> fichiers;//debug

	foreach(IsoDirectory *fileOrDir, fieldDirectory->directories()) {
		if(fileOrDir->isModified() && !fileOrDir->name().endsWith(".X") && fileOrDir->name()!="FIELD.BIN") {
			oldSectorStart = fileOrDir->location();
			oldSize = fileOrDir->size();
			newSectorStart = fileOrDir->newLocation();
			newSize = fileOrDir->newSize();

			QByteArray tempBA = QByteArray((char *)&oldSectorStart, 4).append((char *)&oldSize, 4);
			changementsFieldBin.insert(tempBA, QByteArray((char *)&newSectorStart, 4).append((char *)&newSize, 4));
			fichiers.insert(tempBA, fileOrDir->name());
		}
	}

	/**
	 *	HEADER :
	 *	 4 : ungzipped size
	 *	 4 : ungzipped size - 51588
	 */
	// décompression gzip
	quint32 decSize;
	memcpy(&decSize, data.constData(), 4);
	ungzip = GZIP::decompress(data.mid(8), decSize);
	if(ungzip.isEmpty())	return QByteArray();

//	qDebug() << "header field.bin" << header.toHex() << QString::number(data.size()-8,16) << decSize << ungzip.size();

	// mise à jour

	int indicativePosition=0x30000, count;
	QByteArray copy = ungzip;
	QMapIterator<QByteArray, QByteArray> i(changementsFieldBin);
	while(i.hasNext()) {
		i.next();
		count = copy.count(i.key());
		if(count == 0) {
			qWarning() << "Error not found!" << fichiers.value(i.key());
			return QByteArray();
		} else if(count == 1) {
			indicativePosition = copy.indexOf(i.key());
			ungzip.replace(i.key(), i.value());
		} else {
			qWarning() << "error multiple occurrences 1" << fichiers.value(i.key());
			if(copy.mid(indicativePosition).count(i.key())==1) {
				ungzip.replace(copy.indexOf(i.key(), indicativePosition), 8, i.value());
			} else {
				qWarning() << "error multiple occurrences 2" << fichiers.value(i.key());
				return QByteArray();
			}
		}
	}

	copy = GZIP::compress(ungzip);

	if(copy.isEmpty())	return QByteArray();

	return header.append(copy);
}

FieldArchiveIOPSDir::FieldArchiveIOPSDir(const QString &path, FieldArchivePS *fieldArchive) :
	FieldArchiveIOPS(fieldArchive), dir(path)
{
}

QString FieldArchiveIOPSDir::path() const
{
	return dir.path();
}

void *FieldArchiveIOPSDir::device()
{
	return &dir;
}

QByteArray FieldArchiveIOPSDir::fieldData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".DAT", unlzs);
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
	if(!f.open(QIODevice::ReadOnly))	return QByteArray();
	data = f.readAll();
	f.close();

	return data;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSDir::open2(FieldArchiveIOObserver *observer)
{
	QStringList list;
	list.append("*.DAT");
	list = dir.entryList(list, QDir::Files | QDir::NoSymLinks);

	if(observer)	observer->setObserverMaximum(list.size());

	// QTime t;t.start();

	int i=0;
	foreach(const QString &name, list) {
		if(observer) {
			if(observer->observerWasCanceled()) {
				return Aborted;
			}
			observer->setObserverValue(i++);
		}

		if(!name.startsWith("WM", Qt::CaseInsensitive)) {
			fieldArchive()->addField(new FieldPS(name.left(name.lastIndexOf('.')), this));
		}
	}

	// qDebug("Opening: %d ms", t.elapsed());

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPSDir::save2(const QString &path, FieldArchiveIOObserver *observer)
{
	bool saveAs;

	if(!path.isEmpty()) {
		saveAs = QDir::cleanPath(path) != QDir::cleanPath(dir.path());
	} else {
		saveAs = false;
	}

	quint32 nbFiles = fieldArchive()->size();
	if(observer)	observer->setObserverMaximum(nbFiles);

	for(quint32 fieldID=0 ; fieldID<nbFiles ; ++fieldID) {
		if(observer && observer->observerWasCanceled()) {
			return Aborted;
		}
		Field *field = fieldArchive()->field(fieldID, false);
		QString datName = field->name().toUpper() + ".DAT";
		QString datPath = dir.filePath(datName);
		if(field) {
			if(field && field->isOpen() && field->isModified()) {
				qint8 err = field->save(datPath, true);
				if(err == 2)	return ErrorOpening;
				if(err == 1)	return Invalid;
				if(err != 0)	return NotImplemented;
			} else if(saveAs) {
				QString dstPath = path + "/" + datName;
				if(!QFile::copy(datPath, dstPath)) {
					return ErrorCopying;
				}
			}
		}
		if(observer)	observer->setObserverValue(fieldID);
	}

	return Ok;
}
