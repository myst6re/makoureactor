#ifndef LZSSECTIONFILE_H
#define LZSSECTIONFILE_H

#include <QtCore>
#include "LzsIO.h"

class LzsSectionFile
{
public:
	LzsSectionFile();
	inline virtual ~LzsSectionFile() {}
	inline bool open(const char *data, int size) {
		return open(QByteArray(data, size));
	}
	bool open(const QByteArray &data);
	void saveStart();
	bool save(QByteArray &data);
	void saveEnd();
	void clear();
	QByteArray sectionData(quint8 id) const;
	void setSectionData(quint8 id, const QByteArray &data);
	virtual quint8 sectionCount() const=0;
protected:
	inline LzsRandomAccess *io() const {
		return &_io;
	}
	virtual bool openHeader()=0;
	virtual bool writePositions(QByteArray &data)=0;
	virtual void setSectionData(quint32 pos, quint32 oldSize,
								const QByteArray &section,
								QByteArray &out)=0;
	quint32 *_sectionPositions;
private:
	int sectionSize(quint8 id) const;
	inline int sectionPos(quint8 id) const {
		Q_ASSERT(id >= 0 && id < sectionCount());
		return _sectionPositions[id];
	}
	void shiftPositionsAfter(int id, int shift);

	LzsRandomAccess _io;
	QByteArray _data;
};

#endif // LZSSECTIONFILE_H
