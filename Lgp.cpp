/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel J�r�me <myst6re@gmail.com>
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
/*
 * This file may contains some code (especially the conflict part)
 * inspired from LGP/UnLGP tool written by Aali.
 * http://forums.qhimm.com/index.php?topic=8641.0
 */
#include "Lgp.h"
#include "Lgp_p.h"

LgpToc::LgpToc()
{
}

LgpToc::LgpToc(const LgpToc &other)
{
	for(int i=0; i<LOOKUP_TABLE_ENTRIES; ++i) {
		foreach(LgpHeaderEntry *headerEntry, other.entries(i)) {
			addEntry(new LgpHeaderEntry(*headerEntry));
		}
	}
}

LgpToc::~LgpToc()
{
	foreach(LgpHeaderEntry *entry, _header) {
		delete entry;
	}
}

bool LgpToc::addEntry(LgpHeaderEntry *entry)
{
	qint32 v = lookupValue(entry->fileName());
	if(v < 0) {
		return false;
	}

	_header.insert(v, entry);

	return true;
}

LgpHeaderEntry *LgpToc::entry(const QString &filePath) const
{
	qint32 v = lookupValue(filePath);
	if(v < 0) {
		return NULL; // invalid file name
	}

	return entry(filePath, v);
}

QList<LgpHeaderEntry *> LgpToc::entries(quint16 id) const
{
	return _header.values(id);
}

bool LgpToc::hasEntries(quint16 id) const
{
	return _header.contains(id);
}

LgpHeaderEntry *LgpToc::entry(const QString &filePath, quint16 id) const
{
	foreach(LgpHeaderEntry *entry, entries(id)) {
		if(filePath.compare(entry->filePath(), Qt::CaseInsensitive) == 0) {
			return entry;
		}
	}

	return NULL; // file not found
}

bool LgpToc::removeEntry(const QString &filePath)
{
	qint32 v = lookupValue(filePath);
	if(v < 0) {
		return false; // invalid file name
	}

	LgpHeaderEntry *e = entry(filePath, v);
	if(e == NULL) {
		return false; // file not found
	}

	delete e;

	return _header.remove(v, e) > 0;
}

bool LgpToc::contains(const QString &filePath) const
{
	return entry(filePath) != NULL;
}

void LgpToc::clear()
{
	foreach(LgpHeaderEntry *entry, _header) {
		delete entry;
	}

	_header.clear();
}

bool LgpToc::isEmpty() const
{
	return _header.isEmpty();
}

int LgpToc::size() const
{
	return _header.size();
}

QList<const LgpHeaderEntry *> LgpToc::filesSortedByPosition() const
{
	QMultiMap<quint32, const LgpHeaderEntry *> ret;

	foreach(const LgpHeaderEntry *entry, _header) {
		ret.insert(entry->filePosition(), entry);
	}

	return ret.values();
}

LgpToc &LgpToc::operator=(const LgpToc &other)
{
	if(this != &other) {
		clear();
		for(int i=0; i<LOOKUP_TABLE_ENTRIES; ++i) {
			foreach(LgpHeaderEntry *headerEntry, other.entries(i)) {
				addEntry(new LgpHeaderEntry(*headerEntry));
			}
		}
	}

	return *this;
}

qint32 LgpToc::lookupValue(const QString &filePath)
{
	int index = filePath.lastIndexOf('/');

	if(index != -1) {
		index++;
	} else {
		index = 0;
	}

	if(filePath.size() < index + 2) {
		return -1;
	}

	char c1 = lookupValue(filePath.at(index));

	if(c1 > LOOKUP_VALUE_MAX) {
		return -1;
	}

	char c2 = lookupValue(filePath.at(index + 1));

	if(c1 > LOOKUP_VALUE_MAX) {
		return -1;
	}

	return c1 * LOOKUP_VALUE_MAX + c2 + 1;
}

