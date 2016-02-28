#ifndef FIELDPSDEMO_H
#define FIELDPSDEMO_H

#include <QtCore>
#include "FieldPS.h"

class FieldPSDemo : public FieldPS
{
public:
	FieldPSDemo(const QString &name, FieldArchiveIO *io);
	explicit FieldPSDemo(const Field &field);

protected:
	inline int headerSize() const { return 0; }
	void openHeader(const QByteArray &fileData);
	FieldPart *createPart(FieldSection part);
	int sectionId(FieldSection part) const;
	QString sectionFile(FieldSection part) const;
	quint32 sectionPosition(int idPart) const;
	inline int sectionCount() const {	return 0; }
	inline int paddingBetweenSections() const { return 0; }
	inline int alignment() const { return 0; }
	QList<Field::FieldSection> orderOfSections() const;
	inline quint32 diffSectionPos() const { return 0; }
	inline bool hasSectionHeader() const { return false; }
private:
};

#endif // FIELDPSDEMO_H
