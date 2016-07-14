/****************************************************************************
 ** Néo-Midgar Final Fantasy VII French Retranslation
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
#include "GZIPPS.h"

QByteArray GZIPPS::decompress(const char *data, int size)
{
	if (size <= GZIPPS_HEADER_SIZE) {
		return QByteArray();
	}

	quint32 decSize;
	memcpy(&decSize, data, 4);
	return GZIP::decompress(data + GZIPPS_HEADER_SIZE, size - GZIPPS_HEADER_SIZE, decSize);
}

QByteArray GZIPPS::compress(const char *ungzip, int size, const QByteArray &header, int level)
{
	Q_ASSERT(header.size() == 4);

	QByteArray ret;
	ret.append((char *)&size, 4); // = decSize
	ret.append(header);
	return ret.append(GZIP::compress(ungzip, size, level));
}
