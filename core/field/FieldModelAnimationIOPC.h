#ifndef FIELDMODELANIMATIONIOPC_H
#define FIELDMODELANIMATIONIOPC_H

#include "FieldModelAnimationIO.h"
#include "FieldModelPartIOPC.h"

struct AHeader {
	quint32 version;
	quint32 framesCount;
	quint32 boneCount;
	quint8 rotationOrder[3];
	quint8 unused;
	quint32 runtimeData[5];
};

class FieldModelAnimationIOPC : public FieldModelAnimationIO
{
public:
	explicit FieldModelAnimationIOPC(QIODevice *device);
	virtual ~FieldModelAnimationIOPC() {}

	bool read(FieldModelAnimation &animation, int maxFrames = -1) const;
	bool write(const FieldModelAnimation &animation) const;
};

#endif // FIELDMODELANIMATIONIOPC_H
