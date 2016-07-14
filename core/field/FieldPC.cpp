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
#include "FieldPC.h"
#include "BackgroundFilePC.h"

FieldPC::FieldPC(const QString &name, FieldArchiveIO *io) :
	Field(name, io), _file(new PCFieldFile()), _model(0)
{
}

FieldPC::FieldPC(const Field &field) :
	Field(field), _file(new PCFieldFile()), _model(0)
{
}

FieldPC::~FieldPC()
{
	delete _file;
	if(_model) {
		delete _model;
	}
}

bool FieldPC::open2()
{
	return _file->open(io()->fieldData(this, QString(), false));
}

FieldPart *FieldPC::createPart(FieldSection part)
{
	switch(part) {
	case ModelLoader:	return new FieldModelLoaderPC(this);
	case Background:	return new BackgroundFilePC(this);
	default:			return Field::createPart(part);
	}
}

int FieldPC::sectionSize(FieldSection part) const
{
	switch(part) {
	case Scripts:
	case Akaos:       return _file->sectionSize(PCFieldFile::TextsAndScripts);
	case Camera:      return _file->sectionSize(PCFieldFile::Camera);
	case Walkmesh:    return _file->sectionSize(PCFieldFile::Walkmesh);
	case ModelLoader: return _file->sectionSize(PCFieldFile::ModelLoader);
	case Encounter:   return _file->sectionSize(PCFieldFile::Encounter);
	case Inf:         return _file->sectionSize(PCFieldFile::Triggers);
	case Background:  return _file->sectionSize(PCFieldFile::Background);
	case PalettePC:   return _file->sectionSize(PCFieldFile::Palette);
	case Unused:      return _file->sectionSize(PCFieldFile::_TileMap);
	default:          return -1;
	}
}

QByteArray FieldPC::sectionData(FieldSection part)
{
	switch(part) {
	case Scripts:
	case Akaos:       return _file->sectionData(PCFieldFile::TextsAndScripts);
	case Camera:      return _file->sectionData(PCFieldFile::Camera);
	case Walkmesh:    return _file->sectionData(PCFieldFile::Walkmesh);
	case ModelLoader: return _file->sectionData(PCFieldFile::ModelLoader);
	case Encounter:   return _file->sectionData(PCFieldFile::Encounter);
	case Inf:         return _file->sectionData(PCFieldFile::Triggers);
	case Background:  return _file->sectionData(PCFieldFile::Background);
	case PalettePC:   return _file->sectionData(PCFieldFile::Palette);
	case Unused:      return _file->sectionData(PCFieldFile::_TileMap);
	default:          return QByteArray();
	}
}

FieldModelLoaderPC *FieldPC::fieldModelLoader(bool open)
{
	//if(open && !modelLoader->isOpen()) {
	//	Data::currentCharNames = model_nameChar;
	//	Data::currentHrcNames = &fieldModelLoader->model_nameHRC;
	//	Data::currentAnimNames = &fieldModelLoader->model_anims;
	//}
	return static_cast<FieldModelLoaderPC *>(Field::fieldModelLoader(open));
}

FieldModelFilePC *FieldPC::fieldModel(int modelID, int animationID, bool animate, bool open)
{
	FieldModelLoaderPC *modelLoader = fieldModelLoader();
	QString hrc = modelLoader->HRCName(modelID);
	QString a = modelLoader->AName(modelID, animationID);

	// Optimization: Prevent the loading of the same model twice
	int localModelID = modelNameToId.value(hrc.toLower(), -1);
	if(localModelID == -1) {
		localModelID = modelID;
		modelNameToId.insert(hrc.toLower(), localModelID);
	}

	FieldModelFilePC *fieldModel = new FieldModelFilePC();
	if(open) {
		fieldModel->load(hrc, a, animate);
	}
	return fieldModel;
}

FieldModelFilePC *FieldPC::fieldModel(const QString &hrc, const QString &a, bool animate)
{
	if(!_model)	_model = new FieldModelFilePC();
	_model->load(hrc, a, animate);
	return _model;
}

qint8 FieldPC::importer(const QByteArray &data, bool isPSField, FieldSections part)
{
	if(!isPSField) {
		quint32 sectionPositions[9];

		if(data.size() < 6 + 9 * 4)	return 3;
		memcpy(sectionPositions, data.constData() + 6, 9 * 4); // header

		if(part.testFlag(ModelLoader)) {
			FieldModelLoaderPC *modelLoader = fieldModelLoader(false);
			if(!modelLoader->open(data.mid(sectionPositions[2]+4, sectionPositions[3]-sectionPositions[2]-4))) {
				return 2;
			}
			modelLoader->setModified(true);
		}
	}

	return Field::importer(data, isPSField, part);
}
