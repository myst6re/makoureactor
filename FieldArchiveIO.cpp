#include "FieldArchiveIO.h"
#include "GZIP.h"
#include "LZS.h"
#include "Config.h"
#include "Data.h"
#include "FieldPC.h"
#include "FieldPS.h"

FieldIO::FieldIO(Field *field, QObject *parent) :
	QIODevice(parent), _field(field)
{
}

void FieldIO::close()
{
	_cache.clear();
	QIODevice::close();
}

qint64 FieldIO::readData(char *data, qint64 maxSize)
{
	if(setCache()) {
		if(pos() < _cache.size()) {
			const char *constData = _cache.constData();
			qint64 r = qMin(maxSize, _cache.size() - pos());
			if(r > 0) {
				memcpy(data, constData + pos(), r);
				return r;
			} else if(r == 0) {
				return 0;
			} else {
				return -1;
			}
		} else {
			return 0;
		}
	}
	return -1;
}

bool FieldIO::setCache()
{
	if(_cache.isEmpty()) {
		if(!_field->save(_cache, true)) {
			_cache.clear();
			return false;
		}
	}
	return true;
}

FieldArchiveIO::~FieldArchiveIO()
{
	clearCachedData();
}

FieldArchiveIOLgp::FieldArchiveIOLgp(const QString &path) :
	_lgp(path)
{
}

FieldArchiveIOFile::FieldArchiveIOFile(const QString &path) :
	fic(path)
{
}

FieldArchiveIOIso::FieldArchiveIOIso(const QString &path) :
	iso(path)
{
}

FieldArchiveIODir::FieldArchiveIODir(const QString &path) :
	dir(path)
{
}

QString FieldArchiveIOLgp::path() const
{
	return _lgp.fileName();
}

QString FieldArchiveIOFile::path() const
{
	return fic.fileName();
}

QString FieldArchiveIOIso::path() const
{
	return iso.fileName();
}

QString FieldArchiveIODir::path() const
{
	return dir.path();
}

QString FieldArchiveIO::name() const
{
	if(!hasName())
		return QString();

	QString filePath = path();
	if(filePath.isEmpty()) {
		return filePath;
	}

	return filePath.mid(filePath.lastIndexOf("/") + 1);
}

QString FieldArchiveIO::directory() const
{
	if(!hasName())
		return path() + "/";

	QString filePath = path();
	if(filePath.isEmpty()) {
		return filePath;
	}

	return filePath.left(filePath.lastIndexOf("/") + 1);
}

void *FieldArchiveIOLgp::device()
{
	return &_lgp;
}

void *FieldArchiveIOFile::device()
{
	return &fic;
}

void *FieldArchiveIOIso::device()
{
	return &iso;
}

void *FieldArchiveIODir::device()
{
	return &dir;
}

QByteArray FieldArchiveIO::fieldDataCache;
QByteArray FieldArchiveIO::mimDataCache;
QByteArray FieldArchiveIO::modelDataCache;
Field *FieldArchiveIO::fieldCache=0;
Field *FieldArchiveIO::mimCache=0;
Field *FieldArchiveIO::modelCache=0;

QByteArray FieldArchiveIO::fieldData(Field *field, bool unlzs)
{
	// use data from the cache
	if(unlzs && fieldDataIsCached(field)) {
//		qDebug() << "FieldArchive use field data from cache" << field->name();
		return fieldDataCache;
	} /*else {
		qDebug() << "FieldArchive don't use field data from cache" << field->name() << unlzs;
	}*/

	QByteArray data = fieldData2(field, unlzs);

	// put decompressed data in the cache
	if(unlzs && !data.isEmpty()) {
		fieldCache = field;
		fieldDataCache = data;
	}
	return data;
}

QByteArray FieldArchiveIO::mimData(Field *field, bool unlzs)
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

QByteArray FieldArchiveIO::modelData(Field *field, bool unlzs)
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

