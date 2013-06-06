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
#ifndef TDBFILE_H
#define TDBFILE_H

#include <QtCore>
#include <QImage>

typedef struct {
	quint32 size;
	quint16 imageCount;
	quint16 paletteCount;
	quint32 imageOffset;
	quint32 paletteOffset;
} TdbHeader;

class TdbFile
{
public:
	enum TextureType {
		Eye=0,
		EyeClosed1, EyeClosed2,
		EyeOpened1, EyeOpened2,
		MouthClosed, MouthOpened,
		Empty
	};

	TdbFile();
	bool open(const QByteArray &data);
	QImage texture(quint8 faceID, TextureType type);
private:
	static int faceIdToImageId(quint8 faceID, TextureType type);
	TdbHeader header;
	QByteArray data;
};

#endif // TDBFILE_H
