#ifndef FIELDMODELANIMATIONIOPS_H
#define FIELDMODELANIMATIONIOPS_H

#include "FieldModelAnimationIO.h"
#include "FieldModelPartIOPS.h"

struct Animation {
	quint16 numFrames;                  // Number of frames
	quint8 numBones;                    // Number of bones
	quint8 numFramesTranslation;        // Number of translation frames
	quint8 numStaticTranslation;        // Number of static translation frames
	quint8 numFramesRotation;           // Number of rotation frames
	quint16 offsetFramesTranslation;    // Relative offset to translation frames
	quint16 offsetStaticTranslation;    // Relative offset to statis translation frames
	quint16 offsetFramesRotation;       // Relative offset to rotation frames
	quint32 offsetData;                 // Offset to animation data section
};

struct FrameTranslation {
	quint8 flag;
	quint8 rx, ry, rz;
	quint8 tx, ty, tz;
	quint8 padding;
};

class FieldModelAnimationIOPS : public FieldModelAnimationIO
{
public:
	explicit FieldModelAnimationIOPS(QIODevice *device);
	virtual ~FieldModelAnimationIOPS() {}

	bool read(FieldModelAnimation &animation, int maxFrames = -1) const;
	bool write(const FieldModelAnimation &animation) const;
};

#endif // FIELDMODELANIMATIONIOPS_H
