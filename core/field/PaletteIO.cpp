/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
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

bool PaletteIO::read(Palettes &palettes) const
{
	if(!canRead()) {
		return false;
	}

	QByteArray palData = device()->read(12);

	if(palData.size() != 12) {
		qWarning() << "PaletteIO::read Pal size too short";
		return false;
	}

	quint16 palH;

	memcpy(&palH, palData.constData() + 10, 2);

	palData = device()->read(palH * 512);

	if(palData.size() != palH * 512) {
		qWarning() << "PaletteIO::read Pal size too short 2";
		return false;
	}

	for(quint32 i=0 ; i<palH ; ++i) {
		palettes.append(createPalette(palData.constData() + i*512));
	}

	return readAfter(palettes);
}

bool PaletteIO::write(const Palettes &palettes) const
{
	if(!canWrite()) {
		return false;
	}

	const quint16 palW = 256, palH = palettes.size();
	const quint32 size = 8 + 512 * palH;
	const quint16 palX = 0, palY = 480;

	device()->write((char *)&size, 4);
	device()->write((char *)&palX, 2);
	device()->write((char *)&palY, 2);
	device()->write((char *)&palW, 2);
	device()->write((char *)&palH, 2);

	foreach(const Palette *pal, palettes) {
		device()->write(pal->toByteArray());
	}

	return writeAfter(palettes);
}

PaletteIOPC::PaletteIOPC(QIODevice *device, QIODevice *deviceAlpha) :
	PaletteIO(device), _deviceAlpha(deviceAlpha)
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

bool PaletteIOPC::readAfter(Palettes &palettes) const
{
	if(!canReadAlpha()) {
		return false;
	}

	QByteArray palFlags = deviceAlpha()->read(16);

	if(palFlags.size() < 16) {
		return false;
	}

	quint8 palId=0;

	foreach(Palette *palette, palettes) {
		if(palId >= 16) {
			break;
		}

		((PalettePC *)palette)->setTransparency(palFlags.at(palId));

		++palId;
	}

	return true;
}

bool PaletteIOPC::writeAfter(const Palettes &palettes) const
{
	if(!canWriteAlpha()) {
		return false;
	}

	quint8 palId=0;

	foreach(const Palette *palette, palettes) {
		if(palId >= 16) {
			break;
		}

		quint8 trans = ((PalettePC *)palette)->transparency();
		if(deviceAlpha()->write((char *)&trans, 1) != 1) {
			return false;
		}

		++palId;
	}

	if(deviceAlpha()->write(QByteArray(24 - palId, '\0')) != 24 - palId) {
		return false;
	}

	return true;
}

PaletteIOPS::PaletteIOPS(QIODevice *device) :
	PaletteIO(device)
{
}
