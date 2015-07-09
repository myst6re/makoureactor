/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2015 Arzel Jérôme <myst6re@gmail.com>
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
#include "FieldModelAnimationIOPC.h"

FieldModelAnimationIOPC::FieldModelAnimationIOPC(QIODevice *device) :
	FieldModelAnimationIO(device)
{
}

bool FieldModelAnimationIOPC::read(FieldModelAnimation &animation, int maxFrames) const
{
	if (!canRead()) {
		return false;
	}

	AHeader header;
	PolyVertex rot, trans;

	if (device()->read((char *)&header, 36) != 36
			|| header.framesCount == 0
			|| device()->pos() + header.framesCount * (24 + 12 * header.boneCount) > device()->size()) {
		return false;
	}

	if (maxFrames >= 0) {
		header.framesCount = qMin(header.framesCount, quint32(maxFrames));
	}

	for (quint32 i = 0; i < header.framesCount; ++i) {
		if (!device()->seek(device()->pos() + 12)) {
			return false;
		}
		if (device()->read((char *)&trans, 12) != 12) {
			return false;
		}

		trans.x = trans.x / MODEL_SCALE_PC;
		trans.y = trans.y / MODEL_SCALE_PC;
		trans.z = trans.z / MODEL_SCALE_PC;

		QList<PolyVertex> rotationCoords;

		for (quint32 j = 0; j < header.boneCount; ++j) {
			if(device()->read((char *)&rot, 12) != 12) {
				return false;
			}
			rotationCoords.append(rot);
		}
		animation.insertFrame(i, rotationCoords, QList<PolyVertex>() << trans);
	}

	return true;
}

bool FieldModelAnimationIOPC::write(const FieldModelAnimation &animation) const
{
	Q_UNUSED(animation)
	// TODO
	return false;
}

