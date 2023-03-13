/****************************************************************************
 ** Néo-Midgar Final Fantasy VII French Retranslation
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

#include <QtCore>
#include <IO>
#include "BsxFile.h"

struct BcxModelHeader {
	quint16 unknown;          // always 0xff01
	quint8 numBones;
	quint8 numParts;
	quint8 numAnimations;
	quint8 blank1[17];
	quint16 scale;
	quint16 offsetParts;      // relative to the end of this structure
	quint16 offsetAnimations; // relative to the end of this structure
	quint32 offsetSkeleton;   // offset to skeleton data, substract by 0x80000000 to obtain the right offset
	quint32 blank2;
};

class BcxFile : public BsxFile
{
public:
	explicit BcxFile(QIODevice *io);
	virtual ~BcxFile() override;

	bool read(FieldModelFilePS &model);
	bool write(const FieldModelFilePS &model);
private:
	using BsxFile::read;
	using BsxFile::write;
};