quint8 LgpToc::lookupValue(const QChar &qc)
{
	char c = qc.toLower().toLatin1();

	if(c == '.') {
		return 255;
	}

	if(c >= '0' && c <= '9') {
		c += 'a' - '0';
	}

	if(c == '_') c = 'k';
	if(c == '-') c = 'l';

	return c - 'a';
}

LgpIterator LgpToc::iterator() const
{
	return LgpIterator(_header);
}

LgpIterator::LgpIterator(const QMultiHash<quint16, LgpHeaderEntry *> &header) :
	_header(header), it(header)
{
}

bool LgpIterator::hasNext() const
{
	return it.hasNext();
}

bool LgpIterator::hasPrevious() const
{
	return it.hasPrevious();
}

void LgpIterator::next()
{
	it.next();
}

void LgpIterator::previous()
{
	it.previous();
}

void LgpIterator::toBack()
{
	it.toBack();
}

void LgpIterator::toFront()
{
	it.toFront();
}

QIODevice *LgpIterator::file() const
{
	return it.value()->file();
}

QIODevice *LgpIterator::modifiedFile() const
{
	return it.value()->modifiedFile();
}

const QString &LgpIterator::fileName() const
{
	return it.value()->fileName();
}

Lgp::Lgp() :
	_error(NoError)
{
}

/*!
 * Constructs a new lgp archive object to represent the lgp archive with the given \a name.
 */
Lgp::Lgp(const QString &name) :
	_error(NoError)
{
	_file.setFileName(name);
}

/*!
 * Destroys the lgp archive object, closing it if necessary.
 */
Lgp::~Lgp()
{
	if(_file.isOpen()) {
		_file.close();
	}
	_file.deleteLater();
}

void Lgp::clear()
{
	_companyName.clear();
	_files.clear();
	_productName.clear();
}

/*!
 * Returns a list of file paths sorted by file position.
 */
QStringList Lgp::fileList()
{
	QStringList ret;

	if(_files.isEmpty() && !openHeader()) {
		return ret;
	}

	foreach(const LgpHeaderEntry *entry, _files.filesSortedByPosition()) {
		ret.append(entry->filePath());
	}

	return ret;
}

/*!
 * Returns a list of file names (only files in \a dirPath) sorted by file position.
 */
QStringList Lgp::fileListInDir(const QString &dirPath)
{
	QStringList ret;

	if(_files.isEmpty() && !openHeader()) {
		return ret;
	}

	foreach(const LgpHeaderEntry *entry, _files.filesSortedByPosition()) {
		if(entry->fileDir().compare(dirPath, Qt::CaseInsensitive) == 0) {
			ret.append(entry->fileName());
		}
	}

	return ret;
}

/*!
 * Returns a list of dir names (only dirs in \a dirPath).
 */
QStringList Lgp::dirListInDir(const QString &dirPath)
{
	QStringList ret;

	if(_files.isEmpty() && !openHeader()) {
		return ret;
	}

	foreach(const LgpHeaderEntry *entry, _files.filesSortedByPosition()) {
		if(entry->fileDir().startsWith(dirPath, Qt::CaseInsensitive)
				&& entry->fileDir().compare(dirPath, Qt::CaseInsensitive) != 0) {
			ret.append(entry->fileDir().mid(dirPath.size()));
		}
	}

	return ret;
}

/*LgpIterator Lgp::iterator() const
{
	return _files.iterator();
}*///TODO

/*!
 * Returns true if the file named \a filePath exists; otherwise
 * false.
 */
bool Lgp::fileExists(const QString &filePath)
{
	return headerEntry(filePath) != NULL;// need to open the header
}

/*!
 * Returns the data for the file named \a filePath.
 * \sa fileData(), modifiedFile(), modifiedFileData()
 */
