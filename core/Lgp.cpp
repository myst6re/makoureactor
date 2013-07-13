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
/*
 * This file may contains some code (especially the conflict part)
 * inspired from LGP/UnLGP tool written by Aali.
 * http://forums.qhimm.com/index.php?topic=8641.0
 */
#include "Lgp.h"
#include "Lgp_p.h"

/*!
 * You must use Lgp::iterator() instead.
 */
LgpIterator::LgpIterator(LgpToc *toc, QFile *lgp) :
	it(toc->table()), _lgp(lgp)
{
}

/*!
 * Returns true if there is at least one item ahead of the iterator, i.e.
 * the iterator is not at the back of the container; otherwise returns false.
 * \sa hasPrevious(), next()
 */
bool LgpIterator::hasNext() const
{
	return it.hasNext();
}

/*!
 * Returns true if there is at least one item behind the iterator, i.e.
 * the iterator is not at the front of the container; otherwise returns false.
 * \sa hasNext(), previous()
 */
bool LgpIterator::hasPrevious() const
{
	return it.hasPrevious();
}

/*!
 * Advances the iterator by one position.
 * Calling this function on an iterator located at the back
 * of the container leads to undefined results.
 * \sa hasNext(), previous()
 */
void LgpIterator::next()
{
	it.next();
}

/*!
 * Moves the iterator back by one position.
 * Calling this function on an iterator located at the front of the
 * container leads to undefined results.
 * \sa hasPrevious(), next()
 */
void LgpIterator::previous()
{
	it.previous();
}

/*!
 * Moves the iterator to the back of the container (after the last item).
 * \sa toFront(), previous()
 */
void LgpIterator::toBack()
{
	it.toBack();
}

/*!
 * Moves the iterator to the front of the container (before the first item).
 * \sa toBack(), next()
 */
void LgpIterator::toFront()
{
	it.toFront();
}

/*!
 * Returns the current file.
 * \sa modifiedFile(), fileName(), fileDir(), filePath()
 */
QIODevice *LgpIterator::file()
{
	return it.value()->file(_lgp);
}

/*!
 * Returns the current modified file.
 * \sa file(), fileName(), fileDir(), filePath()
 */
QIODevice *LgpIterator::modifiedFile()
{
	return it.value()->modifiedFile(_lgp);
}

/*!
 * Returns the current file name (without the directory).
 * \sa file(), modifiedFile(), fileDir(), filePath()
 */
const QString &LgpIterator::fileName() const
{
	return it.value()->fileName();
}

/*!
 * Returns the current file directory (without the file name).
 * \sa file(), modifiedFile(), fileName(), filePath()
 */
const QString &LgpIterator::fileDir() const
{
	return it.value()->fileDir();
}

/*!
 * Returns the current full file path (dir + name).
 * \sa file(), modifiedFile(), fileName(), fileDir()
 */
QString LgpIterator::filePath() const
{
	return it.value()->filePath();
}

/*!
 * Constructs a new empty lgp archive.
 */
Lgp::Lgp() :
	Archive(new QLockedFile()), _files(new LgpToc), _error(NoError)
{
}

/*!
 * Constructs a new lgp archive object to represent the lgp archive with the given \a name.
 */
Lgp::Lgp(const QString &name) :
	Archive(new QLockedFile(name)), _files(new LgpToc), _error(NoError)
{
}

/*!
 * Destroys the lgp archive object, closing it if necessary.
 */
Lgp::~Lgp()
{
	delete _files;
}

/*!
 * Clears the contents of the lgp.
 */
void Lgp::clear()
{
	_companyName.clear();
	_files->clear();
	_productName.clear();
}

/*!
 * Returns a list of file paths sorted by file position.
 */
QStringList Lgp::fileList() const
{
	QStringList ret;

	if(_files->isEmpty()) {
		return ret;
	}

	foreach(const LgpHeaderEntry *entry, _files->filesSortedByPosition()) {
		ret.append(entry->filePath());
	}

	return ret;
}

/*!
 * Returns the number of files in the archive, or -1 if there is an error.
 */
int Lgp::fileCount() const
{
	return _files->size();
}

/*!
 * Returns an iterator to iterate over the files.
 * You can not know if there is an error or not.
 */
LgpIterator Lgp::iterator()
{
	return LgpIterator(_files, archiveIO());
}

/*!
 * Returns true if the file named \a filePath exists; otherwise
 * false.
 */
bool Lgp::fileExists(const QString &filePath) const
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

	return entry->file(archiveIO());
}

/*!
 * Returns the data, modified by setData if modified, for the file named \a filePath.
 * \sa file(), fileData(), modifiedFileData()
 */
