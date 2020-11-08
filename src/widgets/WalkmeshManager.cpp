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
#include "core/Config.h"
#include "Data.h"

WalkmeshManager::WalkmeshManager(QWidget *parent) :
	QDialog(parent, Qt::Tool),
    idFile(nullptr), caFile(nullptr), infFile(nullptr), scriptsAndTexts(nullptr)
{
	setWindowTitle(tr("Walkmesh"));

	walkmesh = Config::value("OpenGL", true).toBool() ? new WalkmeshWidget() : nullptr;
	QWidget *walkmeshWidget = walkmesh ? walkmesh : new QWidget(this);

	slider1 = new QSlider(this);
	slider2 = new QSlider(this);
	slider3 = new QSlider(this);

	slider1->setRange(-180, 180);
	slider2->setRange(-180, 180);
	slider3->setRange(-180, 180);

	slider1->setValue(0);
	slider2->setValue(0);
	slider3->setValue(0);

	QLabel *keyInfos = new QLabel(tr("Use the arrow keys to move the camera."));
	keyInfos->setTextFormat(Qt::PlainText);
	keyInfos->setWordWrap(true);

	QPushButton *resetCamera = new QPushButton(tr("Reset Camera"));

	showModels = new QCheckBox(tr("Show 3D models"));
	showModels->setChecked(Config::value("fieldModelsVisible", true).toBool());

	tabWidget = new QTabWidget(this);
	tabWidget->addTab(buildCameraPage(), tr("Camera"));
	tabWidget->addTab(buildWalkmeshPage(), tr("Walkmesh"));
	tabWidget->addTab(buildGatewaysPage(), tr("Gateways"));
	tabWidget->addTab(buildDoorsPage(), tr("Doors"));
	tabWidget->addTab(buildArrowPage(), tr("Arrows"));
	tabWidget->addTab(buildCameraRangePage(), tr("Camera range"));
	tabWidget->addTab(buildMiscPage(), tr("Miscellaneous"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(walkmeshWidget, 0, 0, 4, 1);
	layout->addWidget(slider1, 0, 1);
	layout->addWidget(slider2, 0, 2);
	layout->addWidget(slider3, 0, 3);
	layout->addWidget(keyInfos, 1, 1, 1, 3);
	layout->addWidget(resetCamera, 2, 1, 1, 3);
	layout->addWidget(showModels, 3, 1, 1, 3);
	layout->addWidget(tabWidget, 4, 0, 1, 4);

	if (walkmesh) {
		connect(slider1, SIGNAL(valueChanged(int)), walkmesh, SLOT(setXRotation(int)));
		connect(slider2, SIGNAL(valueChanged(int)), walkmesh, SLOT(setYRotation(int)));
		connect(slider3, SIGNAL(valueChanged(int)), walkmesh, SLOT(setZRotation(int)));
		connect(resetCamera, SIGNAL(clicked()), SLOT(resetCamera()));
		connect(showModels, SIGNAL(toggled(bool)), SLOT(setModelsVisible(bool)));
	}
}

void WalkmeshManager::resetCamera()
{
	slider1->blockSignals(true);
	slider2->blockSignals(true);
	slider3->blockSignals(true);
	slider1->setValue(0);
	slider2->setValue(0);
	slider3->setValue(0);
	slider1->blockSignals(false);
	slider2->blockSignals(false);
	slider3->blockSignals(false);
	walkmesh->resetCamera();
}

void WalkmeshManager::setModelsVisible(bool show)
{
	walkmesh->setModelsVisible(show);
}

QWidget *WalkmeshManager::buildCameraPage()
{
	QWidget *ret = new QWidget(this);

	ListWidget *listWidget = new ListWidget(ret);
	listWidget->addAction(ListWidget::Add, tr("Add camera"), this, SLOT(addCamera()));
	listWidget->addAction(ListWidget::Rem, tr("Remove camera"), this, SLOT(removeCamera()));

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
	caLayout->addWidget(new QLabel(tr("Zoom:")), 0, 1, 1, 3);
	caLayout->addWidget(caZoomEdit, 0, 4, 1, 2);
	caLayout->addWidget(new QLabel(tr("Camera axis:")), 1, 1, 1, 6);
	caLayout->addWidget(caVectorXEdit, 2, 1, 1, 6);
	caLayout->addWidget(caVectorYEdit, 3, 1, 1, 6);
	caLayout->addWidget(caVectorZEdit, 4, 1, 1, 6);
	caLayout->addWidget(new QLabel(tr("Camera position:")), 5, 1, 1, 6);
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
	listWidget->addAction(ListWidget::Add, tr("Add triangle"), this, SLOT(addTriangle()));
	listWidget->addAction(ListWidget::Rem, tr("Remove triangle"), this, SLOT(removeTriangle()));

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
	accessLayout0->addWidget(new QLabel(tr("Triangle accessible via the line 1-2 :")));
	accessLayout0->addWidget(idAccess[0]);

	QHBoxLayout *accessLayout1 = new QHBoxLayout;
	accessLayout1->addWidget(new QLabel(tr("Triangle accessible via the line 2-3 :")));
	accessLayout1->addWidget(idAccess[1]);

	QHBoxLayout *accessLayout2 = new QHBoxLayout;
	accessLayout2->addWidget(new QLabel(tr("Triangle accessible via the line 3-1 :")));
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
	connect(idVertices[2], SIGNAL(valuesChanged(Vertex_s)), SLOT(editIdTriangle(Vertex_s)));
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

	gateEnabled = new QCheckBox(tr("Enable"), ret);

	exitPoints[0] = new VertexWidget(ret);
	exitPoints[1] = new VertexWidget(ret);
	entryPoint = new VertexWidget(QString(), QString(), tr("T"), ret);

	fieldId = new QSpinBox(ret);
	fieldId->setRange(-32768, 32766);

	exitDirection = new QSpinBox(ret);
	exitDirection->setRange(0, 255);

	arrowDisplay = new QCheckBox(tr("Show an arrow"), ret);
	arrowDisplay->setIcon(QIcon(":/images/field-arrow-red.png"));

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(gateList, 0, 0, 8, 1, Qt::AlignLeft);
	layout->addWidget(gateEnabled, 0, 1, 1, 2);
	layout->addWidget(new QLabel(tr("Exit line:")), 1, 1);
	layout->addWidget(exitPoints[0], 1, 2);
	layout->addWidget(exitPoints[1], 2, 2);
	layout->addWidget(new QLabel(tr("Destination point:")), 3, 1);
	layout->addWidget(entryPoint, 3, 2);
	layout->addWidget(new QLabel(tr("Character orientation:")), 4, 1);
	layout->addWidget(exitDirection, 4, 2);
	layout->addWidget(new QLabel(tr("Field ID:")), 5, 1);
	layout->addWidget(fieldId, 5, 2);
	layout->addWidget(arrowDisplay, 6, 1, 1, 2);
	layout->setRowStretch(7, 1);

	connect(gateList, SIGNAL(currentRowChanged(int)), SLOT(setCurrentGateway(int)));
	connect(gateEnabled, SIGNAL(toggled(bool)), SLOT(editGateEnabled(bool)));
	connect(exitPoints[0], SIGNAL(valuesChanged(Vertex_s)), SLOT(editExitPoint(Vertex_s)));
	connect(exitPoints[1], SIGNAL(valuesChanged(Vertex_s)), SLOT(editExitPoint(Vertex_s)));
	connect(entryPoint, SIGNAL(valuesChanged(Vertex_s)), SLOT(editEntryPoint(Vertex_s)));
	connect(fieldId, SIGNAL(valueChanged(int)), SLOT(editFieldId(int)));
	connect(arrowDisplay, SIGNAL(toggled(bool)), SLOT(editArrowDisplay(bool)));
	connect(exitDirection, SIGNAL(valueChanged(int)), SLOT(editExitDirection(int)));

	return ret;
}

QWidget *WalkmeshManager::buildDoorsPage()
{
	QWidget *ret = new QWidget(this);

	doorList = new QListWidget(ret);
	doorList->setFixedWidth(125);

	doorEnabled = new QCheckBox(tr("Enable"), ret);

	doorPosition[0] = new VertexWidget(ret);
	doorPosition[1] = new VertexWidget(ret);

	bgParamId = new QSpinBox(ret);
	bgParamId->setRange(0, 254);

	bgStateId = new QSpinBox(ret);
	bgStateId->setRange(0, 255);

	doorBehavior = new QSpinBox(ret);
	doorBehavior->setRange(0, 255);

	doorSoundId = new QSpinBox(ret);
	doorSoundId->setRange(0, 255);

	QGridLayout *idsLayout = new QGridLayout;
	idsLayout->addWidget(new QLabel(tr("Background parameter ID:")), 0, 0);
	idsLayout->addWidget(bgParamId, 0, 1);
	idsLayout->addWidget(new QLabel(tr("Background state ID:")), 1, 0);
	idsLayout->addWidget(bgStateId, 1, 1);
	idsLayout->addWidget(new QLabel(tr("Behavior:")), 2, 0);
	idsLayout->addWidget(doorBehavior, 2, 1);
	idsLayout->addWidget(new QLabel(tr("Sound ID:")), 3, 0);
	idsLayout->addWidget(doorSoundId, 3, 1);

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(doorList, 0, 0, 5, 1, Qt::AlignLeft);
	layout->addWidget(doorEnabled, 0, 1, 1, 2);
	layout->addWidget(new QLabel(tr("Trigger Line Door:")), 1, 1);
	layout->addWidget(doorPosition[0], 1, 2);
	layout->addWidget(doorPosition[1], 2, 2);
	layout->addLayout(idsLayout, 3, 1, 1, 2);
	layout->setRowStretch(4, 1);

	connect(doorList, SIGNAL(currentRowChanged(int)), SLOT(setCurrentDoor(int)));
	connect(doorEnabled, SIGNAL(toggled(bool)), SLOT(editDoorEnabled(bool)));
	connect(doorPosition[0], SIGNAL(valuesChanged(Vertex_s)), SLOT(editDoorPoint(Vertex_s)));
	connect(doorPosition[1], SIGNAL(valuesChanged(Vertex_s)), SLOT(editDoorPoint(Vertex_s)));
	connect(bgParamId, SIGNAL(valueChanged(int)), SLOT(editParamId(int)));
	connect(bgStateId, SIGNAL(valueChanged(int)), SLOT(editStateId(int)));
	connect(doorBehavior, SIGNAL(valueChanged(int)), SLOT(editBehavior(int)));
	connect(doorSoundId, SIGNAL(valueChanged(int)), SLOT(editSoundId(int)));

	return ret;
}

QWidget *WalkmeshManager::buildArrowPage()
{
	QWidget *ret = new QWidget(this);

	arrowList = new QListWidget(ret);
	arrowList->setFixedWidth(125);

	arrowX = new QSpinBox(ret);
	arrowX->setRange(-32767, 32767);

	arrowY = new QSpinBox(ret);
	arrowY->setRange(-32767, 32767);

	arrowZ = new QSpinBox(ret);
	arrowZ->setRange(-32767, 32767);

	arrowType = new QComboBox(ret);
	arrowType->addItem(tr("Invisible"), 0);
	arrowType->addItem(QIcon(":/images/field-arrow-red.png"), tr("Red"), 1);
	arrowType->addItem(QIcon(":/images/field-arrow-green.png"), tr("Green"), 2);

	QHBoxLayout *posLayout = new QHBoxLayout;
	posLayout->addWidget(new QLabel(tr("X")));
	posLayout->addWidget(arrowX);
	posLayout->addWidget(new QLabel(tr("Y")));
	posLayout->addWidget(arrowY);
	posLayout->addWidget(new QLabel(tr("Z")));
	posLayout->addWidget(arrowZ);
	posLayout->setContentsMargins(QMargins());

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(arrowList, 0, 0, 3, 1, Qt::AlignLeft);
	layout->addWidget(new QLabel(tr("Position:")), 0, 1);
	layout->addWidget(new QLabel(tr("Type:")), 1, 1);
	layout->addLayout(posLayout, 0, 2);
	layout->addWidget(arrowType, 1, 2);
	layout->setRowStretch(2, 1);
	layout->setColumnStretch(2, 1);

	connect(arrowList, SIGNAL(currentRowChanged(int)), SLOT(setCurrentArrow(int)));
	connect(arrowX, SIGNAL(valueChanged(int)), SLOT(editArrowX(int)));
	connect(arrowY, SIGNAL(valueChanged(int)), SLOT(editArrowY(int)));
	connect(arrowZ, SIGNAL(valueChanged(int)), SLOT(editArrowZ(int)));
	connect(arrowType, SIGNAL(currentIndexChanged(int)), SLOT(editArrowType(int)));

	return ret;
}

QWidget *WalkmeshManager::buildCameraRangePage()
{
	QWidget *ret = new QWidget(this);

	QGroupBox *group1 = new QGroupBox(tr("Camera range"), ret);
	QGroupBox *group2 = new QGroupBox(tr("Layer sizes (for layer animations)"), ret);
	QGroupBox *group3 = new QGroupBox(tr("Layer flags"), ret);

	for (int i=0; i<4; ++i) {
		rangeEdit[i] = new QSpinBox(group1);
		rangeEdit[i]->setRange(-32768, 32767);
		bgSizeEdit[i] = new QSpinBox(group2);
		bgSizeEdit[i]->setRange(-32768, 32767);
		bgFlagEdit[i] = new QSpinBox(group3);
		bgFlagEdit[i]->setRange(0, 255);
	}
	QGridLayout *layout1 = new QGridLayout(group1);
	layout1->addWidget(new QLabel(tr("Top")), 0, 0);
	layout1->addWidget(rangeEdit[0], 0, 1);
	layout1->addWidget(new QLabel(tr("Bottom")), 0, 2);
	layout1->addWidget(rangeEdit[1], 0, 3);
	layout1->addWidget(new QLabel(tr("Right")), 1, 0);
	layout1->addWidget(rangeEdit[2], 1, 1);
	layout1->addWidget(new QLabel(tr("Left")), 1, 2);
	layout1->addWidget(rangeEdit[3], 1, 3);
	layout1->setColumnStretch(0, 1);
	layout1->setColumnStretch(1, 1);
	layout1->setColumnStretch(2, 1);
	layout1->setColumnStretch(3, 1);

	QGridLayout *layout2 = new QGridLayout(group2);
	layout2->addWidget(new QLabel(tr("Background layer 3 width")), 0, 0);
	layout2->addWidget(bgSizeEdit[0], 0, 1);
	layout2->addWidget(new QLabel(tr("Background layer 3 height")), 0, 2);
	layout2->addWidget(bgSizeEdit[1], 0, 3);
	layout2->addWidget(new QLabel(tr("Background layer 4 width")), 1, 0);
	layout2->addWidget(bgSizeEdit[2], 1, 1);
	layout2->addWidget(new QLabel(tr("Background layer 4 height")), 1, 2);
	layout2->addWidget(bgSizeEdit[3], 1, 3);
	layout2->setRowStretch(2, 1);
	layout2->setColumnStretch(0, 1);
	layout2->setColumnStretch(1, 1);
	layout2->setColumnStretch(2, 1);
	layout2->setColumnStretch(3, 1);

	QGridLayout *layout3 = new QGridLayout(group3);
	layout3->addWidget(new QLabel(tr("Layer 1")), 0, 0);
	layout3->addWidget(bgFlagEdit[0], 0, 1);
	layout3->addWidget(new QLabel(tr("Layer 2")), 0, 2);
	layout3->addWidget(bgFlagEdit[1], 0, 3);
	layout3->addWidget(new QLabel(tr("Layer 3")), 0, 4);
	layout3->addWidget(bgFlagEdit[2], 0, 5);
	layout3->addWidget(new QLabel(tr("Layer 4")), 0, 6);
	layout3->addWidget(bgFlagEdit[3], 0, 7);
	layout3->setRowStretch(1, 1);

	QVBoxLayout *layout = new QVBoxLayout(ret);
	layout->addWidget(group1);
	layout->addWidget(group2);
	layout->addWidget(group3);
	//layout->addStretch();

	for (int i=0; i<4; ++i) {
		connect(rangeEdit[i], SIGNAL(valueChanged(int)), SLOT(editRange(int)));
		connect(bgSizeEdit[i], SIGNAL(valueChanged(int)), SLOT(editRange(int)));
		connect(bgFlagEdit[i], SIGNAL(valueChanged(int)), SLOT(editRange(int)));
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

	cameraFocusHeight = new QSpinBox(ret);
	cameraFocusHeight->setRange(-32768, 32767);

	unknown = new HexLineEdit(ret);

	mapScale = new QSpinBox(this);
	mapScale->setRange(0, 65535);

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(new QLabel(tr("Movements orientation:")), 0, 0);
	layout->addWidget(navigation, 0, 1);
	layout->addWidget(navigation2, 0, 2);
	layout->addWidget(new QLabel(tr("Camera Focus Height on the playable character:")), 1, 0);
	layout->addWidget(cameraFocusHeight, 1, 1, 1, 2);
	layout->addWidget(new QLabel(tr("Unknown:")), 2, 0);
	layout->addWidget(unknown, 2, 1, 1, 2);
	layout->addWidget(new QLabel(tr("Field map scale:")), 3, 0);
	layout->addWidget(mapScale, 3, 1, 1, 2);
	layout->setRowStretch(4, 1);

	connect(navigation, SIGNAL(valueEdited(int)), navigation2, SLOT(setValue(int)));
	connect(navigation2, SIGNAL(valueChanged(int)), SLOT(editNavigation(int)));
	connect(cameraFocusHeight, SIGNAL(valueChanged(int)), SLOT(editCameraFocusHeight(int)));
	connect(unknown, SIGNAL(dataEdited(QByteArray)), SLOT(editUnknown(QByteArray)));
	connect(mapScale, SIGNAL(valueChanged(int)), SLOT(editMapScale(int)));

	return ret;
}

void WalkmeshManager::saveConfig()
{
	Config::setValue("fieldModelsVisible", showModels->isChecked());
}

void WalkmeshManager::fill(Field *field, bool reload)
{
	if (!field || (!reload && field->inf() == infFile))	return;

	infFile = field->inf();
	idFile = field->walkmesh();
	caFile = field->camera();
	scriptsAndTexts = field->scriptsAndTexts();

	walkmesh->setModelsVisible(showModels->isChecked());

	if (!idFile->isOpen() || !caFile->isOpen() || !infFile->isOpen()) {
		QMessageBox::warning(this, tr("Opening error"), tr("Error opening walkmesh"));
	}

	int camCount = 0;

	if (walkmesh) {
		walkmesh->fill(field);
	}

	if (caFile->isOpen()) {
		camCount = caFile->cameraCount();

		if (camList->count() != camCount) {
			camList->blockSignals(true);
			camList->clear();
			for (int i=0; i<camCount; ++i) {
				camList->addItem(tr("Camera %1").arg(i));
			}
			camList->blockSignals(false);
		}

		setCurrentCamera(0);
	}
	tabWidget->widget(0)->setEnabled(caFile->isOpen() && camCount > 0);

	if (idFile->isOpen()) {
		int triangleCount = idFile->triangleCount();

		if (idList->count() != triangleCount) {
			idList->blockSignals(true);
			idList->clear();
			for (int i=0; i<triangleCount; ++i) {
				idList->addItem(tr("Triangle %1").arg(i));
			}
			idList->blockSignals(false);
		}
		idList->setCurrentRow(0);
		setCurrentId(0);
	}
	tabWidget->widget(1)->setEnabled(idFile->isOpen());

	if (infFile->isOpen()) {
		gateList->clear();
		for (const Exit &gateway : infFile->exitLines()) {
			if (gateway.fieldID != 0x7FFF) {
				gateList->addItem(QString("%1 (%2)").arg(Data::mapName(gateway.fieldID)).arg(gateway.fieldID));
			} else {
				QListWidgetItem *item = new QListWidgetItem(tr("Unused"));
				item->setForeground(Qt::darkGray);
				gateList->addItem(item);
			}
		}
		gateList->setCurrentRow(0);
		setCurrentGateway(0);

		doorList->clear();
		int doorID = 0;
		for (const Trigger &trigger : infFile->triggers()) {
			if (trigger.background_parameter != 0xFF) {
				doorList->addItem(tr("Door %1").arg(doorID));
			} else {
				QListWidgetItem *item = new QListWidgetItem(tr("Unused"));
				item->setForeground(Qt::darkGray);
				doorList->addItem(item);
			}
			++doorID;
		}
		doorList->setCurrentRow(0);
		setCurrentDoor(0);

		if (!infFile->isJap()) {
			arrowList->clear();
			int arrowID = 0;
			for (const Arrow &arrow : infFile->arrows()) {
				if (arrow.type != 0) {
					arrowList->addItem(tr("Arrow %1").arg(arrowID));
				} else {
					QListWidgetItem *item = new QListWidgetItem(tr("Unused"));
					item->setForeground(Qt::darkGray);
					arrowList->addItem(item);
				}
				++arrowID;
			}
			arrowList->setCurrentRow(0);
			setCurrentArrow(0);
		}
		arrowDisplay->setVisible(!infFile->isJap());

		const Range &range1 = infFile->cameraRange();

		rangeEdit[0]->setValue(range1.top);
		rangeEdit[1]->setValue(range1.bottom);
		rangeEdit[2]->setValue(range1.right);
		rangeEdit[3]->setValue(range1.left);

		bgSizeEdit[0]->setValue(infFile->bgLayer3Width());
		bgSizeEdit[1]->setValue(infFile->bgLayer3Height());
		bgSizeEdit[2]->setValue(infFile->bgLayer4Width());
		bgSizeEdit[3]->setValue(infFile->bgLayer4Height());

		bgFlagEdit[0]->setValue(infFile->bgLayer1Flag());
		bgFlagEdit[1]->setValue(infFile->bgLayer2Flag());
		bgFlagEdit[2]->setValue(infFile->bgLayer3Flag());
		bgFlagEdit[3]->setValue(infFile->bgLayer4Flag());

		navigation->setValue(infFile->control());
		navigation2->setValue(infFile->control());

		cameraFocusHeight->setValue(infFile->cameraFocusHeight());
		unknown->setData(infFile->unknown());
	}
	navigation->setEnabled(infFile->isOpen());
	navigation2->setEnabled(infFile->isOpen());
	cameraFocusHeight->setEnabled(infFile->isOpen());
	unknown->setEnabled(infFile->isOpen());

	if (scriptsAndTexts->isOpen()) {
		mapScale->setValue(scriptsAndTexts->scale());
	}
	mapScale->setEnabled(scriptsAndTexts->isOpen());

	tabWidget->widget(2)->setEnabled(infFile->isOpen());
	tabWidget->widget(3)->setEnabled(infFile->isOpen());
	tabWidget->widget(4)->setEnabled(infFile->isOpen() && !infFile->isJap());
	tabWidget->widget(5)->setEnabled(infFile->isOpen());
	tabWidget->widget(6)->setEnabled(infFile->isOpen() || scriptsAndTexts->isOpen());
}

void WalkmeshManager::clear()
{
	infFile = nullptr;
	idFile = nullptr;
	caFile = nullptr;
	scriptsAndTexts = nullptr;

	if (walkmesh) {
		walkmesh->clear();
	}
}

int WalkmeshManager::currentCamera() const
{
	if (!caFile->isOpen())	return 0;

	int camID = camList->currentRow();
	return camID < 0 || camID >= caFile->cameraCount() ? 0 : camID;
}

void WalkmeshManager::setCurrentCamera(int camID)
{
	if (!caFile->isOpen() || camID < 0) {
		return;
	}

	bool hasCamera = camID < caFile->cameraCount();

	if (hasCamera) {
		const Camera &cam = caFile->camera(camID);

		caVectorXEdit->blockSignals(true);
		caVectorYEdit->blockSignals(true);
		caVectorZEdit->blockSignals(true);
		caSpaceXEdit->blockSignals(true);
		caSpaceYEdit->blockSignals(true);
		caSpaceZEdit->blockSignals(true);
		caZoomEdit->blockSignals(true);

		caVectorXEdit->setValues(cam.camera_axis[0]);
		caVectorYEdit->setValues(cam.camera_axis[1]);
		caVectorZEdit->setValues(cam.camera_axis[2]);

		caSpaceXEdit->setValue(cam.camera_position[0]);
		caSpaceYEdit->setValue(cam.camera_position[1]);
		caSpaceZEdit->setValue(cam.camera_position[2]);

		caZoomEdit->setValue(cam.camera_zoom);

		if (walkmesh)	walkmesh->setCurrentFieldCamera(camID);
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

	if (camList->currentRow() != camID) {
		camList->blockSignals(true);
		camList->setCurrentRow(camID);
		camList->blockSignals(false);
	}
}

void WalkmeshManager::addCamera()
{
	int row = camList->currentRow();

	if (caFile->isOpen()) {
		Camera ca;
		if (row < caFile->cameraCount()) {
			ca = caFile->camera(row);
		} else {
			ca = Camera();
		}
		caFile->insertCamera(row+1, ca);
		camList->insertItem(row+1, tr("Camera %1").arg(row+1));
		for (int i=row+2; i<camList->count(); ++i) {
			camList->item(i)->setText(tr("Camera %1").arg(i));
		}
		camList->setCurrentRow(row+1);

		emit modified();
	}
}

void WalkmeshManager::removeCamera()
{
	if (caFile->cameraCount() < 2) return;

	int row = camList->currentRow();

	if (row < 0)		return;

	if (caFile->isOpen() && row < caFile->cameraCount()) {
		if (caFile->removeCamera(row)) {
			delete camList->item(row);
			for (int i=row; i<camList->count(); ++i) {
				camList->item(i)->setText(tr("Camera %1").arg(i));
			}
			setCurrentCamera(row);

			emit modified();
		}
	}
}

void WalkmeshManager::editCaVector(const Vertex_s &values)
{
	QObject *s = sender();

	if (s == caVectorXEdit)			editCaVector(0, values);
	else if (s == caVectorYEdit)		editCaVector(1, values);
	else if (s == caVectorZEdit)		editCaVector(2, values);
}

void WalkmeshManager::editCaVector(int id, const Vertex_s &values)
{
	if (caFile->isOpen() && caFile->hasCamera()) {
		const int camID = currentCamera();
		Camera cam = caFile->camera(camID);
		Vertex_s oldV = cam.camera_axis[id];

		if (oldV.x != values.x || oldV.y != values.y || oldV.z != values.z) {
			cam.camera_axis[id] = values;
			caFile->setCamera(camID, cam);
			if (walkmesh)	walkmesh->update();

			emit modified();
		}
	}
}

void WalkmeshManager::editCaPos(double value)
{
	QObject *s = sender();

	if (s == caSpaceXEdit)			editCaPos(0, value);
	else if (s == caSpaceYEdit)		editCaPos(1, value);
	else if (s == caSpaceZEdit)		editCaPos(2, value);
}

void WalkmeshManager::editCaPos(int id, double value)
{
	if (caFile->isOpen() && caFile->hasCamera()) {
		const int camID = currentCamera();
		Camera cam = caFile->camera(camID);
		if (cam.camera_position[id] != qint32(value)) {
			cam.camera_position[id] = qint32(value);
			caFile->setCamera(camID, cam);
			walkmesh->update();

			emit modified();
		}
	}
}

void WalkmeshManager::editCaZoom(int value)
{
	if (caFile->isOpen() && caFile->hasCamera()) {
		const int camID = currentCamera();
		Camera cam = caFile->camera(camID);
		if (cam.camera_zoom != value) {
			cam.camera_zoom = quint16(value);
			caFile->setCamera(camID, cam);
			if (walkmesh)	walkmesh->updatePerspective();

			emit modified();
		}
	}
}

void WalkmeshManager::setCurrentId(int i)
{
	if (!idFile->isOpen() || i < 0)	return;

	if (idFile->triangleCount() <= i)	return;

	const Triangle &triangle = idFile->triangle(i);
	const Access &access = idFile->access(i);

	idVertices[0]->setValues(IdFile::toVertex_s(triangle.vertices[0]));
	idVertices[1]->setValues(IdFile::toVertex_s(triangle.vertices[1]));
	idVertices[2]->setValues(IdFile::toVertex_s(triangle.vertices[2]));

	idAccess[0]->setValue(access.a[0]);
	idAccess[1]->setValue(access.a[1]);
	idAccess[2]->setValue(access.a[2]);

	if (walkmesh)	walkmesh->setSelectedTriangle(i);
}

void WalkmeshManager::addTriangle()
{
	int row = idList->currentRow();

	if (idFile->isOpen()) {
		Triangle tri;
		Access acc;
		if (row < idFile->triangleCount()) {
			tri = idFile->triangle(row);
			acc = idFile->access(row);
		} else {
			tri = Triangle();
			acc = Access();
		}
		idFile->insertTriangle(row+1, tri, acc);
		idList->insertItem(row+1, tr("Triangle %1").arg(row+1));
		for (int i=row+2; i<idList->count(); ++i) {
			idList->item(i)->setText(tr("Triangle %1").arg(i));
		}
		idList->setCurrentRow(row+1);

		emit modified();
	}
}

void WalkmeshManager::removeTriangle()
{
	int row = idList->currentRow();

	if (row < 0)		return;

	if (idFile->isOpen() && row < idFile->triangleCount()) {
		idFile->removeTriangle(row);
		delete idList->item(row);
		for (int i=row; i<idList->count(); ++i) {
			idList->item(i)->setText(tr("Triangle %1").arg(i));
		}
		setCurrentId(row);

		emit modified();
	}
}

void WalkmeshManager::editIdTriangle(const Vertex_s &values)
{
	QObject *s = sender();

	if (s == idVertices[0])			editIdTriangle(0, values);
	else if (s == idVertices[1])		editIdTriangle(1, values);
	else if (s == idVertices[2])		editIdTriangle(2, values);
}

void WalkmeshManager::editIdTriangle(int id, const Vertex_s &values)
{
	if (idFile->isOpen()) {
		const int triangleID = idList->currentRow();
		if (triangleID > -1 && triangleID < idFile->triangleCount()) {
			Triangle old = idFile->triangle(triangleID);
			Vertex_sr &oldV = old.vertices[id];
			if (oldV.x != values.x || oldV.y != values.y || oldV.z != values.z) {
				oldV = IdFile::fromVertex_s(values);
				idFile->setTriangle(triangleID, old);
				if (walkmesh)	walkmesh->update();

				emit modified();
			}
		}
	}
}

void WalkmeshManager::editIdAccess(int value)
{
	QObject *s = sender();

	if (s == idAccess[0])			editIdAccess(0, value);
	else if (s == idAccess[1])		editIdAccess(1, value);
	else if (s == idAccess[2])		editIdAccess(2, value);
}

void WalkmeshManager::editIdAccess(int id, int value)
{
	if (idFile->isOpen()) {
		const int triangleID = idList->currentRow();
		if (triangleID > -1 && triangleID < idFile->triangleCount()) {
			Access old = idFile->access(triangleID);
			qint16 oldV = old.a[id];
			if (oldV != value) {
				old.a[id] = qint16(value);
				idFile->setAccess(triangleID, old);
				if (walkmesh)	walkmesh->update();

				emit modified();
			}
		}
	}
}

void WalkmeshManager::setCurrentGateway(int id)
{
	if (!infFile->isOpen() || id < 0 || 12 <= id)    return;

	const Exit &gateway = infFile->exitLine(quint8(id));

	exitPoints[0]->setValues(gateway.exit_line[0]);
	exitPoints[1]->setValues(gateway.exit_line[1]);
	entryPoint->setValues(gateway.destination);
	fieldId->blockSignals(true);
	gateEnabled->blockSignals(true);
	if (gateway.fieldID == 0x7FFF) {
		gateEnabled->setChecked(false);
		fieldId->setValue(0);
	} else {
		gateEnabled->setChecked(true);
		fieldId->setValue(gateway.fieldID);
	}
	fieldId->blockSignals(false);
	gateEnabled->blockSignals(false);
	setGateEnabled(gateEnabled->isChecked());
	if (!infFile->isJap()) {
		arrowDisplay->setChecked(infFile->arrowIsDisplayed(quint8(id)));
	}

	exitDirection->setValue(gateway.dir);

	if (walkmesh)	walkmesh->setSelectedGate(id);
}

void WalkmeshManager::setCurrentDoor(int id)
{
	if (!infFile->isOpen() || id < 0 || 12 <= id)    return;

	const Trigger &trigger = infFile->trigger(quint8(id));

	doorEnabled->blockSignals(true);
	bgParamId->blockSignals(true);
	doorPosition[0]->blockSignals(true);
	doorPosition[1]->blockSignals(true);
	bgStateId->blockSignals(true);
	doorBehavior->blockSignals(true);
	doorSoundId->blockSignals(true);

	doorPosition[0]->setValues(trigger.trigger_line[0]);
	doorPosition[1]->setValues(trigger.trigger_line[1]);

	if (trigger.background_parameter == 0xFF) {
		doorEnabled->setChecked(false);
		bgParamId->setValue(0);
	} else {
		doorEnabled->setChecked(true);
		bgParamId->setValue(trigger.background_parameter);
	}

	bgStateId->setValue(trigger.background_state);
	doorBehavior->setValue(trigger.behavior);
	doorSoundId->setValue(trigger.soundID);

	doorEnabled->blockSignals(false);
	bgParamId->blockSignals(false);
	doorPosition[0]->blockSignals(false);
	doorPosition[1]->blockSignals(false);
	bgStateId->blockSignals(false);
	doorBehavior->blockSignals(false);
	doorSoundId->blockSignals(false);

	setDoorEnabled(doorEnabled->isChecked());


	if (walkmesh)	walkmesh->setSelectedDoor(id);
}

void WalkmeshManager::setCurrentArrow(int id)
{
	if (!infFile->isOpen() || id < 0 || 12 <= id)    return;

	const Arrow &arrow = infFile->arrow(quint8(id));

	arrowX->setValue(qint16(arrow.positionX));
	arrowY->setValue(qint16(arrow.positionY));
	arrowZ->setValue(qint16(arrow.positionZ));
	int index = arrowType->findData(arrow.type);
	if (index != -1) {
		arrowType->setCurrentIndex(index);
	} else {
		arrowType->addItem(tr("?"), arrow.type);
		arrowType->setCurrentIndex(arrowType->count()-1);
	}

	if (walkmesh)	walkmesh->setSelectedArrow(id);
}

void WalkmeshManager::editExitPoint(const Vertex_s &values)
{
	QObject *s = sender();

	if (s == exitPoints[0])			editExitPoint(0, values);
	else if (s == exitPoints[1])		editExitPoint(1, values);
}

void WalkmeshManager::editExitPoint(int id, const Vertex_s &values)
{
	if (infFile->isOpen()) {
		quint8 gateId = quint8(gateList->currentRow());
		Exit old = infFile->exitLine(gateId);
		Vertex_s oldVertex = old.exit_line[id];
		if (oldVertex.x != values.x || oldVertex.y != values.y || oldVertex.z != values.z) {
			old.exit_line[id] = values;
			infFile->setExitLine(gateId, old);
			if (walkmesh)	walkmesh->update();

			emit modified();
		}
	}
}

void WalkmeshManager::editEntryPoint(const Vertex_s &values)
{
	if (infFile->isOpen()) {
		quint8 gateId = quint8(gateList->currentRow());
		Exit old = infFile->exitLine(gateId);
		Vertex_s oldVertex = old.destination;
		if (oldVertex.x != values.x || oldVertex.y != values.y || oldVertex.z != values.z) {
			old.destination = values;
			infFile->setExitLine(gateId, old);

			emit modified();
		}
	}
}

void WalkmeshManager::editDoorPoint(const Vertex_s &values)
{
	QObject *s = sender();

	if (s == doorPosition[0])			editDoorPoint(0, values);
	else if (s == doorPosition[1])		editDoorPoint(1, values);
}

void WalkmeshManager::editDoorPoint(int id, const Vertex_s &values)
{
	if (infFile->isOpen()) {
		quint8 gateId = quint8(gateList->currentRow());
		Trigger old = infFile->trigger(gateId);
		Vertex_s oldVertex = old.trigger_line[id];
		if (oldVertex.x != values.x || oldVertex.y != values.y || oldVertex.z != values.z) {
			old.trigger_line[id] = values;
			infFile->setTrigger(gateId, old);
			if (walkmesh)	walkmesh->update();

			emit modified();
		}
	}
}

void WalkmeshManager::editExitDirection(int dir)
{
	if (infFile->isOpen()) {
		quint8 gateId = quint8(gateList->currentRow());
		Exit old = infFile->exitLine(gateId);
		old.dir_copy1 = old.dir_copy2 = old.dir_copy3 = old.dir = quint8(dir);
		if (old.dir != infFile->exitLine(gateId).dir
				|| old.dir_copy1 != infFile->exitLine(gateId).dir_copy1
				|| old.dir_copy2 != infFile->exitLine(gateId).dir_copy2
				|| old.dir_copy3 != infFile->exitLine(gateId).dir_copy3) {
			infFile->setExitLine(gateId, old);

			emit modified();
		}
	}
}

void WalkmeshManager::setGateEnabled(bool enabled)
{
	exitDirection->setEnabled(enabled);
	fieldId->setEnabled(enabled);
	exitPoints[0]->setEnabled(enabled);
	exitPoints[1]->setEnabled(enabled);
	entryPoint->setEnabled(enabled);
	arrowDisplay->setEnabled(enabled);
}

void WalkmeshManager::editGateEnabled(bool enabled)
{
	setGateEnabled(enabled);

	if (enabled) {
		editFieldId(fieldId->value());
	} else {
		editFieldId(0x7FFF);
	}
}

void WalkmeshManager::editFieldId(int v)
{
	if (infFile->isOpen()) {
		if (!gateEnabled->isChecked()) {
			v = 0x7FFF;
		}

		quint8 gateId = quint8(gateList->currentRow());
		Exit old = infFile->exitLine(gateId);
		if (old.fieldID != v) {
			old.fieldID = quint16(v);
			infFile->setExitLine(gateId, old);
			QListWidgetItem *item = gateList->currentItem();
			if (v != 0x7FFF) {
				item->setText(QString("%1 (%2)")
				                  .arg(Data::mapName(v))
				              .arg(v));
				// Default foreground
				item->setForeground(QListWidgetItem().foreground());
			} else {
				item->setText(tr("Unused"));
				item->setForeground(Qt::darkGray);
			}

			emit modified();
		}
	}
}

void WalkmeshManager::editArrowDisplay(bool checked)
{
	if (infFile->isOpen()) {
		quint8 gateId = quint8(gateList->currentRow());
		if (infFile->arrowIsDisplayed(gateId) != checked) {
			infFile->setArrowDiplay(gateId, checked);

			emit modified();
		}
	}
}

void WalkmeshManager::setDoorEnabled(bool enabled)
{
	bgParamId->setEnabled(enabled);
	bgStateId->setEnabled(enabled);
	doorPosition[0]->setEnabled(enabled);
	doorPosition[1]->setEnabled(enabled);
	doorBehavior->setEnabled(enabled);
	doorSoundId->setEnabled(enabled);
}

void WalkmeshManager::editDoorEnabled(bool enabled)
{
	setDoorEnabled(enabled);

	if (enabled) {
		editParamId(fieldId->value());
	} else {
		editParamId(0xFF);
	}
}

void WalkmeshManager::editParamId(int v)
{
	if (infFile->isOpen()) {
		if (!doorEnabled->isChecked()) {
			v = 0xFF;
		}

		quint8 gateId = quint8(doorList->currentRow());
		Trigger old = infFile->trigger(gateId);
		if (old.background_parameter != v) {
			old.background_parameter = quint8(v);
			infFile->setTrigger(gateId, old);
			QListWidgetItem *item = doorList->currentItem();
			if (v != 0xFF) {
				item->setText(tr("Door %1").arg(gateId));
				// Default foreground
				item->setForeground(QListWidgetItem().foreground());
			} else {
				item->setText(tr("Unused"));
				item->setForeground(Qt::darkGray);
			}

			if (walkmesh)	walkmesh->update();
			emit modified();
		}
	}
}

void WalkmeshManager::editStateId(int v)
{
	if (infFile->isOpen()) {
		quint8 gateId = quint8(doorList->currentRow());
		Trigger old = infFile->trigger(gateId);
		if (old.background_state != v) {
			old.background_state = quint8(v);
			infFile->setTrigger(gateId, old);

			if (walkmesh)	walkmesh->update();
			emit modified();
		}
	}
}

void WalkmeshManager::editBehavior(int v)
{
	if (infFile->isOpen()) {
		quint8 gateId = quint8(doorList->currentRow());
		Trigger old = infFile->trigger(gateId);
		if (old.behavior != v) {
			old.behavior = quint8(v);
			infFile->setTrigger(gateId, old);

			if (walkmesh)	walkmesh->update();
			emit modified();
		}
	}
}

void WalkmeshManager::editSoundId(int v)
{
	if (infFile->isOpen()) {
		quint8 gateId = quint8(doorList->currentRow());
		Trigger old = infFile->trigger(gateId);
		if (old.soundID != v) {
			old.soundID = quint8(v);
			infFile->setTrigger(gateId, old);

			if (walkmesh)	walkmesh->update();
			emit modified();
		}
	}
}

void WalkmeshManager::editArrowX(int value)
{
	if (infFile->isOpen()) {
		quint8 arrowId = quint8(arrowList->currentRow());
		Arrow old = infFile->arrow(arrowId);
		if (old.positionX != qint32(value)) {
			old.positionX = qint32(value);
			infFile->setArrow(arrowId, old);

			emit modified();
		}
	}
}

void WalkmeshManager::editArrowY(int value)
{
	if (infFile->isOpen()) {
		quint8 arrowId = quint8(arrowList->currentRow());
		Arrow old = infFile->arrow(arrowId);
		if (old.positionY != qint32(value)) {
			old.positionY = qint32(value);
			infFile->setArrow(arrowId, old);

			emit modified();
		}
	}
}

void WalkmeshManager::editArrowZ(int value)
{
	if (infFile->isOpen()) {
		quint8 arrowId = quint8(arrowList->currentRow());
		Arrow old = infFile->arrow(arrowId);
		if (old.positionZ != qint32(value)) {
			old.positionZ = qint32(value);
			infFile->setArrow(arrowId, old);

			emit modified();
		}
	}
}

void WalkmeshManager::editArrowType(int index)
{
	if (infFile->isOpen()) {
		quint8 arrowId = quint8(arrowList->currentRow());
		quint32 value = arrowType->itemData(index).toUInt();
		Arrow old = infFile->arrow(arrowId);
		if (old.type != value) {
			old.type = quint32(index);
			infFile->setArrow(arrowId, old);
			QListWidgetItem *item = arrowList->currentItem();
			if (index != 0) {
				item->setText(tr("Arrow %1").arg(arrowId));
				// Default foreground
				item->setForeground(QListWidgetItem().foreground());
			} else {
				item->setText(tr("Unused"));
				item->setForeground(Qt::darkGray);
			}

			emit modified();
		}
	}
}

void WalkmeshManager::editRange(int v)
{
	QObject *s = sender();

	if (s == rangeEdit[0])			editRange(0, v);
	else if (s == rangeEdit[1])		editRange(1, v);
	else if (s == rangeEdit[2])		editRange(2, v);
	else if (s == rangeEdit[3])		editRange(3, v);
	else if (s == bgSizeEdit[0])		editBgSize(0, v);
	else if (s == bgSizeEdit[1])		editBgSize(1, v);
	else if (s == bgSizeEdit[2])		editBgSize(2, v);
	else if (s == bgSizeEdit[3])		editBgSize(3, v);
	else if (s == bgFlagEdit[0])		editBgFlag(0, v);
	else if (s == bgFlagEdit[1])		editBgFlag(1, v);
	else if (s == bgFlagEdit[2])		editBgFlag(2, v);
	else if (s == bgFlagEdit[3])		editBgFlag(3, v);
}

void WalkmeshManager::editRange(int id, int v)
{
	if (infFile->isOpen()) {
		Range old = infFile->cameraRange();
		qint16 oldv=0;

		switch (id) {
		case 0:	oldv = old.top;		break;
		case 1:	oldv = old.bottom;	break;
		case 2:	oldv = old.right;	break;
		case 3:	oldv = old.left;	break;
		}

		if (oldv != v) {
			switch (id) {
			case 0: old.top = qint16(v);    break;
			case 1: old.bottom = qint16(v); break;
			case 2: old.right = qint16(v);  break;
			case 3: old.left = qint16(v);   break;
			}
			infFile->setCameraRange(old);

			emit modified();
		}
	}
}

void WalkmeshManager::editBgSize(int id, int v)
{
	if (infFile->isOpen()) {
		qint16 oldv=0;

		switch (id) {
		case 0:	oldv = infFile->bgLayer3Width();	break;
		case 1:	oldv = infFile->bgLayer3Height();	break;
		case 2:	oldv = infFile->bgLayer4Width();	break;
		case 3:	oldv = infFile->bgLayer4Height();	break;
		}

		if (oldv != v) {
			switch (id) {
			case 0: infFile->setBgLayer3Width(qint16(v));  break;
			case 1: infFile->setBgLayer3Height(qint16(v)); break;
			case 2: infFile->setBgLayer4Width(qint16(v));  break;
			case 3: infFile->setBgLayer4Height(qint16(v)); break;
			}

			emit modified();
		}
	}
}

void WalkmeshManager::editBgFlag(int id, int v)
{
	if (infFile->isOpen()) {
		qint16 oldv=0;

		switch (id) {
		case 0:	oldv = infFile->bgLayer1Flag();	break;
		case 1:	oldv = infFile->bgLayer2Flag();	break;
		case 2:	oldv = infFile->bgLayer3Flag();	break;
		case 3:	oldv = infFile->bgLayer4Flag();	break;
		}

		if (oldv != v) {
			switch (id) {
			case 0:	infFile->setBgLayer1Flag(quint8(v));	break;
			case 1:	infFile->setBgLayer2Flag(quint8(v));	break;
			case 2:	infFile->setBgLayer3Flag(quint8(v));	break;
			case 3:	infFile->setBgLayer4Flag(quint8(v));	break;
			}

			emit modified();
		}
	}
}

void WalkmeshManager::editNavigation(int v)
{
	if (infFile->isOpen()) {
		int old = infFile->control();
		if (old != v) {
			navigation->setValue(v);
			infFile->setControl(quint8(v));

			emit modified();
		}
	}
}

void WalkmeshManager::editCameraFocusHeight(int value)
{
	if (infFile->isOpen()) {
		int old = infFile->cameraFocusHeight();
		if (old != value) {
			infFile->setCameraFocusHeight(qint16(value));
			emit modified();
		}
	}
}

void WalkmeshManager::editUnknown(const QByteArray &data)
{
	if (infFile->isOpen()) {
		QByteArray old = infFile->unknown();
		if (old != data) {
			infFile->setUnknown(data);
			emit modified();
		}
	}
}

void WalkmeshManager::editMapScale(int scale)
{
	if (scriptsAndTexts->isOpen()) {
		int old = scriptsAndTexts->scale();
		if (old != scale) {
			scriptsAndTexts->setScale(quint16(scale));
			emit modified();
		}
	}
}

void WalkmeshManager::focusInEvent(QFocusEvent *e)
{
	if (walkmesh)	walkmesh->setFocus();
	QWidget::focusInEvent(e);
}

void WalkmeshManager::focusOutEvent(QFocusEvent *e)
{
	if (walkmesh)	walkmesh->clearFocus();
	QWidget::focusOutEvent(e);
}
