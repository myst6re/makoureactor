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
#include "FieldModelPart.h"

typedef struct {
	quint16 left;
	quint16 bottom;
	quint16 right;
	quint16 top; // maybe bottom
} Range;

typedef struct {
	VertexPS exit_line[2];
	VertexPS destination;
	quint16 fieldID;
	quint32 u1;
} Exit;

typedef struct {
	VertexPS trigger_line[2];
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
} Trigger;

typedef struct {
	qint32 positionX;
	qint32 positionZ;
	qint32 positionY;
	quint32 type;
} Arrow;

typedef struct {
	char name[9];
	quint8 control;
	quint16 u0;
	Range camera_range; // 8 bytes
	quint32 u1;
	Range unknown_range; // 8 bytes
	quint8 u2[24];
	Exit doors[12];// 24 * 12 bytes
	Trigger triggers[12];// 16 * 12 bytes
	// Only in occidental/international version
	quint8 display_arrow[12];
	Arrow arrows[12];// 16 * 12 bytes
} InfData;

class InfFile
{
public:
	InfFile();
	InfFile(const QByteArray &contenu);
	bool isOpen();
	bool isModified();
	void setModified(bool);
	bool open(const QByteArray &contenu, bool importMode=false);
	QByteArray save();
	bool isJap();
	QString mapName();
	void setMapName(const QString &name);
	quint8 control();
	void setControl(quint8 control);
	Exit exitLine(quint8 id);
	void setExitLine(quint8 id, const Exit &line);
	const Trigger &trigger(quint8 id) const;
	void setTrigger(quint8 id, const Trigger &trigger);
	bool arrowIsDisplayed(quint8 id);
	void setArrowDiplay(quint8 id, bool display);
	const Arrow &arrow(quint8 id) const;
	void setArrow(quint8 id, const Arrow &arrow);
	void test();
private:
	bool _isOpen, _isModified;
	InfData data;
	quint32 size;
};

#endif // INFFILE_H
