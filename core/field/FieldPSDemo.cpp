#include "FieldPSDemo.h"
#include "BackgroundFilePS.h"

FieldPSDemo::FieldPSDemo(const QString &name, FieldArchiveIOPS *io) :
	FieldPS(name, io), _dataAte(0), _dataMap(0), _dataId(0), _dataCa(0)
{
}

FieldPSDemo::FieldPSDemo(const Field &field) :
	FieldPS(field), _dataAte(0), _dataMap(0), _dataId(0), _dataCa(0)
{
}

FieldPSDemo::~FieldPSDemo()
{
	if (_dataAte) {
		delete _dataAte;
	}
	if (_dataMap) {
		delete _dataMap;
	}
	if (_dataId) {
		delete _dataId;
	}
	if (_dataCa) {
		delete _dataCa;
	}
}

LzsRandomAccess *FieldPSDemo::openFileFromIO(const QString &extension)
{
	QByteArray lzsData = io()->fieldData(this, extension, false);
	if(!LzsIO::checkHeader(lzsData)) {
		return NULL;
	}

	return new LzsRandomAccess(lzsData, 4, lzsData.size() - 4);
}

bool FieldPSDemo::open2()
{
	_dataAte = openFileFromIO("ATE");
	_dataMap = openFileFromIO("MAP");
	_dataId = openFileFromIO("ID");
	_dataCa = openFileFromIO("CA");

	return _dataAte != NULL;
}

void FieldPSDemo::saveStart()
{
}

bool FieldPSDemo::save2(QByteArray &data, bool compress)
{
	Q_UNUSED(data);
	Q_UNUSED(compress);
	return false; // TODO
}

void FieldPSDemo::saveEnd()
{
}

QByteArray FieldPSDemo::fileData(LzsRandomAccess *io)
{
	if(!io || !io->reset()) {
		return QByteArray();
	}
	return io->readAll();
}

QByteArray FieldPSDemo::sectionData(CommonSection section)
{
	switch(section) {
	case _ScriptsTextsAkaos:
		return fileData(_dataAte);
	case _Camera:
		return fileData(_dataCa);
	case _Walkmesh:
		return fileData(_dataId);
	case _Encounter:
		return QByteArray(); // TODO
	case _Inf:
		return QByteArray(); // TODO
	}
	return QByteArray();
}

bool FieldPSDemo::setSectionData(CommonSection section, const QByteArray &data)
{
	switch(section) {
	case _ScriptsTextsAkaos:
		_saveAte = data;
		return true;
	case _Camera:
		_saveCa = data;
		return true;
	case _Walkmesh:
		_saveId = data;
		return true;
	case _Encounter:
		return false; // TODO
	case _Inf:
		return false; // TODO
	}
	return false;
}
