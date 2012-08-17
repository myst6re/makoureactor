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
#include "WalkmeshManager.h"

WalkmeshManager::WalkmeshManager(WalkmeshFile *walkmeshFile, InfFile *infFile, Field *field, QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint), walkmeshFile(walkmeshFile), infFile(infFile), field(field)
{
	setWindowTitle(tr("Zones"));

	walkmesh = Config::value("OpenGL", true).toBool() ? new WalkmeshWidget(walkmeshFile) : 0;
	QWidget *walkmeshWidget = walkmesh ? walkmesh : new QWidget(this);

	QSlider *slider1 = new QSlider(this);
	QSlider *slider2 = new QSlider(this);
	QSlider *slider3 = new QSlider(this);
//	QSlider *slider4 = new QSlider(this);

	slider1->setRange(0, 360);
	slider2->setRange(0, 360);
	slider3->setRange(0, 360);
//	slider4->setRange(-4096, 4096);

	slider1->setValue(0);
	slider2->setValue(0);
	slider3->setValue(0);
//	slider4->setValue(0);

	tabWidget = new QTabWidget(this);
	tabWidget->addTab(buildCameraPage(), tr("Caméra"));
	tabWidget->addTab(buildWalkmeshPage(), tr("Walkmesh"));
	tabWidget->addTab(buildGatewaysPage(), tr("Sorties"));
	tabWidget->addTab(buildDoorsPage(), tr("Portes"));
	tabWidget->addTab(buildCameraRangePage(), tr("Limites caméra"));
	tabWidget->addTab(buildMiscPage(), tr("Divers"));
	tabWidget->setFixedHeight(250);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(walkmeshWidget, 0, 0);
	layout->addWidget(tabWidget, 1, 0, 1, 4);
	layout->addWidget(slider1, 0, 1);
	layout->addWidget(slider2, 0, 2);
	layout->addWidget(slider3, 0, 3);
//	layout->addWidget(slider4, 0, 4);

	if(walkmesh) {
		connect(slider1, SIGNAL(valueChanged(int)), walkmesh, SLOT(setXRotation(int)));
		connect(slider2, SIGNAL(valueChanged(int)), walkmesh, SLOT(setYRotation(int)));
		connect(slider3, SIGNAL(valueChanged(int)), walkmesh, SLOT(setZRotation(int)));
//		connect(slider4, SIGNAL(valueChanged(int)), walkmesh, SLOT(setZoom(int)));
	}

	fill();
}

QWidget *WalkmeshManager::buildCameraPage()
{
	QWidget *ret = new QWidget(this);

	ListWidget *listWidget = new ListWidget(ret);
	listWidget->addAction(ListWidget::Add, tr("Ajouter caméra"), this, SLOT(addCamera()));
	listWidget->addAction(ListWidget::Rem, tr("Supprimer caméra"), this, SLOT(removeCamera()));

	caToolbar = listWidget->toolBar();
	camList = listWidget->listWidget();

	caVectorXEdit = new VertexWidget(ret);
	caVectorYEdit = new VertexWidget(ret);
	caVectorZEdit = new VertexWidget(ret);

	caSpaceXEdit = new QDoubleSpinBox(ret);
	qreal maxInt = qPow(2,31);
	caSpaceXEdit->setRange(-maxInt, maxInt);
	caSpaceXEdit->setDecimals(0);
	caSpaceYEdit = new QDoubleSpinBox(ret);
	caSpaceYEdit->setRange(-maxInt, maxInt);
	caSpaceYEdit->setDecimals(0);
	caSpaceZEdit = new QDoubleSpinBox(ret);
	caSpaceZEdit->setRange(-maxInt, maxInt);
	caSpaceZEdit->setDecimals(0);

	caZoomEdit = new QSpinBox(ret);
	caZoomEdit->setRange(-32768, 32767);

	QGridLayout *caLayout = new QGridLayout(ret);
	caLayout->addWidget(listWidget, 0, 0, 8, 1);
	caLayout->addWidget(new QLabel(tr("Distance (zoom) :")), 0, 1, 1, 3);
	caLayout->addWidget(caZoomEdit, 0, 4, 1, 2);
	caLayout->addWidget(new QLabel(tr("Axes de la caméra :")), 1, 1, 1, 6);
	caLayout->addWidget(caVectorXEdit, 2, 1, 1, 6);
	caLayout->addWidget(caVectorYEdit, 3, 1, 1, 6);
	caLayout->addWidget(caVectorZEdit, 4, 1, 1, 6);
	caLayout->addWidget(new QLabel(tr("Position de la caméra :")), 5, 1, 1, 6);
	caLayout->addWidget(new QLabel(tr("X")), 6, 1);
	caLayout->addWidget(caSpaceXEdit, 6, 2);
	caLayout->addWidget(new QLabel(tr("Y")), 6, 3);
	caLayout->addWidget(caSpaceYEdit, 6, 4);
	caLayout->addWidget(new QLabel(tr("Z")), 6, 5);
	caLayout->addWidget(caSpaceZEdit, 6, 6);
	caLayout->setRowStretch(7, 1);
	caLayout->setColumnStretch(2, 1);
	caLayout->setColumnStretch(4, 1);
	caLayout->setColumnStretch(6, 1);

	connect(camList, SIGNAL(currentRowChanged(int)), SLOT(setCurrentCamera(int)));

	connect(caVectorXEdit, SIGNAL(valuesChanged(Vertex_s)), SLOT(editCaVector(Vertex_s)));
	connect(caVectorYEdit, SIGNAL(valuesChanged(Vertex_s)), SLOT(editCaVector(Vertex_s)));
	connect(caVectorZEdit, SIGNAL(valuesChanged(Vertex_s)), SLOT(editCaVector(Vertex_s)));

	connect(caSpaceXEdit, SIGNAL(valueChanged(double)), SLOT(editCaPos(double)));
	connect(caSpaceYEdit, SIGNAL(valueChanged(double)), SLOT(editCaPos(double)));
	connect(caSpaceZEdit, SIGNAL(valueChanged(double)), SLOT(editCaPos(double)));

	connect(caZoomEdit, SIGNAL(valueChanged(int)), SLOT(editCaZoom(int)));

	return ret;
}

