#include "TutFilePC.h"

TutFilePC::TutFilePC() :
	TutFile()
{
}

QList<quint32> TutFilePC::openPositions(const QByteArray &data) const
{
	const char *constData = data.constData();
	QList<quint32> positions;
	quint32 dataSize = data.size();

	quint16 posTut;
	for(int i=0 ; i<9 ; ++i) {
		memcpy(&posTut, &constData[i*2], 2);
		if(posTut < 18)		posTut = 18;
		positions.append(posTut);
	}

	positions.append(dataSize);

	return positions;
}

QByteArray TutFilePC::save2(QByteArray &toc, quint32 firstPos) const
{
	Q_UNUSED(firstPos)

	quint32 pos;
	QByteArray ret;

	for(int i=0 ; i<9 ; ++i) {
		if(i < size()) {
			pos = 18 + ret.size();
			ret.append(data(i));
		} else {
			pos = 0xffff;
		}
		toc.append((char *)&pos, 2);
	}

	return ret;
}
