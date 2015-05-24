#include "IsoArchiveFF7.h"
#include "core/LZS.h"
#include "core/GZIPPS.h"

IsoArchiveFF7::IsoArchiveFF7(const QString &name) :
	IsoArchive(name)
{
}

IsoArchiveFF7::~IsoArchiveFF7()
{
	qDeleteAll(_devicesToDelete);
}

const QByteArray &IsoArchiveFF7::fileLzs(const QString &path, quint32 maxSize)
{
	return LZS::decompressAllWithHeader(file(path, maxSize));
}

IsoFile *IsoArchiveFF7::exe() const
{
	QRegExp exeName("[A-Z]{4}_\\d{3}\\.\\d{2}");
	foreach(IsoFile *isoFile, rootDirectory()->files()) {
		if(exeName.exactMatch(isoFile->name())) {
			return isoFile;
		}
	}

	return NULL;
}

QByteArray IsoArchiveFF7::windowBinData() const
{
	IsoDirectory *dir = initDirectory();
	if (!dir) {
		return QByteArray();
	}
	IsoFile *file = dir->file("WINDOW.BIN");
	if (!file) {
		return QByteArray();
	}
	return file->data();
}

IsoArchiveFF7::Country IsoArchiveFF7::country() const
{
	IsoFile *isoFile = exe();
	if (isoFile->name().startsWith("SCES")) {
		if (isoFile->name().endsWith("8.67")) {
			return Uk;
		} else if (isoFile->name().endsWith("8.68")) {
			return Fr;
		} else if (isoFile->name().endsWith("8.69")) {
			return De;
		} else if (isoFile->name().endsWith("9.00")) {
			return Es;
		}
	} else if(isoFile->name().startsWith("SCUS")) {
		return Us;
	} else if(isoFile->name().startsWith("SLPS")
			  || isoFile->name().startsWith("SCPS")
			  || isoFile->name().startsWith("SLPM")) {
		return Jp;
	}
	return NoCountry;
}

IsoDirectory *IsoArchiveFF7::fieldDirectory() const
{
	IsoDirectory *dir = rootDirectory()->directory("FIELD");
	if(!dir) {
		dir = rootDirectory()->directory("NARITA"); // Demo
	}
	return dir;
}

IsoDirectory *IsoArchiveFF7::initDirectory() const
{
	IsoDirectory *dir = rootDirectory()->directory("INIT");
	if(!dir) {
		dir = rootDirectory()->directory("BATTLE"); // Demo
	}
	return dir;
}

bool IsoArchiveFF7::isDemo() const
{
	IsoDirectory *dir = fieldDirectory();
	return dir && dir->name() == "NARITA";
}

bool IsoArchiveFF7::updateFieldBin()
{
	IsoDirectory *fieldDirectory = this->fieldDirectory();
	if(!fieldDirectory) {
		qWarning() << "IsoArchiveFF7::updateFieldBin field directory not found";
		return false;
	}
	IsoFile *isoFieldBin = fieldDirectory->file("FIELD.BIN");
	if(isoFieldBin == NULL) {
		qWarning() << "IsoArchiveFF7::updateFieldBin field.bin not found";
		return false;
	}

	QByteArray data = isoFieldBin->data();
	QByteArray header = data.left(8), ungzip;
	quint32 oldSectorStart, newSectorStart, oldSize, newSize;
	QMap<QByteArray, QByteArray> changesFieldBin;
	QMap<QByteArray, QString> files;//debug

	foreach(IsoFile *file, fieldDirectory->files()) {
		if(file->isModified() && !file->name().endsWith(".X")
				&& file->name() != "FIELD.BIN") {
			oldSectorStart = file->location();
			oldSize = file->size();
			newSectorStart = file->newLocation();
			newSize = file->newSize();

			if(oldSectorStart != newSectorStart || oldSize != newSize) {
				QByteArray tempBA = QByteArray((char *)&oldSectorStart, 4).append((char *)&oldSize, 4);
				changesFieldBin.insert(tempBA, QByteArray((char *)&newSectorStart, 4).append((char *)&newSize, 4));
				files.insert(tempBA, file->name());
				if(oldSectorStart != newSectorStart) {
					qDebug() << "File to update in FIELD.BIN" << file->name() << "position changed:" << oldSectorStart << newSectorStart << oldSize << newSize;
				} else {
					qDebug() << "File to update in FIELD.BIN" << file->name() << "size changed:" << oldSize << newSize;
				}
			}
		}
	}

	/*
	 *	HEADER :
	 *	 4 : ungzipped size
	 *	 4 : ungzipped size - 51588
	 */
	// Gzip uncompress
	ungzip = GZIPPS::decompress(data);
	if(ungzip.isEmpty()) {
		qWarning() << "IsoArchiveFF7::updateFieldBin unable to decompress";
		return false;
	}

	// Update

	const int indicativePosition = 0x30000;
	QByteArray copy = ungzip.mid(indicativePosition);
	QMapIterator<QByteArray, QByteArray> i(changesFieldBin);
	while(i.hasNext()) {
		i.next();
		const int count = copy.count(i.key());
		if(count == 0) {
			qWarning() << "IsoArchiveFF7::updateFieldBin Error not found!" << files.value(i.key()) << i.key().toHex();
			return false;
		} else if(count == 1) {
			qDebug() << "IsoArchiveFF7::updateFieldBin infos updated" << files.value(i.key()) << i.key().toHex() << i.value().toHex();
			ungzip.replace(indicativePosition + copy.indexOf(i.key()), i.key().size(), i.value());
		} else {
			qWarning() << "IsoArchiveFF7::updateFieldBin Error multiple occurrences" << files.value(i.key()) << i.key().toHex();
			return false;
		}
	}

	copy = GZIP::compress(ungzip);

	if(copy.isEmpty()) {
		qWarning() << "IsoArchiveFF7::updateFieldBin unable to compress";
		return false;
	}

	QBuffer *buffer = new QBuffer();
	buffer->setData(header.append(copy));
	_devicesToDelete << buffer;
	isoFieldBin->setModifiedFile(buffer);
	if (isoFieldBin->sectorCount() + isoFieldBin->paddingAfter() < isoFieldBin->newSectorCount()) {
		qWarning() << "FIELD.BIN sector count error" << isoFieldBin->sectorCount() << isoFieldBin->paddingAfter() << isoFieldBin->newSectorCount();
		return false;
	}

	return true;
}

