#ifndef FIELDMODELANIMATIONIO_H
#define FIELDMODELANIMATIONIO_H

#include "../IO.h"
#include "FieldModelAnimation.h"

class FieldModelAnimationIO : public IO
{
public:
	explicit FieldModelAnimationIO(QIODevice *io);
	virtual ~FieldModelAnimationIO() {}

	virtual bool read(FieldModelAnimation &animation, int maxFrames = -1) const=0;
	virtual bool write(const FieldModelAnimation &animation) const=0;
};

#endif // FIELDMODELANIMATIONIO_H
