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
#pragma once

#include <QtWidgets>
#include "core/Renderer.h"
#include "core/field/FieldModelFile.h"
#include "core/field/FieldPC.h"

class FieldModel : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT
public:
	FieldModel(QWidget *parent = nullptr);
	virtual ~FieldModel();
	void setIsAnimated(bool animate);
	void setAnimationID(int animID);
	void clear();
	int boneCount() const;
	int frameCount() const;
	static void paintModel(Renderer *gpuRenderer, FieldModelFile *data, int animationID, int currentFrame=0, float scale=1.0f, QMatrix4x4 initialModelMatrix = QMatrix4x4());
public slots:
	void setFieldModelFile(FieldModelFile *fieldModel, int animationID = 0);
private slots:
	void animate();
private:
	void updateTimer();
	inline void paintModel() { paintModel(gpuRenderer, data, animationID, currentFrame); }
	static void drawP(Renderer *gpuRenderer, FieldModelFile *data, float scale, const FieldModelBone &bone, float globalColor[3]);
	bool setXRotation(int angle);
	bool setYRotation(int angle);
	bool setZRotation(int angle);
	void resetCamera();

	bool blockAll;
	double distance;
	int animationID;
	int currentFrame;
	bool animated;

	FieldModelFile *data;
	QTimer timer;

	int xRot;
	int yRot;
	int zRot;
	QPoint lastPos;
	Renderer *gpuRenderer;
	QMatrix4x4 mProjection;

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
	void wheelEvent(QWheelEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
};
