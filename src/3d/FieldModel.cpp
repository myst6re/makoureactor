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
#include "FieldModel.h"
#include "core/field/FieldModelFile.h"

FieldModel::FieldModel(QWidget *parent) :
	QOpenGLWidget(parent), blockAll(false), distance(-0.25/*-35*/),
    currentFrame(0), animated(true), data(nullptr),
	xRot(270*16), yRot(90*16), zRot(0), gpuRenderer(nullptr)
{
	connect(&timer, &QTimer::timeout, this, &FieldModel::animate);
}

FieldModel::~FieldModel()
{
	if (gpuRenderer) {
		delete gpuRenderer;
	}
}

void FieldModel::clear()
{
	currentFrame = 0;
	data = nullptr;
	timer.stop();

	if (gpuRenderer) {
		gpuRenderer->reset();
	}

	update();
}

void FieldModel::setFieldModelFile(FieldModelFile *fieldModel)
{
	currentFrame = 0;
	data = fieldModel;
	if (data && data->isValid()) {
		update();
		updateTimer();
	}
}

void FieldModel::setIsAnimated(bool animate)
{
	animated = animate;
	updateTimer();
}

void FieldModel::updateTimer()
{
	if (animated && data && frameCount() > 1) {
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
	return data && data->isValid() ? data->currentAnimation().frameCount() : 0;
}

void FieldModel::drawP(Renderer *gpuRenderer, FieldModelFile *data, float scale, const FieldModelBone &bone, float globalColor[3])
{
	if (scale == 0.0f) {
		return;
	}

	int curPolyType = 0;

	for (FieldModelPart *part : bone.parts()) {
		for (FieldModelGroup *g : part->groups()) {
			bool groupTextureBinded = false;
			if (g->hasTexture()) {
				QImage tex = data->loadedTexture(g);
				if (!tex.isNull()) {
					gpuRenderer->bindTexture(tex);
					groupTextureBinded = true;
				}
			}

			for (const Poly *p : g->polygons()) {
				if (curPolyType != p->count()) {
					if (curPolyType != 0) {
						if (p->count() == 3) {
							gpuRenderer->draw(RendererPrimitiveType::PT_TRIANGLES);
						} else {
							gpuRenderer->draw(RendererPrimitiveType::PT_QUADS);
						}
					}

					curPolyType = p->count();
				}

				QRgba64 color;

				if (p->isMonochrome()) {
					const QRgb &_color = p->color();
					color = QRgba64::fromRgba(qRed(_color) * globalColor[0], qGreen(_color) * globalColor[1], qBlue(_color) * globalColor[2], UINT8_MAX);
				}

				for (int j=0; j<p->count(); ++j) {
					const PolyVertex &vertex = p->vertex(j);
					QVector3D position(vertex.x/scale, vertex.y/scale, vertex.z/scale);
					QVector2D texcoord(0, 0);

					if (!p->isMonochrome()) {
						QRgb _color = p->color(j);
						// TODO: color projector effect
						/*
						float spot = qMax(vertex.x * 0.0f + vertex.y * 0.0f + (1.0 - (vertex.z/scale)) * -1.0f, 0.0f);
						if (spot >= qCos(180.0))
							spot = 1.0;
						else
							spot = qPow(spot, 0.0);
						*/
						color = QRgba64::fromRgba(qRed(_color) * globalColor[0], qGreen(_color) * globalColor[1], qBlue(_color) * globalColor[2], UINT8_MAX);
					}

					if (groupTextureBinded && p->hasTexture()) {
						const TexCoord &_coord = p->texCoord(j);
						texcoord = QVector2D(_coord.x, _coord.y);
					}

					gpuRenderer->bufferVertex(position, color, texcoord);
				}
			}

			if (curPolyType != 0) {
				if (curPolyType == 3) {
					gpuRenderer->draw(RendererPrimitiveType::PT_TRIANGLES);
				} else {
					gpuRenderer->draw(RendererPrimitiveType::PT_QUADS);
				}

				curPolyType = 0;
			}
		}
	}
}

void FieldModel::mouseMoveEvent(QMouseEvent *event)
{
	int dx = event->x() - lastPos.x(); //, dy = event->y() - lastPos.y();

	bool needsUpdate = false;

	if (event->buttons() & Qt::LeftButton) {
		needsUpdate |= setXRotation(xRot + 8 * dx);
		// setYRotation(yRot + 8 * dx);
	}
	if (event->buttons() & Qt::RightButton) {
		// setXRotation(xRot + 8 * dy);
		needsUpdate |= setZRotation(zRot + 8 * dx);
	}
	if (event->buttons() & Qt::MiddleButton) {
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

void FieldModel::initializeGL()
{
	gpuRenderer = new Renderer(this);
}

void FieldModel::resizeGL(int width, int height)
{
	gpuRenderer->setViewport(0, 0, width, height);
}

void FieldModel::paintModel(Renderer *gpuRenderer, FieldModelFile *data, int currentFrame, float scale, QMatrix4x4 initialModelMatrix)
{
	if (!data || !data->isValid() || scale == 0.0f || !gpuRenderer || gpuRenderer->hasError()) {
		return;
	}

	float globalColor[] = { 1.0f, 1.0f, 1.0f };

	// TODO: for PC too
	/* if (!data->translateAfter()) {
		FieldModelFilePS *filePS = static_cast<FieldModelFilePS *>(data);
		globalColor[0] = qRed(filePS->globalColor()) / 255.0f;
		globalColor[1] = qGreen(filePS->globalColor()) / 255.0f;
		globalColor[2] = qBlue(filePS->globalColor()) / 255.0f;
	} */

	if (data->boneCount() <= 1) {
		drawP(gpuRenderer, data, scale, data->bone(0), globalColor);
		return;
	}
	
	QList<PolyVertex> rot = data->currentAnimation().rotations(currentFrame);

	QMatrix4x4 mModel = initialModelMatrix;
	QStack<int> boneStack;
	QStack<QMatrix4x4> matrixStack;
	boneStack.push(-1);

	for (int i = 0; i < data->boneCount(); ++i) {
		const FieldModelBone &bone = data->bone(i);

		while (!boneStack.isEmpty() && boneStack.top() != bone.parent()) {
			boneStack.pop();
			mModel = matrixStack.pop();
		}
		boneStack.push(i);
		matrixStack.push(mModel);

		if (!data->translateAfter()) {
			mModel.translate(0.0, 0.0, bone.size() / scale);
		}

		if (i < rot.size()) {
			const PolyVertex &rotation = rot.at(i);
			mModel.rotate(rotation.y, 0.0, 1.0, 0.0);
			mModel.rotate(rotation.x, 1.0, 0.0, 0.0);
			mModel.rotate(rotation.z, 0.0, 0.0, 1.0);
		}

		gpuRenderer->bindModelMatrix(mModel);

		drawP(gpuRenderer, data, scale, bone, globalColor);

		if (data->translateAfter()) {
			mModel.translate(0.0, 0.0, bone.size() / scale);
		}
	}

	while (!boneStack.isEmpty() && boneStack.top() != -1) {
		boneStack.pop();
		mModel = matrixStack.pop();
	}
}

void FieldModel::paintGL()
{
	if (!data || !data->isValid() || !gpuRenderer || gpuRenderer->hasError()) {
		return;
	}

	//scale the view port if the window manager is scaling
	/* int scale = 1;
	if (qApp->desktop()->physicalDpiX() > 140) {
		scale = 2;
	} */ // TODO

	mProjection.setToIdentity();
	mProjection.perspective(70.0f, (float)width() / (float)height(), 0.001f, 1000.0f);
	gpuRenderer->bindProjectionMatrix(mProjection);

	QMatrix4x4 mView;

	QVector3D eye(distance, 0, 0), center(0, 0, 0), up(0, 0, 1);
	mView.lookAt(eye, center, up);

	mView.rotate(xRot / 16.0f, 1.0, 0.0, 0.0);
	mView.rotate(yRot / 16.0f, 0.0, 1.0, 0.0);
	mView.rotate(zRot / 16.0f, 0.0, 0.0, 1.0);

	gpuRenderer->bindViewMatrix(mView);

	paintModel();

	gpuRenderer->show();
}

void FieldModel::wheelEvent(QWheelEvent *event)
{
	distance += double(event->angleDelta().y())/4096.0;
	update();
}

void FieldModel::mousePressEvent(QMouseEvent *event)
{
	lastPos = event->pos();
	if (event->button() == Qt::MiddleButton) {
		resetCamera();
	}
}

void FieldModel::animate()
{
	if (data && data->isValid() && isVisible()) {
		int count = frameCount();
		if (count > 0) {
			currentFrame = (currentFrame + 1) % count;
			update();
		} else if (currentFrame != 0) {
			currentFrame = 0;
			update();
		}
	}
}
