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

QIODevice *LgpHeaderEntry::file() const
{
	return _io;
}

QIODevice *LgpHeaderEntry::modifiedFile() const
{
	return _newIO;
}

void LgpHeaderEntry::setFile(QIODevice *io)
{
	_io = io;
}

void LgpHeaderEntry::setModifiedFile(QIODevice *io)
{
	_newIO = io;
}

LgpIO::LgpIO(QIODevice *lgp, LgpHeaderEntry *header, QObject *parent) :
	QIODevice(parent), _lgp(lgp), _header(header)
{
}

bool LgpIO::open(OpenMode mode) {
	if(mode.testFlag(QIODevice::WriteOnly)
			|| mode.testFlag(QIODevice::Append)
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
