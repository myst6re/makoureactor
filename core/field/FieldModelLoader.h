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
#ifndef FIELDMODELLOADER_H
#define FIELDMODELLOADER_H

#include <QtCore>
#include <QRgb>
#include "FieldPart.h"

struct FieldModelColorDir
{
	FieldModelColorDir() :
	    dirA(0), dirB(0), dirC(0), color(Qt::black) {}
	FieldModelColorDir(qint16 dirA, qint16 dirB, qint16 dirC, QRgb color) :
	    dirA(dirA), dirB(dirB), dirC(dirC), color(color) {}

	qint16 dirA, dirB, dirC;
	QRgb color;

	inline bool operator ==(const FieldModelColorDir &other) const {
		return dirA == other.dirA &&
		        dirB == other.dirB &&
		        dirC == other.dirC &&
		        color == other.color;
	}
	inline bool operator !=(const FieldModelColorDir &other) const {
		return !(*this == other);
	}
};

class FieldModelLoader : public FieldPart
{
public:
	explicit FieldModelLoader(Field *field);
	virtual ~FieldModelLoader();
	virtual int modelCount() const=0;
	virtual int animCount(int modelID) const=0;
	virtual quint16 unknown(int modelID) const=0;
	virtual void setUnknown(int modelID, quint16 unknown)=0;
	/* Even the file format can store more than 256 models,
	 * it's useless since the scripts can't deal with it. */
	inline static int maxModelCount() { return 256; }
	inline static int maxAnimCount() { return 255; } // PS version can't store more than that
};

#endif // FIELDMODELLOADER_H
