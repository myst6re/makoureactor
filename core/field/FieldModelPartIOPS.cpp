#include "FieldModelPartIOPS.h"

FieldModelPartIOPS::FieldModelPartIOPS(QIODevice *io) :
	FieldModelPartIO(io)
{
}

bool FieldModelPartIOPS::read(FieldModelPartPS *part) const
{
	if(!canRead()) {
		qWarning() << "FieldModelPartIOPS::read device not opened";
		return false;
	}

	quint32 i;
	QList<FieldModelGroup *> _groups;
	QList<TextureInfo> _textures;
	QList<PolyVertex> vertices;
	QList<TexCoord> texCoords;
	Part partHeader;

	// Header
	if (sizeof(Part) != device()->read((char *)&partHeader, sizeof(Part))) {
		qWarning() << "FieldModelPartIOPS::read Invalid part size" << device()->size();
		return false;
	}

	part->setBoneID(partHeader.bone_index);

//	qDebug() << "==== PART ====";
//	qDebug() << "BoneID" << _boneID << "unknown" << partHeader.unknown;

	// Vertices
	quint32 offsetToVertex = partHeader.offset_vertex - 0x80000000;

	// Note: there are four empty bytes here
	if(!device()->seek(offsetToVertex + 4)) {
		qWarning() << "FieldModelPartIOPS::read Invalid part size2" << offsetToVertex << partHeader.numVertices << device()->size();
		return false;
	}

	for(i=0 ; i<partHeader.numVertices ; ++i) {
		Vertex v;

		if (sizeof(Vertex) != device()->read((char *)&v, sizeof(Vertex))) {
			qWarning() << "FieldModelPartIOPS::read Invalid part size2" << offsetToVertex << partHeader.numVertices << device()->size();
			return false;
		}

		PolyVertex vertex;

		vertex.x = v.v.x / MODEL_SCALE_PS;
		vertex.y = v.v.y / MODEL_SCALE_PS;
		vertex.z = v.v.z / MODEL_SCALE_PS;

		vertices.append(vertex);
	}

	// Texture coordinates
	quint32 offsetToTexCoords = offsetToVertex + partHeader.offset_texcoord;

	if(!device()->seek(offsetToTexCoords)) {
		qWarning() << "FieldModelPartIOPS::read Invalid part size3" << offsetToTexCoords << device()->size();
		return false;
	}

	for(i=0 ; i<partHeader.numTexCs ; ++i) {
		TexCoord texCoord;
		char c;

		if (!device()->getChar(&c)) {
			qWarning() << "FieldModelPartIOPS::read Invalid part size3" << offsetToVertex << partHeader.numVertices << device()->size();
			return false;
		}
		texCoord.x = c;
		if (!device()->getChar(&c)) {
			qWarning() << "FieldModelPartIOPS::read Invalid part size3" << offsetToVertex << partHeader.numVertices << device()->size();
			return false;
		}
		texCoord.y = c;

		texCoords.append(texCoord);
	}

	// Texture informations
	quint32 offsetToTexInfos = offsetToVertex + partHeader.offset_flags;

	if(!device()->seek(offsetToTexInfos)) {
		qWarning() << "FieldModelPartIOPS::read Invalid part size4" << offsetToTexInfos << device()->size();
		return false;
	}

	_groups.append(new FieldModelGroup()); // group with no textured polygons

	for(i=0 ; i<partHeader.num_flags ; ++i) {
		quint32 flag;
		if (4 != device()->read((char *)&flag, 4)) {
			qWarning() << "FieldModelPartIOPS::read Invalid part size4" << device()->size();
			return false;
		}

		TextureInfo texInfo;

		texInfo.type	= flag & 0x3F;							// 0000 0000 0000 0000 0000 0000 0011 1111
		texInfo.bpp		= (flag >> 6) & 0x03;					// 0000 0000 0000 0000 0000 0000 1100 0000
		texInfo.imgX	= /*512 + */((flag >> 8) & 0x0f) * 64;		// 0000 0000 0000 0000 0000 1111 0000 0000
		texInfo.imgY	= /*256 + */((flag >> 12) & 0x01) * 256;	// 0000 0000 0000 0000 0001 0000 0000 0000
//		quint8 u1		= ((flag >> 13) & 0x07);
		texInfo.palX	= /*512 + */((flag >> 16) & 0x3F) * 16;		// 0000 0000 0011 1111 0000 0000 0000 0000
		texInfo.palY	= /*256 + */((flag >> 22) & 0x01FF);		// 0111 1111 1100 0000 0000 0000 0000 0000
//		quint8 u2		= ((flag >> 31) & 0x01);

		_textures.append(texInfo);

//		qDebug() << "ADDTEXTURE" << i;
//		qDebug() << "type" << texInfo.type << "bpp" << texInfo.bpp;
//		qDebug() << "imgX" << texInfo.imgX << "imgY" << texInfo.imgY << "u1" << u1;
//		qDebug() << "palX" << texInfo.palX << "palY" << texInfo.palY << "u2" << u2;

		_groups.append(new FieldModelGroup(i));
	}

	part->setGroups(_groups);
	part->setTextures(_textures);

	// Polygons
	const quint32 offsetToPoly = offsetToVertex + partHeader.offset_poly;
	const quint32 offsetToControl = offsetToVertex + partHeader.offset_control;

	if (!device()->seek(offsetToControl)) {
		return false;
	}
	QByteArray controlData = device()->read(partHeader.num_control);
	if (controlData.size() != partHeader.num_control) {
		return false;
	}

//	qDebug() << "Offset to vertex" << offsetToVertex << "num" << partHeader.numVertices;
//	qDebug() << "Offset to vertex (after all)" << (offsetToVertex + partHeader.numVertices*8);
//	quint32 offsetDebug = offsetToPoly;
//	qDebug() << "Offset to textured quads" << offsetDebug << "num" << partHeader.num_quad_color_tex;
//	offsetDebug += partHeader.num_quad_color_tex*sizeof(TexturedQuad);
//	qDebug() << "Offset to textured triangles" << offsetDebug << "num" << partHeader.num_tri_color_tex;
//	offsetDebug += partHeader.num_tri_color_tex*sizeof(TexturedTriangle);
//	qDebug() << "Offset to mono textured quads" << offsetDebug << "num" << partHeader.num_quad_mono_tex;
//	offsetDebug += partHeader.num_quad_mono_tex*sizeof(MonochromeTexturedQuad);
//	qDebug() << "Offset to mono textured triangles" << offsetDebug << "num" << partHeader.num_tri_mono_tex;
//	offsetDebug += partHeader.num_tri_mono_tex*sizeof(MonochromeTexturedTriangle);
//	qDebug() << "Offset to mono triangles" << offsetDebug << "num" << partHeader.num_tri_mono;
//	offsetDebug += partHeader.num_tri_mono*sizeof(MonochromeTriangle);
//	qDebug() << "Offset to mono quads" << offsetDebug << "num" << partHeader.num_quad_mono;
//	offsetDebug += partHeader.num_quad_mono*sizeof(MonochromeQuad);
//	qDebug() << "Offset to color triangles" << offsetDebug << "num" << partHeader.num_tri_color;
//	offsetDebug += partHeader.num_tri_color*sizeof(ColorTriangle);
//	qDebug() << "Offset to color quads" << offsetDebug << "num" << partHeader.num_quad_color;
//	offsetDebug += partHeader.num_quad_color*sizeof(ColorQuad);
//	qDebug() << "Offset to polygons (after all)" << offsetDebug;
//	qDebug() << "Offset to texsCoords" << offsetToTexCoords << "num" << partHeader.numTexCs;
//	qDebug() << "Offset to texsCoords (after all)" << (offsetToTexCoords + partHeader.numTexCs * 2);
//	qDebug() << "Offset to flags" << offsetToTexInfos << "num" << partHeader.num_flags;
//	qDebug() << "Offset to flags (after all)" << (offsetToTexInfos + partHeader.num_flags*4);
//	qDebug() << "Offset to control" << offsetToControl << "num" << partHeader.num_control;
//	qDebug() << "Offset to control (after all)" << (offsetToControl + partHeader.num_control);
//	qDebug() << "BufferSize" << partHeader.buffer_size;
//	qDebug() << "Offset to prec" << partHeader.offset_prec;

	bool notAdd;

	if (!device()->seek(offsetToPoly)) {
		return false;
	}
	int offsetControl = 0;

	//--> Textured Quads

	for(i=0 ; i<partHeader.num_quad_color_tex ; ++i) {
		TexturedQuad texturedQuad;
		if (sizeof(TexturedQuad) != device()->read((char *)&texturedQuad, sizeof(TexturedQuad))) {
			return false;
		}

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for(int j=0 ; j<4 ; ++j) {
			quint8 vertexIndex = texturedQuad.vertexIndex[j];
			quint8 texCoordIndex = texturedQuad.texCoordId[j];
			if(vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				const ColorRGBA &color = texturedQuad.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qWarning() << "error index col tex quad" << i << j << vertexIndex << vertices.size() << texCoordIndex << texCoords.size();
				break;
			}
		}

		if(!notAdd) {
			if (offsetControl >= controlData.size()) {
				return false;
			}
			part->addTexturedPolygon(controlData[offsetControl++], new QuadPoly(polyVertices, polyColors, polyTexCoords));
		}
	}

	//--> Textured Triangles

	for(i=0 ; i<partHeader.num_tri_color_tex ; ++i) {
		TexturedTriangle texturedTriangle;
		if (sizeof(TexturedTriangle) != device()->read((char *)&texturedTriangle, sizeof(TexturedTriangle))) {
			return false;
		}

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for(int j=0 ; j<3 ; ++j) {
			quint8 vertexIndex = texturedTriangle.vertexIndex[j];
			quint8 texCoordIndex = texturedTriangle.texCoordId[j];
			if(vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				const ColorRGBA &color = texturedTriangle.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qWarning() << "error index col tex tri" << i << j << vertexIndex << vertices.size() << texCoordIndex << texCoords.size();
				break;
			}
		}

		if(!notAdd) {
			if (offsetControl >= controlData.size()) {
				return false;
			}
			part->addTexturedPolygon(controlData[offsetControl++], new TrianglePoly(polyVertices, polyColors, polyTexCoords));
		}
	}

	//--> Monochrome Textured Quads

	for(i=0 ; i<partHeader.num_quad_mono_tex ; ++i) {
		MonochromeTexturedQuad monochromeTexturedQuad;
		if (sizeof(MonochromeTexturedQuad) != device()->read((char *)&monochromeTexturedQuad, sizeof(MonochromeTexturedQuad))) {
			return false;
		}

		QList<PolyVertex> polyVertices;
		const ColorRGBA &color = monochromeTexturedQuad.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for(int j=0 ; j<4 ; ++j) {
			quint8 vertexIndex = monochromeTexturedQuad.vertexIndex[j];
			quint8 texCoordIndex = monochromeTexturedQuad.texCoordId[j];
			if(vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qWarning() << "error index mono tex quad" << i << j << vertexIndex << vertices.size() << texCoordIndex << texCoords.size();
				break;
			}
		}

		if(!notAdd) {
			if (offsetControl >= controlData.size()) {
				return false;
			}
			part->addTexturedPolygon(controlData[offsetControl++], new QuadPoly(polyVertices, polyColor, polyTexCoords));
		}
	}

	//--> Monochrome Textured Triangles

	for(i=0 ; i<partHeader.num_tri_mono_tex ; ++i) {
		MonochromeTexturedTriangle monochromeTexturedTriangle;
		if (sizeof(MonochromeTexturedTriangle) != device()->read((char *)&monochromeTexturedTriangle, sizeof(MonochromeTexturedTriangle))) {
			return false;
		}

		QList<PolyVertex> polyVertices;
		const ColorRGBA &color = monochromeTexturedTriangle.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		QList<TexCoord> polyTexCoords;
		notAdd = false;

		for(int j=0 ; j<3 ; ++j) {
			quint8 vertexIndex = monochromeTexturedTriangle.vertexIndex[j];
			quint8 texCoordIndex = monochromeTexturedTriangle.texCoordId[j];
			if(vertexIndex < vertices.size() && texCoordIndex < texCoords.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				polyTexCoords.append(texCoords.at(texCoordIndex));
			} else {
				notAdd = true;
				qWarning() << "error index mono tex tri" << i << j << vertexIndex << vertices.size() << texCoordIndex << texCoords.size();
				break;
			}
		}

		if(!notAdd) {
			if (offsetControl >= controlData.size()) {
				return false;
			}
			part->addTexturedPolygon(controlData[offsetControl++], new TrianglePoly(polyVertices, polyColor, polyTexCoords));
		}
	}

	//--> Monochrome Triangles

	for(i=0 ; i<partHeader.num_tri_mono ; ++i) {
		MonochromeTriangle monochromeTriangle;
		if (sizeof(MonochromeTriangle) != device()->read((char *)&monochromeTriangle, sizeof(MonochromeTriangle))) {
			return false;
		}

		QList<PolyVertex> polyVertices;
		const ColorRGBA &color = monochromeTriangle.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		notAdd = false;

		for(int j=0 ; j<3 ; ++j) {
			quint8 vertexIndex = monochromeTriangle.vertexIndex[j];
			if(vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
			} else {
				notAdd = true;
				qWarning() << "error index mono tri" << i << j << vertexIndex << vertices.size();
				break;
			}
		}

		if(!notAdd) {
			part->addPolygon(new TrianglePoly(polyVertices, polyColor));
		}
	}

	//--> Monochrome Quads

	for(i=0 ; i<partHeader.num_quad_mono ; ++i) {
		MonochromeQuad monochromeQuad;
		if (sizeof(MonochromeQuad) != device()->read((char *)&monochromeQuad, sizeof(MonochromeQuad))) {
			return false;
		}

		QList<PolyVertex> polyVertices;
		const ColorRGBA &color = monochromeQuad.color;
		QRgb polyColor = qRgb(color.red, color.green, color.blue);
		notAdd = false;

		for(int j=0 ; j<4 ; ++j) {
			quint8 vertexIndex = monochromeQuad.vertexIndex[j];
			if(vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
			} else {
				notAdd = true;
				qWarning() << "error index mono quad" << i << j << vertexIndex << vertices.size();
				break;
			}
		}

		if(!notAdd) {
			part->addPolygon(new QuadPoly(polyVertices, polyColor));
		}
	}

	//--> Color Triangles

	for(i=0 ; i<partHeader.num_tri_color ; ++i) {
		ColorTriangle colorTriangle;
		if (sizeof(ColorTriangle) != device()->read((char *)&colorTriangle, sizeof(ColorTriangle))) {
			return false;
		}

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		notAdd = false;

		for(int j=0 ; j<3 ; ++j) {
			quint8 vertexIndex = colorTriangle.vertexIndex[j];
			if(vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				const ColorRGBA &color = colorTriangle.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
			} else {
				notAdd = true;
				qWarning() << "error index col tri" << i << j << "index" << vertexIndex << "size" << vertices.size();
				break;
			}
		}

		if(!notAdd) {
			part->addPolygon(new TrianglePoly(polyVertices, polyColors));
		}
	}

	//--> Color Quads

	for(i=0 ; i<partHeader.num_quad_color ; ++i) {
		ColorQuad colorQuad;
		if (sizeof(ColorQuad) != device()->read((char *)&colorQuad, sizeof(ColorQuad))) {
			return false;
		}

		QList<PolyVertex> polyVertices;
		QList<QRgb> polyColors;
		notAdd = false;

		for(int j=0 ; j<4 ; ++j) {
			quint8 vertexIndex = colorQuad.vertexIndex[j];
			if(vertexIndex < vertices.size()) {
				polyVertices.append(vertices.at(vertexIndex));
				const ColorRGBA &color = colorQuad.color[j];
				polyColors.append(qRgb(color.red, color.green, color.blue));
			} else {
				notAdd = true;
				qWarning() << "error index col quad" << i << j << "index" << vertexIndex << "size" << vertices.size();
				break;
			}
		}

		if(!notAdd) {
			part->addPolygon(new QuadPoly(polyVertices, polyColors));
		}
	}

	return true;
}

bool FieldModelPartIOPS::write(const FieldModelPartPS *part) const
{
	Q_UNUSED(part)
	// TODO
	return false;
}

