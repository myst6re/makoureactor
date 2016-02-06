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
#ifndef BACKGROUNDTEXTURESIO_H
#define BACKGROUNDTEXTURESIO_H

#include <QtCore>
#include "../IO.h"
#include "BackgroundTextures.h"

class BackgroundTexturesIO : public IO
{
public:
	explicit BackgroundTexturesIO(QIODevice *device);
	virtual ~BackgroundTexturesIO() {}

	virtual bool read(BackgroundTextures *textures) {
		Q_UNUSED(textures)
		return false;
	}
	virtual bool write(const BackgroundTextures *textures) {
		Q_UNUSED(textures)
		return false;
	}
};

class BackgroundTexturesIOPC : public BackgroundTexturesIO
{
public:
	explicit BackgroundTexturesIOPC(QIODevice *device);
	bool read(BackgroundTexturesPC *textures);
	bool write(const BackgroundTexturesPC *textures);
};

class BackgroundTexturesIOPS : public BackgroundTexturesIO
{
public:
	explicit BackgroundTexturesIOPS(QIODevice *device);
	bool read(BackgroundTexturesPS *textures);
	bool write(const BackgroundTexturesPS *textures);
};

#endif // BACKGROUNDTEXTURESIO_H
