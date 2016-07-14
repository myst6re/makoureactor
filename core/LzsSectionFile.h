#ifndef LZSSECTIONFILE_H
#define LZSSECTIONFILE_H

#include <QtCore>
#include "LzsIO.h"

class LzsSectionFile
{
public:
	LzsSectionFile();
	virtual ~LzsSectionFile();
	inline bool open(const char *data, int size) {
		return open(QByteArray(data, size));
	}
	bool open(const QByteArray &data);
	void saveStart();
	bool save(QByteArray &data);
	void saveEnd();
	void clear();
	virtual QByteArray sectionData(quint8 id);
	void setSectionData(quint8 id, const QByteArray &data);
	quint32 sectionSize(quint8 id, bool &eof) const;
	virtual quint8 sectionCount() const=0;
protected:
	inline LzsRandomAccess *io() {
		return _io;
	}
	virtual bool openHeader()=0;
	virtual bool writePositions(QByteArray &data)=0;
	virtual int setSectionData(int pos, int oldSize,
							   const QByteArray &section,
							   QByteArray &out)=0;
	inline virtual quint32 sectionPos(quint8 id) const {
		Q_ASSERT(id >= 0 && id < sectionCount());
		return _sectionPositions[id];
	}
	quint32 *_sectionPositions;
private:
	void shiftPositionsAfter(quint8 id, int shift);

	LzsRandomAccess *_io;
	QByteArray _data;
};

#endif // LZSSECTIONFILE_H
