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

WalkmeshWidget::WalkmeshWidget(QWidget *parent, const QGLWidget *shareWidget) :
	QGLWidget(parent, shareWidget), distance(0.0f), xRot(0.0f), yRot(0.0f), zRot(0.0f),
	xTrans(0.0f), yTrans(0.0f), transStep(360.0f), lastKeyPressed(-1),
	camID(0), _selectedTriangle(-1), _selectedDoor(-1), _selectedGate(-1),
	_selectedArrow(-1), fovy(70.0), walkmesh(0), camera(0), infFile(0)
{
	setMinimumSize(640, 480);
}

void WalkmeshWidget::clear()
{
	walkmesh = 0;
	camera = 0;
	infFile = 0;
	updateGL();
}

void WalkmeshWidget::fill(IdFile *walkmesh, CaFile *camera, InfFile *infFile)
{
	this->walkmesh = walkmesh;
	this->camera = camera;
	this->infFile = infFile;
	updatePerspective();
}

void WalkmeshWidget::computeFov()
{
	if(camera && camera->isOpen()
			&& camera->hasCamera()
			&& camID < camera->cameraCount()) {
		const Camera &cam = camera->camera(camID);
		fovy = (2 * atan(240.0/(2.0 * cam.camera_zoom))) * 57.29577951;
	} else {
		fovy = 70.0;
	}
}

void WalkmeshWidget::updatePerspective()
{
	computeFov();
	resizeGL(width(), height());
	updateGL();
}

void WalkmeshWidget::initializeGL()
{
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

	gluPerspective(fovy, (double)width/(double)height, 0.001, 1000.0);

	glMatrixMode(GL_MODELVIEW);
}

void WalkmeshWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(!walkmesh)	return;

	glTranslatef(xTrans, yTrans, distance);
	glRotatef(xRot, 1.0, 0.0, 0.0);
	glRotatef(yRot, 0.0, 1.0, 0.0);
	glRotatef(zRot, 0.0, 0.0, 1.0);

	if(camera->isOpen() && camera->hasCamera() && camID < camera->cameraCount()) {
		const Camera &cam = camera->camera(camID);

		double camAxisXx = cam.camera_axis[0].x/4096.0;
		double camAxisXy = cam.camera_axis[0].y/4096.0;
		double camAxisXz = cam.camera_axis[0].z/4096.0;

		double camAxisYx = -cam.camera_axis[1].x/4096.0;
		double camAxisYy = -cam.camera_axis[1].y/4096.0;
		double camAxisYz = -cam.camera_axis[1].z/4096.0;

		double camAxisZx = cam.camera_axis[2].x/4096.0;
		double camAxisZy = cam.camera_axis[2].y/4096.0;
		double camAxisZz = cam.camera_axis[2].z/4096.0;

		double camPosX = cam.camera_position[0]/4096.0;
		double camPosY = -cam.camera_position[1]/4096.0;
		double camPosZ = cam.camera_position[2]/4096.0;

		double tx = -(camPosX*camAxisXx + camPosY*camAxisYx + camPosZ*camAxisZx);
		double ty = -(camPosX*camAxisXy + camPosY*camAxisYy + camPosZ*camAxisZy);
		double tz = -(camPosX*camAxisXz + camPosY*camAxisYz + camPosZ*camAxisZz);


		gluLookAt(tx, ty, tz, tx + camAxisZx, ty + camAxisZy, tz + camAxisZz, camAxisYx, camAxisYy, camAxisYz);
	}

	if(walkmesh->isOpen()) {

		/*glBegin(GL_LINES);
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
		glEnd();*/

		glColor3ub(0xFF, 0xFF, 0xFF);

		glBegin(GL_LINES);

		int i=0;

		foreach(const Triangle &triangle, walkmesh->getTriangles()) {
			const Access &access = walkmesh->access(i);

			drawIdLine(i, triangle.vertices[0], triangle.vertices[1], access.a[0]);
			drawIdLine(i, triangle.vertices[1], triangle.vertices[2], access.a[1]);
			drawIdLine(i, triangle.vertices[2], triangle.vertices[0], access.a[2]);

			++i;
		}

		if(infFile && infFile->isOpen()) {
			glColor3ub(0xFF, 0x00, 0x00);

			foreach(const Exit &gate, infFile->exitLines()) {
				if(gate.fieldID != 0x7FFF) {
					Vertex_s vertex;
					vertex = gate.exit_line[0];
					glVertex3f(vertex.x/4096.0f, vertex.y/4096.0f, vertex.z/4096.0f);
					vertex = gate.exit_line[1];
					glVertex3f(vertex.x/4096.0f, vertex.y/4096.0f, vertex.z/4096.0f);
				}
			}

			glColor3ub(0x00, 0xFF, 0x00);

			foreach(const Trigger &trigger, infFile->triggers()) {
				if(trigger.background_parameter != 0xFF) {
					Vertex_s vertex;
					vertex = trigger.trigger_line[0];
					glVertex3f(vertex.x/4096.0f, vertex.y/4096.0f, vertex.z/4096.0f);
					vertex = trigger.trigger_line[1];
					glVertex3f(vertex.x/4096.0f, vertex.y/4096.0f, vertex.z/4096.0f);
				}
			}
		}

		glEnd();

		glPointSize(7.0);

		glBegin(GL_POINTS);

		glColor3ub(0xFF, 0x90, 0x00);

		if(_selectedTriangle >= 0 && _selectedTriangle < walkmesh->triangleCount()) {
			const Triangle &triangle = walkmesh->triangle(_selectedTriangle);
			const Vertex_sr &vertex1 = triangle.vertices[0];
			glVertex3f(vertex1.x/4096.0f, vertex1.y/4096.0f, vertex1.z/4096.0f);
			const Vertex_sr &vertex2 = triangle.vertices[1];
			glVertex3f(vertex2.x/4096.0f, vertex2.y/4096.0f, vertex2.z/4096.0f);
			const Vertex_sr &vertex3 = triangle.vertices[2];
			glVertex3f(vertex3.x/4096.0f, vertex3.y/4096.0f, vertex3.z/4096.0f);
		}

		if(infFile && infFile->isOpen()) {
			glColor3ub(0xFF, 0x00, 0x00);

			if(_selectedGate >= 0 && _selectedGate < 12) {
				const Exit &gate = infFile->exitLine(_selectedGate);
				if(gate.fieldID != 0x7FFF) {
					const Vertex_s &vertex1 = gate.exit_line[0];
					glVertex3f(vertex1.x/4096.0f, vertex1.y/4096.0f, vertex1.z/4096.0f);
					const Vertex_s &vertex2 = gate.exit_line[1];
					glVertex3f(vertex2.x/4096.0f, vertex2.y/4096.0f, vertex2.z/4096.0f);
				}
			}

			glColor3ub(0x00, 0xFF, 0x00);

			if(_selectedDoor >= 0 && _selectedDoor < 12) {
				const Trigger &trigger = infFile->trigger(_selectedDoor);
				if(trigger.background_parameter != 0xFF) {
					const Vertex_s &vertex1 = trigger.trigger_line[0];
					glVertex3f(vertex1.x/4096.0f, vertex1.y/4096.0f, vertex1.z/4096.0f);
					const Vertex_s &vertex2 = trigger.trigger_line[1];
					glVertex3f(vertex2.x/4096.0f, vertex2.y/4096.0f, vertex2.z/4096.0f);
				}
			}
		}

		glEnd();
	}
}

