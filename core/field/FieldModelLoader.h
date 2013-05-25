/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef FIELDMODELLOADER_H
#define FIELDMODELLOADER_H

#include <QtCore>
#include "FieldPart.h"

class FieldModelLoader : public FieldPart
{
public:
	explicit FieldModelLoader(Field *field);
	virtual ~FieldModelLoader();
	virtual int modelCount() const=0;
	virtual int animCount(int modelID) const=0;
	virtual quint16 unknown(int modelID) const=0;
	virtual void setUnknown(int modelID, quint16 unknown)=0;
	/* Even the file format can store more than 255 models,
	 * it's useless since the scripts can't deal with it. */
	inline int maxModelCount() const { return 255; }
	inline int maxAnimCount() const { return 255; }
};

#endif // FIELDMODELLOADER_H
