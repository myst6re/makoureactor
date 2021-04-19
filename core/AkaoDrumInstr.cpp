#include "AkaoDrumInstr.h"

AkaoDrumInstr::AkaoDrumInstr(quint8 instrument, quint8 key, quint16 volume, quint8 pan) :
    _instrument(instrument),
    _key(key),
    _volume(volume),
    _pan(pan)
{
}
