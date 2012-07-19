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

WalkmeshManager::WalkmeshManager(WalkmeshFile *walkmeshFile, QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint)
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

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(walkmeshWidget, 0, 0);
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
}
