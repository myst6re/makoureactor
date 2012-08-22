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

FieldModelFilePS::FieldModelFilePS() :
	FieldModelFile()
{
}

quint8 FieldModelFilePS::load(FieldArchive *fieldArchive, Field *currentField, int model_id, int animation_id, bool animate)
{
	QByteArray BSX_data = fieldArchive->getModelData(currentField);
	FieldModelLoaderPS *modelLoader = currentField->getFieldModelLoaderPS();
	quint8 model_global_id = model_id < modelLoader->modelCount() ? modelLoader->model(model_id).modelID : 0;
	const char *constData = BSX_data.constData();
	BSX_header header;
	Model_header model_header;
	Model model;

	if((quint32)BSX_data.size() < sizeof(BSX_header)) {
		qWarning() << "invalid BSX size" << BSX_data.size();
		return 2;
	}

	memcpy(&header, constData, sizeof(BSX_header));
//	qDebug() << "==== BSX HEADER ====" << 0 << sizeof(BSX_header);
//	qDebug() << "size" << header.size << "offset_models" << header.offset_models;

	if((quint32)BSX_data.size() != header.size || header.offset_models >= header.size) {
		qWarning() << BSX_data.size() << "error";
		return 2;
	}

	memcpy(&model_header, &constData[header.offset_models], sizeof(Model_header));

//	qDebug() << "==== Model header ====" << header.offset_models << sizeof(Model_header);
//	qDebug() << "num_models" << model_header.num_models << "vram" << QString::number(model_header.psx_memory, 16);
//	qDebug() << "texPointer" << model_header.texture_pointer << "unknownPointer" << model_header.unknown_pointer;

	if(model_id >= (int)model_header.num_models) {
		qWarning() << "model_id too large" << model_id << model_header.num_models;
		return 2;
	}

//	for(model_id=0 ; (quint32)model_id<model_header.num_models ; ++model_id) {

		quint32 offsetModelHeader = header.offset_models + sizeof(Model_header) + model_id*sizeof(Model);
		memcpy(&model, &constData[offsetModelHeader], sizeof(Model));

		qDebug() << "==== Model" << model_id << "====" << offsetModelHeader << sizeof(Model) << model_header.num_models*sizeof(Model);
		qDebug() << "modelID" << model.model_id << "modelScale" << model.scale << "offsetSkeleton" << model.offset_skeleton;
		qDebug() << "r" << model.r1 << "g" << model.g1 << "b" << model.b1;
		qDebug() << "???" << model.unknown1;
		qDebug() << "???" << model.unknown2[0] << model.unknown2[1] << model.unknown2[2] << model.unknown2[3] << model.unknown2[4] << model.unknown2[5];
		qDebug() << "???" << model.unknown3;
		qDebug() << "r" << model.r2 << "g" << model.g2 << "b" << model.b2;
		qDebug() << "bones index" << model.index_bones << "count" << model.num_bones;
		qDebug() << "???" << model.unknown4[0] << model.unknown4[1] << model.unknown4[2] << model.unknown4[3] << model.unknown4[4] << model.unknown4[5];
		qDebug() << "???" << model.unknown5;
		qDebug() << "r" << model.r3 << "g" << model.g3 << "b" << model.b3;
		qDebug() << "parts index" << model.index_parts << "count" << model.num_parts;
		qDebug() << "???" << model.unknown6[0] << model.unknown6[1] << model.unknown6[2] << model.unknown6[3] << model.unknown6[4] << model.unknown6[5];
		qDebug() << "???" << model.unknown7;
		qDebug() << "r" << model.r4 << "g" << model.g4 << "b" << model.b4;
		qDebug() << "animations index" << model.index_animations << "count" << model.num_animations;

		int curOff = offsetModelHeader + model.offset_skeleton;

//		qDebug() << "after model header" << (offsetModelHeader + sizeof(Model)) << "skeleton offset" << curOff;

		if(model_global_id >= 1 && model_global_id <= 9) {
			QString fileName;
			switch(model_global_id) {
			case 1:
				fileName = "CLOUD";
				break;
			case 2:
				fileName = "EARITH";
				break;
			case 3:
				fileName = "BALLET";
				break;
			case 4:
				fileName = "TIFA";
				break;
			case 5:
				fileName = "RED";
				break;
			case 6:
				fileName = "CID";
				break;
			case 7:
				fileName = "YUFI";
				break;
			case 8:
				fileName = "KETCY";
				break;
			case 9:
				fileName = "VINCENT";
				break;
			default:
				break;
			}
			if(!openBCX(fieldArchive->getFileData(fileName + ".BCX"), animation_id, animate)) {
				return false;
			}

			if(!_frames.isEmpty()) {
				animation_id -= 3;
			}
		}

		if(curOff + model.num_bones * sizeof(BonePS) >= (quint32)BSX_data.size()) {
			qWarning() << "invalid skeleton size" << model.num_bones;
			return false;
		}

		curOff = openSkeleton(constData, curOff, BSX_data.size(), model.num_bones);

//		qDebug() << "after bones position" << curOff;

		if(curOff + model.num_parts * sizeof(Part) >= (quint32)BSX_data.size()) {
			qWarning() << "invalid parts size" << model.num_parts;
			return false;
		}

		curOff = openMesh(constData, curOff, BSX_data.size(), model.num_parts);

//		qDebug() << "after part headers position" << curOff;

		if(_frames.isEmpty()) {
			if(animation_id >= model.num_animations) {
				qWarning() << "No animation called" << animation_id;
				return false;
			}

//			for(animation_id=0 ; animation_id < model.num_animations ; ++animation_id) {
				openAnimation(constData, curOff, animation_id, BSX_data.size(), animate);
//			}

//			qDebug() << "after animation headers position" << (curOff + sizeof(Animation) * model.num_animations);
		}
//	}

	quint32 offsetTexHeader = header.offset_models + model_header.texture_pointer;

	quint32 unknownTex, unknownTex2;
	memcpy(&unknownTex, &constData[offsetTexHeader], 4);
	memcpy(&unknownTex2, &constData[offsetTexHeader+4], 4);
	quint8 texCount = unknownTex2 & 0xFF;

	qDebug() << "==== TEXTURES HEADER ====" << offsetTexHeader;
	qDebug() << "texCount" << texCount << "unknown1" << unknownTex << "unknown2" << (unknownTex2 & 0xFFFFFF00);

	for(int tex=0 ; tex<texCount ; ++tex) {
		TexHeader texHeader;
		memcpy(&texHeader, &constData[offsetTexHeader + 8 + tex*sizeof(TexHeader)], sizeof(TexHeader));

		texHeader.offset_data += offsetTexHeader;

		qDebug() << "==== TEXTURE HEADER" << tex << "====" << (offsetTexHeader + 8 + tex*sizeof(TexHeader));
		qDebug() << "width" << texHeader.width << "height" << texHeader.height << "x" << texHeader.vramX << "y" << texHeader.vramY;
		qDebug() << "offsetData" << texHeader.offset_data << "endData" << (texHeader.offset_data + texHeader.width * 2 * texHeader.height);

//		QImage img(texHeader.width * 2, texHeader.height, QImage::Format_RGB32);
//		QRgb *px = (QRgb *)img.bits();
//		int i=0;
//		for(int y=0 ; y<texHeader.height ; ++y) {
//			for(int x=0 ; x<texHeader.width*2 ; ++x) {
//				px[i] = qRgb(constData[texHeader.offset_data + i], constData[texHeader.offset_data + i], constData[texHeader.offset_data + i]);
//				++i;
//			}
//		}
//		img.save(QString("FieldModelFilePSTex%1.png").arg(tex));
	}

	if(model_id < modelLoader->modelCount()) {
		TdbFile tdb;
		tdb.open(fieldArchive->getFileData("FIELD.TDB"));

		quint8 faceID = modelLoader->model(model_id).faceID;
		QMap<TextureInfo, int> texAlreadyLoaded;

		if(faceID < 0x21) {
			int texID=0;

			foreach(FieldModelPart *part, _parts) {
				QList<int> texIds;

				foreach(const TextureInfo &texInfo, ((FieldModelPartPS *)part)->textures()) {

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
						texIds.append(texID);
						texAlreadyLoaded.insert(texInfo, texID);

						if(texInfo.type == 0) { // Eye
							qDebug() << "EYE" << texID;
							_loaded_tex.insert(texID++, tdb.texture(faceID, TdbFile::Eye));
						} else if(texInfo.type == 1) { // Mouth
							qDebug() << "MOUTH" << texID;
							_loaded_tex.insert(texID++, tdb.texture(faceID, TdbFile::MouthClosed));
						} else {
							qDebug() << "OTHER" << texID;
							_loaded_tex.insert(texID++, QPixmap());
						}
					}
				}

				if(!texIds.isEmpty())
					_tex_files.insert(((FieldModelPartPS *)part)->boneID(), texIds);
			}
		}
	}

	dataLoaded = true;

	return true;
}

