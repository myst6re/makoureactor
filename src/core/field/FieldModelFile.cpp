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
#include "FieldModelFile.h"

FieldModelFile::FieldModelFile()
{
}

FieldModelFile::~FieldModelFile()
{
	for (const FieldModelBone &bone : _skeleton.bones()) {
		qDeleteAll(bone.parts());
	}
}

void FieldModelFile::clear()
{
	for (const FieldModelBone &bone : _skeleton.bones()) {
		qDeleteAll(bone.parts());
	}
	_skeleton.clear();
	_animations.clear();
}

bool FieldModelFile::isValid() const
{
	return _skeleton.boneCount() == 1 || (!_animations.isEmpty() && !_skeleton.isEmpty());
}
