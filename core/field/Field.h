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
		Background = 0x80,
		PalettePC = 0x100,
		Unused = 0x200
	};
	Q_DECLARE_FLAGS(FieldSections, FieldSection)

	Field(const QString &name, FieldArchiveIO *io);
	explicit Field(const QString &name);
	virtual ~Field();

	inline bool isOpen() const {
		return _isOpen;
	}
	inline bool isModified() const {
		return _isModified;
	}
	void setModified(bool modified);

	virtual bool isPC() const=0;
	inline bool isPS() const { return !isPC(); }

	bool open(bool dontOptimize=false);

	qint8 open(const QString &path, bool isDat, bool compressed, QIODevice *device2=0);
	qint8 open(const QByteArray &data, bool isPSField, QIODevice *device2=0);

	void initEmpty();

	void setSaved();
	bool save(QByteArray &newData, bool compress);
	qint8 save(const QString &path, bool compress);
	qint8 importer(const QString &path, bool isDat, bool compressed, FieldSections part, QIODevice *bsxDevice = nullptr,
	               QIODevice *mimDevice = nullptr);
	qint8 importer(const QByteArray &data, bool isPSField, FieldSections part, QIODevice *bsxDevice = nullptr,
	               QIODevice *mimDevice = nullptr);

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

	inline const QString &name() const {
		return _name;
	}
	void setName(const QString &name);
	inline virtual FieldArchiveIO *io() const {
		return _io;
	}
	int sectionSize(FieldSection part) const;
	QByteArray sectionData(FieldSection part, bool dontOptimize=false);

	inline void setRemoveUnusedSection(bool remove) { // FIXME: only in PC version, ugly hack detected!
		_removeUnusedSection = remove;
	}
protected:
	virtual int headerSize() const=0;
	virtual void openHeader(const QByteArray &fileData)=0;
	virtual QByteArray saveHeader() const=0;
	virtual QByteArray saveFooter() const=0;
	virtual FieldPart *createPart(FieldSection part);
	FieldPart *part(FieldSection section) const;
	virtual int sectionId(FieldSection part) const=0;
	virtual QString sectionFile(FieldSection part) const=0;
	virtual quint32 sectionPosition(int idPart) const=0;
	virtual int sectionCount() const=0;
	virtual int paddingBetweenSections() const=0;
	virtual int alignment() const=0;
	virtual QList<Field::FieldSection> orderOfSections() const=0;
	virtual quint32 diffSectionPos() const=0;
	virtual bool hasSectionHeader() const=0;
	virtual bool importModelLoader(const QByteArray &sectionData, bool isPSField, QIODevice *bsxDevice);
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
