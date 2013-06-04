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
			infos.pos = device()->pos();
			textures->addTexInfos(texID, infos);

			if(!device()->seek(device()->pos() + depth * 65536)) {
				return false;
			}
		}
	}

	return true;
}

bool BackgroundTexturesIOPC::write(const BackgroundTexturesPC *textures) const
{
	if(!canWrite()) {
		return false;
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

	textures->clear();

	return false;
}

bool BackgroundTexturesIOPS::write(const BackgroundTexturesPS *textures) const
{
	if(!canWrite()) {
		return false;
	}

	return false;
}
