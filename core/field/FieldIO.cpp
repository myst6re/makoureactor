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

void FieldSaveIO::close()
{
	_cache.clear();
	QIODevice::close();
}

qint64 FieldSaveIO::readData(char *data, qint64 maxSize)
{
	if(setCache()) {
		if(pos() < _cache.size()) {
			const char *constData = _cache.constData();
			qint64 r = qMin(maxSize, _cache.size() - pos());
			if(r > 0) {
				memcpy(data, constData + pos(), r);
				return r;
			} else if(r == 0) {
				return 0;
			} else {
				return -1;
			}
		} else {
			return 0;
		}
	}
	return -1;
}

bool FieldSaveIO::setCache()
{
	if(_cache.isEmpty()) {
		if(!_field->save(_cache, true)) {
			_cache.clear();
			return false;
		}
	}
	return true;
}
