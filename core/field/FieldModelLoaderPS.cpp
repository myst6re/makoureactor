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
#include "Field.h"

FieldModelLoaderPS::FieldModelLoaderPS(Field *field) :
	FieldModelLoader(field)
{
}

void FieldModelLoaderPS::clear()
{
	_modelLoaders.clear();
}

bool FieldModelLoaderPS::open()
{
	return open(field()->sectionData(Field::ModelLoader));
}

bool FieldModelLoaderPS::open(const QByteArray &data)
{
	const char *constData = data.constData();

	if(data.size() < 4) {
		return false;
	}

	quint16 size, modelCount;
	memcpy(&size, constData, 2);
	memcpy(&modelCount, constData + 2, 2);

	if(size != data.size() || (size-4) / 8 != modelCount || (size-4) % 8 != 0) {
		qWarning() << "invalid model loader size" << size << modelCount << data.size();
		return false;
	}

	if(sizeof(FieldModelLoaderStruct) != 8) {
		qWarning() << "invalid model loader struct size" << sizeof(FieldModelLoaderStruct);
		Q_ASSERT(false);
	}

	clear();

	FieldModelLoaderStruct modelLoader;

	for(quint32 i=0 ; i<modelCount ; ++i) {
		memcpy(&modelLoader, constData + 4 + i*sizeof(FieldModelLoaderStruct), sizeof(FieldModelLoaderStruct));

		_modelLoaders.append(modelLoader);

//		qDebug() << "=== model loader ===";
//		qDebug() << "faceID" << modelLoader.faceID << "bonesCount" << modelLoader.bonesCount;
//		qDebug() << "partsCount" << modelLoader.partsCount << "animationCount" << modelLoader.animationCount;
//		qDebug() << "u" << modelLoader.unknown1 << modelLoader.unknown2 << modelLoader.unknown3;
//		qDebug() << "modelID" << modelLoader.modelID;
	}

	setOpen(true);

	return true;
}

QByteArray FieldModelLoaderPS::save() const
{
	QByteArray ret;

	quint16 modelCount = _modelLoaders.size(),
			size = 4 + modelCount * sizeof(FieldModelLoaderStruct);

	ret.append((char *)&size, 2);
	ret.append((char *)&modelCount, 2);

	foreach(const FieldModelLoaderStruct &modelLoader, _modelLoaders) {
		ret.append((char *)&modelLoader, sizeof(FieldModelLoaderStruct));
	}

	return ret;
}

int FieldModelLoaderPS::modelCount() const
{
	return _modelLoaders.size();
}

int FieldModelLoaderPS::animCount(int modelID) const
{
	if(modelID >= 0 && modelID < _modelLoaders.size())
		return _modelLoaders.at(modelID).animationCount;
	return 0;
}

quint16 FieldModelLoaderPS::unknown(int modelID) const
{
	// TODO: returns a quint16 word? (unknown2 is quint8)
	if(modelID >= 0 && modelID < _modelLoaders.size())
		return _modelLoaders.at(modelID).unknown2/* | (_modelLoaders.at(modelID).unknown3 << 8)*/;
	return 0;
}

void FieldModelLoaderPS::setUnknown(int modelID, quint16 unknown)
{
	// TODO: sets a quint16 word?
	if(modelID >= 0 && modelID < _modelLoaders.size()
			&& _modelLoaders.at(modelID).unknown2 != unknown) {
		_modelLoaders[modelID].unknown2 = unknown;
		setModified(true);
	}
}

const FieldModelLoaderStruct &FieldModelLoaderPS::model(int modelID) const
{
	return _modelLoaders.at(modelID);
}

void FieldModelLoaderPS::setModel(int modelID, const FieldModelLoaderStruct &modelLoader)
{
	if(modelID >= 0 && modelID < _modelLoaders.size()
			&& memcmp(&modelLoader, &_modelLoaders.at(modelID), sizeof(FieldModelLoaderStruct)) != 0) {
		_modelLoaders.replace(modelID, modelLoader);
		setModified(true);
	}
}
