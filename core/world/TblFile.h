#ifndef TBLFILE_H
#define TBLFILE_H

#include <QtCore>

struct WorldToField
{
	qint16 x, y, z;
	quint16 fieldId;
	quint8 dir;
};

struct TblFileEntry
{
	WorldToField wm2Field[2]; // 0= default, 1= alternate
};

class TblFile
{
public:
	TblFile();
	bool open(const QByteArray &data);
	QByteArray save() const;
	QString toString() const;
private:
	QList<TblFileEntry> _entries;
};

#endif // TBLFILE_H
