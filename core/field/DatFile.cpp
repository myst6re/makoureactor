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

DatFile::DatFile()
{
}

bool DatFile::save(QByteArray &data)
{

	return false;
}

void DatFile::saveStart()
{
	if (!_io.isOpen()) {
		qWarning() << "DatFile::saveStart io closed";
		return;
	}

	_data = _io.readAll();
}

void DatFile::saveClear()
{
	_data.clear();
}

bool DatFile::saveFlush(QByteArray &data)
{
	writePositions();
	data = _data; // Apply
	saveClear();

	return true;
}

bool DatFile::open(const QByteArray &data)
{
	if (!_data.isEmpty()) {
		qWarning() << "DatFile::open cannot open at saving state";
		return false;
	}

	_io = LzsRandomAccess(data);
	if (!_io.open(QIODevice::ReadOnly)) {
		qWarning() << "DatFile::setData cannot open io" << _io.errorString();
		return false;
	}

	if (_io.read((char *)_sectionPositions, DAT_FILE_HEADER_SIZE) != DAT_FILE_HEADER_SIZE) {
		qWarning() << "DatFile::setData file too short" << _io.errorString();
		return false;
	}

	_shift = _sectionPositions[0] - DAT_FILE_HEADER_SIZE;

	for (quint8 i = 0; i < DAT_FILE_SECTION_COUNT; ++i) {
		_sectionPositions[i] -= _shift;
	}

	for (quint8 i = 0; i < DAT_FILE_SECTION_COUNT - 1; ++i) {
		if (_sectionPositions[i + 1] < _sectionPositions[i]) {
			qWarning() << "DatFile::setData Wrong order:" << i << _sectionPositions[i] << _sectionPositions[i + 1];
			return false;
		}
	}

	return true;
}

int DatFile::sectionSize(int id) const
{
	if (id + 1 >= DAT_FILE_SECTION_COUNT) {
		return -1;
	}
	return sectionPos(id + 1) - sectionPos(id);
}

QByteArray DatFile::sectionData(Section id) const
{
	if (_io.seek(sectionPos(id))) {
		int size = sectionSize(id);
		if (size < 0) {
			return _io.readAll();
		}
		return _io.read(size);
	}
	qWarning() << "DatFile::sectionData cannot seek to" << id;
	return QByteArray();
}

void DatFile::setSectionData(Section id, const QByteArray &data)
{
	if (_data.isEmpty()) {
		qWarning() << "DatFile::setSectionData not saving state, please use saveStart() before";
		return;
	}

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
