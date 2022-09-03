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
#include "FieldPSDemo.h"
#include "BackgroundFilePS.h"

FieldPSDemo::FieldPSDemo(const QString &name, FieldArchiveIO *io) :
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
	switch (part) {
	case Scripts:     return "ATE";
	case Akaos:       return "ATE";
	case Walkmesh:    return "ID";
	case Background:  return "MAP";
	case Camera:      return "CA";
	case Inf:         return QString();
	case Encounter:   return QString();
	case ModelLoader: return QString();
	default:          return QString();
	}
}

quint32 FieldPSDemo::sectionPosition(int idPart) const
{
	Q_UNUSED(idPart)
	return 0;
}

FieldPart *FieldPSDemo::createPart(FieldSection part)
{
	switch (part) {
	case ModelLoader: return new FieldModelLoaderPS(this);
	case Background:  return new BackgroundFilePS(this);
	default:          return Field::createPart(part);
	}
}

QList<Field::FieldSection> FieldPSDemo::orderOfSections() const
{
	return QList<FieldSection>();
}
