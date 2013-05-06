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
#include "FieldModelPartPS.h"
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
	_currentColors << COLORRGB_2_QRGB(model.color1) << COLORRGB_2_QRGB(model.color2) << COLORRGB_2_QRGB(model.color3)
				   << COLORRGB_2_QRGB(model.color4) << COLORRGB_2_QRGB(model.color5) << COLORRGB_2_QRGB(model.color6)
				   << COLORRGB_2_QRGB(model.color7) << COLORRGB_2_QRGB(model.color8) << COLORRGB_2_QRGB(model.color9)
				   << COLORRGB_2_QRGB(model.color10);

	_currentScale = model.scale;

	if(model.unknown != 0) {
		qDebug() << "???" << model.unknown;
	}

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

	if(_frames.isEmpty()) { // if there is no animation opened from BCX
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
		tdb.open(currentField->io()->fileData("FIELD.TDB"));

		quint8 faceID = modelLoader->model(model_id).faceID;
		QMap<TextureInfo, int> texAlreadyLoaded;

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
		Bone bone;
		memcpy(&bonePS, constData + curOff, sizeof(BonePS));
//		qDebug() << "bone" << i << bonePS.length << bonePS.parent << bonePS.unknown;
		bone.size = bonePS.length / MODEL_SCALE_PS;
		bone.parent = bonePS.parent;
		_bones.append(bone);

		curOff += sizeof(BonePS);
	}

	return curOff;
}

int FieldModelFilePS::openMesh(const char *constData, int curOff, int size, quint8 numParts)
{
	for(quint32 i=0 ; i<numParts ; ++i) {
		FieldModelPartPS *part = new FieldModelPartPS();
//		qDebug() << "==== PART" << i << "====" << curOff;
		if(part->open(constData, curOff, size)) {
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
	Animation a;
	memcpy(&a, constData + curOff + sizeof(Animation)*animation_id, sizeof(Animation));

	a.offset_data -= 0x80000000;

//	qDebug() << "==== ANIMATION" << animation_id << "====" << (curOff + sizeof(Animation)*animation_id);
//	qDebug() << "numFrames" << a.num_frames << "numBones" << a.num_bones;
//	qDebug() << "numFramesTrans" << a.num_frames_translation << "numFramesStaticTrans" << a.num_static_translation << "numFramesRot" << a.num_frames_rotation;
//	qDebug() << "offsetFramesTrans" << (a.offset_data + a.offset_frames_translation) << "offsetFramesStaticTrans" << (a.offset_data + a.offset_static_translation) << "offsetFramesRot" << (a.offset_data + a.offset_frames_rotation);
//	qDebug() << "offsetData" << a.offset_data;

	quint32 offsetFrameRotation = a.offset_data + a.offset_frames_rotation;
	quint32 offsetFrameStatic = a.offset_data + a.offset_static_translation;
	quint32 offsetFrameTranslation = a.offset_data + a.offset_frames_translation;

	if(a.offset_data + 4 + a.num_bones * 8 >= (quint32)size) {
		qWarning() << "invalid size animation" << (a.offset_data + 4 + a.num_bones * 8) << size;
		return false;
	}

//	quint32 unknown;
//	memcpy(&unknown, constData + a.offset_data, 4);

//	qDebug() << "Unknown" << unknown << QString::number(unknown, 16);

	this->a_bones_count = qMin((int)a.num_bones, _bones.size());

	quint16 numFrames2 = animate ? a.num_frames : qMin((quint16)1, a.num_frames);

	for(int frame=0 ; frame<numFrames2 ; ++frame) {
		QList<PolyVertex> rotation_coords, rotation_coordsTrans;

		for(int bone=0 ; bone<a.num_bones ; ++bone) {
			FrameTranslation frameTrans;
			PolyVertex rot, trans;

			memcpy(&frameTrans, constData + a.offset_data + 4 + bone*8, sizeof(FrameTranslation));

			// Rotation

			if(frameTrans.flag & 0x01) {
				quint32 offsetToRotation = offsetFrameRotation + frameTrans.rx * a.num_frames + frame;

				if(offsetToRotation >= (quint32)size) {
					qWarning() << "OffsetToRotation too large" << offsetToRotation << size;
					continue;
				}

				rot.x = 360.0f * (quint8)constData[offsetToRotation] / 255.0f;
			} else {
				rot.x = 360.0f * frameTrans.rx / 255.0f;
			}

			if(frameTrans.flag & 0x02) {
				quint32 offsetToRotation = offsetFrameRotation + frameTrans.ry * a.num_frames + frame;

				if(offsetToRotation >= (quint32)size) {
					qWarning() << "OffsetToRotation too large" << offsetToRotation << size;
					continue;
				}

				rot.y = 360.0f * (quint8)constData[offsetToRotation] / 255.0f;
			} else {
				rot.y = 360.0f * frameTrans.ry / 255.0f;
			}

			if(frameTrans.flag & 0x04) {
				quint32 offsetToRotation = offsetFrameRotation + frameTrans.rz * a.num_frames + frame;

				if(offsetToRotation >= (quint32)size) {
					qWarning() << "OffsetToRotation too large" << offsetToRotation << size;
					continue;
				}

				rot.z = 360.0f * (quint8)constData[offsetToRotation] / 255.0f;
			} else {
				rot.z = 360.0f * frameTrans.rz / 255.0f;
			}

			// (translation)

			qint16 translation=0;

			if(frameTrans.flag & 0x10) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.tx * a.num_frames * 2 + frame * 2;

				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}

				memcpy(&translation, constData + offsetToTranslation, 2);
			} else if(frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.tx * 2;

				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}

				memcpy(&translation, constData + offsetToTranslation, 2);
			}
			trans.x = -translation / MODEL_SCALE_PS;
			translation=0;

			if(frameTrans.flag & 0x20) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.ty * a.num_frames * 2 + frame * 2;

				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}

				memcpy(&translation, constData + offsetToTranslation, 2);
			} else if(frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.ty * 2;

				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}

				memcpy(&translation, constData + offsetToTranslation, 2);
			}
			trans.y = -translation / MODEL_SCALE_PS;
			translation=0;

			if(frameTrans.flag & 0x40) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.tz * a.num_frames * 2 + frame * 2;

				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}

				memcpy(&translation, constData + offsetToTranslation, 2);
			} else if(frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.tz * 2;

				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}

				memcpy(&translation, constData + offsetToTranslation, 2);
			}
			trans.z = -translation / MODEL_SCALE_PS;

			rotation_coords.append(rot);
			rotation_coordsTrans.append(trans);
		}

		_frames.insert(frame, rotation_coords);
		_framesTrans.insert(frame, rotation_coordsTrans);
	}

	return true;
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
