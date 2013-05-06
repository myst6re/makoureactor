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
#include "FieldModelPart.h"

typedef struct {
	float size;
	int parent;
} Bone;

class FieldModelFile
{
public:
	FieldModelFile();
	virtual ~FieldModelFile();
	bool isOpen() const;
	virtual void clear();
	virtual bool translateAfter() const=0;

	const Bone &bone(int index) const;
	int boneCount() const;
	int animBoneCount() const; // valid bone count
	QList<FieldModelPart *> parts(int boneID) const;
	int loadedTextureCount() const;
	QImage loadedTexture(int texID) const;
	QList<PolyVertex> rotations(int frameID) const;
	QList<PolyVertex> translations(int frameID) const;
	int frameCount() const;
	QString toStringBones() const;
protected:
	QMultiMap<int, FieldModelPart *> _parts;
	QHash<int, QImage> _loaded_tex;
	QList<Bone> _bones;
	int a_bones_count;
	QHash<int, QList<PolyVertex> > _frames;
	QHash<int, QList<PolyVertex> > _framesTrans;
	bool dataLoaded;
};

#endif // FIELDMODELFILE_H
