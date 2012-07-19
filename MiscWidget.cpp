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
#include "MiscWidget.h"

MiscWidget::MiscWidget(InfFile *data, Field *field, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint), data(data), dataCpy(*data), field(field)
{
	setWindowTitle(tr("Divers"));

	mapName = new QLineEdit(this);
	mapName->setMaxLength(8);
	mapAuthor = new QLineEdit(this);
	mapAuthor->setMaxLength(7);
	mapScale = new QSpinBox(this);
	mapScale->setRange(0, 65535);
	mapControl = new QSpinBox(this);
	mapControl->setRange(0, 255);

	QGroupBox *exitGroup = new QGroupBox(tr("Sorties d'écran"), this);

	exitList = new QListWidget(exitGroup);
	exitList->setFixedWidth(100);
	arrowDisplay = new QCheckBox(tr("Afficher flêche"), exitGroup);
	arrowDisplay->setIcon(QIcon(":/images/field-arrow.png"));
	fieldID = new QComboBox(exitGroup);
	fieldID->addItem(tr("Désactivé"), 0x7FFF);
	int i=0;
	foreach(const QString &fieldName, Data::field_names) {
		fieldID->addItem(fieldName, i++);
	}

	QGroupBox *triggerPointGroup = new QGroupBox(tr("Ligne de sortie"));
	QVBoxLayout *triggerPointLayout = new QVBoxLayout(triggerPointGroup);
	triggerPointLayout->addWidget(triggerPoint1 = new VertexWidget);
	triggerPointLayout->addWidget(triggerPoint2 = new VertexWidget);

	QGroupBox *destinationGroup = new QGroupBox(tr("Écran de destination"));
	QGridLayout *destinationLayout = new QGridLayout(destinationGroup);
	destinationLayout->addWidget(new QLabel(tr("Field ID")), 0, 0, Qt::AlignRight);
	destinationLayout->addWidget(fieldID, 0, 1);
	destinationLayout->addWidget(destination = new VertexWidget, 1, 0, 1, 2);

	unknown0 = new QLineEdit(exitGroup);
	unknown0->setMaxLength(32);
	unknown1 = new QLineEdit(exitGroup);
	unknown1->setMaxLength(32);
	unknown2 = new QLineEdit(exitGroup);
	unknown2->setMaxLength(8);

	QGridLayout *exitLayout = new QGridLayout(exitGroup);
	exitLayout->addWidget(exitList, 0, 0, 6, 1, Qt::AlignLeft);
	exitLayout->addWidget(destinationGroup, 0, 1, 1, 2);
	exitLayout->addWidget(triggerPointGroup, 1, 1, 1, 2);
	exitLayout->addWidget(new QLabel(tr("Inconnues 1")), 2, 1);
	exitLayout->addWidget(unknown0, 2, 2);
	exitLayout->addWidget(arrowDisplay, 3, 1, 1, 2);
	exitLayout->addWidget(new QLabel(tr("Inconnues 2")), 4, 1);
	exitLayout->addWidget(unknown1, 4, 2);
	exitLayout->addWidget(unknown2Lbl = new QLabel(tr("Inconnues 3")), 5, 1);
	exitLayout->addWidget(unknown2, 5, 2);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel(tr("Nom écran :")), 0, 0);
	layout->addWidget(mapName, 0, 1);
	layout->addWidget(new QLabel(tr("Auteur :")), 1, 0);
	layout->addWidget(mapAuthor, 1, 1);
	layout->addWidget(new QLabel(tr("Zoom écran :")), 2, 0);
	layout->addWidget(mapScale, 2, 1);
	layout->addWidget(new QLabel(tr("Directions contrôles :")), 3, 0);
	layout->addWidget(mapControl, 3, 1);
	layout->addWidget(exitGroup, 4, 0, 1, 2);
	layout->addWidget(buttonBox, 5, 0, 1, 2, Qt::AlignRight);

	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

	connect(exitList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(changeGate(QListWidgetItem*,QListWidgetItem*)));
	connect(fieldID, SIGNAL(currentIndexChanged(int)), SLOT(setEnabledGate(int)));

	fill();
}

