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
	case Inf:
	case PalettePC:
	case Unused:      return Field::createPart(part);
	}
	return Field::createPart(part);
}

QByteArray FieldPS::sectionData(FieldSection part)
{
	switch(part) {
	case Scripts:
	case Akaos:       return _file->sectionData(DatFile::TextsAndScripts);
	case Camera:      return _file->sectionData(DatFile::Camera);
	case Walkmesh:    return _file->sectionData(DatFile::Walkmesh);
	case ModelLoader: return _file->sectionData(DatFile::ModelLoader);
	case Encounter:   return _file->sectionData(DatFile::Encounter);
	case Inf:         return _file->sectionData(DatFile::Triggers);
	case Background:  return _file->sectionData(DatFile::TileMap);
	case PalettePC:
	case Unused:      return QByteArray();
	}
	return QByteArray();
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
