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
#include "BackgroundTexturesIO.h"

BackgroundTexturesIO::BackgroundTexturesIO(QIODevice *device) :
	_device(device)
{
}

BackgroundTexturesIO::~BackgroundTexturesIO()
{
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

bool BackgroundTexturesIOPC::read(BackgroundTexturesPC *textures)
{
	if(!canRead()) {
		return false;
	}

	textures->clear();
	qint64 initPos = device()->pos();

	for(quint8 texID=0 ; texID<42 ; ++texID) {

		quint16 exists;

		if(device()->read((char *)&exists, 2) != 2) {
			qWarning() << "BackgroundTexturesIOPC::read cannot read exists" << texID;
			return false;
		}

		if(bool(exists)) {
			quint16 size, depth;

			if(device()->read((char *)&size, 2) != 2 ||
					device()->read((char *)&depth, 2) != 2) {
				qWarning() << "BackgroundTexturesIOPC::read cannot read size or depth" << texID;
				return false;
			}

			BackgroundTexturesPCInfos infos;
			infos.size = size;
			infos.depth = depth;
			infos.pos = device()->pos() - initPos;
			textures->addTexInfos(texID, infos);

			if(!device()->seek(device()->pos() + (depth == 0 ? 32768 : depth * 65536))) {
				qWarning() << "BackgroundTexturesIOPC::read cannot seek texture" << texID;
				return false;
			}
		}
	}

	if(!device()->seek(initPos)) {
		qWarning() << "BackgroundTexturesIOPC::read cannot reset";
		return false;
	}
	textures->setData(device()->readAll());

	return true;
}

bool BackgroundTexturesIOPC::write(const BackgroundTexturesPC *textures)
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

			quint16 size = infos.size, depth = infos.depth;

			if(device()->write((char *)&size, 2) != 2 ||
					device()->write((char *)&depth, 2) != 2) {
				return false;
			}

			if(device()->write(textures->data().mid(infos.pos, infos.depth * 65536))
					!= infos.depth * 65536) {
				return false;
			}
		}
	}

	return true;
}

BackgroundTexturesIOPS::BackgroundTexturesIOPS(QIODevice *device) :
	BackgroundTexturesIO(device)
{
}

bool BackgroundTexturesIOPS::read(BackgroundTexturesPS *textures)
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

	if (mimDataDec.size() != headerPalSize + headerImg.size + headerEffect.size) {
		qWarning() << "BackgroundTexturesIOPS::open padding after" << (mimDataDec.size() - int(headerPalSize + headerImg.size + headerEffect.size));
	}

	return true;
}

bool BackgroundTexturesIOPS::write(const BackgroundTexturesPS *textures)
{
	if(!canWrite()) {
		return false;
	}

	if(device()->write(textures->data()) != textures->data().size()) {
		return false;
	}

	return true;
}
