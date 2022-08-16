/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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
#include <LZS.h>
#include "Field.h"
#include "FieldArchiveIO.h"
#include "FieldPC.h"
#include "FieldPS.h"
#include "BackgroundFilePC.h"

Field::Field(const QString &name, FieldArchiveIO *io) :
	_io(io), _isOpen(false), _isModified(false),
	_name(name.toLower()), _removeUnusedSection(false)
{
}

Field::Field(const QString &name) :
	Field(name, nullptr)
{

}

Field::~Field()
{
	for (FieldPart *part : qAsConst(_parts)) {
		if (part) {
			delete part;
		}
	}
}

void Field::setModified(bool modified)
{
	if (!_isOpen && _io != nullptr) {
		if (!open()) {
			qWarning() << "Unable to reopen!";
			return;
		}
	}
	_isModified = modified;
	FieldPart *section1 = part(Scripts);
	if (section1)	section1->setModified(modified);
}

bool Field::open(bool dontOptimize)
{
	QByteArray fileData;

	if (_io == nullptr) {
		return false;
	}

	if (headerSize() > 0) {
		QString fileType = sectionFile(Scripts);
		if (!dontOptimize && !_io->fieldDataIsCached(this, fileType)) {
			QByteArray lzsData = _io->fieldData(this, fileType, false);

			if (lzsData.size() < 4)	return false;

			const char *lzsDataConst = lzsData.constData();
			quint32 lzsSize;
			memcpy(&lzsSize, lzsDataConst, 4);

			if (quint32(lzsData.size()) != lzsSize + 4 && lzsSize == 0x90000) { // Maybe it is not compressed
				fileData = lzsData;
			} else {
				fileData = LZS::decompress(lzsDataConst + 4, int(qMin(qsizetype(lzsSize), lzsData.size() - 4)), headerSize());//partial decompression
			}
		} else {
			fileData = _io->fieldData(this, fileType);
		}

		if (fileData.size() < headerSize())	return false;

		openHeader(fileData);
	}

	_isOpen = true;

	return true;
}
/*
bool Field::open(const QString &path, bool isDat, bool compressed,
                 QIODevice *device2)
{
	_isOpen = importer(path, isDat, compressed,
	                     Field::FieldSection(0xFFFF), // Everything
	                     device2);
	return _isOpen;
}

bool Field::open(const QByteArray &data, bool isPSField, QIODevice *device2)
{
	_isOpen = importer(data, isPSField,
	                     Field::FieldSection(0xFFFF), // Everything
	                     device2);
	return _isOpen;
}*/

int Field::sectionSize(FieldSection part) const
{
	int idPart = sectionId(part);

	if (idPart < sectionCount() - 1) {
		return int(sectionPosition(idPart + 1)) - paddingBetweenSections() - int(sectionPosition(idPart));
	}
	return -1;
}

QByteArray Field::sectionData(FieldSection part, bool dontOptimize)
{
	if (!_isOpen) {
		open();
	}

	if (!_isOpen || _io == nullptr) {
		return QByteArray();
	}

	int idPart = sectionId(part),
			position = sectionPosition(idPart),
			size = sectionSize(part);
	QString fileType = sectionFile(part);

	if (size < 0) {
		dontOptimize = true;
	}

	QByteArray data;
	if (dontOptimize || _io->fieldDataIsCached(this, fileType)) {
		data = _io->fieldData(this, fileType);
	} else {
		QByteArray lzsData = _io->fieldData(this, fileType, false);

		if (lzsData.size() < 4) {
			return QByteArray();
		}

		const char *lzsDataConst = lzsData.constData();
		quint32 lzsSize;
		memcpy(&lzsSize, lzsDataConst, 4);

		if (quint32(lzsData.size()) != lzsSize + 4 && lzsSize == 0x90000) { // Maybe it is not compressed
			data = lzsData;
		} else {
			data = LZS::decompress(lzsDataConst + 4, qMin(lzsSize, quint32(lzsData.size() - 4)), sectionPosition(idPart+1));
		}
	}

	if (size < 0) {
		QByteArray footer = saveFooter();
		if (!footer.isEmpty() && data.endsWith(footer)) {
			return data.mid(position, data.size() - position - footer.size());
		}
	}
	return data.mid(position, size);
}

FieldPart *Field::createPart(FieldSection section)
{
	switch (section) {
	case Scripts:		return new Section1File(this);
	case Akaos:			return new TutFileStandard(this);
	case Camera:		return new CaFile(this);
//	case PalettePC:		return ;
	case Walkmesh:		return new IdFile(this);
	case Encounter:		return new EncounterFile(this);
	case Inf:			return new InfFile(this);
	case Tiles:			return new BackgroundTilesFile(this);
	default:			return nullptr;
	}
}

