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
	FieldPC(const QString &name, FieldArchiveIOPC *io);
	explicit FieldPC(const Field &field);
	virtual ~FieldPC();

	inline bool isPC() const { return true; }

	using Field::importer;
	qint8 importer(const QByteArray &data, bool isPSField, FieldSections part);

	FieldModelLoaderPC *fieldModelLoader(bool open=true);
	FieldModelFilePC *fieldModel(int modelID, int animationID = 0, bool animate = true, bool open = true);
	FieldModelFilePC *fieldModel(const QString &hrc, const QString &a, bool animate = true);
	inline FieldArchiveIOPC *io() const {
		return static_cast<FieldArchiveIOPC *>(Field::io());
	}
	inline PCFieldFile *pcFieldFile() {
		return &_file;
	}
	QByteArray sectionData(CommonSection section);
	bool setSectionData(CommonSection section, const QByteArray &data);

	inline void setRemoveUnusedSection(bool remove) {
		_removeUnusedSection = remove;
	}
	inline bool removeUnusedSection() const {
		return _removeUnusedSection;
	}
protected:
	virtual bool open2();
	virtual bool save2(QByteArray &data, bool compress);
	virtual void saveStart();
	virtual void saveEnd();
	virtual FieldPart *createPart(FieldSection section);
private:
	PCFieldFile _file;
	FieldModelFilePC *_model;
	QMap<QString, int> modelNameToId;
	bool _removeUnusedSection;
};

#endif // DEF_FIELDPC
