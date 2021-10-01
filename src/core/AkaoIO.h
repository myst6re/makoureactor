#pragma once

#include "core/IO.h"
#include "core/Akao.h"

class AkaoIO : public IO
{
public:
	explicit AkaoIO(QIODevice *device);
	virtual ~AkaoIO() {}
	bool read(Akao &akao) const;
	bool write(const Akao &akao) const;
};
