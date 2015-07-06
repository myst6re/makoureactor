#ifndef FIELDMODELSKELETONIO_H
#define FIELDMODELSKELETONIO_H

#include "../IO.h"
#include "FieldModelSkeleton.h"

class FieldModelSkeletonIO : public IO
{
public:
	explicit FieldModelSkeletonIO(QIODevice *io);
	virtual ~FieldModelSkeletonIO() {}

	virtual bool read(FieldModelSkeleton &skeleton) const=0;
	virtual bool write(const FieldModelSkeleton &skeleton) const=0;
};

#endif // FIELDMODELSKELETONIO_H
