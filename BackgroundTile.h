#ifndef BACKGROUNDTILE_H
#define BACKGROUNDTILE_H

#include <QtCore>

class BackgroundTile
{
public:
	BackgroundTile(const char *data, quint8 layerID, quint8 type);
	qint16 cibleX, cibleY, cibleZ;
	quint8 srcX, srcY;
	quint8 paletteID;
	quint8 param, state;
	quint8 blending;
	quint8 deph;
	quint8 layerID;
	quint8 textureID;
	quint8 width;
};

#endif // BACKGROUNDTILE_H
