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
#include "FieldArchiveIO.h"
#include "FieldPC.h"
#include "FieldPS.h"
#include "../LZS.h"
#include "../Config.h"
#include "BackgroundFilePC.h"

Field::Field(const QString &name, FieldArchiveIO *io) :
	_io(io), _isOpen(false), _isModified(false),
	_name(name.toLower()), _removeUnusedSection(false)
{
}

Field::~Field()
{
	foreach(FieldPart *part, _parts) {
		if(part)	delete part;
	}

	if(currentFieldForFieldModels != this) {
		qDeleteAll(_fieldModels);
		_fieldModels.clear();
	}
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
	FieldPart *section1 = part(Scripts);
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

int Field::sectionSize(FieldSection part) const
{
	int idPart = sectionId(part);

	if(idPart < sectionCount() - 1) {
		return sectionPosition(idPart+1) - paddingBetweenSections() - sectionPosition(idPart);
	}
	return -1;
}

QByteArray Field::sectionData(FieldSection part, bool dontOptimize)
{
	if(!_isOpen) {
		open();
	}

	if(!_isOpen)	return QByteArray();

	int idPart = sectionId(part);
	int position = sectionPosition(idPart);
	int size = sectionSize(part);

	if(size < 0) {
		dontOptimize = true;
	}

	if(dontOptimize || _io->fieldDataIsCached(this)) {
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

FieldPart *Field::createPart(FieldSection section)
{
	switch(section) {
	case Scripts:		return new Section1File(this);
	case Akaos:			return new TutFileStandard(this);
	case Camera:		return new CaFile(this);
//	case PalettePC:		return ;
	case Walkmesh:		return new IdFile(this);
	case Encounter:		return new EncounterFile(this);
	case Inf:			return new InfFile(this);
	default:			return 0;
	}
}

FieldPart *Field::part(FieldSection section) const
{
	return _parts.value(section);
}

FieldPart *Field::part(FieldSection section, bool open)
{
	FieldPart *p = part(section);

	if(!p) {
		p = createPart(section);
		_parts.insert(section, p);
	}

	if(open && !p->isOpen()) {
		p->open();
	}

	return p;
}

Section1File *Field::scriptsAndTexts(bool open)
{
	return (Section1File *)part(Scripts, open);
}

EncounterFile *Field::encounter(bool open)
{
	return (EncounterFile *)part(Encounter, open);
}

TutFileStandard *Field::tutosAndSounds(bool open)
{
	TutFileStandard *tut = (TutFileStandard *)part(Akaos, open);
	scriptsAndTexts(false)->setTut(tut);
	return tut;
}

IdFile *Field::walkmesh(bool open)
{
	return (IdFile *)part(Walkmesh, open);
}

CaFile *Field::camera(bool open)
{
	return (CaFile *)part(Camera, open);
}

InfFile *Field::inf(bool open)
{
	return (InfFile *)part(Inf, open);
}

FieldModelLoader *Field::fieldModelLoader(bool open)
{
	return (FieldModelLoader *)part(ModelLoader, open);
}

BackgroundFile *Field::background(bool open)
{
	return (BackgroundFile *)part(Background, open);
}

Field *Field::currentFieldForFieldModels = 0;
QMap<int, FieldModelFile *> Field::_fieldModels;

FieldModelFile *Field::fieldModelPtr(int modelID) const
{
	if(currentFieldForFieldModels == this) {
		return _fieldModels.value(modelID, 0);
	}
	return 0;
}

void Field::addFieldModel(int modelID, FieldModelFile *fieldModel)
{
	if(currentFieldForFieldModels != this) {
		qDeleteAll(_fieldModels);
		_fieldModels.clear();
		currentFieldForFieldModels = this;
	}

	_fieldModels.insert(modelID, fieldModel);
}

QMap<int, FieldModelFile *> Field::fieldModels(bool animate, bool open)
{
	QMap<int, FieldModelFile *> ret;

	int modelCount = scriptsAndTexts()->modelCount();
	for(int modelId=0 ; modelId < modelCount ; ++modelId) {
		ret.insert(modelId, fieldModel(modelId, 0, animate, open));
	}

	return ret;
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
	foreach(FieldPart *part, _parts) {
		part->setModified(false);
	}
}

bool Field::save(QByteArray &newData, bool compress)
{
	newData = QByteArray();

	if(!isOpen()) {
		return false;
	}

	QByteArray toc;

	// Header
	toc.append(saveHeader());

	// Sections
	int id=0;
	foreach(const FieldSection &fieldSection, orderOfSections()) {
		// Section position
		quint32 pos = headerSize() + newData.size() + diffSectionPos();
		toc.append((char *)&pos, 4);

		QByteArray section;

		if(fieldSection != Unused || !_removeUnusedSection) { // FIXME: ugly hack only for PC version

			// Section data
			FieldPart *fieldPart = part(fieldSection == Field::PalettePC
										? Field::Background // FIXME: EXCEPTION NEEDS TO BE REMOVED IN THE FUTURE
										: fieldSection);

			if(fieldPart && fieldPart->canSave() &&
					fieldPart->isOpen() && fieldPart->isModified()) {
				if(fieldSection == Field::PalettePC) { // FIXME: EXCEPTION NEEDS TO BE REMOVED IN THE FUTURE
					section = ((BackgroundFilePC *)fieldPart)->savePal();
				} else {
					section = fieldPart->save();
				}
			} else {
				section = sectionData(fieldSection, true);
			}
		}

		if(hasSectionHeader()) {
			quint32 section_size = section.size();
			newData.append((char *)&section_size, 4);
		}
		newData.append(section);

		// Alignment padding
		if (alignment() > 0 && newData.size() % alignment() != 0) {
			newData.append(QByteArray(alignment() - newData.size() % alignment(), '\0'));
		}

		++id;
	}

	// Footer
//	newData.append(saveFooter()); // For PC footer: Already present in the background data

	// Header prepended to the section data
	newData.prepend(toc);

	if(compress) {
		const QByteArray &compresse = LZS::compress(newData);
		quint32 lzsSize = compresse.size();
		newData = QByteArray((char *)&lzsSize, 4).append(compresse);
	}

	return true;
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

qint8 Field::importer(const QString &path, bool isDat, bool compressed, FieldSections part, QIODevice *device2)
{
	QFile fic(path);
	if(!fic.open(QIODevice::ReadOnly))	return 1;
	if(fic.size() > 10000000)	return 2;

	QByteArray data;

	if(compressed) { // compressed field
		quint32 fileSize=0;
		if(fic.read((char *)&fileSize, 4) != 4)	return 2;
		if(fileSize+4 != fic.size()) return 2;

		data = LZS::decompressAll(fic.readAll());
	} else { // uncompressed field
		data = fic.readAll();
	}
	
	return importer(data, isDat, part, device2);
}

qint8 Field::importer(const QByteArray &data, bool isPSField, FieldSections part, QIODevice *device2)
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
			InfFile *inf = this->inf(false);
			if(!inf->open(data.mid(sectionPositions[4], sectionPositions[5]-sectionPositions[4])))	return 2;
			inf->setModified(true);
		}
		if(part.testFlag(Background)) {
			if(!device2) {
				qWarning() << "Field::importer Additional device need to be initialized";
				return 2;
			}
			if(!device2->open(QIODevice::ReadOnly)) {
				return 1;
			}

			quint32 lzsSize;

			if(device2->read((char *)&lzsSize, 4) != 4) {
				return 2;
			}

			if(lzsSize + 4 != device2->size()) {
				return 2;
			}

			QByteArray mimData = LZS::decompressAll(device2->readAll()),
					tilesData = data.mid(sectionPositions[2], sectionPositions[3]-sectionPositions[2]);

			BackgroundFilePS *bg;

			if(isPS()) {
				bg = (BackgroundFilePS *)background(false);
			} else {
				bg = new BackgroundFilePS(0);
			}

			if(!bg->open(mimData, tilesData)) {
				if(isPC()) {
					delete bg;
				}
				return 2;
			}
			if(isPC()) {
				_parts.insert(Background, new BackgroundFilePC(bg->toPC((FieldPC *)this)));
				delete bg;
			}

			background(false)->setModified(true);
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
			InfFile *inf = this->inf(false);
			if(!inf->open(data.mid(sectionPositions[7]+4, sectionPositions[8]-sectionPositions[7]-4)))	return 2;
			inf->setModified(true);
		}
		if(part.testFlag(Background) && isPC()) {
			QByteArray mimData = data.mid(sectionPositions[8]+4),
					palData = data.mid(sectionPositions[3]+4, sectionPositions[4]-sectionPositions[3]-4);

			BackgroundFilePC *bg;

			if(isPC()) {
				bg = (BackgroundFilePC *)background(false);
			} else {
				bg = new BackgroundFilePC(0);
			}

			if(!bg->open(mimData, palData)) {
				if(isPS()) {
					delete bg;
				}
				return 2;
			}
			if(isPS()) {
				_parts.insert(Background, new BackgroundFilePS(bg->toPS((FieldPS *)this)));
				delete bg;
			}

			background(false)->setModified(true);
		}
	}

	return 0;
}

void Field::setRemoveUnusedSection(bool remove)
{
	_removeUnusedSection = remove;
}
