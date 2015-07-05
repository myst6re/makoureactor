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
	inline int bonesCount() const {
		return isEmpty() ? 0 : rotations(0).size();
	}
	inline bool isEmpty() const {
		return _framesRot.isEmpty();
	}
private:
	QHash<int, QList<PolyVertex> > _framesRot, _framesTrans;
};

#endif // FIELDMODELANIMATION_H
