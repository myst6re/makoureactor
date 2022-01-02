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
#pragma once

#include <QtCore>
#include "FieldPS.h"

class FieldPSDemo : public FieldPS
{
public:
	FieldPSDemo(const QString &name, FieldArchiveIO *io);
	explicit FieldPSDemo(const Field &field);

protected:
	inline int headerSize() const override { return 0; }
	void openHeader(const QByteArray &fileData) override;
	FieldPart *createPart(FieldSection part) override;
	int sectionId(FieldSection part) const override;
	QString sectionFile(FieldSection part) const override;
	quint32 sectionPosition(int idPart) const override;
	inline int sectionCount() const override {	return 0; }
	inline int paddingBetweenSections() const override { return 0; }
	inline int alignment() const override { return 0; }
	QList<Field::FieldSection> orderOfSections() const override;
	inline qint32 diffSectionPos() const override { return 0; }
	inline bool hasSectionHeader() const override { return false; }
};
