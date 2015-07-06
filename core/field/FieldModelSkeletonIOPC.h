#ifndef FIELDMODELSKELETONIOPC_H
#define FIELDMODELSKELETONIOPC_H

#include "FieldModelSkeletonIO.h"

class FieldModelSkeletonIOPC : public FieldModelSkeletonIO
{
public:
	explicit FieldModelSkeletonIOPC(QIODevice *io);
	virtual ~FieldModelSkeletonIOPC() {}

	bool read(FieldModelSkeleton &skeleton) const {
		QMultiMap<int, QStringList> rsdFiles;
		return read(skeleton, rsdFiles);
	}
	bool read(FieldModelSkeleton &skeleton, QMultiMap<int, QStringList> &rsdFiles) const;
	bool write(const FieldModelSkeleton &skeleton) const;
};

#endif // FIELDMODELSKELETONIOPC_H
