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
#include "EncounterFile.h"

EncounterFile::EncounterFile()
	: _isOpen(false), _isModified(false)
{
}

EncounterFile::EncounterFile(const QByteArray &data)
	: _isOpen(false), _isModified(false)
{
	open(data);
}

bool EncounterFile::isOpen() const
{
	return _isOpen;
}

bool EncounterFile::isModified() const
{
	return _isModified;
}

void EncounterFile::setModified(bool modified)
{
	_isModified = modified;
}

bool EncounterFile::open(const QByteArray &data)
{
	if(sizeof(EncounterTable) != 24) {
		qWarning() << "Encounter invalid struct size" << sizeof(EncounterTable);
		return false;
	}

	if(data.size() != 48) {
		qWarning() << "Encounter invalid data size" << data.size();
		return false;
	}

	memcpy(tables, data.constData(), sizeof(EncounterTable) * 2);

	_isOpen = true;

	return true;
}

QByteArray EncounterFile::save() const
{
	return QByteArray((char *)&tables, sizeof(EncounterTable) * 2);
}

const EncounterTable &EncounterFile::encounterTable(bool tableID) const
{
	return tables[(int)tableID];
}

void EncounterFile::setEncounterTable(bool tableID, const EncounterTable &table)
{
	tables[(int)tableID] = table;
	_isModified = true;
}