FieldPart *Field::part(FieldSection section) const
{
	return _parts.value(section);
}

FieldPart *Field::part(FieldSection section, bool open)
{
	FieldPart *p = part(section);

	if (!p) {
		p = createPart(section);
		_parts.insert(section, p);
	}

	if (open && !p->isOpen()) {
		p->open();
	}

	return p;
}

void Field::initEmpty()
{
	QList<Field::FieldSection> sections = orderOfSections();
	sections << Akaos;
	for (const FieldSection &section : qAsConst(sections)) {
		FieldPart *p = part(section);

		if (!p) {
			p = createPart(section);
			if (p) {
				_parts.insert(section, p);
			}
		}

		if (p) {
			p->initEmpty();
			p->setOpen(true);
			p->setModified(true);
		}
	}

	setRemoveUnusedSection(true);
	_isOpen = true;
	setModified(true);
}

Section1File *Field::scriptsAndTexts(bool open)
{
	return static_cast<Section1File *>(part(Scripts, open));
}

EncounterFile *Field::encounter(bool open)
{
	return static_cast<EncounterFile *>(part(Encounter, open));
}

TutFileStandard *Field::tutosAndSounds(bool open)
{
	return static_cast<TutFileStandard *>(part(Akaos, open));
}

IdFile *Field::walkmesh(bool open)
{
	return static_cast<IdFile *>(part(Walkmesh, open));
}

CaFile *Field::camera(bool open)
{
	return static_cast<CaFile *>(part(Camera, open));
}

InfFile *Field::inf(bool open)
{
	return static_cast<InfFile *>(part(Inf, open));
}

FieldModelLoader *Field::fieldModelLoader(bool open)
{
	return static_cast<FieldModelLoader *>(part(ModelLoader, open));
}

BackgroundTilesFile *Field::tiles(bool open)
{
	return static_cast<BackgroundTilesFile *>(part(Tiles, open));
}

BackgroundFile *Field::background(bool open)
{
	return static_cast<BackgroundFile *>(part(Background, open));
}

QMap<int, FieldModelFile *> Field::fieldModels(bool animate, bool open)
{
	QMap<int, FieldModelFile *> ret;

	int modelCount = scriptsAndTexts()->modelCount();
	for (int modelId=0; modelId < modelCount; ++modelId) {
		ret.insert(modelId, fieldModel(modelId, 0, animate, open));
	}

	return ret;
}

void Field::setName(const QString &name)
{
	_name = name;
	_isModified = true;
}

void Field::setSaved()
{
	if (_io != nullptr) {
		_isOpen = false; // Force reopen to refresh positions automatically
	}
	for (FieldPart *part : qAsConst(_parts)) {
		part->setModified(false);
	}
}