QIODevice *Lgp::file(const QString &filePath)
{
	LgpHeaderEntry *entry = headerEntry(filePath);// need to open the header
	if(entry == NULL) return NULL;

	QIODevice *io = entry->file();
	if(io != NULL) {
		io->close();
		return io;
	} else {
		if(!_file.seek(entry->filePosition())) {
			return NULL;
		}
		QByteArray name = _file.read(20);
		if(name.size() != 20) {
			return NULL;
		}
		quint32 size;
		if(_file.read((char *)&size, 4) != 4) {
			return NULL;
		}

		entry->setFileSize(size);
		entry->setFileName(name);
		io = new LgpIO(&_file, entry);
		entry->setFile(io);
		return io;
	}
}

/*!
 * Returns the data for the file named \a filePath.
 * \sa file(), modifiedFile(), modifiedFileData()
 */
QByteArray Lgp::fileData(const QString &filePath)
{
	QIODevice *io = file(filePath);
	if(io == NULL || !io->open(QIODevice::ReadOnly)) {
		qWarning() << "fileData error";
		return QByteArray();
	}
	QByteArray data = io->readAll();
	io->close();
	return data;
}

/*!
 * Returns the data, modified by setData if modified, for the file named \a filePath.
 * \sa file(), fileData(), modifiedFileData()
 */
QIODevice *Lgp::modifiedFile(const QString &filePath)
{
	LgpHeaderEntry *entry = headerEntry(filePath);// need to open the header
	if(entry == NULL) return NULL;

	QIODevice *io = entry->modifiedFile();
	if(io != NULL) {
		io->close();
		return io;
	} else {
		return file(filePath);
	}
}

/*!
 * Returns the data, modified by setData if modified, for the file named \a filePath.
 * \sa file(), fileData(), modifiedFile()
 */
QByteArray Lgp::modifiedFileData(const QString &filePath)
{
	QIODevice *io = modifiedFile(filePath);
	if(io == NULL || !io->open(QIODevice::ReadOnly)) {
		qWarning() << "fileData error";
		return QByteArray();
	}
	QByteArray data = io->readAll();
	io->close();
	return data;
}

/*!
 * Change the \a data for the file named \a filePath.
 * Returns false if the file doesn't exists; otherwise
 * returns true.
 * \sa setFile()
 */
bool Lgp::setFile(const QString &filePath, QIODevice *data)
{
	LgpHeaderEntry *entry = headerEntry(filePath);// need to open the header
	if(entry == NULL) return false;

	entry->setModifiedFile(data);

	return true;
}

/*!
 * \overload
 *
 * Change the \a data for the file named \a filePath.
 * Returns false if the file doesn't exists; otherwise
 * returns true.
 * \sa setFile()
 */
bool Lgp::setFile(const QString &filePath, const QByteArray &data)
{
	QBuffer *buf = new QBuffer();
	buf->setData(data);
	return setFile(filePath, buf);
}

/*!
 * Add a new file named \a filePath with \a data.
 * Returns false if the file exists; otherwise returns
 * true.
 * \sa addFile()
 */
bool Lgp::addFile(const QString &filePath, QIODevice *data)
{
	LgpHeaderEntry *entry = headerEntry(filePath);// need to open the header
	if(entry != NULL) return false;

	entry = new LgpHeaderEntry(filePath, _file.size());
	entry->setModifiedFile(data);

	bool ret = _files.addEntry(entry);

	if(!ret) {
		delete entry;
	}

	return ret;
}

/*!
 * \overload
 *
 * Add a new file named \a filePath with \a data.
 * Returns false if the file exists; otherwise returns
 * true.
 * \sa addFile()
 */
bool Lgp::addFile(const QString &filePath, const QByteArray &data)
{
	QBuffer *buf = new QBuffer();
	buf->setData(data);
	return addFile(filePath, buf);
}

/*!
 * Remove the file named \a filePath.
 * Returns false if the file is successfully removed.
 */
bool Lgp::removeFile(const QString &filePath)
{
	return _files.removeEntry(filePath);
}

