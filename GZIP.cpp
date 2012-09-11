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
#include "GZIP.h"
#ifdef Q_OS_WIN32
#include "zlib-1.2.5/zlib.h"
#else
#include <zlib.h>
#endif

QByteArray GZIP::decompress(const QByteArray &data, int/* decSize*/)
{
	QByteArray ungzip;

	QTemporaryFile temp;
	if(!temp.open()) {
		return QByteArray();
	}
	temp.write(data);
	temp.close();
	gzFile file = gzopen(temp.fileName().toLatin1(), "rb");
	if(!file) {
		return QByteArray();
	}
	char buffer[10000];
	int r;
	while((r = gzread(file, buffer, 10000)) > 0) {
		ungzip.append(buffer, r);
	}
	gzclose(file);

	return ungzip;
}

QByteArray GZIP::compress(const QByteArray &ungzip)
{
	QString tempPath = QDir::tempPath()+"/qt_temp.gz";

	gzFile file2 = gzopen(tempPath.toLatin1(), "wb");
	if(!file2) {
		return QByteArray();
	}
	gzwrite(file2, ungzip.constData(), ungzip.size());
	gzclose(file2);
	QFile finalFile(tempPath);
	if(!finalFile.open(QIODevice::ReadOnly)) {
		return QByteArray();
	}

	QByteArray data = finalFile.readAll();
	finalFile.remove();

	return data;
}
