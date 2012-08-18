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
#include "IdFile.h"

typedef struct {
	qint16 left;
	qint16 top;
	qint16 right;
	qint16 bottom;
} Range;

typedef struct {
	Vertex_s exit_line[2];
	Vertex_s destination;
	quint16 fieldID;
	quint32 u1;
} Exit;

typedef struct {
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
	const Range &cameraRange() const;
	void setCameraRange(const Range &range);
	const Range &screenRange() const;
	void setScreenRange(const Range &range);
	QList<Exit> exitLines() const;
	Exit exitLine(quint8 id);
	void setExitLine(quint8 id, const Exit &line);
	QList<Trigger> triggers() const;
	const Trigger &trigger(quint8 id) const;
	void setTrigger(quint8 id, const Trigger &trigger);
	bool arrowIsDisplayed(quint8 id);
	void setArrowDiplay(quint8 id, bool display);
	QList<Arrow> arrows() const;
	const Arrow &arrow(quint8 id) const;
	void setArrow(quint8 id, const Arrow &arrow);
	QByteArray unknown0() const;
	void setUnknown0(const QByteArray &u);
	QByteArray unknown1() const;
	void setUnknown1(const QByteArray &u);
	QByteArray unknown2() const;
	void setUnknown2(const QByteArray &u);
	void test();
private:
	bool _isOpen, _isModified;
	InfData data;
	quint32 size;
};

#endif // INFFILE_H
