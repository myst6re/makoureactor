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
#ifndef FIELDMODELSKELETON_H
#define FIELDMODELSKELETON_H

#include <QtGui>

struct FieldModelBone {
	float size;
	int parent;
};

class FieldModelSkeleton
{
public:
	FieldModelSkeleton();
	FieldModelSkeleton(const QList<FieldModelBone> &bones);

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
	inline int boneCount() const {
		return _bones.size();
	}
	inline bool isEmpty() const {
		return _bones.isEmpty();
	}
	QString toString() const;
private:
	QList<FieldModelBone> _bones;
};

#endif // FIELDMODELSKELETON_H
