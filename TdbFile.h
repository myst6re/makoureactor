#ifndef TDBFILE_H
#define TDBFILE_H

#include <QtCore>
#include <QImage>
#include "Palette.h"

typedef struct {
	quint32 size;
	quint16 imageCount;
	quint16 paletteCount;
	quint32 imageOffset;
	quint32 paletteOffset;
} TdbHeader;

class TdbFile
{
public:
	TdbFile();
	bool open(const QByteArray &data);
};

#endif // TDBFILE_H