QWidget *WalkmeshManager::buildWalkmeshPage()
{
	QWidget *ret = new QWidget(this);

	ListWidget *listWidget = new ListWidget(ret);
	listWidget->addAction(ListWidget::Add, tr("Ajouter triangle"), this, SLOT(addTriangle()));
	listWidget->addAction(ListWidget::Rem, tr("Supprimer triangle"), this, SLOT(removeTriangle()));

	idToolbar = listWidget->toolBar();
	idList = listWidget->listWidget();

	idVertices[0] = new VertexWidget(ret);
	idVertices[1] = new VertexWidget(ret);
	idVertices[2] = new VertexWidget(ret);

	idAccess[0] = new QSpinBox(ret);
	idAccess[1] = new QSpinBox(ret);
	idAccess[2] = new QSpinBox(ret);

	idAccess[0]->setRange(-32768, 32767);
	idAccess[1]->setRange(-32768, 32767);
	idAccess[2]->setRange(-32768, 32767);

	QHBoxLayout *accessLayout0 = new QHBoxLayout;
	accessLayout0->addWidget(new QLabel(tr("Triangle accessible via la ligne 1-2 :")));
	accessLayout0->addWidget(idAccess[0]);

	QHBoxLayout *accessLayout1 = new QHBoxLayout;
	accessLayout1->addWidget(new QLabel(tr("Triangle accessible via la ligne 2-3 :")));
	accessLayout1->addWidget(idAccess[1]);

	QHBoxLayout *accessLayout2 = new QHBoxLayout;
	accessLayout2->addWidget(new QLabel(tr("Triangle accessible via la ligne 3-1 :")));
	accessLayout2->addWidget(idAccess[2]);

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(listWidget, 0, 0, 7, 1, Qt::AlignLeft);
	layout->addWidget(new QLabel(tr("Point 1 :")), 0, 1);
	layout->addWidget(idVertices[0], 0, 2);
	layout->addWidget(new QLabel(tr("Point 2 :")), 1, 1);
	layout->addWidget(idVertices[1], 1, 2);
	layout->addWidget(new QLabel(tr("Point 3 :")), 2, 1);
	layout->addWidget(idVertices[2], 2, 2);
	layout->addLayout(accessLayout0, 3, 1, 1, 2);
	layout->addLayout(accessLayout1, 4, 1, 1, 2);
	layout->addLayout(accessLayout2, 5, 1, 1, 2);
	layout->setRowStretch(6, 1);

	connect(idList, SIGNAL(currentRowChanged(int)), SLOT(setCurrentId(int)));
	connect(idVertices[0], SIGNAL(valuesChanged(Vertex_s)), SLOT(editIdTriangle(Vertex_s)));
	connect(idVertices[1], SIGNAL(valuesChanged(Vertex_s)), SLOT(editIdTriangle(Vertex_s)));
	connect(idVertices[1], SIGNAL(valuesChanged(Vertex_s)), SLOT(editIdTriangle(Vertex_s)));
	connect(idAccess[0], SIGNAL(valueChanged(int)), SLOT(editIdAccess(int)));
	connect(idAccess[1], SIGNAL(valueChanged(int)), SLOT(editIdAccess(int)));
	connect(idAccess[2], SIGNAL(valueChanged(int)), SLOT(editIdAccess(int)));

	return ret;
}

