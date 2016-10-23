#include "SectionFile.h"
#include "Config.h"
#include "LZS.h"

SectionFile::SectionFile() :
	_io(0)
{
}

SectionFile::~SectionFile()
{
	if (_io) {
		delete _io;
	}
}

bool SectionFile::openLzs(const QByteArray &lzsData)
{
	if(!LzsIO::checkHeader(lzsData)) {
		return false;
	}

	if (_io) {
		delete _io;
	}

	_io = new LzsRandomAccess(lzsData, 4, lzsData.size() - 4);

	return open();
}

bool SectionFile::openData(const QByteArray &data)
{
	QBuffer *io = new QBuffer();
	io->setData(data);

	if (_io) {
		delete _io;
	}

	_io = io;

	return open();
}

bool SectionFile::open()
{
	if (sectionCount() > SECTION_FILE_MAX_POSITIONS) {
		qFatal("SectionFile::open sectionCount too short");
	}

	if (!_io->open(QIODevice::ReadOnly)) {
		qWarning() << "SectionFile::open cannot open io" << _io->errorString();
		return false;
	}

	if (openHeader()) {
		for (quint8 i = 0; i < sectionCount() - 1; ++i) {
			if (_sectionPositions[i + 1] < _sectionPositions[i]) {
				qWarning() << "SectionFile::open wrong order:" << i << _sectionPositions[i] << _sectionPositions[i + 1];
				return false;
			}
		}
	}

	return true;
}

void SectionFile::saveStart()
{
	_io->reset();
	_data = _io->readAll();
}

bool SectionFile::save(QByteArray &data, bool compressed)
{
	if (_data.isEmpty()) {
		qWarning() << "SectionFile::save call saveStart() before save()";
		return false;
	}

	if (!writePositions(_data)) {
		return false;
	}

	if (compressed) {
		data = LZS::compressWithHeader(_data);
	} else {
		data = _data;
	}

	return true;
}

void SectionFile::saveEnd()
{
	_data.clear();
}

void SectionFile::clear()
{
	_data.clear();
	_io->close();
}

quint32 SectionFile::sectionSize(quint8 id, bool &eof) const
{
	if (id + 1 >= sectionCount()) {
		eof = true;
		return 0;
	}
	eof = false;
	return sectionPos(id + 1) - sectionPos(id);
}

QByteArray SectionFile::sectionData(quint8 id)
{
	if (_io->seek(sectionPos(id))) {
		bool eof;
		quint32 size = sectionSize(id, eof);
		if (eof) {
			return _io->readAll();
		}
		return _io->read(size);
	}
	qWarning() << "SectionFile::sectionData cannot seek to" << id;
	return QByteArray();
}

void SectionFile::setSectionData(quint8 id, const QByteArray &data)
{
	if (_data.isEmpty()) {
		qWarning() << "SectionFile::setSectionData call saveStart() before setSectionData()";
		return;
	}

	bool eof;
	quint32 pos = sectionPos(id),
	        size = sectionSize(id, eof);
	if (eof) {
		size = qMax(quint32(0), quint32(_data.size()) - pos);
	}

	shiftPositionsAfter(id, setSectionData(int(pos), int(size), data, _data));
}

void SectionFile::shiftPositionsAfter(quint8 id, int shift)
{
	if (shift == 0) {
		return;
	}

	for (quint16 i = id + 1; i < sectionCount(); ++i) {
		_sectionPositions[i] = quint32(int(_sectionPositions[i]) + shift);
	}
}
