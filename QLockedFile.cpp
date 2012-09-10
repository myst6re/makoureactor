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

void QLockedFile::close()
{
	// Unlock file
#ifdef Q_WS_WIN
	CloseHandle(handle);
#else
	struct flock lock;
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	fcntl(handle, F_SETLK, &lock);

	::close(handle);
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
#ifdef Q_WS_WIN
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
#else
	struct flock lock;

	handle = ::open(QDir::toNativeSeparators(fileName()).toLatin1().data(), O_RDWR);
	if(handle == -1) {
		qWarning() << "QLockedFile::open error open";
		return false;
	}
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	if(fcntl(handle, F_GETLK, &lock) == -1) {
		qWarning() << "QLockedFile::open error test lock";
		return false;
	}

	if(lock.l_type == F_UNLCK) {
		lock.l_type = F_WRLCK;

		if(fcntl(handle, F_SETLKW, &lock) == -1) {
			qWarning() << "QLockedFile::open error lock";
			return false;
		}/* else {
			qDebug() << "file locked";
		}*/
	} else {
		return false;
	}
#endif

	return QFile::open(mode);
}
