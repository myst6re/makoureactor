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
#ifndef WALKMESHFILE_H
#define WALKMESHFILE_H

#include <QtCore>

typedef struct {
	qint16 x, y, z;
} Vertex_s;

typedef struct {
	qint16 x, y, z, res;
} Vertex_sr;

typedef struct {
	Vertex_sr vertices[3];
} Triangle;

typedef struct {
	qint16 a[3];
} Access;

class WalkmeshFile
{
public:
	WalkmeshFile();
	explicit WalkmeshFile(const QByteArray &contenu);
	bool isOpen() const;
	bool isOpenCa() const;
	bool isOpenId() const;
	bool isModifiedCa() const;
	bool isModifiedId() const;
	void setModifiedCa(bool);
	void setModifiedId(bool);
	bool open(const QByteArray &data, bool openCa=true);
	QByteArray saveCa() const;
	QByteArray saveId() const;

	qint16 camera_axis2z;
	qint32 camera_blank;
	quint16 camera_zoom;
	Vertex_s camera_axis[3];
	qint32 camera_position[3];
	QList<Triangle> triangles;
	QList<Access> access;

	bool test() const;
private:
	bool _isOpenCa, _isOpenId, _isModifiedCa, _isModifiedId;
};

#endif // WALKMESHFILE_H
