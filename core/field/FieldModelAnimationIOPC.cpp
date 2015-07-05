#include "FieldModelAnimationIOPC.h"

FieldModelAnimationIOPC::FieldModelAnimationIOPC(QIODevice *device) :
	FieldModelAnimationIO(device)
{
}

bool FieldModelAnimationIOPC::read(FieldModelAnimation &animation, int maxFrames) const
{
	if (!canRead()) {
		return false;
	}

	AHeader header;
	PolyVertex rot, trans;

	if (device()->read((char *)&header, 36) != 36
			|| header.framesCount == 0
			|| device()->pos() + header.framesCount * (24 + 12 * header.bonesCount) > device()->size()) {
		return false;
	}

//	qDebug() << header.bones_count << _bones.size();
//	qDebug() << header.frames_count << device()->size()-device()->pos();

	if (maxFrames >= 0) {
		header.framesCount = qMin(header.framesCount, quint32(maxFrames));
	}

	for (quint32 i = 0; i < header.framesCount; ++i) {
		if (!device()->seek(device()->pos() + 12)) {
			return false;
		}
		if (device()->read((char *)&trans, 12) != 12) {
			return false;
		}

		trans.x = trans.x / MODEL_SCALE_PC;
		trans.y = trans.y / MODEL_SCALE_PC;
		trans.z = trans.z / MODEL_SCALE_PC;

		QList<PolyVertex> rotationCoords;

		for (quint32 j = 0; j < header.bonesCount; ++j) {
			if(device()->read((char *)&rot, 12) != 12) {
				return false;
			}
			rotationCoords.append(rot);
		}
		animation.insertFrame(i, rotationCoords, QList<PolyVertex>() << trans);
	}

	return true;
}

bool FieldModelAnimationIOPC::write(const FieldModelAnimation &animation) const
{
	Q_UNUSED(animation)
	// TODO
	return false;
}

