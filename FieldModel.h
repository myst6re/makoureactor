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
#include <GL/glu.h>
#include "FieldModelFile.h"
#include "Data.h"
#include "Palette.h"
#include "Field.h"

class FieldModel : public QGLWidget
{
	Q_OBJECT
public:
	FieldModel(QWidget *parent=0, const QGLWidget *shareWidget=0);
	quint8 load(const QString &hrc, const QString &a, bool animate=false);
	quint8 load(const QByteArray &BSX_data, int model_id);
	void clear();
	int nb_bones();
public slots:
	void animate();
private:
	void drawP(int);
	void setXRotation(int angle);
	void setYRotation(int angle);
	void setZRotation(int angle);

	bool blockAll;
	int distance;
	int currentFrame;

	FieldModelFile data;
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
