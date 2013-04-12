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
#ifndef ENCOUNTERFILE_H
#define ENCOUNTERFILE_H

#include <QtCore>
#include "FieldPart.h"

#define PROBABILITY(x)		(x >> 10)
#define BATTLE_ID(x)		(x & 0x03FF)

typedef struct {
	quint8 enabled;
	quint8 rate;
	quint16 enc_standard[6];// probability(6) | battle_id(10)
	quint16 enc_special[4];// probability(6) | battle_id(10)
	quint16 _pad;
} EncounterTable;

class EncounterFile : public FieldPart
{
public:
	enum Table {
		Table1=0, Table2=1
	};

	explicit EncounterFile(Field *field);
	bool open();
	bool open(const QByteArray &data);
	QByteArray save() const;
	void clear();
	const EncounterTable &encounterTable(Table tableID) const;
	void setEncounterTable(Table tableID, const EncounterTable &table);
private:
	EncounterTable tables[2];
};

#endif // ENCOUNTERFILE_H
