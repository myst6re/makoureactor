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

#include <QtWidgets>
#include "core/Renderer.h"
#include "core/field/Field.h"
#include "widgets/FieldModel.h"

class WalkmeshWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	explicit WalkmeshWidget(QWidget *parent = nullptr);
	~WalkmeshWidget();
	void clear();
	void fill(Field *field);
	void updatePerspective();
public slots:
	void setXRotation(int);
	void setYRotation(int);
	void setZRotation(int);
	void setZoom(int);
	void resetCamera();
	void setModelsVisible(bool show);
	void setCurrentFieldCamera(int camID);
	void setSelectedTriangle(int triangle);
	void setSelectedDoor(int door);
	void setSelectedGate(int gate);
	void setSelectedArrow(int arrow);
private slots:
	void addModel(Field *field, FieldModelFile *fieldModelFile, int modelId);
private:
	void computeFov();
	void drawBackground();
	void openModels();
	double distance;
	float xRot, yRot, zRot;
	float xTrans, yTrans, transStep;
	int lastKeyPressed;
	int _camID;
	int _selectedTriangle;
	int _selectedDoor;
	int _selectedGate;
	int _selectedArrow;
	float fovy;
	IdFile *walkmesh;
	CaFile *camera;
	InfFile *infFile;
	BackgroundFile *bgFile;
	Section1File *scripts;
	Field *field;
	QMap<int, FieldModelFile *> fieldModels;
	//	FieldModelThread *thread;
	QPoint moveStart;
//	QPixmap arrow;
	bool modelsVisible;
	Renderer *gpuRenderer;
	QMatrix4x4 mProjection;

protected:
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();
	virtual void wheelEvent(QWheelEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void focusInEvent(QFocusEvent *event);
	virtual void focusOutEvent(QFocusEvent *event);
};

#endif // WALKMESHWIDGET_H
