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

FieldModel::FieldModel(QWidget *parent, const QGLWidget *shareWidget) :
	QGLWidget(parent, shareWidget), blockAll(false), distance(-0.25/*-35*/),
	animationID(0), currentFrame(0), animated(true), data(0),
	xRot(270*16), yRot(90*16), zRot(0)
{
	connect(&timer, SIGNAL(timeout()), SLOT(animate()));
}

FieldModel::~FieldModel()
{
}

void FieldModel::clear()
{
	currentFrame = 0;
	data = 0;
	updateTimer();
//	glClearColor(0.0,0.0,0.0,0.0);
	updateGL();
}

void FieldModel::setFieldModelFile(FieldModelFile *fieldModel, int animID)
{
	currentFrame = 0;
	data = fieldModel;
	animationID = animID;
	if(!data->isEmpty()) {
		updateGL();
		updateTimer();
	}
}

void FieldModel::setIsAnimated(bool animate)
{
	animated = animate;
	updateTimer();
}

void FieldModel::setAnimationID(int animID)
{
	if (animationID != animID) {
		animationID = animID;
		updateGL();
		updateTimer();
	}
}

void FieldModel::updateTimer()
{
	if(animated && data && frameCount() > 1) {
		timer.start(30);
	} else {
		timer.stop();
	}
}

int FieldModel::boneCount() const
{
	return data ? data->boneCount() : 0;
}

int FieldModel::frameCount() const
{
	return data && animationID < data->animationCount()
			? data->animation(animationID).frameCount()
			: 0;
}

void FieldModel::initializeGL()
{
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	gluPerspective(70, (double)width()/(double)height(), 0.001, 1000.0);
}

void FieldModel::drawP(QGLWidget *glWidget, FieldModelFile *data, float scale, const FieldModelBone &bone,
					   GLuint &texture_id, quint64 &lastTexID)
{
	if (scale == 0.0f) {
		return;
	}

	foreach(FieldModelPart *part, bone.parts()) {

		foreach(FieldModelGroup *g, part->groups()) {
			if(g->hasTexture()) {
				if(texture_id != (GLuint)-1) {
					glWidget->deleteTexture(texture_id);
				} else {
					glEnable(GL_TEXTURE_2D);
				}
				texture_id = glWidget->bindTexture(data->loadedTexture(g), GL_TEXTURE_2D, GL_RGBA, QGLContext::MipmapBindOption);
			} else if(!g->hasTexture() && texture_id != (GLuint)-1) {
				glWidget->deleteTexture(texture_id);
				glDisable(GL_TEXTURE_2D);
				texture_id = -1;
				lastTexID = quint64(-1);
			}

			int curPolyType = 0;

			foreach(const Poly *p, g->polygons()) {
				if(curPolyType != p->count()) {
					if(curPolyType != 0) {
						glEnd();
					}

					if(p->count() == 3) {
						glBegin(GL_TRIANGLES);
					} else {
						glBegin(GL_QUADS);
					}
					curPolyType = p->count();
				}

				if(p->isMonochrome()) {
					const QRgb &color = p->color();
					glColor3ub(qRed(color), qGreen(color), qBlue(color));
				}

				for(quint16 j=0 ; j<(quint8)p->count() ; ++j) {
					if(!p->isMonochrome()) {
						QRgb color = p->color(j);
						glColor3ub(qRed(color), qGreen(color), qBlue(color));
					}

					if(g->hasTexture() && p->hasTexture()) {
						const TexCoord &coord = p->texCoord(j);
						glTexCoord2d(coord.x, coord.y);
					}

					const PolyVertex &vertex = p->vertex(j);
					glVertex3f(vertex.x/scale, vertex.y/scale, vertex.z/scale);
				}
			}

			if(curPolyType != 0) {
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

void FieldModel::resetCamera()
{
	distance = -0.25/*-35*/;
	xRot = 270*16;
	yRot = 90*16;
	zRot = 0;
	updateGL();
}

void FieldModel::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(70, (double)width/(double)height, 0.001, 1000.0);

	glMatrixMode(GL_MODELVIEW);
}

void FieldModel::paintModel(QGLWidget *glWidget, FieldModelFile *data, int animationID, int currentFrame, float scale)
{
	if(!data || data->isEmpty() || scale == 0.0f) {
		return;
	}

	QStack<int> parent;
	int i;
	GLuint texture_id = -1;
	quint64 lastTexID = quint64(-1);

	if(data->boneCount() <= 1) {
		drawP(glWidget, data, scale, data->bone(0), texture_id, lastTexID);
		if(texture_id != (GLuint)-1) {
			glWidget->deleteTexture(texture_id);
			glDisable(GL_TEXTURE_2D);
		}
		return;
	}

	if (animationID >= data->animationCount()) {
		return;
	}

	QList<PolyVertex> rot = data->animation(animationID).rotations(currentFrame);
	parent.push(-1);

	for(i = 0 ; i < data->boneCount() ; ++i) {
		const FieldModelBone &bone = data->bone(i);

		while(!parent.isEmpty() && parent.top() != bone.parent()) {
			parent.pop();
			glPopMatrix();
		}
		parent.push(i);
		glPushMatrix();

		if(!data->translateAfter()) {
			glTranslatef(0.0, 0.0, bone.size() / scale);
		}

		if (i < rot.size()) {
			const PolyVertex &rotation = rot.at(i);
			glRotatef(rotation.y, 0.0, 1.0, 0.0);
			glRotatef(rotation.x, 1.0, 0.0, 0.0);
			glRotatef(rotation.z, 0.0, 0.0, 1.0);
		}

		drawP(glWidget, data, scale, bone, texture_id, lastTexID);

		if(data->translateAfter()) {
			glTranslatef(0.0, 0.0, bone.size() / scale);
		}
	}

	if(texture_id != (GLuint)-1) {
		glWidget->deleteTexture(texture_id);
		glDisable(GL_TEXTURE_2D);
	}

	while(!parent.isEmpty() && parent.top() != -1) {
		parent.pop();
		glPopMatrix();
	}
}

void FieldModel::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(!data || data->isEmpty())	return;

	//scale the view port if the window manager is scaling
	int scale = 1;
	if(qApp->desktop()->physicalDpiX() > 140){scale = 2;}

	resizeGL(width() * scale, height() * scale); // hack (?)

	gluLookAt(distance,0,0,0,0,0,0,0,1);
//	glTranslatef(distance, 0.0f, 0.0f);

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

	paintModel();
}

void FieldModel::wheelEvent(QWheelEvent *event)
{
	distance += double(event->delta())/4096.0;
	updateGL();
}

void FieldModel::mousePressEvent(QMouseEvent *event)
{
	lastPos = event->pos();
	if(event->button() == Qt::MidButton) {
		resetCamera();
	}
}

void FieldModel::animate()
{
	if(data && !data->isEmpty() && isVisible()) {
		currentFrame = (currentFrame + 1) % frameCount();
		updateGL();
	}
}
