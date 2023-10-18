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
#include "BsxFile.h"

BsxFile::BsxFile(QIODevice *io) :
	IO(io), _offsetModels(0),
	_numModels(0), _offsetTextures(0)
{
}

BsxFile::~BsxFile()
{
}

bool BsxFile::read(QList<FieldModelFilePS *> &models)
{
	// Implicit seek
	if (!readHeader()) {
		return false;
	}

	// Implicit seek
	if (!readModelsHeader()) {
		return false;
	}

	for (quint64 modelID = 0; modelID < _numModels; ++modelID) {
		if (!seek(modelID)) {
			return false;
		}

		FieldModelFilePS *model = new FieldModelFilePS();
		if (!read(model)) {
			delete model;
			return false;
		}

		models.append(model);
	}

	// Textures
	if (!seekTextures()) {
		qWarning() << "BsxFile::read error 2";
		return false;
	}

	return true;
}

bool BsxFile::read(FieldModelFilePS *model) const
{
	BsxModelHeader modelHeader;
	qint64 pos = device()->pos();

	if (sizeof(BsxModelHeader) != device()->read((char *)&modelHeader, sizeof(BsxModelHeader))) {
		qWarning() << "BsxFile::read model error 1";
		return false;
	}

	// We put colors in the same order that the PC version
	QList<FieldModelColorDir> colors;
	colors << FieldModelColorDir(modelHeader.colorADir1, modelHeader.colorADir2,
	                             modelHeader.colorADir3,
	                             COLORRGB_2_QRGB(modelHeader.colorA))
	       << FieldModelColorDir(modelHeader.colorBDir1, modelHeader.colorBDir2,
	                             modelHeader.colorBDir3,
	                             COLORRGB_2_QRGB(modelHeader.colorB))
	       << FieldModelColorDir(modelHeader.colorCDir1, modelHeader.colorCDir2,
	                             modelHeader.colorCDir3,
	                             COLORRGB_2_QRGB(modelHeader.colorC));
	model->setLightColors(colors);
	model->setGlobalColor(COLORRGB_2_QRGB(modelHeader.globalColor));
	model->setScale(modelHeader.scale);

	// Relative to model header
	if (!device()->seek(pos + modelHeader.offsetSkeleton)) {
		qWarning() << "BsxFile::read model error 2";
		return false;
	}

	return readModel(modelHeader.numBones,
					 modelHeader.numParts,
					 modelHeader.numAnimations,
					 model);
}

bool BsxFile::readModel(quint8 numBones, quint8 numParts, quint8 numAnimations, FieldModelFilePS *model) const
{
	FieldModelSkeleton skeleton;
	QList<FieldModelPartPSHeader> partsHeaders;
	QList<FieldModelAnimationPSHeader> animationsHeaders;
	QList<FieldModelAnimation> animations;

	if (!readSkeleton(numBones, skeleton)) {
		return false;
	}

	if (!readPartsHeaders(numParts, partsHeaders)) {
		return false;
	}

	if (!readAnimationsHeaders(numAnimations, animationsHeaders)) {
		return false;
	}

	// Warning: Seek to another section of the device

	if (!readMesh(partsHeaders, skeleton)) {
		return false;
	}

	if (!readAnimations(animationsHeaders, animations)) {
		return false;
	}

	model->setSkeleton(skeleton);
	model->setAnimations(animations);

	return true;
}

bool BsxFile::readTextures(FieldModelTexturesPS *textures) const
{
	BsxTexturesHeader texturesHeader;

	if (sizeof(BsxTexturesHeader) != device()->read((char *)&texturesHeader, sizeof(BsxTexturesHeader))) {
		qWarning() << "BsxFile::read error 3";
		return false;
	}

	if (texturesHeader.textureSectionSize != device()->size() - device()->pos() + sizeof(BsxTexturesHeader)) {
		qWarning() << "BsxFile::readTextures textures header inconsistency" << texturesHeader.textureSectionSize << device()->pos() << device()->size();
	}

	QList<BsxTextureHeader> headers;

	if (!readTexturesHeaders(texturesHeader.numTextures, headers)) {
		return false;
	}

	QList<QByteArray> dataList;

	if (!readTexturesData(headers, dataList)) {
		return false;
	}

	QList<QRect> rects;

	for (const BsxTextureHeader &h : std::as_const(headers)) {
		rects.append(QRect(h.vramX, h.vramY,
						   h.width, h.height));
	}

	textures->setRects(rects);
	textures->setData(dataList);

	return true;
}

