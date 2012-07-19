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
#include "FieldModel.h"

FieldModel::FieldModel(QWidget *parent, const QGLWidget *shareWidget)
	: QGLWidget(parent, shareWidget), blockAll(false), distance(-35), currentFrame(0), xRot(270*16), yRot(90*16), zRot(0)
{
	setFixedSize(304, 214);
	connect(&timer, SIGNAL(timeout()), SLOT(animate()));
}

void FieldModel::clear()
{
	timer.stop();
	currentFrame = 0;
	data.clear();
	glClearColor(0.0,0.0,0.0,0.0);
}

quint8 FieldModel::load(const QString &hrc, const QString &a, bool animate)
{
	if(blockAll) {
		return 2;
	}

	clear();

	blockAll = true;

	quint8 err = data.load(hrc, a, animate);
	if(err==1) {
		updateGL();
		if(animate && data.frames.size()>1)	timer.start(30);
	}

	blockAll = false;

	return err;
}

quint8 FieldModel::load(const QByteArray &BSX_data, int model_id)
{
	return data.load(BSX_data, model_id);
}

int FieldModel::nb_bones()
{
	return data.bones.size();
}

void FieldModel::initializeGL()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gluPerspective(70, 1, 1, 1000);
}

void FieldModel::drawP(int index)
{
	Color color;
	color.blue = color.red = color.green = color.alpha = 0;
	Coord coord;
	coord.x = coord.y = 0;
	VertexPC vertex;
	vertex.x = vertex.y = vertex.z = 0;
	quint32 polyID, j;
	QList<FieldModelPart> ps = data.P_files.values(index);
	QList<QList<int> > texss = data.tex_files.values(index);
	QList<int> texs;

	for(int pID=0 ; pID<ps.size() ; ++pID)
	{
		const FieldModelPart &p = ps.at(pID);
		texs = texss.value(pID, QList<int>());

		foreach(const Group &g, p.groups) {
			if(g.areTexturesUsed && (int)g.textureNumber<texs.size()) {
				glEnable(GL_TEXTURE_2D);

				GLuint texture_id = bindTexture(data.loaded_tex.value(texs.value(g.textureNumber, -1), QPixmap()), GL_TEXTURE_2D, GL_RGBA, QGLContext::MipmapBindOption);

				glBegin(GL_TRIANGLES);
				//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

				for(polyID=0 ; polyID<g.numPolygons ; ++polyID) {
					const Polygon_p &poly = p.polys.at(g.polygonStartIndex+polyID);
					for(j=0 ; j<3 ; ++j) {
						color = p.vertexColors.value(g.verticesStartIndex+poly.VertexIndex[j], color);
						glColor3ub(color.red, color.green, color.blue);

						coord = p.texCs.value(g.texCoordStartIndex+poly.VertexIndex[j], coord);
						glTexCoord2d(coord.x, coord.y);

						vertex = p.vertices.value(g.verticesStartIndex+poly.VertexIndex[j], vertex);
						glVertex3f(vertex.x, vertex.y, vertex.z);
					}
				}

				glEnd();

				deleteTexture(texture_id);

				glDisable(GL_TEXTURE_2D);
			}
			else {
				glBegin(GL_TRIANGLES);
				//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

				for(polyID=0 ; polyID<g.numPolygons ; ++polyID) {
					const Polygon_p &poly = p.polys.at(g.polygonStartIndex+polyID);
					for(j=0 ; j<3 ; ++j) {
						color = p.vertexColors.value(g.verticesStartIndex+poly.VertexIndex[j], color);
						glColor3ub(color.red, color.green, color.blue);

						//vertex = p.normals.value(poly.NormalIndex[j], vertex);
						//glNormal3f(vertex.x, vertex.y, vertex.z);

						vertex = p.vertices.value(g.verticesStartIndex+poly.VertexIndex[j], vertex);
						glVertex3f(vertex.x, vertex.y, vertex.z);
					}
				}

				glEnd();
			}
		}
	}
}

