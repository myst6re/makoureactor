/****************************************************************************
 ** Néo-Midgar Final Fantasy VII French Retranslation
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
#include "IsoArchiveFF7.h"
#include "core/LZS.h"
#include "core/GZIPPS.h"

IsoArchiveFF7::IsoArchiveFF7(const QString &name) :
	IsoArchive(name), _exe(NULL), _country(NoCountry)
{
}

IsoArchiveFF7::~IsoArchiveFF7()
{
	qDeleteAll(_devicesToDelete);
}

bool IsoArchiveFF7::open(QIODevice::OpenMode mode)
{
	bool ok = IsoArchive::open(mode);

	if (ok) {
		_exe = searchExe();
		_country = searchCountry();
	} else {
		_exe = NULL;
		_country = NoCountry;
	}

	return ok;
}

const QByteArray &IsoArchiveFF7::fileLzs(const QString &path, quint32 maxSize) const
{
	return LZS::decompressAllWithHeader(file(path, maxSize));
}

const QByteArray &IsoArchiveFF7::modifiedFileLzs(const QString &path, quint32 maxSize) const
{
	return LZS::decompressAllWithHeader(modifiedFile(path, maxSize));
}

IsoFile *IsoArchiveFF7::searchExe() const
{
	QRegExp exeName("[A-Z]{4}_\\d{3}\\.\\d{2}");
	foreach(IsoFile *isoFile, rootDirectory()->files()) {
		if(exeName.exactMatch(isoFile->name())) {
			return isoFile;
		}
	}

	return NULL;
}

IsoFile *IsoArchiveFF7::exe() const
{
	return _exe;
}

bool IsoArchiveFF7::isDisc(int num) const
{
	QIODevice *file = fileDevice("MINT/DISKINFO.CNF");
	if (!file) {
		return false;
	}

	if (!file->isOpen() && !file->open(QIODevice::ReadOnly)) {
		return false;
	}
	file->reset();

	QString line = file->readLine(16);
	return line.startsWith(QString("DISK000%1").arg(num));
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

/*
 * JP
 * 
 * SLPS-00700
 * SLPS-00701
 * SLPS-00702
 * 
 * SCPS-45007
 * SCPS-45008
 * SCPS-45009
 * 
 * SLPS-01057
 * SLPS-01058
 * SLPS-01059
 * SLPS-01060
 * 
 * SLPS-91440
 * SLPS-91441
 * SLPS-91442
 * SLPS-91443
 *
 * SLPM-87380
 * SLPM-87381
 * SLPM-87382
 * SLPM-87383
 *
 * US
 * 
 * SCUS-94163
 * SCUS-94164
 * SCUS-94165
 *
 * EU
 * 
 * SCES-00867
 * SCES-10867
 * SCES-20867
 * 
 * SCES-00868
 * SCES-10868
 * SCES-20868
 * 
 * SCES-00869
 * SCES-10869
 * SCES-20869
 * 
 * SCES-00900
 * SCES-10900
 * SCES-20900
 */

IsoArchiveFF7::Country IsoArchiveFF7::searchCountry() const
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

