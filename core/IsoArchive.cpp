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
#include "IsoArchive.h"

IsoFileOrDirectory::IsoFileOrDirectory(const QString &name, quint32 location, quint32 size, qint64 structPosition) :
	structPosition(structPosition), _name(name), _location(location), _size(size),
	_newLocation(location), _newSize(size), _paddingAfter(0)
{
}

IsoFileOrDirectory::~IsoFileOrDirectory()
{
}

const QString &IsoFileOrDirectory::name() const
{
	return _name;
}

quint32 IsoFileOrDirectory::location() const
{
	return _location;
}

quint32 IsoFileOrDirectory::locationAfter() const
{
	return location() + sectorCount();
}

quint32 IsoFileOrDirectory::size() const
{
	return _size;
}

quint32 IsoFileOrDirectory::sectorCount() const
{
	return _size / SECTOR_SIZE_DATA + (int)(_size % SECTOR_SIZE_DATA != 0);
}

quint32 IsoFileOrDirectory::newLocation() const
{
	return _newLocation;
}

quint32 IsoFileOrDirectory::newSize() const
{
	return _newSize;
}

quint32 IsoFileOrDirectory::newSectorCount() const
{
	return _newSize / SECTOR_SIZE_DATA + (int)(_newSize % SECTOR_SIZE_DATA != 0);
}

void IsoFileOrDirectory::setName(const QString &name)
{
	_name = name;
}

void IsoFileOrDirectory::setLocation(quint32 location)
{
	_newLocation = location;
}

bool IsoFileOrDirectory::isFile() const
{
	return !isDirectory();
}

bool IsoFileOrDirectory::isModified() const
{
	return _location != _newLocation || _size != _newSize;
}

void IsoFileOrDirectory::applyModifications()
{
	_location = _newLocation;
	_size = _newSize;
}

bool IsoFileOrDirectory::isSpecial() const
{
	return _name.isEmpty() || _name == "\x01";
}

void IsoFileOrDirectory::setPaddingAfter(quint8 after)
{
	_paddingAfter = after;
}

quint8 IsoFileOrDirectory::paddingAfter() const
{
	return _paddingAfter;
}

IsoDirectory::IsoDirectory(const QString &name, quint32 location, quint32 size, qint64 structPosition) :
	IsoFileOrDirectory(name, location, size, structPosition)
{
}

IsoDirectory::~IsoDirectory()
{
	qDeleteAll(_filesAndDirectories);
}

bool IsoDirectory::isDirectory() const
{
	return true;
}

const QMap<QString, IsoFileOrDirectory *> &IsoDirectory::filesAndDirectories() const
{
	return _filesAndDirectories;
}

QList<IsoFile *> IsoDirectory::files() const
{
	QList<IsoFile *> fs;

	foreach(IsoFileOrDirectory *fOrD, _filesAndDirectories) {
		if(fOrD->isFile()) {
			fs.append(static_cast<IsoFile *>(fOrD));
		}
	}

	return fs;
}

QList<IsoDirectory *> IsoDirectory::directories() const
{
	QList<IsoDirectory *> ds;

	foreach(IsoFileOrDirectory *fOrD, _filesAndDirectories) {
		if(fOrD->isDirectory()) {
			ds.append(static_cast<IsoDirectory *>(fOrD));
		}
	}

	return ds;
}

IsoFileOrDirectory *IsoDirectory::fileOrDirectory(const QString &path) const
{
	int index;

	if((index = path.indexOf("/")) != -1) {
		IsoFileOrDirectory *fOrD = _filesAndDirectories.value(path.left(index).toUpper(), NULL);

		if(fOrD == NULL)	return NULL;

		if(fOrD->isDirectory()) {
			return static_cast<IsoDirectory *>(fOrD)->fileOrDirectory(path.mid(index+1));
		} else {
			return NULL;
		}
	} else {
		return _filesAndDirectories.value(path.toUpper(), NULL);
	}

}

IsoFile *IsoDirectory::file(const QString &path) const
{
	IsoFileOrDirectory *fOrD = fileOrDirectory(path);

	if(fOrD == NULL)	return NULL;

	if(fOrD->isDirectory()) {
		return NULL;
	}

	return static_cast<IsoFile *>(fOrD);
}

IsoDirectory *IsoDirectory::directory(const QString &path) const
{
	IsoFileOrDirectory *fOrD = fileOrDirectory(path);

	if(fOrD == NULL)	return NULL;

	if(fOrD->isFile()) {
		return NULL;
	}

	return static_cast<IsoDirectory *>(fOrD);
}

void IsoDirectory::add(IsoFileOrDirectory *fileOrDirectory)
{
	_filesAndDirectories.insert(fileOrDirectory->name(), fileOrDirectory);
}

IsoFile::IsoFile(const QString &name, quint32 location, quint32 size, qint64 structPosition, IsoArchiveIO *io) :
	IsoFileOrDirectory(name, location, size, structPosition),
	_io(0), _newIO(0), dataChanged(false),
	_newIOMustBeRemoved(false)
{
	setFile(new IsoFileIO(io, this));
}

IsoFile::~IsoFile()
{
	if (_io)	delete _io;
	cleanNewIO();
}

bool IsoFile::isDirectory() const
{
	return false;
}

QByteArray IsoFile::data(quint32 maxSize) const
{
	if(_io->isOpen() || _io->open(QIODevice::ReadOnly)) {
		_io->reset();
		return maxSize == 0 ? _io->readAll() : _io->read(maxSize);
	}
	return QByteArray();
}

QByteArray IsoFile::modifiedData(quint32 maxSize) const
{
	if(_newIO && (_newIO->isOpen() || _newIO->open(QIODevice::ReadOnly))) {
		_newIO->reset();
		return maxSize == 0 ? _newIO->readAll() : _newIO->read(maxSize);
	}
	return data(maxSize);
}

bool IsoFile::extract(const QString &destination, quint32 maxSize) const
{
//	QTime t;t.start();

	maxSize = maxSize == 0 ? size() : qMin(maxSize, size());
	char data[MAX_ISO_READ];
	qint64 r, totalR=0;
	QFile ret(destination);
	if(!ret.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return false;
	}
	if(!_io->isOpen() && !_io->open(QIODevice::ReadOnly)) {
		return false;
	}

	while(maxSize-totalR > 0 && (r = _io->read(data, qMin((qint64)MAX_ISO_READ, maxSize-totalR))) > 0) {
		ret.write(data, r);
		totalR += r;
	}

//	qDebug() << "time" << t.elapsed();

	return true;
}

QIODevice *IsoFile::file() const
{
	return _io;
}

QIODevice *IsoFile::modifiedFile() const
{
	if (_newIO) {
		return _newIO;
	}
	return file();
}

void IsoFile::setFile(QIODevice *io)
{
	_io = io;
}

bool IsoFile::setModifiedFile(QIODevice *io)
{
	if(!io->isOpen() && !io->open(QIODevice::ReadOnly)) {
		return false;
	}

	cleanNewIO();
	_newIO = io;
	_newSize = _newIO->size();
	dataChanged = true;

	return true;
}

