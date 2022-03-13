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
#include "FieldModelFilePS.h"
#include "FieldArchiveIO.h"
#include "TdbFile.h"
#include <PsColor.h>
#include "FieldPS.h"
#include "BcxFile.h"
#include "BsxFile.h"

FieldModelFilePS::FieldModelFilePS() :
	FieldModelFile(),
    _currentField(nullptr), _currentModelID(-1),
    _scale(0), _isModified(false)
{
}

void FieldModelFilePS::clear()
{
	_loadedTex.clear();
	_textures = FieldModelTexturesPS();
	FieldModelFile::clear();
	_isModified = false;
}

bool FieldModelFilePS::load(FieldPS *currentField, int modelID, int animationID, bool animate)
{
	Q_UNUSED(animate)
	quint8 modelGlobalId = 0;
	bool modelIdIsValid = currentField->fieldModelLoader()->isOpen() && modelID < currentField->fieldModelLoader()->modelCount();
	FieldModelFilePS modelBcx;

	_isModified = false;
	_currentField = currentField;
	_currentModelID = modelID;

	if (modelIdIsValid) {
		modelGlobalId = currentField->fieldModelLoader()->model(modelID).modelID;

		if (modelGlobalId >= 1 && modelGlobalId <= 9) {
			QString fileName;
			switch (modelGlobalId) {
			case 1:    fileName = "CLOUD";      break;
			case 2:    fileName = "EARITH";     break;
			case 3:    fileName = "BALLET";     break;
			case 4:    fileName = "TIFA";       break;
			case 5:    fileName = "RED";        break;
			case 6:    fileName = "CID";        break;
			case 7:    fileName = "YUFI";       break;
			case 8:    fileName = "KETCY";      break;
			case 9:    fileName = "VINCENT";    break;
			}

			QByteArray BCXData = currentField->io()->fileData(fileName + ".BCX");
			if (BCXData.isEmpty()) {
				qWarning() << "FieldModelFilePS::load cannot open bcx file";
				return false;
			}
			QBuffer ioBcx;
			ioBcx.setData(BCXData);
			if (!ioBcx.open(QIODevice::ReadOnly)) {
				qWarning() << "FieldModelFilePS::load cannot open bcx buffer" << ioBcx.errorString();
				return false;
			}

			BcxFile bcx(&ioBcx);
			if (!bcx.read(modelBcx)) {
				return false;
			}
		}
	}

	QByteArray BSXData = currentField->io()->modelData(currentField);
	if (BSXData.isEmpty()) {
		qWarning() << "FieldModelFilePS::load cannot open bsx file";
		return false;
	}
	QBuffer ioBsx;
	ioBsx.setData(BSXData);
	if (!ioBsx.open(QIODevice::ReadOnly)) {
		qWarning() << "FieldModelFilePS::load cannot open bsx buffer" << ioBsx.errorString();
		return false;
	}

	BsxFile bsx(&ioBsx);
	if (!bsx.seek(modelID)) {
		return false;
	}
	if (!bsx.read(this)) {
		return false;
	}
	if (!bsx.seekTextures()) {
		return false;
	}
	if (!bsx.readTextures(&_textures)) {
		return false;
	}

	const QList<QRect> &texturesRects = _textures.rects();

	if (!texturesRects.isEmpty()) {
		for (const FieldModelBone &bone : _skeleton.bones()) {

			for (FieldModelPart *part : bone.parts()) {
				QHash<int, int> imgY;

				for (FieldModelGroup *group : part->groups()) {
					if (group->hasTexture()) {
						FieldModelTextureRefPS *textureRef = static_cast<FieldModelTextureRefPS *>(group->textureRef());

						if (textureRef->type() > 1) {
							QPoint newPos = textureRef->imgPos() + QPoint(0, imgY.value(textureRef->imgX()));
							int rectId = 0;
							for (const QRect &rect : texturesRects) {
								if (rect.topLeft() == newPos) {
									break;
								}
								rectId++;
							}

							if (rectId >= texturesRects.size()) {
								qWarning() << "FieldModelFilePS::load adjust texture pos error 1";
								imgY.clear();
							} else {
								textureRef->setImgY(newPos.y());
								const QRect &rect = texturesRects.at(rectId);
								imgY.insert(textureRef->imgX(), imgY.value(textureRef->imgX()) + rect.height());
							}
						}
					}
				}
			}
		}
	}

	if (modelBcx.isValid()) {
		if (!skeleton().isEmpty()) {
			qWarning() << "FieldPS::fieldModel bsx present, but bcx skeleton not empty";
			for (const FieldModelBone &bone : skeleton().bones()) {
				qDeleteAll(bone.parts());
			}
		}
		setSkeleton(modelBcx.skeleton());
		setAnimations(modelBcx.animations() + animations());
		modelBcx.setSkeleton(FieldModelSkeleton());
	}
	// FIXME: only selected animation -> not optimal
	if (animationID < animationCount()) {
		setAnimations(QList<FieldModelAnimation>() << animation(animationID));
	}

	return true;
}

QImage FieldModelFilePS::loadedTexture(FieldModelGroup *group)
{
	if (_loadedTex.contains(group)) {
		return _loadedTex.value(group);
	}

	FieldModelTextureRefPS *texRefPS = static_cast<FieldModelTextureRefPS *>(group->textureRef());
	QImage tex;

	if (texRefPS->type() == 0 || texRefPS->type() == 1) { // Eye and Mouth
		if (!_currentField) {
			qWarning() << "FieldModelFilePS::loadedTexture currentField not set";
			return QImage();
		}
		quint8 faceID = _currentField->fieldModelLoader()->model(_currentModelID).faceID;
		TdbFile tdb;
		bool useTdb = faceID < 0x21;

		if (useTdb && tdb.open(_currentField->io()->fileData("FIELD.TDB"))) {
			tex = tdb.texture(faceID, texRefPS->type() == 0 ? TdbFile::Eye : TdbFile::MouthClosed);
		} else {
			qWarning() << "FieldModelFilePS::loadedTexture error";
			return QImage();
		}
		group->setFloatCoords(tex.width(), tex.height());
	} else {
		tex = _textures.toImage(texRefPS->imgX(), texRefPS->imgY(),
								texRefPS->palX(), texRefPS->palY(),
								texRefPS->bpp() == 0 ? FieldModelTexturesPS::Bpp4
													 : FieldModelTexturesPS::Bpp8);

		// Fix tex coords according to tex size (implies setFloatCoords)
		group->removeSpriting(tex.width(), tex.height());
	}

	_loadedTex.insert(group, tex);

	return tex;
}

QHash<void *, QImage> FieldModelFilePS::loadedTextures()
{
	QHash<void *, QImage> ret;

	for (const FieldModelBone &bone : _skeleton.bones()) {
		for (FieldModelPart *part : bone.parts()) {
			for (FieldModelGroup *group : part->groups()) {
				if (group->hasTexture()) {
					QImage texture = loadedTexture(group);
					if (!texture.isNull()) {
						ret.insert((void *)group, texture);
					}
				}
			}
		}
	}

	return ret;
}

QImage FieldModelFilePS::vramImage() const
{
	for (const QRect &rect : _textures.rects()) {
		if (rect.height() == 1) {
			return _textures.toImage(rect.topLeft(), FieldModelTexturesPS::Bpp8);
		}
	}
	return QImage();
}