bool BsxFile::seekModels()
{
	// Get offset models (Implicit seek)
	if (!readHeader()) {
		return false;
	}

	return device()->seek(_offsetModels);
}

bool BsxFile::seekTextures()
{
	// Get offset textures (Implicit seek)
	if (!readModelsHeader()) {
		return false;
	}

	return device()->seek(_offsetModels + _offsetTextures);
}

bool BsxFile::seek(quint32 modelId)
{
	// Get offset models
	if (!readHeader()) {
		return false;
	}

	return device()->seek(_offsetModels + sizeof(BsxModelsHeader) + modelId * sizeof(BsxModelHeader));
}

bool BsxFile::readHeader()
{
	if (_offsetModels > 0) {
		return true;
	}

	if (!device()->seek(4)) {
		qWarning() << "BsxFile::readHeader error 1";
		return false;
	}
	if (4 != device()->read((char *)&_offsetModels, 4)) {
		qWarning() << "BsxFile::readHeader error 2";
		return false;
	}

	return true;
}

bool BsxFile::readModelsHeader()
{
	if (_offsetTextures > 0) {
		return true;
	}

	if (!seekModels()) {
		return false;
	}

	BsxModelsHeader modelsHeader;

	if (sizeof(BsxModelsHeader) != device()->read((char *)&modelsHeader, sizeof(BsxModelsHeader))) {
		qWarning() << "BsxFile::readModelsHeader error";
		return false;
	}

	_numModels = modelsHeader.numModels;
	_offsetTextures = modelsHeader.texturePointer;

	return true;
}

bool BsxFile::readSkeleton(quint8 numBones, FieldModelSkeleton &skeleton) const
{
	for (quint16 i = 0; i < numBones; ++i) {
		FieldModelBonePS bone;
		if (sizeof(FieldModelBonePS) != device()->read((char *)&bone, sizeof(FieldModelBonePS))) {
			qWarning() << "BsxFile::readSkeleton error" << i;
			return false;
		}
		if (bone.parent >= numBones) {
			qWarning() << "BsxFile::readSkeleton error invalid parent" << i << bone.parent << numBones;
			return false;
		}
		skeleton.addBone(FieldModelBone(bone.length / MODEL_SCALE_PS, bone.parent));
	}

	return true;
}

bool BsxFile::readPartsHeaders(quint8 numParts, QList<FieldModelPartPSHeader> &partsHeaders) const
{
	for (quint16 i = 0; i < numParts; ++i) {
		FieldModelPartPSHeader partHeader;
		if (sizeof(FieldModelPartPSHeader) != device()->read((char *)&partHeader, sizeof(FieldModelPartPSHeader))) {
			qWarning() << "BsxFile::readPartsHeaders error" << i;
			return false;
		}
		partsHeaders.append(partHeader);
	}

	return true;
}

bool BsxFile::readAnimationsHeaders(quint8 numAnimations, QList<FieldModelAnimationPSHeader> &animationsHeaders) const
{
	for (quint16 i = 0; i < numAnimations; ++i) {
		FieldModelAnimationPSHeader animationHeader;
		if (sizeof(FieldModelAnimationPSHeader) != device()->read((char *)&animationHeader, sizeof(FieldModelAnimationPSHeader))) {
			qWarning() << "BsxFile::readAnimationsHeaders error" << i;
			return false;
		}
		animationsHeaders.append(animationHeader);
	}

	return true;
}

bool BsxFile::readMesh(const QList<FieldModelPartPSHeader> &partsHeaders, FieldModelSkeleton &skeleton) const
{
	for (const FieldModelPartPSHeader &header : partsHeaders) {
		FieldModelPart *part = new FieldModelPart();

		if (!readPart(header, part)) {
			delete part;
			return false;
		}

		skeleton[header.boneIndex].addPart(part);
	}

	return true;
}

