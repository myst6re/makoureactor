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
#include "../LZS.h"
#include "FieldArchiveIO.h"
#include "FieldPC.h"
#include "FieldPS.h"
#include "../../Data.h"
#include "../Config.h"

Field::Field(const QString &name) :
	section1(0), _encounter(0), _tut(0), id(0), ca(0), _inf(0),
	modelLoader(0), _fieldModel(0), _bg(0),
	_io(0),
	_isOpen(false), _isModified(false), _name(name.toLower())
{
}

Field::Field(const QString &name, FieldArchiveIO *io) :
	section1(0), _encounter(0), _tut(0), id(0), ca(0), _inf(0),
	modelLoader(0), _fieldModel(0), _bg(0),
	_io(io),
	_isOpen(false), _isModified(false), _name(name.toLower())
{
}

Field::~Field()
{
	if(section1)		delete section1;
	if(_encounter)		delete _encounter;
	if(_tut)			delete _tut;
	if(id)				delete id;
	if(ca)				delete ca;
	if(_inf)			delete _inf;
	if(modelLoader)		delete modelLoader;
	if(_fieldModel)		delete _fieldModel;
	if(_bg)				delete _bg;
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

bool Field::open(bool dontOptimize)
{
	QByteArray fileData;

	if(!dontOptimize && !_io->fieldDataIsCached(this)) {
		QByteArray lzsData = _io->fieldData(this, false);

		if(lzsData.size() < 4)	return false;

		const char *lzsDataConst = lzsData.constData();
		quint32 lzsSize;
		memcpy(&lzsSize, lzsDataConst, 4);

		if(!Config::value("lzsNotCheck").toBool() && (quint32)lzsData.size() != lzsSize + 4)
			return false;

		fileData = LZS::decompress(lzsDataConst + 4, qMin(lzsSize, quint32(lzsData.size() - 4)), headerSize());//partial decompression
	} else {
		fileData = _io->fieldData(this);
	}

	if(fileData.size() < headerSize())	return false;

	openHeader(fileData);

	_isOpen = true;

	return true;
}

QByteArray Field::sectionData(FieldPart part)
{
	if(!_isOpen) {
		open();
	}

	if(!_isOpen)	return QByteArray();

	int idPart = sectionId(part);
	int position = sectionPosition(idPart);
	int size;

	if(idPart < sectionCount() - 1) {
		size = sectionPosition(idPart+1) - paddingBetweenSections() - position;
	} else {
		size = -1;
	}

	if(size == -1 || _io->fieldDataIsCached(this)) {
		return _io->fieldData(this).mid(position, size);
	} else {
		QByteArray lzsData = _io->fieldData(this, false);

		if(lzsData.size() < 4) {
			return QByteArray();
		}

		const char *lzsDataConst = lzsData.constData();
		quint32 lzsSize;
		memcpy(&lzsSize, lzsDataConst, 4);

		if(!Config::value("lzsNotCheck").toBool() && (quint32)lzsData.size() != lzsSize + 4) {
			return QByteArray();
		}

		return LZS::decompress(lzsDataConst + 4, qMin(lzsSize, quint32(lzsData.size() - 4)), sectionPosition(idPart+1))
				.mid(position, size);
	}
}

FieldArchiveIO *Field::io() const
{
	return _io;
}

QPixmap Field::openBackground()
{
	// Search default background params
	QHash<quint8, quint8> paramActifs;
	qint16 z[] = {-1, -1};
	scriptsAndTexts()->bgParamAndBgMove(paramActifs, z);

	return openBackground(paramActifs, z);
}

bool Field::usedParams(QHash<quint8, quint8> &usedParams, bool *layerExists)
{
	return background()->usedParams(sectionData(Background), usedParams, layerExists);
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

TutFileStandard *Field::tutosAndSounds(bool open)
{
	if(!_tut)	_tut = new TutFileStandard();
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

InfFile *Field::inf(bool open)
{
	if(!_inf)	_inf = new InfFile();
	if(open && !_inf->isOpen())	_inf->open(sectionData(Inf));
	return _inf;
}

FieldModelLoader *Field::fieldModelLoader(bool open)
{
	if(!modelLoader)	modelLoader = createFieldModelLoader();
	if(open && !modelLoader->isLoaded()) {
		modelLoader->load(sectionData(ModelLoader));
	}

	return modelLoader;
}

BackgroundFile *Field::background()
{
	if(!_bg)	_bg = createBackground();
	return _bg;
}

const QString &Field::name() const
{
	return _name;
}

void Field::setName(const QString &name)
{
	_name = name;
	_isModified = true;
}

void Field::setSaved()
{
	_isOpen = false; // Force reopen to refresh positions automatically
	if(_encounter)	_encounter->setModified(false);
	if(_tut)		_tut->setModified(false);
	if(id)			id->setModified(false);
	if(ca)			ca->setModified(false);
	if(_inf)		_inf->setModified(false);
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
			TutFileStandard *_tut = tutosAndSounds(false);
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
			InfFile *inf = this->inf(false);
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
			if(section1->isOpen())	Data::currentTextes = section1->texts();
			section1->setModified(true);
		}
		if(part.testFlag(Akaos)) {
			TutFileStandard *_tut = tutosAndSounds(false);
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
			InfFile *inf = this->inf(false);
			if(!inf->open(data.mid(sectionPositions[7]+4, sectionPositions[8]-sectionPositions[7]-4), true))	return 2;
			inf->setModified(true);
		}
	}

	return 0;
}
