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
#include "FieldModelSkeleton.h"

FieldModelBone::FieldModelBone(float size, int parent,
							   const QList<FieldModelPart *> &parts) :
	_size(size), _parent(parent), _parts(parts)
{
}

FieldModelSkeleton::FieldModelSkeleton()
{
}

FieldModelSkeleton::FieldModelSkeleton(const QList<FieldModelBone> &bones) :
	_bones(bones)
{
}

QString FieldModelSkeleton::toString() const
{
	QString ret;
	int boneID=0;

	foreach(const FieldModelBone &bone, _bones) {
		ret.append(QString("Bone %1: parent= %2 size= %3\n")
				   .arg(boneID)
				   .arg(bone.parent())
				   .arg(bone.size()));
		++boneID;
	}

	return ret;
}