bool BsxFile::readPart(const FieldModelPartPSHeader &partHeader, FieldModelPart *part) const
{
	QList<FieldModelGroup *> groups;
	QList<PolyVertex> vertices;
	QList<TexCoord> texCoords;

	if (partHeader.numControl != partHeader.numQuadColorTex
			+ partHeader.numQuadMonoTex
			+ partHeader.numTriColorTex
			+ partHeader.numTriMonoTex) {
		qWarning() << "BsxFile::readPart num control error" << partHeader.numControl << partHeader.numQuadColorTex << partHeader.numQuadMonoTex << partHeader.numTriColorTex << partHeader.numTriMonoTex;
		return false;
	}

	// Vertices
	quint32 offsetToVertex = partHeader.offsetVertex & 0x7FFFFFFF;

	// Note: there are four empty bytes here
	if (!device()->seek(offsetToVertex + 4)) {
		qWarning() << "BsxFile::readPart error 1" << QString::number(partHeader.offsetVertex, 16);
		return false;
	}

	for (quint16 i = 0; i < partHeader.numVertices; ++i) {
		FieldModelVertexPS v;

		if (sizeof(FieldModelVertexPS) != device()->read((char *)&v, sizeof(FieldModelVertexPS))) {
			qWarning() << "BsxFile::readPart error 2" << i;
			return false;
		}

		PolyVertex vertex;

		vertex.x = v.v.x / MODEL_SCALE_PS;
		vertex.y = v.v.y / MODEL_SCALE_PS;
		vertex.z = v.v.z / MODEL_SCALE_PS;

		vertices.append(vertex);
	}

	// Texture informations
	quint32 offsetToTexInfos = offsetToVertex + partHeader.offsetFlags;

	if (!device()->seek(offsetToTexInfos)) {
		qWarning() << "BsxFile::readPart error 6";
		return false;
	}

	groups.append(new FieldModelGroup()); // group without textured polygons

	for (quint16 i = 0; i < partHeader.numFlags; ++i) {
		quint32 flag;
		if (4 != device()->read((char *)&flag, 4)) {
			qWarning() << "BsxFile::readPart error 7" << i;
			return false;
		}

		FieldModelTextureRefPS *texRef = new FieldModelTextureRefPS();

		texRef->setType(flag & 0x3F);                           // 0000 0000 0000 0000 0000 0000 0011 1111
		texRef->setBpp((flag >> 6) & 0x03);                     // 0000 0000 0000 0000 0000 0000 1100 0000
		texRef->setImgX(/*512 + */((flag >> 8) & 0x0f) * 64);   // 0000 0000 0000 0000 0000 1111 0000 0000
		texRef->setImgY(/*256 + */((flag >> 12) & 0x01) * 256); // 0000 0000 0000 0000 0001 0000 0000 0000
//		quint8 u1 = ((flag >> 13) & 0x07);
		texRef->setPalX(/*512 + */((flag >> 16) & 0x3F) * 16);  // 0000 0000 0011 1111 0000 0000 0000 0000
		texRef->setPalY(/*256 + */((flag >> 22) & 0x01FF));     // 0111 1111 1100 0000 0000 0000 0000 0000
//		quint8 u2 = ((flag >> 31) & 0x01);

		groups.append(new FieldModelGroup(texRef));
	}

	// Texture coordinates
	quint32 offsetToTexCoords = offsetToVertex + partHeader.offsetTexcoord;

	if (!device()->seek(offsetToTexCoords)) {
		qWarning() << "BsxFile::readPart error 3";
		return false;
	}

	for (quint16 i = 0; i < partHeader.numTexCs; ++i) {
		TexCoord texCoord;
		char c;

		if (!device()->getChar(&c)) {
			qWarning() << "BsxFile::readPart error 4" << i;
			return false;
		}
		texCoord.x = uchar(c);
		if (!device()->getChar(&c)) {
			qWarning() << "BsxFile::readPart error 5" << i;
			return false;
		}
		texCoord.y = uchar(c);

		texCoords.append(texCoord);
	}

	// Polygons
	const quint32 offsetToPoly = offsetToVertex + partHeader.offsetPoly;
	const quint32 offsetToControl = offsetToVertex + partHeader.offsetControl;

	if (!device()->seek(offsetToControl)) {
		qWarning() << "BsxFile::readPart error 8";
		return false;
	}
	QByteArray controlData = device()->read(partHeader.numControl);
	if (controlData.size() != partHeader.numControl) {
		qWarning() << "BsxFile::readPart error 9";
		return false;
	}

	bool notAdd;

	if (!device()->seek(offsetToPoly)) {
		qWarning() << "BsxFile::readPart error 10";
		return false;
	}
	int offsetControl = 0;

	//--> Textured Quads

	for (quint16 i = 0; i < partHeader.numQuadColorTex; ++i) {
		TexturedQuad texturedQuad;
		if (sizeof(TexturedQuad) != device()->read((char *)&texturedQuad, sizeof(TexturedQuad))) {
			qWarning() << "BsxFile::readPart error 11" << i;
			return false;
		}

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for (quint8 j = 0; j < 4; ++j) {
			quint8 vertexIndex = texturedQuad.vertexIndex[j];
			quint8 texCoordIndex = texturedQuad.texCoordId[j];
			if (vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				const ColorRGBA &color = texturedQuad.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qWarning() << "BsxFile::readPart error index col tex quad" << i << j << vertexIndex << vertices.size() << texCoordIndex << texCoords.size();
				break;
			}
		}

		if (!notAdd) {
			QuadPoly *poly = new QuadPoly(polyVertices, polyColors, polyTexCoords);
			if (!addTexturedPolygonToGroup(controlData.at(offsetControl++), poly, groups)) {
				delete poly;
				return false;
			}
		}
	}

	//--> Textured Triangles

	for (quint16 i = 0; i < partHeader.numTriColorTex; ++i) {
		TexturedTriangle texturedTriangle;
		if (sizeof(TexturedTriangle) != device()->read((char *)&texturedTriangle, sizeof(TexturedTriangle))) {
			qWarning() << "BsxFile::readPart error 13" << i;
			return false;
		}

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for (quint8 j = 0; j < 3; ++j) {
			quint8 vertexIndex = texturedTriangle.vertexIndex[j];
			quint8 texCoordIndex = texturedTriangle.texCoordId[j];
			if (vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				const ColorRGBA &color = texturedTriangle.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qWarning() << "BsxFile::readPart error index col tex tri" << i << j << vertexIndex << vertices.size() << texCoordIndex << texCoords.size();
				break;
			}
		}

		if (!notAdd) {
			TrianglePoly *poly = new TrianglePoly(polyVertices, polyColors, polyTexCoords);
			if (!addTexturedPolygonToGroup(controlData.at(offsetControl++), poly, groups)) {
				delete poly;
				return false;
			}
		}
	}

	//--> Monochrome Textured Quads

	for (quint16 i = 0; i < partHeader.numQuadMonoTex; ++i) {
		MonochromeTexturedQuad monochromeTexturedQuad;
		if (sizeof(MonochromeTexturedQuad) != device()->read((char *)&monochromeTexturedQuad, sizeof(MonochromeTexturedQuad))) {
			qWarning() << "BsxFile::readPart error 15" << i;
			return false;
		}

		QList<PolyVertex> polyVertices;
		const ColorRGBA &color = monochromeTexturedQuad.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for (quint8 j = 0; j < 4; ++j) {
			quint8 vertexIndex = monochromeTexturedQuad.vertexIndex[j];
			quint8 texCoordIndex = monochromeTexturedQuad.texCoordId[j];
			if (vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qWarning() << "BsxFile::readPart error index mono tex quad" << i << j << vertexIndex << vertices.size() << texCoordIndex << texCoords.size();
				break;
			}
		}

		if (!notAdd) {
			QuadPoly *poly = new QuadPoly(polyVertices, polyColor, polyTexCoords);
			if (!addTexturedPolygonToGroup(controlData.at(offsetControl++), poly, groups)) {
				delete poly;
				return false;
			}
		}
	}

	//--> Monochrome Textured Triangles

	for (quint16 i = 0; i < partHeader.numTriMonoTex; ++i) {
		MonochromeTexturedTriangle monochromeTexturedTriangle;
		if (sizeof(MonochromeTexturedTriangle) != device()->read((char *)&monochromeTexturedTriangle, sizeof(MonochromeTexturedTriangle))) {
			qWarning() << "BsxFile::readPart error 17" << i;
			return false;
		}

		QList<PolyVertex> polyVertices;
		const ColorRGBA &color = monochromeTexturedTriangle.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for (quint8 j = 0; j < 3; ++j) {
			quint8 vertexIndex = monochromeTexturedTriangle.vertexIndex[j];
			quint8 texCoordIndex = monochromeTexturedTriangle.texCoordId[j];
			if (vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qWarning() << "BsxFile::readPart error index mono tex tri" << i << j << vertexIndex << vertices.size() << texCoordIndex << texCoords.size();
				break;
			}
		}

		if (!notAdd) {
			TrianglePoly *poly = new TrianglePoly(polyVertices, polyColor, polyTexCoords);
			if (!addTexturedPolygonToGroup(controlData.at(offsetControl++), poly, groups)) {
				delete poly;
				return false;
			}
		}
	}

	//--> Monochrome Triangles

	for (quint16 i = 0; i < partHeader.numTriMono; ++i) {
		MonochromeTriangle monochromeTriangle;
		if (sizeof(MonochromeTriangle) != device()->read((char *)&monochromeTriangle, sizeof(MonochromeTriangle))) {
			qWarning() << "BsxFile::readPart error 19" << i;
			return false;
		}

		QList<PolyVertex> polyVertices;
		const ColorRGBA &color = monochromeTriangle.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		notAdd = false;

		for (quint8 j = 0; j < 3; ++j) {
			quint8 vertexIndex = monochromeTriangle.vertexIndex[j];
			if (vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
			} else {
				notAdd = true;
				qWarning() << "BsxFile::readPart error index mono tri" << i << j << vertexIndex << vertices.size();
				break;
			}
		}

		if (!notAdd) {
			groups.first()->addPolygon(new TrianglePoly(polyVertices, polyColor));
		}
	}

	//--> Monochrome Quads

	for (quint16 i = 0; i < partHeader.numQuadMono; ++i) {
		MonochromeQuad monochromeQuad;
		if (sizeof(MonochromeQuad) != device()->read((char *)&monochromeQuad, sizeof(MonochromeQuad))) {
			qWarning() << "BsxFile::readPart error 20" << i;
			return false;
		}

		QList<PolyVertex> polyVertices;
		const ColorRGBA &color = monochromeQuad.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		notAdd = false;

		for (quint8 j = 0; j < 4; ++j) {
			quint8 vertexIndex = monochromeQuad.vertexIndex[j];
			if (vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
			} else {
				notAdd = true;
				qWarning() << "BsxFile::readPart error index mono quad" << i << j << vertexIndex << vertices.size();
				break;
			}
		}

		if (!notAdd) {
			groups.first()->addPolygon(new QuadPoly(polyVertices, polyColor));
		}
	}

	//--> Color Triangles

	for (quint16 i = 0; i < partHeader.numTriColor; ++i) {
		ColorTriangle colorTriangle;
		if (sizeof(ColorTriangle) != device()->read((char *)&colorTriangle, sizeof(ColorTriangle))) {
			qWarning() << "BsxFile::readPart error 21" << i;
			return false;
		}

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		notAdd = false;

		for (quint8 j = 0; j < 3; ++j) {
			quint8 vertexIndex = colorTriangle.vertexIndex[j];
			if (vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				const ColorRGBA &color = colorTriangle.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
			} else {
				notAdd = true;
				qWarning() << "BsxFile::readPart error index col tri" << i << j << "index" << vertexIndex << "size" << vertices.size();
				break;
			}
		}

		if (!notAdd) {
			groups.first()->addPolygon(new TrianglePoly(polyVertices, polyColors));
		}
	}

	//--> Color Quads

	for (quint16 i = 0; i < partHeader.numQuadColor; ++i) {
		ColorQuad colorQuad;
		if (sizeof(ColorQuad) != device()->read((char *)&colorQuad, sizeof(ColorQuad))) {
			qWarning() << "BsxFile::readPart error 22" << i;
			return false;
		}

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		notAdd = false;

		for (quint8 j = 0; j < 4; ++j) {
			quint8 vertexIndex = colorQuad.vertexIndex[j];
			if (vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				const ColorRGBA &color = colorQuad.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
			} else {
				notAdd = true;
				qWarning() << "error index col quad" << i << j << "index" << vertexIndex << "size" << vertices.size();
				break;
			}
		}

		if (!notAdd) {
			groups.first()->addPolygon(new QuadPoly(polyVertices, polyColors));
		}
	}

	part->setGroups(groups);

	return true;
}

