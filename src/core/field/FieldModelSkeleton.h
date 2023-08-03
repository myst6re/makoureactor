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
#include "FieldModelPart.h"

class FieldModelBone
{
public:
	FieldModelBone(float size, int parent,
	               const QList<FieldModelPart *> &parts = QList<FieldModelPart *>());

	inline float size() const {
		return _size;
	}

	inline void setSize(float size) {
		_size = size;
	}

	inline int parent() const {
		return _parent;
	}

	inline void setParent(int parent) {
		_parent = parent;
	}

	inline const QList<FieldModelPart *> &parts() const {
		return _parts;
	}

	inline void setParts(const QList<FieldModelPart *> &parts) {
		_parts = parts;
	}

	inline void addPart(FieldModelPart *part) {
		_parts.append(part);
	}

	QImage toImage(int width, int height) const;
	int commonColorCount(const FieldModelBone &other) const;
private:
	float _size;
	int _parent;
	QList<FieldModelPart *> _parts;
};

class FieldModelSkeleton
{
public:
	FieldModelSkeleton();
	explicit FieldModelSkeleton(const QList<FieldModelBone> &bones);

	inline const QList<FieldModelBone> &bones() const {
		return _bones;
	}
	inline const FieldModelBone &bone(int index) const {
		return _bones.at(index);
	}
	inline void setBones(const QList<FieldModelBone> &bones) {
		_bones = bones;
	}
	inline void addBone(const FieldModelBone &bone) {
		_bones.append(bone);
	}
	inline void clear() {
		_bones.clear();
	}
	inline qsizetype boneCount() const {
		return _bones.size();
	}
	inline bool isEmpty() const {
		return _bones.isEmpty();
	}
	inline const FieldModelBone &operator[](int i) const {
		return _bones[i];
	}
	inline FieldModelBone &operator[](int i) {
		return _bones[i];
	}
	QString toString() const;
	QImage toImage(int width, int height) const;
private:
	QList<FieldModelBone> _bones;
};
