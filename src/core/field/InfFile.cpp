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
#include "InfFile.h"
#include "Field.h"

InfFile::InfFile(Field *field) :
	FieldPart(field)
{
}

void InfFile::initEmpty()
{
	// Fill with zeroes
	data = InfData();

	memcpy(data.name, field()->name().toLatin1().left(8), 8);
	data.control = 128; // Left on the left
	data.camera_range.left = -256;
	data.camera_range.right = 256;
	data.camera_range.top = -256;
	data.camera_range.bottom = 256;
	data.bg_layer3_width = 1024;
	data.bg_layer3_height = 1024;
	data.bg_layer4_width = 1024;
	data.bg_layer4_height = 1024;
	for (int i = 0; i < 12; i++) {
		data.doors[i].fieldID = 0x7FFF;
		data.triggers[i].background_parameter = 0xFF;
	}
}

bool InfFile::open()
{
	return open(field()->sectionData(Field::Inf));
}

bool InfFile::open(const QByteArray &data)
{
	qsizetype size = data.size();

	if (sizeof(InfData) != 740) {
		qWarning() << "Error InfData" << sizeof(InfData) << "must be 740";
		Q_ASSERT(false);
	}

	if (size != 740 && size != 536) {
		qWarning() << "Error inf size" << size;
//		QFile debinf("debugInf.inf");
//		debinf.open(QIODevice::WriteOnly);
//		debinf.write(data);
//		debinf.close();
		return false;
	}

	this->data = InfData();
	memcpy(&this->data, data.constData(), size_t(size));
	this->data.name[8] = '\x00';

	setOpen(true);

	return true;
}

QByteArray InfFile::save() const
{
	qsizetype size = field()->sectionSize(Field::Inf);
	if (size != 740 && size != 536) {
		size = 740;
	}
	return QByteArray((char *)&data, size);
}

void InfFile::clear()
{
}

bool InfFile::isJap() const
{
	return field()->sectionSize(Field::Inf) == 536;
}

QString InfFile::mapName()
{
	return QString::fromLatin1(data.name, qstrnlen(data.name, 9));
}

void InfFile::setMapName(const QString &name)
{
	memcpy(data.name, name.toLatin1().leftJustified(9, '\0', true).constData(), 9);
	data.name[8] = '\0';
	qDebug() << "InfFile::setMapName" << name;
	setModified(true);
}

quint8 InfFile::control() const
{
	return data.control;
}

void InfFile::setControl(quint8 control)
{
	data.control = control;
	qDebug() << "InfFile::setControl" << control;
	setModified(true);
}

qint16 InfFile::cameraFocusHeight() const
{
	return data.cameraFocusHeight;
}

void InfFile::setCameraFocusHeight(qint16 cameraFocusHeight)
{
	data.cameraFocusHeight = cameraFocusHeight;
	qDebug() << "InfFile::setCameraFocusHeight" << cameraFocusHeight;
	setModified(true);
}

const Range &InfFile::cameraRange() const
{
	return data.camera_range;
}

void InfFile::setCameraRange(const Range &range)
{
	data.camera_range = range;
	qDebug() << "InfFile::setCameraRange";
	setModified(true);
}

quint8 InfFile::bgLayer1Flag() const
{
	return data.bg_layer1_flag;
}

void InfFile::setBgLayer1Flag(quint8 flag)
{
	data.bg_layer1_flag = flag;
	qDebug() << "InfFile::setBgLayer1Flag" << flag;
	setModified(true);
}

quint8 InfFile::bgLayer2Flag() const
{
	return data.bg_layer2_flag;
}

void InfFile::setBgLayer2Flag(quint8 flag)
{
	data.bg_layer2_flag = flag;
	qDebug() << "InfFile::setBgLayer2Flag" << flag;
	setModified(true);
}

quint8 InfFile::bgLayer3Flag() const
{
	return data.bg_layer3_flag;
}

void InfFile::setBgLayer3Flag(quint8 flag)
{
	data.bg_layer3_flag = flag;
	qDebug() << "InfFile::setBgLayer3Flag" << flag;
	setModified(true);
}

quint8 InfFile::bgLayer4Flag() const
{
	return data.bg_layer4_flag;
}

void InfFile::setBgLayer4Flag(quint8 flag)
{
	data.bg_layer4_flag = flag;
	qDebug() << "InfFile::setBgLayer4Flag" << flag;
	setModified(true);
}

qint16 InfFile::bgLayer3Width() const
{
	return data.bg_layer3_width;
}

void InfFile::setBgLayer3Width(qint16 width)
{
	data.bg_layer3_width = width;
	setModified(true);
}

