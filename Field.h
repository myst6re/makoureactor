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

#include <QtGui>
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
	enum FieldPart {
		Scripts = 0x01,
		Akaos = 0x02,
		Camera = 0x04,
		Walkmesh = 0x08,
		ModelLoader = 0x10,
		Encounter = 0x20,
		Inf = 0x40,
		Background = 0x80,
		PalettePC = 0x100
	};
	Q_DECLARE_FLAGS(FieldParts, FieldPart)

	Field(const QString &name);
	Field(const QString &name, FieldArchiveIO *fieldArchive);
	virtual ~Field();

	bool isOpen() const;
	bool isModified() const;
	void setModified(bool modified);

	virtual bool isPC() const=0;
	inline bool isPS() const { return !isPC(); }

	bool open(bool dontOptimize=false);

	QPixmap openBackground();
	virtual QPixmap openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL)=0;

	bool usedParams(QHash<quint8, quint8> &usedParams, bool *layerExists);

	void setSaved();
	virtual bool save(QByteArray &newData, bool compress)=0;
	qint8 save(const QString &path, bool compress);
	qint8 importer(const QString &path, int type, FieldParts part);
	virtual qint8 importer(const QByteArray &data, bool isPSField, FieldParts part);

	Section1File *scriptsAndTexts(bool open=true);
	EncounterFile *encounter(bool open=true);
	TutFile *tutosAndSounds(bool open=true);
	IdFile *walkmesh(bool open=true);
	CaFile *camera(bool open=true);
	InfFile *inf(bool open=true);
	virtual FieldModelLoader *fieldModelLoader(bool open=true);
	virtual FieldModelFile *fieldModel(int modelID, int animationID=0, bool animate=true)=0;

	const QString &name() const;
	void setName(const QString &name);
protected:
	virtual int headerSize()=0;
	virtual void openHeader(const QByteArray &fileData)=0;
	virtual FieldModelLoader *createFieldModelLoader() const=0;
	virtual BackgroundFile *createBackground() const=0;
	BackgroundFile *background();
	QByteArray sectionData(FieldPart part);
	virtual int sectionId(FieldPart part) const=0;
	virtual quint32 sectionPosition(int idPart)=0;
	virtual int sectionCount()=0;
	virtual int paddingBetweenSections()=0;

	Section1File *section1;
	EncounterFile *_encounter;
	TutFile *_tut;
	IdFile *id;
	CaFile *ca;
	InfFile *_inf;
	FieldModelLoader *modelLoader;
	FieldModelFile *_fieldModel;
	BackgroundFile *_bg;
	FieldArchiveIO *fieldArchive;
private:
	bool _isOpen, _isModified;

	QString _name;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Field::FieldParts)

#endif
