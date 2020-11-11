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
#include "Parameters.h"
#include "FieldModel.h"

WalkmeshWidget::WalkmeshWidget(QWidget *parent)
    : QOpenGLWidget(parent), distance(0.0f),
      xRot(0.0f), yRot(0.0f), zRot(0.0f), xTrans(0.0f), yTrans(0.0f),
      transStep(360.0f), lastKeyPressed(-1), _camID(0), _selectedTriangle(-1),
      _selectedDoor(-1), _selectedGate(-1), _selectedArrow(-1), fovy(70.0),
      walkmesh(nullptr), camera(nullptr), infFile(nullptr), bgFile(nullptr),
      scripts(nullptr), field(nullptr), modelsVisible(true) /*, thread(0)*/
{
	setMinimumSize(320, 240);
}

void WalkmeshWidget::clear()
{
	walkmesh = nullptr;
	camera = nullptr;
	infFile = nullptr;
	bgFile = nullptr;
	scripts = nullptr;
	field = nullptr;
	//	if (thread)	thread->deleteLater();
	fieldModels.clear();
	update();

	gpuRenderer->reset();
}

WalkmeshWidget::~WalkmeshWidget()
{
	delete gpuRenderer;
}

void WalkmeshWidget::fill(Field *field)
{
	this->walkmesh = field->walkmesh();
	this->camera = field->camera();
	this->infFile = field->inf();
	this->bgFile = field->background();
	this->scripts = field->scriptsAndTexts();
	this->field = field;
	this->fieldModels.clear();
	if (modelsVisible) {
		openModels();
	}

	updatePerspective();
	resetCamera();
}

void WalkmeshWidget::openModels()
{
	//	if (thread)	thread->deleteLater();
	if (!this->fieldModels.isEmpty() || !field || !scripts) {
		return;
	}
	int modelCount = scripts->modelCount();
	QList<int> modelIds;
	for (int modelId = 0; modelId < modelCount; ++modelId) {
		modelIds.append(modelId);
	}
	//	if (field->isPC()) {
	//		thread = new FieldModelThread(this);
	//		thread->setField(field);
	//		connect(thread,
	// SIGNAL(modelLoaded(Field*,FieldModelFile*,int,int,bool)),
	// SLOT(addModel(Field*,FieldModelFile*,int)));
	// thread->setModels(modelIds); 		thread->start(); 	} else {
	for (int modelId : modelIds) {
		addModel(field, field->fieldModel(modelId), modelId);
	}
	//	}
}

void WalkmeshWidget::addModel(Field *field, FieldModelFile *fieldModelFile, int modelId)
{
	if (this->field == field) {
		fieldModels.insert(modelId, fieldModelFile);
		update();
	}
}

void WalkmeshWidget::computeFov()
{
	if (camera && camera->isOpen() && camera->hasCamera() && _camID < camera->cameraCount()) {
		const Camera &cam = camera->camera(_camID);
		fovy = (2 * atan(240.0 / (2.0 * cam.camera_zoom))) * 57.29577951;
	} else {
		fovy = 70.0;
	}
}

void WalkmeshWidget::updatePerspective()
{
	computeFov();
	resize(width(), height());
	update();
}

void WalkmeshWidget::initializeGL()
{
	gpuRenderer = new Renderer(this);
}

void WalkmeshWidget::resizeGL(int width, int height)
{
	gpuRenderer->setViewport(0, 0, width, height);
}

