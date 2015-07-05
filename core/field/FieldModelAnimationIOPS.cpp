#include "FieldModelAnimationIOPS.h"

FieldModelAnimationIOPS::FieldModelAnimationIOPS(QIODevice *device) :
	FieldModelAnimationIO(device)
{
}

bool FieldModelAnimationIOPS::read(FieldModelAnimation &animation, int maxFrames) const
{
	if (!canRead()) {
		return false;
	}

	Animation a;
	if (sizeof(Animation) != device()->read((char *)&a, sizeof(Animation))) {
		qWarning() << "FieldModelAnimationIOPS::read invalid size animation 1" << device()->size();
		return false;
	}

	a.offsetData -= 0x80000000;

//	qDebug() << "==== ANIMATION" << animation_id << "====" << (curOff + sizeof(Animation)*animation_id);
//	qDebug() << "numFrames" << a.numFrames << "numBones" << a.numBones;
//	qDebug() << "numFramesTrans" << a.numFramesTranslation << "numFramesStaticTrans" << a.numStaticTranslation << "numFramesRot" << a.numFramesRotation;
//	qDebug() << "offsetFramesTrans" << (a.offsetData + a.offsetFramesTranslation) << "offsetFramesStaticTrans" << (a.offsetData + a.offsetStaticTranslation) << "offsetFramesRot" << (a.offsetData + a.offsetFramesRotation);
//	qDebug() << "offsetData" << a.offsetData;

	quint32 offsetFrameRotation = a.offsetData + a.offsetFramesRotation;
	quint32 offsetFrameStatic = a.offsetData + a.offsetStaticTranslation;
	quint32 offsetFrameTranslation = a.offsetData + a.offsetFramesTranslation;

	if (!device()->seek(a.offsetData + 4)) {
		qWarning() << "FieldModelAnimationIOPS::read invalid size animation 2" << device()->size();
		return false;
	}

//	qDebug() << "Unknown" << unknown << QString::number(unknown, 16);

	quint16 numFrames2 = maxFrames < 0 ? a.numFrames : qMin(quint16(maxFrames), a.numFrames);

	for (int frame = 0; frame < numFrames2; ++frame) {
		QList<PolyVertex> rotationCoords, rotationCoordsTrans;

		for (int bone = 0; bone < a.numBones; ++bone) {
			FrameTranslation frameTrans;
			PolyVertex rot, trans;
			char rotChar;

			if (!device()->seek(a.offsetData + 4 + bone * sizeof(FrameTranslation))) {
				qWarning() << "FieldModelAnimationIOPS::read invalid size animation 3" << device()->size();
				return false;
			}
			if (sizeof(FrameTranslation) != device()->read((char *)&frameTrans, sizeof(FrameTranslation))) {
				qWarning() << "FieldModelAnimationIOPS::read invalid size animation 4" << device()->size();
				return false;
			}

			// Rotation

			if (frameTrans.flag & 0x01) {
				quint32 offsetToRotation = offsetFrameRotation + frameTrans.rx * a.numFrames + frame;

				if (!device()->seek(offsetToRotation)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToRotation too large" << offsetToRotation << device()->size();
					return false;
				}
				if (!device()->getChar(&rotChar)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToRotation too large 2" << offsetToRotation << device()->size();
					return false;
				}

				rot.x = 360.0f * (quint8)rotChar / 255.0f;
			} else {
				rot.x = 360.0f * frameTrans.rx / 255.0f;
			}

			if (frameTrans.flag & 0x02) {
				quint32 offsetToRotation = offsetFrameRotation + frameTrans.ry * a.numFrames + frame;

				if (!device()->seek(offsetToRotation)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToRotation too large" << offsetToRotation << device()->size();
					return false;
				}
				if (!device()->getChar(&rotChar)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToRotation too large 2" << offsetToRotation << device()->size();
					return false;
				}

				rot.y = 360.0f * (quint8)rotChar / 255.0f;
			} else {
				rot.y = 360.0f * frameTrans.ry / 255.0f;
			}

			if (frameTrans.flag & 0x04) {
				quint32 offsetToRotation = offsetFrameRotation + frameTrans.rz * a.numFrames + frame;

				if (!device()->seek(offsetToRotation)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToRotation too large" << offsetToRotation << device()->size();
					return false;
				}
				if (!device()->getChar(&rotChar)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToRotation too large 2" << offsetToRotation << device()->size();
					return false;
				}

				rot.z = 360.0f * (quint8)rotChar / 255.0f;
			} else {
				rot.z = 360.0f * frameTrans.rz / 255.0f;
			}

			// (translation)

			qint16 translation=0;

			if (frameTrans.flag & 0x10) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.tx * a.numFrames * 2 + frame * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large" << offsetToTranslation << device()->size();
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large 2" << offsetToTranslation << device()->size();
					return false;
				}
			} else if (frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.tx * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large" << offsetToTranslation << device()->size();
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large 2" << offsetToTranslation << device()->size();
					return false;
				}
			}
			trans.x = -translation / MODEL_SCALE_PS;
			translation=0;

			if (frameTrans.flag & 0x20) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.ty * a.numFrames * 2 + frame * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large" << offsetToTranslation << device()->size();
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large 2" << offsetToTranslation << device()->size();
					return false;
				}
			} else if (frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.ty * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large" << offsetToTranslation << device()->size();
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large 2" << offsetToTranslation << device()->size();
					return false;
				}
			}
			trans.y = -translation / MODEL_SCALE_PS;
			translation = 0;

			if (frameTrans.flag & 0x40) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.tz * a.numFrames * 2 + frame * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large" << offsetToTranslation << device()->size();
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large 2" << offsetToTranslation << device()->size();
					return false;
				}
			} else if (frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.tz * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large" << offsetToTranslation << device()->size();
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "FieldModelAnimationIOPS::read OffsetToTranslation too large 2" << offsetToTranslation << device()->size();
					return false;
				}
			}
			trans.z = -translation / MODEL_SCALE_PS;

			rotationCoords.append(rot);
			rotationCoordsTrans.append(trans);
		}

		animation.insertFrame(frame, rotationCoords, rotationCoordsTrans);
	}

	return true;
}

bool FieldModelAnimationIOPS::write(const FieldModelAnimation &animation) const
{
	Q_UNUSED(animation)
	// TODO
	return false;
}
