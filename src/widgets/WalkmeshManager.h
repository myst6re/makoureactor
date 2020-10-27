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
#ifndef WALKMESHMANAGER_H
#define WALKMESHMANAGER_H

#include <QtWidgets>
#include "core/field/Field.h"
#include "WalkmeshWidget.h"
#include "VertexWidget.h"
#include "HexLineEdit.h"
#include "OrientationWidget.h"
#include "Listwidget.h"

class WalkmeshManager : public QDialog
{
	Q_OBJECT
public:
	explicit WalkmeshManager(QWidget *parent=nullptr);
	void saveConfig();
	void fill(Field *field, bool reload=false);
	void clear();
	int currentCamera() const;
signals:
	void modified();
public slots:
	void resetCamera();
	void setModelsVisible(bool show);
	void setCurrentCamera(int camID);
private slots:
	void addCamera();
	void removeCamera();
	void editCaVector(const Vertex_s &values);
	void editCaPos(double value);
	void editCaZoom(int value);
	void setCurrentId(int i);
	void addTriangle();
	void removeTriangle();
	void editIdTriangle(const Vertex_s &values);
	void editIdAccess(int value);
	void setCurrentGateway(int id);
	void setCurrentDoor(int id);
	void setCurrentArrow(int id);
	void editExitPoint(const Vertex_s &values);
	void editEntryPoint(const Vertex_s &values);
	void editDoorPoint(const Vertex_s &values);
	void setGateEnabled(bool enabled);
	void editGateEnabled(bool enabled);
	void editFieldId(int v);
	void editArrowDisplay(bool checked);
	void setDoorEnabled(bool enabled);
	void editDoorEnabled(bool enabled);
	void editParamId(int v);
	void editStateId(int v);
	void editBehavior(int v);
	void editSoundId(int v);
	void editArrowX(int value);
	void editArrowY(int value);
	void editArrowZ(int value);
	void editArrowType(int index);
	void editRange(int v);
	void editExitDirection(int dir);
	void editNavigation(int v);
	void editCameraFocusHeight(int value);
	void editUnknown(const QByteArray &data);
	void editMapScale(int scale);
protected:
	virtual void focusInEvent(QFocusEvent *e);
	virtual void focusOutEvent(QFocusEvent *e);
private:
	QWidget *buildCameraPage();
	QWidget *buildWalkmeshPage();
	QWidget *buildGatewaysPage();
	QWidget *buildDoorsPage();
	QWidget *buildArrowPage();
	QWidget *buildCameraRangePage();
	QWidget *buildMiscPage();
	void editCaVector(int id, const Vertex_s &values);
	void editCaPos(int id, double value);
	void editIdTriangle(int id, const Vertex_s &values);
	void editIdAccess(int id, int value);
	void editExitPoint(int id, const Vertex_s &values);
	void editDoorPoint(int id, const Vertex_s &values);
	void editRange(int id, int v);
	void editBgSize(int id, int v);
	void editBgFlag(int id, int v);

	IdFile *idFile;
	CaFile *caFile;
	InfFile *infFile;
	WalkmeshWidget *walkmesh;
	QCheckBox *showModels;
	Section1File *scriptsAndTexts;
	QSlider *slider1, *slider2, *slider3;
	QTabWidget *tabWidget;
	//CamPage
	QToolBar *caToolbar;
	QListWidget *camList;
	VertexWidget *caVectorXEdit, *caVectorYEdit, *caVectorZEdit;
	QDoubleSpinBox *caSpaceXEdit, *caSpaceYEdit, *caSpaceZEdit;
	QSpinBox *caZoomEdit;
	//WalkmeshPage
	QToolBar *idToolbar;
	QListWidget *idList;
	VertexWidget *idVertices[3];
	QSpinBox *idAccess[3];
	//GatePage
	QListWidget *gateList;
	QCheckBox *gateEnabled;
	QSpinBox *exitDirection;
	QSpinBox *fieldId;
	VertexWidget *exitPoints[2], *entryPoint;
	QCheckBox *arrowDisplay;
	//ArrowPage
	QListWidget *arrowList;
	QSpinBox *arrowX, *arrowY, *arrowZ;
	QComboBox *arrowType;
	//DoorPage
	QListWidget *doorList;
	QCheckBox *doorEnabled;
	QSpinBox *bgParamId, *bgStateId, *doorBehavior, *doorSoundId;
	VertexWidget *doorPosition[2];
	//CameraRangePage
	QSpinBox *rangeEdit[4], *bgSizeEdit[4], *bgFlagEdit[4];
	//MiscPage
	OrientationWidget *navigation;
	QSpinBox *navigation2, *cameraFocusHeight;
	HexLineEdit *unknown;
	QSpinBox *mapScale;
};

#endif // WALKMESHMANAGER_H
