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
#include "FieldArchiveIO.h"
#include "Palette.h"
#include "LZS.h"
#include "Config.h"
#include "BackgroundFilePS.h"

FieldPS::FieldPS(const QString &name, FieldArchiveIO *fieldArchive) :
	Field(name, fieldArchive)
{
}

FieldPS::FieldPS(const Field &field) :
	Field(field)
{
}

void FieldPS::openHeader(const QByteArray &fileData)
{
	memcpy(sectionPositions, fileData.constData(), headerSize()); // header
	qint32 vramDiff = sectionPositions[0] - headerSize();// vram section1 pos - real section 1 pos

	for(int i=0 ; i<7 ; ++i) {
		sectionPositions[i] -= vramDiff;
	}
}

int FieldPS::sectionId(FieldPart part) const
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

quint32 FieldPS::sectionPosition(int idPart)
{
	return sectionPositions[idPart];
}

QPixmap FieldPS::openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
	return background()->openBackground(
				sectionData(Background),
				fieldArchive->mimData(this),
				paramActifs, z, layers);
}

FieldModelLoader *FieldPS::createFieldModelLoader() const
{
	return new FieldModelLoaderPS();
}

BackgroundFile *FieldPS::createBackground() const
{
	return new BackgroundFilePS();
}

FieldModelLoaderPS *FieldPS::fieldModelLoader(bool open)
{
	return (FieldModelLoaderPS *)Field::fieldModelLoader(open);
}

FieldModelFilePS *FieldPS::fieldModel(int modelID, int animationID, bool animate)
{
	if(!_fieldModel) 	_fieldModel = new FieldModelFilePS();
	((FieldModelFilePS *)_fieldModel)->load(fieldArchive, this, modelID, animationID, animate);
	return (FieldModelFilePS *)_fieldModel;
}

bool FieldPS::save(QByteArray &newData, bool compress)
{
	if(!isOpen())	return false;

	QByteArray decompresse = fieldArchive->fieldData(this), toc;
	const char *decompresseData = decompresse.constData();
	quint32 padd, pos, debutSections[9];

	if(decompresse.isEmpty())	return false;

	for(quint8 i=0 ; i<7 ; ++i)
		memcpy(debutSections + i, decompresseData + 4*i, 4);

	padd = debutSections[0] - 28;

	toc.append((char *)&debutSections[0], 4);

	// Section 1 (scripts + textes + akaos/tutos)
	if(section1 && section1->isModified()) {
		newData.append(section1->save(decompresse.mid(28, debutSections[1]-debutSections[0])));
	} else {
		newData.append(decompresse.mid(28, debutSections[1]-debutSections[0]));
	}
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 2 (walkmesh)
	QByteArray section;
	if(id && id->isModified() && id->save(section)) {
		newData.append(section);
	} else {
		newData.append(decompresse.mid(debutSections[1]-padd, debutSections[2]-debutSections[1]));
	}
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 3 (background tileMap)
	newData.append(decompresse.mid(debutSections[2]-padd, debutSections[3]-debutSections[2]));
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 4 (camera)
	section = QByteArray();
	if(ca && ca->isModified() && ca->save(section)) {
		newData.append(section);
	} else {
		newData.append(decompresse.mid(debutSections[3]-padd, debutSections[4]-debutSections[3]));
	}
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 5 (trigger)
	if(_inf && _inf->isModified()) {
		newData.append(_inf->save());
	} else {
		newData.append(decompresse.mid(debutSections[4]-padd, debutSections[5]-debutSections[4]));
	}
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 6 (encounter)
	if(_encounter && _encounter->isModified()) {
		newData.append(_encounter->save());
	} else {
		newData.append(decompresse.mid(debutSections[5]-padd, debutSections[6]-debutSections[5]));
	}
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 7 (model loader PS)
	newData.append(decompresse.mid(debutSections[6]-padd));

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
//		newData = decompresse;
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
