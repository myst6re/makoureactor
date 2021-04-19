#ifndef AKAOIO_H
#define AKAOIO_H

#include "core/IO.h"
#include "core/AkaoFile.h"

class AkaoIO : public IO
{
public:
	explicit AkaoIO(QIODevice *device);
	virtual ~AkaoIO() {}
	bool read(AkaoFile &akao);
	bool write(const AkaoFile &akao);
private:
	static quint8 _opcodeParamsLen[0x60];
};

#endif // AKAOIO_H
