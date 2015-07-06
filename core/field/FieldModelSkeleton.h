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
