/****************************************************************************
 ** Makou Reactor Final Fantasy VII FieldPS Script Editor
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
#include "Field.h"
#include "FieldModelLoaderPS.h"
#include "FieldModelFilePS.h"
#include "FieldArchiveIOPS.h"

class FieldPS : public Field
{
public:
	FieldPS(const QString &name, FieldArchiveIO *io);
	explicit FieldPS(const Field &field);
	virtual ~FieldPS() override;

	inline bool isPC() const override { return false; }

	FieldModelLoaderPS *fieldModelLoader(bool open = true) override;
	FieldModelFilePS *fieldModel(int modelID, int animationID = 0, bool animate = true, bool open = true) override;
	FieldArchiveIOPS *io() const override;
	bool saveModels(QByteArray &newData, bool compress);
	qint8 saveModels(const QString &path, bool compress);
	bool saveBackground(QByteArray &newData, bool compress);
	qint8 saveBackground(const QString &path, bool compress);
	bool isDatModified() const;
	bool isBsxModified() const;
	bool isMimModified() const;
protected:
	inline virtual int headerSize() const override { return 28; }
	virtual void openHeader(const QByteArray &fileData) override;
	virtual QByteArray saveHeader() const override;
	virtual QByteArray saveFooter() const override;
	virtual FieldPart *createPart(FieldSection part) override;
	virtual int sectionId(FieldSection part) const override;
	inline QString sectionFile(FieldSection part) const override {
		Q_UNUSED(part)
		return "DAT";
	}
	virtual quint32 sectionPosition(int idPart) const override;
	inline virtual int sectionCount() const override {	return 7; }
	inline virtual int paddingBetweenSections() const override { return 0; }
	inline virtual int alignment() const override { return 4; } // Aligned
	virtual QList<Field::FieldSection> orderOfSections() const override;
	inline virtual qint32 diffSectionPos() const override { return vramDiff; }
	inline virtual bool hasSectionHeader() const override { return false; }
private:
	quint32 sectionPositions[7];
	qint32 vramDiff;
	QVector<FieldModelFilePS *> _models;
};
