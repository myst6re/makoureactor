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
#include "Parameters.h"
#include "core/field/FieldModelFilePS.h"

FieldModel::FieldModel(QWidget *parent) :
    QOpenGLWidget(parent), blockAll(false), distance(-0.25/*-35*/),
    animationID(0), currentFrame(0), animated(true), data(nullptr),
	xRot(270*16), yRot(90*16), zRot(0)
{
	connect(&timer, SIGNAL(timeout()), SLOT(animate()));

	QSurfaceFormat format;
	format.setRenderableType(QSurfaceFormat::OpenGL);
	// asks for a OpenGL 2.1 debug context using the Core profile
	format.setVersion(2, 1);
	format.setProfile(QSurfaceFormat::CoreProfile);
#ifdef OPENGL_DEBUG
	format.setOption(QSurfaceFormat::DebugContext);
#endif

	QOpenGLContext *context = new QOpenGLContext;
	context->setFormat(format);
	context->create();

	setFormat(format);
}

FieldModel::~FieldModel()
{
}

void FieldModel::clear()
{
	currentFrame = 0;
	data = nullptr;
	timer.stop();
//	glClearColor(0.0,0.0,0.0,0.0);
	update();
}

void FieldModel::setFieldModelFile(FieldModelFile *fieldModel, int animID)
{
	currentFrame = 0;
	data = fieldModel;
	animationID = animID;
	if(data && data->isValid()) {
		update();
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
		update();
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
	initializeOpenGLFunctions();
	// GLfloat ambient[] = {0.8f, 0.8f, 0.8f, 1.0f};
	// glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	gluPerspective(70, (double)width()/(double)height(), 0.001, 1000.0);
}

void FieldModel::drawP(FieldModelFile *data, float scale,
                       const FieldModelBone &bone,
                       const QHash<void *, QOpenGLTexture *> &textures,
                       float globalColor[3], QOpenGLTexture *&texture)
{
	if (scale == 0.0f) {
		return;
	}

	int curPolyType = 0;

	foreach(FieldModelPart *part, bone.parts()) {

		foreach(FieldModelGroup *g, part->groups()) {
			if(g->hasTexture()) {
				if(curPolyType != 0) {
					glEnd();
					curPolyType = 0;
				}
				QOpenGLTexture *tex = textures.value(data->textureIdForGroup(g));
				if (texture && tex != texture) {
					texture->release();
				}
				tex->bind();
				texture = tex;
			} else if (texture) {
				texture->release();
				texture = nullptr;
			}

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
					glColor3ub(qRed(color) * globalColor[0],
					           qGreen(color) * globalColor[1],
					           qBlue(color) * globalColor[2]);
				}

				for(int j=0 ; j<p->count() ; ++j) {
					const PolyVertex &vertex = p->vertex(j);

					if(!p->isMonochrome()) {
						QRgb color = p->color(j);
						// TODO: color projector effect
						/* float spot = qMax(vertex.x * 0.0f + vertex.y * 0.0f + (1.0 - (vertex.z/scale)) * -1.0f, 0.0f);
						if (spot >= qCos(180.0)) {
							spot = 1.0;
						} else {
							spot = qPow(spot, 0.0);
						} */
						glColor3ub(qRed(color) * globalColor[0],
						           qGreen(color) * globalColor[1],
						           qBlue(color) * globalColor[2]);
					}

					if(g->hasTexture() && p->hasTexture()) {
						const TexCoord &coord = p->texCoord(j);
						glTexCoord2d(coord.x, coord.y);
					}

					glVertex3f(vertex.x/scale, vertex.y/scale, vertex.z/scale);
				}
			}
		}
	}



	if(curPolyType != 0) {
		glEnd();
	}
}

void FieldModel::mouseMoveEvent(QMouseEvent *event)
{
	int dx = event->x() - lastPos.x(),
	    dy = event->y() - lastPos.y();

	bool needsUpdate = false;

	if (event->buttons() & Qt::LeftButton) {
		needsUpdate |= setXRotation(xRot + 8 * dx);
		//setYRotation(yRot + 8 * dx);
	}
	if (event->buttons() & Qt::RightButton) {
		//setXRotation(xRot + 8 * dy);
		needsUpdate |= setZRotation(zRot + 8 * dx);
	}
	if (event->buttons() & Qt::MidButton) {
		needsUpdate |= setYRotation(yRot + 8 * dx);
	}

	if (needsUpdate) {
		update();
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

bool FieldModel::setXRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != xRot) {
		xRot = angle;
		return true;
	}

	return false;
}

