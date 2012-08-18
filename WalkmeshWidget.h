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
#ifndef WALKMESHWIDGET_H
#define WALKMESHWIDGET_H

#include <QtGui>
#include <QGLWidget>
#include <GL/glu.h>
#include "IdFile.h"
#include "CaFile.h"
#include "InfFile.h"

class WalkmeshWidget : public QGLWidget
{
	Q_OBJECT
public:
	explicit WalkmeshWidget(QWidget *parent=0, const QGLWidget *shareWidget=0);
	void clear();
	void fill(IdFile *walkmesh, CaFile *camera, InfFile *infFile);
	void updatePerspective();
public slots:
	void setXRotation(int);
	void setYRotation(int);
	void setZRotation(int);
	void setZoom(int);
	void setCurrentFieldCamera(int camID);
	void setSelectedTriangle(int triangle);
	void setSelectedDoor(int door);
	void setSelectedGate(int gate);
	void setSelectedArrow(int arrow);
private:
	void computeFov();
	void drawIdLine(int triangleID, const Vertex_sr &vertex1, const Vertex_sr &vertex2, qint16 access);
	double distance;
	float xRot, yRot, zRot;
	int camID;
	int _selectedTriangle;
	int _selectedDoor;
	int _selectedGate;
	int _selectedArrow;
	double fovy;
	IdFile *walkmesh;
	CaFile *camera;
	InfFile *infFile;
protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
	void wheelEvent(QWheelEvent *event);
	void mousePressEvent(QMouseEvent *event);
};

#endif // WALKMESHWIDGET_H