void WalkmeshWidget::paintGL()
{
	if (!walkmesh)
		return;

	drawBackground();

	mProjection.setToIdentity();
	mProjection.perspective(fovy, (float)width() / (float)height(), 0.001f, 1000.0f);
	gpuRenderer->bindProjectionMatrix(mProjection);

	QMatrix4x4 mModel;
	mModel.translate(xTrans, yTrans, distance);
	mModel.rotate(xRot, 1.0f, 0.0f, 0.0f);
	mModel.rotate(yRot, 0.0f, 1.0f, 0.0f);
	mModel.rotate(zRot, 0.0f, 0.0f, 1.0f);

	QMatrix4x4 mView;

	if (camera->isOpen() && camera->hasCamera() && _camID < camera->cameraCount())
	{
		const Camera &cam = camera->camera(_camID);

		double camAxisXx = cam.camera_axis[0].x / 4096.0f;
		double camAxisXy = cam.camera_axis[0].y / 4096.0f;
		double camAxisXz = cam.camera_axis[0].z / 4096.0f;

		double camAxisYx = -cam.camera_axis[1].x / 4096.0f;
		double camAxisYy = -cam.camera_axis[1].y / 4096.0f;
		double camAxisYz = -cam.camera_axis[1].z / 4096.0f;

		double camAxisZx = cam.camera_axis[2].x / 4096.0f;
		double camAxisZy = cam.camera_axis[2].y / 4096.0f;
		double camAxisZz = cam.camera_axis[2].z / 4096.0f;

		double camPosX = cam.camera_position[0] / 4096.0f;
		double camPosY = -cam.camera_position[1] / 4096.0f;
		double camPosZ = cam.camera_position[2] / 4096.0f;

		double tx = -(camPosX * camAxisXx + camPosY * camAxisYx + camPosZ * camAxisZx);
		double ty = -(camPosX * camAxisXy + camPosY * camAxisYy + camPosZ * camAxisZy);
		double tz = -(camPosX * camAxisXz + camPosY * camAxisYz + camPosZ * camAxisZz);

		const QVector3D eye(tx, ty, tz), center(tx + camAxisZx, ty + camAxisZy, tz + camAxisZz), up(camAxisYx, camAxisYy, camAxisYz);
		mView.lookAt(eye, center, up);
	}

	gpuRenderer->bindModelMatrix(mModel);
	gpuRenderer->bindViewMatrix(mView);

	if (walkmesh->isOpen()) {
		int i = 0;

		for (const Triangle &triangle : walkmesh->triangles()) {
			const Access &access = walkmesh->access(i);

			drawLine(triangle.vertices[0], triangle.vertices[1], (i == _selectedTriangle ? 0xFFFF9000 : (access.a[0] == -1 ? 0xFF6699CC : 0xFFFFFFFF)));
			drawLine(triangle.vertices[1], triangle.vertices[2], (i == _selectedTriangle ? 0xFFFF9000 : (access.a[1] == -1 ? 0xFF6699CC : 0xFFFFFFFF)));
			drawLine(triangle.vertices[2], triangle.vertices[0], (i == _selectedTriangle ? 0xFFFF9000 : (access.a[2] == -1 ? 0xFF6699CC : 0xFFFFFFFF)));

			++i;
		}

		if (infFile && infFile->isOpen()) {
			int gateID = 0;
			for (const Exit &gate : infFile->exitLines()) {
				if (gate.fieldID != 0x7FFF) {
					Vertex_sr vertex[2]{
						{ gate.exit_line[0].x, gate.exit_line[0].y, gate.exit_line[0].z, 1.0f },
						{ gate.exit_line[1].x, gate.exit_line[1].y, gate.exit_line[1].z, 1.0f },
					};

					drawLine(vertex[0], vertex[1], (0xFFFF0000));
				}
				++gateID;
			}

			for (const Trigger &trigger : infFile->triggers()) {
				if (trigger.background_parameter != 0xFF) {
					Vertex_sr vertex[2]{
						{ trigger.trigger_line[0].x, trigger.trigger_line[0].y, trigger.trigger_line[0].z, 1.0f },
						{ trigger.trigger_line[1].x, trigger.trigger_line[1].y, trigger.trigger_line[1].z, 1.0f },
					};

					drawLine(vertex[0], vertex[1], (0xFF00FF00));
				}
			}
		}

		gpuRenderer->draw(RendererPrimitiveType::PT_LINES);

		if (_selectedTriangle >= 0 && _selectedTriangle < walkmesh->triangleCount()) {
			const Triangle &triangle = walkmesh->triangle(_selectedTriangle);

			drawTriangle(triangle.vertices[0], triangle.vertices[1], triangle.vertices[2], (0xFFFF9000));
		}

		if (infFile && infFile->isOpen()) {
			if (_selectedGate >= 0 && _selectedGate < 12) {
				const Exit &gate = infFile->exitLine(_selectedGate);
				if (gate.fieldID != 0x7FFF) {
					Vertex_sr vertex[2]{
						{ gate.exit_line[0].x, gate.exit_line[0].y, gate.exit_line[0].z, 1.0f },
						{ gate.exit_line[1].x, gate.exit_line[1].y, gate.exit_line[1].z, 1.0f },
					};

					drawLine(vertex[0], vertex[1], (0xFFFF0000));
				}
			}

			if (_selectedDoor >= 0 && _selectedDoor < 12) {
				const Trigger &trigger = infFile->trigger(_selectedDoor);
				if (trigger.background_parameter != 0xFF) {
					Vertex_sr vertex[2]{
						{ trigger.trigger_line[0].x, trigger.trigger_line[0].y, trigger.trigger_line[0].z, 1.0f },
						{ trigger.trigger_line[1].x, trigger.trigger_line[1].y, trigger.trigger_line[1].z, 1.0f },
					};

					drawLine(vertex[0], vertex[1], (0xFF00FF00));
				}
			}
		}

		gpuRenderer->draw(RendererPrimitiveType::PT_POINTS, 7.0f);
	}

	if (scripts && scripts->isOpen()) {
		QMap<int, FF7Position *> positions;
		scripts->linePosition(positions);

		if (!positions.isEmpty()) {
			QMapIterator<int, FF7Position *> i(positions);
			while (i.hasNext()) {
				i.next();
				FF7Position *pos = i.value();

				Vertex_sr vertex[2]{
					{ pos[0].x, pos[0].y, pos[0].z, 1.0f },
					{ pos[1].x, pos[1].y, pos[1].z, 1.0f },
				};

				drawLine(vertex[0], vertex[1], (0xFF90FF00));

				delete[] pos;
			}

			gpuRenderer->draw(RendererPrimitiveType::PT_LINES);
		}

		if (modelsVisible && !fieldModels.isEmpty()) {
			QMultiMap<int, FF7Position> modelPositions;
			scripts->listModelPositions(modelPositions);

			QMap<int, int> modelDirection;
			int modelID = 0;
			for (GrpScript *group : scripts->grpScripts()) {
				if (group->typeID() == GrpScript::Model) {
					if (!group->scripts().isEmpty()) {
						for (Opcode *op : group->script(0)->opcodes()) {
							if (op->id() == Opcode::DIR) {
								OpcodeDIR *opDir = (OpcodeDIR *)op;
								if (opDir->banks == 0) {
									modelDirection.insert(modelID, opDir->direction);
									break;
								}
							}
						}
					}
					++modelID;
				}
			}

			if (!modelPositions.isEmpty()) {
				int previousModelId = -1;
				QMapIterator<int, FF7Position> i(modelPositions);
				while (i.hasNext()) {
					i.next();
					const int modelId = i.key();
					if (previousModelId == modelId) {
						continue;
					}
					previousModelId = modelId;
					FF7Position position = i.value();

					if (!position.hasZ && position.hasId && position.id < walkmesh->triangleCount()) {
						position.z = walkmesh->triangle(position.id).vertices[0].z;
					} else if (!position.hasZ) {
						continue;
					}

					FieldModelFile *fieldModel = fieldModels.value(modelId);
					if (fieldModel)
					{
					  QMatrix4x4 mModel;
					  mModel.translate(position.x / 4096.0f, position.y / 4096.0f, position.z / 4096.0f);
					  mModel.rotate(270.0f, 1.0, 0.0, 0.0);

					  int direction = modelDirection.value(modelId, -1);
					  if (direction != -1) {
						  mModel.rotate(-360.0f * direction / 256.0f, 0.0, 1.0, 0.0);
					  }

					  FieldModel::paintModel(gpuRenderer, fieldModel, 0, 0, 8.0f, mModel);
					}
				}
			}
		}
	}

	gpuRenderer->show();
}

