#include "TblFile.h"

TblFile::TblFile()
{
}

bool TblFile::open(const QByteArray &data)
{
	if(data.size() % 24 != 0) {
		return false;
	}

	int entryCount = data.size() / 24;
	const char *constData = data.constData();

	for(int i=0 ; i<entryCount ; ++i) {
		TblFileEntry entry;

		for(int j=0 ; j<2 ; ++j) {
			memcpy(entry.wm2Field + j, constData, sizeof(WorldToField));

			constData += 12;
		}

		_entries.append(entry);
	}

	return true;
}

QByteArray TblFile::save() const
{
	QByteArray data;

	foreach(const TblFileEntry &entry, _entries) {
		for(int i=0 ; i<2 ; ++i) {
			const WorldToField &wm2Field = entry.wm2Field[i];

			data.append((char *)&wm2Field.x, 2);
			data.append((char *)&wm2Field.y, 2);
			data.append((char *)&wm2Field.z, 2);
			data.append((char *)&wm2Field.fieldId, 2);
			data.append(char(wm2Field.dir));
			data.append(char(wm2Field.dir)); // padding
			data.append(char(wm2Field.dir)); // padding
			data.append(char(wm2Field.dir)); // padding
		}
	}

	return data;
}

QString TblFile::toString() const
{
	QString ret;

	int mapId=0;
	foreach(const TblFileEntry &entry, _entries) {
		ret.append(QString("WM%1: ").arg(mapId++));
		QStringList entriesStr;
		for(int i=0 ; i<2 ; ++i) {
			const WorldToField &wm2Field = entry.wm2Field[i];
			if(wm2Field.fieldId != 0) {
				entriesStr.append(QString("At (x=%1, y=%2, z=%3) => field %4 (direction=%5)")
								  .arg(wm2Field.x).arg(wm2Field.y).arg(wm2Field.z)
								  .arg(wm2Field.fieldId).arg(wm2Field.dir));
			}
		}
		ret.append(entriesStr.join(" | "));
		ret.append("\n");
	}

	return ret;
}