bool FieldModel::setYRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != yRot) {
		yRot = angle;
		return true;
	}

	return false;
}

bool FieldModel::setZRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != zRot) {
		zRot = angle;
		return true;
	}

	return false;
}

void FieldModel::resetCamera()
{
	distance = -0.25/*-35*/;
	xRot = 270*16;
	yRot = 90*16;
	zRot = 0;
	update();
}

void FieldModel::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(70, (double)width/(double)height, 0.001, 1000.0);

	glMatrixMode(GL_MODELVIEW);
}

void FieldModel::paintModel(FieldModelFile *data, int animationID,
                            int currentFrame, float scale)
{
	if(!data || !data->isValid() || scale == 0.0f) {
		return;
	}

	float globalColor[] = { 1.0f, 1.0f, 1.0f };

	/* if(!data->translateAfter()) { // TODO: for PC too
		FieldModelFilePS *filePS = static_cast<FieldModelFilePS *>(data);
		globalColor[0] = qRed(filePS->globalColor()) / 255.0f;
		globalColor[1] = qGreen(filePS->globalColor()) / 255.0f;
		globalColor[2] = qBlue(filePS->globalColor()) / 255.0f;
	} */

	QStack<int> parent;
	int i;
	QHash<void *, QOpenGLTexture *> textures;
	QHashIterator<void *, QImage> it(data->loadedTextures());

	while (it.hasNext()) {
		it.next();
		QOpenGLTexture *tex = new QOpenGLTexture(it.value());
		tex->setMinificationFilter(QOpenGLTexture::NearestMipMapLinear);
		tex->setMagnificationFilter(QOpenGLTexture::Nearest);
		textures.insert(it.key(), tex);
	}

	QOpenGLTexture *texture = nullptr;

	if(data->boneCount() <= 1) {
		drawP(data, scale, data->bone(0), textures, globalColor, texture);

		qDeleteAll(textures);
		return;
	}

	if (animationID >= data->animationCount()) {
		qDeleteAll(textures);
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

		drawP(data, scale, bone, textures, globalColor, texture);

		if(data->translateAfter()) {
			glTranslatef(0.0, 0.0, bone.size() / scale);
		}
	}

	while(!parent.isEmpty() && parent.top() != -1) {
		parent.pop();
		glPopMatrix();
	}

	qDeleteAll(textures);
}

void FieldModel::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(!data || !data->isValid())	return;

#ifdef OPENGL_DEBUG
	QOpenGLContext *ctx = QOpenGLContext::currentContext();
	QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
	logger->initialize();
#endif

	//scale the view port if the window manager is scaling
	int scale = 1;
	if(qApp->desktop()->physicalDpiX() > 140) {
		scale = 2;
	}

	resize(width() * scale, height() * scale); // hack (?)

	gluLookAt(distance,0,0,0,0,0,0,0,1);
//	glTranslatef(distance, 0.0f, 0.0f);

//	glRotatef(270.0, 1.0, 0.0, 0.0);
//	glRotatef(90.0, 0.0, 1.0, 0.0);
//	glRotatef(0.0, 0.0, 0.0, 1.0);
	glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
	glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
	glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);

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

#ifdef OPENGL_DEBUG
	const QList<QOpenGLDebugMessage> messages = logger->loggedMessages();
	for (const QOpenGLDebugMessage &message : messages) {
		qDebug() << message;
	}
#endif
}

void FieldModel::wheelEvent(QWheelEvent *event)
{
	distance += double(event->delta())/4096.0;
	update();
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
	if(data && data->isValid() && isVisible()) {
		int count = frameCount();
		if(count > 0) {
			currentFrame = (currentFrame + 1) % count;
			update();
		} else if(currentFrame != 0) {
			currentFrame = 0;
			update();
		}
	}
}
