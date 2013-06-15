/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef FIELDPART_H
#define FIELDPART_H

#include <QtCore>

class Field;

class FieldPart
{
public:
	FieldPart(Field *field);
	virtual ~FieldPart();

	virtual bool open()=0;
	virtual bool open(const QByteArray &data)=0;
	virtual QByteArray save() const=0;
	virtual bool canSave() const;
	virtual void close();
	virtual void clear()=0;

	bool isOpen() const;
	void setOpen(bool open);
	virtual bool isModified() const;
	void setModified(bool modified);
protected:
	Field *field() const;
private:
	bool modified, opened;
	Field *_field;
};

#endif // FIELDPART_H
