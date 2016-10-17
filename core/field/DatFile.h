/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel Jï¿½rï¿½me <myst6re@gmail.com>
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
#include "../SectionFile.h"

#define DAT_FILE_SECTION_COUNT		7
#define DAT_FILE_HEADER_SIZE		28 // DAT_FILE_SECTION_COUNT * 4

class DatFile : public SectionFile
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
	inline virtual ~DatFile() {}

	inline QByteArray sectionData(Section id) {
		return SectionFile::sectionData(quint8(id));
	}
	inline void setSectionData(Section id, const QByteArray &data) {
		SectionFile::setSectionData(quint8(id), data);
	}
	inline quint8 sectionCount() const {
		return DAT_FILE_SECTION_COUNT;
	}
	inline quint32 sectionSize(Section id, bool &eof) const {
		return SectionFile::sectionSize(quint8(id), eof);
	}
private:
	bool openHeader();
	int setSectionData(int pos, int oldSize,
					   const QByteArray &section,
					   QByteArray &out);
	bool writePositions(QByteArray &data);

	quint32 _shift;
};

#endif // DATFILE_H
