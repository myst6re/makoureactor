/****************************************************************************
 ** Makou Reactor Final Fantasy VII FieldPS Script Editor
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
#ifndef DEF_FIELDPS
#define DEF_FIELDPS

#include <QtCore>
#include "Field.h"
#include "DatFile.h"
#include "FieldModelLoaderPS.h"
#include "FieldModelFilePS.h"
#include "FieldArchiveIOPS.h"

class FieldPS : public Field
{
public:
	FieldPS(const QString &name, FieldArchiveIOPS *io);
	explicit FieldPS(const Field &field);

	inline bool isPC() const { return false; }

	FieldModelLoaderPS *fieldModelLoader(bool open=true);
	FieldModelFilePS *fieldModel(int modelID, int animationID = 0, bool animate = true, bool open = true);
	inline FieldArchiveIOPS *io() const {
		return static_cast<FieldArchiveIOPS *>(Field::io());
	}
	inline DatFile *datFile() {
		return &_file;
	}
	QByteArray sectionData(CommonSection part);
	bool setSectionData(CommonSection section, const QByteArray &data);
protected:
	virtual bool open2();
	virtual bool save2(QByteArray &data, bool compress);
	virtual void saveStart();
	virtual void saveEnd();
	virtual FieldPart *createPart(FieldSection part);
private:
	DatFile _file;
};

#endif // DEF_FIELDPS
