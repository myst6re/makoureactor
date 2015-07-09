/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2015 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef FIELDMODELPARTIO_H
#define FIELDMODELPARTIO_H

#include "../IO.h"
#include "FieldModelPart.h"
#include "IdFile.h"

typedef struct {
	Vertex_s v;
	quint16 unknown;
} Vertex;

class FieldModelPartIO : public IO
{
public:
	explicit FieldModelPartIO(QIODevice *io);
	virtual ~FieldModelPartIO() {}

	virtual bool read(FieldModelPart *part) const=0;
	virtual bool write(const FieldModelPart *part) const=0;
};

#endif // FIELDMODELPARTIO_H