int FieldModelFilePS::openSkeleton(const char *constData, int curOff, int size, quint8 numBones)
{
	for(quint32 i=0 ; i<numBones ; ++i) {
		BonePS bonePS;
		Bone bone;
		memcpy(&bonePS, &constData[curOff], sizeof(BonePS));
//		qDebug() << "bone" << i << bonePS.length << bonePS.parent << bonePS.unknown;
		bone.size = bonePS.length / 31.0f;
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
		qDebug() << "==== PART" << i << "====" << curOff;
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
	memcpy(&a, &constData[curOff + sizeof(Animation)*animation_id], sizeof(Animation));

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

	quint32 unknown;
	memcpy(&unknown, &constData[a.offset_data], 4);

//	qDebug() << "Unknown" << unknown << QString::number(unknown, 16);

	this->a_bones_count = qMin((int)a.num_bones, _bones.size());

	quint16 numFrames2 = animate ? a.num_frames : qMin((quint16)1, a.num_frames);

	quint32 maxOffset = a.offset_data + 4 + a.num_bones*8;

	for(int frame=0 ; frame<numFrames2 ; ++frame) {
		QList<PolyVertex> rotation_coords;

		for(int bone=0 ; bone<a.num_bones ; ++bone) {
			FrameTranslation frameTrans;
			PolyVertex rot;

			memcpy(&frameTrans, &constData[a.offset_data + 4 + bone*8], sizeof(FrameTranslation));

			// Rotation

			if(frameTrans.flag & 0x01) {
				quint32 offsetToRotation = offsetFrameRotation + frameTrans.rx * a.num_frames + frame;
				if(maxOffset < offsetToRotation + 1) {
					maxOffset = offsetToRotation + 1;
				}
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
				if(maxOffset < offsetToRotation + 1) {
					maxOffset = offsetToRotation + 1;
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
				if(maxOffset < offsetToRotation + 1) {
					maxOffset = offsetToRotation + 1;
				}
				rot.z = 360.0f * (quint8)constData[offsetToRotation] / 255.0f;
			} else {
				rot.z = 360.0f * frameTrans.rz / 255.0f;
			}

			// (translation)

			qint16 translation;

			if(frameTrans.flag & 0x10) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.tx * a.num_frames * 2 + frame * 2;
				if(maxOffset < offsetToTranslation + 2) {
					maxOffset = offsetToTranslation + 2;
				}
				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}
				memcpy(&translation, &constData[offsetToTranslation], 2);
			} else if(frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.tx * 2;
				if(maxOffset < offsetToTranslation + 2) {
					maxOffset = offsetToTranslation + 2;
				}
				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}
				memcpy(&translation, &constData[offsetToTranslation], 2);
			}

			if(frameTrans.flag & 0x20) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.ty * a.num_frames * 2 + frame * 2;
				if(maxOffset < offsetToTranslation + 2) {
					maxOffset = offsetToTranslation + 2;
				}
				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}
				memcpy(&translation, &constData[offsetToTranslation], 2);
			} else if(frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.ty * 2;
				if(maxOffset < offsetToTranslation + 2) {
					maxOffset = offsetToTranslation + 2;
				}
				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}
				memcpy(&translation, &constData[offsetToTranslation], 2);
			}

			if(frameTrans.flag & 0x40) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.tz * a.num_frames * 2 + frame * 2;
				if(maxOffset < offsetToTranslation + 2) {
					maxOffset = offsetToTranslation + 2;
				}
				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}
				memcpy(&translation, &constData[offsetToTranslation], 2);
			} else if(frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.tz * 2;
				if(maxOffset < offsetToTranslation + 2) {
					maxOffset = offsetToTranslation + 2;
				}
				if(offsetToTranslation >= (quint32)size) {
					qWarning() << "offsetToTranslation too large" << offsetToTranslation << size;
					continue;
				}
				memcpy(&translation, &constData[offsetToTranslation], 2);
			}

			rotation_coords.append(rot);
		}

		_frames.insert(frame, rotation_coords);
	}

