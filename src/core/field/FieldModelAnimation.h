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
	const PolyVertex &initialRot() const {
		return _initialRot;
	}
	void setInitialRot(const PolyVertex &rot) {
		_initialRot = rot;
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
	inline qsizetype frameCount() const {
		return _framesRot.size();
	}
	inline qsizetype boneCount() const {
		return isEmpty() ? 0 : rotations(0).size();
	}
	inline bool isEmpty() const {
		return _framesRot.isEmpty();
	}
	int commonRotationCount(const FieldModelAnimation &other) const;
	FieldModelAnimation toPC(bool *ok) const;
	FieldModelAnimation toPS(bool *ok) const;
private:
	PolyVertex _initialRot;
	QHash<int, QList<PolyVertex> > _framesRot, _framesTrans;
};