IsoArchiveFF7::Country IsoArchiveFF7::country() const
{
	return _country;
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

bool IsoArchiveFF7::updateBin(IsoFile *isoBin, const QList<IsoFile *> &filesRefByBin, int startOffset)
{
	QByteArray data = isoBin->modifiedData(),
	        ungzip;
	quint32 oldSectorStart, newSectorStart, oldSize, newSize;
	QMap<QByteArray, QByteArray> changesFieldBin;
#ifdef ISOARCHIVE_DEBUG
	QMap<QByteArray, QString> filesDebug; // debug
#endif

	foreach(IsoFile *file, filesRefByBin) {
		if(file->isModified()) {
			oldSectorStart = file->location();
			oldSize = file->size();
			newSectorStart = file->newLocation();
			newSize = file->newSize();

			if(oldSectorStart != newSectorStart || oldSize != newSize) {
				QByteArray tempBA = QByteArray((char *)&oldSectorStart, 4).append((char *)&oldSize, 4);
				changesFieldBin.insert(tempBA, QByteArray((char *)&newSectorStart, 4).append((char *)&newSize, 4));
#ifdef ISOARCHIVE_DEBUG
				filesDebug.insert(tempBA, file->name());
				if(oldSectorStart != newSectorStart) {
					qDebug() << "IsoArchiveFF7::updateBin File to update in" << isoBin->name() << file->name() << "position changed:" << oldSectorStart << newSectorStart << oldSize << newSize;
				} else {
					qDebug() << "IsoArchiveFF7::updateBin File to update in" << isoBin->name() << file->name() << "size changed:" << oldSize << newSize;
				}
#endif
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
		qWarning() << "IsoArchiveFF7::updateBin" << isoBin->name() << "unable to decompress";
		return false;
	}

	// Update

	QByteArray copy = ungzip;
	QMapIterator<QByteArray, QByteArray> i(changesFieldBin);
	while(i.hasNext()) {
		i.next();
		int count = copy.count(i.key());
		if(count == 0) {
			qWarning() << "IsoArchiveFF7::updateBin" << isoBin->name() << "Error not found!" << i.key().toHex();
#ifdef ISOARCHIVE_DEBUG
			qWarning() << "IsoArchiveFF7::updateBin" << filesDebug.value(i.key());
#endif
			return false;
		} else if(count == 1) {
			startOffset = copy.indexOf(i.key());
			ungzip.replace(startOffset, i.key().size(), i.value());
		} else {
			qWarning() << "IsoArchiveFF7::updateBin" << isoBin->name() << "Error multiple occurrences 1" << i.key().toHex();
#ifdef ISOARCHIVE_DEBUG
			qWarning() << "IsoArchiveFF7::updateBin" << filesDebug.value(i.key());
#endif
			if(copy.mid(startOffset).count(i.key()) == 1) {
				ungzip.replace(copy.indexOf(i.key(), startOffset), 8, i.value());
			} else {
				qWarning() << "IsoArchiveFF7::updateBin" << isoBin->name() << "Error multiple occurrences 2" << i.key().toHex();
#ifdef ISOARCHIVE_DEBUG
				qWarning() << "IsoArchiveFF7::updateBin" << filesDebug.value(i.key());
#endif
				return false;
			}
		}
	}

	if (ungzip == copy) {
		qWarning() << "IsoArchiveFF7::updateBin" << isoBin->name() << "nothing to update";
		return true;
	}

	copy = GZIPPS::compress(ungzip, data.mid(4, 4), 9);

	if(copy.isEmpty()) {
		qWarning() << "IsoArchiveFF7::updateBin" << isoBin->name() << "unable to compress";
		return false;
	}

	QBuffer *buffer = new QBuffer();
	buffer->setData(copy);
	_devicesToDelete << buffer;
	isoBin->setModifiedFile(buffer);
	if (isoBin->sectorCount() + isoBin->paddingAfter() < isoBin->newSectorCount()) {
		qWarning() << "IsoArchiveFF7::updateBin" << isoBin->name() << "sector count error" << isoBin->sectorCount() << isoBin->paddingAfter() << isoBin->newSectorCount();
		return false;
	}

	return true;
}

bool IsoArchiveFF7::updateFieldBin()
{
	IsoDirectory *fieldDirectory = rootDirectory()->directory("FIELD");
	if(!fieldDirectory) {
		qWarning() << "IsoArchiveFF7::updateFieldBin field directory not found";
		return false;
	}
	IsoFile *isoFieldBin = fieldDirectory->file("FIELD.BIN");
	if(isoFieldBin == NULL) {
		qWarning() << "IsoArchiveFF7::updateFieldBin field.bin not found";
		return false;
	}

	QList<IsoFile *> files;
	foreach(IsoFile *file, fieldDirectory->files()) {
		if(!file->name().endsWith(".X")
				&& file->name() != "FIELD.BIN") {
			files << file;
		}
	}

	return updateBin(isoFieldBin, files, 0x30000);
}

bool IsoArchiveFF7::updateWorldBin()
{
	IsoDirectory *worldDirectory = rootDirectory()->directory("WORLD");
	if(!worldDirectory) {
		qWarning() << "IsoArchiveFF7::updateWorldBin world directory not found";
		return false;
	}
	IsoFile *isoWorldBin = worldDirectory->file("WORLD.BIN");
	if(isoWorldBin == NULL) {
		qWarning() << "IsoArchiveFF7::updateFieldBin world.bin not found";
		return false;
	}

	QList<IsoFile *> files;
	foreach(IsoFile *file, worldDirectory->files()) {
		if(file->name() != "WORLD.BIN") {
			files << file;
		}
	}

	return updateBin(isoWorldBin, files, 0x27000);
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

	QByteArray yamadaData = isoYamadaBin->modifiedData();
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
#ifdef ISOARCHIVE_DEBUG
			qDebug() << "IsoArchiveFF7::updateYamadaBin" << filename << "location updated" << pos << isoFile->newLocation();
#endif
		}

		if(isoFile->newSize() != size) {
			data[i * 2 + 1] = isoFile->newSize();
#ifdef ISOARCHIVE_DEBUG
			qDebug() << "IsoArchiveFF7::updateYamadaBin" << filename << "size updated" << size << isoFile->newSize();
#endif
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
	if(updateFieldBin() && updateWorldBin() && updateYamadaBin()) {
		IsoFile *isoFieldBin = fieldDirectory()->file("FIELD.BIN"),
		        *isoWorldBin = rootDirectory()->file("WORLD/WORLD.BIN"),
				*isoYamadaBin = initDirectory()->file("YAMADA.BIN");
		writeToTheMain.insert(isoFieldBin->newLocation(), isoFieldBin);
		writeToTheMain.insert(isoWorldBin->newLocation(), isoWorldBin);
		writeToTheMain.insert(isoYamadaBin->newLocation(), isoYamadaBin);
		return true;
	}
	setError(Archive::InvalidError, QObject::tr("Cannot update game binaries."));
	return false;
}
