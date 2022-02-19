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
#include "FieldArchivePS.h"

FieldArchivePS::FieldArchivePS() :
	FieldArchive()
{
}

FieldArchivePS::FieldArchivePS(const QString &path, FieldArchiveIO::Type type) :
	FieldArchive()
{
	switch (type) {
	case FieldArchiveIO::Dir:
		setIO(new FieldArchiveIOPSDir(path, this));
		break;
	case FieldArchiveIO::Iso:
		setIO(new FieldArchiveIOPSIso(path, this));
		break;
	case FieldArchiveIO::File:
		setIO(new FieldArchiveIOPSFile(path, this));
		break;
	default:
		break;
	}
}

FieldArchiveIOPS *FieldArchivePS::io() const
{
	return static_cast<FieldArchiveIOPS *>(FieldArchive::io());
}
