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
#pragma once

#include <QtWidgets>
#include "Renderer.h"
#include "core/field/Field.h"

class WalkmeshWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	explicit WalkmeshWidget(QWidget *parent = nullptr);
	~WalkmeshWidget() override;
	void clear();
	void fill(Field *field);
	void updatePerspective();
	inline bool hasError() const {
		return gpuRenderer && gpuRenderer->hasError();
	}
public slots:
	void setXRotation(int);
	void setYRotation(int);
	void setZRotation(int);
	void setZoom(int);
	void resetCamera();
	void setModelsVisible(bool show);
	void setBackgroundVisible(bool show);
	void setCurrentFieldCamera(int camID);
	void setSelectedTriangle(int triangle);
	void setSelectedDoor(int door);
	void setSelectedGate(int gate);
	void setSelectedArrow(int arrow);
	void setCustomLine(const Vertex_s &customLinePoint1, const Vertex_s &customLinePoint2);
	void setCustomLineVisible(bool show);
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
	Vertex_s _customLinePoint1, _customLinePoint2;
	bool _hasCustomLine;
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
	bool modelsVisible, backgroundVisible;
	Renderer *gpuRenderer;
	QMatrix4x4 mProjection;
	QImage tex;

protected:
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;
	virtual void wheelEvent(QWheelEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void focusInEvent(QFocusEvent *event) override;
	virtual void focusOutEvent(QFocusEvent *event) override;
};
