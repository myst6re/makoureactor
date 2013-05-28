#ifndef TUTFILEPC_H
#define TUTFILEPC_H

#include <QtCore>
#include "TutFile.h"

class TutFilePC : public TutFile
{
public:
	TutFilePC();
	bool open();
	QByteArray save() const;
	inline int maxTutCount() const { return 9; }
protected:
	QList<quint32> openPositions(const QByteArray &data) const;
};

#endif // TUTFILEPC_H
