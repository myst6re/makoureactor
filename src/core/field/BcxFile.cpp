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
#include "BcxFile.h"

BcxFile::BcxFile(QIODevice *io) :
	BsxFile(io)
{
}

BcxFile::~BcxFile()
{
}

bool BcxFile::read(FieldModelFilePS &model)
{
	if (!readHeader()) {
		return false;
	}

	if (!seekModels()) {
		return false;
	}

	BcxModelHeader modelHeader;

	if (sizeof(BcxModelHeader) != device()->read((char *)&modelHeader, sizeof(BcxModelHeader))) {
		return false;
	}

	return readModel(modelHeader.numBones,
					 modelHeader.numParts,
					 modelHeader.numAnimations,
					 &model);
}

bool BcxFile::write(const FieldModelFilePS &model)
{
	Q_UNUSED(model)
	return false;
}
