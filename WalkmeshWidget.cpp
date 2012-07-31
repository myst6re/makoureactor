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
#include "WalkmeshWidget.h"

WalkmeshWidget::WalkmeshWidget(WalkmeshFile *walkmesh, QWidget *parent)
	: QGLWidget(parent), distance(0.0), xRot(0.0), yRot(0.0), zRot(0.0), walkmesh(walkmesh)
{
	setFixedSize(640, 480);
}

void WalkmeshWidget::initializeGL()
{
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void WalkmeshWidget::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if(walkmesh->isOpen()) {
		double fovy = (2 * atan(240.0/(2.0 * walkmesh->camera_zoom))) * 57.29577951;
		gluPerspective(fovy, (double)width/(double)height, 0.001, 1000.0);
	} else {
		gluPerspective(70.0, (double)width/(double)height, 0.001, 1000.0);
	}

	glMatrixMode(GL_MODELVIEW);
}

void WalkmeshWidget::paintGL()
{
	Vertex_sr vertex;
//	const qint16 *acc;
	int /*i=0,*/ j;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(!walkmesh->isOpen())	return;

	double camAxisXx = walkmesh->camera_axis[0].x/4096.0;
	double camAxisXy = walkmesh->camera_axis[0].y/4096.0;
	double camAxisXz = walkmesh->camera_axis[0].z/4096.0;

	double camAxisYx = -walkmesh->camera_axis[1].x/4096.0;
	double camAxisYy = -walkmesh->camera_axis[1].y/4096.0;
	double camAxisYz = -walkmesh->camera_axis[1].z/4096.0;

	double camAxisZx = walkmesh->camera_axis[2].x/4096.0;
	double camAxisZy = walkmesh->camera_axis[2].y/4096.0;
	double camAxisZz = walkmesh->camera_axis[2].z/4096.0;

	double camPosX = walkmesh->camera_position[0]/4096.0;
	double camPosY = -walkmesh->camera_position[1]/4096.0;
	double camPosZ = walkmesh->camera_position[2]/4096.0;

	double tx = -(camPosX*camAxisXx + camPosY*camAxisYx + camPosZ*camAxisZx);
	double ty = -(camPosX*camAxisXy + camPosY*camAxisYy + camPosZ*camAxisZy);
	double tz = -(camPosX*camAxisXz + camPosY*camAxisYz + camPosZ*camAxisZz);

//	qDebug() << posCamX << posCamY << posCamZ;
//	gluLookAt(distance,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0);
	gluLookAt(tx, ty, tz, tx + camAxisZx, ty + camAxisZy, tz + camAxisZz, camAxisYx, camAxisYy, camAxisYz);

	glRotatef(xRot, 1.0, 0.0, 0.0);
	glRotatef(yRot, 0.0, 1.0, 0.0);
	glRotatef(zRot, 0.0, 0.0, 1.0);

	//glTranslatef(xtrans, ytrans, ztrans);

	glBegin(GL_LINES);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);
	glColor3f(0.0, 0.0, 0.5);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(-1.0, 0.0, 0.0);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0);
	glColor3f(0.0, 0.5, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, -1.0, 0.0);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 1.0);
	glColor3f(0.5, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, -1.0);
	glEnd();

	glBegin(GL_TRIANGLES);

	foreach(const Triangle &triangle, walkmesh->triangles) {
//		acc = walkmesh->access.at(i++).a;
		for(j=0 ; j<3 ; ++j) {
			vertex = triangle.vertices[j];
			glColor3ub(0xFF, 0xFF, 0xFF);
			glVertex3f(vertex.x/4096.0, vertex.y/4096.0, vertex.z/4096.0);
//			renderText(vertex.x/4096.0, vertex.y/4096.0, vertex.z/4096.0, QString::number(acc[j]));
		}
	}

	glEnd();

}

void WalkmeshWidget::wheelEvent(QWheelEvent *event)
{
	distance += event->delta()/120;
	updateGL();
}

void WalkmeshWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button()==Qt::MidButton)
	{
		distance = -35;
		updateGL();
	}
}

static void qNormalizeAngle(int &angle)
{
	while (angle < 0)
		angle += 360 * 16;
	while (angle > 360 * 16)
		angle -= 360 * 16;
}

void WalkmeshWidget::setXRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != xRot) {
		xRot = angle;
		updateGL();
	}
}

void WalkmeshWidget::setYRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != yRot) {
		yRot = angle;
		updateGL();
	}
}

void WalkmeshWidget::setZRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != zRot) {
		zRot = angle;
		updateGL();
	}
}

void WalkmeshWidget::setZoom(int zoom)
{
	distance = zoom / 4096.0;
}
