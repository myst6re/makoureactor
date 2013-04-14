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

WalkmeshManager::WalkmeshManager(QWidget *parent, const QGLWidget *shareWidget) :
	QDialog(parent, Qt::Tool),
	idFile(0), caFile(0), infFile(0)
{
	setWindowTitle(tr("Zones"));

	walkmesh = Config::value("OpenGL", true).toBool() ? new WalkmeshWidget(0, shareWidget) : 0;
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

	QLabel *keyInfos = new QLabel(tr("Utilisez les touches directionnelles pour déplacer la caméra."));
	keyInfos->setTextFormat(Qt::PlainText);
	keyInfos->setWordWrap(true);

	QPushButton *resetCamera = new QPushButton(tr("Remettre à 0"));

	tabWidget = new QTabWidget(this);
	tabWidget->addTab(buildCameraPage(), tr("Caméra"));
	tabWidget->addTab(buildWalkmeshPage(), tr("Walkmesh"));
	tabWidget->addTab(buildGatewaysPage(), tr("Sorties"));
	tabWidget->addTab(buildDoorsPage(), tr("Portes"));
	tabWidget->addTab(buildArrowPage(), tr("Flêches"));
	tabWidget->addTab(buildCameraRangePage(), tr("Limites caméra"));
	tabWidget->addTab(buildMiscPage(), tr("Divers"));
	tabWidget->setFixedHeight(250);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(walkmeshWidget, 0, 0, 3, 1);
	layout->addWidget(slider1, 0, 1);
	layout->addWidget(slider2, 0, 2);
	layout->addWidget(slider3, 0, 3);
	layout->addWidget(keyInfos, 1, 1, 1, 3);
	layout->addWidget(resetCamera, 2, 1, 1, 3);
	layout->addWidget(tabWidget, 3, 0, 1, 4);

	if(walkmesh) {
		connect(slider1, SIGNAL(valueChanged(int)), walkmesh, SLOT(setXRotation(int)));
		connect(slider2, SIGNAL(valueChanged(int)), walkmesh, SLOT(setYRotation(int)));
		connect(slider3, SIGNAL(valueChanged(int)), walkmesh, SLOT(setZRotation(int)));
		connect(resetCamera, SIGNAL(clicked()), SLOT(resetCamera()));
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

	QString IDLabel = tr("ID");
	exitPoints[0] = new VertexWidget(QString(), QString(), IDLabel, ret);
	exitPoints[1] = new VertexWidget(QString(), QString(), IDLabel, ret);
	entryPoint = new VertexWidget(QString(), QString(), IDLabel, ret);

	fieldId = new QSpinBox(ret);
	fieldId->setRange(0, 65535);

	exitDirection = new QSpinBox(ret);
	exitDirection->setRange(0, 255);

	arrowDisplay = new QCheckBox(tr("Afficher une flêche"), ret);
	arrowDisplay->setIcon(QIcon(":/images/field-arrow-red.png"));

	QGridLayout *idsLayout = new QGridLayout;
	idsLayout->addWidget(new QLabel(tr("Id écran :")), 0, 0);
	idsLayout->addWidget(fieldId, 0, 1);
	idsLayout->addWidget(arrowDisplay, 0, 2, 1, 2);
	idsLayout->addWidget(new QLabel(tr("Orientation du personnage :")), 1, 0);
	idsLayout->addWidget(exitDirection, 1, 1, 1, 3);

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
	connect(arrowDisplay, SIGNAL(toggled(bool)), SLOT(editArrowDisplay(bool)));
	connect(exitDirection, SIGNAL(valueChanged(int)), SLOT(editExitDirection(int)));

	return ret;
}

QWidget *WalkmeshManager::buildDoorsPage()
{
	QWidget *ret = new QWidget(this);

	doorList = new QListWidget(ret);
	doorList->setFixedWidth(125);

	QString IDLabel = tr("ID");
	doorPosition[0] = new VertexWidget(QString(), QString(), IDLabel, ret);
	doorPosition[1] = new VertexWidget(QString(), QString(), IDLabel, ret);

	bgParamId = new QSpinBox(ret);
	bgParamId->setRange(0, 255);

	bgStateId = new QSpinBox(ret);
	bgStateId->setRange(0, 255);

	doorBehavior = new QSpinBox(ret);
	doorBehavior->setRange(0, 255);

	doorSoundId = new QSpinBox(ret);
	doorSoundId->setRange(0, 255);

	QGridLayout *idsLayout = new QGridLayout;
	idsLayout->addWidget(new QLabel(tr("Id parametre décor :")), 0, 0);
	idsLayout->addWidget(bgParamId, 0, 1);
	idsLayout->addWidget(new QLabel(tr("Id état décor :")), 1, 0);
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

QWidget *WalkmeshManager::buildArrowPage()
{
	QWidget *ret = new QWidget(this);

	arrowList = new QListWidget(ret);
	arrowList->setFixedWidth(125);

	arrowX = new QDoubleSpinBox(ret);
	arrowX->setDecimals(0);
	arrowX->setRange(-pow(2, 31), pow(2, 31)-1);

	arrowY = new QDoubleSpinBox(ret);
	arrowY->setDecimals(0);
	arrowY->setRange(-pow(2, 31), pow(2, 31)-1);

	arrowZ = new QDoubleSpinBox(ret);
	arrowZ->setDecimals(0);
	arrowZ->setRange(-pow(2, 31), pow(2, 31)-1);

	arrowType = new QComboBox(ret);
	arrowType->addItem(tr("Invisible"), 0);
	arrowType->addItem(QIcon(":/images/field-arrow-red.png"), tr("Rouge"), 1);
	arrowType->addItem(QIcon(":/images/field-arrow-green.png"), tr("Vert"), 2);

	QHBoxLayout *posLayout = new QHBoxLayout;
	posLayout->addWidget(new QLabel(tr("X :")));
	posLayout->addWidget(arrowX);
	posLayout->addWidget(new QLabel(tr("Y :")));
	posLayout->addWidget(arrowY);
	posLayout->addWidget(new QLabel(tr("Z :")));
	posLayout->addWidget(arrowZ);
	posLayout->addStretch();

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(arrowList, 0, 0, 4, 1, Qt::AlignLeft);
	layout->addWidget(new QLabel(tr("Position :")), 0, 1);
	layout->addLayout(posLayout, 1, 1);
	layout->addWidget(arrowType, 2, 1);
	layout->setRowStretch(3, 1);

	connect(arrowList, SIGNAL(currentRowChanged(int)), SLOT(setCurrentArrow(int)));
	connect(arrowX, SIGNAL(valueChanged(double)), SLOT(editArrowX(double)));
	connect(arrowY, SIGNAL(valueChanged(double)), SLOT(editArrowY(double)));
	connect(arrowZ, SIGNAL(valueChanged(double)), SLOT(editArrowZ(double)));
	connect(arrowType, SIGNAL(currentIndexChanged(int)), SLOT(editArrowType(int)));

	return ret;
}

QWidget *WalkmeshManager::buildCameraRangePage()
{
	QWidget *ret = new QWidget(this);

	QGroupBox *group1 = new QGroupBox(tr("Limites caméra"), ret);
	QGroupBox *group2 = new QGroupBox(tr("Tailles des couches (pour les animations de couche)"), ret);
	QGroupBox *group3 = new QGroupBox(tr("Flags couches"), ret);

	for(int i=0 ; i<4 ; ++i) {
		rangeEdit[i] = new QSpinBox(group1);
		rangeEdit[i]->setRange(-32768, 32767);
		bgSizeEdit[i] = new QSpinBox(group2);
		bgSizeEdit[i]->setRange(-32768, 32767);
		bgFlagEdit[i] = new QSpinBox(group3);
		bgFlagEdit[i]->setRange(0, 255);
	}

	QGridLayout *layout1 = new QGridLayout(group1);
	layout1->addWidget(new QLabel(tr("Haut")), 0, 0);
	layout1->addWidget(rangeEdit[0], 0, 1);
	layout1->addWidget(new QLabel(tr("Bas")), 0, 2);
	layout1->addWidget(rangeEdit[1], 0, 3);
	layout1->addWidget(new QLabel(tr("Droite")), 1, 0);
	layout1->addWidget(rangeEdit[2], 1, 1);
	layout1->addWidget(new QLabel(tr("Gauche")), 1, 2);
	layout1->addWidget(rangeEdit[3], 1, 3);
	layout1->setColumnStretch(0, 1);
	layout1->setColumnStretch(1, 1);
	layout1->setColumnStretch(2, 1);
	layout1->setColumnStretch(3, 1);

	QGridLayout *layout2 = new QGridLayout(group2);
	layout2->addWidget(new QLabel(tr("Largeur couche 3 décor")), 0, 0);
	layout2->addWidget(bgSizeEdit[0], 0, 1);
	layout2->addWidget(new QLabel(tr("Hauteur couche 3 décor")), 0, 2);
	layout2->addWidget(bgSizeEdit[1], 0, 3);
	layout2->addWidget(new QLabel(tr("Largeur couche 4 décor")), 1, 0);
	layout2->addWidget(bgSizeEdit[2], 1, 1);
	layout2->addWidget(new QLabel(tr("Hauteur couche 4 décor")), 1, 2);
	layout2->addWidget(bgSizeEdit[3], 1, 3);
	layout2->setRowStretch(2, 1);
	layout2->setColumnStretch(0, 1);
	layout2->setColumnStretch(1, 1);
	layout2->setColumnStretch(2, 1);
	layout2->setColumnStretch(3, 1);

	QGridLayout *layout3 = new QGridLayout(group3);
	layout3->addWidget(new QLabel(tr("Couche 1")), 0, 0);
	layout3->addWidget(bgFlagEdit[0], 0, 1);
	layout3->addWidget(new QLabel(tr("Couche 2")), 0, 2);
	layout3->addWidget(bgFlagEdit[1], 0, 3);
	layout3->addWidget(new QLabel(tr("Couche 3")), 0, 4);
	layout3->addWidget(bgFlagEdit[2], 0, 5);
	layout3->addWidget(new QLabel(tr("Couche 4")), 0, 6);
	layout3->addWidget(bgFlagEdit[3], 0, 7);
	layout3->setRowStretch(1, 1);
	layout3->setColumnStretch(0, 1);
	layout3->setColumnStretch(1, 1);
	layout3->setColumnStretch(2, 1);
	layout3->setColumnStretch(3, 1);

	QVBoxLayout *layout = new QVBoxLayout(ret);
	layout->addWidget(group1);
	layout->addWidget(group2);
	layout->addWidget(group3);
	layout->addStretch();

	for(int i=0 ; i<4 ; ++i) {
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

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(new QLabel(tr("Orientation des mouvements :")), 0, 0);
	layout->addWidget(navigation, 0, 1);
	layout->addWidget(navigation2, 0, 2);
	layout->addWidget(new QLabel(tr("Hauteur focus caméra sur le personnage :")), 1, 0);
	layout->addWidget(cameraFocusHeight, 1, 1, 1, 2);
	layout->addWidget(new QLabel(tr("Inconnu :")), 2, 0);
	layout->addWidget(unknown, 2, 1, 1, 2);
	layout->setRowStretch(3, 1);

	connect(navigation, SIGNAL(valueEdited(int)), navigation2, SLOT(setValue(int)));
	connect(navigation2, SIGNAL(valueChanged(int)), SLOT(editNavigation(int)));
	connect(cameraFocusHeight, SIGNAL(valueChanged(int)), SLOT(editCameraFocusHeight(int)));
	connect(unknown, SIGNAL(dataEdited(QByteArray)), SLOT(editUnknown(QByteArray)));

	return ret;
}

void WalkmeshManager::fill(Field *field, bool reload)
{
	if((!reload && field->inf() == infFile) || !field)	return;

	infFile = field->inf();
	idFile = field->walkmesh();
	caFile = field->camera();

	if(!idFile->isOpen() || !caFile->isOpen() || !infFile->isOpen()) {
		QMessageBox::warning(this, tr("Erreur d'ouverture"), tr("Erreur d'ouverture du walkmesh"));
	}

	int camCount = 0;

	if(walkmesh) {
		walkmesh->fill(idFile, caFile, infFile);
	}

	if(caFile->isOpen()) {
		camCount = caFile->cameraCount();

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
	tabWidget->widget(0)->setEnabled(caFile->isOpen() && camCount > 0);

	if(idFile->isOpen()) {
		int triangleCount = idFile->triangleCount();

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
	tabWidget->widget(1)->setEnabled(idFile->isOpen());

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
				doorList->addItem(tr("Porte %1").arg(doorID));
			} else {
				doorList->addItem(tr("Inutilisé"));
			}
			++doorID;
		}
		doorList->setCurrentRow(0);
		setCurrentDoor(0);

		if(!infFile->isJap()) {
			arrowList->clear();
			int arrowID = 0;
			foreach(const Arrow &arrow, infFile->arrows()) {
				if(arrow.type != 0) {
					arrowList->addItem(tr("Flêche %1").arg(arrowID));
				} else {
					arrowList->addItem(tr("Inutilisé"));
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
	tabWidget->widget(2)->setEnabled(infFile->isOpen());
	tabWidget->widget(3)->setEnabled(infFile->isOpen());
	tabWidget->widget(4)->setEnabled(infFile->isOpen() && !infFile->isJap());
	tabWidget->widget(5)->setEnabled(infFile->isOpen());
	tabWidget->widget(6)->setEnabled(infFile->isOpen());
}

void WalkmeshManager::clear()
{
	infFile = 0;
	idFile = 0;
	caFile = 0;

	if(walkmesh) {
		walkmesh->clear();
	}
}

int WalkmeshManager::currentCamera() const
{
	if(!caFile->isOpen())	return 0;

	int camID = camList->currentRow();
	return camID < 0 || camID >= caFile->cameraCount() ? 0 : camID;
}

void WalkmeshManager::setCurrentCamera(int camID)
{
	if(!caFile->isOpen() || camID < 0) {
		return;
	}

	bool hasCamera = camID < caFile->cameraCount();

	if(hasCamera) {
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

		if(walkmesh)	walkmesh->setCurrentFieldCamera(camID);
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
	int row = camList->currentRow();

	if(caFile->isOpen()) {
		Camera ca;
		if(row < caFile->cameraCount()) {
			ca = caFile->camera(row);
		} else {
			ca = Camera();
		}
		caFile->insertCamera(row+1, ca);
		camList->insertItem(row+1, tr("Camera %1").arg(row+1));
		for(int i=row+2 ; i<camList->count() ; ++i) {
			camList->item(i)->setText(tr("Camera %1").arg(i));
		}
		camList->setCurrentRow(row+1);

		emit modified();
	}
}

void WalkmeshManager::removeCamera()
{
	if(caFile->cameraCount() < 2) return;

	int row = camList->currentRow();

	if(row < 0)		return;

	if(caFile->isOpen() && row < caFile->cameraCount()) {
		if(caFile->removeCamera(row)) {
			delete camList->item(row);
			for(int i=row ; i<camList->count() ; ++i) {
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

	if(s == caVectorXEdit)			editCaVector(0, values);
	else if(s == caVectorYEdit)		editCaVector(1, values);
	else if(s == caVectorZEdit)		editCaVector(2, values);
}

void WalkmeshManager::editCaVector(int id, const Vertex_s &values)
{
	if(caFile->isOpen() && caFile->hasCamera()) {
		const int camID = currentCamera();
		Camera cam = caFile->camera(camID);
		Vertex_s oldV = cam.camera_axis[id];

		if(oldV.x != values.x || oldV.y != values.y || oldV.z != values.z) {
			cam.camera_axis[id] = values;
			caFile->setCamera(camID, cam);
			if(walkmesh)	walkmesh->updateGL();

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
	if(caFile->isOpen() && caFile->hasCamera()) {
		const int camID = currentCamera();
		Camera cam = caFile->camera(camID);
		if(cam.camera_position[id] != (qint32)value) {
			cam.camera_position[id] = value;
			caFile->setCamera(camID, cam);
			walkmesh->updateGL();

			emit modified();
		}
	}
}

void WalkmeshManager::editCaZoom(int value)
{
	if(caFile->isOpen() && caFile->hasCamera()) {
		const int camID = currentCamera();
		Camera cam = caFile->camera(camID);
		if(cam.camera_zoom != value) {
			cam.camera_zoom = value;
			caFile->setCamera(camID, cam);
			if(walkmesh)	walkmesh->updatePerspective();

			emit modified();
		}
	}
}

void WalkmeshManager::setCurrentId(int i)
{
	if(!idFile->isOpen() || i < 0)	return;

	if(idFile->triangleCount() <= i)	return;

	const Triangle &triangle = idFile->triangle(i);
	const Access &access = idFile->access(i);

	idVertices[0]->setValues(IdFile::toVertex_s(triangle.vertices[0]));
	idVertices[1]->setValues(IdFile::toVertex_s(triangle.vertices[1]));
	idVertices[2]->setValues(IdFile::toVertex_s(triangle.vertices[2]));

	idAccess[0]->setValue(access.a[0]);
	idAccess[1]->setValue(access.a[1]);
	idAccess[2]->setValue(access.a[2]);

	if(walkmesh)	walkmesh->setSelectedTriangle(i);
}

void WalkmeshManager::addTriangle()
{
	int row = idList->currentRow();

	if(idFile->isOpen()) {
		Triangle tri;
		Access acc;
		if(row < idFile->triangleCount()) {
			tri = idFile->triangle(row);
			acc = idFile->access(row);
		} else {
			tri = Triangle();
			acc = Access();
		}
		idFile->insertTriangle(row+1, tri, acc);
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

	if(idFile->isOpen() && row < idFile->triangleCount()) {
		idFile->removeTriangle(row);
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
	if(idFile->isOpen()) {
		const int triangleID = idList->currentRow();
		if(triangleID > -1 && triangleID < idFile->triangleCount()) {
			Triangle old = idFile->triangle(triangleID);
			Vertex_sr &oldV = old.vertices[id];
			if(oldV.x != values.x || oldV.y != values.y || oldV.z != values.z) {
				oldV = IdFile::fromVertex_s(values);
				idFile->setTriangle(triangleID, old);
				if(walkmesh)	walkmesh->updateGL();

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
	if(idFile->isOpen()) {
		const int triangleID = idList->currentRow();
		if(triangleID > -1 && triangleID < idFile->triangleCount()) {
			Access old = idFile->access(triangleID);
			qint16 oldV = old.a[id];
			if(oldV != value) {
				old.a[id] = value;
				idFile->setAccess(triangleID, old);
				if(walkmesh)	walkmesh->updateGL();

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
	if(!infFile->isJap()) {
		arrowDisplay->setChecked(infFile->arrowIsDisplayed(id));
	}

	exitDirection->setValue(gateway.dir);

	if(walkmesh)	walkmesh->setSelectedGate(id);
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

	if(walkmesh)	walkmesh->setSelectedDoor(id);
}

void WalkmeshManager::setCurrentArrow(int id)
{
	if(!infFile->isOpen() || id < 0 || 12 <= id)    return;

	const Arrow &arrow = infFile->arrow(id);

	arrowX->setValue(arrow.positionX);
	arrowY->setValue(arrow.positionY);
	arrowZ->setValue(arrow.positionZ);
	int index = arrowType->findData(arrow.type);
	if(index != -1) {
		arrowType->setCurrentIndex(index);
	} else {
		arrowType->addItem(tr("?"), arrow.type);
		arrowType->setCurrentIndex(arrowType->count()-1);
	}

	if(walkmesh)	walkmesh->setSelectedArrow(id);
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
			if(walkmesh)	walkmesh->updateGL();

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
			if(walkmesh)	walkmesh->updateGL();

			emit modified();
		}
	}
}

void WalkmeshManager::editExitDirection(int dir)
{
	if(infFile->isOpen()) {
		int gateId = gateList->currentRow();
		Exit old = infFile->exitLine(gateId);
		old.dir_copy1 = old.dir_copy2 = old.dir_copy3 = old.dir = dir;
		if(old.dir != infFile->exitLine(gateId).dir
				|| old.dir_copy1 != infFile->exitLine(gateId).dir_copy1
				|| old.dir_copy2 != infFile->exitLine(gateId).dir_copy2
				|| old.dir_copy3 != infFile->exitLine(gateId).dir_copy3) {
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

void WalkmeshManager::editArrowDisplay(bool checked)
{
	if(infFile->isOpen()) {
		int gateId = gateList->currentRow();
		if(infFile->arrowIsDisplayed(gateId) != checked) {
			infFile->setArrowDiplay(gateId, checked);

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

			if(walkmesh)	walkmesh->updateGL();
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

			if(walkmesh)	walkmesh->updateGL();
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

			if(walkmesh)	walkmesh->updateGL();
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

			if(walkmesh)	walkmesh->updateGL();
			emit modified();
		}
	}
}

void WalkmeshManager::editArrowX(double value)
{
	if(infFile->isOpen()) {
		int arrowId = arrowList->currentRow();
		Arrow old = infFile->arrow(arrowId);
		if(old.positionX != value) {
			old.positionX = value;
			infFile->setArrow(arrowId, old);

			emit modified();
		}
	}
}

void WalkmeshManager::editArrowY(double value)
{
	if(infFile->isOpen()) {
		int arrowId = arrowList->currentRow();
		Arrow old = infFile->arrow(arrowId);
		if(old.positionY != value) {
			old.positionY = value;
			infFile->setArrow(arrowId, old);

			emit modified();
		}
	}
}

void WalkmeshManager::editArrowZ(double value)
{
	if(infFile->isOpen()) {
		int arrowId = arrowList->currentRow();
		Arrow old = infFile->arrow(arrowId);
		if(old.positionZ != value) {
			old.positionZ = value;
			infFile->setArrow(arrowId, old);

			emit modified();
		}
	}
}

void WalkmeshManager::editArrowType(int index)
{
	if(infFile->isOpen()) {
		int arrowId = arrowList->currentRow();
		quint32 value = arrowType->itemData(index).toUInt();
		Arrow old = infFile->arrow(arrowId);
		if(old.type != value) {
			old.type = index;
			infFile->setArrow(arrowId, old);
			if(index != 0) {
				arrowList->currentItem()->setText(tr("Flêche %1").arg(arrowId));
			} else {
				arrowList->currentItem()->setText(tr("Inutilisé"));
			}

			emit modified();
		}
	}
}

void WalkmeshManager::editRange(int v)
{
	QObject *s = sender();

	if(s == rangeEdit[0])			editRange(0, v);
	else if(s == rangeEdit[1])		editRange(1, v);
	else if(s == rangeEdit[2])		editRange(2, v);
	else if(s == rangeEdit[3])		editRange(3, v);
	else if(s == bgSizeEdit[0])		editBgSize(0, v);
	else if(s == bgSizeEdit[1])		editBgSize(1, v);
	else if(s == bgSizeEdit[2])		editBgSize(2, v);
	else if(s == bgSizeEdit[3])		editBgSize(3, v);
	else if(s == bgFlagEdit[0])		editBgFlag(0, v);
	else if(s == bgFlagEdit[1])		editBgFlag(1, v);
	else if(s == bgFlagEdit[2])		editBgFlag(2, v);
	else if(s == bgFlagEdit[3])		editBgFlag(3, v);
}

void WalkmeshManager::editRange(int id, int v)
{
	if(infFile->isOpen()) {
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

void WalkmeshManager::editBgSize(int id, int v)
{
	if(infFile->isOpen()) {
		qint16 oldv=0;

		switch(id) {
		case 0:	oldv = infFile->bgLayer3Width();	break;
		case 1:	oldv = infFile->bgLayer3Height();	break;
		case 2:	oldv = infFile->bgLayer4Width();	break;
		case 3:	oldv = infFile->bgLayer4Height();	break;
		}

		if(oldv != v) {
			switch(id) {
			case 0:	infFile->setBgLayer3Width(v);	break;
			case 1:	infFile->setBgLayer3Height(v);	break;
			case 2:	infFile->setBgLayer4Width(v);	break;
			case 3:	infFile->setBgLayer4Height(v);	break;
			}

			emit modified();
		}
	}
}

void WalkmeshManager::editBgFlag(int id, int v)
{
	if(infFile->isOpen()) {
		qint16 oldv=0;

		switch(id) {
		case 0:	oldv = infFile->bgLayer1Flag();	break;
		case 1:	oldv = infFile->bgLayer2Flag();	break;
		case 2:	oldv = infFile->bgLayer3Flag();	break;
		case 3:	oldv = infFile->bgLayer4Flag();	break;
		}

		if(oldv != v) {
			switch(id) {
			case 0:	infFile->setBgLayer1Flag(v);	break;
			case 1:	infFile->setBgLayer2Flag(v);	break;
			case 2:	infFile->setBgLayer3Flag(v);	break;
			case 3:	infFile->setBgLayer4Flag(v);	break;
			}

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

void WalkmeshManager::editCameraFocusHeight(int value)
{
	if(infFile->isOpen()) {
		int old = infFile->cameraFocusHeight();
		if(old != value) {
			infFile->setCameraFocusHeight(value);
			emit modified();
		}
	}
}

void WalkmeshManager::editUnknown(const QByteArray &data)
{
	if(infFile->isOpen()) {
		QByteArray old = infFile->unknown();
		if(old != data) {
			infFile->setUnknown(data);
			emit modified();
		}
	}
}

void WalkmeshManager::focusInEvent(QFocusEvent *e)
{
	if(walkmesh)	walkmesh->setFocus();
	QWidget::focusInEvent(e);
}

void WalkmeshManager::focusOutEvent(QFocusEvent *e)
{
	if(walkmesh)	walkmesh->clearFocus();
	QWidget::focusOutEvent(e);
}
