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
#ifndef QLOCKEDFILE_H
#define QLOCKEDFILE_H

#include <QtCore>
#ifdef Q_WS_WIN
#include <windef.h>
#include <winbase.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class QLockedFile : public QFile
{
	Q_OBJECT
public:
	explicit QLockedFile(const QString &name);
	explicit QLockedFile(QObject *parent);
	explicit QLockedFile(const QString &name, QObject *parent);
	virtual ~QLockedFile();
	virtual void close();
	virtual bool open(OpenMode mode);
signals:

public slots:
private:
#ifdef Q_WS_WIN
	HANDLE handle;
#else
	int handle;
#endif
};

#endif // QLOCKEDFILE_H
