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
#ifndef BACKGROUNDFILEPS_H
#define BACKGROUNDFILEPS_H

#include "BackgroundFile.h"
#include "BackgroundTiles.h"
#include "BackgroundTilesIO.h"
#include "BackgroundTextures.h"

class FieldPS;

class BackgroundFilePS : public BackgroundFile
{
public:
	explicit BackgroundFilePS(FieldPS *field);

	QPixmap openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL);
protected:
	quint16 textureWidth(const Tile &tile) const;
	quint8 depth(const Tile &tile) const;
	quint32 originInData(const Tile &tile) const;
	QRgb directColor(quint16 color) const;
private:
	static bool openPalettes(const QByteArray &data, QList<Palette *> &palettes);
	bool openTiles(const QByteArray &data);
	BackgroundTexturesPS textures;
};

#endif // BACKGROUNDFILEPS_H
