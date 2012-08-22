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
	enum TextureType {
		Eye=0,
		EyeClosed1, EyeClosed2,
		EyeOpened1, EyeOpened2,
		MouthClosed, MouthOpened,
		Empty
	};

	TdbFile();
	bool open(const QByteArray &data);
	QPixmap texture(quint8 faceID, TextureType type);
private:
	static int faceIdToImageId(quint8 faceID, TextureType type);
	TdbHeader header;
	QByteArray data;
};

#endif // TDBFILE_H
