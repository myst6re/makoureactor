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
#include "BackgroundIO.h"
#include "BackgroundTiles.h"
#include "BackgroundTilesIO.h"
#include "BackgroundTextures.h"
#include "BackgroundTexturesIO.h"

class FieldPS;
class FieldPC;
class BackgroundFilePC;

class BackgroundFilePS : public BackgroundFile
{
public:
	explicit BackgroundFilePS(FieldPS *field);
	BackgroundFilePS(const BackgroundFilePS &other);

	bool open();
	bool open(const QByteArray &mimData, const QByteArray &tilesData);
	BackgroundFilePC toPC(FieldPC *field) const;
};

#endif // BACKGROUNDFILEPS_H
