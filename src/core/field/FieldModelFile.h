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

#include <QtGui>
#include "FieldModelSkeleton.h"
#include "FieldModelAnimation.h"

class FieldModelFile
{
public:
	FieldModelFile();
	virtual ~FieldModelFile();

	virtual void clear();
	virtual bool isValid() const;
	virtual bool translateAfter() const=0;
	QByteArray signature() const; // To identify the model between PC and PS vesions
	int commonColorCount(const FieldModelFile &other) const;

	inline const FieldModelSkeleton &skeleton() const {
		return _skeleton;
	}
	inline void setSkeleton(const FieldModelSkeleton &skeleton) {
		_skeleton = skeleton;
	}
	inline const FieldModelBone &bone(int boneID) const {
		return _skeleton.bone(boneID);
	}
	inline qsizetype boneCount() const {
		return _skeleton.boneCount();
	}
	virtual const FieldModelAnimation &currentAnimation() const=0;
	virtual QImage loadedTexture(FieldModelGroup *group)=0;
	virtual void *textureIdForGroup(FieldModelGroup *group) const=0;
	virtual QHash<void *, QImage> loadedTextures()=0;
private:
	Q_DISABLE_COPY(FieldModelFile)
protected:
	FieldModelSkeleton _skeleton;
};
