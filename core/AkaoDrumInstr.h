#ifndef AKAODRUMINSTR_H
#define AKAODRUMINSTR_H

#include <QtCore>

class AkaoDrumInstr
{
public:
	AkaoDrumInstr(quint8 instrument, quint8 key, quint16 volume, quint8 pan);
	inline quint8 instrument() const {
		return _instrument;
	}
	inline void setInstrument(quint8 instrument) {
		_instrument = instrument;
	}
	inline quint8 key() const {
		return _key;
	}
	inline void setKey(quint8 key) {
		_key = key;
	}
	inline quint16 volume() const {
		return _volume;
	}
	inline void setVolume(quint16 volume) {
		_volume = volume;
	}
	inline quint8 pan() const {
		return _pan;
	}
	inline void setPan(quint8 pan) {
		_pan = pan;
	}
private:
	quint8 _instrument, _key;
	quint16 _volume;
	quint8 _pan;
};

#endif // AKAODRUMINSTR_H