QByteArray FieldArchiveIO::fileData(const QString &fileName, bool unlzs)
{
	QByteArray data = fileData2(fileName);

	if(data.size() < 4)		return QByteArray();

	const char *lzsDataConst = data.constData();
	quint32 lzsSize;
	memcpy(&lzsSize, lzsDataConst, 4);

	if(!Config::value("lzsNotCheck").toBool() && (quint32)data.size() != lzsSize + 4)
		return QByteArray();

	return unlzs
			? LZS::decompressAll(lzsDataConst + 4, qMin(lzsSize, quint32(data.size() - 4)))
			: data;
}

QByteArray FieldArchiveIOLgp::fieldData2(Field *field, bool unlzs)
{
	return fileData(field->name(), unlzs);
}

QByteArray FieldArchiveIOLgp::mimData2(Field *field, bool unlzs)
{
	Q_UNUSED(field)
	Q_UNUSED(unlzs)
	return QByteArray();
}

QByteArray FieldArchiveIOLgp::modelData2(Field *field, bool unlzs)
{
	Q_UNUSED(field)
	Q_UNUSED(unlzs)
	return QByteArray();
}

QByteArray FieldArchiveIOLgp::fileData2(const QString &fileName)
{
	if(!_lgp.isOpen() && !_lgp.open()) return QByteArray();
	return _lgp.fileData(fileName);
}

QByteArray FieldArchiveIOFile::fieldData2(Field *field, bool unlzs)
{
	Q_UNUSED(field)
	return fileData(QString(), unlzs);
}

QByteArray FieldArchiveIOFile::mimData2(Field *field, bool unlzs)
{
	Q_UNUSED(field)
	Q_UNUSED(unlzs)
	return QByteArray();
}

QByteArray FieldArchiveIOFile::modelData2(Field *field, bool unlzs)
{
	Q_UNUSED(field)
	Q_UNUSED(unlzs)
	return QByteArray();
}

QByteArray FieldArchiveIOFile::fileData2(const QString &fileName)
{
	Q_UNUSED(fileName)
	if(!fic.isOpen() && !fic.open(QIODevice::ReadOnly))		return QByteArray();
	fic.reset();
	QByteArray data = fic.readAll();
	fic.close();

	return data;
}
QByteArray FieldArchiveIOIso::fieldData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".DAT", unlzs);
}

QByteArray FieldArchiveIOIso::mimData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".MIM", unlzs);
}

QByteArray FieldArchiveIOIso::modelData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".BSX", unlzs);
}

QByteArray FieldArchiveIOIso::fileData2(const QString &fileName)
{
	return iso.file(isoFieldDirectory->file(fileName.toUpper()));
}

QByteArray FieldArchiveIODir::fieldData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".DAT", unlzs);
}

QByteArray FieldArchiveIODir::mimData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".MIM", unlzs);
}

QByteArray FieldArchiveIODir::modelData2(Field *field, bool unlzs)
{
	return fileData(field->name().toUpper() + ".BSX", unlzs);
}

QByteArray FieldArchiveIODir::fileData2(const QString &fileName)
{
	QByteArray data;

	QFile f(dir.filePath(fileName.toUpper()));
	if(!f.open(QIODevice::ReadOnly))	return QByteArray();
	data = f.readAll();
	f.close();

	return data;
}

bool FieldArchiveIO::fieldDataIsCached(Field *field) const
{
	return fieldCache && fieldCache == field;
}

bool FieldArchiveIO::mimDataIsCached(Field *field) const
{
	return mimCache && mimCache == field;
}

bool FieldArchiveIO::modelDataIsCached(Field *field) const
{
	return modelCache && modelCache == field;
}

void FieldArchiveIO::clearCachedData()
{
//	qDebug() << "FieldArchive::clearCachedData()";
	fieldCache = 0;
	mimCache = 0;
	modelCache = 0;
	fieldDataCache.clear();
	mimDataCache.clear();
	modelDataCache.clear();
}

void FieldArchiveIO::close()
{
	clearCachedData();
}

void FieldArchiveIOLgp::close()
{
	_lgp.close();
	FieldArchiveIO::close();
}

