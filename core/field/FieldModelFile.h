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
#ifndef FIELDMODELFILE_H
#define FIELDMODELFILE_H

#include <QtGui>
#include "FieldModelSkeleton.h"
#include "FieldModelPart.h"
#include "FieldModelAnimation.h"

class FieldModelFile
{
public:
	FieldModelFile();
	virtual ~FieldModelFile();

	inline bool isEmpty() const {
		return _skeleton.isEmpty();
	}
	virtual void clear();
	virtual bool translateAfter() const=0;

	inline const FieldModelSkeleton &skeleton() const {
		return _skeleton;
	}
	inline void setSkeleton(const FieldModelSkeleton &skeleton) {
		_skeleton = skeleton;
	}
	inline const FieldModelBone &bone(int boneID) const {
		return _skeleton.bone(boneID);
	}
	inline int boneCount() const {
		return _skeleton.boneCount();
	}
	inline const QList<FieldModelAnimation> &animations() const {
		return _animations;
	}
	inline void setAnimations(const QList<FieldModelAnimation> &animations) {
		_animations = animations;
	}
	inline int animationCount() const {
		return _animations.size();
	}
	inline const FieldModelAnimation &animation(int animationID) const {
		return _animations.at(animationID);
	}
	virtual QImage loadedTexture(FieldModelGroup *group)=0;
private:
	Q_DISABLE_COPY(FieldModelFile)
protected:
	FieldModelSkeleton _skeleton;
	QList<FieldModelAnimation> _animations;
};

#endif // FIELDMODELFILE_H
