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
#include "FieldArchiveIO.h"
#include "Palette.h"
#include "LZS.h"
#include "Config.h"
#include "BackgroundFilePC.h"

FieldPC::FieldPC(const QString &name, FieldArchiveIO *io) :
	Field(name, io)
{
}

FieldPC::FieldPC(const Field &field) :
	Field(field)
{
}

void FieldPC::openHeader(const QByteArray &fileData)
{
	memcpy(sectionPositions, fileData.constData() + 6, 9 * 4); // header
}

int FieldPC::sectionId(FieldPart part) const
{
	switch(part) {
	case Scripts:		return 0;
	case Akaos:			return 0;
	case Camera:		return 1;
	case ModelLoader:	return 2;
	case PalettePC:		return 3;
	case Walkmesh:		return 4;
	case Encounter:		return 6;
	case Inf:			return 7;
	case Background:	return 8;
	default:			return -1;
	}
}

quint32 FieldPC::sectionPosition(int idPart)
{
	return sectionPositions[idPart] + paddingBetweenSections();
}

FieldArchiveIOPC *FieldPC::io() const
{
	return (FieldArchiveIOPC *)Field::io();
}

QPixmap FieldPC::openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
	return background()->openBackground(
				sectionData(Background),
				sectionData(PalettePC),
				paramActifs, z, layers);
}

FieldModelLoader *FieldPC::createFieldModelLoader() const
{
	return new FieldModelLoaderPC();
}

BackgroundFile *FieldPC::createBackground() const
{
	return new BackgroundFilePC();
}

FieldModelLoaderPC *FieldPC::fieldModelLoader(bool open)
{
	//if(open && !modelLoader->isLoaded()) {
	//	Data::currentCharNames = model_nameChar;
	//	Data::currentHrcNames = &fieldModelLoader->model_nameHRC;
	//	Data::currentAnimNames = &fieldModelLoader->model_anims;
	//}
	return (FieldModelLoaderPC *)Field::fieldModelLoader(open);
}

FieldModelFilePC *FieldPC::fieldModel(int modelID, int animationID, bool animate)
{
	FieldModelLoaderPC *modelLoader = fieldModelLoader();
	QString hrc = modelLoader->HRCName(modelID);
	QString a = modelLoader->AName(modelID, animationID);

	return fieldModel(hrc, a, animate);
}

FieldModelFilePC *FieldPC::fieldModel(const QString &hrc, const QString &a, bool animate)
{
	if(!_fieldModel)	_fieldModel = new FieldModelFilePC();
	((FieldModelFilePC *)_fieldModel)->load(hrc, a, animate);
	return (FieldModelFilePC *)_fieldModel;
}

bool FieldPC::save(QByteArray &newData, bool compress)
{
	newData = QByteArray();

	if(!isOpen()) {
		return false;
	}

	QByteArray decompresse = io()->fieldData(this), section, toc;
	const char *decompresseData = decompresse.constData();
	quint32 sectionPositions[9], size, section_size;

	if(decompresse.isEmpty()) {
		return false;
	}

	sectionPositions[0] = 42;
	for(quint8 i=1 ; i<9 ; ++i)
		memcpy(sectionPositions + i, decompresseData + 6+4*i, 4);

	// Header + pos section 1
	toc.append("\x00\x00", 2);
	toc.append("\x09\x00\x00\x00", 4); // section count
	toc.append("\x2A\x00\x00\x00", 4); // pos section 1

	// Section 1 (scripts + textes + akaos/tutos)
	if(section1 && section1->isModified()) {
		section = section1->save(decompresse.mid(sectionPositions[0]+4, sectionPositions[1]-sectionPositions[0]-4));
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(decompresseData + sectionPositions[0], sectionPositions[1]-sectionPositions[0]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 2

	// Section 2 (camera)
	section = QByteArray();
	if(ca && ca->isModified() && ca->save(section)) {
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(decompresseData + sectionPositions[1], sectionPositions[2]-sectionPositions[1]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 3

	// Section 3 (model loader PC)
	if(modelLoader && modelLoader->isLoaded() && modelLoader->isModified()) {
		section = fieldModelLoader()->save();
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(decompresseData + sectionPositions[2], sectionPositions[3]-sectionPositions[2]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 4
	
	// Section 4 (background palette PC)
	newData.append(decompresseData + sectionPositions[3], sectionPositions[4]-sectionPositions[3]);

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 5

	// Section 5 (walkmesh)
	section = QByteArray();
	if(id && id->isModified() && id->save(section)) {
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(decompresseData + sectionPositions[4], sectionPositions[5]-sectionPositions[4]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 6

	// Section 6 (background tileMap -unused-)
	newData.append(decompresseData + sectionPositions[5], sectionPositions[6]-sectionPositions[5]);

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 7

	// Section 7 (encounter)
	if(_encounter && _encounter->isModified()) {
		section = _encounter->save();
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(decompresseData + sectionPositions[6], sectionPositions[7]-sectionPositions[6]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 8

	// Section 8 (trigger)
	if(_inf && _inf->isModified()) {
		section = _inf->save();
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(decompresseData + sectionPositions[7], sectionPositions[8]-sectionPositions[7]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 9

	// Section 9 (background PC)
	newData.append(decompresse.mid(sectionPositions[8]));

	newData.prepend(toc);

//	if(decompresse != newData) {
//		QFile fic("test_"+name()+"_nouveau");
//		fic.open(QIODevice::WriteOnly);
//		fic.write(newData);
//		fic.close();
//		QFile fic2("test_"+name()+"_original");
//		fic2.open(QIODevice::WriteOnly);
//		fic2.write(decompresse);
//		fic2.close();
//		qDebug() << name() << " : ERROR";
////		newData = decompresse;
//	}

	if(compress)
	{
		const QByteArray &compresse = LZS::compress(newData);
		quint32 taille = compresse.size();
		newData = QByteArray((char *)&taille, 4).append(compresse);
		return true;
	}

	return true;
}

qint8 FieldPC::importer(const QByteArray &data, bool isPSField, FieldParts part)
{
	if(!isPSField) {
		quint32 sectionPositions[9];

		if(data.size() < 6 + 9 * 4)	return 3;
		memcpy(sectionPositions, &(data.constData()[6]), 9 * 4); // header

		if(part.testFlag(ModelLoader)) {
			FieldModelLoaderPC *modelLoader = fieldModelLoader(false);
			if(!modelLoader->load(data.mid(sectionPositions[2]+4, sectionPositions[3]-sectionPositions[2]-4))) {
				return 2;
			}
			modelLoader->setModified(true);
		}
	}

	return Field::importer(data, isPSField, part);
}
