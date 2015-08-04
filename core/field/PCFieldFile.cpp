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
#include "PCFieldFile.h"

PCFieldFile::PCFieldFile()
{
}

const QByteArray &PCFieldFile::data()
{
	writePositions();
	return _data;
}

bool PCFieldFile::setData(const QByteArray &data)
{
	if (data.size() < 46) {
		qWarning() << "PCFieldFile::setData file too short:" << data.size();
		return false;
	}

	memcpy(_sectionPositions, data.constData() + 6, PC_FIELD_FILE_SECTION_COUNT * 4);

	if (_sectionPositions[0] != 42) {
		qWarning() << "PCFieldFile::setData first position must be 42:" << _sectionPositions[0];
		return false;
	}

	_sectionPositions[PC_FIELD_FILE_SECTION_COUNT] = data.size();

	for (int i = 0; i < PC_FIELD_FILE_SECTION_COUNT; ++i) {
		if (_sectionPositions[i + 1] < _sectionPositions[i]) {
			qWarning() << "PCFieldFile::setData Wrong order:" << _sectionPositions[i] << _sectionPositions[i + 1];
			return false;
		}
	}

	_data = data;

	return true;
}

QByteArray PCFieldFile::sectionData(Section id) const
{
	Q_ASSERT(id >= 0 && id < PC_FIELD_FILE_SECTION_COUNT);
	return _data.mid(sectionPos(id) + 4, sectionSize(id) - 4);
}

const char *PCFieldFile::sectionConstData(Section id, int &size) const
{
	Q_ASSERT(id >= 0 && id < PC_FIELD_FILE_SECTION_COUNT);
	size = sectionSize(id) - 4;
	return _data.constData() + sectionPos(id) + 4;
}

void PCFieldFile::setSectionData(Section id, const QByteArray &data)
{
	Q_ASSERT(id >= 0 && id < PC_FIELD_FILE_SECTION_COUNT);
	int pos = sectionPos(id),
			oldSize = sectionSize(id) - 4,
			newSize = data.size();

	_data.replace(pos, 4, (char *)&newSize, 4);
	_data.replace(pos + 4, oldSize, data);
	shiftPositionsAfter(id, newSize - oldSize);
}

void PCFieldFile::shiftPositionsAfter(int id, int shift)
{
	for (int i = id + 1; i < PC_FIELD_FILE_SECTION_COUNT + 1; ++i) {
		_sectionPositions[i] += shift;
	}
}

void PCFieldFile::writePositions()
{
	int size = PC_FIELD_FILE_SECTION_COUNT * 4;
	_data.replace(6, size, (char *)_sectionPositions, size);
}
