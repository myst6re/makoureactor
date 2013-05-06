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
	inline bool translateAfter() const { return true; }
	quint8 load(const QString &hrc, const QString &a, bool animate=true);
private:
	QMultiMap<int, QList<int> > _tex_files;
	QList<QString> tex2id;

	bool openHrc(QIODevice *hrc_file, QMultiMap<int, QStringList> &rsd_files);
	bool openA(QIODevice *a_file, bool animate=false);
	QString openRsd(QIODevice *, int);
	static QImage openTex(QIODevice *);
};

#endif // FIELDMODELFILEPC_H