void FieldArchiveIOFile::close()
{
	fic.close();
	FieldArchiveIO::close();
}

FieldArchiveIO::ErrorCode FieldArchiveIO::open(QList<Field *> &fields, QMap<QString, TutFile *> &tuts, FieldArchiveIOObserver *observer)
{
	clearCachedData();

	return open2(fields, tuts, observer);
}

FieldArchiveIO::ErrorCode FieldArchiveIOLgp::open2(QList<Field *> &fields, QMap<QString, TutFile *> &tuts,
												   FieldArchiveIOObserver *observer)
{
	if(!_lgp.isOpen() && !_lgp.open()) {
		return ErrorOpening;
	}

	QStringList archiveList = _lgp.fileList();

	if(archiveList.isEmpty()) {
		return Invalid;
	}

	if(observer)	observer->setObserverMaximum(archiveList.size());

	quint32 i, freq = archiveList.size()>50 ? archiveList.size()/50 : 1;

//	QTime t;t.start();

	i = 0;
	foreach(const QString &name, archiveList) {
		if(i % freq == 0) {
			QCoreApplication::processEvents();
			if(observer)	observer->setObserverValue(i);
		}

		if(name.compare("maplist", Qt::CaseInsensitive) == 0) {
			if(!Data::openMaplist(_lgp.fileData(name))) {
				qWarning() << "Cannot open maplist!";
			}
		} else if(name.endsWith(".tut", Qt::CaseInsensitive)) {
			tuts.insert(name.toLower().left(name.size()-4), NULL);
		} else if(!name.contains('.')) {
			fields.append(new FieldPC(name, this));
			++i;
		}
	}
//	qDebug("Ouverture : %d ms", t.elapsed());
	// qDebug() << FICHIER::size << " o";
	// qDebug() << "nbGS : " << GrpScript::COUNT;
	// qDebug() << "nbS : " << Script::COUNT;
	// qDebug() << "nbC : " << Opcode::COUNT;
	// qDebug("-------------------------------------------------");

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOFile::open2(QList<Field *> &fields, QMap<QString, TutFile *> &tuts,
													FieldArchiveIOObserver *observer)
{
	QString path = fic.fileName();
	QString name = path.mid(path.lastIndexOf('/') + 1);
	fields.append(new FieldPS(name.left(name.lastIndexOf('.')), this));

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOIso::open2(QList<Field *> &fields, QMap<QString, TutFile *> &tuts,
												   FieldArchiveIOObserver *observer)
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
		QCoreApplication::processEvents();
		if(observer)	observer->setObserverValue(i);

		if(file->name().endsWith(".DAT") && !file->name().startsWith("WM")) {
			QString name = file->name().mid(file->name().lastIndexOf('/')+1);
			fields.append(new FieldPS(name.left(name.lastIndexOf('.')), this));
		}
	}
	// qDebug("Ouverture : %d ms", t.elapsed());

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIODir::open2(QList<Field *> &fields, QMap<QString, TutFile *> &tuts,
												   FieldArchiveIOObserver *observer)
{
	QStringList list;
	list.append("*.DAT");
	list = dir.entryList(list, QDir::Files | QDir::NoSymLinks);

	if(observer)	observer->setObserverMaximum(list.size());

	// QTime t;t.start();

	int i=0;
	foreach(const QString &name, list) {
		QCoreApplication::processEvents();
		if(observer)	observer->setObserverValue(i++);

		if(!name.startsWith("WM", Qt::CaseInsensitive)) {
			fields.append(new FieldPS(name.left(name.lastIndexOf('.')), this));
		}
	}

	// qDebug("Ouverture : %d ms", t.elapsed());

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIO::save(const QList<Field *> &fields, const QMap<QString, TutFile *> &tuts,
											   const QString &path, FieldArchiveIOObserver *observer)
{
	ErrorCode error = save2(fields, tuts, path, observer);
	if(error == Ok) {
		clearCachedData(); // Important: the file data will change
	}
	return error;
}

FieldArchiveIO::ErrorCode FieldArchiveIOLgp::save2(const QList<Field *> &fields, const QMap<QString, TutFile *> &tuts,
												   const QString &path, FieldArchiveIOObserver *observer)
{
	if(!_lgp.isOpen() && !_lgp.open()) {
		return ErrorOpening;
	}

	foreach(Field *field, fields) {
		if(field->isOpen() && field->isModified()) {
			if(!_lgp.setFile(field->name(), new FieldIO(field))) {
				return FieldNotFound;
			}
		}
	}

	QMapIterator<QString, TutFile *> itTut(tuts);

	while(itTut.hasNext()) {
		itTut.next();
		TutFile *tut = itTut.value();

		if(tut != NULL && tut->isModified()) {
			QByteArray tocTut, tutData;
			tutData = tut->save(tocTut);
			tocTut.append(tutData);
			if(!_lgp.setFile(itTut.key() + ".tut", tocTut)) {
				return FieldNotFound;
			}
		}
	}

	this->observer = observer;

	if(!_lgp.pack(path, this)) {
		this->observer = 0;

		switch(_lgp.error()) {
		case Lgp::OpenError:
			return ErrorOpeningTemp;
		case Lgp::InvalidError:
			return Invalid;
		case Lgp::CopyError:
		case Lgp::RemoveError:
		case Lgp::RenameError:
			return ErrorRemoving;
		default:
			return Invalid;
		}
	}

	this->observer = 0;

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOFile::save2(const QList<Field *> &fields, const QMap<QString, TutFile *> &tuts,
													const QString &path0, FieldArchiveIOObserver *observer)
{
	QString path = path0.isNull() ? fic.fileName() : path0;

	Field *field = fields.first();
	if(field->isOpen() && field->isModified()) {
		qint8 err = field->save(path, true);
		if(err == 2)	return ErrorOpening;
		if(err == 1)	return Invalid;
		if(err != 0)	return NotImplemented;
	}

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOIso::save2(const QList<Field *> &fields, const QMap<QString, TutFile *> &tuts,
												   const QString &path0, FieldArchiveIOObserver *observer)
{
	QString path = path0.isNull() ? iso.fileName() : path0;

	bool saveAs = QFileInfo(path) != QFileInfo(iso);

	IsoArchive isoTemp(path%".makoutemp");
	if(!isoTemp.open(QIODevice::ReadWrite | QIODevice::Truncate))		return ErrorOpening;

	if(observer)	observer->setObserverMaximum(100);
	// Reset IsoControl
	baseEstimation = 0;
	estimation = 100;

	// FIELD/*.DAT

	bool archiveModified = false;

	foreach(Field *field, fields) {
		if(field->isOpen() && field->isModified()) {
			IsoFile *isoField = isoFieldDirectory->file(field->name().toUpper() + ".DAT");
			if(isoField == NULL) {
				continue;
			}

			QByteArray newData;

			if(field->save(newData, true)) {
				isoField->setData(newData);
				archiveModified = true;
			} else {
				return Invalid;
			}
		}
	}

	if(!archiveModified) {
		return Invalid;
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

FieldArchiveIO::ErrorCode FieldArchiveIODir::save2(const QList<Field *> &fields, const QMap<QString, TutFile *> &tuts,
												   const QString &path, FieldArchiveIOObserver *observer)
{
	quint32 nbFiles = fields.size();
	if(observer)	observer->setObserverMaximum(nbFiles);

	for(quint32 fieldID=0 ; fieldID<nbFiles ; ++fieldID) {
		Field *field = fields.at(fieldID);
		if(field->isOpen() && field->isModified()) {
			QString datPath = dir.filePath(field->name().toUpper() + ".DAT");
			qint8 err = field->save(datPath, true);
			if(err == 2)	return ErrorOpening;
			if(err == 1)	return Invalid;
			if(err != 0)	return NotImplemented;
		}
		if(observer)	observer->setObserverValue(fieldID);
	}

	return Ok;
}

QByteArray FieldArchiveIOIso::updateFieldBin(const QByteArray &data, IsoDirectory *fieldDirectory)
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
