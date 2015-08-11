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
#ifndef DATFILE_H
#define DATFILE_H

#include <QtCore>
#include "../LzsIO.h"

#define DAT_FILE_SECTION_COUNT		7
#define DAT_FILE_HEADER_SIZE		28 // DAT_FILE_SECTION_COUNT * 4

class DatFile
{
public:
	enum Section {
		TextsAndScripts = 0,
		Walkmesh,
		TileMap,
		Camera,
		Triggers,
		Encounter,
		ModelLoader
	};

	DatFile();
	inline bool open(const char *data, int size) {
		return open(QByteArray(data, size));
	}
	bool open(const QByteArray &data);
	void saveStart();
	void saveClear();
	bool saveFlush(QByteArray &data);
	QByteArray sectionData(Section id) const;
	void setSectionData(Section id, const QByteArray &data);
private:
	int sectionSize(int id) const;
	inline int sectionPos(int id) const {
		Q_ASSERT(id >= 0 && id < DAT_FILE_SECTION_COUNT);
		return _sectionPositions[id];
	}
	void shiftPositionsAfter(int id, int shift);
	void writePositions();
	quint32 _sectionPositions[DAT_FILE_SECTION_COUNT];
	quint32 _shift;
	LzsRandomAccess _io;
	QByteArray _data;
};

#endif // DATFILE_H
