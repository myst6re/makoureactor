#ifndef AKAOFILE_H
#define AKAOFILE_H

#include <QtCore>
#include "core/AkaoInstr.h"
#include "core/AkaoDrumInstr.h"

class ByteArrayDate : public QByteArray
{
public:
	inline ByteArrayDate() {}
	inline ByteArrayDate(const QByteArray &data) : QByteArray(data) {}
	QString toString() const;
};

class AkaoFile
{
public:
	AkaoFile();
	inline const ByteArrayDate &date() const {
		return _date;
	}
	inline void setDate(const ByteArrayDate &date) {
		_date = date;
	}
	inline bool isChannelUsed(quint8 channel) const {
		return _instructions.contains(channel);
	}
	inline const QList<AkaoInstr> instructions(quint8 channel) const {
		return _instructions.value(channel);
	}
	inline void setInstructions(quint8 channel, const QList<AkaoInstr> &instructions) {
		_instructions.insert(channel, instructions);
	}
	inline void setInstruction(quint8 channel, int id, const AkaoInstr &instruction) {
		_instructions[channel][id] = instruction;
	}
	inline const QList< QList<AkaoDrumInstr> > drumInstructions() const {
		return _drumInstructions;
	}
	inline void setDrumInstructions(const QList< QList<AkaoDrumInstr> > &drumInstructions) {
		_drumInstructions = drumInstructions;
	}
	inline quint16 songId() const {
		return _songId;
	}
	inline void setSongId(quint16 songId) {
		_songId = songId;
	}
	inline quint16 reverbType() const {
		return _reverbType;
	}
	inline void setReverbType(quint16 reverbType) {
		_reverbType = reverbType;
	}
	AkaoInstr *createInstruction(quint8 op, const char *data, int size) const;

private:
	quint16 _songId, _reverbType;
	ByteArrayDate _date;
	QMap<quint8, QList<AkaoInstr> > _instructions;
	QList< QList<AkaoDrumInstr> > _drumInstructions;
};

#endif // AKAOFILE_H