bool IsoFile::setModifiedFile(const QByteArray &data)
{
	QBuffer *io = new QBuffer();
	io->setData(data);
	_newIOMustBeRemoved = true;

	return setModifiedFile(io);
}

void IsoFile::cleanNewIO()
{
	if (_newIOMustBeRemoved && _newIO) {
		delete _newIO;
		_newIOMustBeRemoved = false;
	}
	_newIO = 0;
}

bool IsoFile::isModified() const
{
	return dataChanged || IsoFileOrDirectory::isModified();
}

void IsoFile::applyModifications()
{
	cleanNewIO();
	dataChanged = false;
	IsoFileOrDirectory::applyModifications();
}

IsoArchiveIO::IsoArchiveIO()
{
}

IsoArchiveIO::IsoArchiveIO(const QString &name) :
	QFile(name)
{
}

IsoArchiveIO::~IsoArchiveIO()
{
}

bool IsoArchiveIO::open(QIODevice::OpenMode mode)
{
	bool open = QFile::open(mode);
	if(!open)	return false;

	if(mode.testFlag(QIODevice::Truncate)) {
		return true;
	}

	reset();

	return true;
}

qint64 IsoArchiveIO::posIso() const
{
	return isoPos(pos());
}

bool IsoArchiveIO::seekIso(qint64 off)
{
	return seek(filePos(off));
}

qint64 IsoArchiveIO::sizeIso() const
{
	return isoPos(size());
}

qint64 IsoArchiveIO::isoPos(qint64 pos)
{
	qint64 start = (pos % SECTOR_SIZE) - SECTOR_SIZE_HEADER;
	if(start < 0) {
		start = 0;
	} else if(start > SECTOR_SIZE_DATA) {
		start = SECTOR_SIZE_DATA;
	}

	return SECTOR_SIZE_DATA * (pos / SECTOR_SIZE) + start;
}

qint64 IsoArchiveIO::filePos(qint64 pos)
{
	return SECTOR_SIZE_HEADER + (pos / SECTOR_SIZE_DATA) * SECTOR_SIZE + pos % SECTOR_SIZE_DATA;
}

qint64 IsoArchiveIO::readIso(char *data, qint64 maxSize)
{
	qint64 read, readTotal = 0, seqLen;

//	qDebug() << pos() << "seek >> " << seqLen;
	if(!seekIso(isoPos(pos())))	return 0;

	seqLen = qMin(SECTOR_SIZE_HEADER + SECTOR_SIZE_DATA - (pos() % SECTOR_SIZE), maxSize);
	if(seqLen < 0)	return 0;

	while((read = this->read(data, seqLen)) > 0) {
//		qDebug() << "read" << seqLen << read;
		data += read;
		maxSize -= read;
		readTotal += read;
		seqLen = qMin((qint64)SECTOR_SIZE_DATA, maxSize);
//		qDebug() << "seqLen" << seqLen << maxSize << pos();
		// Si on est à la fin du secteur
		if(pos() % SECTOR_SIZE >= SECTOR_SIZE_HEADER + SECTOR_SIZE_DATA) {
			if(!seek(pos() + SECTOR_SIZE_HEADER + SECTOR_SIZE_FOOTER))	break;
//			qDebug() << "seek >> 304" << pos();
		}
	}

	return read < 0 ? read : readTotal;
}

QByteArray IsoArchiveIO::readIso(qint64 maxSize)
{
	char *data = new char[maxSize];

	QByteArray baData(data, readIso(data, maxSize));

	delete[] data;

	return baData;
}

qint64 IsoArchiveIO::writeIso(const char *data, qint64 maxSize)
{
	qint64 write, writeTotal = 0, seqLen;

//	qDebug() << pos() << "seek >> " << seqLen;
	if(!seekIso(isoPos(pos())))	return 0;

	seqLen = qMin(SECTOR_SIZE_HEADER + SECTOR_SIZE_DATA - (pos() % SECTOR_SIZE), maxSize);

	while((write = this->write(data, seqLen)) > 0) {
//		qDebug() << "write" << seqLen << write;
		data += write;
		maxSize -= write;
		writeTotal += write;
		seqLen = qMin((qint64)2048, maxSize);
//		qDebug() << "seqLen" << seqLen << maxSize << pos();
		// Si on est à la fin du secteur
		if(pos() % SECTOR_SIZE >= SECTOR_SIZE_HEADER + SECTOR_SIZE_DATA) {
			if(!seek(pos() + SECTOR_SIZE_HEADER + SECTOR_SIZE_FOOTER))	break;
//			qDebug() << "seek >> 304" << pos();
		}
	}

	return write < 0 ? write : writeTotal;
}

qint64 IsoArchiveIO::writeIso(const QByteArray &byteArray)
{
	return writeIso(byteArray.constData(), byteArray.size());
}

QByteArray IsoArchiveIO::sectorHeader(quint32 num)
{
	seek(SECTOR_SIZE * num);
	return read(SECTOR_SIZE_HEADER);
}

QByteArray IsoArchiveIO::sectorFooter(quint32 num)
{
	seek(SECTOR_SIZE * num + SECTOR_SIZE_HEADER + SECTOR_SIZE_DATA);
	return read(SECTOR_SIZE_FOOTER);
}

QByteArray IsoArchiveIO::sector(quint32 num, quint16 maxSize)
{
	seek(SECTOR_SIZE * num + SECTOR_SIZE_HEADER);
	return read(qMin(maxSize, quint16(SECTOR_SIZE_DATA)));
}

quint32 IsoArchiveIO::currentSector() const
{
	return pos() / SECTOR_SIZE;
}

quint32 IsoArchiveIO::sectorCount() const
{
	return size() / SECTOR_SIZE;
}

quint32 IsoArchiveIO::sectorCountData(quint32 dataSize)
{
	return dataSize / SECTOR_SIZE_DATA;
}

bool IsoArchiveIO::seekToSector(quint32 num)
{
	return seek(SECTOR_SIZE * num);
}

bool IsoArchiveIO::writeSector(const QByteArray &data, quint8 type, quint8 mode)
{
	qint64 dataSize = data.size();
	quint32 sectorCur = currentSector();
	QByteArray sectorData;

	Q_ASSERT(pos() % SECTOR_SIZE == 0);
	Q_ASSERT(dataSize <= SECTOR_SIZE_DATA);

	// sector header
	sectorData = buildHeader(sectorCur, type, mode);
	// data
	sectorData.append(data);
	if(dataSize != SECTOR_SIZE_DATA) {
		sectorData.append(QByteArray(SECTOR_SIZE_DATA - dataSize, '\x00'));
	}
	// sector footer
	sectorData.append(buildFooter(sectorCur));

	return SECTOR_SIZE == write(sectorData);
}