QIODevice *Lgp::modifiedFile(const QString &filePath)
{
	LgpHeaderEntry *entry = headerEntry(filePath);// need to open the header
	if(entry == NULL) return NULL;

	return entry->modifiedFile(archiveIO());
}

/*!
 * Change the \a data for the file named \a filePath.
 * Returns false if the file doesn't exists; otherwise
 * returns true.
 * \sa setFileData()
 */
bool Lgp::setFile(const QString &filePath, QIODevice *data)
{
	LgpHeaderEntry *entry = headerEntry(filePath);// need to open the header
	if(entry == NULL) return false;

	entry->setModifiedFile(data);

	return true;
}

/*!
 * Add a new file named \a filePath with \a data.
 * Returns false if the file exists; otherwise returns
 * true.
 * \sa addFileData()
 */
bool Lgp::addFile(const QString &filePath, QIODevice *data)
{
	LgpHeaderEntry *entry = headerEntry(filePath);// need to open the header
	if(entry != NULL) return false;

	entry = new LgpHeaderEntry(filePath, archiveIO()->size());
	entry->setModifiedFile(data);

	bool ret = _files->addEntry(entry);

	if(!ret) {
		delete entry;
	}

	return ret;
}

/*!
 * Remove the file named \a filePath.
 * Returns true if the file is successfully removed.
 */
bool Lgp::removeFile(const QString &filePath)
{
	return _files->removeEntry(filePath);
}

/*!
 * Check if the \a filePath is a valid name.
 * Returns true if \a filePath is a valid name, false otherwise.
 */
bool Lgp::isNameValid(const QString &filePath) const
{
	return _files->isNameValid(filePath);
}

/*!
 * Rename the file named \a filePath by \a newFilePath.
 * Returns true if the file is successfully renamed.
 */
bool Lgp::renameFile(const QString &filePath, const QString &newFilePath)
{
	return _files->renameEntry(filePath, newFilePath);
}

bool Lgp::openCompanyName()
{
	if(!isOpen()) {
		qWarning() << "Lgp::companyName: The device is not open for reading";
		return false;
	}

	if(!archiveIO()->reset()) {
		return false;
	}
	QByteArray companyData = archiveIO()->read(12);
	if(companyData.size() != 12) {
		return false;
	}
	const char *data = companyData.constData();
	const char *last = data + 12;
	while(*data == '\0' && data < last) {
		data++;
	}
	_companyName = QByteArray(data, (int)(last - data));

	return true;
}

/*!
 * Returns the company name (like "SQUARESOFT") or
 * a null string if there is an error.
 * \sa setCompanyName()
 */
