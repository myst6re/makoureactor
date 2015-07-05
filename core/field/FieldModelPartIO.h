#ifndef FIELDMODELPARTIO_H
#define FIELDMODELPARTIO_H

#include "../IO.h"
#include "FieldModelPart.h"
#include "IdFile.h"

typedef struct {
	Vertex_s v;
	quint16 unknown;
} Vertex;

class FieldModelPartIO : public IO
{
public:
	explicit FieldModelPartIO(QIODevice *io);
	virtual ~FieldModelPartIO() {}

	virtual bool read(FieldModelPart *part) const=0;
	virtual bool write(const FieldModelPart *part) const=0;
};

#endif // FIELDMODELPARTIO_H
