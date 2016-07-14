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
#include "FieldPS.h"
#include "BackgroundFilePS.h"

FieldPS::FieldPS(const QString &name, FieldArchiveIO *io) :
	Field(name, io), _file(new DatFile())
{
}

FieldPS::FieldPS(const Field &field) :
	Field(field), _file(new DatFile())
{
}

bool FieldPS::open2()
{
	QByteArray lzsData = io()->fieldData(this, "DAT", false);

	return _file->open(lzsData);
}

bool FieldPS::save2(QByteArray &data)
{
	_file->saveStart();

	QHashIterator<FieldSection, FieldPart *> it(parts());
	while (it.hasNext()) {
		it.next();
		FieldPart *part = it.value();
		if (part && part->isOpen() && part->isModified()) {
			if (!setSectionData(it.key(), part)) {
				return false;
			}
		}
	}

	bool ok = _file->save(data);

	_file->saveEnd();

	return ok;
}

FieldPart *FieldPS::createPart(FieldSection part)
{
	switch(part) {
	case ModelLoader: return new FieldModelLoaderPS(this);
	case Background:  return new BackgroundFilePS(this);
	case Scripts:
	case Akaos:
	case Camera:
	case Walkmesh:
	case Encounter:
	case Inf:         return Field::createPart(part);
	}
	return 0;
}

QByteArray FieldPS::sectionData(CommonSection part)
{
	switch(part) {
	case _ScriptsTextsAkaos:
		return _file->sectionData(DatFile::TextsAndScripts);
	case _Camera:
		return _file->sectionData(DatFile::Camera);
	case _Walkmesh:
		return _file->sectionData(DatFile::Walkmesh);
	case _Encounter:
		return _file->sectionData(DatFile::Encounter);
	case _Inf:
		return _file->sectionData(DatFile::Triggers);
	}
	return QByteArray();
}

bool FieldPS::setSectionData(FieldSection section, FieldPart *part)
{
	switch(section) {
	case Scripts:
	case Akaos:
		_file->setSectionData(DatFile::TextsAndScripts, part->save());
		return true;
	case Camera:
		_file->setSectionData(DatFile::Camera, part->save());
		return true;
	case Walkmesh:
		_file->setSectionData(DatFile::Walkmesh, part->save());
		return true;
	case ModelLoader:
		_file->setSectionData(DatFile::ModelLoader, part->save());
		return true;
	case Encounter:
		_file->setSectionData(DatFile::Encounter, part->save());
		return true;
	case Inf:
		_file->setSectionData(DatFile::Triggers, part->save());
		return true;
	}
	return false;
}

FieldModelLoaderPS *FieldPS::fieldModelLoader(bool open)
{
	return static_cast<FieldModelLoaderPS *>(Field::fieldModelLoader(open));
}

FieldModelFilePS *FieldPS::fieldModel(int modelID, int animationID, bool animate, bool open)
{
	FieldModelFilePS *fieldModel = new FieldModelFilePS();
	if(open) {
		fieldModel->load(this, modelID, animationID, animate);
	}
	return fieldModel;
}
