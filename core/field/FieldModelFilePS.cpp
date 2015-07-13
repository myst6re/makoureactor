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
#include "TdbFile.h"
#include "../PsColor.h"
#include "FieldPS.h"

FieldModelFilePS::FieldModelFilePS() :
	FieldModelFile()
{
}

quint8 FieldModelFilePS::load(FieldPS *currentField, int modelId, int animationId, bool animate)
{
	/*** Open textures header ***/

	/* quint32 offsetTexHeader = header.offset_models + model_header.texture_pointer;

	if(offsetTexHeader + 8 > (quint32)BSX_data.size()) {
		qWarning() << BSX_data.size() << "error tex header";
		return 2;
	}

	quint32 unknownTex, unknownTex2;
	memcpy(&unknownTex, constData + offsetTexHeader, 4);
	memcpy(&unknownTex2, constData + offsetTexHeader+4, 4);
	quint8 texCount = unknownTex2 & 0xFF; */

//	qDebug() << "==== TEXTURES HEADER ====" << offsetTexHeader;
//	qDebug() << "texCount" << texCount << "unknown1" << unknownTex << "unknown2" << (unknownTex2 & 0xFFFFFF00);

	/*** Open texture Headers ***/

	/* if(offsetTexHeader + 8 + texCount*sizeof(TexHeader) > (quint32)BSX_data.size()) {
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
	} */

	/*
	// DEBUG
	for(int tex=0 ; tex<texHeaders.size() ; ++tex) {
		if(tex==1)	continue;
		openTexture(constData, 0, texHeaders.at(tex), texHeaders.at(1), 1).save(QString("Makou-Texture%1.png").arg(tex));
	}*/

	/* if(model_id < modelLoader->modelCount()) {
		TdbFile tdb;
		quint8 faceID = modelLoader->model(model_id).faceID;
		QMap<TextureInfo, int> texAlreadyLoaded;

		if(faceID < 0x21) {
			tdb.open(currentField->io()->fileData("FIELD.TDB"));
		}

		int texID=0;

		foreach(const FieldModelBone &bone, _skeleton.bones()) {
			foreach(FieldModelPart *part, bone.parts()) {
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
							_loadedTex.insert(texID++, tdb.texture(faceID, texInfo.type == 0 ? TdbFile::Eye : TdbFile::MouthClosed));
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
								_loadedTex.insert(texID++, openTexture(constData, BSX_data.size(), imgHeader, palHeader, texInfo.bpp));
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
							const QImage currentTex = _loadedTex.value(group->textureNumber());
							int texWidth=currentTex.width(), texHeight=currentTex.height();

							foreach(Poly *poly, group->polygons()) {
								poly->divTexCoords(texWidth, texHeight);
							}
						}
					}
				}
			}
		}
	}

	dataLoaded = true; */

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