void MiscWidget::fill()
{
	mapName->setText(dataCpy.mapName());
	mapAuthor->setText(field->getAuthor());
	mapScale->setValue(field->getScale());
	mapControl->setValue(dataCpy.control());
	arrowDisplay->setVisible(!dataCpy.isJap());
	unknown2Lbl->setVisible(!dataCpy.isJap());
	unknown2->setVisible(!dataCpy.isJap());

	exitList->clear();
	for(int i=1 ; i<=12 ; ++i) {
		exitList->addItem(tr("Sortie %1").arg(i));
	}

	exitList->setCurrentRow(0);
}

void MiscWidget::setEnabledGate(int index)
{
	setEnabledGate(fieldID->itemData(index).toInt() != 0x7FFF);
}

void MiscWidget::setEnabledGate(bool enable)
{
	triggerPoint1->setEnabled(enable);
	triggerPoint2->setEnabled(enable);
	destination->setEnabled(enable);
	unknown0->setEnabled(enable);
	unknown1->setEnabled(enable);
	if(!dataCpy.isJap()) {
		arrowDisplay->setEnabled(enable);
		unknown2->setEnabled(enable);
	}
}

void MiscWidget::changeGate(QListWidgetItem *, QListWidgetItem *previous)
{
	if(previous) {
		int previousId = exitList->row(previous);
		if(previousId != -1) {
			saveExit(previousId);
		}
	}

	int id = exitList->currentRow();

	if(id < 0)	return;

	Exit gate = dataCpy.exitLine(id);
	int index;
	if((index = fieldID->findData(gate.fieldID)) != -1) {
		fieldID->setCurrentIndex(index);
	} else {
		fieldID->addItem(QString::number(gate.fieldID), gate.fieldID);
		fieldID->setCurrentIndex(fieldID->count()-1);
	}

	setEnabledGate(gate.fieldID != 0x7FFF);

	triggerPoint1->setValues(gate.trigger_line[0]);
	triggerPoint2->setValues(gate.trigger_line[1]);
	destination->setValues(gate.destination);
	unknown0->setText(QByteArray((char *)&gate.u1, 4).toHex());
	unknown1->setText(QByteArray((char *)&dataCpy.unknownData1(id), 16).toHex());

	if(!dataCpy.isJap()) {
		arrowDisplay->setChecked(dataCpy.arrowIsDisplayed(id));
		unknown2->setText(QByteArray((char *)&dataCpy.unknownData2(id), 16).toHex());
	}
}

void MiscWidget::saveExit(int row)
{
	Exit gate;

	gate.trigger_line[0] = triggerPoint1->values();
	gate.trigger_line[1] = triggerPoint2->values();
	gate.destination = destination->values();
	memcpy(&gate.u1, QByteArray::fromHex(unknown0->text().toLatin1()).leftJustified(4, 0, true).constData(), 4);

	bool ok;
	int curFieldId = fieldID->itemData(fieldID->currentIndex()).toInt(&ok);

	if(fieldID->currentIndex() == 0) {
		gate.fieldID = 0x7FFF;
	} else if(ok) {
		gate.fieldID = curFieldId;
	} else {
		gate.fieldID = fieldID->currentIndex()-1;
	}

	U1 u1;
	memcpy(&u1, QByteArray::fromHex(unknown1->text().toLatin1()).leftJustified(16, 0, true).constData(), 16);
	dataCpy.setUnknownData1(row, u1);

	dataCpy.setExitLine(row, gate);
	if(!dataCpy.isJap()) {
		dataCpy.setArrowDiplay(row, arrowDisplay->isChecked());
		U2 u2;
		memcpy(&u2, QByteArray::fromHex(unknown2->text().toLatin1()).leftJustified(16, 0, true).constData(), 16);
		dataCpy.setUnknownData2(row, u2);
	}
}

void MiscWidget::accept()
{
	int row = exitList->currentRow();
	if(row != -1) {
		saveExit(row);
	}
	bool isJap = dataCpy.isJap();
	for(int i=0 ; i<12 ; ++i) {
		data->setExitLine(i, dataCpy.exitLine(i));
		data->setUnknownData1(i, dataCpy.unknownData1(i));
		if(!isJap) {
			data->setArrowDiplay(i, dataCpy.arrowIsDisplayed(i));
			data->setUnknownData2(i, dataCpy.unknownData2(i));
		}
	}

	data->setMapName(mapName->text());
	data->setControl(mapControl->value());

	field->setAuthor(mapAuthor->text());
	field->setScale(mapScale->value());

	QDialog::accept();
}
