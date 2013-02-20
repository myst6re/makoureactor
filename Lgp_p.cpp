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
#include "Lgp_p.h"

LgpHeaderEntry::LgpHeaderEntry(const QString &fileName, quint32 filePosition) :
	_fileName(fileName), _filePosition(filePosition),
	_hasFileSize(false), _io(NULL), _newIO(NULL)
{
}

LgpHeaderEntry::~LgpHeaderEntry()
{
	if(_io != NULL) {
		_io->deleteLater();
	}
	if(_newIO != NULL) {
		_newIO->deleteLater();
	}
}

const QString &LgpHeaderEntry::fileName() const
{
	return _fileName;
}

const QString &LgpHeaderEntry::fileDir() const
{
	return _fileDir;
}

QString LgpHeaderEntry::filePath() const
{
	return _fileDir.isEmpty()
			? _fileName
			: _fileDir + '/' + _fileName;
}

quint32 LgpHeaderEntry::filePosition() const
{
	return _filePosition;
}

qint64 LgpHeaderEntry::fileSize() const
{
	if(_hasFileSize) {
		return _fileSize;
	}
	return -1;
}

void LgpHeaderEntry::setFileName(const QString &fileName)
{
	if(fileName.size() > 20) {
		_fileName = fileName.left(20);
	} else {
		_fileName = fileName;
	}
}

void LgpHeaderEntry::setFileDir(const QString &fileDir)
{
	if(fileDir.size() > 128) {
		_fileDir = fileDir.left(128);
	} else {
		_fileDir = fileDir;
	}
}

void LgpHeaderEntry::setFilePosition(quint32 filePosition)
{
	_filePosition = filePosition;
}

void LgpHeaderEntry::setFileSize(quint32 fileSize)
{
	_fileSize = fileSize;
	_hasFileSize = true;
}

QIODevice *LgpHeaderEntry::file(QFile *lgp)
{
	if(_io) {
		_io->close();
		return _io;
	} else {
		return createFile(lgp);
	}
}

QIODevice *LgpHeaderEntry::modifiedFile(QFile *lgp)
{
	if(_newIO) {
		_newIO->close();
		return _newIO;
	} else {
		return file(lgp);
	}
}

void LgpHeaderEntry::setFile(QIODevice *io)
{
	_io = io;
}

void LgpHeaderEntry::setModifiedFile(QIODevice *io)
{
	_newIO = io;
}

QIODevice *LgpHeaderEntry::createFile(QFile *lgp)
{
	if(!lgp->seek(filePosition())) {
		return NULL;
	}
	QByteArray name = lgp->read(20);
	if(name.size() != 20) {
		return NULL;
	}
	quint32 size;
	if(lgp->read((char *)&size, 4) != 4) {
		return NULL;
	}

	setFileSize(size);
	setFileName(name);
	QIODevice *io = new LgpIO(lgp, this);
	setFile(io);
	return io;
}

LgpIO::LgpIO(QIODevice *lgp, const LgpHeaderEntry *header, QObject *parent) :
	QIODevice(parent), _lgp(lgp), _header(header)
{
}

bool LgpIO::open(OpenMode mode) {
	if(mode.testFlag(QIODevice::Append)
			|| mode.testFlag(QIODevice::Truncate)) {
		return false;
	}
	return QIODevice::open(mode);
}

qint64 LgpIO::size() const
{
	return _header->fileSize();
}

qint64 LgpIO::readData(char *data, qint64 maxSize)
{
	if(_lgp->seek(_header->filePosition() + 24 + pos())) {
		qint64 size = this->size();
		if(size < 0) {
			return -1;
		}
		return _lgp->read(data, qMin(maxSize, size - pos()));
	}
	return -1;
}

/*!
 * You cannot write more than the initial file size.
 */
qint64 LgpIO::writeData(const char *data, qint64 maxSize)
{
	if(_lgp->seek(_header->filePosition() + 24 + pos())) {
		qint64 size = this->size();
		if(size < 0) {
			return -1;
		}
		return _lgp->write(data, qMin(maxSize, size - pos()));
	}
	return -1;
}

bool LgpIO::canReadLine() const
{
	return pos() < size();
}

LgpToc::LgpToc()
{
}

LgpToc::LgpToc(const LgpToc &other)
{
	foreach(LgpHeaderEntry *headerEntry, other.table()) {
		addEntry(new LgpHeaderEntry(*headerEntry));
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

const QMultiHash<quint16, LgpHeaderEntry *> &LgpToc::table() const
{
	return _header;
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
		foreach(LgpHeaderEntry *headerEntry, other.table()) {
			addEntry(new LgpHeaderEntry(*headerEntry));
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
