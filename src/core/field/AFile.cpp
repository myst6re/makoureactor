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
#include "AFile.h"
#include "PFile.h"

AFile::AFile(QIODevice *device) :
	IO(device)
{
}

AFile::~AFile()
{
}

bool AFile::readHeader(AHeader &header) const
{
	if (!canRead()) {
		return false;
	}

	if (device()->read((char *)&header, 36) != 36
	    || header.framesCount == 0
	    || device()->pos() + header.framesCount * (24 + 12 * header.boneCount) > device()->size()) {
		return false;
	}

	return true;
}

bool AFile::read(FieldModelAnimation &animation, int maxFrames) const
{
	if (!canRead()) {
		return false;
	}

	AHeader header;
	PolyVertex rot, trans;

	if (!readHeader(header)) {
		return false;
	}

	if (maxFrames >= 0) {
		header.framesCount = qMin(header.framesCount, quint32(maxFrames));
	}

	for (qint64 i = 0; i < header.framesCount; ++i) {
		if (device()->read((char *)&rot, 12) != 12) {
			return false;
		}

		animation.setInitialRot(rot);

		if (device()->read((char *)&trans, 12) != 12) {
			return false;
		}

		trans.x /= MODEL_SCALE_PC;
		trans.y /= MODEL_SCALE_PC;
		trans.z /= MODEL_SCALE_PC;

		QList<PolyVertex> rotationCoords;

		for (quint32 j = 0; j < header.boneCount; ++j) {
			if (device()->read((char *)&rot, 12) != 12) {
				return false;
			}
			rotationCoords.append(rot);
		}
		animation.insertFrame(int(i), rotationCoords, QList<PolyVertex>() << trans);
	}

	return true;
}

bool AFile::write(const FieldModelAnimation &animation) const
{
	if (!canWrite()) {
		return false;
	}
	
	AHeader header;
	header.version = 1;
	header.framesCount = animation.frameCount();
	header.boneCount = animation.boneCount();
	header.rotationOrder[0] = 1;
	header.rotationOrder[1] = 0;
	header.rotationOrder[2] = 2;
	header.unused = 0;
	header.runtimeData[0] = 0;
	header.runtimeData[1] = 0;
	header.runtimeData[2] = 0;
	header.runtimeData[3] = 0;
	header.runtimeData[4] = 0;
	
	if (device()->write((const char *)&header, sizeof(AHeader)) != sizeof(AHeader)) {
		return false;
	}
	
	for (qint64 i = 0; i < header.framesCount; ++i) {
		if (device()->write((const char *)&animation.initialRot(), 12) != 12) {
			return false;
		}
		
		PolyVertex trans = animation.translations(i).at(0);

		trans.x *= MODEL_SCALE_PC;
		trans.y *= MODEL_SCALE_PC;
		trans.z *= MODEL_SCALE_PC;
		
		if (device()->write((const char *)&trans, 12) != 12) {
			return false;
		}
		
		for (quint32 j = 0; j < header.boneCount; ++j) {
			const PolyVertex &rot = animation.rotations(i).at(j);
			
			if (device()->write((const char *)&rot, 12) != 12) {
				return false;
			}
		}
	}
	
	return true;
}
