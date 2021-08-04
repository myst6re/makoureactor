/****************************************************************************
 ** Makou Reactor Final Fantasy VII FieldPS Script Editor
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
#ifndef DEF_FIELDPS
#define DEF_FIELDPS

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
	virtual ~FieldPS();

	inline bool isPC() const { return false; }

	FieldModelLoaderPS *fieldModelLoader(bool open=true);
	FieldModelFilePS *fieldModel(int modelID, int animationID = 0, bool animate = true, bool open = true);
	FieldArchiveIOPS *io() const;
	bool saveModels(QByteArray &newData);
	bool saveBackground(QByteArray &newData);
	bool isDatModified() const;
	bool isBsxModified() const;
	bool isMimModified() const;
protected:
	inline virtual int headerSize() const { return 28; }
	virtual void openHeader(const QByteArray &fileData);
	virtual QByteArray saveHeader() const;
	virtual QByteArray saveFooter() const;
	virtual FieldPart *createPart(FieldSection part);
	virtual int sectionId(FieldSection part) const;
	inline QString sectionFile(FieldSection part) const {
		Q_UNUSED(part)
		return "DAT";
	}
	virtual quint32 sectionPosition(int idPart) const;
	inline virtual int sectionCount() const {	return 7; }
	inline virtual int paddingBetweenSections() const { return 0; }
	inline virtual int alignment() const { return 4; } // Aligned
	virtual QList<Field::FieldSection> orderOfSections() const;
	inline virtual quint32 diffSectionPos() const { return vramDiff; }
	inline virtual bool hasSectionHeader() const { return false; }
private:
	quint32 sectionPositions[7];
	qint32 vramDiff;
	QVector<FieldModelFilePS *> _models;
};

#endif // DEF_FIELDPS
