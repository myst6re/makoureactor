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
#include "Data.h"
#include "Palette.h"

typedef struct {
	float size;
	int parent;
} Bone;

class FieldModelFile
{
public:
	FieldModelFile();
	virtual ~FieldModelFile();
	bool isLoaded() const;
	virtual void clear();
	virtual bool isPS() const=0;

	QString toStringBones() const;

	QMultiMap<int, FieldModelPart *> parts;
	QMultiMap<int, QList<int> > tex_files;
	QHash<int, QPixmap> loaded_tex;
	QList<Bone> bones;
	int a_bones_count;
	QHash<int, QList<PolyVertex> > frames;
protected:
	bool dataLoaded;
};

#endif // FIELDMODELFILE_H