void WalkmeshWidget::drawLine(const Vertex_sr &pointA, const Vertex_sr &pointB, uint32_t argbColor)
{
	QRgba64 color = QRgba64::fromArgb32(argbColor);

	struct RendererVertex _vertex[]{
		{
			{pointA.x / 4096.0f, pointA.y / 4096.0f, pointA.z / 4096.0f, 1.0f},
			{color.red8() / float(UINT8_MAX), color.green8() / float(UINT8_MAX), color.blue8() / float(UINT8_MAX), color.alpha8() / float(UINT8_MAX)},
			{0, 0},
		},
		{
			{pointB.x / 4096.0f, pointB.y / 4096.0f, pointB.z / 4096.0f, 1.0f},
			{color.red8() / float(UINT8_MAX), color.green8() / float(UINT8_MAX), color.blue8() / float(UINT8_MAX), color.alpha8() / float(UINT8_MAX)},
			{0, 0},
		},
	};

	gpuRenderer->bindVertex(_vertex, 2);
}

void WalkmeshWidget::drawBackground()
{
  if (bgFile)
  {
    RendererVertex vertices[] = {
      {
        {-1.0f, -1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f},
      },
      {
        {-1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f},
      },
      {
        {1.0f, -1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f},
      },
      {
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 0.0f},
      }
    };

    uint32_t indices[] = {
      0, 1, 2,
      1, 3, 2
    };

    QMatrix4x4 mBG;

    gpuRenderer->bindProjectionMatrix(mBG);
    gpuRenderer->bindViewMatrix(mBG);
    gpuRenderer->bindModelMatrix(mBG);

    gpuRenderer->bindVertex(vertices, 4);
    gpuRenderer->bindIndex(indices, 6);
    gpuRenderer->bindTexture(bgFile->openBackground());
    gpuRenderer->draw(RendererPrimitiveType::PT_TRIANGLES);
  }
}

