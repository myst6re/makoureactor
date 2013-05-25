#ifndef TUTFILESTANDARD_H
#define TUTFILESTANDARD_H

#include <QtCore>
#include "TutFile.h"

class Field;

class TutFileStandard : public TutFile
{
public:
	explicit TutFileStandard(Field *field);
	bool open();
	QByteArray save(QByteArray &toc, quint32 firstPos) const;
	QByteArray save() const;
	bool hasTut() const;
	bool isTut(int tutID) const;
	bool isAkao(int tutID) const;
	int akaoID(int tutID) const;
	void setAkaoID(int tutID, quint16 akaoID);
	QString parseScripts(int tutID) const;
protected:
	QList<quint32> openPositions(const QByteArray &data) const;
	QByteArray save2(QByteArray &toc, quint32 firstPos) const;
	inline int maxTutCount() const { return 255; }
};

#endif // TUTFILESTANDARD_H
