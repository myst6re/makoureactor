/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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
#pragma once

#include <Lgp.h>

class CharArchive
{
public:
	CharArchive();
	explicit CharArchive(Lgp *io);
	explicit CharArchive(const QString &filename);
	virtual ~CharArchive();

	static CharArchive *instance();

	inline bool isOpen() const {
		return _io->isOpen();
	}
	inline bool open() {
		return _io->open();
	}
	void close();
	inline QString filename() const {
		return _io->fileName();
	}
	inline void setFilename(const QString &filename) {
		_io->setFileName(filename);
	}
	QStringList hrcFiles() const;
	QStringList aFiles(int boneCount = -1);
	QIODevice *fileIO(const QString &filename);

private:
	bool openAnimBoneCount();
	Lgp *_io;
	QMultiHash<int, QString> _animBoneCount;
	bool _delete;
	static CharArchive *_instance;
};