IsoFileIO::IsoFileIO(IsoArchiveIO *io, const IsoFile *infos, QObject *parent) :
	QIODevice(parent), _io(io), _infos(infos)
{
}

bool IsoFileIO::open(OpenMode mode) {
	if(mode.testFlag(QIODevice::Append)
			|| mode.testFlag(QIODevice::Truncate)
			|| mode.testFlag(QIODevice::WriteOnly)) {
		return false;
	}
	return QIODevice::open(mode);
}

qint64 IsoFileIO::size() const
{
	return _infos->size();
}

qint64 IsoFileIO::readData(char *data, qint64 maxSize)
{
	if(_io->seekToSector(_infos->location()) && _io->seekIso(_io->posIso() + pos())) {
		qint64 size = this->size();
		if(size < 0) {
			return -1;
		}
		return _io->readIso(data, qMin(maxSize, size - pos()));
	}
	return -1;
}

qint64 IsoFileIO::writeData(const char *data, qint64 maxSize)
{
	Q_UNUSED(data)
	Q_UNUSED(maxSize)
	return -1;
}

bool IsoFileIO::canReadLine() const
{
	return pos() < size();
}

IsoArchive::IsoArchive() :
	_rootDirectory(NULL), _error(Archive::NoError)
{
}

IsoArchive::IsoArchive(const QString &name) :
	_io(name), _rootDirectory(NULL), _error(Archive::NoError)
{
}

IsoArchive::~IsoArchive()
{
	if(_rootDirectory != NULL)		delete _rootDirectory;
}

bool IsoArchive::open(QIODevice::OpenMode mode)
{
	if(!_io.open(mode)) {
		return false;
	}

	if (mode.testFlag(QIODevice::Truncate)) {
		return true;
	}

	if(!openVolumeDescriptor()) {
		qWarning() << "IsoArchive::open" << "cannot open volume descriptor";
		_io.setErrorString("Cannot open volume descriptor");
		return false;
	}
//	qDebug() << volumeDescriptorToString(volume);
	qint64 size = _io.size();
	if(size%SECTOR_SIZE != 0 || volume.vd1.volume_space_size != _io.sectorCount() || volume.vd1.id[0] != 'C' || volume.vd1.id[1] != 'D' || volume.vd1.id[2] != '0' || volume.vd1.id[3] != '0' || volume.vd1.id[4] != '1') {
		qWarning() << "IsoArchive::open error archive size" << (size%SECTOR_SIZE) << volume.vd1.volume_space_size << _io.sectorCount();
		_io.setErrorString("Archive size error");
		return false;
	}

	if (!openRootDirectory(volume.dr.drh.location_extent, volume.dr.drh.data_length)) {
		qWarning() << "IsoArchive::_open cannot open root directory";
		_io.setErrorString("Cannot open root directory");
		return false;
	}
//	pathTables1a = pathTable(volume.vd1.type_path_table, volume.vd1.path_table_size);
//	pathTables1b = pathTable(volume.vd1.opt_type_path_table, volume.vd1.path_table_size);
//	pathTables2a = pathTable(qFromBigEndian(volume.vd1.type_path_table2), volume.vd1.path_table_size);
//	pathTables2b = pathTable(qFromBigEndian(volume.vd1.opt_type_path_table2), volume.vd1.path_table_size);

	/*qDebug() << "PATHTABLE1";
	foreach(PathTable pt, pathTables1a)
		qDebug() << pathTableToString(pt);
	qDebug() << "PATHTABLE2";
	foreach(PathTable pt, pathTables1b)
		qDebug() << pathTableToString(pt);
	qDebug() << "PATHTABLE3";
	foreach(PathTable pt, pathTables2a)
		qDebug() << pathTableToString(pt, true);
	qDebug() << "PATHTABLE4";
	foreach(PathTable pt, pathTables2b)
		qDebug() << pathTableToString(pt, true);
	*/

	return true;
}

void IsoArchive::close()
{
	_io.close();
	if(_rootDirectory != NULL) {
		delete _rootDirectory;
		_rootDirectory = NULL;
	}
}

int IsoArchive::findPadding(const QList<IsoFileOrDirectory *> &filesWithPadding, quint32 minSectorCount)
{
	int i=0;
	foreach(IsoFileOrDirectory *fileOrDir, filesWithPadding) {
		if(fileOrDir->paddingAfter() >= minSectorCount) {
			return i;
		}
		++i;
	}

	return -1;
}

/**
 * @brief Used to extend IsoArchive. Called in pack() afer file reorganization.
 * @param directory
 * @return false to cancel pack()
 */
bool IsoArchive::reorganizeModifiedFilesAfter(QMap<quint32, const IsoFile *> &writeToTheMain, QList<const IsoFile *> &writeToTheEnd)
{
	Q_UNUSED(writeToTheMain)
	Q_UNUSED(writeToTheEnd)
	return true;
}

