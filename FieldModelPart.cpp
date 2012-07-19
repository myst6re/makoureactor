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
#include "FieldModelPart.h"

FieldModelPart::FieldModelPart()
{
}

bool FieldModelPart::open_p(QFile *p_file)
{
	p_header header;
	VertexPC vertex;
	Coord texC;
	Color vertexColor;
	Polygon_p poly;
	Group group;
	quint32 i;

	if(p_file->read((char *)&header, 128)!=128
		|| header.version!=1 || header.off04!=1 || header.vertexType!=1)
		return false;

	for(i=0 ; i<header.numVertices ; ++i) {
		if(p_file->read((char *)&vertex, 12)!=12)	return false;
		vertices.append(vertex);
//		qDebug() << "vertex" << i << vertex.x << vertex.y << vertex.z;
	}

	for(i=0 ; i<header.numNormals ; ++i) {
		if(p_file->read((char *)&vertex, 12)!=12)	return false;
		normals.append(vertex);
	}

	p_file->seek(p_file->pos()+header.numUnknown1*12);

	for(i=0 ; i<header.numTexCs ; ++i) {
		if(p_file->read((char *)&texC, 8)!=8)	return false;
		this->texCs.append(texC);
	}

	for(i=0 ; i<header.numVertexColors ; ++i) {
		if(p_file->read((char *)&vertexColor, 4)!=4)	return false;
		this->vertexColors.append(vertexColor);
	}

	p_file->seek(p_file->pos()+(header.numPolys+header.numEdges)*4);

	for(i=0 ; i<header.numPolys ; ++i) {
		if(p_file->read((char *)&poly, 24)!=24)	return false;
		this->polys.append(poly);
	}

	p_file->seek(p_file->pos()+header.numHundreds*100);

	for(i=0 ; i<header.numGroups ; ++i) {
		if(p_file->read((char *)&group, 56)!=56)	return false;
		this->groups.append(group);
	}

	return true;
}
