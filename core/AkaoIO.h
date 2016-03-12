#ifndef AKAOIO_H
#define AKAOIO_H

#include "core/IO.h"
#include "core/Akao.h"

class AkaoIO : public IO
{
public:
	AkaoIO();
	bool read(Akao &akao) const;
	bool write(const Akao &akao) const;
};

#endif // AKAOIO_H
