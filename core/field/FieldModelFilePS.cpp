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
#include "FieldModelFilePS.h"
#include "FieldArchiveIO.h"
#include "FieldModelPartIOPS.h"
#include "FieldModelPartPS.h"
#include "FieldModelAnimationIOPS.h"
#include "FieldModelLoaderPS.h"
#include "TdbFile.h"
#include "../PsColor.h"
#include "FieldPS.h"

FieldModelFilePS::FieldModelFilePS() :
	FieldModelFile()
{
}

quint8 FieldModelFilePS::load(FieldPS *currentField, int model_id, int animation_id, bool animate)
{
	QByteArray BSX_data = currentField->io()->modelData(currentField);
	FieldModelLoaderPS *modelLoader = currentField->fieldModelLoader();
	quint8 model_global_id = model_id < modelLoader->modelCount() ? modelLoader->model(model_id).modelID : 0;
	const char *constData = BSX_data.constData();
	BSX_header header;
	Model_header model_header;
	Model model;

	clear();

	if((quint32)BSX_data.size() < sizeof(BSX_header)) {
		qWarning() << "invalid BSX size" << BSX_data.size();
		return 2;
	}

	/*** Open Header ***/

	memcpy(&header, constData, sizeof(BSX_header));
//	qDebug() << "==== BSX HEADER ====" << 0 << sizeof(BSX_header);
//	qDebug() << "size" << header.size << "offset_models" << header.offset_models;

	if((quint32)BSX_data.size() != header.size || header.offset_models + sizeof(Model_header) > header.size) {
		qWarning() << BSX_data.size() << "error header";
		return 2;
	}

	/*** Open current model Header ***/

	memcpy(&model_header, constData + header.offset_models, sizeof(Model_header));

//	qDebug() << "==== Model header ====" << header.offset_models << sizeof(Model_header);
//	qDebug() << "num_models" << model_header.num_models << "vram" << QString::number(model_header.psx_memory, 16);
//	qDebug() << "texPointer" << model_header.texture_pointer << "unknownPointer" << model_header.unknown_pointer;

	if(model_id >= (int)model_header.num_models) {
		qWarning() << "model_id too large" << model_id << model_header.num_models;
		return 2;
	}

	quint32 offsetModelHeader = header.offset_models + sizeof(Model_header) + model_id*sizeof(Model);

	if(offsetModelHeader + sizeof(Model) > (quint32)BSX_data.size()) {
		qWarning() << BSX_data.size() << "error model";
		return 2;
	}

	memcpy(&model, constData + offsetModelHeader, sizeof(Model));

	_currentColors.clear();
	// We put colors in the same order that the PC version
	_currentColors << COLORRGB_2_QRGB(model.color4) << COLORRGB_2_QRGB(model.color2) << COLORRGB_2_QRGB(model.color3)
				   << COLORRGB_2_QRGB(model.color7) << COLORRGB_2_QRGB(model.color5) << COLORRGB_2_QRGB(model.color6)
				   << COLORRGB_2_QRGB(model.color10) << COLORRGB_2_QRGB(model.color8) << COLORRGB_2_QRGB(model.color9)
				   << COLORRGB_2_QRGB(model.color1);

	_currentScale = model.scale;

//	if(model.unknown != 0) {
//		qDebug() << "???" << model.unknown;
//	}

//	qDebug() << "==== Model" << model_id << "====" << offsetModelHeader << sizeof(Model) << model_header.num_models*sizeof(Model);
//	qDebug() << "modelID" << model.model_id << "modelScale" << model.scale << "offsetSkeleton" << model.offset_skeleton;
//	qDebug() << "???" << model.unknown;
//	qDebug() << "bones index" << model.index_bones_start << "end" << model.index_bones_end << "count" << model.num_bones;
//	qDebug() << "parts index" << model.index_parts_start << "end" << model.index_parts_end << "count" << model.num_parts;
//	qDebug() << "animations index" << model.index_animations_start << "end" << model.index_animations_end << "count" << model.num_animations;

	int curOff = offsetModelHeader + model.offset_skeleton;

//	qDebug() << "after model header" << (offsetModelHeader + sizeof(Model)) << "skeleton offset" << curOff;

	/*** Open model from BCX if main character ***/

	if(model_global_id >= 1 && model_global_id <= 9) {
		QString fileName;
		switch(model_global_id) {
		case 1:		fileName = "CLOUD";		break;
		case 2:		fileName = "EARITH";	break;
		case 3:		fileName = "BALLET";	break;
		case 4:		fileName = "TIFA";		break;
		case 5:		fileName = "RED";		break;
		case 6:		fileName = "CID";		break;
		case 7:		fileName = "YUFI";		break;
		case 8:		fileName = "KETCY";		break;
		case 9:		fileName = "VINCENT";	break;
		}
		int bcxAnimationCount;
		if(!openBCX(currentField->io()->fileData(fileName + ".BCX"), animation_id, animate, &bcxAnimationCount)) {
			return false;
		}

		animation_id -= bcxAnimationCount;
	}

	if(curOff + model.num_bones * sizeof(BonePS) > (quint32)BSX_data.size()) {
		qWarning() << "invalid skeleton size" << model.num_bones;
		return false;
	}

	/*** Open skeleton ***/

	curOff = openSkeleton(constData, curOff, model.num_bones);

	if(curOff + model.num_parts * sizeof(Part) > (quint32)BSX_data.size()) {
		qWarning() << "invalid parts size" << model.num_parts;
		return false;
	}

	/*** Open mesh ***/

	curOff = openMesh(constData, curOff, BSX_data.size(), model.num_parts);

	if(_animation.isEmpty()) { // if there is no animation opened from BCX
		if(animation_id >= model.num_animations) {
			qWarning() << "No animation called" << animation_id;
			return false;
		}

		if(curOff + (animation_id + 1) * sizeof(Animation) > (quint32)BSX_data.size()) {
			qWarning() << "invalid animation size" << animation_id;
			return false;
		}

		/*** Open animations ***/

		openAnimation(constData, curOff, animation_id, BSX_data.size(), animate);
	}

	/*** Open textures header ***/

	quint32 offsetTexHeader = header.offset_models + model_header.texture_pointer;

	if(offsetTexHeader + 8 > (quint32)BSX_data.size()) {
		qWarning() << BSX_data.size() << "error tex header";
		return 2;
	}

	quint32 unknownTex, unknownTex2;
	memcpy(&unknownTex, constData + offsetTexHeader, 4);
	memcpy(&unknownTex2, constData + offsetTexHeader+4, 4);
	quint8 texCount = unknownTex2 & 0xFF;

//	qDebug() << "==== TEXTURES HEADER ====" << offsetTexHeader;
//	qDebug() << "texCount" << texCount << "unknown1" << unknownTex << "unknown2" << (unknownTex2 & 0xFFFFFF00);

	/*** Open texture Headers ***/

	if(offsetTexHeader + 8 + texCount*sizeof(TexHeader) > (quint32)BSX_data.size()) {
		qWarning() << BSX_data.size() << "error tex data";
		return 2;
	}

	QList<TexHeader> texHeaders;

	for(int tex=0 ; tex<texCount ; ++tex) {
		TexHeader texHeader;
		memcpy(&texHeader, constData + offsetTexHeader + 8 + tex*sizeof(TexHeader), sizeof(TexHeader));

		texHeader.offset_data += offsetTexHeader;

//		qDebug() << "==== TEXTURE HEADER" << tex << "====" << (offsetTexHeader + 8 + tex*sizeof(TexHeader));
//		qDebug() << "width" << texHeader.width << "height" << texHeader.height << "x" << texHeader.vramX << "y" << texHeader.vramY;
//		qDebug() << "offsetData" << texHeader.offset_data << "endData" << (texHeader.offset_data + texHeader.width * 2 * texHeader.height);

		texHeaders.append(texHeader);
	}

	/*
	// DEBUG
	for(int tex=0 ; tex<texHeaders.size() ; ++tex) {
		if(tex==1)	continue;
		openTexture(constData, 0, texHeaders.at(tex), texHeaders.at(1), 1).save(QString("Makou-Texture%1.png").arg(tex));
	}*/

	if(model_id < modelLoader->modelCount()) {
		TdbFile tdb;
		quint8 faceID = modelLoader->model(model_id).faceID;
		QMap<TextureInfo, int> texAlreadyLoaded;

		if(faceID < 0x21) {
			tdb.open(currentField->io()->fileData("FIELD.TDB"));
		}

		int texID=0;

		foreach(FieldModelPart *part, _parts) {
			QList<int> texIds;
			QList<TextureInfo> texAlreadyLoadedInPart;
			int lastTexHeight=0;

			foreach(TextureInfo texInfo, ((FieldModelPartPS *)part)->textures()) {
				bool contains2 = false;
				foreach(const TextureInfo &texInfo2, texAlreadyLoadedInPart) {
					if(texInfo2 == texInfo) {
						contains2 = true;
						break;
					}
				}
				if(!contains2) {
					texAlreadyLoadedInPart.append(texInfo);
				}
				if(contains2 && (faceID >= 0x21 || (texInfo.type != 0 && texInfo.type != 1))) {
					texInfo.imgY += lastTexHeight;
				}

//				qDebug() << "TEXTUREINFO";
//				qDebug() << "type" << texInfo.type << "bpp" << texInfo.bpp;
//				qDebug() << "imgX" << texInfo.imgX << "imgY" << texInfo.imgY;
//				qDebug() << "palX" << texInfo.palX << "palY" << texInfo.palY;

				QMapIterator<TextureInfo, int> it(texAlreadyLoaded);
				bool contains = false;
				while(it.hasNext()) {
					it.next();
					if(it.key() == texInfo) {
						contains = true;
						break;
					}
				}

				if(contains) {
					texIds.append(texAlreadyLoaded.value(texInfo));
				} else {
					if(faceID < 0x21 && (texInfo.type == 0 || texInfo.type == 1)) { // Eye and Mouth
//						qDebug() << (texInfo.type == 0 ? "EYE" : "MOUTH") << texID;
						texIds.append(texID);
						texAlreadyLoaded.insert(texInfo, texID);
						_loaded_tex.insert(texID++, tdb.texture(faceID, texInfo.type == 0 ? TdbFile::Eye : TdbFile::MouthClosed));
					} else {
//						qDebug() << "OTHER" << texID;

						TexHeader imgHeader=TexHeader(), palHeader=TexHeader();
						bool imgFound=false, palFound=false;
						foreach(const TexHeader &texHeader, texHeaders) {
							if(!imgFound && texInfo.imgX == texHeader.vramX && texInfo.imgY == texHeader.vramY) {
								imgHeader = texHeader;
								imgFound = true;
							} else if(!palFound && texInfo.palX == texHeader.vramX && texInfo.palY == texHeader.vramY) {
								palHeader = texHeader;
								palFound = true;
							}
							if(imgFound && palFound)	break;
						}
						if(imgFound && palFound) {
							lastTexHeight += imgHeader.height;
							texIds.append(texID);
							texAlreadyLoaded.insert(texInfo, texID);
							_loaded_tex.insert(texID++, openTexture(constData, BSX_data.size(), imgHeader, palHeader, texInfo.bpp));
						} else {
							qWarning() << "Tex not found!";
						}
					}
				}
			}

//			qDebug() << "texIds" << ((FieldModelPartPS *)part)->boneID() << texIds;

			if(!texIds.isEmpty()) {
				foreach(FieldModelGroup *group, part->groups()) {
					if(group->textureNumber() != -1) {
						group->setTextureNumber(texIds.at(group->textureNumber()));
						const QImage currentTex = _loaded_tex.value(group->textureNumber());
						int texWidth=currentTex.width(), texHeight=currentTex.height();

						foreach(Poly *poly, group->polygons()) {
							poly->divTexCoords(texWidth, texHeight);
						}
					}
				}
			}
		}
	}

	dataLoaded = true;

	return true;
}