QWidget *WalkmeshManager::buildGatewaysPage()
{
	QWidget *ret = new QWidget(this);

	gateList = new QListWidget(ret);
	gateList->setFixedWidth(125);

	exitPoints[0] = new VertexWidget(ret);
	exitPoints[1] = new VertexWidget(ret);
	entryPoint = new VertexWidget(ret);

	fieldId = new QSpinBox(ret);
	fieldId->setRange(0, 65535);

	unknownExit = new HexLineEdit(ret);

	QGridLayout *idsLayout = new QGridLayout;
	idsLayout->addWidget(new QLabel(tr("Id écran :")), 0, 0);
	idsLayout->addWidget(fieldId, 0, 1, 1, 4);
	idsLayout->addWidget(new QLabel(tr("Inconnu :")), 1, 0);
	idsLayout->addWidget(unknownExit, 1, 1, 1, 4);

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(gateList, 0, 0, 5, 1, Qt::AlignLeft);
	layout->addWidget(new QLabel(tr("Ligne de sortie :")), 0, 1);
	layout->addWidget(exitPoints[0], 0, 2);
	layout->addWidget(exitPoints[1], 1, 2);
	layout->addWidget(new QLabel(tr("Point de destination :")), 2, 1);
	layout->addWidget(entryPoint, 2, 2);
	layout->addLayout(idsLayout, 3, 1, 1, 2);
	layout->setRowStretch(4, 1);

	connect(gateList, SIGNAL(currentRowChanged(int)), SLOT(setCurrentGateway(int)));
	connect(exitPoints[0], SIGNAL(valuesChanged(Vertex_s)), SLOT(editExitPoint(Vertex_s)));
	connect(exitPoints[1], SIGNAL(valuesChanged(Vertex_s)), SLOT(editExitPoint(Vertex_s)));
	connect(entryPoint, SIGNAL(valuesChanged(Vertex_s)), SLOT(editEntryPoint(Vertex_s)));
	connect(fieldId, SIGNAL(valueChanged(int)), SLOT(editFieldId(int)));
	connect(unknownExit, SIGNAL(dataEdited(QByteArray)), SLOT(editUnknownExit(QByteArray)));

	return ret;
}

QWidget *WalkmeshManager::buildDoorsPage()
{
	QWidget *ret = new QWidget(this);

	doorList = new QListWidget(ret);
	doorList->setFixedWidth(125);

	doorPosition[0] = new VertexWidget(ret);
	doorPosition[1] = new VertexWidget(ret);

	bgParamId = new QSpinBox(ret);
	bgParamId->setRange(0, 255);

	bgStateId = new QSpinBox(ret);
	bgStateId->setRange(0, 255);

	doorBehavior = new QSpinBox(ret);
	doorBehavior->setRange(0, 255);

	doorSoundId = new QSpinBox(ret);
	doorSoundId->setRange(0, 255);

	QGridLayout *idsLayout = new QGridLayout;
	idsLayout->addWidget(new QLabel(tr("Id parametre décors :")), 0, 0);
	idsLayout->addWidget(bgParamId, 0, 1);
	idsLayout->addWidget(new QLabel(tr("Id état décors :")), 1, 0);
	idsLayout->addWidget(bgStateId, 1, 1);
	idsLayout->addWidget(new QLabel(tr("Comportement :")), 2, 0);
	idsLayout->addWidget(doorBehavior, 2, 1);
	idsLayout->addWidget(new QLabel(tr("Id son :")), 3, 0);
	idsLayout->addWidget(doorSoundId, 3, 1);

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(doorList, 0, 0, 4, 1, Qt::AlignLeft);
	layout->addWidget(new QLabel(tr("Ligne déclench. porte :")), 0, 1);
	layout->addWidget(doorPosition[0], 0, 2);
	layout->addWidget(doorPosition[1], 1, 2);
	layout->addLayout(idsLayout, 2, 1, 1, 2);
	layout->setRowStretch(3, 1);

	connect(doorList, SIGNAL(currentRowChanged(int)), SLOT(setCurrentDoor(int)));
	connect(doorPosition[0], SIGNAL(valuesChanged(Vertex_s)), SLOT(editDoorPoint(Vertex_s)));
	connect(doorPosition[1], SIGNAL(valuesChanged(Vertex_s)), SLOT(editDoorPoint(Vertex_s)));
	connect(bgParamId, SIGNAL(valueChanged(int)), SLOT(editParamId(int)));
	connect(bgStateId, SIGNAL(valueChanged(int)), SLOT(editStateId(int)));
	connect(doorBehavior, SIGNAL(valueChanged(int)), SLOT(editBehavior(int)));
	connect(doorSoundId, SIGNAL(valueChanged(int)), SLOT(editSoundId(int)));

	return ret;
}

