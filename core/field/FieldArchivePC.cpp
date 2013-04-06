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
#include "FieldArchivePC.h"

FieldArchivePC::FieldArchivePC() :
	FieldArchive()
{
}

FieldArchivePC::FieldArchivePC(const QString &path, FieldArchiveIO::Type type) :
	FieldArchive()
{
	switch(type) {
	case FieldArchiveIO::Lgp:
		setIO(new FieldArchiveIOPCLgp(path, this));
		break;
	case FieldArchiveIO::File:
		setIO(new FieldArchiveIOPCFile(path, this));
		break;
	case FieldArchiveIO::Dir:
		setIO(new FieldArchiveIOPCDir(path, this));
		break;
	default:
		break;
	}
}

FieldArchivePC::~FieldArchivePC()
{
	foreach(TutFile *tut, _tuts) {
		if(tut != NULL)	delete tut;
	}
}

void FieldArchivePC::clear()
{
	foreach(TutFile *tut, _tuts) {
		if(tut != NULL)	delete tut;
	}

	_tuts.clear();

	FieldArchive::clear();
}

TutFile *FieldArchivePC::tut(const QString &name)
{
	QMapIterator<QString, TutFile *> it(_tuts);

	while(it.hasNext()) {
		it.next();

		const QString &tutName = it.key();

		if(name.startsWith(tutName, Qt::CaseInsensitive)) {
			TutFile *tutFile = it.value();
			if(tutFile == NULL) {
				QByteArray data = io()->fileData(tutName + ".tut", false, false);
				if(!data.isEmpty()) {
					tutFile = new TutFile(data, true);
					_tuts.insert(tutName, tutFile);
					return tutFile;
				} else {
					return NULL;
				}
			} else {
				return tutFile;
			}
		}
	}

	return NULL;
}

const QMap<QString, TutFile *> &FieldArchivePC::tuts() const
{
	return _tuts;
}

void FieldArchivePC::addTut(const QString &name)
{
	_tuts.insert(name, NULL);
}

void FieldArchivePC::setSaved()
{
	foreach(TutFile *tut, _tuts) {
		if(tut != NULL) {
			tut->setModified(false);
		}
	}

	FieldArchive::setSaved();
}

FieldArchiveIOPC *FieldArchivePC::io() const
{
	return (FieldArchiveIOPC *)FieldArchive::io();
}
