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
	_shift(0)
{
}

const QByteArray &DatFile::data()
{
	writePositions();
	return _data;
}

bool DatFile::setData(const QByteArray &data)
{
	if (data.size() < DAT_FILE_HEADER_SIZE) {
		qWarning() << "DatFile::setData file too short:" << data.size();
		return false;
	}

	memcpy(_sectionPositions, data.constData(), DAT_FILE_HEADER_SIZE);

	_sectionPositions[DAT_FILE_SECTION_COUNT] = data.size();

	_shift = _sectionPositions[0] - DAT_FILE_HEADER_SIZE;

	for (int i=0; i<DAT_FILE_SECTION_COUNT; ++i) {
		_sectionPositions[i] -= _shift;
	}

	for (quint8 i=0; i<DAT_FILE_SECTION_COUNT; ++i) {
		if (_sectionPositions[i + 1] < _sectionPositions[i]) {
			qWarning() << "DatFile::setData Wrong order:" << i << _sectionPositions[i] << _sectionPositions[i + 1];
			return false;
		}
	}

	_data = data;

	return true;
}

QByteArray DatFile::sectionData(Section id) const
{
	Q_ASSERT(id >= 0 && id < DAT_FILE_SECTION_COUNT);
	return _data.mid(sectionPos(id), sectionSize(id));
}

const char *DatFile::sectionConstData(Section id, int &size) const
{
	Q_ASSERT(id >= 0 && id < DAT_FILE_SECTION_COUNT);
	size = sectionSize(id);
	return _data.constData() + sectionPos(id);
}

void DatFile::setSectionData(Section id, const QByteArray &data)
{
	Q_ASSERT(id >= 0 && id < DAT_FILE_SECTION_COUNT);
	int oldSize = sectionSize(id),
			newSize = data.size();

	_data.replace(sectionPos(id), oldSize, data);
	// 4-bytes alignment
	if(data.size() % 4 != 0) {
		int padding = 4 - data.size() % 4;
		_data.insert(sectionPos(id) + data.size(), QByteArray(padding, '\0'));
		newSize += padding;
	}
	shiftPositionsAfter(id, newSize - oldSize);
}

void DatFile::shiftPositionsAfter(int id, int shift)
{
	for (int i=id + 1; i<DAT_FILE_SECTION_COUNT + 1; ++i) {
		_sectionPositions[i] += shift;
	}
}

void DatFile::writePositions()
{
	quint32 sectionPositions[DAT_FILE_SECTION_COUNT];
	for (int i=0; i<DAT_FILE_SECTION_COUNT; ++i) {
		sectionPositions[i] = _sectionPositions[i] + _shift;
	}

	_data.replace(0, DAT_FILE_HEADER_SIZE, (char *)sectionPositions, DAT_FILE_HEADER_SIZE);
}