QWidget *WalkmeshManager::buildCameraRangePage()
{
	QWidget *ret = new QWidget(this);

	rangeList1 = new QListWidget(ret);
	rangeList1->setFixedWidth(125);

	for(int i=0 ; i<1 ; ++i) {
		rangeList1->addItem(tr("Limite caméra %1").arg(i+1));
	}

	rangeList2 = new QListWidget(ret);
	rangeList2->setFixedWidth(125);

	for(int i=0 ; i<1 ; ++i) {
		rangeList2->addItem(tr("Limite écran %1").arg(i+1));
	}

	for(int i=0 ; i<4 ; ++i) {
		rangeEdit1[i] = new QSpinBox(ret);
		rangeEdit1[i]->setRange(-32768, 32767);
		rangeEdit2[i] = new QSpinBox(ret);
		rangeEdit2[i]->setRange(-32768, 32767);
	}

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(rangeList1, 0, 0, 3, 1, Qt::AlignLeft);
	layout->addWidget(rangeList2, 3, 0, 3, 1, Qt::AlignLeft);
	layout->addWidget(new QLabel(tr("Haut")), 0, 1);
	layout->addWidget(rangeEdit1[0], 0, 2);
	layout->addWidget(new QLabel(tr("Bas")), 0, 3);
	layout->addWidget(rangeEdit1[1], 0, 4);
	layout->addWidget(new QLabel(tr("Droite")), 1, 1);
	layout->addWidget(rangeEdit1[2], 1, 2);
	layout->addWidget(new QLabel(tr("Gauche")), 1, 3);
	layout->addWidget(rangeEdit1[3], 1, 4);
	layout->addWidget(new QLabel(tr("Haut")), 3, 1);
	layout->addWidget(rangeEdit2[0], 3, 2);
	layout->addWidget(new QLabel(tr("Bas")), 3, 3);
	layout->addWidget(rangeEdit2[1], 3, 4);
	layout->addWidget(new QLabel(tr("Droite")), 4, 1);
	layout->addWidget(rangeEdit2[2], 4, 2);
	layout->addWidget(new QLabel(tr("Gauche")), 4, 3);
	layout->addWidget(rangeEdit2[3], 4, 4);
	layout->setRowStretch(2, 1);
	layout->setRowStretch(5, 1);
	layout->setColumnStretch(1, 1);
	layout->setColumnStretch(2, 1);
	layout->setColumnStretch(3, 1);
	layout->setColumnStretch(4, 1);

	connect(rangeList1, SIGNAL(currentRowChanged(int)), SLOT(setCurrentRange1(int)));
	connect(rangeList2, SIGNAL(currentRowChanged(int)), SLOT(setCurrentRange2(int)));
	for(int i=0 ; i<4 ; ++i) {
		connect(rangeEdit1[i], SIGNAL(valueChanged(int)), SLOT(editRange(int)));
		connect(rangeEdit2[i], SIGNAL(valueChanged(int)), SLOT(editRange(int)));
	}

	return ret;
}

QWidget *WalkmeshManager::buildMiscPage()
{
	QWidget *ret = new QWidget(this);

	navigation = new OrientationWidget(ret);
	navigation2 = new QSpinBox(ret);
	navigation2->setRange(0, 255);
	navigation2->setWrapping(true);

//	unknown = new HexLineEdit(ret);
//	cameraFocus = new QSpinBox(ret);
//	cameraFocus->setRange(0, 65535);

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(new QLabel(tr("Orientation des mouvements :")), 0, 0);
	layout->addWidget(navigation, 0, 1);
	layout->addWidget(navigation2, 0, 2);
//	layout->addWidget(new QLabel(tr("Inconnu :")), 1, 0);
//	layout->addWidget(unknown, 1, 1, 1, 2);
//	layout->addWidget(new QLabel(tr("Hauteur focus caméra sur le personnage :")), 2, 0);
//	layout->addWidget(cameraFocus, 2, 1, 1, 2);
	layout->setRowStretch(4, 1);

	connect(navigation, SIGNAL(valueEdited(int)), navigation2, SLOT(setValue(int)));
	connect(navigation2, SIGNAL(valueChanged(int)), SLOT(editNavigation(int)));
//	connect(unknown, SIGNAL(dataEdited(QByteArray)), SLOT(editUnknown(QByteArray)));
//	connect(cameraFocus, SIGNAL(valueChanged(int)), SLOT(editCameraFocus(int)));

	return ret;
}

void WalkmeshManager::fill()
{
	int camCount = 0;

	if(walkmeshFile->isOpenCa()) {
		camCount = 1; // TODO

		if(camList->count() != camCount) {
			camList->blockSignals(true);
			camList->clear();
			for(int i=0 ; i<camCount ; ++i) {
				camList->addItem(tr("Caméra %1").arg(i));
			}
			camList->blockSignals(false);
		}

		setCurrentCamera(0);
	}
	tabWidget->widget(0)->setEnabled(walkmeshFile->isOpenCa() && camCount > 0);

	if(walkmeshFile->isOpenId()) {
		int triangleCount = walkmeshFile->triangles.size();

		if(idList->count() != triangleCount) {
			idList->blockSignals(true);
			idList->clear();
			for(int i=0 ; i<triangleCount ; ++i) {
				idList->addItem(tr("Triangle %1").arg(i));
			}
			idList->blockSignals(false);
		}
		idList->setCurrentRow(0);
		setCurrentId(0);
	}
	tabWidget->widget(1)->setEnabled(walkmeshFile->isOpenId());

	if(infFile->isOpen()) {
		gateList->clear();
		foreach(const Exit &gateway, infFile->exitLines()) {
			if(gateway.fieldID != 0x7FFF) {
				gateList->addItem(QString("%1 (%2)").arg(Data::field_names.value(gateway.fieldID)).arg(gateway.fieldID));
			} else {
				gateList->addItem(tr("Inutilisé"));
			}
		}
		gateList->setCurrentRow(0);
		setCurrentGateway(0);

		doorList->clear();
		int doorID = 0;
		foreach(const Trigger &trigger, infFile->triggers()) {
			if(trigger.background_parameter != 0xFF) {
				doorList->addItem(tr("Porte %1").arg(doorID++));
			} else {
				doorList->addItem(tr("Inutilisé"));
			}
		}
		doorList->setCurrentRow(0);
		setCurrentDoor(0);

		rangeList1->setCurrentRow(0);
		setCurrentRange1(0);
		rangeList2->setCurrentRow(0);
		setCurrentRange2(0);

		navigation->setValue(infFile->control());
		navigation2->setValue(infFile->control());
//		unknown->setData(infFile->unknown());
//		cameraFocus->setValue(data()->getInfFile()->cameraFocusHeight());
	}
	tabWidget->widget(2)->setEnabled(infFile->isOpen());
	tabWidget->widget(3)->setEnabled(infFile->isOpen());
	tabWidget->widget(4)->setEnabled(infFile->isOpen());
	tabWidget->widget(5)->setEnabled(infFile->isOpen());
}

