#ifndef FIELDPSDEMO_H
#define FIELDPSDEMO_H

#include <QtCore>
#include "FieldPS.h"

class FieldPSDemo : public FieldPS
{
public:
	FieldPSDemo(const QString &name, FieldArchiveIOPS *io);
	explicit FieldPSDemo(const Field &field);
	virtual ~FieldPSDemo();

	QByteArray sectionData(CommonSection part);
	bool setSectionData(CommonSection section, const QByteArray &data);
protected:
	virtual bool open2();
	virtual bool save2(QByteArray &data, bool compress);
	virtual void saveStart();
	virtual void saveEnd();
private:
	LzsRandomAccess *openFileFromIO(const QString &extension);
	static QByteArray fileData(LzsRandomAccess *io);

	LzsRandomAccess *_dataAte, *_dataMap, *_dataId, *_dataCa;
	QByteArray _saveAte, _saveId, _saveCa;
};

#endif // FIELDPSDEMO_H
