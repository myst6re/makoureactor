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
#pragma once

#include <QtCore>
#include "Archive.h"

#define MAX_ISO_READ			10000
#define MAX_FILENAME_LENGTH		207
#define SEPARATOR_1				'\x2E' // .
#define SEPARATOR_2				'\x3B' // ;
#define SECTOR_SIZE				2352
#define SECTOR_SIZE_HEADER		24
#define SECTOR_SIZE_DATA		2048
#define SECTOR_SIZE_FOOTER		280

//#define ISOARCHIVE_DEBUG

//d-characters : [\w ]
//a-characters : [!"%&'\(\)*+,\-\./\w:;<=>? ] (+ SP)

class IsoArchiveIO;
class IsoFileIO;

struct IsoTime {
	char year[4];
	char month[2];
	char day[2];
	char hour[2];
	char minute[2];
	char second[2];
	char millis[2];
	qint8 GMT;
};

struct PathTable {
	quint8 length_di;
	quint8 extended_attr_record_length;
	quint32 location_extent;
	quint16 parent_directory_number;
	QString name;
	quint32 position;
};

struct DirectoryRecordHead {
	quint32 location_extent;// little endian //first sector in the directory
	quint32 location_extent2;// big endian
	quint32 data_length;// little endian
	quint32 data_length2;// big endian
	quint8 year;// since 1900
	quint8 month;
	quint8 day;
	quint8 hour;
	quint8 minute;
	quint8 second;
	qint8 GMT;
	quint8 file_flags;// Multi-Extent/Reserved/Reserved/Protection/Record/Associated File/Directory/Existence
	quint8 file_unit_size;
	quint8 interleave_grap_size;
	quint16 volume_sequence_number;// little endian
	quint16 volume_sequence_number2;// big endian
	quint8 length_fi;
};

struct DirectoryRecord {
	quint8 length_dr;
	quint8 extended_attr_record_length;
	DirectoryRecordHead drh;
	QString name;
	QString version;
	// padding;
};

struct VolumeDescriptor1 {
	quint8 type;// 0x01 or 0xff
	quint8 id[5];// CD001
	quint8 version;// 1 = international standard
	quint8 unused1;
	quint8 system_id[32];
	quint8 volume_id[32];
	quint8 unused2[8];
	quint32 volume_space_size;// little endian // number of sectors
	quint32 volume_space_size2;// big endian
	quint8 unused3[32];
	quint16 volume_set_size;// little endian
	quint16 volume_set_size2;// big endian
	quint16 volume_sequence_number;// little endian
	quint16 volume_sequence_number2;// big endian
	quint16 logical_block_size;// little endian
	quint16 logical_block_size2;// big endian
	quint32 path_table_size;// little endian
	quint32 path_table_size2;// big endian
	quint32 type_path_table;// little endian
	quint32 opt_type_path_table;// little endian
	quint32 type_path_table2;// big endian
	quint32 opt_type_path_table2;// big endian
};

struct VolumeDescriptor2 {
	quint8 volume_set_id[128];
	quint8 publisher_id[128];
	quint8 preparer_id[128];
	quint8 application_id[128];
	quint8 copyright_file_id[37];
	quint8 abstract_file_id[37];
	quint8 bibliographic_file_id[37];
	IsoTime creation_date;
	IsoTime modification_date;
	IsoTime expiration_date;
	IsoTime effective_date;
	quint8 file_structure_version;
	quint8 unused4;
	quint8 application_data[512];
	quint8 unused5[653];
};

struct VolumeDescriptor {
	VolumeDescriptor1 vd1;
	DirectoryRecord dr;
	VolumeDescriptor2 vd2;
};

class IsoFileOrDirectory
{
public:
	virtual ~IsoFileOrDirectory();
	const QString &name() const;
	quint32 location() const;
	quint32 locationAfter() const;
	quint32 size() const;
	quint32 sectorCount() const;
	quint32 newLocation() const;
	quint32 newSize() const;
	quint32 newSectorCount() const;
	void setName(const QString &name);
	void setLocation(quint32 location);
	virtual bool isDirectory() const=0;
	bool isFile() const;
	virtual bool isModified() const;
	virtual void applyModifications();
	bool isSpecial() const;
	qint64 structPosition;
	void setPaddingAfter(quint8 after);
	quint8 paddingAfter() const;
protected:
	Q_DISABLE_COPY(IsoFileOrDirectory)
	IsoFileOrDirectory(const QString &name, quint32 location, quint32 size, qint64 structPosition);
	QString _name;
	quint32 _location, _size;
	quint32 _newLocation, _newSize;
	quint8 _paddingAfter;
};

