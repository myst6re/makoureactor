/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
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

void FieldPart::close()
{
	clear();
	setOpen(false);
}