bool IsoArchive::pack(IsoArchive *destination, ArchiveObserver *control, IsoDirectory *directory)
{
	IsoArchiveIO *destinationIO = &(destination->_io);
	QMap<quint32, const IsoFile *> writeToTheMain;
	QList<const IsoFile *> writeToTheEnd;
	QList<IsoFileOrDirectory *> filesWithPadding = getIntegrity();
	IsoFileOrDirectory *lastFileWithPadding = filesWithPadding.isEmpty() ? NULL : filesWithPadding.last();
	quint32 endOfIso = _io.sectorCount(),
	        sectorCountAtTheEnd = lastFileWithPadding ? (endOfIso - lastFileWithPadding->locationAfter()) : 0,
	        lastPadding = lastFileWithPadding ? lastFileWithPadding->paddingAfter() : 0,
	        endOfFilledIso, endOfFilledIsoAfter;

	if (sectorCountAtTheEnd == lastPadding) {
		endOfFilledIso = lastFileWithPadding->locationAfter();
		lastFileWithPadding->setPaddingAfter(0);
	} else {
		endOfFilledIso = endOfIso;
	}
	endOfFilledIsoAfter = endOfFilledIso;
	
	if(directory == NULL) {
		directory = _rootDirectory;
		if(directory == NULL) {
			setError(Archive::OpenError, "Not opened to pack");
			return false;
		}
	}

	foreach(IsoFile *isoFile, getModifiedFiles(directory)) {
		// Est-ce que les nouvelles données sont plus grandes que les anciennes ? Est-ce qu'on est pas à la fin de l'archive ?
		if(isoFile->newSectorCount() > isoFile->sectorCount() + isoFile->paddingAfter()
				&& isoFile->location() + isoFile->sectorCount() < _io.sectorCount()) {
//#ifdef ISOARCHIVE_DEBUG
			qWarning() << "File too big" << isoFile->newSectorCount() << isoFile->sectorCount() << (isoFile->newSectorCount() - isoFile->sectorCount()) << isoFile->name();
//#endif
			int index = findPadding(filesWithPadding, isoFile->newSectorCount());
			if(index >= 0) {
				IsoFileOrDirectory *fileWithPaddingAfter = filesWithPadding.at(index);
				isoFile->setLocation(fileWithPaddingAfter->newLocation() + fileWithPaddingAfter->newSectorCount());
				isoFile->setPaddingAfter(fileWithPaddingAfter->paddingAfter() - isoFile->newSectorCount());
				fileWithPaddingAfter->setPaddingAfter(0);
				if (isoFile->paddingAfter() > 0) {
					filesWithPadding.replace(index, isoFile);
				} else {
					filesWithPadding.removeAt(index);
				}
				writeToTheMain.insert(isoFile->newLocation(), isoFile);
//#ifdef ISOARCHIVE_DEBUG
				qWarning() << "Found enough space at" << fileWithPaddingAfter->name() << isoFile->newLocation() << "for" << isoFile->name();
//#endif
			} else {
				writeToTheEnd.append(isoFile);
				isoFile->setLocation(endOfFilledIsoAfter);
//#ifdef ISOARCHIVE_DEBUG
				qWarning() << "Add file at the end" << isoFile->name() << endOfFilledIsoAfter;
//#endif
				endOfFilledIsoAfter += isoFile->newSectorCount();
			}
		} else {
			if(isoFile->paddingAfter() > isoFile->newSectorCount() - isoFile->sectorCount())
				isoFile->setPaddingAfter(isoFile->paddingAfter() - (isoFile->newSectorCount() - isoFile->sectorCount()));
			writeToTheMain.insert(isoFile->newLocation(), isoFile);
		}
	}

	if(!reorganizeModifiedFilesAfter(writeToTheMain, writeToTheEnd)) {
		return false;
	}

	control->setObserverMaximum(endOfIso);

	destinationIO->reset();
	_io.reset();

	foreach(const IsoFile *isoFile, writeToTheMain) {
		if(control && control->observerWasCanceled()) {
			setError(Archive::AbortError);
			return false;
		}

		quint32 fileLocation = isoFile->newLocation();

		// Données avant le fichier
		if(!copySectors(destinationIO, qint64(fileLocation) - qint64(_io.currentSector()), control)) {
			return false;
		}

#ifdef ISOARCHIVE_DEBUG
		if(destinationIO->pos() % SECTOR_SIZE != 0)           qWarning() << "destination error 1a" << (destinationIO->pos() % SECTOR_SIZE) << isoFile->name();
		if(destinationIO->currentSector() != fileLocation)    qWarning() << "destination error 1b" << destinationIO->currentSector() << fileLocation << isoFile->name();
		if(destinationIO->pos() != _io.pos())                 qWarning() << "destination error 1c" << destinationIO->pos() << _io.pos() << isoFile->name();
#endif

		// On écrit le même nombre de secteurs que le fichier d'origine
		if(!destination->writeFile(isoFile->modifiedFile(), isoFile->sectorCount() + isoFile->paddingAfter(), control)) {
			return false;
		}
		_io.seek(destinationIO->pos());

#ifdef ISOARCHIVE_DEBUG
		if(destinationIO->pos() % SECTOR_SIZE != 0)                  qWarning() << "destination error 2a" << (destinationIO->pos() % SECTOR_SIZE) << isoFile->name();
		if(destinationIO->currentSector() != _io.currentSector())    qWarning() << "destination error 2b" << destinationIO->currentSector() << _io.currentSector() << isoFile->name();
		if(destinationIO->pos() != _io.pos())                        qWarning() << "destination error 2c" << destinationIO->pos() << _io.pos() << isoFile->name();
#endif

		// Envoi de la position courante à l'output
		if(control) control->setObserverValue(destinationIO->currentSector());
	}

	// Données après les fichiers patchés
	if(!copySectors(destinationIO, endOfFilledIso - _io.currentSector(), control)) {
		return false;
	}

#ifdef ISOARCHIVE_DEBUG
	if(destinationIO->pos() % SECTOR_SIZE != 0)                qWarning() << "destination error 3a" << (destinationIO->pos() % SECTOR_SIZE);
	if(destinationIO->currentSector() != endOfFilledIso)       qWarning() << "destination error 3b" << destinationIO->currentSector() << _io.sectorCount();
	if(destinationIO->pos() != _io.pos())                      qWarning() << "destination error 3c" << destinationIO->pos() << _io.pos();
#endif

	// Fichiers trop gros mis à la fin de l'ISO
	foreach(const IsoFile *isoFile, writeToTheEnd) {
		if(control && control->observerWasCanceled()) {
			setError(Archive::AbortError);
			return false;
		}
#ifdef ISOARCHIVE_DEBUG
		qWarning() << "Write file at the end" << isoFile->name() << destinationIO->currentSector();
#endif

		if(!destination->writeFile(isoFile->modifiedFile(), 0, control)) {
			return false;
		}

#ifdef ISOARCHIVE_DEBUG
		if(destinationIO->pos() % SECTOR_SIZE != 0)      qWarning() << "destination error 3a" << (destinationIO->pos() % SECTOR_SIZE) << isoFile->name();
#endif

		// Envoi de la position courante à l'output
		if(control) control->setObserverValue(destinationIO->currentSector());
	}

	if(!copySectors(destinationIO, endOfIso - endOfFilledIso, control, true)) {
		return false;
	}

	// Modifications données

	if(destinationIO->size() != _io.size()) {
#ifdef ISOARCHIVE_DEBUG
		qDebug() << "size iso !=" << destinationIO->sectorCount() << _io.sectorCount();
#endif
		// volume_space_size (taille totale de l'ISO)
		destinationIO->seekIso(SECTOR_SIZE_DATA * 16 + 80);// sector 16 : pos 80 size 4+4
		quint32 volume_space_size = destinationIO->size() / SECTOR_SIZE, volume_space_size2 = qToBigEndian(volume_space_size);
		destinationIO->write((char *)&volume_space_size, 4);
		destinationIO->write((char *)&volume_space_size2, 4);
	}

	// Update ISO files locations
	repairLocationSectors(directory, destination);

	//Debug

	/* qDebug() << "check headers";
	for(quint32 i = 0 ; i < destinationIO->sectorCount() ; ++i) {
		if(control && control->observerWasCanceled())	return false;
		destinationIO->seek(12 + i*SECTOR_SIZE);
		if(destinationIO->peek(3) != IsoArchiveIO::int2Header(i)) {
			qDebug() << "Error header" << i << destinationIO->peek(3).toHex();
			break;
		}
	}
	qDebug() << "check done"; */

#ifdef ISOARCHIVE_DEBUG
	if(destinationIO->size() % SECTOR_SIZE != 0)    qWarning() << "Invalid size" << destinationIO->size();
#endif

	return true;
}

