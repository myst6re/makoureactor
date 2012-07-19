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

EncounterFile::EncounterFile(const QByteArray &contenu)
	: _isOpen(false), _isModified(false)
{
	open(contenu);
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

bool EncounterFile::open(const QByteArray &contenu)
{
	const char *constData = contenu.constData();
	quint32 debutSection1, debutSection7, debutSection8, debutSection;

	if(contenu.startsWith(QByteArray("\x00\x00\x09\x00\x00\x00", 6))) {
		memcpy(&debutSection, &constData[30], 4);// section 7
		memcpy(&debutSection8, &constData[34], 4);
		debutSection += 4;

		if((quint32)contenu.size() <= debutSection8 || debutSection8-debutSection != 48)
			return false;
	} else {
		memcpy(&debutSection1, constData, 4);
		memcpy(&debutSection, &constData[20], 4);// section 6
		memcpy(&debutSection7, &constData[24], 4);
		debutSection = debutSection - debutSection1 + 28;
		debutSection7 = debutSection7 - debutSection1 + 28;

		if((quint32)contenu.size() <= debutSection7 || debutSection7-debutSection != 48)
			return false;
	}

	memcpy(tables, &constData[debutSection], sizeof(EncounterTable) * 2);

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
