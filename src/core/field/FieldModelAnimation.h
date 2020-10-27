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
#ifndef FIELDMODELANIMATION_H
#define FIELDMODELANIMATION_H

#include <QtCore>
#include "FieldModelPart.h"

class FieldModelAnimation
{
public:
	FieldModelAnimation();

	inline QList<PolyVertex> rotations(int frame) const {
		return _framesRot.value(frame);
	}
	inline QList<PolyVertex> translations(int frame) const {
		return _framesTrans.value(frame);
	}
	inline void insertFrame(int frame, const QList<PolyVertex> &rotations,
							const QList<PolyVertex> &translations) {
		_framesRot.insert(frame, rotations);
		_framesTrans.insert(frame, translations);
	}
	inline void clear() {
		_framesRot.clear();
		_framesTrans.clear();
	}
	inline int frameCount() const {
		return _framesRot.size();
	}
	inline int boneCount() const {
		return isEmpty() ? 0 : rotations(0).size();
	}
	inline bool isEmpty() const {
		return _framesRot.isEmpty();
	}
private:
	QHash<int, QList<PolyVertex> > _framesRot, _framesTrans;
};

#endif // FIELDMODELANIMATION_H