bool IsoArchive::copySectors(IsoArchiveIO *out, qint64 sectorCount, ArchiveObserver *control, bool repair)
{
	if (sectorCount < 0) {
		qWarning() << "IsoArchive::copySectors sectorCount < 0" << sectorCount;
		setError(Archive::InvalidError);
		return false;
	}

	Q_ASSERT(out->pos() % SECTOR_SIZE == 0);
	Q_ASSERT(_io.pos() % SECTOR_SIZE == 0);

	for(int i = 0 ; i < sectorCount ; ++i) {
		if(control && control->observerWasCanceled()) {
			setError(Archive::AbortError);
			return false;
		}

		QByteArray data = _io.read(SECTOR_SIZE);
		if(data.size() != SECTOR_SIZE) {
			qWarning() << "IsoArchive::copySectors read error" << data.size() << SECTOR_SIZE;
			setError(Archive::ReadError, _io.errorString());
			return false;
		}

		if (!repair) {
			if(out->write(data) != SECTOR_SIZE) {
				qWarning() << "IsoArchive::copySectors write error" << data.size() << SECTOR_SIZE;
				setError(Archive::WriteError, out->errorString());
				return false;
			}
		} else {
			quint8 type, mode;
			IsoArchiveIO::headerInfos(data, &type, &mode);
			if(!out->writeSector(data.mid(SECTOR_SIZE_HEADER, SECTOR_SIZE_DATA), type, mode)) {
				qWarning() << "IsoArchive::copySectors writeSector error";
				setError(Archive::WriteError, out->errorString());
				return false;
			}
		}

		// Envoi de la position courante à l'output
		if(control) control->setObserverValue(out->currentSector());
	}

	return true;
}

bool IsoArchive::writeFile(QIODevice *in, quint32 sectorCount, ArchiveObserver *control)
{
	if(!in->isOpen() || !in->reset()) {
		setError(Archive::OpenError, in->errorString());
		return false;
	}

	qint64 remainingSize = in->size();
	const quint32 sectorStart = _io.currentSector();

	while(remainingSize > 0) {
		if(control && control->observerWasCanceled()) {
			setError(Archive::AbortError);
			return false;
		}

		QByteArray sectorData;
		qint64 toRead;
		quint8 type;
		if(remainingSize <= SECTOR_SIZE_DATA) { // Last sector of file
			toRead = remainingSize;
			type = 0x89;
		} else {
			toRead = SECTOR_SIZE_DATA;
			type = 0x08;
		}

		sectorData = in->read(toRead);
		if(sectorData.size() != toRead) {
			setError(Archive::ReadError, in->errorString());
			return false;
		}

		if(!_io.writeSector(sectorData, type)) {
			setError(Archive::WriteError, _io.errorString());
			return false;
		}

		remainingSize = in->size() - in->pos();
	}

	if(sectorCount != 0) {
		// Write empty sectors
		while(_io.currentSector() - sectorStart < sectorCount) {
			if(control && control->observerWasCanceled()) {
				setError(Archive::AbortError);
				return false;
			}

			if(!_io.writeSector(QByteArray(), 0x20)) {
				setError(Archive::WriteError, _io.errorString());
				return false;
			}
		}
	}

	return true;
}

void IsoArchive::repairLocationSectors(IsoDirectory *directory, IsoArchive *newIso)
{
	quint32 pos, oldSectorStart, newSectorStart, newSectorStart2, oldSize, newSize, newSize2;
	QList<IsoDirectory *> dirs;

	foreach(IsoFileOrDirectory *fileOrDir, directory->filesAndDirectories()) {
		if(fileOrDir->isModified()) {
			pos = fileOrDir->structPosition + 2;
			oldSectorStart = fileOrDir->location();
			oldSize = fileOrDir->size();
			newSectorStart = fileOrDir->newLocation();
			newSize = fileOrDir->newSize();

			// location_extent
			if(newSectorStart != oldSectorStart) {
				newSectorStart2 = qToBigEndian(newSectorStart);
				newIso->_io.seekIso(pos);
				newIso->_io.writeIso((char *)&newSectorStart, 4);
				newIso->_io.writeIso((char *)&newSectorStart2, 4);
//				qDebug() << "nouvelle position" << fileOrDir->name() << oldSectorStart << newSectorStart;
			}

			// data_length
			if(newSize != oldSize) {
				newSize2 = qToBigEndian(newSize);
				newIso->_io.seekIso(pos + 8);
				newIso->_io.writeIso((char *)&newSize, 4);
				newIso->_io.writeIso((char *)&newSize2, 4);
//				qDebug() << "nouvelle taille" << fileOrDir->name() << oldSize << newSize;
			}
		}

		if(fileOrDir->isDirectory()) {
			dirs.append(static_cast<IsoDirectory *>(fileOrDir));
		}
	}

	foreach(IsoDirectory *d, dirs) {
		repairLocationSectors(d, newIso);
	}
}

IsoDirectory *IsoArchive::rootDirectory() const
{
	return _rootDirectory;
}

/*const QList<PathTable> &IsoArchive::getPathTables1a() const
{
	return pathTables1a;
}

const QList<PathTable> &IsoArchive::getPathTables1b() const
{
	return pathTables1b;
}

const QList<PathTable> &IsoArchive::getPathTables2a() const
{
	return pathTables2a;
}

const QList<PathTable> &IsoArchive::getPathTables2b() const
{
	return pathTables2b;
}*/

bool IsoArchive::openVolumeDescriptor(quint8 num)
{
//	qDebug() << "sizeof(VolumeDescriptor)" << sizeof(VolumeDescriptor1) << sizeof(VolumeDescriptor2) << sizeof(IsoTime) << sizeof(DirectoryRecord);

	if(!_io.seek(SECTOR_SIZE * (16 + num) + SECTOR_SIZE_HEADER)) {
		qWarning() << "1";
		return false;
	}
//	qint64 pos = this->pos();
	if(_io.read((char *)&volume.vd1, sizeof(VolumeDescriptor1)) != sizeof(VolumeDescriptor1)) {
		qWarning() << "2" << _io.pos();
		return false;
	}
//	qint64 pos2 = this->pos();
	if(_io.read((char *)&volume.dr.length_dr, 1) != 1) {
		qWarning() << "3";
		return false;
	}
	if(_io.read((char *)&volume.dr.extended_attr_record_length, 1) != 1) {
		qWarning() << "4";
		return false;
	}
	if(_io.peek((char *)&volume.dr.drh, sizeof(DirectoryRecordHead)) != sizeof(DirectoryRecordHead)) {
		qWarning() << "5";
		return false;
	}
	if(!_io.seek(_io.pos() + 31)) {
		qWarning() << "6";
		return false;
	}
	volume.dr.name = _io.read(qMin((int)volume.dr.drh.length_fi, MAX_FILENAME_LENGTH));
	if(_io.pos() & 1) {
		if(!_io.seek(_io.pos() + 1)) {// padding
			qWarning() << "7";
			return false;
		}
	}
//	qDebug() << "sizeDirectoryRecord : " << (this->pos() - pos2);
	if(_io.read((char *)&volume.vd2, sizeof(VolumeDescriptor2)) != sizeof(VolumeDescriptor2)) {
		qWarning() << "8";
		return false;
	}

//	qDebug() << "size : " << (this->pos() - pos);
	return true;
}

