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
#include "Field.h"
#include "LZS.h"
#include "FieldArchive.h"
#include "FieldPC.h"
#include "FieldPS.h"
#include "Data.h"

Field::Field(const QString &name) :
	_isOpen(false), _isModified(false), name(name.toLower()),
	section1(0), _encounter(0), _tut(0), id(0), ca(0), inf(0),
	modelLoader(0), fieldModel(0),
	fieldArchive(0)
{
}

Field::Field(const QString &name, FieldArchive *fieldArchive) :
	_isOpen(false), _isModified(false), name(name.toLower()),
	section1(0), _encounter(0), _tut(0), id(0), ca(0), inf(0),
	modelLoader(0), fieldModel(0),
	fieldArchive(fieldArchive)
{
}

Field::~Field()
{
	if(section1)		delete section1;
	if(_encounter)		delete _encounter;
	if(_tut)			delete _tut;
	if(id)				delete id;
	if(ca)				delete ca;
	if(inf)				delete inf;
	if(modelLoader)		delete modelLoader;
	if(fieldModel)		delete fieldModel;
}

bool Field::isOpen() const
{
	return _isOpen;
}

bool Field::isModified() const
{
	return _isModified;
}

void Field::setModified(bool modified)
{
	if(!_isOpen) {
		if(!open()) {
			qWarning() << "Unable to reopen!";
			return;
		}
	}
	_isModified = modified;
	if(section1)	section1->setModified(modified);
}

QRgb Field::blendColor(quint8 type, QRgb color0, QRgb color1)
{
	int r, g, b;

	switch(type) {
	case 1:
		r = qRed(color0) + qRed(color1);
		if(r>255)	r = 255;
		g = qGreen(color0) + qGreen(color1);
		if(g>255)	g = 255;
		b = qBlue(color0) + qBlue(color1);
		if(b>255)	b = 255;
		break;
	case 2:
		r = qRed(color0) - qRed(color1);
		if(r<0)	r = 0;
		g = qGreen(color0) - qGreen(color1);
		if(g<0)	g = 0;
		b = qBlue(color0) - qBlue(color1);
		if(b<0)	b = 0;
		break;
	case 3:
		r = qRed(color0) + 0.25*qRed(color1);
		if(r>255)	r = 255;
		g = qGreen(color0) + 0.25*qGreen(color1);
		if(g>255)	g = 255;
		b = qBlue(color0) + 0.25*qBlue(color1);
		if(b>255)	b = 255;
		break;
	default://0
		r = (qRed(color0) + qRed(color1))/2;
		g = (qGreen(color0) + qGreen(color1))/2;
		b = (qBlue(color0) + qBlue(color1))/2;
		break;
	}

	return qRgb(r, g, b);
}

Section1File *Field::scriptsAndTexts(bool open)
{
	if(!section1) 	section1 = new Section1File();
	if(open && !section1->isOpen())	section1->open(sectionData(Scripts));
	if(section1->isOpen())	Data::currentTextes = section1->texts();
	return section1;
}

EncounterFile *Field::encounter(bool open)
{
	if(!_encounter)	_encounter = new EncounterFile();
	if(open && !_encounter->isOpen())		_encounter->open(sectionData(Encounter));
	return _encounter;
}

TutFile *Field::tutosAndSounds(bool open)
{
	if(!_tut)	_tut = new TutFile();
	scriptsAndTexts(false)->setTut(_tut);
	if(open && !_tut->isOpen())	_tut->open(sectionData(Scripts));
	return _tut;
}

IdFile *Field::walkmesh(bool open)
{
	if(!id)		id = new IdFile();
	if(open && !id->isOpen())	id->open(sectionData(Walkmesh));
	return id;
}

CaFile *Field::camera(bool open)
{
	if(!ca)		ca = new CaFile();
	if(open && !ca->isOpen())	ca->open(sectionData(Camera));
	return ca;
}

InfFile *Field::getInf(bool open)
{
	if(!inf)	inf = new InfFile();
	if(open && !inf->isOpen())	inf->open(sectionData(Inf));
	return inf;
}

const QString &Field::getName() const
{
	return name;
}

void Field::setName(const QString &name)
{
	this->name = name;
	_isModified = true;
}

void Field::setSaved()
{
	_isOpen = false; // Force reopen to refresh positions automatically
	if(_encounter)	_encounter->setModified(false);
	if(_tut)		_tut->setModified(false);
	if(id)			id->setModified(false);
	if(ca)			ca->setModified(false);
	if(inf)			inf->setModified(false);
}

qint8 Field::save(const QString &path, bool compress)
{
	QByteArray newData;

	if(save(newData, compress)) {
		QFile fic(path);
		if(!fic.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			return 2;
		}
		fic.write(newData);
		fic.close();
	} else {
		return 1;
	}

	return 0;
}

