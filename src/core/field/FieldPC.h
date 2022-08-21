/****************************************************************************
 ** Makou Reactor Final Fantasy VII FieldPC Script Editor
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
#include "Field.h"
#include "FieldModelLoaderPC.h"
#include "FieldModelFilePC.h"

class FieldPC : public Field
{
public:
	FieldPC(const QString &name, FieldArchiveIO *io);
	FieldPC(const QString &name);
	explicit FieldPC(const Field &field);
	virtual ~FieldPC() override;

	inline bool isPC() const override { return true; }

	FieldModelLoaderPC *fieldModelLoader(bool open = true) override;
	FieldModelFilePC *fieldModel(int modelID, int animationID = 0, bool animate = true, bool open = true) override;
	FieldModelFilePC *fieldModel(const QString &hrc, const QString &a, bool animate = true);
	bool exportToChunks(const QDir &dir) override;
protected:
	inline int headerSize() const override { return 42; }
	void openHeader(const QByteArray &fileData) override;
	QByteArray saveHeader() const override;
	QByteArray saveFooter() const override;
	FieldPart *createPart(FieldSection part) override;
	int sectionId(FieldSection part) const override;
	inline QString sectionFile(FieldSection part) const override {
		Q_UNUSED(part)
		return QString();
	}
	quint32 sectionPosition(int idPart) const override;
	inline int sectionCount() const override {	return 9; }
	inline int paddingBetweenSections() const override { return 4; }
	inline int alignment() const override { return 0; } // Not aligned
	QList<FieldSection> orderOfSections() const override;
	inline qint32 diffSectionPos() const override { return 0; }
	inline bool hasSectionHeader() const override { return true; }
	bool importModelLoader(const QByteArray &sectionData, bool isPSField, QIODevice *bsxDevice) override;
private:
	quint32 sectionPositions[9];
	FieldModelFilePC *_model;
	QMap<QString, int> modelNameToId;
};