bool IsoArchive::openRootDirectory(quint32 sector, quint32 dataSize)
{
	QList<quint32> dirVisisted;
	_rootDirectory = _openDirectoryRecord(new IsoDirectory(QString(), sector, dataSize, 0), dirVisisted);
	return _rootDirectory != NULL;
}

IsoDirectory *IsoArchive::_openDirectoryRecord(IsoDirectory *directories, QList<quint32> &dirVisisted)
{
	const quint32 sector = directories->location(), dataSize = directories->size();
	// anti-loop forever
	if(dirVisisted.contains(sector))	return directories;
	for(quint32 i=0 ; i<_io.sectorCountData(dataSize) ; ++i)
		dirVisisted.append(sector + i);

	if(!_io.seekToSector(sector)) {
		goto _openDirectoryRecordError;
	}

	{
	const quint32 maxPos = _io.posIso() + dataSize;

	while(_io.posIso() < maxPos) {
		DirectoryRecord dr;
		const qint64 beginPos = _io.posIso();

		if(_io.readIso((char *)&dr.length_dr, 1) != 1) {
			qWarning() << "IsoArchive::_openDirectoryRecord cannot read length_dr at" << beginPos;
			goto _openDirectoryRecordError;
		}

		if(dr.length_dr == 0) {
			// Next sector
			_io.seekToSector(_io.currentSector() + 1);
			continue;
		}

		if(_io.readIso((char *)&dr.extended_attr_record_length, 1) != 1) {
			qWarning() << "IsoArchive::_openDirectoryRecord cannot read extended_attr_record_length at" << beginPos;
			goto _openDirectoryRecordError;
		}
		if(_io.readIso((char *)&dr.drh, sizeof(DirectoryRecordHead)) != sizeof(DirectoryRecordHead)) {
			qWarning() << "IsoArchive::_openDirectoryRecord cannot read drh at" << beginPos;
			goto _openDirectoryRecordError;
		}

		if(dr.drh.length_fi > MAX_FILENAME_LENGTH) {
			qWarning() << "IsoArchive::_openDirectoryRecord length filename >" << MAX_FILENAME_LENGTH << dr.drh.length_fi;
			goto _openDirectoryRecordError;
		}

		if(!_io.seekIso(beginPos + 33)) {
			qWarning() << "IsoArchive::_openDirectoryRecord cannot seek to" << beginPos + 33;
			goto _openDirectoryRecordError;
		}

		dr.name = _io.readIso(dr.drh.length_fi);
		int index = dr.name.lastIndexOf(SEPARATOR_2);
		//dr.version = dr.name.mid(index+1);
		dr.name = dr.name.left(index);
		if(!_io.seekIso(beginPos + dr.length_dr)) {
			qWarning() << "IsoArchive::_openDirectoryRecord cannot seek to" << beginPos +  + dr.length_dr;
			goto _openDirectoryRecordError;
		}

//		qDebug() << directoryRecordToString(dr) << beginPos;

		if((dr.drh.file_flags >> 1) & 1) { // Directory
			directories->add(new IsoDirectory(dr.name, dr.drh.location_extent, dr.drh.data_length, beginPos));
		} else {
			directories->add(new IsoFile(dr.name, dr.drh.location_extent, dr.drh.data_length, beginPos, &_io));
		}
	}

	foreach(IsoDirectory *dir, directories->directories()) {
		if(!dir->isSpecial()) {
//			qDebug() << "IN DIR" << dir->name() << dir->location();
			if(!_openDirectoryRecord(dir, dirVisisted)) {
				qWarning() << "IsoArchive::_openDirectoryRecord cannot open directory" << dir->name() << dir->location();
				goto _openDirectoryRecordError;
			}
//			qDebug() << "OUT DIR" << dir->name();
		}
	}

	return directories;
	}
_openDirectoryRecordError:
	delete directories;
	return NULL;
}

/*QList<PathTable> IsoArchive::pathTable(quint32 sector, quint32 dataSize)
{
	QList<PathTable> pathTables;

	if(!_io.seekToSector(sector)) {
		return pathTables;
	}
	const quint32 maxPos = _io.posIso() + dataSize;

	while(_io.posIso() < maxPos) {
		PathTable pt;
		pt.position = _io.posIso();

		if(_io.readIso((char *)&pt.length_di, 1) != 1
				|| _io.readIso((char *)&pt.extended_attr_record_length, 1) != 1
				|| _io.readIso((char *)&pt.location_extent, 4) != 4
				|| _io.readIso((char *)&pt.parent_directory_number, 2) != 2) {
			break;
		}
		pt.name = _io.readIso(pt.length_di);
		if(pt.length_di & 1) {
			if(!_io.seekIso(_io.posIso() + 1)) {
				break;
			}
		}

		pathTables.append(pt);
	}

	return pathTables;
}*/

QByteArray IsoArchive::file(const QString &path, quint32 maxSize) const
{
	if (_rootDirectory == NULL) {
		return QByteArray();
	}
	IsoFile *file = _rootDirectory->file(path);
	if (file == NULL) {
		return QByteArray();
	}
	return file->data(maxSize);
}

QIODevice *IsoArchive::fileDevice(const QString &path) const
{
	if (_rootDirectory == NULL) {
		return NULL;
	}
	IsoFile *file = _rootDirectory->file(path);
	if (file == NULL) {
		return NULL;
	}
	return file->file();
}

QByteArray IsoArchive::modifiedFile(const QString &path, quint32 maxSize) const
{
	if (_rootDirectory == NULL) {
		return QByteArray();
	}
	IsoFile *file = _rootDirectory->file(path);
	if (file == NULL) {
		return QByteArray();
	}
	return file->modifiedData(maxSize);
}

QIODevice *IsoArchive::modifiedFileDevice(const QString &path) const
{
	if (_rootDirectory == NULL) {
		return NULL;
	}
	IsoFile *file = _rootDirectory->file(path);
	if (file == NULL) {
		return NULL;
	}
	return file->modifiedFile();
}

bool IsoArchive::extract(const QString &path, const QString &destination, quint32 maxSize) const
{
	if (_rootDirectory == NULL) {
		return false;
	}
	IsoFile *file = _rootDirectory->file(path);
	if (file == NULL) {
		return false;
	}
	return file->extract(destination, maxSize);
}

bool IsoArchive::extractDir(const QString &path, const QString &destination) const
{
	if (_rootDirectory == NULL) {
		return false;
	}
	IsoDirectory *dir = _rootDirectory->directory(path);
	if (dir == NULL) {
		return false;
	}
	QDir destDir(destination);
	bool error;

	foreach (IsoFile *file, dir->files()) {
		if (!file->extract(destDir.filePath(file->name()))) {
			error = true;
		}
	}

	return error;
}

void IsoArchive::extractAll(const QString &destination) const
{
	if (_rootDirectory == NULL) {
		return;
	}
	_extractAll(destination, _rootDirectory);
}

