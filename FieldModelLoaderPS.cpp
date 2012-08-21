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
#include "FieldModelLoaderPS.h"

FieldModelLoaderPS::FieldModelLoaderPS() :
	FieldModelLoader()
{
}

bool FieldModelLoaderPS::load(const QByteArray &data)
{
	const char *constData = data.constData();
	quint32 posSection1, posSectionModelLoader;

	memcpy(&posSection1, constData, 4);
	memcpy(&posSectionModelLoader, &constData[24], 4);// section 7
	// convert vram position to relative position
	posSectionModelLoader = posSectionModelLoader - posSection1 + 28;

	if(posSectionModelLoader >= (quint32)data.size()) {
		qWarning() << "empty model loader!";
		return false;
	}

	quint16 size, modelCount;
	memcpy(&size, &constData[posSectionModelLoader], 2);
	memcpy(&modelCount, &constData[posSectionModelLoader+2], 2);

	if(size != data.size() - posSectionModelLoader || (size-4) / 8 != modelCount || (size-4) % 8 != 0) {
		qWarning() << "invalid model loader size" << size << modelCount << (data.size() - posSectionModelLoader);
		return false;
	}

	if(sizeof(FieldModelLoaderStruct) != 8) {
		qWarning() << "invalid model loader struct size" << sizeof(FieldModelLoaderStruct);
		return false;
	}

	FieldModelLoaderStruct modelLoader;

	for(quint32 i=0 ; i<modelCount ; ++i) {
		memcpy(&modelLoader, &constData[posSectionModelLoader + 4 + i*sizeof(FieldModelLoaderStruct)], sizeof(FieldModelLoaderStruct));

		_modelLoaders.append(modelLoader);

		qDebug() << "=== model loader ===";
		qDebug() << "faceID" << modelLoader.faceID << "bonesCount" << modelLoader.bonesCount;
		qDebug() << "partsCount" << modelLoader.partsCount << "animationCount" << modelLoader.animationCount;
		qDebug() << "u" << modelLoader.unknown1 << modelLoader.unknown2 << modelLoader.unknown3;
		qDebug() << "modelID" << modelLoader.modelID;
	}



	loaded = true;
	return true;
}

int FieldModelLoaderPS::modelCount() const
{
	return _modelLoaders.size();
}

const FieldModelLoaderStruct &FieldModelLoaderPS::model(int id) const
{
	return _modelLoaders.at(id);
}
