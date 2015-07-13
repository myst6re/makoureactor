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
#include "FieldModelPartPS.h"

FieldModelPartPS::FieldModelPartPS()
{
}

void FieldModelPartPS::addTexturedPolygon(quint8 control, Poly *polygon)
{
//	quint8 blend = (control >> 4) & 0x03;
	quint8 flagID = control & 0x0F;

	if(flagID < _groups.size()) {
		_groups.at(flagID+1)->addPolygon(polygon);
	} else {
		addPolygon(polygon);
	}
}

void FieldModelPartPS::addPolygon(Poly *polygon)
{
	_groups.first()->addPolygon(polygon);
}