int WalkmeshManager::currentCamera() const
{
	if(!walkmeshFile->isOpenCa())	return 0;

	int camID = camList->currentRow();
	return camID < 0 || camID >= 1 ? 0 : camID;//TODO: camCount
}

void WalkmeshManager::setCurrentCamera(int camID)
{
	if(!walkmeshFile->isOpenCa() || camID < 0) {
		return;
	}

	bool hasCamera = camID < 1;//TODO: camCount

	if(hasCamera) {
		caVectorXEdit->blockSignals(true);
		caVectorYEdit->blockSignals(true);
		caVectorZEdit->blockSignals(true);
		caSpaceXEdit->blockSignals(true);
		caSpaceYEdit->blockSignals(true);
		caSpaceZEdit->blockSignals(true);
		caZoomEdit->blockSignals(true);

		caVectorXEdit->setValues(walkmeshFile->camera_axis[0]);
		caVectorYEdit->setValues(walkmeshFile->camera_axis[1]);
		caVectorZEdit->setValues(walkmeshFile->camera_axis[2]);

		caSpaceXEdit->setValue(walkmeshFile->camera_position[0]);
		caSpaceYEdit->setValue(walkmeshFile->camera_position[1]);
		caSpaceZEdit->setValue(walkmeshFile->camera_position[2]);

		caZoomEdit->setValue(walkmeshFile->camera_zoom);

//		walkmesh->setCurrentFieldCamera(camID);
		caVectorXEdit->blockSignals(false);
		caVectorYEdit->blockSignals(false);
		caVectorZEdit->blockSignals(false);
		caSpaceXEdit->blockSignals(false);
		caSpaceYEdit->blockSignals(false);
		caSpaceZEdit->blockSignals(false);
		caZoomEdit->blockSignals(false);
	}

	caVectorXEdit->setEnabled(hasCamera);
	caVectorYEdit->setEnabled(hasCamera);
	caVectorZEdit->setEnabled(hasCamera);

	caSpaceXEdit->setEnabled(hasCamera);
	caSpaceYEdit->setEnabled(hasCamera);
	caSpaceZEdit->setEnabled(hasCamera);

	caZoomEdit->setEnabled(hasCamera);

	if(camList->currentRow() != camID) {
		camList->blockSignals(true);
		camList->setCurrentRow(camID);
		camList->blockSignals(false);
	}
}

void WalkmeshManager::addCamera()
{
	/*int row = camList->currentRow();

	if(walkmeshFile->isOpenCa()) {
		CaStruct ca;
		if(row < data()->getCaFile()->cameraCount()) {
			ca = data()->getCaFile()->camera(row);
		} else {
			memset(&ca, 0, sizeof(CaStruct));
		}
		data()->getCaFile()->insertCamera(row+1, ca);
		camList->insertItem(row+1, tr("Camera %1").arg(row+1));
		for(int i=row+2 ; i<camList->count() ; ++i) {
			camList->item(i)->setText(tr("Camera %1").arg(i));
		}
		camList->setCurrentRow(row+1);
		emit modified();
	}*/
}

void WalkmeshManager::removeCamera()
{
	/*if(data()->getCaFile()->cameraCount() < 2) return;

	int row = camList->currentRow();

	if(row < 0)		return;

	if(walkmeshFile->isOpenCa() && row < data()->getCaFile()->cameraCount()) {
		data()->getCaFile()->removeCamera(row);
		delete camList->item(row);
		for(int i=row ; i<camList->count() ; ++i) {
			camList->item(i)->setText(tr("Camera %1").arg(i));
		}
		setCurrentCamera(row);
		emit modified();
	}*/
}

void WalkmeshManager::editCaVector(const Vertex_s &values)
{
	QObject *s = sender();

	if(s == caVectorXEdit)			editCaVector(0, values);
	else if(s == caVectorYEdit)		editCaVector(1, values);
	else if(s == caVectorZEdit)		editCaVector(2, values);
}