class IsoFile : public IsoFileOrDirectory
{
public:
	IsoFile(const QString &name, quint32 location, quint32 size, qint64 structPosition, IsoArchiveIO *io);
	virtual ~IsoFile();
	bool isDirectory() const;
	QByteArray data(quint32 maxSize=0) const;
	QByteArray modifiedData(quint32 maxSize=0) const;
	bool extract(const QString &destination, quint32 maxSize=0) const;
	QIODevice *file() const;
	QIODevice *modifiedFile() const;
	bool setModifiedFile(QIODevice *io);
	bool setModifiedFile(const QByteArray &data);
	bool isModified() const;
	void applyModifications();
private:
	Q_DISABLE_COPY(IsoFile)
	void setFile(QIODevice *io);
	void cleanNewIO();
	QIODevice *_io, *_newIO;
	bool dataChanged, _newIOMustBeRemoved;
};

class IsoDirectory : public IsoFileOrDirectory
{
public:
	IsoDirectory(const QString &name, quint32 location, quint32 size, qint64 structPosition);
	virtual ~IsoDirectory();
	bool isDirectory() const;
	const QMap<QString, IsoFileOrDirectory *> &filesAndDirectories() const;
	QList<IsoFile *> files() const;
	QList<IsoDirectory *> directories() const;
	IsoFileOrDirectory *fileOrDirectory(const QString &path) const;
	IsoFile *file(const QString &path) const;
	IsoDirectory *directory(const QString &path) const;
	void add(IsoFileOrDirectory *fileOrDirectory);
private:
	Q_DISABLE_COPY(IsoDirectory)
	QMap<QString, IsoFileOrDirectory *> _filesAndDirectories;
};

class IsoArchiveIO : public QFile
{
	friend class IsoArchive;
public:
	IsoArchiveIO();
	explicit IsoArchiveIO(const QString &name);
	virtual ~IsoArchiveIO();

	bool open(QIODevice::OpenMode mode);
	qint64 posIso() const;
	bool seekIso(qint64 off);
	qint64 sizeIso() const;

	qint64 readIso(char *data, qint64 maxSize);
	QByteArray readIso(qint64 maxSize);

	qint64 writeIso(const char *data, qint64 maxSize);
	qint64 writeIso(const QByteArray &byteArray);

	static inline QByteArray int2Header(quint32 id) {
		quint8 h1, h2, h3;

		if (id < 4350) {
			h1 = 0;
			h2 = dec2Hex(id/75 + 2);
			h3 = dec2Hex(id - 75*(hex2Dec(h2) - 2));
		} else {
			h1 = dec2Hex((id + 150) / 4500);
			h2 = dec2Hex((id + 150 - hex2Dec(h1)*4500) / 75);
			h3 = dec2Hex(id + 150 - hex2Dec(h1)*4500 - hex2Dec(h2)*75);
		}

		return QByteArray().append((char)h1).append((char)h2).append((char)h3);
	}
	static inline QByteArray buildHeader(quint32 sector, quint8 type, quint8 mode=2) {
		return QByteArray("\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00", 12)
				.append(int2Header(sector)).append((char)mode)
				.append("\x00\x00", 2).append((char)type).append('\x00')
				.append("\x00\x00", 2).append((char)type).append('\x00');
	}
	static inline QByteArray buildFooter(quint32 sector) {
		Q_UNUSED(sector)
		//TODO (if possible): Checksum EDC/ECC (Error Detection Code & Error Correction Code)
		return QByteArray(SECTOR_SIZE_FOOTER, '\x00');
	}
	static inline void headerInfos(const QByteArray &header, quint8 *type, quint8 *mode = nullptr) {
		Q_ASSERT(header.size() != SECTOR_SIZE_HEADER);
		if (type) {
			*type = header.at(18);
		}
		if (mode) {
			*mode = header.at(15);
		}
	}

	QByteArray sector(quint32 num, quint16 maxSize=SECTOR_SIZE_DATA);
	QByteArray sectorHeader(quint32 num);
	QByteArray sectorFooter(quint32 num);
	quint32 currentSector() const;
	quint32 sectorCount() const;
	static quint32 sectorCountData(quint32 dataSize);
	bool seekToSector(quint32 num);
	bool writeSector(const QByteArray &data, quint8 type, quint8 mode=2);
private:
	Q_DISABLE_COPY(IsoArchiveIO)
	static qint64 isoPos(qint64 pos);
	static qint64 filePos(qint64 pos);

	static inline quint8 hex2Dec(quint8 hex) {
		return 10*(hex/16) + hex%16;
	}
	static inline quint8 dec2Hex(quint8 dec) {
		return 16*(dec/10) + dec%10;
	}
};

