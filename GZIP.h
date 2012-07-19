#ifndef GZIP_H
#define GZIP_H

#include <QtCore>

class GZIP
{
public:
	static QByteArray decompress(const QByteArray &data, int decSize);
	static QByteArray compress(const QByteArray &ungzip);
};

#endif // GZIP_H
