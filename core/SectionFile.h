#ifndef SECTIONFILE_H
#define SECTIONFILE_H

#include <QtCore>
#include "LzsIO.h"

#define SECTION_FILE_MAX_POSITIONS 9

class SectionFile
{
public:
	SectionFile();
	virtual ~SectionFile();
	inline bool openLzs(const char *data, int size) {
		return openLzs(QByteArray(data, size));
	}
	bool openLzs(const QByteArray &data);
	inline bool openData(const char *data, int size) {
		return openData(QByteArray(data, size));
	}
	bool openData(const QByteArray &data);
	void saveStart();
	bool save(QByteArray &data, bool compressed);
	void saveEnd();
	void clear();
	virtual QByteArray sectionData(quint8 id);
	void setSectionData(quint8 id, const QByteArray &data);
	virtual quint32 sectionSize(quint8 id, bool &eof) const;
	virtual quint8 sectionCount() const=0;
protected:
	inline QIODevice *io() {
		return _io;
	}
	virtual bool openHeader()=0;
	virtual bool writePositions(QByteArray &data)=0;
	virtual int setSectionData(int pos, int oldSize,
							   const QByteArray &section,
							   QByteArray &out)=0;
	inline virtual quint32 sectionPos(quint8 id) const {
		return _sectionPositions[id];
	}
	quint32 _sectionPositions[SECTION_FILE_MAX_POSITIONS];
private:
	bool open();
	void shiftPositionsAfter(quint8 id, int shift);

	QIODevice *_io;
	QByteArray _data;
};

#endif // SECTIONFILE_H
