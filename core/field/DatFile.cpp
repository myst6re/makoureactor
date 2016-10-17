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
#include "DatFile.h"

DatFile::DatFile() :
	LzsSectionFile()
{
}

bool DatFile::openHeader()
{
	if (io()->read(reinterpret_cast<char *>(_sectionPositions),
	               DAT_FILE_HEADER_SIZE) != DAT_FILE_HEADER_SIZE) {
		qWarning() << "DatFile::openHeader file too short" << io()->errorString();
		return false;
	}

	_shift = sectionPos(0) - DAT_FILE_HEADER_SIZE;

	for (quint8 i = 0; i < DAT_FILE_SECTION_COUNT; ++i) {
		_sectionPositions[i] -= _shift;
	}

	return true;
}

int DatFile::setSectionData(int pos, int oldSize,
								const QByteArray &section,
								QByteArray &out)
{
	int newSize = section.size();

	out.replace(pos, oldSize, section);
	// 4-bytes alignment
	if(newSize % 4 != 0) {
		int padding = 4 - newSize % 4;
		out.insert(pos + newSize, QByteArray(padding, '\0'));
		newSize += padding;
	}

	return newSize - oldSize;
}

bool DatFile::writePositions(QByteArray &data)
{
	quint32 sectionPositions[DAT_FILE_SECTION_COUNT];
	for (quint8 i = 0; i < DAT_FILE_SECTION_COUNT; ++i) {
		sectionPositions[i] = sectionPos(i) + _shift;
	}

	data.replace(0, DAT_FILE_HEADER_SIZE, reinterpret_cast<char *>(sectionPositions),
	             DAT_FILE_HEADER_SIZE);

	return true;
}