void WalkmeshWidget::drawTriangle(const Vertex_sr &pointA, const Vertex_sr &pointB, const Vertex_sr &pointC, uint32_t argbColor)
{
	QRgba64 color = QRgba64::fromArgb32(argbColor);

	struct RendererVertex _vertex[]{
		{
			{pointA.x / 4096.0f, pointA.y / 4096.0f, pointA.z / 4096.0f, 1.0f},
			{color.red8() / float(UINT8_MAX), color.green8() / float(UINT8_MAX), color.blue8() / float(UINT8_MAX), color.alpha8() / float(UINT8_MAX)},
			{0, 0},
		},
		{
			{pointB.x / 4096.0f, pointB.y / 4096.0f, pointB.z / 4096.0f, 1.0f},
			{color.red8() / float(UINT8_MAX), color.green8() / float(UINT8_MAX), color.blue8() / float(UINT8_MAX), color.alpha8() / float(UINT8_MAX)},
			{0, 0},
		},
		{
			{pointC.x / 4096.0f, pointC.y / 4096.0f, pointC.z / 4096.0f, 1.0f},
			{color.red8() / float(UINT8_MAX), color.green8() / float(UINT8_MAX), color.blue8() / float(UINT8_MAX), color.alpha8() / float(UINT8_MAX)},
			{0, 0},
		},
	};

	gpuRenderer->bindVertex(_vertex, 3);
}

void WalkmeshWidget::wheelEvent(QWheelEvent *event)
{
	setFocus();
	distance += event->delta() / 4096.0f;
	update();
}

void WalkmeshWidget::mousePressEvent(QMouseEvent *event)
{
	setFocus();
	if (event->button() == Qt::MidButton) {
		resetCamera();
	} else if (event->button() == Qt::LeftButton) {
		moveStart = event->pos();
	}
}

void WalkmeshWidget::mouseMoveEvent(QMouseEvent *event)
{
	xTrans += (event->pos().x() - moveStart.x()) / 4096.0f;
	yTrans -= (event->pos().y() - moveStart.y()) / 4096.0f;
	moveStart = event->pos();
	update();
}

void WalkmeshWidget::keyPressEvent(QKeyEvent *event)
{
	if (lastKeyPressed == event->key()
	    && (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right
	        || event->key() == Qt::Key_Down || event->key() == Qt::Key_Up)) {
		if (transStep > 100.0f) {
			transStep *= 0.90f; // accelerator
		}
	} else {
		transStep = 360.0f;
	}
	lastKeyPressed = event->key();

	switch (event->key()) {
	case Qt::Key_Left:
		xTrans += 1.0f / transStep;
		update();
		break;
	case Qt::Key_Right:
		xTrans -= 1.0f / transStep;
		update();
		break;
	case Qt::Key_Down:
		yTrans += 1.0f / transStep;
		update();
		break;
	case Qt::Key_Up:
		yTrans -= 1.0f / transStep;
		update();
		break;
	default:
		QWidget::keyPressEvent(event);
		return;
	}
}

void WalkmeshWidget::focusInEvent(QFocusEvent *event)
{
	grabKeyboard();
	QWidget::focusInEvent(event);
}

void WalkmeshWidget::focusOutEvent(QFocusEvent *event)
{
	releaseKeyboard();
	QWidget::focusOutEvent(event);
}

static void qNormalizeAngle(int &angle)
{
	while (angle < 0)
		angle += 360;
	while (angle > 360)
		angle -= 360;
}

void WalkmeshWidget::setXRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != xRot) {
		xRot = angle;
		update();
	}
}

void WalkmeshWidget::setYRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != yRot) {
		yRot = angle;
		update();
	}
}

void WalkmeshWidget::setZRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != zRot) {
		zRot = angle;
		update();
	}
}

void WalkmeshWidget::setZoom(int zoom)
{
	distance = zoom / 4096.0f;
}

void WalkmeshWidget::resetCamera()
{
	distance = 0;
	zRot = yRot = xRot = 0;
	xTrans = yTrans = 0;
	update();
}

void WalkmeshWidget::setModelsVisible(bool show)
{
	if (modelsVisible != show) {
		modelsVisible = show;
		if (modelsVisible) {
			openModels();
		}
		update();
	}
}

void WalkmeshWidget::setCurrentFieldCamera(int camID)
{
	if (_camID != camID) {
		_camID = camID;
		updatePerspective();
	}
}

void WalkmeshWidget::setSelectedTriangle(int triangle)
{
	if (_selectedTriangle != triangle) {
		_selectedTriangle = triangle;
		update();
	}
}

void WalkmeshWidget::setSelectedDoor(int door)
{
	if (_selectedDoor != door) {
		_selectedDoor = door;
		update();
	}
}

void WalkmeshWidget::setSelectedGate(int gate)
{
	if (_selectedGate != gate) {
		_selectedGate = gate;
		update();
	}
}

void WalkmeshWidget::setSelectedArrow(int arrow)
{
	if (_selectedArrow != arrow) {
		_selectedArrow = arrow;
		update();
	}
}
