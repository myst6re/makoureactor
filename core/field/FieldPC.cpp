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

FieldPC::FieldPC(const QString &name, FieldArchiveIOPC *io) :
	Field(name, io), _model(0)
{
}

FieldPC::FieldPC(const Field &field) :
	Field(field), _model(0)
{
}

FieldPC::~FieldPC()
{
	if(_model) {
		delete _model;
	}
}

bool FieldPC::open2()
{
	return _file.openLzs(io()->fieldData(this, QString(), false));
}

void FieldPC::saveStart()
{
	_file.saveStart();
}

bool FieldPC::save2(QByteArray &data, bool compress)
{
	if (removeUnusedSection()) {
		_file.setSectionData(PCFieldFile::_TileMap, QByteArray());
	}
	return _file.save(data, compress);
}

void FieldPC::saveEnd()
{
	_file.saveEnd();
}

FieldPart *FieldPC::createPart(FieldSection section)
{
	switch(section) {
	case ModelLoader: return new FieldModelLoaderPC(this);
	case Background:  return new BackgroundFilePC(this);
	case Scripts:
	case Akaos:
	case Camera:
	case Walkmesh:
	case Encounter:
	case Inf:         return Field::createPart(section);
	}
	return 0;
}

QByteArray FieldPC::sectionData(CommonSection section)
{
	switch(section) {
	case _ScriptsTextsAkaos:
		return _file.sectionData(PCFieldFile::TextsAndScripts);
	case _Camera:
		return _file.sectionData(PCFieldFile::Camera);
	case _Walkmesh:
		return _file.sectionData(PCFieldFile::Walkmesh);
	case _Encounter:
		return _file.sectionData(PCFieldFile::Encounter);
	case _Inf:
		return _file.sectionData(PCFieldFile::Triggers);
	}
	return QByteArray();
}

bool FieldPC::setSectionData(CommonSection section, const QByteArray &data)
{
	switch(section) {
	case _ScriptsTextsAkaos:
		_file.setSectionData(PCFieldFile::TextsAndScripts, data);
		return true;
	case _Camera:
		_file.setSectionData(PCFieldFile::Camera, data);
		return true;
	case _Walkmesh:
		_file.setSectionData(PCFieldFile::Walkmesh, data);
		return true;
	case _Encounter:
		_file.setSectionData(PCFieldFile::Encounter, data);
		return true;
	case _Inf:
		_file.setSectionData(PCFieldFile::Triggers, data);
		return true;
	}
	return false;
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
