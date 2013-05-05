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
#include "../LZS.h"
#include "../Config.h"

FieldPS::FieldPS(const QString &name, FieldArchiveIO *io) :
	Field(name, io)
{
}

FieldPS::FieldPS(const Field &field) :
	Field(field)
{
}

void FieldPS::openHeader(const QByteArray &fileData)
{
	memcpy(sectionPositions, fileData.constData(), headerSize()); // header
	vramDiff = sectionPositions[0] - headerSize();// vram section1 pos - real section 1 pos

	for(int i=0 ; i<7 ; ++i) {
		sectionPositions[i] -= vramDiff;
	}
}

int FieldPS::sectionId(FieldSection part) const
{
	switch(part) {
	case Scripts:		return 0;
	case Akaos:			return 0;
	case Walkmesh:		return 1;
	case Background:	return 2;
	case Camera:		return 3;
	case Inf:			return 4;
	case Encounter:		return 5;
	case ModelLoader:	return 6;
	default:			return -1;
	}
}

quint32 FieldPS::sectionPosition(int idPart) const
{
	return sectionPositions[idPart];
}

FieldArchiveIOPS *FieldPS::io() const
{
	return (FieldArchiveIOPS *)Field::io();
}

FieldPart *FieldPS::createPart(FieldSection part)
{
	switch(part) {
	case ModelLoader:	return new FieldModelLoaderPS(this);
	case Background:	return new BackgroundFilePS(this);
	default:			return Field::createPart(part);
	}
}

FieldModelLoaderPS *FieldPS::fieldModelLoader(bool open)
{
	return (FieldModelLoaderPS *)Field::fieldModelLoader(open);
}

FieldModelFilePS *FieldPS::fieldModel(int modelID, int animationID, bool animate)
{
	FieldModelFilePS *fieldModel = (FieldModelFilePS *)fieldModelPtr(modelID);
	if(!fieldModel) 	addFieldModel(modelID, fieldModel = new FieldModelFilePS());
	if(!fieldModel->isOpen()) {
		fieldModel->load(this, modelID, animationID, animate);
	}
	return fieldModel;
}

QByteArray FieldPS::saveHeader() const
{
	// No header
	return QByteArray();
}

QByteArray FieldPS::saveFooter() const
{
	// No footer
	return QByteArray();
}

QList<Field::FieldSection> FieldPS::orderOfSections() const
{
	return QList<FieldSection>() << Scripts << Walkmesh << Background << Camera << Inf << Encounter << ModelLoader;
}
