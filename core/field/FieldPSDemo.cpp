#include "FieldPSDemo.h"
#include "BackgroundFilePS.h"

FieldPSDemo::FieldPSDemo(const QString &name, FieldArchiveIOPS *io) :
	FieldPS(name, io)
{
}

FieldPSDemo::FieldPSDemo(const Field &field) :
	FieldPS(field)
{
}

void FieldPSDemo::openHeader(const QByteArray &fileData)
{
	Q_UNUSED(fileData)
}

int FieldPSDemo::sectionId(FieldSection part) const
{
	Q_UNUSED(part)
	return -1;
}

QString FieldPSDemo::sectionFile(FieldSection part) const
{
	switch(part) {
	case Scripts:		return "ATE";
	case Akaos:			return "ATE";
	case Walkmesh:		return "ID";
	case Background:	return "MAP";
	case Camera:		return "CA";
	case Inf:			return QString();
	case Encounter:		return QString();
	case ModelLoader:	return QString();
	default:			return QString();
	}
}

quint32 FieldPSDemo::sectionPosition(int idPart) const
{
	Q_UNUSED(idPart)
	return 0;
}

FieldPart *FieldPSDemo::createPart(FieldSection part)
{
	switch(part) {
	case ModelLoader:	return new FieldModelLoaderPS(this);
	case Background:	return new BackgroundFilePS(this);
	default:			return Field::createPart(part);
	}
}

QList<Field::FieldSection> FieldPSDemo::orderOfSections() const
{
	return QList<FieldSection>();
}
