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
	IO(device)
{
}

BackgroundTexturesIOPC::BackgroundTexturesIOPC(QIODevice *device) :
	BackgroundTexturesIO(device)
{
}

bool BackgroundTexturesIOPC::read(BackgroundTexturesPC *textures) const
{
	if (!canRead()) {
		return false;
	}

	textures->clear();
	qint64 initPos = device()->pos();

	for (quint8 texID=0; texID<BACKGROUND_TEXTURE_PC_MAX_COUNT; ++texID) {

		quint16 exists;

		if (device()->read((char *)&exists, 2) != 2) {
			qWarning() << "BackgroundTexturesIOPC::read cannot read exists" << texID;
			return false;
		}

		if (bool(exists)) {
			quint16 size, depth;

			if (device()->read((char *)&size, 2) != 2 ||
					device()->read((char *)&depth, 2) != 2) {
				qWarning() << "BackgroundTexturesIOPC::read cannot read size or depth" << texID;
				return false;
			}

			BackgroundTexturesPCInfos infos;
			infos.size = size;
			infos.depth = depth;
			infos.pos = device()->pos() - initPos;
			textures->addTexInfos(texID, infos);

			if (!device()->seek(device()->pos() + (depth == 0 ? 32768 : depth * 65536))) {
				qWarning() << "BackgroundTexturesIOPC::read cannot seek texture" << texID;
				return false;
			}
		}
	}

	if (!device()->seek(initPos)) {
		qWarning() << "BackgroundTexturesIOPC::read cannot reset";
		return false;
	}
	textures->setData(device()->readAll());

	return true;
}

bool BackgroundTexturesIOPC::write(const BackgroundTexturesPC *textures) const
{
	if (!canWrite() || !textures) {
		return false;
	}

	for (quint8 texID=0; texID<BACKGROUND_TEXTURE_PC_MAX_COUNT; ++texID) {

		quint16 exists = textures->hasTex(texID);

		if (device()->write((char *)&exists, 2) != 2) {
			return false;
		}

		if (bool(exists)) {
			BackgroundTexturesPCInfos infos = textures->texInfos(texID);

			quint16 size = infos.size, depth = infos.depth;

			if (device()->write((char *)&size, 2) != 2 ||
					device()->write((char *)&depth, 2) != 2) {
				return false;
			}

			int dataSize = infos.depth == 0 ? 32768 : infos.depth * 65536;

			if (device()->write(textures->data().mid(infos.pos, dataSize)) != dataSize) {
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

bool BackgroundTexturesIOPS::read(BackgroundTexturesPS *textures) const
{
	if (!canRead()) {
		return false;
	}

	QByteArray mimDataDec = device()->readAll();
	const char *constMimData = mimDataDec.constData();
	quint32 mimDataSize = mimDataDec.size(), headerPalSize;
	MIM headerImg, headerEffect = MIM();

	memcpy(&headerPalSize, constMimData, 4);

	if (mimDataSize < headerPalSize + 12) {
		return false;
	}

	memcpy(&headerImg, constMimData + headerPalSize, 12);

	headerImg.w *= 2;

	if (headerPalSize+headerImg.size+12 <= mimDataSize) {
		memcpy(&headerEffect, constMimData + headerPalSize+headerImg.size, 12);
		headerEffect.w *= 2;
	} else {
		headerEffect.size = 4;
	}

	textures->setHeaderImg(headerImg);
	textures->setHeaderEffect(headerEffect);
	textures->setData(mimDataDec.mid(headerPalSize));

	if (quint32(mimDataDec.size()) != headerPalSize + headerImg.size + headerEffect.size) {
		qWarning() << "BackgroundTexturesIOPS::open padding after" << (mimDataDec.size() - int(headerPalSize + headerImg.size + headerEffect.size));
	}

	return true;
}

bool BackgroundTexturesIOPS::write(const BackgroundTexturesPS *textures) const
{
	if (!canWrite()) {
		return false;
	}

	// Assume device position is correct (after palette section)
	return device()->write(textures->data()) == textures->data().size();
}
