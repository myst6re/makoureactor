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
#include "FieldModelLoaderPS.h"
#include "CharArchive.h"
#include "Field.h"
#include "FieldModelFilePC.h"
#include "AFile.h"
#include <TexFile>

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

	if (data.size() < 4) {
		return false;
	}

	quint16 size, modelCount;
	memcpy(&size, constData, 2);
	memcpy(&modelCount, constData + 2, 2);

	if (size != data.size() || (size-4) / 8 != modelCount || (size-4) % 8 != 0) {
		qWarning() << "invalid model loader size" << size << modelCount << data.size();
		return false;
	}

	if (sizeof(FieldModelLoaderStruct) != 8) {
		qWarning() << "invalid model loader struct size" << sizeof(FieldModelLoaderStruct);
		Q_ASSERT(false);
	}

	clear();

	FieldModelLoaderStruct modelLoader;

	for (quint32 i=0; i<modelCount; ++i) {
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

	quint16 modelCount = quint16(_modelLoaders.size()),
			size = 4 + modelCount * sizeof(FieldModelLoaderStruct);

	ret.append((char *)&size, 2);
	ret.append((char *)&modelCount, 2);

	for (const FieldModelLoaderStruct &modelLoader : _modelLoaders) {
		ret.append((char *)&modelLoader, sizeof(FieldModelLoaderStruct));
	}

	return ret;
}

qsizetype FieldModelLoaderPS::modelCount() const
{
	return _modelLoaders.size();
}

qsizetype FieldModelLoaderPS::animCount(int modelID) const
{
	if (modelID >= 0 && modelID < _modelLoaders.size()) {
		return _modelLoaders.at(modelID).animationCount;
	}
	return 0;
}

quint16 FieldModelLoaderPS::unknown(int modelID) const
{
	// TODO: returns a quint16 word? (unknown2 is quint8)
	if (modelID >= 0 && modelID < _modelLoaders.size()) {
		return _modelLoaders.at(modelID).unknown2/* | (_modelLoaders.at(modelID).unknown3 << 8)*/;
	}
	return 0;
}

void FieldModelLoaderPS::setUnknown(int modelID, quint16 unknown)
{
	// TODO: sets a quint16 word?
	if (modelID >= 0 && modelID < _modelLoaders.size()
			&& _modelLoaders.at(modelID).unknown2 != unknown) {
		_modelLoaders[modelID].unknown2 = quint8(unknown);
		setModified(true);
	}
}

const FieldModelLoaderStruct &FieldModelLoaderPS::model(int modelID) const
{
	return _modelLoaders.at(modelID);
}

void FieldModelLoaderPS::setModel(int modelID, const FieldModelLoaderStruct &modelLoader)
{
	if (modelID >= 0 && modelID < _modelLoaders.size()
			&& memcmp(&modelLoader, &_modelLoaders.at(modelID), sizeof(FieldModelLoaderStruct)) != 0) {
		_modelLoaders.replace(modelID, modelLoader);
		setModified(true);
	}
}

FieldModelLoaderPC FieldModelLoaderPS::toPC(BsxFile *bsx, CharArchive *charArchive, bool *ok) const
{
	FieldModelLoaderPC ret(field());

	int i = 0;
	*ok = true;

	for (const FieldModelLoaderStruct &psLoader : _modelLoaders) {
		FieldModelFilePS modelFile;
		qDebug() << "modelID" << psLoader.faceID << psLoader.unknown1 << psLoader.unknown2 << psLoader.unknown3 << psLoader.bonesCount;

		if (!bsx->seek(i)) {
			*ok = false;
			return ret;
		}

		if (!bsx->read(&modelFile)) {
			*ok = false;
			return ret;
		}

		QString fileName;
		bool isMainModel = true;

		switch (psLoader.modelID) {
		case 1:    fileName = "AAAA";    break; // Cloud
		case 2:    fileName = "AUFF";    break; // Aerith
		case 3:    fileName = "ACGD";    break; // Barret
		case 4:    fileName = "AAGB";    break; // Tifa
		case 5:    fileName = "ADDA";    break; // Red XIII
		case 6:    fileName = "ABDA";    break; // Cid
		case 7:    fileName = "ABJB";    break; // Yuffie
		case 8:    fileName = "AEBC";    break; // Cait Sith
		case 9:    fileName = "AEHD";    break; // Vincent
		default:
			isMainModel = false;
			fileName = "AAAA"; // AAAA: Cloud
			if (charArchive != nullptr) {
				QByteArray psIdentifier = modelFile.signature();
				QStringList hrcFiles = charArchive->hrcFiles(std::max(1, psLoader.bonesCount - 1), psLoader.partsCount);
				QHash<QString, int> matchingFileNames;

				for (const QString &hrc: qAsConst(hrcFiles)) {
					FieldModelFilePC modelFilePC;
					QStringList textureNames;
					modelFilePC.load(charArchive, hrc, textureNames);
					QByteArray identifier = modelFilePC.signature();
					
					if (identifier == psIdentifier) {
						int commonColorCount = modelFile.commonColorCount(modelFilePC);
						matchingFileNames.insert(hrc, commonColorCount);
					}
				}
				
				if (matchingFileNames.size() > 1) {
					int maxCommonColorCount = -1;
					
					QHashIterator<QString, int> it(matchingFileNames);
					while (it.hasNext()) {
						it.next();
						int commonColorCount = it.value();
						
						if (commonColorCount > maxCommonColorCount) { // FIXME: approximation
							maxCommonColorCount = commonColorCount;
							fileName = it.key();
						}
					}
				}
			}
			break;
		}

		ret.insertModel(i, fileName + ".HRC");
		
		QStringList mainAnimationNames;
		
		if (isMainModel) {
			mainAnimationNames = FieldModelLoaderPC::mainAnimationNames(fileName);
		}

		for (int j = 0; j < psLoader.animationCount; ++j) {
			// ACFE: Cloud standing animation
			QString animationName = "ACFE";
			int animationId = isMainModel ? j - 3 : j;
			
			if (animationId >= 0 && charArchive != nullptr && animationId < modelFile.animationCount()) {
				FieldModelAnimation animationPs = modelFile.animation(animationId);
				QStringList aFiles = charArchive->aFiles(std::max(1, psLoader.bonesCount - 1), animationPs.frameCount());
				int maxCommonRotationCount = -1;

				for (const QString &aFile: aFiles) {
					FieldModelAnimation animation;
					AFile io(charArchive->fileIO(aFile % ".a"));
					if (io.read(animation, -1)) {
						int commonRotationCount = animationPs.commonRotationCount(animation);
						
						if (commonRotationCount > maxCommonRotationCount) { // FIXME: approximation
							maxCommonRotationCount = commonRotationCount;
							animationName = aFile;
						}
					}
				}
			} else if (animationId < 0 && j < mainAnimationNames.size()) {
				animationName = mainAnimationNames.at(j);
			}
			ret.insertAnim(i, j, animationName);
			ret.setAnimUnknown(i, j, 1);
		}

		ret.setScale(i, modelFile.scale());
		ret.setUnknown(i, psLoader.unknown2);
		ret.setGlobalColor(i, modelFile.globalColor());
		ret.setLightColors(i, modelFile.lightColors());

		i += 1;
	}


	return ret;
}
