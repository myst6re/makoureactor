#ifndef TUTFILEPC_H
#define TUTFILEPC_H

#include <QtCore>
#include "TutFile.h"

class TutFilePC : public TutFile
{
public:
	TutFilePC();
protected:
	QList<quint32> openPositions(const QByteArray &data) const;
	QByteArray save2(QByteArray &toc, quint32 firstPos) const;
	inline int maxTutCount() const { return 9; }
};

#endif // TUTFILEPC_H
