#ifndef FIELDMODELPARTIOPS_H
#define FIELDMODELPARTIOPS_H

#include "FieldModelPartIO.h"
#include "FieldModelPartPS.h"

class FieldModelPartIOPS : public FieldModelPartIO
{
public:
	explicit FieldModelPartIOPS(QIODevice *io);
	virtual ~FieldModelPartIOPS() {}

	inline bool read(FieldModelPart *part) const {
		return read((FieldModelPartPS *)part);
	}
	inline bool write(const FieldModelPart *part) const {
		return write((FieldModelPartPS *)part);
	}
	bool read(FieldModelPartPS *part) const;
	bool write(const FieldModelPartPS *part) const;
};

#endif // FIELDMODELPARTIOPS_H