bool BsxFile::addTexturedPolygonToGroup(quint8 control, Poly *polygon, QList<FieldModelGroup *> &groups)
{
	quint8 blend = (control >> 4) & 0x03,
			flagID = control & 0x0F;

	if (flagID >= groups.size()) {
		qWarning() << "BsxFile::addTexturedPolygonToGroup error 2" << flagID << groups.size();
		return false;
	}

	FieldModelGroup *group = groups.at(flagID + 1);
	group->setBlendMode(blend);
	group->addPolygon(polygon);

	return true;
}

bool BsxFile::readAnimations(const QList<FieldModelAnimationPSHeader> &animationHeaders,
							 QList<FieldModelAnimation> &animations) const
{
	for (const FieldModelAnimationPSHeader &header : animationHeaders) {
		FieldModelAnimation animation;

		if (!readAnimation(header, animation)) {
			return false;
		}

		animations.append(animation);
	}
	return true;
}

bool BsxFile::readAnimation(const FieldModelAnimationPSHeader &header, FieldModelAnimation &animation) const
{
	quint32 offsetToAnimation = header.offsetData & 0x7FFFFFFF,
			offsetFrameRotation = offsetToAnimation + header.offsetFramesRotation,
			offsetFrameStatic = offsetToAnimation + header.offsetStaticTranslation,
			offsetFrameTranslation = offsetToAnimation + header.offsetFramesTranslation;

	if (!device()->seek(offsetToAnimation + 4)) {
		qWarning() << "BsxFile::readAnimation error 1";
		return false;
	}

	for (quint32 frame = 0; frame < header.numFrames; ++frame) {
		QList<PolyVertex> rotationCoords, rotationCoordsTrans;

		for (quint16 bone = 0; bone < header.numBones; ++bone) {
			FrameTranslation frameTrans;
			PolyVertex rot, trans;
			char rotChar;

			if (!device()->seek(offsetToAnimation + 4 + bone * sizeof(FrameTranslation))) {
				qWarning() << "BsxFile::readAnimation error 2";
				return false;
			}
			if (sizeof(FrameTranslation) != device()->read((char *)&frameTrans, sizeof(FrameTranslation))) {
				qWarning() << "BsxFile::readAnimation error 3";
				return false;
			}

			// Rotation

			if (frameTrans.flag & 0x01) {
				quint32 offsetToRotation = offsetFrameRotation + frameTrans.rx * header.numFrames + frame;

				if (!device()->seek(offsetToRotation)) {
					qWarning() << "BsxFile::readAnimation error 4";
					return false;
				}
				if (!device()->getChar(&rotChar)) {
					qWarning() << "BsxFile::readAnimation error 5";
					return false;
				}
				
				rot.x = 360.0f * quint8(rotChar) / 256.0f;
			} else {
				rot.x = 360.0f * frameTrans.rx / 256.0f;
			}

			if (frameTrans.flag & 0x02) {
				quint32 offsetToRotation = offsetFrameRotation + frameTrans.ry * header.numFrames + frame;

				if (!device()->seek(offsetToRotation)) {
					qWarning() << "BsxFile::readAnimation error 6";
					return false;
				}
				if (!device()->getChar(&rotChar)) {
					qWarning() << "BsxFile::readAnimation error 7";
					return false;
				}
				
				rot.y = 360.0f * quint8(rotChar) / 256.0f;
			} else {
				rot.y = 360.0f * frameTrans.ry / 256.0f;
			}

			if (frameTrans.flag & 0x04) {
				quint32 offsetToRotation = offsetFrameRotation + frameTrans.rz * header.numFrames + frame;

				if (!device()->seek(offsetToRotation)) {
					qWarning() << "BsxFile::readAnimation error 8";
					return false;
				}
				if (!device()->getChar(&rotChar)) {
					qWarning() << "BsxFile::readAnimation error 9";
					return false;
				}
				
				rot.z = 360.0f * quint8(rotChar) / 256.0f;
			} else {
				rot.z = 360.0f * frameTrans.rz / 256.0f;
			}

			// (translation)

			qint16 translation=0;

			if (frameTrans.flag & 0x10) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.tx * header.numFrames * 2 + frame * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "BsxFile::readAnimation error 10";
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "BsxFile::readAnimation error 11";
					return false;
				}
			} else if (frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.tx * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "BsxFile::readAnimation error 12";
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "BsxFile::readAnimation error 13";
					return false;
				}
			}
			trans.x = -translation / MODEL_SCALE_PS;
			translation=0;

			if (frameTrans.flag & 0x20) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.ty * header.numFrames * 2 + frame * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "BsxFile::readAnimation error 14";
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "BsxFile::readAnimation error 15";
					return false;
				}
			} else if (frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.ty * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "BsxFile::readAnimation error 16";
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "BsxFile::readAnimation error 17";
					return false;
				}
			}
			trans.y = -translation / MODEL_SCALE_PS;
			translation = 0;

			if (frameTrans.flag & 0x40) {
				quint32 offsetToTranslation = offsetFrameTranslation + frameTrans.tz * header.numFrames * 2 + frame * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "BsxFile::readAnimation error 18";
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "BsxFile::readAnimation error 19";
					return false;
				}
			} else if (frameTrans.tx != 0xFF) {
				quint32 offsetToTranslation = offsetFrameStatic + frameTrans.tz * 2;

				if (!device()->seek(offsetToTranslation)) {
					qWarning() << "BsxFile::readAnimation error 20";
					return false;
				}
				if (2 != device()->read((char *)&translation, 2)) {
					qWarning() << "BsxFile::readAnimation error 21";
					return false;
				}
			}
			trans.z = -translation / MODEL_SCALE_PS;

			rotationCoords.append(rot);
			rotationCoordsTrans.append(trans);
		}

		animation.insertFrame(frame, rotationCoords, rotationCoordsTrans);
	}

	return true;
}

