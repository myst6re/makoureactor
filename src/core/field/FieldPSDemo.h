/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#pragma once

#include <QtCore>
#include "FieldPS.h"

class FieldPSDemo : public FieldPS
{
public:
	FieldPSDemo(const QString &name, FieldArchiveIO *io);
	explicit FieldPSDemo(const Field &field);

protected:
	inline int headerSize() const { return 0; }
	void openHeader(const QByteArray &fileData);
	FieldPart *createPart(FieldSection part);
	int sectionId(FieldSection part) const;
	QString sectionFile(FieldSection part) const;
	quint32 sectionPosition(int idPart) const;
	inline int sectionCount() const {	return 0; }
	inline int paddingBetweenSections() const { return 0; }
	inline int alignment() const { return 0; }
	QList<Field::FieldSection> orderOfSections() const;
	inline quint32 diffSectionPos() const { return 0; }
	inline bool hasSectionHeader() const { return false; }
private:
};
