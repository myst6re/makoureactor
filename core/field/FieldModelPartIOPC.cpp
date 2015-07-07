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
#include "FieldModelPartIOPC.h"

FieldModelPartIOPC::FieldModelPartIOPC(QIODevice *ioRsd) :
	FieldModelPartIO(0), _deviceRsd(ioRsd)
{
}

bool FieldModelPartIOPC::canReadRsd() const
{
	if(_deviceRsd) {
		if(!_deviceRsd->isOpen()) {
			return _deviceRsd->open(QIODevice::ReadOnly);
		}
		return _deviceRsd->isReadable();
	}
	return false;
}

bool FieldModelPartIOPC::read(FieldModelPart *part, const QList<int> &texIds) const
{
	if(!canRead()) {
		return false;
	}

	QList<PolyVertex> vertices/*, normals*/;
	QList<TexCoord> texCs;
	QList<ColorBGRA> vertexColors;
	QList<PolygonP> polys;
	QList<Group> groups;
	QList<FieldModelGroup *> _groups;
	PHeader header;
	PolyVertex vertex;
	TexCoord texC;
	ColorBGRA vertexColor;
	PolygonP poly;
	Group group;
	quint32 i;

	if(device()->read((char *)&header, 128)!=128
		|| header.version!=1 || header.off04!=1 || header.vertexType!=1)
		return false;

	for(i=0 ; i<header.numVertices ; ++i) {
		if(device()->read((char *)&vertex, 12)!=12)	return false;
		vertex.x = vertex.x / MODEL_SCALE_PC;
		vertex.y = vertex.y / MODEL_SCALE_PC;
		vertex.z = vertex.z / MODEL_SCALE_PC;
		vertices.append(vertex);
	}

//	for(i=0 ; i<header.numNormals ; ++i) {
//		if(device()->read((char *)&vertex, 12)!=12)	return false;
//		normals.append(vertex);
//	}

	device()->seek(device()->pos() + header.numNormals*12 + header.numUnknown1*12);

	for(i=0 ; i<header.numTexCs ; ++i) {
		if(device()->read((char *)&texC, 8)!=8)	return false;
		texCs.append(texC);
	}

	for(i=0 ; i<header.numVertexColors ; ++i) {
		if(device()->read((char *)&vertexColor, 4)!=4)	return false;
		vertexColors.append(vertexColor);
	}

	device()->seek(device()->pos()+(header.numPolys+header.numEdges)*4);

	for(i=0 ; i<header.numPolys ; ++i) {
		if(device()->read((char *)&poly, 24)!=24)	return false;
		polys.append(poly);
	}

	device()->seek(device()->pos()+header.numHundreds*100);

	for(i=0 ; i<header.numGroups ; ++i) {
		if(device()->read((char *)&group, 56)!=56)	return false;
		groups.append(group);
	}

	PolyVertex polyVertex;
	QRgb color;

	foreach(const Group &g, groups) {
		FieldModelGroup *grp = new FieldModelGroup();

		if(g.areTexturesUsed) {
			if (g.textureNumber < quint32(texIds.size())) {
				// Convert relative group tex IDs to absolute tex IDs
				grp->setTextureNumber(texIds.at(g.textureNumber));
			}
		}

		for(quint32 polyID=0 ; polyID<g.numPolygons && g.polygonStartIndex + polyID < (quint32)polys.size() ; ++polyID) {
			const PolygonP &poly = polys.at(g.polygonStartIndex + polyID);
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

	part->setGroups(_groups);

	return true;
}

bool FieldModelPartIOPC::readRsd(Rsd &rsd, QStringList &textureNames) const
{
	if (!canReadRsd()) {
		return false;
	}

	QString pname;
	QList<int> texIds;
	quint32 nTex = 0;
	int index;
	bool ok;

	while(deviceRsd()->canReadLine()) {
		QString line = QString(deviceRsd()->readLine()).trimmed();
		if(pname.isNull() && (line.startsWith(QString("PLY="))
							  || line.startsWith(QString("MAT="))
							  || line.startsWith(QString("GRP=")))) {
			index = line.lastIndexOf('.');
			if(index != -1) {
				line.truncate(index);
			}
			pname = line.mid(4).toLower();
		} else if(!pname.isNull() && nTex == 0
				  && line.startsWith(QString("NTEX="))) {
			nTex = line.mid(5).toUInt(&ok);
			if(!ok) {
				return false;
			}

			for(quint32 i = 0 ; i < nTex && deviceRsd()->canReadLine() ; ++i) {
				line = QString(deviceRsd()->readLine()).trimmed();
				if(!line.startsWith(QString("TEX[%1]=").arg(i))) {
					return false;
				}

				index = line.lastIndexOf('.');
				if(index != -1) {
					line.truncate(index);
				}
				QString tex = line.mid(line.indexOf('=') + 1).toLower();

				index = textureNames.indexOf(tex);
				if(index > -1) {
					texIds.append(index);
				} else {
					texIds.append(textureNames.size());
					textureNames.append(tex);
				}
			}
		}
	}

	rsd.pName = pname;
	rsd.texIds = texIds;

	return true;
}

bool FieldModelPartIOPC::write(const FieldModelPart *part) const
{
	Q_UNUSED(part)
	// TODO
	return false;
}
