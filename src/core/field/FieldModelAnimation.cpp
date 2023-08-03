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
#include "FieldModelAnimation.h"

FieldModelAnimation::FieldModelAnimation() : _initialRot()
{
}

int FieldModelAnimation::commonRotationCount(const FieldModelAnimation &other) const
{
	if (other.frameCount() != frameCount()) {
		return -1;
	}
	
	int bones = 0, bonesStart = 0, bonesOtherStart = 0;
	
	if (other.boneCount() != boneCount()) {
		if (other.boneCount() + 1 == boneCount()) {
			bonesStart = 1;
			bones = other.boneCount();
		} else if (other.boneCount() == boneCount() + 1) {
			bonesOtherStart = 1;
			bones = boneCount();
		} else {
			return -1;
		}
	}
	
	int count = frameCount();
	int ret = 0;
	
	for (int frame = 0; frame < count; ++frame) {
		QList<PolyVertex> rotations = _framesRot.value(frame);

		for (int i = 0; i < bones; ++i) {
			PolyVertex left = rotations.at(bonesStart + i),
			    right = other.rotations(frame).at(bonesOtherStart + i);
			
			if (left.x == right.x && left.y == right.y && left.z == right.z && (left.x != 0 || left.y != 0 || left.z != 0)) {
				ret += 1;
			}
		}
	}
	
	return ret;
}


FieldModelAnimation FieldModelAnimation::toPC(bool *ok) const
{
	*ok = false;
	
	FieldModelAnimation ret;
	
	if (isEmpty()) {
		*ok = true;
		return ret;
	} 
	
	int count = boneCount() - 1;
	
	if (count <= 0) {
		return ret;
	}
	
	ret.setInitialRot(_framesRot[0].at(0));
	
	int frames = frameCount();
	
	for (int f = 0; f < frames; ++f) {
		QList<PolyVertex> rots = rotations(f);
		rots.removeFirst();
		ret.insertFrame(f, rots, QList<PolyVertex>() << translations(f).at(0));
	}
	
	*ok = true;
	
	return ret;
}

FieldModelAnimation FieldModelAnimation::toPS(bool *ok) const
{
	*ok = false;
	// TODO
	
	return *this;
}
