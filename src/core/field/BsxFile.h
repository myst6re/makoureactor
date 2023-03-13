/****************************************************************************
 ** Néo-Midgar Final Fantasy VII French Retranslation
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
#pragma once

#include <QtCore>
#include <IO>
#include "FieldModelFilePS.h"
#include "FieldModelTextureRefPS.h"
#include "CaFile.h"

#define MODEL_SCALE_PS        4096.0f // 31.0f
#define COLORRGB_2_QRGB(c)    qRgb(c.red, c.green, c.blue)
#define QRGB_2_COLORRGB(c, r) r.red = qRed(c);r.green = qGreen(c);r.blue = qBlue(c)

struct BsxModelsHeader {
	quint32 psxMemory;
	quint32 numModels;
	quint32 texturePointer;
	quint32 bufferSize;
};

struct ColorRGB {
	quint8 red, green, blue;
};

struct BsxModelHeader {
	quint16 modelId;        // ID of the model
	quint16 scale;
	quint32 offsetSkeleton; // Offset to the parts, bones and animations of the model
	ColorRGB globalColor;
	quint8 unknown;
	qint16 colorADir1;
	qint16 colorADir2;
	qint16 colorADir3;
	quint8 indexBonesStart;
	quint8 indexBonesEnd;
	ColorRGB colorA;
	quint8 numBones;      // Number of bones in the model's skeleton
	qint16 colorBDir1;
	qint16 colorBDir2;
	qint16 colorBDir3;
	quint8 indexMeshStart;
	quint8 indexMeshEnd;
	ColorRGB colorB;
	quint8 numParts;      // Number of parts in the model's skeleton
	qint16 colorCDir1;
	qint16 colorCDir2;
	qint16 colorCDir3;
	quint8 indexAnimationsStart;
	quint8 indexAnimationsEnd;
	ColorRGB colorC;
	quint8 numAnimations; // Number of animations
};

struct FieldModelPartPSHeader {
	quint8 unknown;         // 0 - not calculate stage lighting and color. 1 - calculate.
	qint8 boneIndex;        // bone to which this part attached to.
	quint8 numVertices;     // Number of vertices
	quint8 numTexCs;        // Number of Texture coord
	quint8 numQuadColorTex; // number of textured quads (Gourad Shading)
	quint8 numTriColorTex;  // number of textured triangles (Gourad Shading)
	quint8 numQuadMonoTex;  // number of textured quads (Flat Shading)
	quint8 numTriMonoTex;   // number of textured triangles (Flat Shading)
	quint8 numTriMono;      // number of monochrome triangles
	quint8 numQuadMono;     // number of monochrome quads
	quint8 numTriColor;     // number of gradated triangles
	quint8 numQuadColor;    // number of gradated quads
	quint8 numFlags;        // number of data in block 4 (flags).
	quint8 numControl;      // number of data in block 5 (control).
	quint16 offsetPoly;     // Relative offset to ?
	quint16 offsetTexcoord; // Relative offset to ?
	quint16 offsetFlags;    // Relative offset to texture settings. Indexed by 5th block data (control).
	quint16 offsetControl;  // Relative offset to one byte stream for every packet with texture.
	quint16 bufferSize;     // Relative offset to ?
	quint32 offsetVertex;   // Offset to skeleton data section
	quint32 offsetPrec;     // Offset to ?
};

struct FieldModelAnimationPSHeader {
	quint16 numFrames;               // Number of frames
	quint8 numBones;                 // Number of bones
	quint8 numFramesTranslation;     // Number of translation frames
	quint8 numStaticTranslation;     // Number of static translation frames
	quint8 numFramesRotation;        // Number of rotation frames
	quint16 offsetFramesTranslation; // Relative offset to translation frames
	quint16 offsetStaticTranslation; // Relative offset to statis translation frames
	quint16 offsetFramesRotation;    // Relative offset to rotation frames
	quint32 offsetData;              // Offset to animation data section
};

struct FrameTranslation {
	quint8 flag;
	quint8 rx, ry, rz;
	quint8 tx, ty, tz;
	quint8 padding;
};

struct FieldModelBonePS {
	qint16 length;
	qint8 parent;
	quint8 unknown;
};

struct FieldModelVertexPS {
	Vertex_s v;
	quint16 padding;
};

struct ColorRGBA {
	quint8 red, green, blue, alpha;
};

struct TexturedQuad {
	quint8 vertexIndex[4];
	ColorRGBA color[4];
	quint8 texCoordId[4];
};

struct TexturedTriangle {
	quint8 vertexIndex[3];
	quint8 padding1;
	ColorRGBA color[3];
	quint8 texCoordId[3];
	quint8 padding2;
};

struct MonochromeTexturedQuad {
	quint8 vertexIndex[4];
	ColorRGBA color;
	quint8 texCoordId[4];
};

struct MonochromeTexturedTriangle {
	quint8 vertexIndex[3];
	quint8 padding1;
	ColorRGBA color;
	quint8 texCoordId[3];
	quint8 padding2;
};

struct MonochromeTriangle {
	quint8 vertexIndex[3];
	quint8 padding;
	ColorRGBA color;
};

struct MonochromeQuad {
	quint8 vertexIndex[4];
	ColorRGBA color;
};

struct ColorTriangle {
	quint8 vertexIndex[3];
	quint8 padding;
	ColorRGBA color[3];
};

struct ColorQuad {
	quint8 vertexIndex[4];
	ColorRGBA color[4];
};

struct BsxTexturesHeader {
	quint32 textureSectionSize;
	quint32 numTextures;
};

struct BsxTextureHeader {
	quint16 width, height;
	quint16 vramX, vramY;
	quint32 offsetData;
};

class BsxFile : public IO
{
public:
	explicit BsxFile(QIODevice *io);
	virtual ~BsxFile() override;

	virtual bool read(QList<FieldModelFilePS *> &models);
	virtual bool read(FieldModelFilePS *model) const;
	virtual bool readTextures(FieldModelTexturesPS *textures) const;
	virtual bool seek(quint32 modelId);
	virtual bool seekModels();
	virtual bool seekTextures();
	virtual bool write(const QList<FieldModelFilePS> &models) const;
	virtual bool writeModelHeader(const FieldModelFilePS &model) const;
protected:
	bool readHeader();
	bool readModelsHeader();
	bool readModel(quint8 numBones, quint8 numParts, quint8 numAnimations, FieldModelFilePS *model) const;
	bool readSkeleton(quint8 numBones, FieldModelSkeleton &skeleton) const;
	bool readPartsHeaders(quint8 numParts, QList<FieldModelPartPSHeader> &partsHeaders) const;
	bool readAnimationsHeaders(quint8 numAnimations, QList<FieldModelAnimationPSHeader> &animationsHeaders) const;
	bool readMesh(const QList<FieldModelPartPSHeader> &partsHeaders, FieldModelSkeleton &skeleton) const;
	bool readPart(const FieldModelPartPSHeader &partHeader, FieldModelPart *part) const;
	static bool addTexturedPolygonToGroup(quint8 control, Poly *polygon, QList<FieldModelGroup *> &groups);
	bool readAnimations(const QList<FieldModelAnimationPSHeader> &animationHeaders, QList<FieldModelAnimation> &animations) const;
	bool readAnimation(const FieldModelAnimationPSHeader &header, FieldModelAnimation &animation) const;
	bool readTexturesHeaders(quint8 numTextures, QList<BsxTextureHeader> &headers) const;
	bool readTexturesData(const QList<BsxTextureHeader> &headers, QList<QByteArray> &dataList) const;
private:
	qint64 _offsetModels;
	quint32 _numModels, _offsetTextures;
};
