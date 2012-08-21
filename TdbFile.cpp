#include "TdbFile.h"

TdbFile::TdbFile()
{
}

bool TdbFile::open(const QByteArray &data)
{
	const char *constData = data.constData();
	TdbHeader header;

	memcpy(&header, constData, sizeof(TdbHeader));

	if(header.size != (quint32)data.size()) {
		qWarning() << "invalid tdb size" << header.size << data.size();
		return false;
	}

//	quint8 faceID = 0;

	for(int pal=0 ; pal<header.paletteCount ; ++pal) {
		quint32 offsetPalette = header.paletteOffset + pal * 32;
		QImage palette(16, 1, QImage::Format_RGB32);
		QRgb *px = (QRgb *)palette.bits();

		for(int i=0 ; i<16 ; ++i) {
			quint16 color;
			memcpy(&color, &constData[offsetPalette + i*2], 2);
			px[i] = PsColor::fromPsColor(color);
		}

		palette.save(QString("TdbPal%1.png").arg(pal));
	}

	for(int tex=0 ; tex<header.imageCount ; ++tex) {
		quint32 offsetImage = header.imageOffset + tex * 512;
		QImage img(32, 32, QImage::Format_RGB32);
		QRgb *px = (QRgb *)img.bits();

		for(int i=0 ; i<512 ; ++i) {
			quint8 index = constData[offsetImage + i];
			quint16 color;
			memcpy(&color, &constData[header.paletteOffset + (index & 0x0F)*2], 2);
			px[i*2] = PsColor::fromPsColor(color);

			memcpy(&color, &constData[header.paletteOffset + (index >> 4)*2], 2);
			px[i*2+1] = PsColor::fromPsColor(color);
		}

		img.save(QString("TdbTex%1.png").arg(tex));
	}

	return true;
}
