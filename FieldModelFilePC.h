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
#ifndef FIELDMODELFILEPC_H
#define FIELDMODELFILEPC_H

#include <QtGui>
#include "FieldModelFile.h"
#include "FieldModelPartPC.h"
#include "FieldModelLoaderPC.h"

typedef struct {
	quint32 version;
	quint32 frames_count;
	quint32 bones_count;
	quint8 rotation_order[3];
	quint8 unused;
	quint32 runtime_data[5];
} a_header;

class FieldModelFilePC : public FieldModelFile
{
public:
	FieldModelFilePC();
	void clear();
	bool isPS() const { return false; }
	quint8 load(FieldModelLoaderPC *modelLoader, int modelID, int animID, bool animate=false);
	quint8 load(QString hrc, QString a, bool animate=false);
private:
	QMultiMap<int, QStringList> rsd_files;
	QList<QString> tex2id;

	bool open_hrc(QFile *);
	bool open_a(QFile *, bool animate=false);
	QString open_rsd(QFile *, int);
	static QPixmap open_tex(QFile *);
};

#endif // FIELDMODELFILEPC_H