void FieldModel::mouseMoveEvent(QMouseEvent *event)
{
	int dx = event->x() - lastPos.x();
	int dy = event->y() - lastPos.y();

	if (event->buttons() & Qt::LeftButton) {
		setXRotation(xRot + 8 * dy);
		//setYRotation(yRot + 8 * dx);
	}
	else if (event->buttons() & Qt::RightButton) {
		//setXRotation(xRot + 8 * dy);
		setZRotation(zRot + 8 * dx);
	}
	else {
		setYRotation(yRot + 8 * dx);
	}
	lastPos = event->pos();
}

static void qNormalizeAngle(int &angle)
{
	while (angle < 0)
		angle += 360 * 16;
	while (angle > 360 * 16)
		angle -= 360 * 16;
}

void FieldModel::setXRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != xRot) {
		xRot = angle;
		updateGL();
	}
}

void FieldModel::setYRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != yRot) {
		yRot = angle;
		updateGL();
	}
}

void FieldModel::setZRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != zRot) {
		zRot = angle;
		updateGL();
	}
}

void FieldModel::paintGL()
{
	QStack<int> parent;
	int i;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(!data.isLoaded())	return;

	gluLookAt(distance,0,0,0,0,0,0,0,1);

//	glRotatef(270.0, 1.0, 0.0, 0.0);
//	glRotatef(90.0, 0.0, 1.0, 0.0);
//	glRotatef(0.0, 0.0, 0.0, 1.0);
	glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
	glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
	glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);

//	qDebug() << (xRot / 16.0) << (yRot / 16.0) << (zRot / 16.0);

//	glBegin(GL_LINES);
//	glColor3f(0.0, 0.0, 1.0);
//	glVertex3f(0.0, 0.0, 0.0);
//	glVertex3f(20.0, 0.0, 0.0);
//	glColor3f(0.0, 0.0, 0.5);
//	glVertex3f(0.0, 0.0, 0.0);
//	glVertex3f(-20.0, 0.0, 0.0);
//	glColor3f(0.0, 1.0, 0.0);
//	glVertex3f(0.0, 0.0, 0.0);
//	glVertex3f(0.0, 20.0, 0.0);
//	glColor3f(0.0, 0.5, 0.0);
//	glVertex3f(0.0, 0.0, 0.0);
//	glVertex3f(0.0, -20.0, 0.0);
//	glColor3f(1.0, 0.0, 0.0);
//	glVertex3f(0.0, 0.0, 0.0);
//	glVertex3f(0.0, 0.0, 20.0);
//	glColor3f(0.5, 0.0, 0.0);
//	glVertex3f(0.0, 0.0, 0.0);
//	glVertex3f(0.0, 0.0, -20.0);
//	glEnd();

	if(data.a_bones_count<=1) {
		drawP(0);
		return;
	}

	QList<VertexPC> rot = data.frames.value(currentFrame, QList<VertexPC>());
	if(rot.isEmpty()) return;
	parent.push(-1);

	for(i=0 ; i<data.a_bones_count ; ++i)
	{
		const VertexPC &rotation = rot.at(i);
		const Bone &bone = data.bones.at(i);

		while(!parent.isEmpty() && parent.top() != bone.parent) {
			parent.pop();
			glPopMatrix();
		}
		parent.push(i);
		glPushMatrix();

		glRotatef(rotation.y, 0.0, 1.0, 0.0);
		glRotatef(rotation.x, 1.0, 0.0, 0.0);
		glRotatef(rotation.z, 0.0, 0.0, 1.0);
		drawP(i);

		glTranslatef(0.0, 0.0, -bone.size);
	}

	while(parent.top() != -1) {
		parent.pop();
		glPopMatrix();
	}
}

void FieldModel::wheelEvent(QWheelEvent *event)
{
	distance += event->delta()/120;
	updateGL();
}

void FieldModel::mousePressEvent(QMouseEvent *event)
{
	lastPos = event->pos();
	if(event->button()==Qt::MidButton)
	{
		distance = -35;
		updateGL();
	}
}

void FieldModel::animate()
{
	if(isVisible()) {
		currentFrame = (currentFrame + 1) % data.frames.size();
		updateGL();
	}
}
