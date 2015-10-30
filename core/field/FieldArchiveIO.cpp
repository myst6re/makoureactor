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
#include "FieldArchiveIO.h"
#include "../LZS.h"
#include "../Config.h"
#include "FieldArchive.h"
#include "Field.h"

QByteArray FieldArchiveIO::fieldDataCache;
Field *FieldArchiveIO::fieldCache=0;
QString FieldArchiveIO::fieldExtensionCache;

FieldArchiveIO::FieldArchiveIO(FieldArchive *fieldArchive) :
	_fieldArchive(fieldArchive)
{
}

FieldArchiveIO::~FieldArchiveIO()
{
	clearCachedData();
}

FieldArchive *FieldArchiveIO::fieldArchive()
{
	return _fieldArchive;
}

QString FieldArchiveIO::name() const
{
	if(!hasName())
		return QString();

	QString filePath = path();
	if(filePath.isEmpty()) {
		return filePath;
	}

	return filePath.mid(filePath.lastIndexOf('/') + 1);
}

QString FieldArchiveIO::directory() const
{
	if(!hasName())
		return path() + "/";

	QString filePath = path();
	if(filePath.isEmpty()) {
		return filePath;
	}

	return filePath.left(filePath.lastIndexOf('/') + 1);
}

QByteArray FieldArchiveIO::fieldData(Field *field, const QString &extension, bool unlzs)
{
	// use data from the cache
	if(unlzs && fieldDataIsCached(field, extension)) {
//		qDebug() << "FieldArchive use field data from cache" << field->name();
		return fieldDataCache;
	} /*else {
		qDebug() << "FieldArchive don't use field data from cache" << field->name() << unlzs;
	}*/

	QByteArray data = fieldData2(field, extension, unlzs);

	// put decompressed data in the cache
	if(unlzs && !data.isEmpty()) {
		fieldCache = field;
		fieldDataCache = data;
		fieldExtensionCache = extension;
	}
	return data;
}

QByteArray FieldArchiveIO::fileData(const QString &fileName, bool unlzs, bool isLzsFile)
{
	QByteArray data = fileData2(fileName);
	bool checkLzsHeader = !Config::value("lzsNotCheck").toBool();

	if(isLzsFile && (unlzs || checkLzsHeader)) {
		if(data.size() < 4)		return QByteArray();

		const char *lzsDataConst = data.constData();
		quint32 lzsSize;
		memcpy(&lzsSize, lzsDataConst, 4);

		if(checkLzsHeader && (quint32)data.size() != lzsSize + 4) {
			return QByteArray();
		}

		return unlzs
			? LZS::decompressAll(lzsDataConst + 4, qMin(lzsSize, quint32(data.size() - 4)))
			: data;
	} else {
		return data;
	}
}

int FieldArchiveIO::exportFieldData(Field *field, const QString &extension, const QString &path, bool unlzs)
{
	QByteArray newData = fieldData(field, extension, unlzs);

	if(!newData.isEmpty()) {
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

bool FieldArchiveIO::fieldDataIsCached(Field *field, const QString &fileType) const
{
	return fieldCache && fieldCache == field && fieldExtensionCache == fileType;
}

void FieldArchiveIO::clearCachedData()
{
//	qDebug() << "FieldArchive::clearCachedData()";
	fieldCache = 0;
	fieldDataCache.clear();
}

void FieldArchiveIO::close()
{
	clearCachedData();
}

FieldArchiveIO::ErrorCode FieldArchiveIO::open(ArchiveObserver *observer)
{
	clearCachedData();

	return open2(observer);
}

FieldArchiveIO::ErrorCode FieldArchiveIO::save(const QString &path, ArchiveObserver *observer)
{
	ErrorCode error = save2(path, observer);
	if(error == Ok) {
		clearCachedData(); // Important: the file data will change
	}
	return error;
}
