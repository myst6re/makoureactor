#include "FieldModelSkeleton.h"

FieldModelSkeleton::FieldModelSkeleton()
{
}

FieldModelSkeleton::FieldModelSkeleton(const QList<FieldModelBone> &bones) :
	_bones(bones)
{
}

QString FieldModelSkeleton::toString() const
{
	QString ret;
	int boneID=0;

	foreach(const FieldModelBone &bone, _bones) {
		ret.append(QString("Bone %1: parent= %2 size= %3\n")
				   .arg(boneID)
				   .arg(bone.parent)
				   .arg(bone.size));
		++boneID;
	}

	return ret;
}