int FieldModelFilePS::openSkeleton(const char *constData, int curOff, quint8 numBones)
{
	for(quint32 i=0 ; i<numBones ; ++i) {
		BonePS bonePS;
		FieldModelBone bone;
		memcpy(&bonePS, constData + curOff, sizeof(BonePS));
//		qDebug() << "bone" << i << bonePS.length << bonePS.parent << bonePS.unknown;
		bone.size = bonePS.length / MODEL_SCALE_PS;
		bone.parent = bonePS.parent;
		_skeleton.addBone(bone);

		curOff += sizeof(BonePS);
	}

	return curOff;
}

int FieldModelFilePS::openMesh(const char *constData, int curOff, int size, quint8 numParts)
{
	QBuffer io;
	io.setData(constData, size);
	if (!io.open(QIODevice::ReadOnly)) {
		qWarning() << "FieldModelFilePS::openMesh device not opened" << io.errorString();
		return curOff + numParts * sizeof(Part);
	}
	FieldModelPartIOPS partIO(&io);

	for(quint32 i=0 ; i<numParts ; ++i) {
		FieldModelPartPS *part = new FieldModelPartPS();
//		qDebug() << "==== PART" << i << "====" << curOff;
		partIO.device()->seek(curOff);
		if(partIO.read(part)) {
			_parts.insert(part->boneID(), part);

//			QFile textOut(QString("fieldModelPartPS%1.txt").arg(i));
//			textOut.open(QIODevice::WriteOnly);
//			textOut.write(part->toString().toLatin1());
//			textOut.close();

		} else {
			qWarning() << "Error open part" << i;
			delete part;
		}
		curOff += sizeof(Part);
	}

	return curOff;
}

