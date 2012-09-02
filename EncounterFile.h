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
#define PROBABILITY(x)		(x >> 10)
#define BATTLE_ID(x)		(x & 0x03FF)

typedef struct {
	quint8 enabled;
	quint8 rate;
	quint16 enc_standard[6];// probability(6) | battle_id(10)
	quint16 enc_special[4];// probability(6) | battle_id(10)
	quint16 _pad;
} EncounterTable;

class EncounterFile
{
public:
	EncounterFile();
	EncounterFile(const QByteArray &data);
	bool isOpen() const;
	bool isModified() const;
	void setModified(bool);
	bool open(const QByteArray &data);
	QByteArray save() const;
	const EncounterTable &encounterTable(bool tableID) const;
	void setEncounterTable(bool tableID, const EncounterTable &table);
private:
	EncounterTable tables[2];
	bool _isOpen, _isModified;
};

#endif // ENCOUNTERFILE_H
