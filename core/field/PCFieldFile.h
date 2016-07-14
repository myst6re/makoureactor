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
#ifndef PCFIELDFILE_H
#define PCFIELDFILE_H

#include <QtCore>
#include "../LzsSectionFile.h"

#define PC_FIELD_FILE_SECTION_COUNT		9

class PCFieldFile : public LzsSectionFile
{
public:
	enum Section {
		TextsAndScripts = 0,
		Camera,
		ModelLoader,
		Palette,
		Walkmesh,
		_TileMap,
		Encounter,
		Triggers,
		Background
	};

	PCFieldFile();

	inline QByteArray sectionData(Section id) {
		return LzsSectionFile::sectionData(quint8(id));
	}
	inline void setSectionData(Section id, const QByteArray &data) {
		LzsSectionFile::setSectionData(quint8(id), data);
	}
	inline quint8 sectionCount() const {
		return PC_FIELD_FILE_SECTION_COUNT;
	}
	inline quint32 sectionSize(Section id, bool &eof) const {
		return LzsSectionFile::sectionSize(quint8(id), eof);
	}
private:
	bool openHeader();
	int setSectionData(int pos, int oldSize,
					   const QByteArray &section,
					   QByteArray &out);
	bool writePositions(QByteArray &data);
	quint32 sectionPos(quint8 id) const;
};

#endif // PCFIELDFILE_H
