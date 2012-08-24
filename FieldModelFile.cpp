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
	a_bones_count(0), dataLoaded(false)
{
}

FieldModelFile::~FieldModelFile()
{
	foreach(FieldModelPart *part, _parts)
		delete part;
}

void FieldModelFile::clear()
{
	dataLoaded = false;
	foreach(FieldModelPart *part, _parts)
		delete part;
	_parts.clear();
	_bones.clear();
	_frames.clear();
}

bool FieldModelFile::isLoaded() const
{
	return dataLoaded;
}

const Bone &FieldModelFile::bone(int index) const
{
	return _bones.at(index);
}

int FieldModelFile::boneCount() const
{
	return _bones.size();
}

int FieldModelFile::animBoneCount() const
{
	return a_bones_count;
}

QList<FieldModelPart *> FieldModelFile::parts(int boneID) const
{
	return _parts.values(boneID);
}

int FieldModelFile::loadedTextureCount() const
{
	return _loaded_tex.size();
}

QPixmap FieldModelFile::loadedTexture(int texID) const
{
	return _loaded_tex.value(texID);
}

QList<PolyVertex> FieldModelFile::rotation(int frameID) const
{
	return _frames.value(frameID);
}

int FieldModelFile::frameCount() const
{
	return _frames.size();
}

QString FieldModelFile::toStringBones() const
{
	QString ret;
	int boneID=0;

	foreach(const Bone &bone, _bones) {
		ret.append(QString("Bone %1: parent= %2 size= %3\n")
				   .arg(boneID)
				   .arg(bone.parent)
				   .arg(bone.size));
		++boneID;
	}

	return ret;
}