bool BsxFile::readTexturesHeaders(quint8 numTextures, QList<BsxTextureHeader> &headers) const
{
	for (quint16 texId = 0; texId < numTextures; ++texId) {
		BsxTextureHeader header;
		if (sizeof(BsxTextureHeader) != device()->read((char *)&header, sizeof(BsxTextureHeader))) {
			qWarning() << "BsxFile::readTexturesHeaders error";
			return false;
		}
		headers.append(header);
	}
	return true;
}

bool BsxFile::readTexturesData(const QList<BsxTextureHeader> &headers,
							   QList<QByteArray> &dataList) const
{
	qint64 offsetTextures = _offsetModels + _offsetTextures;

	for (const BsxTextureHeader &header : headers) {
		if (device()->pos() != offsetTextures + header.offsetData) {
			qWarning() << "BsxFile::readTexturesData error pos" << _offsetModels << _offsetTextures << header.offsetData << device()->pos();
			return false;
		}

		int size = header.width * header.height * 2;
		QByteArray data = device()->read(size);

		if (data.size() != size) {
			qWarning() << "BsxFile::readTexturesData error read";
			return false;
		}

		dataList.append(data);
	}

	if (device()->pos() != device()->size()) {
		qWarning() << "BsxFile::readTexturesData not at the end" << device()->pos() << device()->size();
	}

	return true;
}