void IsoArchive::_extractAll(const QString &destination, IsoDirectory *directories, QString currentInternalDir) const
{
//	QTime t;t.start();

	QDir dir(destination);
	QString currentPath = dir.absolutePath().append('/');
//	qDebug() << currentPath;
	foreach(IsoFileOrDirectory *fileOrDir, directories->filesAndDirectories()) {
		QCoreApplication::processEvents();

		if(fileOrDir->isDirectory())// Directory
		{
			if(!fileOrDir->isSpecial()) {
				dir.mkdir(fileOrDir->name());
				_extractAll(currentPath + fileOrDir->name(), static_cast<IsoDirectory *>(fileOrDir), currentInternalDir.isEmpty() ? fileOrDir->name() : currentInternalDir + '/' + fileOrDir->name());
			}
		}
		else
		{
//			qDebug() << fileOrDir->name();
			extract(currentInternalDir.isEmpty() ? fileOrDir->name() : currentInternalDir + '/' + fileOrDir->name(), currentPath + fileOrDir->name());
		}
	}

//	qDebug() << "time" << t.elapsed();
}

qint32 IsoArchive::diffCountSectors(const QString &path, quint32 newSize) const
{
	if(_rootDirectory == NULL) {
		return false;
	}
	IsoFileOrDirectory *isoFile = _rootDirectory->fileOrDirectory(path);
	if(isoFile == NULL) {
		return false;
	}

	return _io.sectorCountData(newSize - isoFile->size());
}

//bool IsoArchive::insert(QString path, QString newISO, QByteArray data)
//{
//	QTime t;t.start();

//	IsoFile *isoFile = _rootDirectory->file(path);
//	if(isoFile == NULL) {
//		return false;
//	}

//	seekToFile(isoFile);

//	qDebug() << "time" << t.elapsed();

//	return true;
//}

bool IsoArchive::getIntegritySetPaddingAfter(IsoFileOrDirectory *prevFile, quint32 fileLocation)
{
	if(prevFile != NULL) {
		prevFile->setPaddingAfter(fileLocation - prevFile->locationAfter());

		if(prevFile->paddingAfter() > 0) {
			QByteArray sectorHeaderEmpty("\x00\x00\x20\x00\x00\x00\x20\x00", 8),
					sectorHeaderVoid(8, '\0');
			int sector;

			for (sector = 0 ; sector < prevFile->paddingAfter() ; ++sector) {
				QByteArray sectorHeaderPart = _io.sectorHeader(prevFile->locationAfter() + sector).mid(0x10, 8);
				if (sectorHeaderPart != sectorHeaderEmpty
						&& sectorHeaderPart != sectorHeaderVoid) {
					break;
				}
			}

#ifdef ISOARCHIVE_DEBUG
			if(sector > 0) {
				qDebug() << QString("%1 -> %2 (%3 sectors, %4 empty) : padding after %5 (%6 sectors)")
							.arg(prevFile->locationAfter())
							.arg(fileLocation)
							.arg(prevFile->paddingAfter())
							.arg(sector)
							.arg(prevFile->name())
							.arg(prevFile->sectorCount());
			}
#endif

			prevFile->setPaddingAfter(sector);

			return true;
		}
	}

	return false;
}

QList<IsoFileOrDirectory *> IsoArchive::getIntegrity()
{
	QMap<quint32, IsoFileOrDirectory *> files;
	QList<IsoFileOrDirectory *> filesWithPadding;

	_getIntegrity(files, _rootDirectory);

	IsoFileOrDirectory *prevFile = NULL;

	QMapIterator<quint32, IsoFileOrDirectory *> i(files);
	while (i.hasNext()) {
		i.next();
		IsoFileOrDirectory *file = i.value();

		if (getIntegritySetPaddingAfter(prevFile, file->location())) {
			filesWithPadding.append(prevFile);
		}

		prevFile = file;
	}

	if (prevFile && getIntegritySetPaddingAfter(prevFile, _io.sectorCount())) {
		filesWithPadding.append(prevFile);
	}

	return filesWithPadding;
}

void IsoArchive::_getIntegrity(QMap<quint32, IsoFileOrDirectory *> &files, IsoDirectory *directory) const
{
	foreach(IsoFileOrDirectory *fileOrDir, directory->filesAndDirectories()) {
		if(!fileOrDir->isSpecial()) {
			files.insert(fileOrDir->location(), fileOrDir);

			if(fileOrDir->isDirectory()) {
				_getIntegrity(files, static_cast<IsoDirectory *>(fileOrDir));
			}
		}
	}
}

QMap<quint32, IsoFile *> IsoArchive::getModifiedFiles(IsoDirectory *directory) const
{
	QMap<quint32, IsoFile *> ret;
	getModifiedFiles(ret, directory);
	return ret;
}

void IsoArchive::getModifiedFiles(QMap<quint32, IsoFile *> &files, IsoDirectory *directory) const
{
	foreach(IsoFileOrDirectory *fileOrDir, directory->filesAndDirectories()) {
		if(fileOrDir->isDirectory()) {
			getModifiedFiles(files, static_cast<IsoDirectory *>(fileOrDir));
		} else if(static_cast<IsoFile *>(fileOrDir)->isModified()) {
			files.insert(fileOrDir->newLocation(), static_cast<IsoFile *>(fileOrDir));
		}
	}
}

void IsoArchive::applyModifications(IsoDirectory *directory)
{
	foreach(IsoFileOrDirectory *fileOrDir, directory->filesAndDirectories()) {
		if(fileOrDir->isDirectory()) {
			applyModifications(static_cast<IsoDirectory *>(fileOrDir));
		}
		if(fileOrDir->isModified()) {
			fileOrDir->applyModifications();
		}
	}
}

/*!
 * Returns the last error status.
 * \sa unsetError(), errorString()
 */
Archive::ArchiveError IsoArchive::error() const
{
	return _error;
}

/*!
 * Sets the file's error type and text.
 * \sa error(), errorString()
 */
void IsoArchive::setError(Archive::ArchiveError error, const QString &errorString)
{
	_error = error;
	_io.setErrorString(errorString);
}

#ifdef ISOARCHIVE_DEBUG

QString IsoArchive::isoTimeToString(const IsoTime &time)
{
	return QString("%1/%2/%3 %4:%5:%6 (%7ms) [GMT%8]").arg(
				QByteArray((char *)time.day, 2),
				QByteArray((char *)time.month, 2),
				QByteArray((char *)time.year, 4),
				QByteArray((char *)time.hour, 2),
				QByteArray((char *)time.minute, 2),
				QByteArray((char *)time.second, 2),
				QByteArray((char *)time.millis, 2),
				QString("%1%2").arg(time.GMT >= 0 ? "+" : "").arg(time.GMT));
}

