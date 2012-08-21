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
#include <QRgb>
#include "FieldModelLoader.h"

typedef struct {
	quint8 faceID, bonesCount, partsCount, animationCount;
	quint8 unknown1, unknown2, unknown3, modelID;
} FieldModelLoaderStruct;

class FieldModelLoaderPS : public FieldModelLoader
{
public:
	FieldModelLoaderPS();
	bool load(const QByteArray &data);
	int modelCount() const;
	const FieldModelLoaderStruct &model(int id) const;
private:
	QList<FieldModelLoaderStruct> _modelLoaders;
};

#endif // FIELDMODELLOADERPS_H
