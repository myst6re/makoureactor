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

void IsoFileOrDirectory::setSize(quint32 size)
{
	_newSize = size;
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
	foreach(IsoFileOrDirectory *fOrD, _filesAndDirectories) {
		delete fOrD;
	}
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
			fs.append((IsoFile *)fOrD);
		}
	}

	return fs;
}

QList<IsoDirectory *> IsoDirectory::directories() const
{
	QList<IsoDirectory *> ds;

	foreach(IsoFileOrDirectory *fOrD, _filesAndDirectories) {
		if(fOrD->isDirectory()) {
			ds.append((IsoDirectory *)fOrD);
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
			return ((IsoDirectory *)fOrD)->fileOrDirectory(path.mid(index+1));
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

	return (IsoFile *)fOrD;
}

IsoDirectory *IsoDirectory::directory(const QString &path) const
{
	IsoFileOrDirectory *fOrD = fileOrDirectory(path);

	if(fOrD == NULL)	return NULL;

	if(fOrD->isFile()) {
		return NULL;
	}

	return (IsoDirectory *)fOrD;
}

void IsoDirectory::add(IsoFileOrDirectory *fileOrDirectory)
{
	_filesAndDirectories.insert(fileOrDirectory->name(), fileOrDirectory);
}

IsoFile::IsoFile(const QString &name, quint32 location, quint32 size, qint64 structPosition) :
	IsoFileOrDirectory(name, location, size, structPosition), dataChanged(false)
{
}

bool IsoFile::isDirectory() const
{
	return false;
}

const QByteArray &IsoFile::newData() const
{
	return _newData;
}

void IsoFile::setData(const QByteArray &data)
{
	_newData = data;
	_newSize = data.size();
	dataChanged = true;
}

bool IsoFile::isModified() const
{
	return dataChanged || IsoFileOrDirectory::isModified();
}

void IsoFile::applyModifications()
{
	_newData.clear();
	dataChanged = false;
	IsoFileOrDirectory::applyModifications();
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

	if(!_open()) {
		close();
		return false;
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
	qint64 start = (pos % SECTOR_SIZE) - 24;
	if(start < 0) {
		start = 0;
	} else if(start > SECTOR_SIZE_DATA) {
		start = SECTOR_SIZE_DATA;
	}

	return SECTOR_SIZE_DATA * (pos / SECTOR_SIZE) + start;
}

qint64 IsoArchiveIO::filePos(qint64 pos)
{
	return 24 + (pos / SECTOR_SIZE_DATA) * SECTOR_SIZE + pos % SECTOR_SIZE_DATA;
}

qint64 IsoArchiveIO::readIso(char *data, qint64 maxSize)
{
	qint64 read, readTotal = 0, seqLen;

//	qDebug() << pos() << "seek >> " << seqLen;
	if(!seekIso(isoPos(pos())))	return 0;

	seqLen = qMin(2072 - (pos() % SECTOR_SIZE), maxSize);
	if(seqLen < 0)	return 0;

	while((read = this->read(data, seqLen)) > 0) {
//		qDebug() << "read" << seqLen << read;
		data += read;
		maxSize -= read;
		readTotal += read;
		seqLen = qMin((qint64)SECTOR_SIZE_DATA, maxSize);
//		qDebug() << "seqLen" << seqLen << maxSize << pos();
		// Si on est à la fin du secteur
		if(pos() % SECTOR_SIZE >= 2072) {
			if(!seek(pos() + 304))	break;
//			qDebug() << "seek >> 304" << pos();
		}
	}

	return read < 0 ? read : readTotal;
}

QByteArray IsoArchiveIO::readIso(qint64 maxSize)
{
	char *data = new char[maxSize];

	QByteArray baData(data, readIso(data, maxSize));

	delete data;

	return baData;
}

qint64 IsoArchiveIO::writeIso(const char *data, qint64 maxSize)
{
	qint64 write, writeTotal = 0, seqLen;

//	qDebug() << pos() << "seek >> " << seqLen;
	if(!seekIso(isoPos(pos())))	return 0;

	seqLen = qMin(2072 - (pos() % SECTOR_SIZE), maxSize);

	while((write = this->write(data, seqLen)) > 0) {
//		qDebug() << "write" << seqLen << write;
		data += write;
		maxSize -= write;
		writeTotal += write;
		seqLen = qMin((qint64)2048, maxSize);
//		qDebug() << "seqLen" << seqLen << maxSize << pos();
		// Si on est à la fin du secteur
		if(pos() % SECTOR_SIZE >= 2072) {
			if(!seek(pos() + 304))	break;
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
	return read(24);
}

QByteArray IsoArchiveIO::sectorFooter(quint32 num)
{
	seek(SECTOR_SIZE * num + 2072);
	return read(280);
}

QByteArray IsoArchiveIO::sector(quint32 num, quint16 maxSize)
{
	seek(SECTOR_SIZE * num + 24);
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

IsoArchive::IsoArchive(const QString &name) :
	IsoArchiveIO(name), _rootDirectory(NULL)
{
}

IsoArchive::~IsoArchive()
{
	if(_rootDirectory != NULL)		delete _rootDirectory;
}

bool IsoArchive::_open()
{
	if(!openVolumeDescriptor()) {
		return false;
	}
//	qDebug() << volumeDescriptorToString(volume);
	qint64 size = QFile::size();
	if(size%SECTOR_SIZE != 0 || volume.vd1.volume_space_size != sectorCount() || volume.vd1.id[0] != 'C' || volume.vd1.id[1] != 'D' || volume.vd1.id[2] != '0' || volume.vd1.id[3] != '0' || volume.vd1.id[4] != '1') {
		qWarning() << (size%SECTOR_SIZE) << volume.vd1.volume_space_size << sectorCount();
		return false;
	}

	openRootDirectory(volume.dr.drh.location_extent, volume.dr.drh.data_length);
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

int IsoArchive::findPadding(const QList<IsoFileOrDirectory *> &orderedFileList, quint32 minSectorCount)
{
	int i=0;
	foreach(IsoFileOrDirectory *fileOrDir, orderedFileList) {
		if(fileOrDir->paddingAfter() >= minSectorCount) {
			return i;
		}
		++i;
	}

	return -0;
}

bool IsoArchive::pack(IsoArchive *destination, IsoControl *control, IsoDirectory *directory)
{
	QMap<quint32, IsoFile *> ordrePatchage, writeToTheMain;
	QList<IsoFile *> writeToTheEnd;
	qint64 secteur=0;
	quint32 endOfIso = sectorCount();
	IsoFileOrDirectory *fileWithPaddingAfter;
	int last_esti = control->baseEstimation, index;
	QList<IsoFileOrDirectory *> orderedFileList = getIntegrity();

	if(directory == NULL) {
		directory = _rootDirectory;
	}

	getModifiedFiles(ordrePatchage, directory);

	foreach(IsoFile *isoFile, ordrePatchage) {
		// Est-ce que les nouvelles données sont plus grandes que les anciennes ? Est-ce qu'on est pas à la fin de l'archive ?
		if(isoFile->newSectorCount() > isoFile->sectorCount() + isoFile->paddingAfter()
				&& isoFile->location() + isoFile->sectorCount() < sectorCount()) {
			if((index = findPadding(orderedFileList, isoFile->newSectorCount())) != -1) {
				fileWithPaddingAfter = orderedFileList.at(index);
				isoFile->setLocation(fileWithPaddingAfter->location() + fileWithPaddingAfter->sectorCount());
				isoFile->setPaddingAfter(fileWithPaddingAfter->paddingAfter() - isoFile->sectorCount());
				fileWithPaddingAfter->setPaddingAfter(0);
				orderedFileList.replace(index, isoFile);
				writeToTheMain.insert(isoFile->newLocation(), isoFile);
//				qDebug() << "Trouvé un espace suffisant à" << fileWithPaddingAfter->name() << (fileWithPaddingAfter->location() + fileWithPaddingAfter->sectorCount()) << "pour" << isoFile->name();
			} else {
				writeToTheEnd.append(isoFile);
				isoFile->setLocation(endOfIso);
				endOfIso += isoFile->newSectorCount();
			}
//			qDebug() << "Fichier trop gros" << isoFile->newSectorCount() << isoFile->sectorCount() << (isoFile->newSectorCount() - isoFile->sectorCount()) << isoFile->name();
		} else {
			if(isoFile->paddingAfter() > isoFile->newSectorCount() - isoFile->sectorCount())
				isoFile->setPaddingAfter(isoFile->paddingAfter() - (isoFile->newSectorCount() - isoFile->sectorCount()));
			writeToTheMain.insert(isoFile->newLocation(), isoFile);
		}
	}

//	qDebug() << "décompression...";

	control->estimation /= (double)endOfIso;

	reset();

	foreach(IsoFile *isoFile, writeToTheMain) {
		if(control->wasCanceled())	return false;

		secteur = isoFile->newLocation();

		// Données avant le fichier
		last_esti = copyBytes(destination, SECTOR_SIZE*secteur - pos(), last_esti, control);
		if(last_esti == -1)			return false;

		// Debug
		if(destination->pos()%SECTOR_SIZE != 0)			qWarning() << "destination error 1a" << (destination->pos()%SECTOR_SIZE) << isoFile->name();
		if(destination->pos()/SECTOR_SIZE != secteur)	qWarning() << "destination error 1b" << (destination->pos()/SECTOR_SIZE) << secteur << isoFile->name();
		if(destination->pos() != pos())					qWarning() << "destination error 1c" << destination->pos() << pos() << isoFile->name();

		// On écrit le même nombre de secteurs que le fichier d'origine
//		qDebug() << "écriture de" << isoFile->name() << "(" << isoFile->sectorCount() << "+" << isoFile->paddingAfter() << "secteurs) dans isoTemp au secteur" << secteur;
		secteur = writeSectors(isoFile->newData(), destination, secteur, control, isoFile->sectorCount() + isoFile->paddingAfter());
		if(secteur == -1)			return false;
		seek(destination->pos());

		// Debug
		if(destination->pos()%SECTOR_SIZE != 0)			qWarning() << "destination error 2a" << (destination->pos()%SECTOR_SIZE) << isoFile->name();
		if(destination->pos()/SECTOR_SIZE != secteur)	qWarning() << "destination error 2b" << (destination->pos()/SECTOR_SIZE) << secteur << isoFile->name();
		if(destination->pos() != pos())					qWarning() << "destination error 2c" << destination->pos() << pos() << isoFile->name();

		// Envoi de la position courante à l'output
		if(last_esti != (int)(control->baseEstimation + (destination->pos()/SECTOR_SIZE)*control->estimation)) {
			last_esti = control->baseEstimation + (destination->pos()/SECTOR_SIZE)*control->estimation;
			control->setIsoOut(last_esti);
		}
	}

	// Données après les fichiers patchés

	if(-1 == copyBytes(destination, size() - pos(), last_esti, control))
		return false;

	// Debug
	if(destination->pos()%SECTOR_SIZE != 0)				qWarning() << "destination error 3a" << (destination->pos()%SECTOR_SIZE);
	if(destination->pos()/SECTOR_SIZE != sectorCount())	qWarning() << "destination error 3b" << (destination->pos()/SECTOR_SIZE) << secteur;
	if(destination->pos() != pos())						qWarning() << "destination error 3c" << destination->pos() << pos();

	// A la fin de l'ISO

	// Fichiers trop gros mis à la fin de l'ISO
	secteur = destination->pos()/SECTOR_SIZE;

	foreach(IsoFile *isoFile, writeToTheEnd) {
//		qDebug() << "écriture de" << isoFile->name() << "(" << isoFile->sectorCount() << "secteurs) dans isoTemp (à la fin) au secteur" << secteur;
		secteur = writeSectors(isoFile->newData(), destination, secteur, control);
		if(secteur == -1)	return false;

		// Envoi de la position courante à l'output
		if(last_esti != (int)(control->baseEstimation + (destination->pos()/SECTOR_SIZE)*control->estimation)) {
			last_esti = control->baseEstimation + (destination->pos()/SECTOR_SIZE)*control->estimation;
			control->setIsoOut(last_esti);
		}
	}

	// Modifications données

	if(destination->size() != size()) {
		// volume_space_size (taille totale de l'ISO)
		destination->seekIso(SECTOR_SIZE_DATA * 16 + 80);// sector 16 : pos 80 size 4+4
		quint32 volume_space_size = destination->size()/SECTOR_SIZE, volume_space_size2 = qToBigEndian(volume_space_size);
		destination->write((char *)&volume_space_size, 4);
		destination->write((char *)&volume_space_size2, 4);
	}

	// Update ISO files locations
	repairLocationSectors(directory, destination);

	//Debug

//	qDebug() << "vérification headers";
//	int newSectorCount = destination->size()/SECTOR_SIZE;
//	for(int i=0 ; i<newSectorCount ; ++i) {
//		if(control->wasCanceled())	return false;
//		destination->seek(12 + i*SECTOR_SIZE);
//		if(destination->read(3) != IsoArchive::int2Header(i)) {
//			destination->seek(12 + i*SECTOR_SIZE);
//			qDebug() << "Erreur header" << i << destination->read(3).toHex();
//			break;
//		}
//	}
//	qDebug() << "vérification done";

	if(destination->size()%SECTOR_SIZE != 0)	qWarning() << "Invalid size" << destination->size();
//	if(destination->size() != size())			qWarning() << "Taille différente" << (destination->size()/SECTOR_SIZE) << (size()/SECTOR_SIZE);

	return true;
}


qint64 IsoArchive::writeSectors(const QByteArray &data, IsoArchive *isoTemp, quint32 secteur, IsoControl *control, quint32 sectorCount)
{
	int size = data.size();
	const char *constData = data.constData();
	QByteArray sector;
	bool continueWriteField = size > 0;
	bool padding = sectorCount != 0;

	if(padding) {
		sectorCount += secteur;
	}

	while(continueWriteField) {
		if(control->wasCanceled())	return -1;

		if(size <= SECTOR_SIZE_DATA) {
			// sector header
			sector = buildHeader(secteur, 0x89);
			// data
			if(size == SECTOR_SIZE_DATA) {
				sector.append(constData, size);
			} else {
				sector.append(constData, size).append(QByteArray(SECTOR_SIZE_DATA - size, '\x00'));
			}

			continueWriteField = false;
		} else {
			// sector header + data + sector footer
			sector = buildHeader(secteur, 0x08).append(constData, SECTOR_SIZE_DATA);

			size -= SECTOR_SIZE_DATA;
			constData += SECTOR_SIZE_DATA;
			continueWriteField = true;
		}
		// sector footer
		isoTemp->write(sector.append(secteur < this->sectorCount() ? sectorFooter(secteur) : buildFooter(secteur)));

		secteur++;
	}

	if(padding) {
		while(secteur < sectorCount) {
			if(control->wasCanceled())	return -1;
			// sector header + empty data + empty footer
			isoTemp->write(buildHeader(secteur, 0x20).append(QByteArray(2328, '\x00')));

			secteur++;
		}
	}

	return secteur;
}

int IsoArchive::copyBytes(IsoArchive *isoTemp, int size, int last_esti, IsoControl *control)
{
	int count = size/READ_MAX, i;

//	qDebug() << "écriture de" << size << "octets (" << (size/SECTOR_SIZE) << "secteurs) dans isoTemp";

	for(i=0 ; i<count ; ++i) {
		if(control->wasCanceled())	return -1;
		isoTemp->write(read(READ_MAX));

		// Envoi de la position courante à l'output
		if(last_esti != (int)(control->baseEstimation + (isoTemp->pos()/SECTOR_SIZE)*control->estimation)) {
			last_esti = control->baseEstimation + (isoTemp->pos()/SECTOR_SIZE)*control->estimation;
			control->setIsoOut(last_esti);
		}
	}
	if(size % READ_MAX != 0) {
		isoTemp->write(read(size % READ_MAX));
	}

	return last_esti;
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
				newIso->seekIso(pos);
				newIso->writeIso((char *)&newSectorStart, 4);
				newIso->writeIso((char *)&newSectorStart2, 4);
//				qDebug() << "nouvelle position" << fileOrDir->name() << oldSectorStart << newSectorStart;
			}

			// data_length
			if(newSize != oldSize) {
				newSize2 = qToBigEndian(newSize);
				newIso->seekIso(pos + 8);
				newIso->writeIso((char *)&newSize, 4);
				newIso->writeIso((char *)&newSize2, 4);
//				qDebug() << "nouvelle taille" << fileOrDir->name() << oldSize << newSize;
			}
		}

		if(fileOrDir->isDirectory()) {
			dirs.append((IsoDirectory *)fileOrDir);
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

	if(!seek(SECTOR_SIZE * (16 + num) + 24)) {
		return false;
	}
//	qint64 pos = this->pos();
	if(read((char *)&volume.vd1, sizeof(VolumeDescriptor1)) != sizeof(VolumeDescriptor1)) {
		return false;
	}
//	qint64 pos2 = this->pos();
	if(read((char *)&volume.dr.length_dr, 1) != 1) {
		return false;
	}
	if(read((char *)&volume.dr.extended_attr_record_length, 1) != 1) {
		return false;
	}
	if(peek((char *)&volume.dr.drh, sizeof(DirectoryRecordHead)) != sizeof(DirectoryRecordHead)) {
		return false;
	}
	if(!seek(this->pos() + 31)) {
		return false;
	}
	volume.dr.name = read(qMin((int)volume.dr.drh.length_fi, MAX_FILENAME_LENGTH));
	if(this->pos() & 1) {
		if(!seek(this->pos() + 1)) {// padding
			return false;
		}
	}
//	qDebug() << "sizeDirectoryRecord : " << (this->pos() - pos2);
	if(read((char *)&volume.vd2, sizeof(VolumeDescriptor2)) != sizeof(VolumeDescriptor2)) {
		return false;
	}

//	qDebug() << "size : " << (this->pos() - pos);
	return true;
}

void IsoArchive::openRootDirectory(quint32 sector, quint32 dataSize)
{
	QList<quint32> dirVisisted;
	_rootDirectory = _openDirectoryRecord(new IsoDirectory(QString(), sector, dataSize, 0), dirVisisted);
}

IsoDirectory *IsoArchive::_openDirectoryRecord(IsoDirectory *directories, QList<quint32> &dirVisisted)
{
	const quint32 sector = directories->location(), dataSize = directories->size();
	// anti-loop forever
	if(dirVisisted.contains(sector))	return directories;
	for(quint32 i=0 ; i<sectorCountData(dataSize) ; ++i)
		dirVisisted.append(sector + i);

	if(!seekToSector(sector)) {
		goto _openDirectoryRecordError;
	}

	{
	const quint32 maxPos = posIso() + dataSize;

	while(posIso() < maxPos) {
		DirectoryRecord dr;
		const qint64 beginPos = posIso();

		if(readIso((char *)&dr.length_dr, 1) != 1) {
			goto _openDirectoryRecordError;
		}

		if(dr.length_dr == 0) {
			// Next sector
			seekToSector(currentSector() + 1);
			continue;
		}

		if(readIso((char *)&dr.extended_attr_record_length, 1) != 1) {
			goto _openDirectoryRecordError;
		}
		if(readIso((char *)&dr.drh, sizeof(DirectoryRecordHead)) != sizeof(DirectoryRecordHead)) {
			goto _openDirectoryRecordError;
		}

		if(dr.drh.length_fi > MAX_FILENAME_LENGTH)	break;

		if(!seekIso(beginPos + 33)) {
			goto _openDirectoryRecordError;
		}

		dr.name = readIso(dr.drh.length_fi);
		int index = dr.name.lastIndexOf(SEPARATOR_2);
		//dr.version = dr.name.mid(index+1);
		dr.name = dr.name.left(index);
		if(!seekIso(beginPos + dr.length_dr)) {
			goto _openDirectoryRecordError;
		}

//		qDebug() << directoryRecordToString(dr) << beginPos;

		if((dr.drh.file_flags >> 1) & 1) { // Directory
			directories->add(new IsoDirectory(dr.name, dr.drh.location_extent, dr.drh.data_length, beginPos));
		} else {
			directories->add(new IsoFile(dr.name, dr.drh.location_extent, dr.drh.data_length, beginPos));
		}
	}

	foreach(IsoDirectory *dir, directories->directories()) {
		if(!dir->isSpecial()) {
//			qDebug() << "IN DIR" << dir->name() << dir->location();
			if(!_openDirectoryRecord(dir, dirVisisted)) {
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

QList<PathTable> IsoArchive::pathTable(quint32 sector, quint32 dataSize)
{
	QList<PathTable> pathTables;
	
	if(!seekToSector(sector)) {
		return pathTables;
	}
	const quint32 maxPos = posIso() + dataSize;

	while(posIso() < maxPos) {
		PathTable pt;
		pt.position = posIso();

		if(readIso((char *)&pt.length_di, 1) != 1
				|| readIso((char *)&pt.extended_attr_record_length, 1) != 1
				|| readIso((char *)&pt.location_extent, 4) != 4
				|| readIso((char *)&pt.parent_directory_number, 2) != 2) {
			break;
		}
		pt.name = readIso(pt.length_di);
		if(pt.length_di & 1) {
			if(!seekIso(posIso() + 1)) {
				break;
			}
		}
		
		pathTables.append(pt);
	}
	
	return pathTables;
}

QByteArray IsoArchive::file(const QString &path, quint32 maxSize)
{
	return file(_rootDirectory->file(path), maxSize);
}

QByteArray IsoArchive::file(IsoFile *isoFile, quint32 maxSize)
{
	if(isoFile == NULL)		return QByteArray();

	seekToFile(isoFile);

	return readIso(maxSize == 0 ? isoFile->size() : qMin(maxSize, isoFile->size()));
}

bool IsoArchive::extract(const QString &path, const QString &destination, quint32 maxSize)
{
	return extract(_rootDirectory->file(path), destination, maxSize);
}

bool IsoArchive::extract(IsoFile *isoFile, const QString &destination, quint32 maxSize)
{
	if(isoFile == NULL)		return false;

//	QTime t;t.start();

	maxSize = maxSize == 0 ? isoFile->size() : qMin(maxSize, isoFile->size());
	char data[MAX_ISO_READ];
	qint64 r, totalR=0;
	QFile ret(destination);
	if(!ret.open(QIODevice::WriteOnly))		return false;

	seekToFile(isoFile);

	while(maxSize-totalR > 0 && (r = readIso(data, qMin((qint64)MAX_ISO_READ, maxSize-totalR))) > 0) {
		ret.write(data, r);
		totalR += r;
	}

//	qDebug() << "time" << t.elapsed();

	return true;
}

void IsoArchive::extractAll(const QString &destination)
{
	_extractAll(destination, _rootDirectory);
}

void IsoArchive::_extractAll(const QString &destination, IsoDirectory *directories, QString currentInternalDir)
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
				_extractAll(currentPath + fileOrDir->name(), (IsoDirectory *)fileOrDir, currentInternalDir.isEmpty() ? fileOrDir->name() : currentInternalDir + '/' + fileOrDir->name());
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

bool IsoArchive::seekToFile(const QString &path)
{
	return seekToFile(_rootDirectory->fileOrDirectory(path));
}

bool IsoArchive::seekToFile(IsoFileOrDirectory *isoFile)
{
	if(isoFile == NULL)		return false;
	return seekToSector(isoFile->location());
}

qint32 IsoArchive::diffCountSectors(const QString &path, quint32 newSize) const
{
	IsoFileOrDirectory *isoFile = _rootDirectory->fileOrDirectory(path);
	if(isoFile == NULL) {
		return false;
	}

	return sectorCountData(newSize - isoFile->size());
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

QList<IsoFileOrDirectory *> IsoArchive::getIntegrity() const
{
	QMap<quint32, IsoFileOrDirectory *> files;
	QList<IsoFileOrDirectory *> filesWithPadding;

	_getIntegrity(files, _rootDirectory);

	quint32 cur=0;
	IsoFileOrDirectory *prevFile = NULL;

	QMapIterator<quint32, IsoFileOrDirectory *> i(files);
	while(i.hasNext()) {
		i.next();
		IsoFileOrDirectory *file = i.value();

		if(prevFile != NULL) {
			prevFile->setPaddingAfter(i.key() - (prevFile->location() + prevFile->sectorCount()));
			if(prevFile->paddingAfter() > 0) {
				filesWithPadding.append(prevFile);
			}
		}

        if(i.key() != cur && prevFile != NULL) {
            qDebug() << QString("%1 -> %2 (%3 sectors) : padding after %4 (%5 sectors)").arg(prevFile->location() + prevFile->sectorCount())
                    .arg(i.key()).arg(i.key() - (prevFile->location() + prevFile->sectorCount())).arg(prevFile->name()).arg(prevFile->sectorCount());
        }

		cur = i.key() + file->sectorCount();
		prevFile = file;
	}

	if(prevFile != NULL) {
		prevFile->setPaddingAfter(sectorCount() - (prevFile->location() + prevFile->sectorCount()));
        qDebug() << QString("%1 -> %2 (%3 sectors) : padding after %4 (%5 sectors)").arg(prevFile->location() + prevFile->sectorCount())
                    .arg(sectorCount()).arg(sectorCount() - (prevFile->location() + prevFile->sectorCount()))
                    .arg(prevFile->name()).arg(prevFile->sectorCount());
		if(prevFile->paddingAfter() > 0) {
			filesWithPadding.append(prevFile);
		}
	}

	return filesWithPadding;
}

void IsoArchive::_getIntegrity(QMap<quint32, IsoFileOrDirectory *> &files, IsoDirectory *directory) const
{
	foreach(IsoFileOrDirectory *fileOrDir, directory->filesAndDirectories()) {
		if(!fileOrDir->isSpecial()) {
			files.insert(fileOrDir->location(), fileOrDir);

			if(fileOrDir->isDirectory()) {
				_getIntegrity(files, (IsoDirectory *)fileOrDir);
			}
		}
	}
}

void IsoArchive::getModifiedFiles(QMap<quint32, IsoFile *> &files, IsoDirectory *directory) const
{
	foreach(IsoFileOrDirectory *fileOrDir, directory->filesAndDirectories()) {
		if(fileOrDir->isDirectory()) {
			getModifiedFiles(files, (IsoDirectory *)fileOrDir);
		} else if(((IsoFile *)fileOrDir)->isModified()) {
			files.insert(fileOrDir->newLocation(), (IsoFile *)fileOrDir);
		}
	}
}

void IsoArchive::applyModifications(IsoDirectory *directory)
{
	foreach(IsoFileOrDirectory *fileOrDir, directory->filesAndDirectories()) {
		if(fileOrDir->isDirectory()) {
			applyModifications((IsoDirectory *)fileOrDir);
		}
		if(fileOrDir->isModified()) {
			fileOrDir->applyModifications();
		}
	}
}
