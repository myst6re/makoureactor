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
#ifndef GZIP_H
#define GZIP_H

#include <QtCore>

class GZIP
{
public:
	enum Strategy {
		StrategyDefault,
		StrategyFiltered,
		StrategyHuffmanOnly,
		StrategyRle,
		StrategyFixed
	};

	static QByteArray decompress(const QByteArray &data, int decSize, Strategy strategy = StrategyDefault);
	static QByteArray compress(const QByteArray &ungzip, int level = -1, Strategy strategy = StrategyDefault);
	static QByteArray decompress(const char *data, int size, int decSize, Strategy strategy = StrategyDefault);
	static QByteArray compress(const char *ungzip, int size, int level = -1, Strategy strategy = StrategyDefault);
	static QByteArray decompress(const QString &path, int decSize, Strategy strategy = StrategyDefault);
	static QByteArray decompressNoHeader(const char *data, int size);
	static QByteArray compressNoHeader(const char *data, int size, int level = -1);
	static ulong crc(const char *data, int size);
private:
	static char strategyToChar(Strategy strategy);
	static QString gzMode(const char *mode, int level = -1, Strategy strategy = StrategyDefault);
};

#endif // GZIP_H
