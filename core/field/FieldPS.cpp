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

void FieldPS::saveStart()
{
	_file->saveStart();
}

bool FieldPS::save2(QByteArray &data, bool compress, bool removeUnusedSection)
{
	Q_UNUSED(removeUnusedSection);

	return _file->save(data, compress);
}

void FieldPS::saveEnd()
{
	_file->saveEnd();
}

FieldPart *FieldPS::createPart(FieldSection section)
{
	switch(section) {
	case ModelLoader: return new FieldModelLoaderPS(this);
	case Background:  return new BackgroundFilePS(this);
	case Scripts:
	case Akaos:
	case Camera:
	case Walkmesh:
	case Encounter:
	case Inf:         return Field::createPart(section);
	}
	return 0;
}

QByteArray FieldPS::sectionData(CommonSection section)
{
	switch(section) {
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

bool FieldPS::setSectionData(CommonSection section, const QByteArray &data)
{
	switch(section) {
	case _ScriptsTextsAkaos:
		_file->setSectionData(DatFile::TextsAndScripts, data);
		return true;
	case _Camera:
		_file->setSectionData(DatFile::Camera, data);
		return true;
	case _Walkmesh:
		_file->setSectionData(DatFile::Walkmesh, data);
		return true;
	case _Encounter:
		_file->setSectionData(DatFile::Encounter, data);
		return true;
	case _Inf:
		_file->setSectionData(DatFile::Triggers, data);
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
