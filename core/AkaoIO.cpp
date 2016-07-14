#include "AkaoIO.h"

AkaoIO::AkaoIO(QIODevice *device) :
    IO(device)
{
	
}

bool AkaoIO::read(Akao &akao) const
{
	Q_UNUSED(akao)
	// TODO
	return false;
}

bool AkaoIO::write(const Akao &akao) const
{
	Q_UNUSED(akao)
	// TODO
	return false;
}