const QString &Lgp::companyName()
{
	if(_companyName.isNull()) {
		openCompanyName();
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

bool Lgp::openProductName()
{
	if(!isOpen()) {
		qWarning() << "Lgp::companyName: The device is not open for reading";
		return false;
	}

	if(!archiveIO()->seek(archiveIO()->size() - 14)) {
		return false;
	}
	_productName = archiveIO()->read(14);

	return true;
}

/*!
 * Returns the product name (like "FINAL FANTASY7") or
 * a null string if there is an error.
 * \sa setProductName()
 */
const QString &Lgp::productName()
{
	if(_productName.isNull()) {
		openProductName();
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

LgpHeaderEntry *Lgp::headerEntry(const QString &filePath) const
{
	return _files->entry(filePath);
}

bool Lgp::openHeader()
{
	if(!archiveIO()->exists()) {
		return true; // Create the file
	}

	if(!isOpen()) {
		qWarning() << "Lgp::openHeader: The device is not open for reading";
		setError(OpenError);
		return false;
	}

	if(!archiveIO()->seek(12)) {
		setError(PositionError);
		return false;
	}

	qint32 fileCount;

	if(archiveIO()->read((char *)&fileCount, 4) != 4) {
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

	const qint32 sizeToc = fileCount * 27;
	QByteArray headerData = archiveIO()->read(sizeToc);

	if(headerData.size() != sizeToc) {
		setError(ReadError);
		return false;
	}

	if(sizeToc < 0) {
		setError(InvalidError);
		return false;
	}

	const char *headerConstData = headerData.constData();
	QList<LgpHeaderEntry *> tocEntries;
	QList<quint16> headerConflict;
	bool hasConflict = false;

	for(qint32 cur=0; cur<sizeToc; cur += 27) {
		quint32 filePos;
		quint16 conflict;
		memcpy(&filePos, headerConstData + cur + 20, 4);
		memcpy(&conflict, headerConstData + cur + 25, 2);
		tocEntries.append(new LgpHeaderEntry(
							  headerData.mid(cur, 20),
							  filePos));
		headerConflict.append(conflict);
		if(conflict != 0 && !hasConflict) {
			hasConflict = true;
		}
	}

	/* Resolve conflicts */

	QList< QList<LgpConflictEntry> > conflicts;

	if(hasConflict) {

		// Lookup table ignored
		if(!archiveIO()->seek(archiveIO()->pos() + LOOKUP_TABLE_ENTRIES * 4)) {
			setError(PositionError);
			return false;
		}

		// Open conflicts
		quint16 conflictCount;

		if(archiveIO()->read((char *)&conflictCount, 2) != 2) {
			setError(ReadError);
			return false;
		}

		for(qint32 i=0; i<conflictCount; ++i) {
			quint16 conflictEntryCount;

			// Open conflict entries
			if(archiveIO()->read((char *)&conflictEntryCount, 2) != 2) {
				setError(ReadError);
				return false;
			}

			const qint32 sizeConflicData = conflictEntryCount * 130;
			QByteArray conflictData = archiveIO()->read(sizeConflicData);

			if(conflictData.size() != sizeConflicData) {
				setError(ReadError);
				return false;
			}

			if(sizeConflicData < 0) {
				setError(InvalidError);
				return false;
			}

			const char *conflictConstData = conflictData.constData();
			QList<LgpConflictEntry> conflictEntries;

			for(qint32 cur=0; cur<sizeConflicData; cur += 130) {
				LgpConflictEntry conflictEntry(conflictData.mid(cur, 128));

				memcpy(&conflictEntry.tocIndex, conflictConstData + cur + 128, 2);

				conflictEntries.append(conflictEntry);
			}

			conflicts.append(conflictEntries);
		}

	}

	/* Populate _files */

	int headerEntryID = 0;

	_files->clear();

	foreach(LgpHeaderEntry *entry, tocEntries) {
		if(!_files->addEntry(entry)) {
			qWarning() << "Invalid toc name" << entry->fileName();
			delete entry;
			continue;
		}

		// Set fileDir
		if(hasConflict) {
			const quint16 &conflict = headerConflict.at(headerEntryID);

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
	}

	return true;
}

/*!
 * Save the lgp into \a destination (or overwrite the
 * current archive if \a destination is empty).
 * The archive is closed after this operation.
 * \a observer is used to notify the progression of the save.
 * It can be NULL.
 */
bool Lgp::pack(const QString &destination, ArchiveObserver *observer)
{
	const int nbFiles = _files->size();
	int fileId;

	// Range (0 to max) for the progression indicator
	if(observer) {
		observer->setObserverMaximum(nbFiles);
		fileId = 0;
	}

	QString destPath = destination;

	if(destination.isEmpty()) {
		QFileInfo fileInfo(*archiveIO());
		destPath = fileInfo.absoluteFilePath();
	}

	// Temporary file (same dir as destination)
	QFile temp(destPath + ".temp");
	if(!temp.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		setError(OpenError, temp.errorString());
		return false;
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
	QHash<const LgpHeaderEntry *, LgpTocEntry> tocEntries;
	int tocIndex = 0;

	for(int i=0; i<LOOKUP_TABLE_ENTRIES; ++i) {
		// toc index initialization
		foreach(const LgpHeaderEntry *headerEntry, _files->entries(i)) {
			tocEntries.insert(headerEntry, LgpTocEntry(tocIndex++));
		}
	}

	QList< QList<LgpConflictEntry> > conflicts;

	for(int i=0; i<LOOKUP_TABLE_ENTRIES; ++i) {
		QList<LgpHeaderEntry *> headerEntries = _files->entries(i);

		// Build list conflicts
		foreach(const LgpHeaderEntry *headerEntry, headerEntries) {
			LgpTocEntry &tocEntry = tocEntries[headerEntry];

			if(tocEntry.conflict == 0) {
				QList<LgpConflictEntry> conflictEntries;

				foreach(const LgpHeaderEntry *headerEntry2, headerEntries) {
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

	LgpToc newToc;

	// Write files
	foreach(const LgpHeaderEntry *lgpEntry, _files->filesSortedByPosition()) {
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
		const QByteArray data = io->readAll();
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
		foreach(const LgpHeaderEntry *headerEntry, newToc.entries(i)) {
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

	archiveIO()->close();

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
	setFileName(destPath);

	*_files = newToc;
	setError(NoError);

	return true;
}

/*!
 * Returns the last error status.
 * \sa unsetError(), errorString()
 */
Lgp::LgpError Lgp::error() const
{
	return _error;
}

void Lgp::setError(LgpError error, const QString &errorString)
{
	_error = error;
	setErrorString(errorString);
}

/*!
 * Sets the file's error to Lgp::NoError.
 * \sa error(), errorString()
 */
void Lgp::unsetError()
{
	setError(NoError);
}
