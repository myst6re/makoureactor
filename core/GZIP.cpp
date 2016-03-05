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
#include <zlib.h>
#undef compress // conflict with GZIP::compress

QByteArray GZIP::decompress(const QByteArray &data, int decSize, Strategy strategy)
{
	return decompress(data.constData(), data.size(), decSize, strategy);
}

QByteArray GZIP::compress(const QByteArray &ungzip, int level, Strategy strategy)
{
	return compress(ungzip.constData(), ungzip.size(), level, strategy);
}

QByteArray GZIP::decompress(const char *data, int size, int decSize, Strategy strategy)
{
	QTemporaryFile temp;
	if(!temp.open()) {
		return QByteArray();
	}
	temp.write(data, size);
	temp.close();

	return decompress(temp.fileName(), decSize, strategy);
}

QByteArray GZIP::compress(const char *ungzip, int size, int level, Strategy strategy)
{
	QString tempPath = QDir::tempPath() + "/qt_temp.gz";

	gzFile file2 = gzopen(tempPath.toLatin1(), gzMode("w", level, strategy).toLatin1());
	if(!file2) {
		return QByteArray();
	}
	gzwrite(file2, ungzip, size);
	gzclose(file2);
	QFile finalFile(tempPath);
	if(!finalFile.open(QIODevice::ReadOnly)) {
		return QByteArray();
	}

	QByteArray data = finalFile.readAll();
	finalFile.remove();

	return data;
}

QByteArray GZIP::decompress(const QString &path, int decSize, Strategy strategy)
{
	Q_UNUSED(decSize);
	QByteArray ungzip;

	gzFile file = gzopen(path.toLatin1(), gzMode("r", -1, strategy).toLatin1());
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

char GZIP::strategyToChar(Strategy strategy)
{
	switch (strategy) {
	case StrategyDefault:     return '*';
	case StrategyFiltered:    return 'f';
	case StrategyHuffmanOnly: return 'h';
	case StrategyRle:         return 'R';
	case StrategyFixed:       return 'F';
	}
	return '*';
}

QString GZIP::gzMode(const char *mode, int level, Strategy strategy)
{
	QString m(mode);
	if (level >= 0 && level <= 9) {
		m.append(QString::number(level));
	}
	char s = strategyToChar(strategy);
	if (s != '*') {
		m.append(s);
	}
	return m;
}