QString IsoArchive::volumeDescriptorToString(const VolumeDescriptor &vd)
{
	QString out;
	out.append("struct VolumeDescriptor {\n");
	out.append(QString("\t type = %1\n").arg(vd.vd1.type, 2, 16, QChar('0')));
	out.append(QString("\t id = %1\n").arg(QString(QByteArray((char *)vd.vd1.id, sizeof(vd.vd1.id)))));
	out.append(QString("\t version = %1\n").arg(vd.vd1.version));
	if(vd.vd1.type != 0xff) {
		out.append(QString("\t system_id = %1\n").arg(QString(QByteArray((char *)vd.vd1.system_id, sizeof(vd.vd1.system_id)))));
		out.append(QString("\t volume_id = %1\n").arg(QString(QByteArray((char *)vd.vd1.volume_id, sizeof(vd.vd1.volume_id)))));
		out.append(QString("\t volume_space_size = %1\n").arg(vd.vd1.volume_space_size));
		out.append(QString("\t volume_space_size2 = %1\n").arg(qFromBigEndian(vd.vd1.volume_space_size2)));
		out.append(QString("\t volume_set_size = %1\n").arg(vd.vd1.volume_set_size));
		out.append(QString("\t volume_set_size2 = %1\n").arg(qFromBigEndian(vd.vd1.volume_set_size2)));
		out.append(QString("\t volume_sequence_number = %1\n").arg(vd.vd1.volume_sequence_number));
		out.append(QString("\t volume_sequence_number2 = %1\n").arg(qFromBigEndian(vd.vd1.volume_sequence_number2)));
		out.append(QString("\t logical_block_size = %1\n").arg(vd.vd1.logical_block_size));
		out.append(QString("\t logical_block_size2 = %1\n").arg(qFromBigEndian(vd.vd1.logical_block_size2)));
		out.append(QString("\t path_table_size = %1\n").arg(vd.vd1.path_table_size));
		out.append(QString("\t path_table_size2 = %1\n").arg(qFromBigEndian(vd.vd1.path_table_size2)));
		out.append(QString("\t type_path_table = %1\n").arg(vd.vd1.type_path_table));
		out.append(QString("\t opt_type_path_table = %1\n").arg(vd.vd1.opt_type_path_table));
		out.append(QString("\t type_path_table2 = %1\n").arg(qFromBigEndian(vd.vd1.type_path_table2)));
		out.append(QString("\t opt_type_path_table2 = %1\n").arg(qFromBigEndian(vd.vd1.opt_type_path_table2)));
		out.append("\t ====================\n");
		out.append(directoryRecordToString(vd.dr));
		out.append("\t ====================\n");
		out.append(QString("\t volume_set_id = %1\n").arg(QString(QByteArray((char *)vd.vd2.volume_set_id, sizeof(vd.vd2.volume_set_id)))));
		out.append(QString("\t publisher_id = %1\n").arg(QString(QByteArray((char *)vd.vd2.publisher_id, sizeof(vd.vd2.publisher_id)))));
		out.append(QString("\t preparer_id = %1\n").arg(QString(QByteArray((char *)vd.vd2.preparer_id, sizeof(vd.vd2.preparer_id)))));
		out.append(QString("\t application_id = %1\n").arg(QString(QByteArray((char *)vd.vd2.application_id, sizeof(vd.vd2.application_id)))));
		out.append(QString("\t copyright_file_id = %1\n").arg(QString(QByteArray((char *)vd.vd2.copyright_file_id, sizeof(vd.vd2.copyright_file_id)))));
		out.append(QString("\t abstract_file_id = %1\n").arg(QString(QByteArray((char *)vd.vd2.abstract_file_id, sizeof(vd.vd2.abstract_file_id)))));
		out.append(QString("\t bibliographic_file_id = %1\n").arg(QString(QByteArray((char *)vd.vd2.bibliographic_file_id, sizeof(vd.vd2.bibliographic_file_id)))));
		out.append(QString("\t creation_date = %1\n").arg(isoTimeToString(vd.vd2.creation_date)));
		out.append(QString("\t file_structure_version = %1\n").arg(vd.vd2.file_structure_version));
		out.append(QString("\t application_data = %1\n").arg(QString(QByteArray((char *)vd.vd2.application_data, sizeof(vd.vd2.application_data)).toHex())));
	}
	out.append("}");

	return out;
}

QString IsoArchive::directoryRecordToString(const DirectoryRecord &dr)
{
	QString out;
	out.append("struct DirectoryRecord {\n");
	out.append(QString("\t length_dr = %1\n").arg(dr.length_dr));
	out.append(QString("\t extended_attr_record_length = %1\n").arg(dr.extended_attr_record_length));
	out.append(QString("\t location_extent = %1\n").arg(dr.drh.location_extent));
	out.append(QString("\t location_extent2 = %1\n").arg(qFromBigEndian(dr.drh.location_extent2)));
	out.append(QString("\t data_length = %1\n").arg(dr.drh.data_length));
	out.append(QString("\t data_length2 = %1\n").arg(qFromBigEndian(dr.drh.data_length2)));
	out.append(QString("\t %3/%2/%1 %4:%5:%6 [GMT%7]\n").arg(dr.drh.year, 2, 10, QChar('0')).arg(dr.drh.month, 2, 10, QChar('0')).arg(dr.drh.day, 2, 10, QChar('0')).arg(dr.drh.hour, 2, 10, QChar('0')).arg(dr.drh.minute, 2, 10, QChar('0')).arg(dr.drh.second, 2, 10, QChar('0')).arg(QString("%1%2").arg(dr.drh.GMT >= 0 ? "+" : "").arg(dr.drh.GMT)));
	out.append(QString("\t file_flags = %1\n").arg(dr.drh.file_flags, 8, 2, QChar('0')));
	out.append(QString("\t file_unit_size = %1\n").arg(dr.drh.file_unit_size));
	out.append(QString("\t interleave_grap_size = %1\n").arg(dr.drh.interleave_grap_size));
	out.append(QString("\t volume_sequence_number = %1\n").arg(dr.drh.volume_sequence_number));
	out.append(QString("\t volume_sequence_number2 = %1\n").arg(qFromBigEndian(dr.drh.volume_sequence_number2)));
	out.append(QString("\t length_fi = %1\n").arg(dr.drh.length_fi));
	out.append(QString("\t name = %1\n").arg(dr.name));
	out.append("}");
	return out;
}

QString IsoArchive::pathTableToString(const PathTable &pathTable, bool bigEndian)
{
	QString out;
	out.append("struct PathTable {\n");
	out.append(QString("\t length_di = %1\n").arg(pathTable.length_di));
	out.append(QString("\t extended_attr_record_length = %1\n").arg(pathTable.extended_attr_record_length));
	out.append(QString("\t location_extent = %1\n").arg(bigEndian ? qFromBigEndian(pathTable.location_extent) : pathTable.location_extent));
	out.append(QString("\t parent_directory_number = %1\n").arg(bigEndian ? qFromBigEndian(pathTable.parent_directory_number) : pathTable.parent_directory_number));
	out.append(QString("\t name = %1\n").arg(pathTable.name));
	out.append("}");
	return out;
}

#endif