void WalkmeshManager::editCaVector(int id, const Vertex_s &values)
{
	qDebug() << "editCaVector" << id;
	if(walkmeshFile->isOpenCa()/* && data()->getCaFile()->cameraCount() > 0*/) {
//		const int camID = currentCamera();
//		CaStruct cam = data()->getCaFile()->camera(camID);
		Vertex_s oldV = walkmeshFile->camera_axis[id];

		if(oldV.x != values.x || oldV.y != values.y || oldV.z != values.z) {
			walkmeshFile->camera_axis[id] = values;
//			data()->getCaFile()->setCamera(camID, cam);
			walkmesh->updateGL();
			emit modified();
		}
	}
}

void WalkmeshManager::editCaPos(double value)
{
	QObject *s = sender();

	if(s == caSpaceXEdit)			editCaPos(0, value);
	else if(s == caSpaceYEdit)		editCaPos(1, value);
	else if(s == caSpaceZEdit)		editCaPos(2, value);
}

void WalkmeshManager::editCaPos(int id, double value)
{
	if(walkmeshFile->isOpenCa()/* && data()->getCaFile()->cameraCount() > 0*/) {
//		const int camID = currentCamera();
//		CaStruct cam = data()->getCaFile()->camera(camID);
		if(walkmeshFile->camera_position[id] != (qint32)value) {
			walkmeshFile->camera_position[id] = value;
//			data()->getCaFile()->setCamera(camID, cam);
			walkmesh->updateGL();
			emit modified();
		}
	}
}

void WalkmeshManager::editCaZoom(int value)
{
	if(walkmeshFile->isOpenCa()/* && data()->getCaFile()->cameraCount() > 0*/) {
//		const int camID = currentCamera();
//		CaStruct cam = data()->getCaFile()->camera(camID);
		if(walkmeshFile->camera_zoom != value) {
			walkmeshFile->camera_zoom = value;
//			data()->getCaFile()->setCamera(camID, cam);
//			walkmesh->updatePerspective();
			emit modified();
		}
	}
}

void WalkmeshManager::setCurrentId(int i)
{
	if(!walkmeshFile->isOpenId() || i < 0)	return;

	if(walkmeshFile->triangles.size() <= i)	return;

	const Triangle &triangle = walkmeshFile->triangles.at(i);
	const Access &access = walkmeshFile->access.at(i);

	idVertices[0]->setValues(WalkmeshFile::toVertex_s(triangle.vertices[0]));
	idVertices[1]->setValues(WalkmeshFile::toVertex_s(triangle.vertices[1]));
	idVertices[2]->setValues(WalkmeshFile::toVertex_s(triangle.vertices[2]));

	idAccess[0]->setValue(access.a[0]);
	idAccess[1]->setValue(access.a[1]);
	idAccess[2]->setValue(access.a[2]);

//	walkmesh->setSelectedTriangle(i);
}

void WalkmeshManager::addTriangle()
{
	int row = idList->currentRow();

	if(walkmeshFile->isOpenId()) {
		Triangle tri;
		Access acc;
		if(row < walkmeshFile->triangles.size()) {
			tri = walkmeshFile->triangles.at(row);
			acc = walkmeshFile->access.at(row);
		} else {
			memset(&tri, 0, sizeof(Triangle));
			memset(&acc, 0, sizeof(Access));
		}
		walkmeshFile->triangles.insert(row+1, tri);
		walkmeshFile->access.insert(row+1, acc);
		idList->insertItem(row+1, tr("Triangle %1").arg(row+1));
		for(int i=row+2 ; i<idList->count() ; ++i) {
			idList->item(i)->setText(tr("Triangle %1").arg(i));
		}
		idList->setCurrentRow(row+1);
		emit modified();
	}
}

void WalkmeshManager::removeTriangle()
{
	int row = idList->currentRow();

	if(row < 0)		return;

	if(walkmeshFile->isOpenId() && row < walkmeshFile->triangles.size()) {
		walkmeshFile->triangles.removeAt(row);
		walkmeshFile->access.removeAt(row);
		delete idList->item(row);
		for(int i=row ; i<idList->count() ; ++i) {
			idList->item(i)->setText(tr("Triangle %1").arg(i));
		}
		setCurrentId(row);
		emit modified();
	}
}

void WalkmeshManager::editIdTriangle(const Vertex_s &values)
{
	QObject *s = sender();

	if(s == idVertices[0])			editIdTriangle(0, values);
	else if(s == idVertices[1])		editIdTriangle(1, values);
	else if(s == idVertices[2])		editIdTriangle(2, values);
}

void WalkmeshManager::editIdTriangle(int id, const Vertex_s &values)
{
	if(walkmeshFile->isOpenId()) {
		const int triangleID = idList->currentRow();
		if(triangleID > -1 && triangleID < walkmeshFile->triangles.size()) {
			Triangle old = walkmeshFile->triangles.at(triangleID);
			Vertex_sr &oldV = old.vertices[id];
			if(oldV.x != values.x || oldV.y != values.y || oldV.z != values.z) {
				oldV = WalkmeshFile::fromVertex_s(values);
				walkmeshFile->triangles.replace(triangleID, old);
				walkmesh->updateGL();
				emit modified();
			}
		}
	}
}