void WalkmeshWidget::drawIdLine(int triangleID, const Vertex_sr &vertex1, const Vertex_sr &vertex2, qint16 access)
{
	if(triangleID == _selectedTriangle) {
		glColor3ub(0xFF, 0x90, 0x00);
	} else if(access == -1) {
		glColor3ub(0x66, 0x99, 0xCC);
	} else {
		glColor3ub(0xFF, 0xFF, 0xFF);
	}

	glVertex3f(vertex1.x/4096.0f, vertex1.y/4096.0f, vertex1.z/4096.0f);
	glVertex3f(vertex2.x/4096.0f, vertex2.y/4096.0f, vertex2.z/4096.0f);
}

void WalkmeshWidget::wheelEvent(QWheelEvent *event)
{
	distance += event->delta() / 4096.0;
	updateGL();
}

void WalkmeshWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button()==Qt::MidButton)
	{
		distance = -35;
		updateGL();
	}
	else if(event->button()==Qt::LeftButton)
	{
		moveStart = event->pos();
	}
}

void WalkmeshWidget::mouseMoveEvent(QMouseEvent *event)
{
	xTrans += (event->pos().x() - moveStart.x()) / 4096.0;
	yTrans -= (event->pos().y() - moveStart.y()) / 4096.0;
	moveStart = event->pos();
	updateGL();
}

void WalkmeshWidget::keyPressEvent(QKeyEvent *event)
{
	if(lastKeyPressed == event->key()
			&& (event->key() == Qt::Key_Left
				|| event->key() == Qt::Key_Right
				|| event->key() == Qt::Key_Down
				|| event->key() == Qt::Key_Up)) {
		if(transStep > 100.0f) {
			transStep *= 0.90f; // accelerator
		}
	} else {
		transStep = 360.0f;
	}
	lastKeyPressed = event->key();

	switch(event->key())
	{
	case Qt::Key_Left:
		xTrans += 1.0f/transStep;
		updateGL();
		break;
	case Qt::Key_Right:
		xTrans -= 1.0f/transStep;
		updateGL();
		break;
	case Qt::Key_Down:
		yTrans += 1.0f/transStep;
		updateGL();
		break;
	case Qt::Key_Up:
		yTrans -= 1.0f/transStep;
		updateGL();
		break;
	default:
		QWidget::keyPressEvent(event);
		return;
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

void WalkmeshWidget::resetCamera()
{
	distance = 0;
	zRot = yRot = xRot = 0;
	xTrans = yTrans = 0;
	updateGL();
}

void WalkmeshWidget::setCurrentFieldCamera(int camID)
{
	this->camID = camID;
	updatePerspective();
}

void WalkmeshWidget::setSelectedTriangle(int triangle)
{
	_selectedTriangle = triangle;
	updateGL();
}

void WalkmeshWidget::setSelectedDoor(int door)
{
	_selectedDoor = door;
	updateGL();
}

void WalkmeshWidget::setSelectedGate(int gate)
{
	_selectedGate = gate;
	updateGL();
}

void WalkmeshWidget::setSelectedArrow(int arrow)
{
	_selectedArrow = arrow;
	updateGL();
}