bool FieldModelFilePS::openAnimation(const char *constData, int curOff, int animation_id, int size, bool animate)
{
	QBuffer io;
	io.setData(constData, size);
	if (!io.open(QIODevice::ReadOnly)) {
		qWarning() << "FieldModelFilePS::openAnimation device not opened" << io.errorString();
		return false;
	}
	FieldModelAnimationIOPS animIO(&io);
	animIO.device()->seek(curOff + sizeof(Animation) * animation_id);

	return animIO.read(_animation, animate ? -1 : 1);
}

QImage FieldModelFilePS::openTexture(const char *constData, int size, const TexHeader &imgHeader, const TexHeader &palHeader, quint8 bpp)
{
	if(imgHeader.offset_data + imgHeader.height * 2 > (quint32)size) {
		qWarning() << "Offset texture too large" << (imgHeader.offset_data + imgHeader.height * 2);
		return QImage();
	}

	int width = imgHeader.width;
	if(bpp == 1) {
		width *= 2;
	} else if(bpp == 0) {
		width *= 4;
	}

	QImage img(width, imgHeader.height, QImage::Format_ARGB32);
	QRgb *px = (QRgb *)img.bits();
	int i=0;
	for(int y=0 ; y<imgHeader.height ; ++y) {
		for(int x=0 ; x<imgHeader.width*2 ; ++x) {
			quint8 index = constData[imgHeader.offset_data + i];
			quint16 color;

			if(bpp == 1) {
				if(palHeader.offset_data + index*2 + 2 > (quint32)size) {
					qWarning() << "Offset palette too large";
					continue;
				}
				memcpy(&color, constData + palHeader.offset_data + index*2, 2);

				px[i] = PsColor::fromPsColor(color, true);
			} else if(bpp == 0) {
				if(palHeader.offset_data + (index & 0xF)*2 + 2 > (quint32)size) {
					qWarning() << "Offset palette too large";
					continue;
				}
				memcpy(&color, constData + palHeader.offset_data + (index & 0xF)*2, 2);

				px[i*2] = PsColor::fromPsColor(color, true);

				if(palHeader.offset_data + (index >> 4)*2 + 2 > (quint32)size) {
					qWarning() << "Offset palette too large";
					continue;
				}
				memcpy(&color, constData + palHeader.offset_data + (index >> 4)*2, 2);

				px[i*2 + 1] = PsColor::fromPsColor(color, true);
			}
			++i;
		}
	}
	return img;
}