void WalkmeshManager::editIdAccess(int value)
{
	QObject *s = sender();

	if(s == idAccess[0])			editIdAccess(0, value);
	else if(s == idAccess[1])		editIdAccess(1, value);
	else if(s == idAccess[2])		editIdAccess(2, value);
}

void WalkmeshManager::editIdAccess(int id, int value)
{
	if(walkmeshFile->isOpenId()) {
		const int triangleID = idList->currentRow();
		if(triangleID > -1 && triangleID < walkmeshFile->access.size()) {
			Access old = walkmeshFile->access.at(triangleID);
			qint16 oldV = old.a[id];
			if(oldV != value) {
				old.a[id] = value;
				walkmeshFile->access.replace(triangleID, old);
				walkmesh->updateGL();
				emit modified();
			}
		}
	}
}

void WalkmeshManager::setCurrentGateway(int id)
{
	if(!infFile->isOpen() || id < 0 || 12 <= id)    return;

	const Exit &gateway = infFile->exitLine(id);

	exitPoints[0]->setValues(gateway.exit_line[0]);
	exitPoints[1]->setValues(gateway.exit_line[1]);
	entryPoint->setValues(gateway.destination);
	fieldId->setValue(gateway.fieldID);

	unknownExit->setData(QByteArray((char *)&gateway.u1, 4));

//	walkmesh->setSelectedGate(id);
}

void WalkmeshManager::setCurrentDoor(int id)
{
	if(!infFile->isOpen() || id < 0 || 12 <= id)    return;

	const Trigger &trigger = infFile->trigger(id);

	doorPosition[0]->setValues(trigger.trigger_line[0]);
	doorPosition[1]->setValues(trigger.trigger_line[1]);
	bgParamId->setValue(trigger.background_parameter);
	bgStateId->setValue(trigger.background_state);
	doorBehavior->setValue(trigger.behavior);
	doorSoundId->setValue(trigger.soundID);

//	walkmesh->setSelectedDoor(id);
}

void WalkmeshManager::editExitPoint(const Vertex_s &values)
{
	QObject *s = sender();

	if(s == exitPoints[0])			editExitPoint(0, values);
	else if(s == exitPoints[1])		editExitPoint(1, values);
}

void WalkmeshManager::editExitPoint(int id, const Vertex_s &values)
{
	if(infFile->isOpen()) {
		int gateId = gateList->currentRow();
		Exit old = infFile->exitLine(gateId);
		Vertex_s oldVertex = old.exit_line[id];
		if(oldVertex.x != values.x || oldVertex.y != values.y || oldVertex.z != values.z) {
			old.exit_line[id] = values;
			infFile->setExitLine(gateId, old);
			walkmesh->updateGL();
			emit modified();
		}
	}
}

void WalkmeshManager::editEntryPoint(const Vertex_s &values)
{
	if(infFile->isOpen()) {
		int gateId = gateList->currentRow();
		Exit old = infFile->exitLine(gateId);
		Vertex_s oldVertex = old.destination;
		if(oldVertex.x != values.x || oldVertex.y != values.y || oldVertex.z != values.z) {
			old.destination = values;
			infFile->setExitLine(gateId, old);
			emit modified();
		}
	}
}

void WalkmeshManager::editDoorPoint(const Vertex_s &values)
{
	QObject *s = sender();

	if(s == doorPosition[0])			editDoorPoint(0, values);
	else if(s == doorPosition[1])		editDoorPoint(1, values);
}

void WalkmeshManager::editDoorPoint(int id, const Vertex_s &values)
{
	if(infFile->isOpen()) {
		int gateId = gateList->currentRow();
		Trigger old = infFile->trigger(gateId);
		Vertex_s oldVertex = old.trigger_line[id];
		if(oldVertex.x != values.x || oldVertex.y != values.y || oldVertex.z != values.z) {
			old.trigger_line[id] = values;
			infFile->setTrigger(gateId, old);
			walkmesh->updateGL();
			emit modified();
		}
	}
}

void WalkmeshManager::editUnknownExit(const QByteArray &u)
{
	if(infFile->isOpen()) {
		int gateId = gateList->currentRow();
		const char *uData = u.constData();
		Exit old = infFile->exitLine(gateId);
		memcpy(&old.u1, uData, 4);
		if(old.u1 != infFile->exitLine(gateId).u1) {
			infFile->setExitLine(gateId, old);
			emit modified();
		}
	}
}

void WalkmeshManager::editFieldId(int v)
{
	if(infFile->isOpen()) {
		int gateId = gateList->currentRow();
		Exit old = infFile->exitLine(gateId);
		if(old.fieldID != v) {
			old.fieldID = v;
			infFile->setExitLine(gateId, old);
			if(v != 0x7FFF) {
				gateList->currentItem()->setText(QString("%1 (%2)").arg(Data::field_names.value(v)).arg(v));
			} else {
				gateList->currentItem()->setText(tr("Inutilisé"));
			}

			emit modified();
		}
	}
}

