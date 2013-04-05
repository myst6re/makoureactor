#ifndef FIELDARCHIVEPS_H
#define FIELDARCHIVEPS_H

#include "FieldArchive.h"

class FieldArchivePS : public FieldArchive
{
public:
	FieldArchivePS();
	explicit FieldArchivePS(const QString &path, bool isDirectory=false);
	inline bool isPC() const { return false; }

	FieldArchiveIO *io() const;
};

#endif // FIELDARCHIVEPS_H