qint8 Field::importer(const QString &path, int type, FieldParts part)
{
	QFile fic(path);
	if(!fic.open(QIODevice::ReadOnly))	return 1;
	if(fic.size() > 10000000)	return 2;

	QByteArray data;

	if(type == 0 || type == 1) // compressed field
	{
		quint32 fileSize=0;
		if(fic.read((char *)&fileSize, 4) != 4)	return 2;
		if(fileSize+4 != fic.size()) return 2;

		data = LZS::decompressAll(fic.readAll());
	}
	else if(type == 2 || type == 3) // uncompressed field
	{
		data = fic.readAll();
	}
	
	return importer(data, type == 1 || type == 3, part);
}

qint8 Field::importer(const QByteArray &data, bool isPSField, FieldParts part)
{
	if(isPSField) {
		quint32 sectionPositions[7];
		const int headerSize = 28;

		if(data.size() < headerSize)	return 2;
		memcpy(sectionPositions, data.constData(), headerSize); // header
		qint32 vramDiff = sectionPositions[0] - headerSize;// vram section1 pos - real section 1 pos

		for(int i=0 ; i<7 ; ++i) {
			sectionPositions[i] -= vramDiff;
		}

		if(part.testFlag(Scripts)) {
			Section1File *section1 = scriptsAndTexts(false);
			if(!section1->open(data.mid(sectionPositions[0], sectionPositions[1]-sectionPositions[0])))	return 2;
			if(section1->isOpen())	Data::currentTextes = section1->texts();
			section1->setModified(true);
		}
		if(part.testFlag(Akaos)) {
			TutFile *_tut = tutosAndSounds(false);
			if(!_tut->open(data.mid(sectionPositions[0], sectionPositions[1]-sectionPositions[0])))		return 2;
			_tut->setModified(true);
		}
		if(part.testFlag(Encounter)) {
			EncounterFile *enc = encounter(false);
			if(!enc->open(data.mid(sectionPositions[5], sectionPositions[6]-sectionPositions[5])))		return 2;
			enc->setModified(true);
		}
		if(part.testFlag(Walkmesh)) {
			IdFile *walk = walkmesh(false);
			if(!walk->open(data.mid(sectionPositions[1], sectionPositions[2]-sectionPositions[1])))		return 2;
			walk->setModified(true);
		}
		if(part.testFlag(Camera)) {
			CaFile *ca = camera(false);
			if(!ca->open(data.mid(sectionPositions[3], sectionPositions[4]-sectionPositions[3])))		return 2;
			ca->setModified(true);
		}
		if(part.testFlag(Inf)) {
			InfFile *inf = getInf(false);
			if(!inf->open(data.mid(sectionPositions[4], sectionPositions[5]-sectionPositions[4]), true))	return 2;
			inf->setModified(true);
		}
	} else {
		quint32 sectionPositions[9];

		if(data.size() < 6 + 9 * 4)	return 3;
		memcpy(sectionPositions, data.constData() + 6, 9 * 4); // header

		if(part.testFlag(Scripts)) {
			Section1File *section1 = scriptsAndTexts(false);
			if(!section1->open(data.mid(sectionPositions[0]+4, sectionPositions[1]-sectionPositions[0]-4)))	return 2;
			section1->setModified(true);
		}
		if(part.testFlag(Akaos)) {
			TutFile *_tut = tutosAndSounds(false);
			if(!_tut->open(data.mid(sectionPositions[0]+4, sectionPositions[1]-sectionPositions[0]-4)))		return 2;
			_tut->setModified(true);
		}
		if(part.testFlag(Encounter)) {
			EncounterFile *enc = encounter(false);
			if(!enc->open(data.mid(sectionPositions[6]+4, sectionPositions[7]-sectionPositions[6]-4)))		return 2;
			enc->setModified(true);
		}
		if(part.testFlag(Walkmesh)) {
			IdFile *walk = walkmesh(false);
			if(!walk->open(data.mid(sectionPositions[4]+4, sectionPositions[5]-sectionPositions[4]-4)))		return 2;
			walk->setModified(true);
		}
		if(part.testFlag(Camera)) {
			CaFile *ca = camera(false);
			if(!ca->open(data.mid(sectionPositions[1]+4, sectionPositions[2]-sectionPositions[1]-4)))		return 2;
			ca->setModified(true);
		}
		if(part.testFlag(Inf)) {
			InfFile *inf = getInf(false);
			if(!inf->open(data.mid(sectionPositions[7]+4, sectionPositions[8]-sectionPositions[7]-4), true))	return 2;
			inf->setModified(true);
		}
	}

	return 0;
}
