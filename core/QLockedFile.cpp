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
#include "QLockedFile.h"

QLockedFile::QLockedFile()
{
}

QLockedFile::QLockedFile(const QString &name) :
	QFile(name)
{
}

QLockedFile::QLockedFile(QObject *parent) :
	QFile(parent)
{
}

QLockedFile::QLockedFile(const QString &name, QObject *parent) :
	QFile(name, parent)
{
}

QLockedFile::~QLockedFile()
{
	if (isOpen()) {
		close();
	}
}

void QLockedFile::close()
{
	// Unlock file
#ifdef Q_OS_WIN
	CloseHandle(handle);
#else
	::flock(handle(), LOCK_UN);
#endif
	QFile::close();
}

bool QLockedFile::open(OpenMode mode)
{
	if(!mode.testFlag(QIODevice::ReadOnly)) {
		qWarning() << "QLockedFile::open must be opened in ReadOnly mode";
		return false;
	}

	// Lock file
#ifdef Q_OS_WIN
	handle = CreateFileA(QDir::toNativeSeparators(fileName()).toLatin1().data(),
						 GENERIC_READ,
						 FILE_SHARE_READ,
						 NULL,
						 OPEN_EXISTING,
						 FILE_ATTRIBUTE_NORMAL,
						 NULL);
	if(handle == INVALID_HANDLE_VALUE) {
		qWarning() << "QLockedFile::open error lock";
		return false;
	}

	return QFile::open(mode);
#else
	bool isOpen = QFile::open(mode);

	if(isOpen && ::flock(handle(), LOCK_SH) < 0) {
		qWarning() << "QLockedFile::open error flock";
		return false;
	}

	return isOpen;
#endif
}
