#ifndef LZSIO_H
#define LZSIO_H

#include <QIODevice>

#define LZS_BUFFER_SIZE     4096
#define LZS_BUFFER_MASK     0xFFF // 4095
#define LZS_BUFFER_START    4078

class LzsIO : public QIODevice
{
public:
	explicit LzsIO(QObject *parent = 0);
	LzsIO(const QByteArray &data, QObject *parent = 0);
	virtual bool open(OpenMode mode);
	virtual void close();
	virtual bool isSequential() const {
		return true;
	}
protected:
	virtual qint64 readData(char *data, qint64 maxSize);
	virtual qint64 writeData(const char *data, qint64 maxSize);
private:
	QByteArray _data;
	quint16 _curBuff;
	uchar _buffer[LZS_BUFFER_SIZE + 17]; // ring buffer of size 4096, with extra 17 bytes to facilitate string comparison
	quint16 _firstByte, _address, _length;
};

class LzsRandomAccess : public QIODevice
{
public:
	explicit LzsRandomAccess(const QByteArray &data, QObject *parent = 0);
	virtual bool open(OpenMode mode);
	virtual void close();
protected:
	virtual qint64 readData(char *data, qint64 maxSize);
	virtual qint64 writeData(const char *data, qint64 maxSize);
private:
	QByteArray _out;
	LzsIO _io;
	bool _endReached;
};

#endif // LZSIO_H
