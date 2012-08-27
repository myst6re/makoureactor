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
#include "GrpScript.h"
#include "FF7Text.h"
#include "Palette.h"
#include "LZS.h"
#include "EncounterFile.h"
#include "TutFile.h"
#include "CaFile.h"
#include "IdFile.h"
#include "InfFile.h"
#include "FieldModelLoader.h"

//Sizeof : 36
typedef struct {
	qint16 cibleX, cibleY;
	quint32 unused1;
	quint8 srcX, unused2;
	quint8 srcY, unused3;
	quint8 srcX2, unused4;
	quint8 srcY2, unused5;
	quint32 unused6;
	quint8 paletteID, unused7;
	quint16 ID;
	quint8 param;
	quint8 state;
	quint8 blending;
	quint8 unused8;
	quint8 typeTrans, size;//Normaly unused
	quint8 textureID, unused10;
	quint8 textureID2, unused11;
	quint8 deph, unused12;
} Tile;

class Field
{
public:
	enum FieldPart {
		Scripts = 0x01,
		Akaos = 0x02,
		Camera = 0x04,
		Walkmesh = 0x08,
		HrcLoader = 0x10,
		Encounter = 0x20,
		Inf = 0x40,
		Background = 0x80
	};
	Q_DECLARE_FLAGS(FieldParts, FieldPart)

	Field();
	Field(const QString &name);
	virtual ~Field();

	bool isOpen() const;
	bool isModified() const;
	void setModified(bool modified);

	virtual qint8 open(const QByteArray &fileData)=0;
	void close();

	int getModelID(quint8) const;
	void getBgParamAndBgMove(QHash<quint8, quint8> &paramActifs, qint16 *z=0, qint16 *x=0, qint16 *y=0) const;
	virtual bool getUsedParams(const QByteArray &contenu, QHash<quint8, quint8> &usedParams, bool *layerExists) const=0;

	const QList<GrpScript *> &grpScripts() const;
	GrpScript *grpScript(int groupID) const;
	int grpScriptCount() const;
	void insertGrpScript(int row);
	void insertGrpScript(int row, GrpScript *grpScript);
	void deleteGrpScript(int row);
	void removeGrpScript(int row);
	bool moveGrpScript(int row, bool direction);

	QList<FF7Var> searchAllVars() const;
	bool searchOpcode(int opcode, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchVar(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchExec(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchMapJump(quint16 field, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchText(const QRegExp &text, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchOpcodeP(int opcode, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchVarP(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchExecP(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchMapJumpP(quint16 mapJump, int &groupID, int &scriptID, int &opcodeID) const;
	bool searchTextP(const QRegExp &text, int &groupID, int &scriptID, int &opcodeID) const;

	void shiftTutIds(int row, int shift);

	QList<FF7Text *> *getTexts();
	int getNbTexts() const;
	FF7Text *getText(int textID) const;
	void insertText(int row);
	void deleteText(int row);
	QSet<quint8> listUsedTexts() const;
	QSet<quint8> listUsedTuts() const;

	void setSaved();
	virtual QByteArray save(const QByteArray &fileData, bool compress)=0;
	qint8 exporter(const QString &path, const QByteArray &data, bool compress);
	qint8 exporterDat(const QString &path, const QByteArray &data);
	qint8 importer(const QString &path, FieldParts part);
	virtual qint8 importer(const QByteArray &data, bool isDat, FieldParts part);

	EncounterFile *getEncounter();
	TutFile *getTut();
	IdFile *getId();
	CaFile *getCa();
	InfFile *getInf();
	virtual FieldModelLoader *getFieldModelLoader()=0;

	const QString &getName() const;
	void setName(const QString &name);

	const QString &getAuthor() const;
	void setAuthor(const QString &author);

	quint16 getScale() const;
	void setScale(quint16 scale);
protected:
	qint8 openSection1(const QByteArray &contenu, int posStart);
	QByteArray saveSection1(const QByteArray &contenu) const;

	static QRgb blendColor(quint8 type, QRgb color0, QRgb color1);

	bool _isOpen, _isModified;

	QString name;
	QString author;
	quint16 scale;
	// quint8 nbObjets3D;

	QList<GrpScript *> _grpScripts;
	QList<FF7Text *> textes;

	EncounterFile *encounter;
	TutFile *tut;
	IdFile *id;
	CaFile *ca;
	InfFile *inf;
	FieldModelLoader *modelLoader;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Field::FieldParts)

#endif
