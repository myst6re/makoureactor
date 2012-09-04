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
	QGLWidget(parent, shareWidget), blockAll(false), distance(-35),
	currentFrame(0), data(0), xRot(270*16), yRot(90*16), zRot(0)
{
	connect(&timer, SIGNAL(timeout()), SLOT(animate()));
}

FieldModel::~FieldModel()
{
}

void FieldModel::clear()
{
	timer.stop();
	currentFrame = 0;
	data = 0;
//	glClearColor(0.0,0.0,0.0,0.0);
	updateGL();
}

bool FieldModel::load(FieldPC *field, const QString &hrc, const QString &a, bool animate)
{
	if(blockAll) {
		return false;
	}

	clear();

	blockAll = true;

	data = field->getFieldModel(hrc, a, animate); // warning: async!
	if(data->isLoaded()) {
		updateGL();
		if(animate && data->frameCount()>1)	timer.start(30);
	}

	blockAll = false;

//	QFile textOut("fieldModelBonePS.txt");
//	textOut.open(QIODevice::WriteOnly);
//	textOut.write(data->toStringBones().toLatin1());
//	textOut.close();

	return true;
}

bool FieldModel::load(Field *field, int modelID, int animationID, bool animate)
{
	if(blockAll) {
		return false;
	}

	clear();

	blockAll = true;

	data = field->getFieldModel(modelID, animationID, animate); // warning: async!
	if(data->isLoaded()) {
		updateGL();
		if(animate && data->frameCount()>1)	timer.start(30);
	}

	blockAll = false;

//	QFile textOut("fieldModelBonePS.txt");
//	textOut.open(QIODevice::WriteOnly);
//	textOut.write(data->toStringBones().toLatin1());
//	textOut.close();

	return true;
}

int FieldModel::nb_bones()
{
	return data ? data->boneCount() : 0;
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

void FieldModel::drawP(int boneID)
{
	if(!data || !data->isLoaded())	return;

	foreach(FieldModelPart *p, data->parts(boneID)) {

		foreach(FieldModelGroup *g, p->groups()) {
			GLuint texture_id = -1;
			bool hasTexture = g->textureNumber() > -1
					&& (int)g->textureNumber() < data->loadedTextureCount();

			if(hasTexture) {
				glEnable(GL_TEXTURE_2D);
				texture_id = bindTexture(data->loadedTexture(g->textureNumber()), GL_TEXTURE_2D, GL_RGBA, QGLContext::MipmapBindOption);
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

				for(quint32 j=0 ; j<(quint8)p->count() ; ++j) {
					if(!p->isMonochrome()) {
						QRgb color = p->color(j);
						glColor3ub(qRed(color), qGreen(color), qBlue(color));
					}

					if(hasTexture && p->hasTexture()) {
						TexCoord coord = p->texCoord(j);
						glTexCoord2d(coord.x, coord.y);
					}

					PolyVertex vertex = p->vertex(j);
					glVertex3f(vertex.x, vertex.y, vertex.z);
				}
			}

			if(curPolyType != 0) {
				glEnd();
			}

			if(hasTexture) {
				deleteTexture(texture_id);

				glDisable(GL_TEXTURE_2D);
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

void FieldModel::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(70, (double)width/(double)height, 0.001, 1000.0);

	glMatrixMode(GL_MODELVIEW);
}

void FieldModel::paintGL()
{
	QStack<int> parent;
	int i;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(!data || !data->isLoaded())	return;

	resizeGL(width(), height()); // hack (?)

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

	if(data->animBoneCount() <= 1) {
		drawP(0);
		return;
	}

	QList<PolyVertex> rot = data->rotation(currentFrame);
	if(rot.isEmpty()) return;
	parent.push(-1);

	for(i=0 ; i<data->animBoneCount() ; ++i)
	{
		const PolyVertex &rotation = rot.at(i);
		const Bone &bone = data->bone(i);

		while(!parent.isEmpty() && parent.top() != bone.parent) {
			parent.pop();
			glPopMatrix();
		}
		parent.push(i);
		glPushMatrix();

		if(data->isPS()) {
			glTranslatef(0.0, 0.0, bone.size);
		}

		glRotatef(rotation.y, 0.0, 1.0, 0.0);
		glRotatef(rotation.x, 1.0, 0.0, 0.0);
		glRotatef(rotation.z, 0.0, 0.0, 1.0);
		drawP(i);

		if(!data->isPS())
			glTranslatef(0.0, 0.0, bone.size);
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
	if(data && data->isLoaded() && isVisible()) {
		currentFrame = (currentFrame + 1) % data->frameCount();
		updateGL();
	}
}
