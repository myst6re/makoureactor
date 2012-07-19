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
	VertexPS trigger_line[2];
	VertexPS destination;
	quint16 fieldID;
	quint8 u1[4];
} Exit;

typedef struct {
	quint8 u1[16];
} U1;

typedef struct {
	quint8 u1[16];
} U2;

typedef struct {
	char name[9];
	quint8 control;
	quint16 camera_range[5];
	quint32 _blank1;
	quint16 u1[4];
	quint8 _blank2[24];
	Exit doors[12];// 24 * 12 bytes
	U1 u2[12];// 16 * 12 bytes
	// Only in occidental/international version
	quint8 display_arrow[12];
	U2 u3[12];// ??? blank (16 * 12 bytes ???)
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
	const U1 &unknownData1(quint8 id) const;
	void setUnknownData1(quint8 id, const U1 &unknown);
	bool arrowIsDisplayed(quint8 id);
	void setArrowDiplay(quint8 id, bool display);
	const U2 &unknownData2(quint8 id) const;
	void setUnknownData2(quint8 id, const U2 &unknown);
	void test();
private:
	bool _isOpen, _isModified;
	InfData data;
	quint32 size;
};

#endif // INFFILE_H
