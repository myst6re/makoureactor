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
#include "EncounterFile.h"
#include "Field.h"

EncounterFile::EncounterFile(Field *field) :
	FieldPart(field)
{
}

void EncounterFile::initEmpty()
{
	tables[0] = EncounterTable();
	tables[1] = EncounterTable();
}

bool EncounterFile::open()
{
	return open(field()->sectionData(Field::Encounter));
}

bool EncounterFile::open(const QByteArray &data)
{
	if (sizeof(EncounterTable) != 24) {
		qWarning() << "Encounter invalid struct size" << sizeof(EncounterTable);
		Q_ASSERT(false);
	}

	if (data.size() != 48) {
		qWarning() << "Encounter invalid data size" << data.size();
		return false;
	}

	memcpy(tables, data.constData(), sizeof(EncounterTable) * 2);

	setOpen(true);

	return true;
}

QByteArray EncounterFile::save() const
{
	return QByteArray((const char *)&tables, sizeof(EncounterTable) * 2);
}

void EncounterFile::clear()
{
	tables[Table1] = tables[Table2] = EncounterTable();
}

const EncounterTable &EncounterFile::encounterTable(Table tableID) const
{
	return tables[int(tableID)];
}

void EncounterFile::setEncounterTable(Table tableID, const EncounterTable &table)
{
	tables[int(tableID)] = table;
	setModified(true);
}

bool EncounterFile::isBattleEnabled(Table tableID) const
{
	return tables[int(tableID)].enabled;
}

void EncounterFile::setBattleEnabled(Table tableID, bool enabled)
{
	if (tables[int(tableID)].enabled != quint8(enabled)) {
		tables[int(tableID)].enabled = enabled;
		setModified(true);
	}
}
