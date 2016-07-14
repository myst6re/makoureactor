/****************************************************************************
 ** Makou Reactor Final Fantasy VII FieldPC Script Editor
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
#ifndef DEF_FIELDPC
#define DEF_FIELDPC

#include <QtCore>
#include "Field.h"
#include "PCFieldFile.h"
#include "FieldModelLoaderPC.h"
#include "FieldModelFilePC.h"
#include "FieldArchiveIOPC.h"

class FieldPC : public Field
{
public:
	FieldPC(const QString &name, FieldArchiveIO *io);
	explicit FieldPC(const Field &field);
	virtual ~FieldPC();

	inline bool isPC() const { return true; }

	using Field::importer;
	qint8 importer(const QByteArray &data, bool isPSField, FieldSections part);

	FieldModelLoaderPC *fieldModelLoader(bool open=true);
	FieldModelFilePC *fieldModel(int modelID, int animationID = 0, bool animate = true, bool open = true);
	FieldModelFilePC *fieldModel(const QString &hrc, const QString &a, bool animate = true);
	inline FieldArchiveIOPC *io() const {
		return (FieldArchiveIOPC *)Field::io();
	}
	int sectionSize(FieldSection part) const;
	QByteArray sectionData(FieldSection part);
protected:
	virtual bool open2();
	virtual bool save() { return false; }
	virtual FieldPart *createPart(FieldSection part);
private:
	PCFieldFile *_file;
	FieldModelFilePC *_model;
	QMap<QString, int> modelNameToId;
};

#endif // DEF_FIELDPC
