/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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

#include <IO>
#include "FieldModelAnimation.h"

struct AHeader {
	quint32 version;
	quint32 framesCount;
	quint32 boneCount;
	quint8 rotationOrder[3];
	quint8 unused;
	quint32 runtimeData[5];
};

class AFile : public IO
{
public:
	explicit AFile(QIODevice *device);
	virtual ~AFile() override;

	bool readHeader(AHeader &header) const;
	bool read(FieldModelAnimation &animation, int maxFrames = -1) const;
	bool write(const FieldModelAnimation &animation) const;
};
