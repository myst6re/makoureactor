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
#ifndef FIELDMODELPARTPC_H
#define FIELDMODELPARTPC_H

#include <QImage>
#include "FieldModelPart.h"

class FieldModelTextureRefPC : public FieldModelTextureRef
{
public:
	FieldModelTextureRefPC(quint32 id) : _id(id) {}
	virtual ~FieldModelTextureRefPC() {}

	inline quint32 id() const {
		return _id;
	}

	inline void setId(quint32 id) {
		_id = id;
	}

	inline quint64 textureIdentifier() const {
		return id();
	}
private:
	quint32 _id;
};

#endif // FIELDMODELPARTPC_H
