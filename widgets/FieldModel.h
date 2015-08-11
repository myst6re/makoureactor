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
#ifndef FIELDMODEL_H
#define FIELDMODEL_H

#include <QtGui>
#include <QGLWidget>
#ifdef Q_OS_MAC
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include "core/field/FieldModelFile.h"
#include "core/field/FieldPC.h"

class FieldModel : public QGLWidget
{
	Q_OBJECT
public:
	FieldModel(QWidget *parent=0, const QGLWidget *shareWidget=0);
	virtual ~FieldModel();
	void setIsAnimated(bool animate);
	void setAnimationID(int animID);
	void clear();
	int boneCount() const;
	int frameCount() const;
	static void paintModel(QGLWidget *glWidget, FieldModelFile *data, int animationID, int currentFrame=0, float scale=1.0f);
public slots:
	void setFieldModelFile(FieldModelFile *fieldModel, int animationID = 0);
private slots:
	void animate();
private:
	void updateTimer();
	inline void paintModel() { paintModel(this, data, animationID, currentFrame); }
	static void drawP(QGLWidget *glWidget, FieldModelFile *data, float scale, const FieldModelBone &bone,
					  GLuint &texture_id, quint64 &lastTexID);
	void setXRotation(int angle);
	void setYRotation(int angle);
	void setZRotation(int angle);
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
protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
	void wheelEvent(QWheelEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
};

#endif // FIELDMODEL_H