/*!
 * Returns the company name (like "SQUARESOFT") or
 * a null string if there is an error.
 * \sa setCompanyName()
 */
const QString &Lgp::companyName()
{
	if(_companyName.isNull()) {
		if(!isOpen()) {
			qWarning() << "Lgp::companyName: The device is not open for reading";
			return _companyName;
		}

		_file.reset();
		QByteArray companyData = _file.read(12);
		const char *data = companyData.constData();
		const char *last = data + 12;
		while(*data == '\0' && data < last) {
			data++;
		}
		_companyName = QString(QByteArray(data, (int)(last - data)));
	}
	return _companyName;
}

/*!
 * Change the company name.
 * \sa companyName()
 */
void Lgp::setCompanyName(const QString &companyName)
{
	_companyName = companyName;
}

/*!
 * Returns the product name (like "FINAL FANTASY7") or
 * a null string if there is an error.
 * \sa setProductName()
 */
const QString &Lgp::productName()
{
	if(_productName.isNull()) {
		if(!isOpen()) {
			qWarning() << "Lgp::productName: The device is not open for reading";
			return _companyName;
		}

		_file.seek(_file.size() - 14);
		_productName = _file.read(14);
	}
	return _productName;
}

/*!
 * Change the product name.
 * \sa productName()
 */
void Lgp::setProductName(const QString &productName)
{
	_productName = productName;
}

LgpHeaderEntry *Lgp::headerEntry(const QString &filePath)
{
	if(_files.isEmpty() && !openHeader()) {
		return NULL;
	}

	return _files.entry(filePath);
}

/*!
 * Opens the lgp archive, returning true if successful;
 * otherwise false.
 * \sa isOpen(), close()
 */
bool Lgp::open()
{
	if(!_file.exists()) {
		return true; // Create the file
	}

	return _file.open(QIODevice::ReadOnly);
}

/*!
 * Returns true if the lgp archive is open;
 * returns false otherwise.
 * \sa open(), close()
 */
bool Lgp::isOpen() const
{
	return _file.isOpen();
}

/*!
 * Closes the file.
 * \sa open(), isOpen()
 */
void Lgp::close()
{
	_file.close();
}

/*!
 * Returns the name set by setFileName() or to the Lgp
 * constructors.
 * \sa setFileName(), QFile::fileName()
 */
QString Lgp::fileName() const
{
	return _file.fileName();
}

/*!
 * Sets the \a name of the file.
 * Do not call this function if the file has already been opened.
 * \sa fileName(), QFile::setFileName()
 */
void Lgp::setFileName(const QString &fileName)
{
	_file.setFileName(fileName);
}

