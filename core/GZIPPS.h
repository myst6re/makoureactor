#ifndef GZIPPS_H
#define GZIPPS_H

#include "GZIP.h"

#define GZIPPS_HEADER_SIZE		8

class GZIPPS
{
public:
	static QByteArray decompress(const QByteArray &data);
	static QByteArray compress(const QByteArray &ungzip, const QByteArray &header);
	static QByteArray compress(const QByteArray &ungzip, quint32 header);
	static QByteArray decompress(const char *data, int size);
	static QByteArray compress(const char *ungzip, int size, const QByteArray &header);
	static QByteArray compress(const char *ungzip, int size, quint32 header);
};

#endif // GZIPPS_H
