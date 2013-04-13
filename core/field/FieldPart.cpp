#include "FieldPart.h"

FieldPart::FieldPart(Field *field) :
	modified(false), opened(false), _field(field)
{
}

FieldPart::~FieldPart()
{
}

bool FieldPart::isOpen() const
{
	return opened;
}

bool FieldPart::isModified() const
{
	return modified;
}

void FieldPart::setModified(bool modified)
{
	this->modified = modified;
}

void FieldPart::setOpen(bool open)
{
	opened = open;
}

Field *FieldPart::field() const
{
	return _field;
}

bool FieldPart::canSave() const
{
	return true;
}
