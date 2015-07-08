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

FieldModelFile::FieldModelFile() :
	dataLoaded(false)
{
}

FieldModelFile::~FieldModelFile()
{
	dataLoaded = false;
	qDeleteAll(_parts);
}

void FieldModelFile::clear()
{
	dataLoaded = false;
	qDeleteAll(_parts);
	_parts.clear();
	_skeleton.clear();
	_animation.clear();
}

bool FieldModelFile::isOpen() const
{
	return dataLoaded;
}

const FieldModelBone &FieldModelFile::bone(int index) const
{
	return _skeleton.bone(index);
}

int FieldModelFile::boneCount() const
{
	return _skeleton.boneCount();
}

QList<FieldModelPart *> FieldModelFile::parts(int boneID) const
{
	return _parts.values(boneID);
}

int FieldModelFile::loadedTextureCount() const
{
	return _loadedTex.size();
}

QImage FieldModelFile::loadedTexture(int texID) const
{
	return _loadedTex.value(texID);
}

QList<PolyVertex> FieldModelFile::rotations(int frameID) const
{
	return _animation.rotations(frameID);
}

QList<PolyVertex> FieldModelFile::translations(int frameID) const
{
	return _animation.translations(frameID);
}

int FieldModelFile::frameCount() const
{
	return _animation.frameCount();
}
