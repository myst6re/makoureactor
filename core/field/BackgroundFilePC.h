#ifndef BACKGROUNDFILEPC_H
#define BACKGROUNDFILEPC_H

#include "BackgroundFile.h"

class BackgroundFilePC : public BackgroundFile
{
public:
	BackgroundFilePC();

	QPixmap openBackground(const QByteArray &data, const QByteArray &palData, const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL);
	bool usedParams(const QByteArray &data, QHash<quint8, quint8> &usedParams, bool *layerExists);
};

#endif // BACKGROUNDFILEPC_H
