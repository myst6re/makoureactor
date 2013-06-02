#include "PaletteIO.h"

PaletteIO::PaletteIO() :
	_device(0)
{
}

PaletteIO::PaletteIO(QIODevice *device) :
	_device(device)
{
}

PaletteIO::~PaletteIO()
{
}

void PaletteIO::setDevice(QIODevice *device)
{
	_device = device;
}

QIODevice *PaletteIO::device() const
{
	return _device;
}

bool PaletteIO::canRead() const
{
	if(_device) {
		if(!_device->isOpen()) {
			return _device->open(QIODevice::ReadOnly);
		}
		return _device->isReadable();
	}
	return false;
}

bool PaletteIO::canWrite() const
{
	if(_device) {
		if(!_device->isOpen()) {
			return _device->open(QIODevice::WriteOnly);
		}
		return _device->isWritable();
	}
	return false;
}

bool PaletteIO::read(QList<Palette *> &palettes) const
{
	if(!canRead()) {
		return false;
	}

	QByteArray palData = device()->read(12);

	if(palData.size() < 12) {
		return false;
	}

	quint16 palH;

	memcpy(&palH, palData.constData() + 10, 2);

	palData = device()->read(palH * 512);

	if(palData.size() < palH * 512) {
		return false;
	}

	for(quint32 i=0 ; i<palH ; ++i) {
		palettes.append(createPalette(palData.constData() + i*512));
	}

	return readAfter(palettes);
}

bool PaletteIO::write(const QList<const Palette *> &palettes) const
{
	if(!canWrite()) {
		return false;
	}

	const quint16 palW = 256, palH = palettes.size();
	const quint16 size = 8 + 512 * palH;
	const quint16 palX = 0, palY = 480;

	device()->write((char *)&size, 2);
	device()->write((char *)&palX, 2);
	device()->write((char *)&palY, 2);
	device()->write((char *)&palW, 2);
	device()->write((char *)&palH, 2);

	foreach(const Palette *pal, palettes) {
		device()->write(pal->toByteArray());
	}

	return writeAfter(palettes);
}

PaletteIOPC::PaletteIOPC(QIODevice *device) :
	PaletteIO(device)
{
}

void PaletteIOPC::setDeviceAlpha(QIODevice *device)
{
	_deviceAlpha = device;
}

QIODevice *PaletteIOPC::deviceAlpha() const
{
	return _deviceAlpha;
}

bool PaletteIOPC::canReadAlpha() const
{
	if(_deviceAlpha) {
		if(!_deviceAlpha->isOpen()) {
			return _deviceAlpha->open(QIODevice::ReadOnly);
		}
		return _deviceAlpha->isReadable();
	}
	return false;
}

bool PaletteIOPC::canWriteAlpha() const
{
	if(_deviceAlpha) {
		if(!_deviceAlpha->isOpen()) {
			return _deviceAlpha->open(QIODevice::WriteOnly);
		}
		return _deviceAlpha->isWritable();
	}
	return false;
}

bool PaletteIOPC::readAfter(QList<Palette *> &palettes) const
{
	if(!canReadAlpha()) {
		return false;
	}

	QByteArray palFlags = deviceAlpha()->read(24);

	if(palFlags.size() < 24) {
		return false;
	}

	quint8 palId=0;

	foreach(Palette *palette, palettes) {
		if(palId >= 24) {
			/* Not really an error, I don't know the behavior
			 * when there are more than 24 palettes. */
			break;
		}

		((PalettePC *)palette)->setTransparency(palFlags.at(palId));

		++palId;
	}

	return true;
}

bool PaletteIOPC::writeAfter(const QList<const Palette *> &palettes) const
{
	if(!canWriteAlpha()) {
		return false;
	}

	quint8 palId=0;

	foreach(const Palette *palette, palettes) {
		if(palId >= 24) {
			break;
		}

		quint8 trans = ((PalettePC *)palette)->transparency();
		if(deviceAlpha()->write((char *)&trans, 1) != 1) {
			return false;
		}

		++palId;
	}

	if(palId < 24 &&
			deviceAlpha()->write(QByteArray(24 - palId, '\0')) != 24 - palId) {
		return false;
	}

	return true;
}

PaletteIOPS::PaletteIOPS(QIODevice *device) :
	PaletteIO(device)
{
}
