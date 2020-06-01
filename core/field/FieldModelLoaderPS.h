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
#ifndef FIELDMODELLOADERPS_H
#define FIELDMODELLOADERPS_H

#include <QtCore>
#include "FieldModelLoader.h"
#include "FieldModelLoaderPC.h"
#include "BsxFile.h"

struct FieldModelLoaderStruct {
	quint8 faceID, bonesCount, partsCount, animationCount;
	quint8 unknown1, unknown2, unknown3, modelID;
};

class FieldModelLoaderPS : public FieldModelLoader
{
public:
	explicit FieldModelLoaderPS(Field *field);
	void clear();
	bool open();
	bool open(const QByteArray &data);
	QByteArray save() const;
	int modelCount() const;
	int animCount(int modelID) const;
	quint16 unknown(int modelID) const;
	void setUnknown(int modelID, quint16 unknown);
	const FieldModelLoaderStruct &model(int modelID) const;
	void setModel(int modelID, const FieldModelLoaderStruct &modelLoader);
	FieldModelLoaderPC toPC(BsxFile *bsx, bool *ok) const;
private:
	QList<FieldModelLoaderStruct> _modelLoaders;
};

#endif // FIELDMODELLOADERPS_H