bool Field::save(QByteArray &newData, bool compress)
{
	newData = QByteArray();

	if (!isOpen()) {
		return false;
	}

	QByteArray toc;

	// Header
	toc.append(saveHeader());

	// Sections
	int id=0;
	QList<Field::FieldSection> fieldSections = orderOfSections();
	for (const FieldSection &fieldSection : qAsConst(fieldSections)) {
		// Section position
		quint32 pos = headerSize() + newData.size() + diffSectionPos();
		toc.append((char *)&pos, 4);

		QByteArray section;

		if (fieldSection != Tiles || !_removeUnusedSection) { // FIXME: ugly hack only for PC version

			// Section data
			FieldPart *fieldPart = part(fieldSection == Field::PalettePC
										? Field::Background // FIXME: EXCEPTION NEEDS TO BE REMOVED IN THE FUTURE
										: fieldSection);

			if (fieldPart && fieldPart->canSave() &&
					fieldPart->isOpen() && fieldPart->isModified()) {
				if (fieldSection == Field::PalettePC) { // FIXME: EXCEPTION NEEDS TO BE REMOVED IN THE FUTURE
					section = static_cast<BackgroundFilePC *>(fieldPart)->savePal();
				} else {
					section = fieldPart->save();
					if (section.isEmpty()) {
						qWarning() << "Field::save empty section error" << int(fieldSection);
						return false;
					}
				}
			} else {
				section = sectionData(fieldSection, true);
			}
		}

		if (hasSectionHeader()) {
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
	newData.append(saveFooter());

	// Header prepended to the section data
	newData.prepend(toc);

	if (compress) {
		const QByteArray &compresse = LZS::compress(newData);
		quint32 lzsSize = quint32(compresse.size());
		newData = QByteArray((char *)&lzsSize, 4).append(compresse);
	}

	return true;
}

qint8 Field::save(const QString &path, bool compress)
{
	QByteArray newData;

	if (save(newData, compress)) {
		QFile fic(path);
		if (!fic.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			return 2;
		}
		fic.write(newData);
		fic.close();
	} else {
		return 1;
	}

	return 0;
}

bool Field::importer(const QString &path, bool isDat, bool compressed, FieldSections part, QIODevice *bsxDevice,
                      QIODevice *mimDevice)
{
	QFile fic(path);
	if (!fic.open(QIODevice::ReadOnly)) {
		_lastError = fic.errorString();
		return false;
	}
	if (fic.size() > 10000000) {
		_lastError = QObject::tr("File size greater than 10000000");
		return false;
	}

	QByteArray data;

	if (compressed) { // compressed field
		quint32 fileSize=0;
		if (fic.read((char *)&fileSize, 4) != 4) {
			_lastError = fic.errorString();
			return false;
		}
		if (fileSize+4 != fic.size()) {
			_lastError = QObject::tr("Malformed LZS header");
			return false;
		}

		data = LZS::decompressAll(fic.readAll());
	} else { // uncompressed field
		data = fic.readAll();
	}
	
	return importer(data, isDat, part, bsxDevice, mimDevice);
}

bool Field::importModelLoader(const QByteArray &sectionData, bool isPSField, QIODevice *bsxDevice)
{
	Q_UNUSED(sectionData)
	Q_UNUSED(isPSField)
	Q_UNUSED(bsxDevice)
	return false;
}

bool Field::importer(const QByteArray &data, bool isPSField, FieldSections part, QIODevice *bsxDevice,
                      QIODevice *mimDevice)
{
	if (isPSField) {
		quint32 sectionPositions[7];
		const int headerSize = 28;

		if (data.size() < headerSize) {
			_lastError = QObject::tr("Incorrect field file size");
			return false;
		}
		memcpy(sectionPositions, data.constData(), headerSize); // header
		qint32 vramDiff = sectionPositions[0] - headerSize;// vram section1 pos - real section 1 pos

		for (int i=0; i<7; ++i) {
			sectionPositions[i] -= vramDiff;
		}

		if (part.testFlag(Scripts)) {
			Section1File *section1 = scriptsAndTexts(false);
			if (!section1->open(data.mid(sectionPositions[0], sectionPositions[1]-sectionPositions[0]))) {
				_lastError = QObject::tr("Cannot open section 1 (texts, scripts and musics)");
				return false;
			}
			section1->setModified(true);
		}
		if (part.testFlag(Akaos)) {
			TutFile *_tut = tutosAndSounds(false);
			if (!_tut->open(data.mid(sectionPositions[0], sectionPositions[1]-sectionPositions[0]))) {
				_lastError = QObject::tr("Cannot open tutos section");
				return false;
			}
			_tut->setModified(true);
		}
		if (part.testFlag(Encounter)) {
			EncounterFile *enc = encounter(false);
			if (!enc->open(data.mid(sectionPositions[5], sectionPositions[6]-sectionPositions[5]))) {
				_lastError = QObject::tr("Cannot open encounters section");
				return false;
			}
			enc->setModified(true);
		}
		if (part.testFlag(Walkmesh)) {
			IdFile *walk = walkmesh(false);
			if (!walk->open(data.mid(sectionPositions[1], sectionPositions[2]-sectionPositions[1]))) {
				_lastError = QObject::tr("Cannot open walkmesh section");
				return false;
			}
			walk->setModified(true);
		}
		if (part.testFlag(Camera)) {
			CaFile *ca = camera(false);
			if (!ca->open(data.mid(sectionPositions[3], sectionPositions[4]-sectionPositions[3]))) {
				_lastError = QObject::tr("Cannot open camera section");
				return false;
			}
			ca->setModified(true);
		}
		if (part.testFlag(Inf)) {
			InfFile *inf = this->inf(false);
			if (!inf->open(data.mid(sectionPositions[4], sectionPositions[5]-sectionPositions[4]))) {
				_lastError = QObject::tr("Cannot open info section");
				return false;
			}
			inf->setMapName(name());
			inf->setModified(true);
		}
		if (part.testFlag(ModelLoader)) {
			if (!importModelLoader(data.mid(sectionPositions[6]), isPSField, bsxDevice)) {
				_lastError = QObject::tr("Cannot open model loader section");
				return false;
			}
		}
		if (part.testFlag(Background)) {
			if (!mimDevice) {
				qWarning() << "Field::importer Additional device need to be initialized";
				return false;
			}
			if (!mimDevice->open(QIODevice::ReadOnly)) {
				_lastError = mimDevice->errorString();
				return false;
			}

			QByteArray mimData = LZS::decompressAllWithHeader(mimDevice->readAll()),
					tilesData = data.mid(sectionPositions[2], sectionPositions[3]-sectionPositions[2]);

			BackgroundFilePS *bg;

			if (isPS()) {
				bg = static_cast<BackgroundFilePS *>(background(false));
			} else {
				bg = new BackgroundFilePS(nullptr);
			}

			if (!bg->open(mimData, tilesData)) {
				if (isPC()) {
					delete bg;
				}
				_lastError = QObject::tr("Cannot open background section");
				return false;
			}
			if (isPC()) {
				_parts.insert(Background, new BackgroundFilePC(bg->toPC(static_cast<FieldPC *>(this))));
				delete bg;
			}

			background(false)->setModified(true);
		}
	} else {
		quint32 sectionPositions[9];

		if (data.size() < 6 + 9 * 4) {
			_lastError = QObject::tr("Incorrect field file size");
			return false;
		}
		memcpy(sectionPositions, data.constData() + 6, 9 * 4); // header

		if (part.testFlag(Scripts)) {
			Section1File *section1 = scriptsAndTexts(false);
			if (!section1->open(data.mid(sectionPositions[0]+4, sectionPositions[1]-sectionPositions[0]-4))) {
				_lastError = QObject::tr("Cannot open section 1 (texts, scripts and musics)");
				return false;
			}
			section1->setModified(true);
		}
		if (part.testFlag(Akaos)) {
			TutFile *_tut = tutosAndSounds(false);
			if (!_tut->open(data.mid(sectionPositions[0]+4, sectionPositions[1]-sectionPositions[0]-4))) {
				_lastError = QObject::tr("Cannot open tutos section");
				return false;
			}
			_tut->setModified(true);
		}
		if (part.testFlag(ModelLoader)) {
			if (!importModelLoader(data.mid(sectionPositions[2]+4, sectionPositions[3]-sectionPositions[2]-4),
			                       isPSField, bsxDevice)) {
				_lastError = QObject::tr("Cannot open model loader section");
				return false;
			}
		}
		if (part.testFlag(Encounter)) {
			EncounterFile *enc = encounter(false);
			if (!enc->open(data.mid(sectionPositions[6]+4, sectionPositions[7]-sectionPositions[6]-4))) {
				_lastError = QObject::tr("Cannot open encounters section");
				return false;
			}
			enc->setModified(true);
		}
		if (part.testFlag(Walkmesh)) {
			IdFile *walk = walkmesh(false);
			if (!walk->open(data.mid(sectionPositions[4]+4, sectionPositions[5]-sectionPositions[4]-4))) {
				_lastError = QObject::tr("Cannot open walkmesh section");
				return false;
			}
			walk->setModified(true);
		}
		if (part.testFlag(Camera)) {
			CaFile *ca = camera(false);
			if (!ca->open(data.mid(sectionPositions[1]+4, sectionPositions[2]-sectionPositions[1]-4))) {
				_lastError = QObject::tr("Cannot open camera section");
				return false;
			}
			ca->setModified(true);
		}
		if (part.testFlag(Inf)) {
			InfFile *inf = this->inf(false);
			if (!inf->open(data.mid(sectionPositions[7]+4, sectionPositions[8]-sectionPositions[7]-4))) {
				_lastError = QObject::tr("Cannot open info section");
				return false;
			}
			inf->setMapName(name());
			inf->setModified(true);
		}
		if (part.testFlag(Background)) {
			QByteArray mimData = data.mid(sectionPositions[8]+4),
					palData = data.mid(sectionPositions[3]+4, sectionPositions[4]-sectionPositions[3]-4);

			BackgroundFilePC *bg;

			if (isPC()) {
				bg = static_cast<BackgroundFilePC *>(background(false));
			} else {
				bg = new BackgroundFilePC(nullptr);
			}

			if (!bg->open(mimData, palData)) {
				if (isPS()) {
					delete bg;
				}
				_lastError = QObject::tr("Cannot open background section");
				return false;
			}
			if (isPS()) {
				_parts.insert(Background, new BackgroundFilePS(bg->toPS(static_cast<FieldPS *>(this))));
				delete bg;
			}

			background(false)->setModified(true);
		}
	}

	return true;
}
