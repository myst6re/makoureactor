/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#pragma once

#include <QtCore>
#include "FieldPart.h"

#define PROBABILITY(x)		(x >> 10)
#define BATTLE_ID(x)		(x & 0x03FF)

struct EncounterTable {
	quint8 enabled;
	quint8 rate;
	quint16 enc_standard[6];// probability(6) | battle_id(10)
	quint16 enc_special[4];// probability(6) | battle_id(10)
	quint16 _pad;
};

class EncounterFile : public FieldPart
{
public:
	enum Table {
		Table1=0, Table2=1
	};

	explicit EncounterFile(Field *field);
	void initEmpty() override;
	bool open() override;
	bool open(const QByteArray &data) override;
	QByteArray save() const override;
	void clear() override;
	const EncounterTable &encounterTable(Table tableID) const;
	void setEncounterTable(Table tableID, const EncounterTable &table);
	bool isBattleEnabled(Table tableID) const;
	void setBattleEnabled(Table tableID, bool enabled);
private:
	EncounterTable tables[2];
};
