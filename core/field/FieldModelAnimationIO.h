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
#ifndef FIELDMODELANIMATIONIO_H
#define FIELDMODELANIMATIONIO_H

#include "../IO.h"
#include "FieldModelAnimation.h"

class FieldModelAnimationIO : public IO
{
public:
	explicit FieldModelAnimationIO(QIODevice *io);
	virtual ~FieldModelAnimationIO() {}

	virtual bool read(FieldModelAnimation &animation, int maxFrames = -1) const=0;
	virtual bool write(const FieldModelAnimation &animation) const=0;
};

#endif // FIELDMODELANIMATIONIO_H
