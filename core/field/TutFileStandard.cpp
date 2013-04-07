#include "TutFileStandard.h"

TutFileStandard::TutFileStandard() :
	TutFile()
{
}

QList<quint32> TutFileStandard::openPositions(const QByteArray &data) const
{
	const char *constData = data.constData();
	QList<quint32> positions;
	quint32 dataSize = data.size();
	quint32 posAKAO, posAKAOList;
	quint16 nbAKAO;
	quint8 nbEntity;

	if(dataSize <= 8) {
		return positions;
	}

	nbEntity = constData[2];
	memcpy(&nbAKAO, &constData[6], 2);

	posAKAOList = 32+nbEntity*8;

	if(dataSize <= posAKAOList+nbAKAO*4) {
		return positions;
	}

	for(int i=0 ; i<nbAKAO ; ++i) {
		memcpy(&posAKAO, &constData[posAKAOList+i*4], 4);
		positions.append(posAKAO);
	}

	positions.append(dataSize);

	return positions;
}

QByteArray TutFileStandard::save2(QByteArray &toc, quint32 firstPos) const
{
	quint32 pos;
	QByteArray ret;

	foreach(const QByteArray &tuto, dataList()) {
		pos = firstPos + ret.size();
		toc.append((char *)&pos, 4);
		ret.append(tuto);
	}

	return ret;
}

bool TutFileStandard::hasTut() const
{
	int size = this->size();
	for(int i=0 ; i<size ; ++i) {
		if(isTut(i))	return true;
	}
	return false;
}

bool TutFileStandard::isTut(int tutID) const
{
	return !isAkao(tutID);
}

bool TutFileStandard::isAkao(int tutID) const
{
	return data(tutID).startsWith("AKAO");
}

QString TutFileStandard::parseScripts(int tutID) const
{
	QString ret;

	if(!isTut(tutID)) {
		const QByteArray &tuto = data(tutID);
		const char *constTuto = tuto.constData();

		quint16 id, length, firstPos;
		if(tuto.size() < 6) 	return QObject::tr("Erreur");
		memcpy(&id, constTuto + 4, 2);

		ret.append(QObject::tr("totalLength=%1\nid=%2\n").arg(tuto.size()).arg(id));

		if(tuto.size() < 8) 	return ret;
		memcpy(&length, constTuto + 6, 2);

		ret.append(QObject::tr("length=%1\n").arg(length));

		if(tuto.size() < 22) 	return ret;
		memcpy(&firstPos, constTuto + 20, 2);

		ret.append(QObject::tr("nbCanaux=%1\n").arg(firstPos/2 + 1));
		ret.append(tuto.mid(8, 8).toHex());
		ret.append("\n");
		ret.append(tuto.mid(16, 4).toHex());
		ret.append("\n");
	} else {
		ret = TutFile::parseScripts(tutID);
	}

	return ret;
}

int TutFileStandard::akaoID(int tutID) const
{
	if(isTut(tutID))	return -1;

	const QByteArray &data = this->data(tutID);
	if(data.size() < 6)		return -1;
	quint16 id;
	memcpy(&id, data.constData() + 4, 2);

	return id;
}

void TutFileStandard::setAkaoID(int tutID, quint16 akaoID)
{
	if(isTut(tutID))	return;

	dataRef(tutID).replace(4, 2, (char *)&akaoID, 2);
	setModified(true);
}
