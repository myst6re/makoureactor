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
#define PC_FIELD_FILE_SECTION_COUNT		9

class PCFieldFile
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

	const QByteArray &data();
	inline bool setData(const char *data, int size) {
		return setData(QByteArray(data, size));
	}
	bool setData(const QByteArray &data);
	QByteArray sectionData(Section id) const;
	const char *sectionConstData(Section id, int &size) const;
	void setSectionData(Section id, const QByteArray &data);
private:
	inline int sectionSize(int id) const {
		return sectionPos(id + 1) - sectionPos(id);
	}
	inline int sectionPos(int id) const {
		return _sectionPositions[id];
	}
	void shiftPositionsAfter(int id, int shift);
	void writePositions();
	quint32 _sectionPositions[10];
	QByteArray _data;
};

#endif // PCFIELDFILE_H
