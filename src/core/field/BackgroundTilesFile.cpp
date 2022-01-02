/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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
#include "BackgroundTilesFile.h"
#include "BackgroundTilesIO.h"
#include "Field.h"

BackgroundTilesFile::BackgroundTilesFile(Field *field) :
      FieldPart(field)
{
}

BackgroundTilesFile::~BackgroundTilesFile()
{
}

bool BackgroundTilesFile::open()
{
	return open(field()->sectionData(Field::Tiles));
}

bool BackgroundTilesFile::open(const QByteArray &data)
{
	QBuffer buffer;
	buffer.setData(data);

	if (BackgroundTilesIOPS(&buffer).read(_tiles)) {
		setOpen(true);

		return true;
	}

	return false;
}

QByteArray BackgroundTilesFile::save() const
{
	QBuffer buffer;

	BackgroundTilesIOPS(&buffer).write(_tiles);

	return buffer.data();
}

void BackgroundTilesFile::clear()
{
	_tiles.clear();
}
