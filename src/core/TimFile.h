/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
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
#pragma once

#include <QtCore>
#include <QImage>
#include "TextureFile.h"

class TimFile : public TextureFile
{
public:
	TimFile() : TextureFile(), bpp(0), palX(0), palY(0), palW(0), palH(0), imgX(0), imgY(0) {}
	explicit TimFile(const QByteArray &data);
	TimFile(const TextureFile &texture, quint8 bpp, quint16 palX, quint16 palY, quint16 palW, quint16 palH, quint16 imgX, quint16 imgY);
	bool open(const QByteArray &data);
	bool save(QByteArray &data) const;
private:
	quint8 bpp;
	quint32 sizeImgSection;
	quint16 palX, palY;
	quint16 palW, palH;
	quint16 imgX, imgY;
	QList<QBitArray> _alphaBits;
};