class IsoFileIO : public QIODevice
{
public:
	IsoFileIO(IsoArchiveIO *io, const IsoFile *infos, QObject *parent=0);
	bool open(OpenMode mode);
	qint64 size() const;
	bool canReadLine() const;
protected:
	qint64 readData(char *data, qint64 maxSize);
	qint64 writeData(const char *data, qint64 maxSize);
private:
	Q_DISABLE_COPY(IsoFileIO)
	IsoArchiveIO *_io;
	const IsoFile *_infos;
};

class IsoArchive
{
public:
	IsoArchive();
	explicit IsoArchive(const QString &name);
	virtual ~IsoArchive();

	virtual bool open(QIODevice::OpenMode mode);
	inline virtual bool isOpen() const {
		return _io.isOpen() && _rootDirectory;
	}
	virtual void close();
	inline QString fileName() const {
		return _io.fileName();
	}
	inline void setFileName(const QString &fileName) {
		_io.setFileName(fileName);
	}
	inline QString errorString() const {
		return _io.errorString();
	}
	inline const IsoArchiveIO &io() const {
		return _io;
	}

	bool pack(IsoArchive *destination, ArchiveObserver *control = nullptr, IsoDirectory *directory = nullptr);
	void applyModifications(IsoDirectory *directory);

	QByteArray file(const QString &path, quint32 maxSize=0) const;
	QIODevice *fileDevice(const QString &path) const;
	QByteArray modifiedFile(const QString &path, quint32 maxSize=0) const;
	QIODevice *modifiedFileDevice(const QString &path) const;
	bool extract(const QString &path, const QString &destination, quint32 maxSize=0) const;
	bool extractDir(const QString &path, const QString &destination) const;
	void extractAll(const QString &destination) const;
	qint32 diffCountSectors(const QString &path, quint32 newSize) const;

	IsoDirectory *rootDirectory() const;
//	const QList<PathTable> &getPathTables1a() const;
//	const QList<PathTable> &getPathTables1b() const;
//	const QList<PathTable> &getPathTables2a() const;
//	const QList<PathTable> &getPathTables2b() const;
	Archive::ArchiveError error() const;
protected:
	virtual bool reorganizeModifiedFilesAfter(QMap<quint32, const IsoFile *> &writeToTheMain, QList<const IsoFile *> &writeToTheEnd);
	void setError(Archive::ArchiveError error, const QString &errorString = QString());
private:
	Q_DISABLE_COPY(IsoArchive)
	bool openVolumeDescriptor(quint8 num=0);
	bool openRootDirectory(quint32 sector, quint32 dataSize=SECTOR_SIZE_DATA);
	IsoDirectory *_openDirectoryRecord(IsoDirectory *directories, QList<quint32> &dirVisisted);
//	QList<PathTable> pathTable(quint32 sector, quint32 dataSize=SECTOR_SIZE_DATA);

	// Returns index of file in "filesWithPadding" who have paddingAfter >= minSectorCount
	static int findPadding(const QList<IsoFileOrDirectory *> &filesWithPadding, quint32 minSectorCount);
	// Returns files with padding after
	QList<IsoFileOrDirectory *> getIntegrity();
	bool getIntegritySetPaddingAfter(IsoFileOrDirectory *prevFile, quint32 fileLocation);

	void _extractAll(const QString &destination, IsoDirectory *directories, QString currentInternalDir=QString()) const;
	void _getIntegrity(QMap<quint32, IsoFileOrDirectory *> &files, IsoDirectory *directory) const;
	QMap<quint32, IsoFile *> getModifiedFiles(IsoDirectory *directory) const;
	void getModifiedFiles(QMap<quint32, IsoFile *> &files, IsoDirectory *directory) const;
	static void repairLocationSectors(IsoDirectory *directory, IsoArchive *newIso);

	bool writeFile(QIODevice *in, quint32 sectorCount = 0, ArchiveObserver *control = nullptr);
	bool copySectors(IsoArchiveIO *out, qint64 size, ArchiveObserver *control = nullptr, bool repair = false);

	IsoArchiveIO _io;
	VolumeDescriptor volume;
	IsoDirectory *_rootDirectory;
	Archive::ArchiveError _error;
//	QList<PathTable> pathTables1a;
//	QList<PathTable> pathTables1b;
//	QList<PathTable> pathTables2a;
//	QList<PathTable> pathTables2b;
#ifdef ISOARCHIVE_DEBUG
	static QString isoTimeToString(const IsoTime &time);
	static QString volumeDescriptorToString(const VolumeDescriptor &vd);
	static QString directoryRecordToString(const DirectoryRecord &dr);
	static QString pathTableToString(const PathTable &pathTable, bool bigEndian=false);
#endif
};
