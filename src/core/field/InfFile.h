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
#ifndef INFFILE_H
#define INFFILE_H

#include <QtCore>
#include "FieldPart.h"
#include "IdFile.h"

struct Range {
	qint16 left;
	qint16 top;
	qint16 right;
	qint16 bottom;
};

struct Exit {
	Vertex_s exit_line[2];
	Vertex_s destination;
	quint16 fieldID;
	quint8 dir, dir_copy1, dir_copy2, dir_copy3;
};

struct Trigger {
	Vertex_s trigger_line[2];
	quint8 background_parameter;
	quint8 background_state;
	quint8 behavior;
	/* 0 - OnTrigger - ON
	 * 1 - OnTrigger - OFF
	 * 2 - OnTrigger - ON, AwayFromTrigger - OFF
	 * 3 - OnTrigger - OFF, AwayFromTrigger - ON
	 * 4 - OnTrigger - ON, AwayFromTriggerOnPlusSide - OFF
	 * 5 - OnTrigger - OFF, AwayFromTriggerOnPlusSide - ON
	 */
	quint8 soundID;
};

struct Arrow {
	qint32 positionX;
	qint32 positionZ;
	qint32 positionY;
	quint32 type;// 0 - Invisible, 1 - Red, 2 - Green
};

struct InfData {
	char name[9];
	quint8 control;
	qint16 cameraFocusHeight;
	Range camera_range; // 8 bytes
	quint8 bg_layer1_flag;
	quint8 bg_layer2_flag;
	quint8 bg_layer3_flag;
	quint8 bg_layer4_flag;
	qint16 bg_layer3_width;
	qint16 bg_layer3_height;
	qint16 bg_layer4_width;
	qint16 bg_layer4_height;
	quint8 unknown[24];
	Exit doors[12];// 24 * 12 bytes
	Trigger triggers[12];// 16 * 12 bytes
	// Only in occidental/international version
	quint8 display_arrow[12];
	Arrow arrows[12];// 16 * 12 bytes
};

class InfFile : public FieldPart
{
public:
	explicit InfFile(Field *field);
	void initEmpty();
	bool open();
	bool open(const QByteArray &data);
	QByteArray save() const;
	void clear();
	bool isJap() const;
	QString mapName();
	void setMapName(const QString &name);
	quint8 control() const;
	void setControl(quint8 control);
	qint16 cameraFocusHeight() const;
	void setCameraFocusHeight(qint16 cameraFocusHeight);
	const Range &cameraRange() const;
	void setCameraRange(const Range &range);
	quint8 bgLayer1Flag() const;
	void setBgLayer1Flag(quint8 flag);
	quint8 bgLayer2Flag() const;
	void setBgLayer2Flag(quint8 flag);
	quint8 bgLayer3Flag() const;
	void setBgLayer3Flag(quint8 flag);
	quint8 bgLayer4Flag() const;
	void setBgLayer4Flag(quint8 flag);
	qint16 bgLayer3Width() const;
	void setBgLayer3Width(qint16 width);
	qint16 bgLayer3Height() const;
	void setBgLayer3Height(qint16 height);
	qint16 bgLayer4Width() const;
	void setBgLayer4Width(qint16 width);
	qint16 bgLayer4Height() const;
	void setBgLayer4Height(qint16 height);
	QList<Exit> exitLines() const;
	Exit exitLine(quint8 id) const;
	void setExitLine(quint8 id, const Exit &line);
	QList<Trigger> triggers() const;
	const Trigger &trigger(quint8 id) const;
	void setTrigger(quint8 id, const Trigger &trigger);
	bool arrowIsDisplayed(quint8 id) const;
	void setArrowDiplay(quint8 id, bool display);
	QList<Arrow> arrows() const;
	const Arrow &arrow(quint8 id) const;
	void setArrow(quint8 id, const Arrow &arrow);
	QByteArray unknown() const;
	void setUnknown(const QByteArray &u);
private:
	InfData data;
};

#endif // INFFILE_H
