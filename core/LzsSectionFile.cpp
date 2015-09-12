#include "LzsSectionFile.h"
#include "Config.h"

LzsSectionFile::LzsSectionFile() :
	_sectionPositions(0), _io(0)
{
}

LzsSectionFile::~LzsSectionFile()
{
	if (_sectionPositions) {
		delete _sectionPositions;
	}
	if (_io) {
		delete _io;
	}
}

bool LzsSectionFile::open(const QByteArray &lzsData)
{
	if(lzsData.size() < 4) {
		qWarning() << "LzsSectionFile::open lzsData too short" << lzsData.size();
		return false;
	}

	const char *lzsDataConst = lzsData.constData();
	quint32 lzsSize;
	memcpy(&lzsSize, lzsDataConst, 4);

	if(!Config::value("lzsNotCheck").toBool()
			&& (quint32)lzsData.size() != lzsSize + 4) {
		return false;
	}

	if (_io) {
		delete _io;
	}

	_io = new LzsRandomAccess(lzsData.mid(4));

	if (!_io->open(QIODevice::ReadOnly)) {
		qWarning() << "LzsSectionFile::open cannot open io" << _io->errorString();
		return false;
	}

	if (!_sectionPositions) {
		_sectionPositions = new quint32[sectionCount()];
	}

	if (openHeader()) {
		for (quint8 i = 0; i < sectionCount() - 1; ++i) {
			if (_sectionPositions[i + 1] < _sectionPositions[i]) {
				qWarning() << "LzsSectionFile::open wrong order:" << i << _sectionPositions[i] << _sectionPositions[i + 1];
				return false;
			}
		}
	}

	return true;
}

void LzsSectionFile::saveStart()
{
	_data = _io->readAll();
}

bool LzsSectionFile::save(QByteArray &data)
{
	if (_data.isEmpty()) {
		qWarning() << "LzsSectionFile::save call saveStart() before save()";
		return false;
	}

	if (!writePositions(_data)) {
		return false;
	}
	data = _data;

	return true;
}

void LzsSectionFile::saveEnd()
{
	_data.clear();
}

void LzsSectionFile::clear()
{
	if (_sectionPositions) {
		delete _sectionPositions;
		_sectionPositions = 0;
	}
	_data.clear();
	_io->close();
}

int LzsSectionFile::sectionSize(quint8 id) const
{
	if (id + 1 >= sectionCount()) {
		return -1;
	}
	return sectionPos(id + 1) - sectionPos(id);
}

QByteArray LzsSectionFile::sectionData(quint8 id)
{
	if (_io->seek(sectionPos(id))) {
		int size = sectionSize(id);
		if (size < 0) {
			return _io->readAll();
		}
		return _io->read(size);
	}
	qWarning() << "LzsSectionFile::sectionData cannot seek to" << id;
	return QByteArray();
}

void LzsSectionFile::setSectionData(quint8 id, const QByteArray &data)
{
	if (_data.isEmpty()) {
		qWarning() << "LzsSectionFile::setSectionData call saveStart() before setSectionData()";
		return;
	}

	shiftPositionsAfter(id, setSectionData(sectionPos(id), sectionSize(id), data, _data));
}

void LzsSectionFile::shiftPositionsAfter(quint8 id, int shift)
{
	if (shift == 0) {
		return;
	}

	for (quint16 i = id + 1; i < sectionCount(); ++i) {
		_sectionPositions[i] += shift;
	}
}
