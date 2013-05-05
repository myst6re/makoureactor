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
#include "FieldModelPartPC.h"

FieldModelPartPC::FieldModelPartPC()
{
}

bool FieldModelPartPC::open(QIODevice *p_file)
{
	QList<PolyVertex> vertices/*, normals*/;
	QList<TexCoord> texCs;
	QList<ColorBGRA> vertexColors;
	QList<Polygon_p> polys;
	QList<Group> groups;
	p_header header;
	PolyVertex vertex;
	TexCoord texC;
	ColorBGRA vertexColor;
	Polygon_p poly;
	Group group;
	quint32 i;

	if(p_file->read((char *)&header, 128)!=128
		|| header.version!=1 || header.off04!=1 || header.vertexType!=1)
		return false;

	for(i=0 ; i<header.numVertices ; ++i) {
		if(p_file->read((char *)&vertex, 12)!=12)	return false;
		vertex.x = vertex.x / MODEL_SCALE_PC;
		vertex.y = vertex.y / MODEL_SCALE_PC;
		vertex.z = vertex.z / MODEL_SCALE_PC;
		vertices.append(vertex);
	}

//	for(i=0 ; i<header.numNormals ; ++i) {
//		if(p_file->read((char *)&vertex, 12)!=12)	return false;
//		normals.append(vertex);
//	}

	p_file->seek(p_file->pos() + header.numNormals*12 + header.numUnknown1*12);

	for(i=0 ; i<header.numTexCs ; ++i) {
		if(p_file->read((char *)&texC, 8)!=8)	return false;
		texCs.append(texC);
	}

	for(i=0 ; i<header.numVertexColors ; ++i) {
		if(p_file->read((char *)&vertexColor, 4)!=4)	return false;
		vertexColors.append(vertexColor);
	}

	p_file->seek(p_file->pos()+(header.numPolys+header.numEdges)*4);

	for(i=0 ; i<header.numPolys ; ++i) {
		if(p_file->read((char *)&poly, 24)!=24)	return false;
		polys.append(poly);
	}

	p_file->seek(p_file->pos()+header.numHundreds*100);

	for(i=0 ; i<header.numGroups ; ++i) {
		if(p_file->read((char *)&group, 56)!=56)	return false;
		groups.append(group);
	}

	PolyVertex polyVertex;
	QRgb color;

	foreach(const Group &g, groups) {
		FieldModelGroup *grp = new FieldModelGroup();

		if(g.areTexturesUsed) {
			grp->setTextureNumber(g.textureNumber);
		}

		for(quint32 polyID=0 ; polyID<g.numPolygons && g.polygonStartIndex + polyID < (quint32)polys.size() ; ++polyID) {
			const Polygon_p &poly = polys.at(g.polygonStartIndex + polyID);
			QList<PolyVertex> polyVertices;
			QList<QRgb> polyColors;
			QList<TexCoord> polyTexCoords;
			for(quint8 j=0 ; j<3 ; ++j) {
				if(g.areTexturesUsed) {
					int vertexIndex = g.verticesStartIndex + poly.VertexIndex[j];
					int texCoordIndex = g.texCoordStartIndex + poly.VertexIndex[j];

					if(vertexIndex < vertices.size() &&
							vertexIndex < vertexColors.size() &&
							texCoordIndex < texCs.size()) {
						// vertex
						vertex = vertices.at(vertexIndex);
						polyVertex.x = vertex.x;
						polyVertex.y = vertex.y;
						polyVertex.z = vertex.z;
						polyVertices.append(polyVertex);
						// color
						vertexColor = vertexColors.at(vertexIndex);
						color = qRgb(vertexColor.red, vertexColor.green, vertexColor.blue);
						polyColors.append(color);
						// tex coord
						polyTexCoords.append(texCs.at(texCoordIndex));
					}
				} else {
					int vertexIndex = g.verticesStartIndex + poly.VertexIndex[j];

					if(vertexIndex < vertices.size() &&
							vertexIndex < vertexColors.size()) {
						// vertex
						vertex = vertices.at(vertexIndex);
						polyVertex.x = vertex.x;
						polyVertex.y = vertex.y;
						polyVertex.z = vertex.z;
						polyVertices.append(polyVertex);
						// color
						vertexColor = vertexColors.at(vertexIndex);
						color = qRgb(vertexColor.red, vertexColor.green, vertexColor.blue);
						polyColors.append(color);
					}
				}
			}
			grp->addPolygon(new TrianglePoly(polyVertices, polyColors, polyTexCoords));
		}

		_groups.append(grp);
	}

	return true;
}
