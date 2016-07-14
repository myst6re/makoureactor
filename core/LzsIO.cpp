#include "LzsIO.h"

LzsIO::LzsIO(const QByteArray &data, QObject *parent) :
	QIODevice(parent), _data(data),
    _fileData(reinterpret_cast<const quint8 *>(_data.constData())),
	_endFileData(reinterpret_cast<const quint8 *>(_data.constData() + _data.size()))
{
}

LzsIO::LzsIO(const QByteArray &data, int start, int size, QObject *parent) :
	QIODevice(parent), _data(data),
    _fileData(reinterpret_cast<const quint8 *>(_data.constData() + start)),
	_endFileData(reinterpret_cast<const quint8 *>(_data.constData() + start + size))
{
}

bool LzsIO::open(OpenMode mode)
{
	_firstByte = 0;
	_address = 0xFFFF;
	_curBuff = LZS_BUFFER_START;
	memset(_buffer, 0, LZS_BUFFER_START); // Le buffer de 4096 octets est initialisé à 0

	return QIODevice::open(mode | QIODevice::Unbuffered);
}

void LzsIO::close()
{
	_data.clear();
	QIODevice::close();
}

qint64 LzsIO::readData(char *data, qint64 maxSize)
{
	qint64 curResult = 0;

	if (maxSize < 0) {
		return -1;
	}

	if (maxSize == 0 || _endFileData <= _fileData) {
		return 0;
	}

	if (_address != 0xFFFF) {
		for (quint16 i = _address + 1; i <= _length; ++i) {
			data[curResult] = _buffer[_curBuff] = _buffer[i & LZS_BUFFER_MASK]; // On va chercher l'octet (qui est décompressé) dans le buffer à l'address indiquée puis on le sauvegarde dans la chaine finale et le buffer.
			_curBuff = (_curBuff + 1) & LZS_BUFFER_MASK;
			++curResult;

			if (curResult >= maxSize) {
				_address = i;
				return curResult; // Done
			}
		}
	}

	forever {
		if (((_firstByte >>= 1) & 0x100) == 0) {
			_firstByte = *_fileData++ | 0xFF00; // On récupère le premier octet puis on avance d'un octet
		}

		if (_fileData >= _endFileData) {
			return curResult; // Done
		}

		if (_firstByte & 1) {
			data[curResult] = _buffer[_curBuff] = *_fileData++; // On récupère l'octet (qui n'est pas compressé) et on le sauvegarde dans la chaine finale (result) et dans le buffer. Et bien sûr on fait avancer les curseurs d'un octet.
			_curBuff = (_curBuff + 1) & LZS_BUFFER_MASK; // Le curseur du buffer doit toujours être entre 0 et 4095
			++curResult;

			if (curResult >= maxSize) {
				_address = 0xFFFF;
				return curResult; // Done
			}
		} else {
			_address = *_fileData++;
			_length = *_fileData++;
			_address |= (_length & 0xF0) << 4; // On récupère l'address dans les deux octets (qui sont "compressés")
			_length = (_length & 0xF) + 2 + _address;

			for (quint16 i = _address; i <= _length; ++i) {
				data[curResult] = _buffer[_curBuff] = _buffer[i & LZS_BUFFER_MASK]; // On va chercher l'octet (qui est décompressé) dans le buffer à l'address indiquée puis on le sauvegarde dans la chaine finale et le buffer.
				_curBuff = (_curBuff + 1) & LZS_BUFFER_MASK;
				++curResult;

				if (curResult >= maxSize) {
					_address = i;
					return curResult; // Done
				}
			}
		}
	}
}

qint64 LzsIO::writeData(const char *data, qint64 maxSize)
{
	Q_UNUSED(data)
	Q_UNUSED(maxSize)
	return -1; // Not implemented
}

LzsRandomAccess::LzsRandomAccess(const QByteArray &data, QObject *parent) :
	QIODevice(parent), _io(new LzsIO(data, parent)), _endReached(false)
{
}

LzsRandomAccess::LzsRandomAccess(const QByteArray &data, int start, int size,
                                 QObject *parent) :
	QIODevice(parent), _io(new LzsIO(data, start, size, parent)),
    _endReached(false)
{
}

LzsRandomAccess::~LzsRandomAccess()
{
	delete _io;
}

bool LzsRandomAccess::open(OpenMode mode)
{
	int sizeAlloc = _io->size() * 3; // Estimation
	_out.reserve(sizeAlloc);

	mode |= QIODevice::Unbuffered;
	return _io->open(mode) && QIODevice::open(mode);
}

void LzsRandomAccess::close()
{
	_io->close();
	_out.clear();
	_endReached = false;
	QIODevice::close();
}

qint64 LzsRandomAccess::readData(char *data, qint64 maxSize)
{
	const qint64 startPos = pos(),
			endPos = startPos + maxSize;

	if (!_endReached && endPos > _out.size()) {
		const qint64 toRead = endPos - _out.size();
		QByteArray readData = _io->read(toRead);
		qint64 size = readData.size();
		if (size < 0) {
			return size;
		}

		if (size < toRead) {
			_endReached = true;
			_io->close();
		}

		_out.append(readData);
	}

	if (startPos + maxSize > _out.size()) {
		maxSize = qMin(maxSize, qint64(_out.size()) - startPos);
	}

	memcpy(data, _out.constData() + startPos, maxSize);

	return maxSize;
}

qint64 LzsRandomAccess::writeData(const char *data, qint64 maxSize)
{
	Q_UNUSED(data)
	Q_UNUSED(maxSize)
	return -1; // Not implemented
}
