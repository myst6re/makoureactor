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
#include "IsoArchive.h"
#include "EncounterFile.h"
#include "TutFile.h"
#include "WalkmeshFile.h"
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

//Sizeof : 8
typedef struct {
	qint16 cibleX, cibleY;
	quint8 srcX, srcY;
	unsigned ZZ1:6;
	unsigned palID:4;
	unsigned ZZ2:6;
} layer1Tile;

//Sizeof : 2
typedef struct {
	unsigned page_x:4;
	unsigned page_y:1;
	unsigned typeTrans:2;//transparence n°3
	unsigned deph:2;
	unsigned ZZZ:7;
} layer2Tile;

//Sizeof : 2
typedef struct {
	unsigned param:7;
	unsigned blending:1;//transparence n°1
	quint8 state;
} layer3Tile;

//Sizeof : 4
typedef struct {
	quint16 group;//id
	unsigned param:7;
	unsigned blending:1;//transparence n°1
	quint8 state;
} paramTile;

//Sizeof : 12
typedef struct {
	quint32 size;// = 12 + w*2*h
	quint16 x, y;
	quint16 w, h;
} MIM;

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
	Field(quint32 position, const QString &name);
	virtual ~Field();

	bool isOpen() const;
	bool isModified() const;
	void setModified(bool modified);

	qint8 open(const QByteArray &fileData);
	qint8 open2(const QByteArray &fileData);
	void close();
	int getModelID(quint8) const;
	void getBgParamAndBgMove(QHash<quint8, quint8> &paramActifs, qint16 *z=0, qint16 *x=0, qint16 *y=0) const;
	QPixmap openModelAndBackground(const QByteArray &contenu);
	bool getUsedParamsPC(const QByteArray &contenu, QHash<quint8, quint8> &usedParams, bool *layerExists) const;
	bool getUsedParamsPS(const QByteArray &datData, QHash<quint8, quint8> &usedParams, bool *layerExists) const;
	QPixmap ouvrirBackgroundPC(const QByteArray &contenu, const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL) const;
	QPixmap ouvrirBackgroundPS(const QByteArray &mimDataDec, const QByteArray &datDataDec) const;
	QPixmap ouvrirBackgroundPS(const QByteArray &mimDataDec, const QByteArray &datDataDec, const QHash<quint8, quint8> &paramActifs, const qint16 z[2], const bool *layers=NULL) const;

	void insertGrpScript(int row);
	void insertGrpScript(int row, GrpScript *grpScript);
	void deleteGrpScript(int row);
	void removeGrpScript(int row);
	bool moveGrpScript(int row, bool direction);
	QList<FF7Var> searchAllVars() const;
	bool rechercherOpcode(int opcode, int &groupID, int &scriptID, int &opcodeID) const;
	bool rechercherVar(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const;
	bool rechercherExec(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const;
	bool rechercherTexte(const QRegExp &texte, int &groupID, int &scriptID, int &opcodeID) const;
	bool rechercherOpcodeP(int opcode, int &groupID, int &scriptID, int &opcodeID) const;
	bool rechercherVarP(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const;
	bool rechercherExecP(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const;
	bool rechercherTexteP(const QRegExp &texte, int &groupID, int &scriptID, int &opcodeID) const;

	void insertText(int row);
	void deleteText(int row);
	QSet<quint8> listUsedTexts() const;
	QSet<quint8> listUsedTuts() const;

	void shiftTutIds(int row, int shift);

	void setSaved();
	QByteArray save(const QByteArray &fileData, bool compress);
	QByteArray saveDat(const QByteArray &fileData, bool compress);
	qint8 exporter(const QString &path, const QByteArray &data, bool compress);
	qint8 exporterDat(const QString &path, const QByteArray &datData);
	qint8 importer(const QString &path, FieldParts part);

	QString HRCName(int modelID);
	QString AName(int modelID, int numA=0);

	EncounterFile *getEncounter();
	TutFile *getTut();
	WalkmeshFile *getWalkmesh();
	InfFile *getInf();
	FieldModelLoader *getFieldModelLoader();

	const QString &getName() const;
	void setName(const QString &name);

	quint32 getPosition() const;
	void setPosition(quint32 position);

	const QString &getAuthor() const;
	void setAuthor(const QString &author);

	quint16 getScale() const;
	void setScale(quint16 scale);

	QList<GrpScript *> grpScripts;
	QList<FF7Text *> *getTexts();
	int getNbTexts() const;
	FF7Text *getText(int textID) const;
private:
	qint8 openSection1(const QByteArray &contenu, int posStart);
	QByteArray saveSection1(const QByteArray &contenu) const;

	static QRgb blendColor(quint8 type, QRgb color0, QRgb color1);

	bool _isOpen, _isModified;

	QString name;
	quint32 position;
	QString author;
	quint16 scale;
	// quint8 nbObjets3D;

	QList<FF7Text *> textes;

	EncounterFile *encounter;
	TutFile *tut;
	WalkmeshFile *walkmesh;
	InfFile *inf;
	FieldModelLoader *modelLoader;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Field::FieldParts)

#endif