bool IsoArchiveFF7::updateYamadaBin()
{
	IsoDirectory *initDirectory = this->initDirectory();
	if(!initDirectory) {
		qWarning() << "IsoArchiveFF7::updateYamadaBin INIT directory not found";
		return false;
	}
	IsoFile *isoYamadaBin = initDirectory->file("YAMADA.BIN");
	if(!isoYamadaBin) {
		qWarning() << "IsoArchiveFF7::updateYamadaBin YAMADA.BIN file not found";
		return false;
	}

	if(isoYamadaBin->size() != 80) {
		qWarning() << "IsoArchiveFF7::updateYamadaBin YAMADA.BIN file wrong size" << isoYamadaBin->size();
		return false;
	}

	QByteArray yamadaData = isoYamadaBin->data();
	quint32 *data = (quint32 *)yamadaData.data();
	QStringList filenames;
	filenames << /* QString("%1/YAMADA.BIN").arg(initDirectory->name())
			  << */QString("%1/WINDOW.BIN").arg(initDirectory->name())
			  << QString("%1/KERNEL.BIN").arg(initDirectory->name())
			  << "BATTLE/BROM.X"
			  << "BATTLE/TITLE.BIN" << "BATTLE/BATTLE.X"
			  << "BATTLE/BATINI.X" << "BATTLE/SCENE.BIN"
			  << "BATTLE/BATRES.X" << "BATTLE/CO.BIN";

	int i = 1;
	foreach(const QString &filename, filenames) {
		IsoFile *isoFile = rootDirectory()->file(filename);
		if(!isoFile) {
			qWarning() << "IsoArchiveFF7::updateYamadaBin" << filename << "file not found";
			return false;
		}

		quint32 pos = data[i * 2],
				size = data[i * 2 + 1];

		if(isoFile->location() != pos
				|| isoFile->size() != size) {
			qWarning() << "IsoArchiveFF7::updateYamadaBin" << filename << "wrong pos or size" << isoFile->location() << pos << "/" << isoFile->size() << size;
			if(isoFile->location() != pos) {
				return false;
			}
		}

		if(isoFile->newLocation() != pos) {
			data[i * 2] = isoFile->newLocation();
			qDebug() << "IsoArchiveFF7::updateYamadaBin" << filename << "location updated" << pos << isoFile->newLocation();
		}

		if(isoFile->newSize() != size) {
			data[i * 2 + 1] = isoFile->newSize();
			qDebug() << "IsoArchiveFF7::updateYamadaBin" << filename << "size updated" << size << isoFile->newSize();
		}

		++i;
	}

	QBuffer *buffer = new QBuffer();
	buffer->setData(yamadaData);
	_devicesToDelete << buffer;
	isoYamadaBin->setModifiedFile(buffer);

	return true;
}

bool IsoArchiveFF7::reorganizeModifiedFilesAfter(QMap<quint32, const IsoFile *> &writeToTheMain, QList<const IsoFile *> &writeToTheEnd)
{
	Q_UNUSED(writeToTheEnd)
	qDeleteAll(_devicesToDelete);
	_devicesToDelete.clear();
	if(updateFieldBin() && updateYamadaBin()) {
		IsoFile *isoFieldBin = fieldDirectory()->file("FIELD.BIN"),
				*isoYamadaBin = initDirectory()->file("YAMADA.BIN");
		writeToTheMain.insert(isoFieldBin->newLocation(), isoFieldBin);
		writeToTheMain.insert(isoYamadaBin->newLocation(), isoYamadaBin);
		return true;
	}
	setError(Archive::InvalidError, QObject::trUtf8("Impossible de mettre Ã  jour les binaires du jeu."));
	return false;
}
