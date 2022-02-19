/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
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
#include "PFile.h"
#include "FieldModelTextureRefPC.h"

PFile::PFile(QIODevice *io) :
	IO(io)
{
}

PFile::~PFile()
{
}

bool PFile::read(FieldModelPart *part, const QList<int> &texIds) const
{
	if (!canRead()) {
		return false;
	}

	QList<PolyVertex> vertices, normals;
	QList<QByteArray> unknown1;
	QList<TexCoord> texCs;
	QList<ColorBGRA> vertexColors;
	QList<QByteArray> polyColors;
	QList<Edge> edges;
	QList<PolygonP> polys;
	QList<Hundred> hundreds;
	QList<Group> groups;
	QList<FieldModelGroup *> _groups;
	QList<BoundingBox> boundingBoxes;
	QList<quint32> normIndexTable;
	PHeader header;
	PolyVertex vertex;
	TexCoord texC;
	ColorBGRA vertexColor;
	Edge edge;
	PolygonP poly;
	Hundred hundred;
	Group group;
	BoundingBox boundingBox;
	quint32 i, normIndex;

	if (device()->read((char *)&header, 128) != 128
	        || header.version != 1 || header.off04 != 1
	        || header.vertexType > 1) {
		return false;
	}

	for (i = 0; i < header.numVertices; ++i) {
		if (device()->read((char *)&vertex, 12) != 12) {
			return false;
		}
		vertex.x = vertex.x / MODEL_SCALE_PC;
		vertex.y = vertex.y / MODEL_SCALE_PC;
		vertex.z = vertex.z / MODEL_SCALE_PC;
		vertices.append(vertex);
	}

	for (i = 0; i < header.numNormals; ++i) {
		if (device()->read((char *)&vertex, 12) != 12) {
			return false;
		}
		normals.append(vertex);
	}

	for (i = 0; i < header.numUnknown1; ++i) {
		QByteArray u1 = device()->read(12);
		if (u1.size() != 12) {
			return false;
		}
		unknown1.append(u1);
	}

	for (i = 0; i < header.numTexCs; ++i) {
		if (device()->read((char *)&texC, 8) != 8) {
			return false;
		}
		texCs.append(texC);
	}

	for (i = 0; i < header.numVertexColors; ++i) {
		if (device()->read((char *)&vertexColor, 4) != 4) {
			return false;
		}
		vertexColors.append(vertexColor);
	}

	for (i = 0; i < header.numPolys; ++i) {
		QByteArray polyColor = device()->read(4);
		if (polyColor.size() != 4) {
			return false;
		}
		polyColors.append(polyColor);
	}

	for (i = 0; i < header.numEdges; ++i) {
		if (device()->read((char *)&edge, 4) != 4) {
			return false;
		}
		edges.append(edge);
	}

	for (i = 0; i < header.numPolys; ++i) {
		if (device()->read((char *)&poly, 24) != 24) {
			return false;
		}
		polys.append(poly);
	}

	for (i = 0; i < header.numHundreds; ++i) {
		if (device()->read((char *)&hundred, 100) != 100) {
			return false;
		}
		hundreds.append(hundred);
		//qDebug() << "hundred" << header.numTexCs << QByteArray((char *)&hundred, 100).toHex();
	}

	for (i = 0; i < header.numGroups; ++i) {
		if (device()->read((char *)&group, 56) != 56) {
			return false;
		}
		groups.append(group);
	}

	for (i = 0; i < header.numBoundingBoxes; ++i) {
		if (device()->read((char *)&boundingBox, sizeof(boundingBox)) != sizeof(boundingBox)) {
			return false;
		}
		boundingBoxes.append(boundingBox);
	}

	for (i = 0; i < header.numVertices; ++i) {
		if (device()->read((char *)&normIndex, sizeof(normIndex)) != sizeof(normIndex)) {
			return false;
		}
		normIndexTable.append(normIndex);
	}

	PolyVertex polyVertex;
	QRgb color;

	for (const Group &g : groups) {
		FieldModelGroup *grp = new FieldModelGroup();

		if (g.areTexturesUsed) {
			if (g.textureNumber < quint32(texIds.size())) {
				// Convert relative group tex IDs to absolute tex IDs
				grp->setTextureRef(new FieldModelTextureRefPC(texIds.at(g.textureNumber)));
			}
		}

		for (quint32 polyID = 0; polyID < g.numPolygons && g.polygonStartIndex + polyID < quint32(polys.size()); ++polyID) {
			const PolygonP &poly = polys.at(g.polygonStartIndex + polyID);
			QList<PolyVertex> polyVertices;
			QList<QRgb> polyColors;
			QList<TexCoord> polyTexCoords;

			for (quint8 j = 0; j < 3; ++j) {
				int vertexIndex = g.verticesStartIndex + poly.VertexIndex[j];

				if (vertexIndex < vertices.size() &&
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

					if (g.areTexturesUsed) {
						int texCoordIndex = g.texCoordStartIndex + poly.VertexIndex[j];

						if (texCoordIndex < texCs.size()) {
							// tex coord
							polyTexCoords.append(texCs.at(texCoordIndex));
						}
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

bool PFile::write(const FieldModelPart *part, const QList<int> &texIds) const
{
	Q_UNUSED(part)
	Q_UNUSED(texIds)
	// TODO
	return false;
}
