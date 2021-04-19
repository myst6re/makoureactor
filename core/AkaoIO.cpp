#include "AkaoIO.h"

struct AkaoSeqHeader
{
	uint16_t id;                    // song ID, used for playing sequence
	uint16_t data_size;             // data length - sizeof(AkaoSeqHeader) + sizeof(channel_mask)
	uint16_t reverb_type;           // reverb type (range from 0 to 9)
};

struct AkaoDrum
{
	quint8 instrument;
	quint8 key;
	quint16 volume;
	quint8 pan;
};

AkaoIO::AkaoIO(QIODevice *device) :
    IO(device)
{
	
}

bool AkaoIO::read(AkaoFile &akao)
{
	device()->seek(4); // AKAO
	AkaoSeqHeader header;
	if (device()->read((char*)&header, sizeof(AkaoSeqHeader)) != sizeof(AkaoSeqHeader)) {
		setError(QIODevice::tr("Cannot read akao header: %1").arg(device()->errorString()));
		return false;
	}

	akao.setSongId(header.id);
	akao.setReverbType(header.reverb_type);

	ByteArrayDate date = device()->read(6);

	if (date.size() != 6) {
		setError(QIODevice::tr("Cannot read akao header: %1"));
		return false;
	}

	akao.setDate(date);

	QByteArray remainingData = device()->read(header.data_size);

	if (int(sizeof(quint32) + 24 * sizeof(quint16)) >= remainingData.size()) {
		setError(QIODevice::tr("Not enough data to read akao data: %1 bytes").arg(remainingData.size()));
		return false;
	}

	const char *constData = remainingData.constData();

	quint32 channelMask = 0;
	memcpy(&channelMask, constData, sizeof(quint32));

	qDebug() << header.data_size << device()->size() << remainingData.size();

	QMap<quint16, quint8> positions;

	for (quint8 channel = 0; channel < 24; ++channel) {
		if ((channelMask >> channel) & 1) {
			size_t addr = sizeof(quint32) + channel * sizeof(quint16);
			quint16 pos = 0;

			memcpy(&pos, constData + addr, sizeof(quint16));

			pos += addr + sizeof(quint16);

			if (pos >= remainingData.size()) {
				setError(QIODevice::tr("Not enough data for akao position: %1/%2 bytes").arg(pos, remainingData.size()));
				return false;
			}

			positions.insert(pos, channel);
		}
	}

	qDebug() << positions;

	QMap<int, QList< QPair<quint8, int> > > drumPositions, jumpPositions;
	QMap<int, QPair<quint8, int> > instrPositions;
	int maxSize = remainingData.size();

	QMapIterator<quint16, quint8> it(positions);
	while (it.hasNext()) {
		it.next();
		quint16 pos = it.key();
		const quint8 channel = it.value();
		int nextPos = maxSize;
		QList<AkaoInstr> instrs;

		if (it.hasNext()) {
			nextPos = it.peekNext().key();
		}

		while (pos + 1 < nextPos) {
			const quint8 op = quint8(constData[pos]);
			quint8 opParamsLen = 0;

			if (op >= 0xA0) {
				opParamsLen = _opcodeParamsLen[op - 0xA0];
			}

			instrPositions.insert(pos, qMakePair(channel, instrs.size()));

			if (opParamsLen > 0) {
				if (pos + 1 + opParamsLen >= maxSize) {
					setError(QIODevice::tr("Not enough data to read akao instruction parameters: (%1 + %2)/%3").arg(pos + 1).arg(opParamsLen).arg(remainingData.size()));
					return false;
				}

				AkaoParams params = AkaoParams();

				memcpy(&params, constData + pos + 1, opParamsLen);

				AkaoInstr instr(op, params);

				if (op == quint16(AkaoInstr::TurnOnDrumMode)) {
					int drumPos = pos + 1 + int(sizeof(quint16)) + 1 + instr.params().param16.p1;

					if (drumPos >= remainingData.size()) {
						setError(QIODevice::tr("Not enough data for drum position: %1/%2 bytes").arg(drumPos, remainingData.size()));
						return false;
					}

					QList< QPair<quint8, int> > posList = drumPositions.value(drumPos);
					posList.append(qMakePair(channel, instrs.size()));
					drumPositions.insert(drumPos, posList);

					if (maxSize > drumPos) {
						maxSize = drumPos;
					}
				} else if (op == quint16(AkaoInstr::UnconditionalJump) || op == quint16(AkaoInstr::CpuConditionalJump)) {
					int jumpPos = pos + 1 + int(sizeof(quint16));

					if (op == quint16(AkaoInstr::UnconditionalJump)) {
						jumpPos += qint16(instr.params().param16.p1);
					} else {
						jumpPos += qint16(instr.params().param816.p2) + 1;
					}

					if (jumpPos >= maxSize) {
						setError(QIODevice::tr("Not enough data for jump position: %1/%2 bytes").arg(jumpPos, maxSize));
						return false;
					}

					QList< QPair<quint8, int> > posList = jumpPositions.value(jumpPos);
					posList.append(qMakePair(channel, instrs.size()));
					jumpPositions.insert(jumpPos, posList);
				}

				instrs.append(instr);
			} else {
				instrs.append(AkaoInstr(op));
			}

			pos += 1 + opParamsLen;
		}

		akao.setInstructions(channel, instrs);
	}

	QMapIterator<int, QList< QPair<quint8, int> > > itDrum(drumPositions);
	QList< QList<AkaoDrumInstr> > drumInstructions;

	while (itDrum.hasNext()) {
		itDrum.next();
		int drumPos = itDrum.key(),
		    nextPos = remainingData.size();
		const QList< QPair<quint8, int> > &scriptPos = itDrum.value();
		QList<AkaoDrumInstr> drumInstrs;

		if (itDrum.hasNext()) {
			nextPos = itDrum.peekNext().key();
		}

		const int size = nextPos - drumPos;

		if (size % 5 != 0) {
			qWarning() << QIODevice::tr("Drum data not aligned: %1 bytes").arg(size) << remainingData.mid(drumPos + (size / 5) * 5).toHex();
		}

		while (drumPos < size) {
			AkaoDrum drum;
			memcpy(&drum, constData + drumPos, 5); // Force size

			drumInstrs.append(AkaoDrumInstr(drum.instrument, drum.key, drum.volume, drum.pan));

			drumPos += 5;
		}

		// Update drum id in script
		for (const QPair<quint8, int> &sPos: scriptPos) {
			const quint8 channel = sPos.first;
			const int id = sPos.second;

			AkaoInstr instr = akao.instructions(channel).at(id);
			AkaoParams param = AkaoParams();
			param.param32 = quint32(drumInstructions.size());
			instr.setParams(param);
			akao.setInstruction(channel, id, instr);
		}

		drumInstructions.append(drumInstrs);
	}

	akao.setDrumInstructions(drumInstructions);

	QMapIterator<int, QList< QPair<quint8, int> > > itJump(jumpPositions);
	quint16 labelId = 1;

	while (itJump.hasNext()) {
		itJump.next();
		const QList< QPair<quint8, int> > &scriptPos = itJump.value();

		for (const QPair<quint8, int> &sPos: scriptPos) {
			const quint8 channel = sPos.first;
			const int id = sPos.second;

			AkaoInstr instr = akao.instructions(channel).at(id);
			AkaoParams param = instr.params();
			if (instr.op() == quint16(AkaoInstr::UnconditionalJump)) {
				param.param16.p1 = labelId;
			} else {
				param.param816.p2 = labelId;
			}
			instr.setParams(param);
			akao.setInstruction(channel, id, instr);
		}
		labelId += 1;
	}

	itJump.toBack();
	while (itJump.hasPrevious()) {
		itJump.previous();
		labelId -= 1;
		int jumpPos = itJump.key();
		if (!instrPositions.contains(jumpPos)) {
			qWarning() << "No pos for this goto" << jumpPos << itJump.value() << instrPositions;
			continue;
		}
		QPair<quint8, int> labelPos = instrPositions.value(jumpPos);
		const quint8 channel = labelPos.first;
		const int instrId = labelPos.second;

		QList<AkaoInstr> instructions = akao.instructions(channel);
		AkaoParams params = AkaoParams();
		params.param16.p1 = labelId;
		instructions.insert(instrId, AkaoInstr(AkaoInstr::Label, params));
		akao.setInstructions(channel, instructions);
	}

	return true;
}

bool AkaoIO::write(const AkaoFile &akao)
{
	Q_UNUSED(akao)
	// TODO
	return false;
}

quint8 AkaoIO::_opcodeParamsLen[0x60] = {
    0, 1, 1, 1, 2, 1, 0, 0,
    1, 2, 1, 2, 1, 1, 1, 1,

    2, 1, 1, 0, 3, 1, 0, 1,
    3, 1, 0, 1, 2, 1, 0, 1,

    1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1,

    0, 0, 1, 1, 0, 0, 0, 0,
    1, 1, 1, 0, 1, 2, 2, 2,

    0, 0, 0, 0, 0, 0, 0, 0,
    2, 3, 2, 3, 2, 0, 2, 3,

    3, 3, 1, 0, 2, 0, 1, 2,
    1, 0, 0, 0, 0, 2, 2, 0
};
