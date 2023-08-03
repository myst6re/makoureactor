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
#include "FieldModelLoader.h"
#include "FieldModelLoaderPC.h"
#include "BsxFile.h"

class CharArchive;

struct FieldModelLoaderStruct {
	quint8 faceID, bonesCount, partsCount, animationCount;
	quint8 unknown1, unknown2, unknown3, modelID;
};

class FieldModelLoaderPS : public FieldModelLoader
{
public:
	explicit FieldModelLoaderPS(Field *field);
	void clear() override;
	bool open() override;
	bool open(const QByteArray &data) override;
	QByteArray save() const override;
	qsizetype modelCount() const override;
	qsizetype animCount(int modelID) const override;
	quint16 unknown(int modelID) const override;
	void setUnknown(int modelID, quint16 unknown) override;
	const FieldModelLoaderStruct &model(int modelID) const;
	void setModel(int modelID, const FieldModelLoaderStruct &modelLoader);
	FieldModelLoaderPC toPC(BsxFile *bsx, CharArchive *charArchive, bool *ok) const;
private:
	QList<FieldModelLoaderStruct> _modelLoaders;
};
