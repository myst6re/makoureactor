/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#include "CharArchive.h"
#include "AFile.h"
#include "Data.h"

CharArchive::CharArchive() :
    _io(new Lgp()), _delete(true)
{
}

CharArchive::CharArchive(Lgp *io) :
      _io(io), _delete(false)
{
}

CharArchive::CharArchive(const QString &filename) :
      _io(new Lgp(filename)), _delete(true)
{
}

CharArchive::~CharArchive()
{
	if (_delete) {
		delete _io;
	}
}

CharArchive *CharArchive::_instance = 0;

CharArchive *CharArchive::instance()
{
	if (!_instance) {
		_instance = new CharArchive(Data::charlgp_path());
	}
	if (!_instance->isOpen()) {
		_instance->open();
	}
	return _instance;
}

void CharArchive::close()
{
	_io->clear();
	_io->close();
	_animBoneCount.clear();
}

QStringList CharArchive::hrcFiles() const
{
	QStringList files, f = _io->fileList();

	for (const QString &file : f) {
		if (file.endsWith(".hrc", Qt::CaseInsensitive)) {
			files.append(file.toUpper());
		}
	}

	return files;
}

QStringList CharArchive::aFiles(int boneCount)
{
	if (boneCount >= 0 && openAnimBoneCount()) {
		return _animBoneCount.values(boneCount);
	}

	QStringList files, f = _io->fileList();

	for (const QString &file : f) {
		if (file.endsWith(".a", Qt::CaseInsensitive)) {
			files.append(file.left(file.size()-2).toUpper());
		}
	}

	return files;
}

QIODevice *CharArchive::fileIO(const QString &filename)
{
	return _io->file(filename.toLower());
}

bool CharArchive::openAnimBoneCount()
{
	if (!isOpen()) {
		qWarning() << "CharArchive::openAnimBoneCount" << "archive not opened";
		return false;
	}

	_animBoneCount.clear();

	LgpIterator it = _io->iterator();
	while (it.hasNext()) {
		it.next();
		const QString &fileName = it.fileName();
		if (fileName.endsWith(".a", Qt::CaseInsensitive)) {
			QIODevice *aFile = it.file();
			if (aFile && aFile->open(QIODevice::ReadOnly)) {
				AFile a(aFile);
				AHeader header;

				if (!a.readHeader(header)) {
					qWarning() << "CharArchive::openAnimBoneCount" << "animation error" << fileName;
					continue;
				}

				_animBoneCount.insert(header.boneCount, fileName.left(fileName.size()-2).toUpper());
			} else {
				return false;
			}
		}
	}

	return true;
}

