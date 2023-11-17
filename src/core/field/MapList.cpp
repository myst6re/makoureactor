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
#include "MapList.h"

MapList::MapList() : _modified(false)
{

}

bool MapList::open(const QByteArray &data)
{
	if (data.size() < 2) {
		return false;
	}

	quint16 nbMap;
	const char *constData = data.constData();

	memcpy(&nbMap, constData, 2);

	if (data.size() != 2 + nbMap * 32) {
		return false;
	}

	_mapNames.clear();

	for (int i=0; i<nbMap; ++i) {
		const char *fieldName = constData + 2 + i * 32;
		// Trim \0 at the end
		_mapNames.append(QString::fromLatin1(fieldName, qsizetype(qstrnlen(fieldName, 32))).simplified());
	}

	// Remove empty entries at the end
	while (!_mapNames.isEmpty() && _mapNames.last().isEmpty()) {
		_mapNames.removeLast();
	}

	return true;
}

bool MapList::save(QByteArray &data) const
{
	if (_mapNames.size() > 65535) {
		return false;
	}

	quint16 nbMap = quint16(_mapNames.size());
	data.append(reinterpret_cast<char *>(&nbMap), 2);

	for (const QString &fieldName : std::as_const(_mapNames)) {
		data.append(fieldName.toLatin1()
		            .leftJustified(32, '\0', true));
	}

	return true;
}

void MapList::clear()
{
	_modified = false;
	_mapNames.clear();
}

void MapList::addMap(const QString &name)
{
	_modified = true;
	_mapNames.append(name);
}

bool MapList::renameMap(const QString &oldName, const QString &newName)
{
	qsizetype index = _mapNames.indexOf(oldName);
	if (index < 0) {
		return false;
	}

	_mapNames.replace(index, newName);

	_modified = true;
	
	return true;
}

bool MapList::softDeleteMap(const QString &name)
{
	qsizetype index = _mapNames.indexOf(name);
	if (index < 0) {
		return false;
	}

	_modified = true;
	_mapNames.replace(index, QString());

	return true;
}
