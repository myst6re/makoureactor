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
#ifndef DEF_FIELD
#define DEF_FIELD

#include <QtCore>
#include "Section1File.h"
#include "EncounterFile.h"
#include "TutFile.h"
#include "CaFile.h"
#include "IdFile.h"
#include "InfFile.h"
#include "FieldModelLoader.h"
#include "FieldModelFile.h"
#include "BackgroundFile.h"

class FieldArchiveIO;

class Field
{
public:
	enum FieldSection {
		Scripts = 0x01,
		Akaos = 0x02,
		Camera = 0x04,
		Walkmesh = 0x08,
		ModelLoader = 0x10,
		Encounter = 0x20,
		Inf = 0x40,
		Background = 0x80
	};
	Q_DECLARE_FLAGS(FieldSections, FieldSection)

	enum CommonSection {
		_ScriptsTextsAkaos,
		_Camera,
		_Walkmesh,
		_Encounter,
		_Inf
	};

	Field(const QString &name, FieldArchiveIO *io);
	virtual ~Field();

	bool isOpen() const;
	bool isModified() const;
	void setModified(bool modified);

	virtual bool isPC() const=0;
	inline bool isPS() const { return !isPC(); }

	bool open();

	qint8 open(const QString &path, bool isDat, bool compressed, QIODevice *device2=0);
	qint8 open(const QByteArray &data, bool isPSField, QIODevice *device2=0);

	void setSaved();
	bool save(QByteArray &newData, bool compress);
	qint8 save(const QString &path, bool compress);
	qint8 importer(const QString &path, bool isDat, bool compressed, FieldSections part, QIODevice *device2=0);
	virtual qint8 importer(const QByteArray &data, bool isPSField, FieldSections part, QIODevice *device2=0);

	Section1File *scriptsAndTexts(bool open=true);
	EncounterFile *encounter(bool open=true);
	TutFileStandard *tutosAndSounds(bool open=true);
	IdFile *walkmesh(bool open=true);
	CaFile *camera(bool open=true);
	InfFile *inf(bool open=true);
	BackgroundFile *background(bool open=true);
	virtual FieldModelLoader *fieldModelLoader(bool open=true);
	virtual FieldModelFile *fieldModel(int modelID, int animationID=0, bool animate=true, bool open=true)=0;
	QMap<int, FieldModelFile *> fieldModels(bool animate=true, bool open=true);

	const QString &name() const;
	void setName(const QString &name);
	virtual FieldArchiveIO *io() const;
	virtual QByteArray sectionData(CommonSection part)=0;
	virtual bool setSectionData(CommonSection part, const QByteArray &data)=0;

	void setRemoveUnusedSection(bool remove);// FIXME: only in PC version, ugly hack detected!
protected:
	virtual bool open2()=0;
	virtual bool save2(QByteArray &data, bool compress, bool removeUnusedSection)=0;
	virtual void saveStart()=0;
	virtual void saveEnd()=0;
	FieldPart *getOrCreatePart(FieldSection part);
	virtual FieldPart *createPart(FieldSection part);
	FieldPart *part(FieldSection section) const;
	inline const QHash<FieldSection, FieldPart *> &parts() const {
		return _parts;
	}
	FieldModelFile *fieldModelPtr(int modelID) const;
	void addFieldModel(int modelID, FieldModelFile *fieldModel);
private:
	FieldPart *part(FieldSection section, bool open);

	QHash<FieldSection, FieldPart *> _parts;
	FieldArchiveIO *_io;
	bool _isOpen, _isModified;
	QString _name;
	bool _removeUnusedSection;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Field::FieldSections)

#endif
