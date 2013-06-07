#include "BackgroundTexturesIO.h"

BackgroundTexturesIO::BackgroundTexturesIO(QIODevice *device) :
	_device(device)
{
}

BackgroundTexturesIO::~BackgroundTexturesIO()
{
}

void BackgroundTexturesIO::setDevice(QIODevice *device)
{
	_device = device;
}

QIODevice *BackgroundTexturesIO::device() const
{
	return _device;
}

bool BackgroundTexturesIO::canRead() const
{
	if(_device) {
		if(!_device->isOpen()) {
			return _device->open(QIODevice::ReadOnly);
		}
		return _device->isReadable();
	}
	return false;
}

bool BackgroundTexturesIO::canWrite() const
{
	if(_device) {
		if(!_device->isOpen()) {
			return _device->open(QIODevice::WriteOnly);
		}
		return _device->isWritable();
	}
	return false;
}

BackgroundTexturesIOPC::BackgroundTexturesIOPC(QIODevice *device) :
	BackgroundTexturesIO(device)
{
}

bool BackgroundTexturesIOPC::read(BackgroundTexturesPC *textures) const
{
	if(!canRead()) {
		return false;
	}

	textures->clear();
	qint64 initPos = device()->pos();

	for(quint8 texID=0 ; texID<42 ; ++texID) {

		quint16 exists;

		if(device()->read((char *)&exists, 2) != 2) {
			return false;
		}

		if(bool(exists)) {
			quint16 size, depth;

			if(device()->read((char *)&size, 2) != 2 ||
					device()->read((char *)&depth, 2) != 2) {
				return false;
			}

			BackgroundTexturesPCInfos infos;
			infos.size = size;
			infos.depth = depth;
			infos.pos = device()->pos() - initPos;
			textures->addTexInfos(texID, infos);

			if(!device()->seek(device()->pos() + depth * 65536)) {
				return false;
			}
		}
	}

	device()->seek(initPos);
	textures->setData(device()->readAll());

	return true;
}

bool BackgroundTexturesIOPC::write(const BackgroundTexturesPC *textures) const
{
	if(!canWrite()) {
		return false;
	}

	for(quint8 texID=0 ; texID<42 ; ++texID) {

		quint16 exists = textures->hasTex(texID);

		if(device()->write((char *)&exists, 2) != 2) {
			return false;
		}

		if(bool(exists)) {
			BackgroundTexturesPCInfos infos = textures->texInfos(texID);

			if(device()->write((char *)&infos.size, 2) != 2 ||
					device()->write((char *)&infos.depth, 2) != 2) {
				return false;
			}

			if(device()->write(textures->data().mid(infos.pos, infos.depth * 65536))
					!= infos.depth * 65536) {
				return false;
			}
		}
	}

	return false;
}

BackgroundTexturesIOPS::BackgroundTexturesIOPS(QIODevice *device) :
	BackgroundTexturesIO(device)
{
}

bool BackgroundTexturesIOPS::read(BackgroundTexturesPS *textures) const
{
	if(!canRead()) {
		return false;
	}

	QByteArray mimDataDec = device()->readAll();
	const char *constMimData = mimDataDec.constData();
	quint32 mimDataSize = mimDataDec.size(), headerPalSize;
	MIM headerImg, headerEffect = MIM();

	memcpy(&headerPalSize, constMimData, 4);

	if(mimDataSize < headerPalSize + 12) {
		return false;
	}

	memcpy(&headerImg, constMimData + headerPalSize, 12);

	headerImg.w *= 2;

	if(headerPalSize+headerImg.size+12 <= mimDataSize) {
		memcpy(&headerEffect, constMimData + headerPalSize+headerImg.size, 12);
		headerEffect.w *= 2;
	} else {
		headerEffect.size = 4;
	}

	textures->setDataPos(headerPalSize);
	textures->setHeaderImg(headerImg);
	textures->setHeaderEffect(headerEffect);
	textures->setData(mimDataDec);

	return true;
}

bool BackgroundTexturesIOPS::write(const BackgroundTexturesPS *textures) const
{
	if(!canWrite()) {
		return false;
	}

	return false;
}
