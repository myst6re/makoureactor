#ifndef BACKGROUNDFILE_H
#define BACKGROUNDFILE_H

#include <QtCore>
#include <QPixmap>

//Sizeof : 36
typedef struct {
	qint16 cibleX, cibleY;
	quint32 unused1;
	quint8 srcX, unused2;
	quint8 srcY, unused3;
	quint8 srcX2, unused4;
	quint8 srcY2, unused5;
	quint32 unused6;
	quint8 paletteID, unused7;
	quint16 ID;
	quint8 param;
	quint8 state;
	quint8 blending;
	quint8 unused8;
	quint8 typeTrans, size;//Normaly unused
	quint8 textureID, unused10;
	quint8 textureID2, unused11;
	quint8 deph, unused12;
} Tile;

class BackgroundFile
{
public:
	BackgroundFile();
	virtual ~BackgroundFile();

	virtual QPixmap openBackground(const QByteArray &data1, const QByteArray &data2, const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL)=0;
	virtual bool usedParams(const QByteArray &data, QHash<quint8, quint8> &usedParams, bool *layerExists)=0;
protected:
	static QRgb blendColor(quint8 type, QRgb color0, QRgb color1);
};

#endif // BACKGROUNDFILE_H