bool BsxFile::write(const QList<FieldModelFilePS> &models) const
{
	Q_UNUSED(models)
	return false;
}

bool BsxFile::writeModelHeader(const FieldModelFilePS &model) const
{
	BsxModelHeader modelHeader;

	if (sizeof(BsxModelHeader) != device()->peek((char *)&modelHeader, sizeof(BsxModelHeader))) {
		qWarning() << "BsxFile::writeModelHeader model read error 1";
		return false;
	}

	const QList<FieldModelColorDir> &lightColors = model.lightColors();
	modelHeader.colorADir1 = lightColors.at(0).dirA;
	modelHeader.colorADir2 = lightColors.at(0).dirB;
	modelHeader.colorADir3 = lightColors.at(0).dirC;
	QRGB_2_COLORRGB(lightColors.at(0).color, modelHeader.colorA);
	modelHeader.colorBDir1 = lightColors.at(1).dirA;
	modelHeader.colorBDir2 = lightColors.at(1).dirB;
	modelHeader.colorBDir3 = lightColors.at(1).dirC;
	QRGB_2_COLORRGB(lightColors.at(1).color, modelHeader.colorB);
	modelHeader.colorCDir1 = lightColors.at(2).dirA;
	modelHeader.colorCDir2 = lightColors.at(2).dirB;
	modelHeader.colorCDir3 = lightColors.at(2).dirC;
	QRGB_2_COLORRGB(lightColors.at(2).color, modelHeader.colorC);
	QRGB_2_COLORRGB(model.globalColor(), modelHeader.globalColor);
	modelHeader.scale = model.scale();

	if (sizeof(BsxModelHeader) != device()->write((char *)&modelHeader, sizeof(BsxModelHeader))) {
		qWarning() << "BsxFile::writeModelHeader model write error 2";
		return false;
	}

	return true;
}