bool Lgp::openHeader()
{
	if(!_file.exists()) {
		return true; // Create the file
	}

	if(!isOpen()) {
		qWarning() << "Lgp::openHeader: The device is not open for reading";
		setError(OpenError);
		return false;
	}

	QByteArray headerData;
	const char *headerConstData;
	qint32 fileCount;

	if(!_file.seek(12)) {
		setError(PositionError);
		return false;
	}

	if(_file.read((char *)&fileCount, 4) != 4) {
		setError(ReadError);
		return false;
	}

	if(fileCount < 0) {
		setError(InvalidError);
		return false;
	}

	if(fileCount == 0) {
		return true;
	}

	headerData = _file.read(fileCount * 27);
	if(headerData.size() != fileCount * 27) {
		setError(ReadError);
		return false;
	}

	headerConstData = headerData.constData();
	QList<LgpHeaderEntry *> tocEntries;
	QList<quint16> headerConflict;
	for(int i=0; i<fileCount; ++i) {
		QString fileName = headerData.mid(i*27, 20);
		quint32 filePos;
		quint16 conflict;
		memcpy(&filePos, headerConstData + i*27 + 20, 4);
		memcpy(&conflict, headerConstData + i*27 + 25, 2);
		tocEntries.append(new LgpHeaderEntry(
							  fileName, filePos));
		headerConflict.append(conflict);
	}

	/* Resolve conflicts */

	// Lookup table ignored
	if(!_file.seek(_file.pos() + LOOKUP_TABLE_ENTRIES * 4)) {
		setError(PositionError);
		return false;
	}

	// Open conflicts
	quint16 conflictCount;

	if(_file.read((char *)&conflictCount, 2) != 2) {
		setError(ReadError);
		return false;
	}

	QList< QList<LgpConflictEntry> > conflicts;

	for(int i=0; i<conflictCount; ++i) {
		quint16 conflictEntryCount;

		// Open conflict entries
		if(_file.read((char *)&conflictEntryCount, 2) != 2) {
			setError(ReadError);
			return false;
		}

		QByteArray conflictData = _file.read(conflictEntryCount * 130);
		if(conflictData.size() != conflictEntryCount * 130) {
			setError(ReadError);
			return false;
		}

		const char *conflictConstData = conflictData.constData();
		QList<LgpConflictEntry> conflictEntries;

		for(int j=0; j<conflictEntryCount; ++j) {
			LgpConflictEntry conflictEntry(conflictData.mid(j*130, 130));

			memcpy(&conflictEntry.tocIndex, &conflictConstData[j*130 + 128], 2);

			conflictEntries.append(conflictEntry);
		}

		conflicts.append(conflictEntries);
	}

	// Set fileDir
	_files.clear();
	int headerEntryID = 0;
	foreach(LgpHeaderEntry *entry, tocEntries) {
		quint16 conflict = headerConflict.at(headerEntryID);
		if(!_files.addEntry(entry)) {
			qWarning() << "Invalid toc name" << entry->fileName();
			delete entry;
			continue;
		}

		if(conflict != 0) {
			const quint16 conflictID = conflict - 1;
			bool resolved = false;

			if(conflictID < conflicts.size()) {
				const QList<LgpConflictEntry> &conflictEntries = conflicts.at(conflictID);

				foreach(const LgpConflictEntry &conflictEntry, conflictEntries) {
					if(conflictEntry.tocIndex == headerEntryID) {
						entry->setFileDir(conflictEntry.fileDir);
						resolved = true;
						break;
					}
				}
			}

			if(!resolved) {
				qWarning() << "Unresolved conflict for" << entry->fileName();
			}
		}
		++headerEntryID;
	}

	return true;
}

/*!
 * Save the lgp into \a destination (or overwrite the
 * current archive if \a destination is empty).
 * \a observer is used to notify the progression of the save.
 * It can be NULL.
 */
