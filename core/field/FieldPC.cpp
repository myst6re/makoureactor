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

void FieldPC::openHeader(const QByteArray &fileData)
{
	memcpy(sectionPositions, fileData.constData() + 6, 9 * 4); // header
}

int FieldPC::sectionId(FieldSection part) const
{
	switch(part) {
	case Scripts:
	case Akaos:			return 0;
	case Camera:		return 1;
	case ModelLoader:	return 2;
	case PalettePC:		return 3;
	case Walkmesh:		return 4;
	case Unused:		return 5;
	case Encounter:		return 6;
	case Inf:			return 7;
	case Background:	return 8;
	default:			return -1;
	}
}

quint32 FieldPC::sectionPosition(int idPart) const
{
	return sectionPositions[idPart] + paddingBetweenSections();
}

FieldArchiveIOPC *FieldPC::io() const
{
	return static_cast<FieldArchiveIOPC *>(Field::io());
}

FieldPart *FieldPC::createPart(FieldSection part)
{
	switch(part) {
	case ModelLoader:	return new FieldModelLoaderPC(this);
	case Background:	return new BackgroundFilePC(this);
	default:			return Field::createPart(part);
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
	if(open && fieldModel->isEmpty()) {
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

QByteArray FieldPC::saveHeader() const
{
	QByteArray header;
	header.append("\x00\x00", 2); // Padding?
	header.append("\x09\x00\x00\x00", 4); // section count (=9)
	return header;
}

QByteArray FieldPC::saveFooter() const
{
	return QByteArray("FINAL FANTASY7", 14);
}

QList<Field::FieldSection> FieldPC::orderOfSections() const
{
	return QList<FieldSection>() << Scripts << Camera << ModelLoader << PalettePC << Walkmesh << Unused << Encounter << Inf << Background;
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