void WalkmeshManager::editParamId(int v)
{
	if(infFile->isOpen()) {
		int gateId = doorList->currentRow();
		Trigger old = infFile->trigger(gateId);
		if(old.background_parameter != v) {
			old.background_parameter = v;
			infFile->setTrigger(gateId, old);
			if(v != 0xFF) {
				doorList->currentItem()->setText(tr("Porte %1").arg(gateId));
			} else {
				doorList->currentItem()->setText(tr("Inutilisé"));
			}

			walkmesh->updateGL();
			emit modified();
		}
	}
}

void WalkmeshManager::editStateId(int v)
{
	if(infFile->isOpen()) {
		int gateId = doorList->currentRow();
		Trigger old = infFile->trigger(gateId);
		if(old.background_state != v) {
			old.background_state = v;
			infFile->setTrigger(gateId, old);

			walkmesh->updateGL();
			emit modified();
		}
	}
}

void WalkmeshManager::editBehavior(int v)
{
	if(infFile->isOpen()) {
		int gateId = doorList->currentRow();
		Trigger old = infFile->trigger(gateId);
		if(old.behavior != v) {
			old.behavior = v;
			infFile->setTrigger(gateId, old);

			walkmesh->updateGL();
			emit modified();
		}
	}
}

void WalkmeshManager::editSoundId(int v)
{
	if(infFile->isOpen()) {
		int gateId = doorList->currentRow();
		Trigger old = infFile->trigger(gateId);
		if(old.soundID != v) {
			old.soundID = v;
			infFile->setTrigger(gateId, old);

			walkmesh->updateGL();
			emit modified();
		}
	}
}

void WalkmeshManager::setCurrentRange1(int id)
{
	if(!infFile->isOpen())    return;

	const Range &range = infFile->cameraRange();

	rangeEdit1[0]->setValue(range.top);
	rangeEdit1[1]->setValue(range.bottom);
	rangeEdit1[2]->setValue(range.right);
	rangeEdit1[3]->setValue(range.left);
}

void WalkmeshManager::setCurrentRange2(int id)
{
	if(!infFile->isOpen())    return;

	const Range &range = infFile->screenRange();

	rangeEdit2[0]->setValue(range.top);
	rangeEdit2[1]->setValue(range.bottom);
	rangeEdit2[2]->setValue(range.right);
	rangeEdit2[3]->setValue(range.left);
}

void WalkmeshManager::editRange(int v)
{
	QObject *s = sender();

	if(s == rangeEdit1[0])			editRange1(0, v);
	else if(s == rangeEdit1[1])		editRange1(1, v);
	else if(s == rangeEdit1[2])		editRange1(2, v);
	else if(s == rangeEdit1[3])		editRange1(3, v);
	else if(s == rangeEdit2[0])		editRange2(0, v);
	else if(s == rangeEdit2[1])		editRange2(1, v);
	else if(s == rangeEdit2[2])		editRange2(2, v);
	else if(s == rangeEdit2[3])		editRange2(3, v);
}

void WalkmeshManager::editRange1(int id, int v)
{
	if(infFile->isOpen()) {
		const int currentRange = rangeList1->currentRow();
		Range old = infFile->cameraRange();
		qint16 oldv=0;

		switch(id) {
		case 0:	oldv = old.top;		break;
		case 1:	oldv = old.bottom;	break;
		case 2:	oldv = old.right;	break;
		case 3:	oldv = old.left;	break;
		}

		if(oldv != v) {
			switch(id) {
			case 0:	old.top = v;	break;
			case 1:	old.bottom = v;	break;
			case 2:	old.right = v;	break;
			case 3:	old.left = v;	break;
			}
			infFile->setCameraRange(old);
			emit modified();
		}
	}
}

void WalkmeshManager::editRange2(int id, int v)
{
	if(infFile->isOpen()) {
		const int currentRange = rangeList2->currentRow();
		Range old = infFile->screenRange();
		qint16 oldv=0;

		switch(id) {
		case 0:	oldv = old.top;		break;
		case 1:	oldv = old.bottom;	break;
		case 2:	oldv = old.right;	break;
		case 3:	oldv = old.left;	break;
		}

		if(oldv != v) {
			switch(id) {
			case 0:	old.top = v;	break;
			case 1:	old.bottom = v;	break;
			case 2:	old.right = v;	break;
			case 3:	old.left = v;	break;
			}
			infFile->setScreenRange(old);
			emit modified();
		}
	}
}

void WalkmeshManager::editNavigation(int v)
{
	if(infFile->isOpen()) {
		int old = infFile->control();
		if(old != v) {
			navigation->setValue(v);
			infFile->setControl(v);
			emit modified();
		}
	}
}

/*void WalkmeshManager::editUnknown(const QByteArray &u)
{
	if(infFile->isOpen()) {
		if(u != data()->getInfFile()->unknown()) {
			data()->getInfFile()->setUnknown(u);
			emit modified();
		}
	}
}*/

/*void WalkmeshManager::editCameraFocus(int value)
{
	if(infFile->isOpen()) {
		if(value != data()->getInfFile()->cameraFocusHeight()) {
			data()->getInfFile()->setCameraFocusHeight(value);
			emit modified();
		}
	}
}*/
