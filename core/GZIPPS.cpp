#include "GZIPPS.h"

QByteArray GZIPPS::decompress(const QByteArray &data)
{
	return decompress(data.constData(), data.size());
}

QByteArray GZIPPS::compress(const QByteArray &ungzip, const QByteArray &header)
{
	return compress(ungzip.constData(), ungzip.size(), header);
}

QByteArray GZIPPS::compress(const QByteArray &ungzip, quint32 header)
{
	return compress(ungzip.constData(), ungzip.size(), header);
}

QByteArray GZIPPS::decompress(const char *data, int size)
{
	if (size <= GZIPPS_HEADER_SIZE) {
		return QByteArray();
	}

	quint32 decSize;
	memcpy(&decSize, data, 4);
	return GZIP::decompress(data + GZIPPS_HEADER_SIZE, size - GZIPPS_HEADER_SIZE, decSize);
}

QByteArray GZIPPS::compress(const char *ungzip, int size, const QByteArray &header)
{
	Q_ASSERT(header.size() == 4);

	QByteArray ret;
	ret.append((char *)&size, 4); // = decSize
	ret.append(header);
	return ret.append(GZIP::compress(ungzip, size));
}

QByteArray GZIPPS::compress(const char *ungzip, int size, quint32 header)
{
	return compress(ungzip, size, QByteArray((char *)&header, 4));
}