bool Lgp::pack(const QString &destination, LgpObserver *observer)
{
	// Opens the header
	if(_files.isEmpty() && !openHeader()) {
		// Error setted by openHeader()
		return false;
	}

	QString destPath = destination;

	if(destination.isEmpty()) {
		QFileInfo fileInfo(_file);
		destPath = fileInfo.absoluteFilePath();
	}

	// Temporary file (same dir as destination)
	QFile temp(destPath + ".temp");
	if(!temp.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		setError(OpenError, temp.errorString());
		return false;
	}

	LgpToc newToc;
	const QList<const LgpHeaderEntry *> toc = _files.filesSortedByPosition();
	const int nbFiles = toc.size();
	int fileId;

	// Range (0 to max) for the progression indicator
	if(observer) {
		observer->setObserverMaximum(nbFiles);
		fileId = 0;
	}

	// Writes the company name (  SQUARESOFT)
	if(temp.write(companyName().toLatin1().rightJustified(12, '\0', true)) != 12) {
		temp.remove();
		setError(WriteError, temp.errorString());
		return false;
	}

	// Writes the file count
	if(temp.write((char *)&nbFiles, 4) != 4) {
		temp.remove();
		setError(WriteError, temp.errorString());
		return false;
	}

	const quint32 posLookupTable = 16 + nbFiles * 27;

	// We write data first, and toc in second
	if(!temp.resize(posLookupTable)) {
		temp.remove();
		setError(ResizeError, temp.errorString());
		return false;
	}
	if(!temp.seek(posLookupTable)) {
		temp.remove();
		setError(PositionError, temp.errorString());
		return false;
	}

	// Lookup Table + conflicts
	LgpLookupTableEntry lookupTable[LOOKUP_TABLE_ENTRIES];
	QHash<LgpHeaderEntry *, LgpTocEntry> tocEntries;
	int tocIndex = 0;

	for(int i=0; i<LOOKUP_TABLE_ENTRIES; ++i) {
		// toc index initialization
		foreach(LgpHeaderEntry *headerEntry, _files.entries(i)) {
			tocEntries.insert(headerEntry, LgpTocEntry(tocIndex++));
		}
	}

	QList< QList<LgpConflictEntry> > conflicts;

	for(int i=0; i<LOOKUP_TABLE_ENTRIES; ++i) {
		QList<LgpHeaderEntry *> headerEntries = _files.entries(i);

		// Build list conflicts
		foreach(LgpHeaderEntry *headerEntry, headerEntries) {
			LgpTocEntry &tocEntry = tocEntries[headerEntry];

			if(tocEntry.conflict == 0) {
				QList<LgpConflictEntry> conflictEntries;

				foreach(LgpHeaderEntry *headerEntry2, headerEntries) {
					if(headerEntry != headerEntry2 &&
							headerEntry->fileName().compare(headerEntry2->fileName(),
															Qt::CaseInsensitive) == 0) {
						if(conflictEntries.isEmpty()) {
							tocEntry.conflict = conflicts.size() + 1;

							conflictEntries.append(LgpConflictEntry(headerEntry->fileDir(),
																	tocEntry.tocIndex));
						}

						LgpTocEntry &tocEntry2 = tocEntries[headerEntry2];

						tocEntry2.conflict = conflicts.size() + 1;

						conflictEntries.append(LgpConflictEntry(headerEntry2->fileDir(),
																tocEntry2.tocIndex));
					}
				}

				if(!conflictEntries.isEmpty()) {
					conflicts.append(conflictEntries);
				}
			}
		}

		// Build lookup table
		lookupTable[i].tocOffset = headerEntries.isEmpty()
				? 0 : tocEntries.value(headerEntries.first()).tocIndex + 1;
		lookupTable[i].fileCount = headerEntries.size();
	}

	// Write Lookup Table
	if(temp.write((char *)lookupTable, sizeof(lookupTable)) != sizeof(lookupTable)) {
		temp.remove();
		setError(WriteError, temp.errorString());
		return false;
	}

	// Write conflicts
	QByteArray conflictsData;
	const quint16 conflictCount = conflicts.size();
	conflictsData.append((char *)&conflictCount, 2);

	foreach(const QList<LgpConflictEntry> &conflict, conflicts) {
		quint16 conflictEntryCount = conflict.size();
		conflictsData.append((char *)&conflictEntryCount, 2);

		foreach(const LgpConflictEntry &conflictEntry, conflict) {
			conflictsData.append(conflictEntry.fileDir.toLatin1().leftJustified(128, '\0', true));
			conflictsData.append((char *)&conflictEntry.tocIndex, 2);
		}
	}

	if(temp.write(conflictsData) != conflictsData.size()) {
		temp.remove();
		setError(WriteError, temp.errorString());
		return false;
	}

	// Write files
	foreach(const LgpHeaderEntry *lgpEntry, toc) {
		// Cancels if requested
		if(observer && observer->observerWasCanceled()) {
			temp.remove();
			setError(AbortError);
			return false;
		}

		// Infos for the current file
		const QString &path = lgpEntry->filePath();

		// Notify the progression
		if(observer)	observer->setObserverValue(fileId++);

		// Changes the header info
		LgpHeaderEntry *newEntry = new LgpHeaderEntry(*lgpEntry);
		newEntry->setFilePosition(temp.pos());
		newEntry->setFile(0);
		newEntry->setModifiedFile(0);
		newToc.addEntry(newEntry);

		// Writes the file
		QIODevice *io = modifiedFile(path);
		if(io == NULL) {
			temp.remove();
			setError(FileNotFoundError, QT_TRANSLATE_NOOP(Lgp, QString("File '%1' not found")
														  .arg(path).toLatin1().data()));
			return false;
		}
		if(!io->open(QIODevice::ReadOnly)) {
			temp.remove();
			setError(OpenError, temp.errorString());
			return false;
		}
		// File: writes the name
		if(temp.write(lgpEntry->fileName().toLatin1().leftJustified(20, '\0', true)) != 20) {
			temp.remove();
			setError(WriteError, temp.errorString());
			return false;
		}
		// File: writes the size
		QByteArray data = io->readAll();
		io->close();
		const qint64 size = data.size();
		if(temp.write((char *)&size, 4) != 4) {
			temp.remove();
			setError(WriteError, temp.errorString());
			return false;
		}
		// File: writes data
		if(temp.write(data) != size) {
			temp.remove();
			setError(WriteError, temp.errorString());
			return false;
		}
	}

	// Writes the product name (FINAL FANTASY7)
	if(temp.write(productName().toLatin1().leftJustified(14, '\0', true)) != 14) {
		temp.remove();
		setError(WriteError, temp.errorString());
		return false;
	}

	if(observer)	observer->setObserverValue(fileId);

	// Go back to the header
	if(!temp.seek(16)) {
		temp.remove();
		setError(PositionError, temp.errorString());
		return false;
	}

	// Header: TOC
	QByteArray tocData;
	for(int i=0; i<LOOKUP_TABLE_ENTRIES; ++i) {
		QList<LgpHeaderEntry *> headerEntries = newToc.entries(i);

		foreach(LgpHeaderEntry *headerEntry, headerEntries) {
			tocData.append(headerEntry->fileName().toLower().toLatin1().leftJustified(20, '\0', true));
			quint32 filePos = headerEntry->filePosition();
			tocData.append((char *)&filePos, 4);
			tocData.append('\x0e');
			quint16 conflict = tocEntries.value(headerEntry).conflict;
			tocData.append((char *)&conflict, 2);
		}
	}

	if(temp.write(tocData) != tocData.size()) {
		temp.remove();
		setError(WriteError, temp.errorString());
		return false;
	}

	if(observer) {
		// Infinite...
		observer->setObserverValue(-1);
		// Cancel if requested (last chance)
		if(observer->observerWasCanceled()) {
			temp.remove();
			setError(AbortError);
			return false;
		}
	}

	_file.close();

	// Remove destination file
	if(QFile::exists(destPath)) {
		if(!QFile::remove(destPath)) {
			temp.remove();
			setError(RemoveError, temp.errorString());
			return false;
		}
	}
	// Move temp file to destination
	if(!temp.rename(destPath)) {
		temp.remove();
		setError(RenameError, temp.errorString());
		return false;
	}

	// Now the archive is located in "destination"
	_file.setFileName(destPath);

	_files = newToc;
	setError(NoError);

	return true;
}

/*!
 * Returns the last error status.
 * \sa unsetError()
 */
Lgp::LgpError Lgp::error() const
{
	return _error;
}

void Lgp::setError(LgpError error, const QString &errorString)
{
	_error = error;
	_errorString = errorString;
}

/*!
 * Sets the file's error to Lgp::NoError.
 * \sa error()
 */
void Lgp::unsetError()
{
	setError(NoError);
}

QString Lgp::errorString() const
{
	return _errorString.isEmpty()
			? QLatin1String(QT_TRANSLATE_NOOP(Lgp, ("Unknown error")))
			: _errorString;
}
