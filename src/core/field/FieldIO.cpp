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
#include "FieldIO.h"
#include "Field.h"

FieldSaveIO::FieldSaveIO(Field *field, QObject *parent) :
	QIODevice(parent), _field(field)
{
}

bool FieldSaveIO::open(OpenMode mode)
{
	if (!setCache()) {
		return false;
	}
	return QIODevice::open(mode);
}

void FieldSaveIO::close()
{
	_cache.clear();
	QIODevice::close();
}

qint64 FieldSaveIO::size() const
{
	return _cache.size();
}

qint64 FieldSaveIO::readData(char *data, qint64 maxSize)
{
	if (pos() < _cache.size()) {
		const char *constData = _cache.constData();
		qint64 r = qMin(maxSize, _cache.size() - pos());
		if (r > 0) {
			memcpy(data, constData + pos(), r);
			return r;
		} else if (r == 0) {
			return 0;
		}
		return -1;
	}
	return 0;
}

bool FieldSaveIO::setCache()
{
	if (_cache.isEmpty()) {
		if (!_field->save(_cache, true)) {
			_cache.clear();
			setErrorString(tr("Cannot save field map %1").arg(_field->name()));
			return false;
		}
	}
	return true;
}