//	qDebug() << "maxOffset animation" << maxOffset;

	return true;
}

bool FieldModelFilePS::openBCX(const QByteArray &BCX, int animationID, bool animate)
{
	BSX_header header;
	BCXModel model;
	const char *constData = BCX.constData();

	if((quint32)BCX.size() < sizeof(BSX_header)) {
		qWarning() << "invalid BSX size" << BCX.size();
		return false;
	}

	memcpy(&header, constData, sizeof(BSX_header));
	qDebug() << "==== BCX HEADER ====" << 0 << sizeof(BSX_header);
	qDebug() << "size" << header.size << "offset_models" << header.offset_models;

	if((quint32)BCX.size() != header.size || header.offset_models >= header.size) {
		qWarning() << BCX.size() << "error";
		return false;
	}

	memcpy(&model, &constData[header.offset_models], sizeof(BCXModel));

	model.offset_skeleton -= 0x80000000;

	qDebug() << "==== Model ====" << header.offset_models << sizeof(BCXModel) << (header.offset_models+sizeof(BCXModel));
	qDebug() << "unknown" << model.u0;
	qDebug() << "bones count" << model.num_bones;
	qDebug() << "parts count" << model.num_parts;
	qDebug() << "animations count" << model.num_animations;
	qDebug() << "unknown" << QByteArray((char *)&model.u1, 17).toHex();
	qDebug() << "scale" << model.scale;
	qDebug() << "offsetParts" << (header.offset_models + sizeof(BCXModel) + model.offset_parts)
				<< "offsetAnimations" << (header.offset_models + sizeof(BCXModel) + model.offset_animations);
	qDebug() << "offsetSkeleton" << model.offset_skeleton;
	qDebug() << "unknown" << model.u2 << QString::number(model.u2, 16);

	quint32 curOff = model.offset_skeleton;

	if(curOff + model.num_bones * sizeof(BonePS) >= (quint32)BCX.size()) {
		qWarning() << "invalid skeleton size" << model.num_bones;
		return false;
	}

	curOff = openSkeleton(constData, curOff, BCX.size(), model.num_bones);

	qDebug() << "after bones position" << curOff;

	if(curOff + model.num_parts * sizeof(Part) >= (quint32)BCX.size()) {
		qWarning() << "invalid parts size" << model.num_parts;
		return false;
	}

	curOff = openMesh(constData, curOff, BCX.size(), model.num_parts);

	qDebug() << "after part headers position" << curOff;

	if(animationID < model.num_animations) {
		openAnimation(constData, curOff, animationID, BCX.size(), animate);
	}

	qDebug() << "after animation headers position" << (curOff + sizeof(Animation) * model.num_animations);

	qDebug() << "rest of the file" << BCX.mid(curOff + sizeof(Animation) * model.num_animations).toHex();

	return true;
}