qint16 InfFile::bgLayer3Height() const
{
	return data.bg_layer3_height;
}

void InfFile::setBgLayer3Height(qint16 height)
{
	data.bg_layer3_height = height;
	setModified(true);
}

qint16 InfFile::bgLayer4Width() const
{
	return data.bg_layer4_width;
}

void InfFile::setBgLayer4Width(qint16 width)
{
	data.bg_layer4_width = width;
	setModified(true);
}

qint16 InfFile::bgLayer4Height() const
{
	return data.bg_layer4_height;
}

void InfFile::setBgLayer4Height(qint16 height)
{
	data.bg_layer4_height = height;
	setModified(true);
}

qint16 InfFile::bgLayer3XRelated() const
{
	return data.bg_layer3_x_related;
}

void InfFile::setBgLayer3XRelated(qint16 x)
{
	data.bg_layer3_x_related = x;
	setModified(true);
}

qint16 InfFile::bgLayer3YRelated() const
{
	return data.bg_layer3_y_related;
}

void InfFile::setBgLayer3YRelated(qint16 y)
{
	data.bg_layer3_y_related = y;
	setModified(true);
}

qint16 InfFile::bgLayer4XRelated() const
{
	return data.bg_layer4_x_related;
}

void InfFile::setBgLayer4XRelated(qint16 x)
{
	data.bg_layer4_x_related = x;
	setModified(true);
}

qint16 InfFile::bgLayer4YRelated() const
{
	return data.bg_layer4_y_related;
}

void InfFile::setBgLayer4YRelated(qint16 y)
{
	data.bg_layer4_y_related = y;
	setModified(true);
}

qint16 InfFile::bgLayer3XMultiplierRelated() const
{
	return data.bg_layer3_x_multiplier_related;
}

void InfFile::setBgLayer3XMultiplierRelated(qint16 x)
{
	data.bg_layer3_x_multiplier_related = x;
	setModified(true);
}

qint16 InfFile::bgLayer3YMultiplierRelated() const
{
	return data.bg_layer3_y_multiplier_related;
}

void InfFile::setBgLayer3YMultiplierRelated(qint16 y)
{
	data.bg_layer3_y_multiplier_related = y;
	setModified(true);
}

qint16 InfFile::bgLayer4XMultiplierRelated() const
{
	return data.bg_layer4_x_multiplier_related;
}

void InfFile::setBgLayer4XMultiplierRelated(qint16 x)
{
	data.bg_layer4_x_multiplier_related = x;
	setModified(true);
}

qint16 InfFile::bgLayer4YMultiplierRelated() const
{
	return data.bg_layer4_y_multiplier_related;
}

void InfFile::setBgLayer4YMultiplierRelated(qint16 y)
{
	data.bg_layer4_y_multiplier_related = y;
	setModified(true);
}

QList<Exit> InfFile::exitLines() const
{
	QList<Exit> exit;
	for (int i=0; i<12; ++i) {
		exit.append(data.doors[i]);
	}
	return exit;
}

Exit InfFile::exitLine(quint8 id) const
{
	return data.doors[id];
}

void InfFile::setExitLine(quint8 id, const Exit &line)
{
	data.doors[id] = line;
	qDebug() << "InfFile::setExitLine" << id;
	setModified(true);
}

QList<Trigger> InfFile::triggers() const
{
	QList<Trigger> trigger;
	for (int i=0; i<12; ++i) {
		trigger.append(data.triggers[i]);
	}
	return trigger;
}

const Trigger &InfFile::trigger(quint8 id) const
{
	return data.triggers[id];
}

void InfFile::setTrigger(quint8 id, const Trigger &trigger)
{
	data.triggers[id] = trigger;
	qDebug() << "InfFile::setTrigger" << id;
	setModified(true);
}

bool InfFile::arrowIsDisplayed(quint8 id) const
{
	return data.display_arrow[id] & 1;
}

void InfFile::setArrowDiplay(quint8 id, bool display)
{
	data.display_arrow[id] = (data.display_arrow[id] & 0xFE) | quint8(display);
	qDebug() << "InfFile::setArrowDiplay" << id << display;
	setModified(true);
}

QList<Arrow> InfFile::arrows() const
{
	QList<Arrow> arrowList;
	for (int i=0; i<12; ++i) {
		arrowList.append(data.arrows[i]);
	}
	return arrowList;
}

const Arrow &InfFile::arrow(quint8 id) const
{
	return data.arrows[id];
}

void InfFile::setArrow(quint8 id, const Arrow &arrow)
{
	data.arrows[id] = arrow;
	qDebug() << "InfFile::setArrow" << id;
	setModified(true);
}