bool FieldModelFilePS::openBCX(const QByteArray &BCX, int animationID, bool animate, int *numAnimations)
{
	BSX_header header;
	BCXModel model;
	const char *constData = BCX.constData();
	if(numAnimations) {
		*numAnimations = 0;
	}

	if((quint32)BCX.size() < sizeof(BSX_header)) {
		qWarning() << "invalid BSX size" << BCX.size();
		return false;
	}

	/*** Open Header ***/

	memcpy(&header, constData, sizeof(BSX_header));
//	qDebug() << "==== BCX HEADER ====" << 0 << sizeof(BSX_header);
//	qDebug() << "size" << header.size << "offset_models" << header.offset_models;

	if((quint32)BCX.size() != header.size || header.offset_models >= header.size) {
		qWarning() << BCX.size() << "error";
		return false;
	}

	/*** Open model Header ***/

	memcpy(&model, constData + header.offset_models, sizeof(BCXModel));

	if(numAnimations) {
		*numAnimations = model.num_animations;
	}

	model.offset_skeleton -= 0x80000000;

//	qDebug() << "==== Model ====" << header.offset_models << sizeof(BCXModel) << (header.offset_models+sizeof(BCXModel));
//	qDebug() << "unknown" << model.unknown << (model.unknown & 0xFF) << ((model.unknown >> 8) & 0xFF) << QString::number(model.unknown, 16);
//	qDebug() << "bones count" << model.num_bones << "parts count" << model.num_parts << "animations count" << model.num_animations;
//	qDebug() << "scale" << model.scale;
//	qDebug() << "offsetParts" << (header.offset_models + sizeof(BCXModel) + model.offset_parts)
//				<< "offsetAnimations" << (header.offset_models + sizeof(BCXModel) + model.offset_animations);
//	qDebug() << "offsetSkeleton" << model.offset_skeleton;

	quint32 curOff = model.offset_skeleton;

	if(curOff + model.num_bones * sizeof(BonePS) >= (quint32)BCX.size()) {
		qWarning() << "invalid skeleton size" << model.num_bones;
		return false;
	}

	/*** Open skeleton ***/

	curOff = openSkeleton(constData, curOff, model.num_bones);

	if(curOff + model.num_parts * sizeof(Part) >= (quint32)BCX.size()) {
		qWarning() << "invalid parts size" << model.num_parts;
		return false;
	}

	/*** Open mesh ***/

	curOff = openMesh(constData, curOff, BCX.size(), model.num_parts);

	/*** Open current animation ***/

	if(animationID < model.num_animations) {
		openAnimation(constData, curOff, animationID, BCX.size(), animate);
	}

	return true;
}

const QList<QRgb> &FieldModelFilePS::lightColors() const
{
	return _currentColors;
}

quint16 FieldModelFilePS::scale() const
{
	return _currentScale;
}
