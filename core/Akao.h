#ifndef AKAO_H
#define AKAO_H

#include <QtCore>

class Akao
{
public:
	Akao();
private:
	quint16 _musicID, _length, _reverbMode;
	QMap<quint8, QByteArray> _data;
};

#endif // AKAO_H
