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
#ifndef DEF_COMMANDE
#define DEF_COMMANDE

#include <QtGui>
#include "FF7Text.h"
#include "Var.h"
#include "Data.h"
#include "parametres.h"

#define B1(v)		((v>>4)&0xF)
#define B2(v)		(v&0xF)

typedef struct {
	quint16 opcode;
	qint16 x, y;
	quint16 w, h;
	quint16 ask_first, ask_last;
	quint8 type;
	quint16 groupID, scriptID, commandeID;
} FF7Window;

class Commande
{	
public:
	explicit Commande(const QByteArray &commande, int pos=0);
	Commande();

	virtual quint8 id() const;
	virtual QString toString() const;
	virtual QByteArray params() const;

	quint8 size() const;
	QByteArray &getParams();
	const QByteArray &getConstParams() const;
	QByteArray toByteArray() const;
	quint16 getIndent() const;
	quint16 getPos() const;
	int subParam(int cur, int paramSize) const;

	void setCommande(const QByteArray &commande);
	void setPos(quint16 pos);

	bool isVoid() const;

	bool rechercherVar(quint8 bank, quint8 adress, int value=65536) const;
	QList<int> searchAllVars();
	bool rechercherExec(quint8 group, quint8 script) const;
	bool rechercherTexte(const QRegExp &texte) const;
	void listUsedTexts(QSet<quint8> &usedTexts) const;
	void listUsedTuts(QSet<quint8> &usedTuts) const;
	void shiftTextIds(int textId, int steps);
	void shiftTutIds(int tutId, int steps);
	void listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const;

	QString toString2() const;
	QString traduction() const;

	static QString _personnage(quint8 persoID);

protected:
	quint8 opcode;
	quint16 pos;
	QByteArray _params;

	static QString _script(quint8 param);
	static QString _text(quint8 textID);
	static QString _item(const QByteArray &param, quint8 bank);
	static QString _item(quint16 itemID, quint8 bank);
	static QString _materia(const QByteArray &param, quint8 bank);
	static QString _materia(quint8 materiaID, quint8 bank);
	static QString _field(const QByteArray &param);
	static QString _field(quint16 fieldID);
	static QString _movie(quint8 movieID);
	// static QString _objet3D(quint8 objet3D_ID);

	static QString _bank(quint8 adress, quint8 bank);
	static QString _var(const QByteArray &param, quint8 bank);
	static QString _lvar(const QByteArray &param, quint8 bank1, quint8 bank2);
	static QString _lvar(const QByteArray &param, quint8 bank1, quint8 bank2, quint8 bank3);
	static QString _svar(const QByteArray &param, quint8 bank);
	static QString _var(int value, quint8 bank);
	static QString _var(int value, quint8 bank1, quint8 bank2);
	static QString _var(int value, quint8 bank1, quint8 bank2, quint8 bank3);
	static quint32 _toInt(const QByteArray &param);
	static qint16 _toSInt(const QByteArray &param);

	static QString _operateur(quint8 param);
	static QString _miniGame(quint8 ID, quint8 param);
	static QString _menu(quint8 ID, const QString &param);
	static QString _windowType(quint8 param);
	static QString _windowNum(quint8 param);
	static QString _windowCorner(quint8 param, quint8 bank);
	static QString _sensRotation(quint8 param);
	static QString _key(quint16 param);
	static QString _battleMode(quint32 param);
	QString _special() const;
	QString _kawai() const;
	
};

class OpcodeEmpty : public Commande {
public:
	explicit OpcodeEmpty(quint8 id);
	virtual quint8 id() const;
	virtual QString toString() const;
	quint8 _id;
};

class OpcodeRET : public Commande {
public:
	explicit OpcodeRET();
	quint8 id() const { return 0x00; }
	virtual QString toString() const;
};

class OpcodeExec : public Commande {
public:
	explicit OpcodeExec(const QByteArray &params);
	QByteArray params() const;
	quint8 groupID;
	quint8 scriptID;
	quint8 priority;
};

class OpcodeREQ : public OpcodeExec {
public:
	explicit OpcodeREQ(const QByteArray &params);
	quint8 id() const { return 0x01; }
	virtual QString toString() const;
};

class OpcodeREQSW : public OpcodeExec {
public:
	explicit OpcodeREQSW(const QByteArray &params);
	quint8 id() const { return 0x02; }
	virtual QString toString() const;
};

class OpcodeREQEW : public OpcodeExec {
public:
	explicit OpcodeREQEW(const QByteArray &params);
	quint8 id() const { return 0x03; }
	virtual QString toString() const;
};

class OpcodeExecChar : public Commande {
public:
	explicit OpcodeExecChar(const QByteArray &params);
	QByteArray params() const;
	quint8 charID;
	quint8 scriptID;
	quint8 priority;
};

class OpcodePREQ : public OpcodeExecChar {
public:
	explicit OpcodePREQ(const QByteArray &params);
	quint8 id() const { return 0x04; }
	virtual QString toString() const;
};

class OpcodePRQSW : public OpcodeExecChar {
public:
	explicit OpcodePRQSW(const QByteArray &params);
	quint8 id() const { return 0x05; }
	virtual QString toString() const;
};

class OpcodePRQEW : public OpcodeExecChar {
public:
	explicit OpcodePRQEW(const QByteArray &params);
	quint8 id() const { return 0x06; }
	virtual QString toString() const;
};

class OpcodeRETTO : public Commande {
public:
	explicit OpcodeRETTO(const QByteArray &params);
	quint8 id() const { return 0x07; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 scriptID;
	quint8 priority;
};

class OpcodeJOIN : public Commande {
public:
	explicit OpcodeJOIN(const QByteArray &params);
	quint8 id() const { return 0x08; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 speed;
};

class OpcodeSPLIT : public Commande {
public:
	explicit OpcodeSPLIT(const QByteArray &params);
	quint8 id() const { return 0x09; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[3];
	qint16 targetX1, targetY1, targetX2, targetY2;
	quint8 direction1, direction2;
	quint8 speed;
};

class OpcodePartyE : public Commande {
public:
	explicit OpcodePartyE(const QByteArray &params);
	QByteArray params() const;
	quint8 banks[2];
	quint8 party1, party2, party3;
};

class OpcodeSPTYE : public OpcodePartyE {
public:
	explicit OpcodeSPTYE(const QByteArray &params);
	quint8 id() const { return 0x0A; }
	virtual QString toString() const;
};

class OpcodeGTPYE : public OpcodePartyE {
public:
	explicit OpcodeGTPYE(const QByteArray &params);
	quint8 id() const { return 0x0B; }
	virtual QString toString() const;
};

class OpcodeDSKCG : public Commande {
public:
	explicit OpcodeDSKCG(const QByteArray &params);
	quint8 id() const { return 0x0E; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 diskID;
};

class OpcodeSPECIALARROW : public Commande {
public:
	explicit OpcodeSPECIALARROW(const QByteArray &params);
	quint8 id() const { return 0xF5; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 hide;
};

class OpcodeSPECIALPNAME : public Commande {
public:
	explicit OpcodeSPECIALPNAME(const QByteArray &params);
	quint8 id() const { return 0xF6; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown;
};

class OpcodeSPECIALGMSPD : public Commande {
public:
	explicit OpcodeSPECIALGMSPD(const QByteArray &params);
	quint8 id() const { return 0xF7; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 speed;
};

class OpcodeSPECIALSMSPD : public Commande {
public:
	explicit OpcodeSPECIALSMSPD(const QByteArray &params);
	quint8 id() const { return 0xF8; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown, speed;
};

class OpcodeSPECIALFLMAT : public Commande {
public:
	explicit OpcodeSPECIALFLMAT();
	quint8 id() const { return 0xF9; }
	virtual QString toString() const;
};

class OpcodeSPECIALFLITM : public Commande {
public:
	explicit OpcodeSPECIALFLITM();
	quint8 id() const { return 0xFA; }
	virtual QString toString() const;
};

class OpcodeSPECIALBTLCK : public Commande {
public:
	explicit OpcodeSPECIALBTLCK(const QByteArray &params);
	quint8 id() const { return 0xFB; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 lock;
};

class OpcodeSPECIALMVLCK : public Commande {
public:
	explicit OpcodeSPECIALMVLCK(const QByteArray &params);
	quint8 id() const { return 0xFC; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 lock;
};

class OpcodeSPECIALSPCNM : public Commande {
public:
	explicit OpcodeSPECIALSPCNM(const QByteArray &params);
	quint8 id() const { return 0xFD; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 charID;
	quint8 textID;
};

class OpcodeSPECIALRSGLB : public Commande {
public:
	explicit OpcodeSPECIALRSGLB();
	quint8 id() const { return 0xFE; }
	virtual QString toString() const;
};

class OpcodeSPECIALCLITM : public Commande {
public:
	explicit OpcodeSPECIALCLITM();
	quint8 id() const { return 0xFF; }
	virtual QString toString() const;
};

class OpcodeSPECIAL : public Commande {
public:
	explicit OpcodeSPECIAL(const QByteArray &params);
	virtual ~OpcodeSPECIAL();
	quint8 id() const { return 0x0F; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	Commande *opcode;
};

class OpcodeJump : public Commande {
public:
	explicit OpcodeJump();
	void setLabel(quint32 label);
	quint16 jump() const;
	void setJump(quint16 jump);
	quint32 _label;
	qint32 _jump;
};

class OpcodeJMPF : public OpcodeJump {
public:
	explicit OpcodeJMPF(const QByteArray &params);
	quint8 id() const { return 0x10; }
	virtual QString toString() const;
	virtual QByteArray params() const;
};

class OpcodeJMPFL : public OpcodeJump {
public:
	explicit OpcodeJMPFL(const QByteArray &params);
	quint8 id() const { return 0x11; }
	virtual QString toString() const;
	virtual QByteArray params() const;
};

class OpcodeJMPB : public OpcodeJump {
public:
	explicit OpcodeJMPB(const QByteArray &params);
	quint8 id() const { return 0x12; }
	virtual QString toString() const;
	virtual QByteArray params() const;
};

class OpcodeJMPBL : public OpcodeJump {
public:
	explicit OpcodeJMPBL(const QByteArray &params);
	quint8 id() const { return 0x13; }
	virtual QString toString() const;
	virtual QByteArray params() const;
};

class OpcodeIf : public OpcodeJump {
public:
	explicit OpcodeIf();
	quint8 banks;
	qint32 value1, value2;
	quint8 oper;
};

class OpcodeIFUB : public OpcodeIf {
public:
	explicit OpcodeIFUB(const QByteArray &params);
	quint8 id() const { return 0x14; }
	virtual QString toString() const;
	virtual QByteArray params() const;
};

class OpcodeIFUBL : public OpcodeIf {
public:
	explicit OpcodeIFUBL(const QByteArray &params);
	quint8 id() const { return 0x15; }
	virtual QString toString() const;
	virtual QByteArray params() const;
};

class OpcodeIFSW : public OpcodeIf {
public:
	explicit OpcodeIFSW(const QByteArray &params);
	quint8 id() const { return 0x16; }
	virtual QString toString() const;
	virtual QByteArray params() const;
};

class OpcodeIFSWL : public OpcodeIf {
public:
	explicit OpcodeIFSWL(const QByteArray &params);
	quint8 id() const { return 0x17; }
	virtual QString toString() const;
	virtual QByteArray params() const;
};

class OpcodeIFUW : public OpcodeIf {
public:
	explicit OpcodeIFUW(const QByteArray &params);
	quint8 id() const { return 0x18; }
	virtual QString toString() const;
	virtual QByteArray params() const;
};

class OpcodeIFUWL : public OpcodeIf {
public:
	explicit OpcodeIFUWL(const QByteArray &params);
	quint8 id() const { return 0x19; }
	virtual QString toString() const;
	virtual QByteArray params() const;
};

class OpcodeMINIGAME : public Commande {
public:
	explicit OpcodeMINIGAME(const QByteArray &params);
	quint8 id() const { return 0x20; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint16 fieldID;
	qint16 targetX;
	qint16 targetY;
	quint16 targetI;
	quint8 minigameParam;
	quint8 minigameID;
};

class OpcodeTUTOR : public Commande {
public:
	explicit OpcodeTUTOR(const QByteArray &params);
	quint8 id() const { return 0x21; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 tutoID;
};

class OpcodeBTMD2 : public Commande {
public:
	explicit OpcodeBTMD2(const QByteArray &params);
	quint8 id() const { return 0x22; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint32 battleMode;
};

class OpcodeBTRLD : public Commande {
public:
	explicit OpcodeBTRLD(const QByteArray &params);
	quint8 id() const { return 0x23; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint8 var;
};

class OpcodeWAIT : public Commande {
public:
	explicit OpcodeWAIT(const QByteArray &params);
	quint8 id() const { return 0x24; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint16 frameCount;
};

class OpcodeNFADE : public Commande {
public:
	explicit OpcodeNFADE(const QByteArray &params);
	quint8 id() const { return 0x25; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	quint8 r, g, b;
	quint8 unknown1, unknown2, unknown3;
};

class OpcodeBLINK : public Commande {
public:
	explicit OpcodeBLINK(const QByteArray &params);
	quint8 id() const { return 0x26; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 closed;
};

class OpcodeBGMOVIE : public Commande {
public:
	explicit OpcodeBGMOVIE(const QByteArray &params);
	quint8 id() const { return 0x27; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 disabled;
};

class OpcodeKAWAIUnknown : public Commande {
public:
	explicit OpcodeKAWAIUnknown(const QByteArray &params);
	virtual QString toString() const;
	virtual QByteArray params() const;
	QByteArray data;
};

class OpcodeKAWAIEYETX : public OpcodeKAWAIUnknown {
public:
	explicit OpcodeKAWAIEYETX(const QByteArray &params);
	quint8 id() const { return 0x00; }
	virtual QString toString() const;
};

class OpcodeKAWAITRNSP : public Commande {
public:
	explicit OpcodeKAWAITRNSP(const QByteArray &params);
	quint8 id() const { return 0x01; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 enableTransparency;
	QByteArray data;
};

class OpcodeKAWAIAMBNT : public OpcodeKAWAIUnknown {
public:
	explicit OpcodeKAWAIAMBNT(const QByteArray &params);
	quint8 id() const { return 0x02; }
	virtual QString toString() const;
};

class OpcodeKAWAILIGHT : public OpcodeKAWAIUnknown {
public:
	explicit OpcodeKAWAILIGHT(const QByteArray &params);
	quint8 id() const { return 0x06; }
	virtual QString toString() const;
};

class OpcodeKAWAISBOBJ : public OpcodeKAWAIUnknown {
public:
	explicit OpcodeKAWAISBOBJ(const QByteArray &params);
	quint8 id() const { return 0x0A; }
	virtual QString toString() const;
};

class OpcodeKAWAISHINE : public OpcodeKAWAIUnknown {
public:
	explicit OpcodeKAWAISHINE(const QByteArray &params);
	quint8 id() const { return 0x0D; }
	virtual QString toString() const;
};

class OpcodeKAWAIRESET : public OpcodeKAWAIUnknown {
public:
	explicit OpcodeKAWAIRESET(const QByteArray &params);
	quint8 id() const { return 0xFF; }
	virtual QString toString() const;
};

class OpcodeKAWAI : public Commande {
public:
	explicit OpcodeKAWAI(const QByteArray &params);
	virtual ~OpcodeKAWAI();
	quint8 id() const { return 0x28; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	Commande *opcode;
};

class OpcodeKAWIW : public Commande {
public:
	explicit OpcodeKAWIW();
	quint8 id() const { return 0x29; }
	virtual QString toString() const;
};

class OpcodePMOVA : public Commande {
public:
	explicit OpcodePMOVA(const QByteArray &params);
	quint8 id() const { return 0x2A; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 partyID;
};

class OpcodeSLIP : public Commande {
public:
	explicit OpcodeSLIP(const QByteArray &params);
	quint8 id() const { return 0x2B; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 off;
};

class OpcodeBGPDH : public Commande {
public:
	explicit OpcodeBGPDH(const QByteArray &params);
	quint8 id() const { return 0x2C; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint8 layerID;
	qint16 targetZ;
};

class OpcodeBGSCR : public Commande {
public:
	explicit OpcodeBGSCR(const QByteArray &params);
	quint8 id() const { return 0x2D; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint8 layerID;
	qint16 targetX, targetY;
};

class OpcodeWCLS : public Commande {
public:
	explicit OpcodeWCLS(const QByteArray &params);
	quint8 id() const { return 0x2E; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 windowID;
};

class OpcodeWSIZW : public Commande {
public:
	explicit OpcodeWSIZW(const QByteArray &params);
	quint8 id() const { return 0x2F; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 windowID;
	quint16 targetX, targetY;
	quint16 width, height;
};

class OpcodeIfKey : public OpcodeJump {
public:
	explicit OpcodeIfKey(const QByteArray &params);
	virtual QByteArray params() const;
	QString keyString() const;
	quint16 keys;
};

class OpcodeIFKEY : public OpcodeIfKey {
public:
	explicit OpcodeIFKEY(const QByteArray &params);
	quint8 id() const { return 0x30; }
	virtual QString toString() const;
};

class OpcodeIFKEYON : public OpcodeIfKey {
public:
	explicit OpcodeIFKEYON(const QByteArray &params);
	quint8 id() const { return 0x31; }
	virtual QString toString() const;
};

class OpcodeIFKEYOFF : public OpcodeIfKey {
public:
	explicit OpcodeIFKEYOFF(const QByteArray &params);
	quint8 id() const { return 0x32; }
	virtual QString toString() const;
};

class OpcodeUC : public Commande {
public:
	explicit OpcodeUC(const QByteArray &params);
	quint8 id() const { return 0x33; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 disabled;
};

class OpcodePDIRA : public Commande {
public:
	explicit OpcodePDIRA(const QByteArray &params);
	quint8 id() const { return 0x34; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 partyID;
};

class OpcodePTURA : public Commande {
public:
	explicit OpcodePDIRA(const QByteArray &params);
	quint8 id() const { return 0x35; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 partyID;
	quint8 speed;
	quint8 directionRotation;
};

class OpcodeWSPCL : public Commande {
public:
	explicit OpcodeWSPCL(const QByteArray &params);
	quint8 id() const { return 0x36; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 windowID;
	quint8 displayType;
	quint8 marginLeft, marginTop;
};

class OpcodeWNUMB : public Commande {
public:
	explicit OpcodeWNUMB(const QByteArray &params);
	quint8 id() const { return 0x37; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint8 windowID;
	qint32 value;
	quint8 digitCount;
};

class OpcodeSTTIM : public Commande {
public:
	explicit OpcodeSTTIM(const QByteArray &params);
	quint8 id() const { return 0x38; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	quint8 h, m, s;
};

class OpcodeGOLD : public Commande {
public:
	explicit OpcodeGOLD(const QByteArray &params);
	virtual QByteArray params() const;
	quint8 banks;
	qint32 value;
};

class OpcodeGOLDu : public OpcodeGOLD {
public:
	explicit OpcodeGOLDu(const QByteArray &params);
	quint8 id() const { return 0x39; }
	virtual QString toString() const;
};

class OpcodeGOLDd : public OpcodeGOLD {
public:
	explicit OpcodeGOLDd(const QByteArray &params);
	quint8 id() const { return 0x3A; }
	virtual QString toString() const;
};

class OpcodeCHGLD : public Commande {
public:
	explicit OpcodeCHGLD(const QByteArray &params);
	quint8 id() const { return 0x3B; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint8 var1, var2;
};

class OpcodeHMPMAX1 : public Commande {
public:
	explicit OpcodeHMPMAX1();
	quint8 id() const { return 0x3C; }
	virtual QString toString() const;
};

class OpcodeHMPMAX2 : public Commande {
public:
	explicit OpcodeHMPMAX2();
	quint8 id() const { return 0x3D; }
	virtual QString toString() const;
};

class OpcodeMHMMX : public Commande {
public:
	explicit OpcodeMHMMX();
	quint8 id() const { return 0x3E; }
	virtual QString toString() const;
};

class OpcodeHMPMAX3 : public Commande {
public:
	explicit OpcodeHMPMAX3();
	quint8 id() const { return 0x3F; }
	virtual QString toString() const;
};

class OpcodeMESSAGE : public Commande {
public:
	explicit OpcodeMESSAGE(const QByteArray &params);
	quint8 id() const { return 0x40; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 windowID, textID;
};

class OpcodeMPARA : public Commande {
public:
	explicit OpcodeMPARA(const QByteArray &params);
	quint8 id() const { return 0x41; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, windowID, windowVarID, value;
};

class OpcodeMPRA2 : public Commande {
public:
	explicit OpcodeMPRA2(const QByteArray &params);
	quint8 id() const { return 0x42; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, windowID, windowVarID;
	quint16 value;
};

class OpcodeMPNAM : public Commande {
public:
	explicit OpcodeMPNAM(const QByteArray &params);
	quint8 id() const { return 0x43; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 textID;
};

class OpcodeMP : public Commande {
public:
	explicit OpcodeMP(const QByteArray &params);
	virtual QByteArray params() const;
	quint8 banks, partyID;
	quint16 value;
};

class OpcodeMPu : public OpcodeMP {
public:
	explicit OpcodeMPu();
	quint8 id() const { return 0x45; }
	virtual QString toString() const;
};

class OpcodeMPd : public OpcodeMP {
public:
	explicit OpcodeMPd();
	quint8 id() const { return 0x47; }
	virtual QString toString() const;
};

class OpcodeASK : public Commande {
public:
	explicit OpcodeASK(const QByteArray &params);
	quint8 id() const { return 0x48; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, windowID, textID, firstLine, lastLine, varAnswer;
};

class OpcodeMENU : public Commande {
public:
	explicit OpcodeMENU(const QByteArray &params);
	quint8 id() const { return 0x49; }
	QString menu(const QString &param);
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, menuID, param;
};

class OpcodeMENU2 : public Commande {
public:
	explicit OpcodeMENU2(const QByteArray &params);
	quint8 id() const { return 0x4A; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 disabled;
};

class OpcodeBTLTB : public Commande {
public:
	explicit OpcodeBTLTB(const QByteArray &params);
	quint8 id() const { return 0x4B; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 battleTableID;
};

class OpcodeHP : public Commande {
public:
	explicit OpcodeHP(const QByteArray &params);
	virtual QByteArray params() const;
	quint8 banks, partyID;
	quint16 value;
};

class OpcodeHPu : public OpcodeHP {
public:
	explicit OpcodeHPu(const QByteArray &params);
	quint8 id() const { return 0x4D; }
	virtual QString toString() const;
};

class OpcodeHPd : public OpcodeHP {
public:
	explicit OpcodeHPd(const QByteArray &params);
	quint8 id() const { return 0x4F; }
	virtual QString toString() const;
};
// note: same struct as WSIZW
class OpcodeWINDOW : public Commande {
public:
	explicit OpcodeWINDOW(const QByteArray &params);
	quint8 id() const { return 0x50; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 windowID;
	quint16 targetX, targetY;
	quint16 width, height;
};

class OpcodeWMOVE : public Commande {
public:
	explicit OpcodeWMOVE(const QByteArray &params);
	quint8 id() const { return 0x51; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 windowID;
	quint16 targetX, targetY;
};

class OpcodeWMODE : public Commande {
public:
	explicit OpcodeWMODE(const QByteArray &params);
	quint8 id() const { return 0x52; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 windowID, mode;
	quint8 preventClose;
};

class OpcodeWREST : public Commande {
public:
	explicit OpcodeWREST(const QByteArray &params);
	quint8 id() const { return 0x53; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 windowID;
};
// note: same struct as WREST
class OpcodeWCLSE : public Commande {
public:
	explicit OpcodeWCLSE(const QByteArray &params);
	quint8 id() const { return 0x54; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 windowID;
};

class OpcodeWROW : public Commande {
public:
	explicit OpcodeWROW(const QByteArray &params);
	quint8 id() const { return 0x55; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 windowID, rowCount;
};

class OpcodeGWCOL : public Commande {
public:
	explicit OpcodeGWCOL(const QByteArray &params);
	quint8 id() const { return 0x56; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2], corner, varR, varG, varB;
};
// note: same struct as GWCOL
class OpcodeSWCOL : public Commande {
public:
	explicit OpcodeSWCOL(const QByteArray &params);
	quint8 id() const { return 0x57; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2], corner, r, g, b;
};

class OpcodeSTITM : public Commande {
public:
	explicit OpcodeSTITM(const QByteArray &params);
	quint8 id() const { return 0x58; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint16 itemID;
	quint8 quantity;
};
// note: same struct as STITM
class OpcodeDLITM : public Commande {
public:
	explicit OpcodeDLITM(const QByteArray &params);
	quint8 id() const { return 0x59; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint16 itemID;
	quint8 quantity;
};
// note: same struct as STITM
class OpcodeCKITM : public Commande {
public:
	explicit OpcodeCKITM(const QByteArray &params);
	quint8 id() const { return 0x5A; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint16 itemID;
	quint8 varQuantity;
};

class OpcodeSMTRA : public Commande {
public:
	explicit OpcodeSMTRA(const QByteArray &params);
	quint8 id() const { return 0x5B; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	quint8 materiaID;
	quint32 APCount;
};

class OpcodeDMTRA : public Commande {
public:
	explicit OpcodeDMTRA(const QByteArray &params);
	quint8 id() const { return 0x5C; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	quint8 materiaID;
	quint32 APCount;
	quint8 quantity;
};

class OpcodeCMTRA : public Commande {
public:
	explicit OpcodeCMTRA(const QByteArray &params);
	quint8 id() const { return 0x5D; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[3];
	quint8 materiaID;
	quint32 APCount;
	quint8 unknown, varQuantity;
};

class OpcodeSHAKE : public Commande {
public:
	explicit OpcodeSHAKE(const QByteArray &params);
	quint8 id() const { return 0x5E; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown1, unknown2, shakeCount, unknown3, unknown4;
	quint8 amplitude, speed;
};

class OpcodeNOP : public Commande {
public:
	explicit OpcodeNOP();
	quint8 id() const { return 0x5F; }
	virtual QString toString() const;
};

class OpcodeMAPJUMP : public Commande {
public:
	explicit OpcodeMAPJUMP(const QByteArray &params);
	quint8 id() const { return 0x60; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint16 fieldID;
	qint16 targetX, targetY;
	quint16 targetI;
	quint8 direction;
};

class OpcodeSCRLO : public Commande {
public:
	explicit OpcodeSCRLO(const QByteArray &params);
	quint8 id() const { return 0x61; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown;
};

class OpcodeSCRLC : public Commande {
public:
	explicit OpcodeSCRLC(const QByteArray &params);
	quint8 id() const { return 0x62; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint32 unknown;
};

class OpcodeSCRLA : public Commande {
public:
	explicit OpcodeSCRLA(const QByteArray &params);
	quint8 id() const { return 0x63; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint16 speed;
	quint8 groupID, scrollType;
};

class OpcodeSCR2D : public Commande {
public:
	explicit OpcodeSCR2D(const QByteArray &params);
	quint8 id() const { return 0x64; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	qint16 targetX, targetY;
};

class OpcodeSCRCC : public Commande {
public:
	explicit OpcodeSCRCC();
	quint8 id() const { return 0x65; }
	virtual QString toString() const;
};

class OpcodeSCR2DC : public Commande {
public:
	explicit OpcodeSCR2DC(const QByteArray &params);
	quint8 id() const { return 0x66; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 speed;
};

class OpcodeSCRLW : public Commande {
public:
	explicit OpcodeSCRLW();
	quint8 id() const { return 0x67; }
	virtual QString toString() const;
};
// note: same struct as SCR2DC
class OpcodeSCR2DL : public Commande {
public:
	explicit OpcodeSCR2DL(const QByteArray &params);
	quint8 id() const { return 0x68; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 speed;
};

class OpcodeMPDSP : public Commande {
public:
	explicit OpcodeMPDSP(const QByteArray &params);
	quint8 id() const { return 0x69; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown;
};

class OpcodeVWOFT : public Commande {
public:
	explicit OpcodeVWOFT(const QByteArray &params);
	quint8 id() const { return 0x6A; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	qint16 unknown1, unknown2;
	quint8 unknown3;
};

class OpcodeFADE : public Commande {
public:
	explicit OpcodeFADE(const QByteArray &params);
	quint8 id() const { return 0x6B; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	quint8 r, g, b, speed, fadeType, adjust;
};

class OpcodeFADEW : public Commande {
public:
	explicit OpcodeFADEW();
	quint8 id() const { return 0x6C; }
	virtual QString toString() const;
};

class OpcodeIDLCK : public Commande {
public:
	explicit OpcodeIDLCK(const QByteArray &params);
	quint8 id() const { return 0x6D; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint16 triangleID;
	quint8 locked;
};

class OpcodeLSTMP : public Commande {
public:
	explicit OpcodeLSTMP(const QByteArray &params);
	quint8 id() const { return 0x6E; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, var;
};

class OpcodeSCRLP : public Commande {
public:
	explicit OpcodeSCRLP(const QByteArray &params);
	quint8 id() const { return 0x6F; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint16 speed;
	quint8 partyID, scrollType;
};

class OpcodeBATTLE : public Commande {
public:
	explicit OpcodeBATTLE(const QByteArray &params);
	quint8 id() const { return 0x70; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint16 battleID;
};

class OpcodeBTLON : public Commande {
public:
	explicit OpcodeBTLON(const QByteArray &params);
	quint8 id() const { return 0x71; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 disabled;
};

class OpcodeBTLMD : public Commande {
public:
	explicit OpcodeBTLMD(const QByteArray &params);
	quint8 id() const { return 0x72; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint16 unknown;
};

class OpcodePGTDR : public Commande {
public:
	explicit OpcodePGTDR(const QByteArray &params);
	quint8 id() const { return 0x73; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, partyID, varDir;
};
// note: same struct as PGTDR
class OpcodeGETPC : public Commande {
public:
	explicit OpcodeGETPC(const QByteArray &params);
	quint8 id() const { return 0x74; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, partyID, varPC;
};

class OpcodePXYZI : public Commande {
public:
	explicit OpcodePXYZI(const QByteArray &params);
	quint8 id() const { return 0x75; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2], partyID, varX, varY, varZ, varI;
};

class OpcodeOperation : public Commande {
public:
	explicit OpcodeOperation(const QByteArray &params);
	virtual QByteArray params() const;
	quint8 banks, var, value;
};

class OpcodeOperation2 : public Commande {
public:
	explicit OpcodeOperation2(const QByteArray &params);
	virtual QByteArray params() const;
	quint8 banks, var;
	quint16 value;
};

class OpcodeUnaryOperation : public Commande {
public:
	explicit OpcodeUnaryOperation(const QByteArray &params);
	virtual QByteArray params() const;
	quint8 banks, var;
};

class OpcodePLUSX : public OpcodeOperation {
public:
	explicit OpcodePLUSX(const QByteArray &params);
	quint8 id() const { return 0x76; }
	virtual QString toString() const;
};

class OpcodePLUS2X : public OpcodeOperation2 {
public:
	explicit OpcodePLUS2X(const QByteArray &params);
	quint8 id() const { return 0x77; }
	virtual QString toString() const;
};

class OpcodeMINUSX : public OpcodeOperation {
public:
	explicit OpcodeMINUSX(const QByteArray &params);
	quint8 id() const { return 0x78; }
	virtual QString toString() const;
};

class OpcodeMINUS2X : public OpcodeOperation2 {
public:
	explicit OpcodeMINUS2X(const QByteArray &params);
	quint8 id() const { return 0x79; }
	virtual QString toString() const;
};

class OpcodeINCX : public OpcodeUnaryOperation {
public:
	explicit OpcodeINCX(const QByteArray &params);
	quint8 id() const { return 0x7A; }
	virtual QString toString() const;
};

class OpcodeINC2X : public OpcodeUnaryOperation {
public:
	explicit OpcodeINC2X(const QByteArray &params);
	quint8 id() const { return 0x7B; }
	virtual QString toString() const;
};

class OpcodeDECX : public OpcodeUnaryOperation {
public:
	explicit OpcodeDECX(const QByteArray &params);
	quint8 id() const { return 0x7C; }
	virtual QString toString() const;
};

class OpcodeDEC2X : public OpcodeUnaryOperation {
public:
	explicit OpcodeDECX(const QByteArray &params);
	quint8 id() const { return 0x7D; }
	virtual QString toString() const;
};

class OpcodeTLKON : public Commande {
public:
	explicit OpcodeTLKON(const QByteArray &params);
	quint8 id() const { return 0x7E; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 disabled;
};

class OpcodeRDMSD : public OpcodeUnaryOperation {
public:
	explicit OpcodeRDMSD(const QByteArray &params);
	quint8 id() const { return 0x7F; }
	virtual QString toString() const;
};

class OpcodeSETBYTE : public OpcodeOperation {
public:
	explicit OpcodeSETBYTE(const QByteArray &params);
	quint8 id() const { return 0x80; }
	virtual QString toString() const;
};

class OpcodeSETWORD : public OpcodeOperation2 {
public:
	explicit OpcodeSETWORD(const QByteArray &params);
	quint8 id() const { return 0x81; }
	virtual QString toString() const;
};

class OpcodeBitOperation : public Commande {
public:
	explicit OpcodeBitOperation(const QByteArray &params);
	virtual QByteArray params() const;
	quint8 banks, var, position;
};

class OpcodeBITON : public OpcodeBitOperation {
public:
	explicit OpcodeBITON(const QByteArray &params);
	quint8 id() const { return 0x82; }
	virtual QString toString() const;
};

class OpcodeBITOFF : public OpcodeBitOperation {
public:
	explicit OpcodeBITOFF(const QByteArray &params);
	quint8 id() const { return 0x83; }
	virtual QString toString() const;
};

class OpcodeBITXOR : public OpcodeBitOperation {
public:
	explicit OpcodeBITXOR(const QByteArray &params);
	quint8 id() const { return 0x84; }
	virtual QString toString() const;
};

class OpcodePLUS : public OpcodeOperation {
public:
	explicit OpcodePLUS(const QByteArray &params);
	quint8 id() const { return 0x85; }
	virtual QString toString() const;
};

class OpcodePLUS2 : public OpcodeOperation2 {
public:
	explicit OpcodePLUS2(const QByteArray &params);
	quint8 id() const { return 0x86; }
	virtual QString toString() const;
};

class OpcodeMINUS : public OpcodeOperation {
public:
	explicit OpcodeMINUS(const QByteArray &params);
	quint8 id() const { return 0x87; }
	virtual QString toString() const;
};

class OpcodeMINUS2 : public OpcodeOperation2 {
public:
	explicit OpcodeMINUS2(const QByteArray &params);
	quint8 id() const { return 0x88; }
	virtual QString toString() const;
};

class OpcodeMUL : public OpcodeOperation {
public:
	explicit OpcodeMUL(const QByteArray &params);
	quint8 id() const { return 0x89; }
	virtual QString toString() const;
};

class OpcodeMUL2 : public OpcodeOperation2 {
public:
	explicit OpcodeMUL2(const QByteArray &params);
	quint8 id() const { return 0x8A; }
	virtual QString toString() const;
};

class OpcodeDIV : public OpcodeOperation {
public:
	explicit OpcodeDIV(const QByteArray &params);
	quint8 id() const { return 0x8B; }
	virtual QString toString() const;
};

class OpcodeDIV2 : public OpcodeOperation2 {
public:
	explicit OpcodeDIV2(const QByteArray &params);
	quint8 id() const { return 0x8C; }
	virtual QString toString() const;
};

class OpcodeMOD : public OpcodeOperation {
public:
	explicit OpcodeMOD(const QByteArray &params);
	quint8 id() const { return 0x8D; }
	virtual QString toString() const;
};

class OpcodeMOD2 : public OpcodeOperation2 {
public:
	explicit OpcodeMOD2(const QByteArray &params);
	quint8 id() const { return 0x8E; }
	virtual QString toString() const;
};

class OpcodeAND : public OpcodeOperation {
public:
	explicit OpcodeAND(const QByteArray &params);
	quint8 id() const { return 0x8F; }
	virtual QString toString() const;
};

class OpcodeAND2 : public OpcodeOperation2 {
public:
	explicit OpcodeAND2(const QByteArray &params);
	quint8 id() const { return 0x90; }
	virtual QString toString() const;
};

class OpcodeOR : public OpcodeOperation {
public:
	explicit OpcodeOR(const QByteArray &params);
	quint8 id() const { return 0x91; }
	virtual QString toString() const;
};

class OpcodeOR2 : public OpcodeOperation2 {
public:
	explicit OpcodeOR2(const QByteArray &params);
	quint8 id() const { return 0x92; }
	virtual QString toString() const;
};

class OpcodeXOR : public OpcodeOperation {
public:
	explicit OpcodeXOR(const QByteArray &params);
	quint8 id() const { return 0x93; }
	virtual QString toString() const;
};

class OpcodeXOR2 : public OpcodeOperation2 {
public:
	explicit OpcodeXOR2(const QByteArray &params);
	quint8 id() const { return 0x94; }
	virtual QString toString() const;
};

class OpcodeINC : public OpcodeUnaryOperation {
public:
	explicit OpcodeINC(const QByteArray &params);
	quint8 id() const { return 0x95; }
	virtual QString toString() const;
};

class OpcodeINC2 : public OpcodeUnaryOperation {
public:
	explicit OpcodeINC2(const QByteArray &params);
	quint8 id() const { return 0x96; }
	virtual QString toString() const;
};

class OpcodeDEC : public OpcodeUnaryOperation {
public:
	explicit OpcodeDEC(const QByteArray &params);
	quint8 id() const { return 0x97; }
	virtual QString toString() const;
};

class OpcodeDEC2 : public OpcodeUnaryOperation {
public:
	explicit OpcodeDEC2(const QByteArray &params);
	quint8 id() const { return 0x98; }
	virtual QString toString() const;
};

class OpcodeRANDOM : public OpcodeUnaryOperation {
public:
	explicit OpcodeRANDOM(const QByteArray &params);
	quint8 id() const { return 0x99; }
	virtual QString toString() const;
};

class OpcodeLBYTE : public OpcodeOperation {
public:
	explicit OpcodeLBYTE(const QByteArray &params);
	quint8 id() const { return 0x9A; }
	virtual QString toString() const;
};

class OpcodeHBYTE : public OpcodeOperation2 {
public:
	explicit OpcodeHBYTE(const QByteArray &params);
	quint8 id() const { return 0x9B; }
	virtual QString toString() const;
};

class Opcode2BYTE : public Commande {
public:
	explicit Opcode2BYTE(const QByteArray &params);
	quint8 id() const { return 0x9C; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2], var, value1, value2;
};

class OpcodeSETX : public Commande {
public:
	explicit OpcodeSETX(const QByteArray &params);
	quint8 id() const { return 0x9D; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[6];
};

class OpcodeGETX : public Commande {
public:
	explicit OpcodeGETX(const QByteArray &params);
	quint8 id() const { return 0x9E; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[6];
};

class OpcodeSEARCHX : public Commande {
public:
	explicit OpcodeSEARCHX(const QByteArray &params);
	quint8 id() const { return 0x9F; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[10];
};

class OpcodePC : public Commande {
public:
	explicit OpcodePC(const QByteArray &params);
	quint8 id() const { return 0xA0; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 charID;
};

class OpcodeCHAR : public Commande {
public:
	explicit OpcodeCHAR(const QByteArray &params);
	quint8 id() const { return 0xA1; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 objectID;
};

class OpcodeDFANM : public Commande {
public:
	explicit OpcodeDFANM(const QByteArray &params);
	quint8 id() const { return 0xA2; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 animID, speed;
};
// note: same struct as DFANM
class OpcodeANIME1 : public Commande {
public:
	explicit OpcodeANIME1(const QByteArray &params);
	quint8 id() const { return 0xA3; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 animID, speed;
};

class OpcodeVISI : public Commande {
public:
	explicit OpcodeVISI(const QByteArray &params);
	quint8 id() const { return 0xA4; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 show;
};

class OpcodeXYZI : public Commande {
public:
	explicit OpcodeXYZI(const QByteArray &params);
	quint8 id() const { return 0xA5; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	qint16 targetX, targetY, targetZ;
	quint16 targetI;
};

class OpcodeXYI : public Commande {
public:
	explicit OpcodeXYI(const QByteArray &params);
	quint8 id() const { return 0xA6; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 targetI;
};

class OpcodeXYZ : public Commande {
public:
	explicit OpcodeXYZ(const QByteArray &params);
	quint8 id() const { return 0xA7; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	qint16 targetX, targetY, targetZ;
};

class OpcodeMOVE : public Commande {
public:
	explicit OpcodeMOVE(const QByteArray &params);
	quint8 id() const { return 0xA8; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	qint16 targetX, targetY;
};
// note: same struct as MOVE
class OpcodeCMOVE : public Commande {
public:
	explicit OpcodeCMOVE(const QByteArray &params);
	quint8 id() const { return 0xA9; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	qint16 targetX, targetY;
};

class OpcodeMOVA : public Commande {
public:
	explicit OpcodeMOVA(const QByteArray &params);
	quint8 id() const { return 0xAA; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 groupID;
};

class OpcodeTURA : public Commande {
public:
	explicit OpcodeTURA(const QByteArray &params);
	quint8 id() const { return 0xAB; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 groupID, directionRotation, speed;
};

class OpcodeANIMW : public Commande {
public:
	explicit OpcodeANIMW();
	quint8 id() const { return 0xAC; }
	virtual QString toString() const;
};

class OpcodeFMOVE : public Commande {
public:
	explicit OpcodeFMOVE(const QByteArray &params);
	quint8 id() const { return 0xAD; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	qint16 targetX, targetY;
};

class OpcodeANIME2 : public Commande {
public:
	explicit OpcodeANIME2(const QByteArray &params);
	quint8 id() const { return 0xAE; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 animID, speed;
};
// note: same struct as ANIME2
class OpcodeANIMX1 : public Commande {
public:
	explicit OpcodeANIMX1(const QByteArray &params);
	quint8 id() const { return 0xAF; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 animID, speed;
};

class OpcodeCANIM1 : public Commande {
public:
	explicit OpcodeCANIM1(const QByteArray &params);
	quint8 id() const { return 0xB0; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 animID, firstFrame, lastFrame, speed;
};
// note: same struct as CANIM1
class OpcodeCANMX1 : public Commande {
public:
	explicit OpcodeCANMX1(const QByteArray &params);
	quint8 id() const { return 0xB1; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 animID, firstFrame, lastFrame, speed;
};

class OpcodeMSPED : public Commande {
public:
	explicit OpcodeMSPED(const QByteArray &params);
	quint8 id() const { return 0xB2; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint16 speed;
};

class OpcodeDIR : public Commande {
public:
	explicit OpcodeDIR(const QByteArray &params);
	quint8 id() const { return 0xB3; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, direction;
};

class OpcodeTURNGEN : public Commande {
public:
	explicit OpcodeTURNGEN(const QByteArray &params);
	quint8 id() const { return 0xB4; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, direction, turnCount, speed, unknown;
};
// note: same struct as TURNGEN
class OpcodeTURN : public Commande {
public:
	explicit OpcodeTURN(const QByteArray &params);
	quint8 id() const { return 0xB5; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, direction, turnCount, speed, unknown;
};
// note: same struct as MOVA
class OpcodeDIRA : public Commande {
public:
	explicit OpcodeDIRA(const QByteArray &params);
	quint8 id() const { return 0xB6; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 groupID;
};

class OpcodeGETDIR : public Commande {
public:
	explicit OpcodeGETDIR(const QByteArray &params);
	quint8 id() const { return 0xB7; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, groupID, varDir;
};

class OpcodeGETAXY : public Commande {
public:
	explicit OpcodeGETAXY(const QByteArray &params);
	quint8 id() const { return 0xB8; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, groupID, varX, varY;
};

class OpcodeGETAI : public Commande {
public:
	explicit OpcodeGETAI(const QByteArray &params);
	quint8 id() const { return 0xB9; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, groupID, varI;
};
// note: same struct as ANIME2
class OpcodeANIMX2 : public Commande {
public:
	explicit OpcodeANIMX2(const QByteArray &params);
	quint8 id() const { return 0xBA; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 animID, speed;
};
// note: same struct as CANIM1
class OpcodeCANIM2 : public Commande {
public:
	explicit OpcodeCANIM2(const QByteArray &params);
	quint8 id() const { return 0xBB; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 animID, firstFrame, lastFrame, speed;
};
// note: same struct as CANIM1
class OpcodeCANMX2 : public Commande {
public:
	explicit OpcodeCANMX2(const QByteArray &params);
	quint8 id() const { return 0xBC; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 animID, firstFrame, lastFrame, speed;
};

class OpcodeASPED : public Commande {
public:
	explicit OpcodeASPED(const QByteArray &params);
	quint8 id() const { return 0xBD; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint16 speed;
};
// note: same struct as MOVA
class OpcodeCC : public Commande {
public:
	explicit OpcodeCC(const QByteArray &params);
	quint8 id() const { return 0xBF; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 groupID;
};

class OpcodeJUMP : public Commande {
public:
	explicit OpcodeJUMP(const QByteArray &params);
	quint8 id() const { return 0xC0; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 targetI;
	qint16 height;
};

class OpcodeAXYZI : public Commande {
public:
	explicit OpcodeAXYZI(const QByteArray &params);
	quint8 id() const { return 0xC1; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2], groupID, varX, varY, varZ, varI;
};

class OpcodeLADER : public Commande {
public:
	explicit OpcodeLADER(const QByteArray &params);
	quint8 id() const { return 0xC2; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	qint16 targetX, targetY, targetZ;
	quint16 targetI;
	quint8 way, animID, direction, speed;
};

class OpcodeOFST : public Commande {
public:
	explicit OpcodeOFST(const QByteArray &params);
	quint8 id() const { return 0xC3; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2], moveType;
	qint16 targetX, targetY, targetZ;
	quint16 speed;
};

class OpcodeOFSTW : public Commande {
public:
	explicit OpcodeOFSTW();
	quint8 id() const { return 0xC4; }
	virtual QString toString() const;
};

class OpcodeTALKR : public Commande {
public:
	explicit OpcodeTALKR(const QByteArray &params);
	quint8 id() const { return 0xC5; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, distance;
};
// note: same struct as TALKR
class OpcodeSLIDR : public Commande {
public:
	explicit OpcodeSLIDR(const QByteArray &params);
	quint8 id() const { return 0xC6; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, distance;
};

class OpcodeSOLID : public Commande {
public:
	explicit OpcodeSOLID(const QByteArray &params);
	quint8 id() const { return 0xC7; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 disabled;
};
// note: same struct as PC
class OpcodePRTYP : public Commande {
public:
	explicit OpcodePRTYP(const QByteArray &params);
	quint8 id() const { return 0xC8; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 charID;
};
// note: same struct as PC
class OpcodePRTYM : public Commande {
public:
	explicit OpcodePRTYM(const QByteArray &params);
	quint8 id() const { return 0xC9; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 charID;
};

class OpcodePRTYE : public Commande {
public:
	explicit OpcodePRTYE(const QByteArray &params);
	quint8 id() const { return 0xCA; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 charID[3];
};

class OpcodeIFPRTYQ : public OpcodeJump {
public:
	explicit OpcodeIFPRTYQ(const QByteArray &params);
	quint8 id() const { return 0xCB; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 charID;
};
// note: same struct as IFPRTYQ
class OpcodeIFMEMBQ : public OpcodeJump {
public:
	explicit OpcodeIFMEMBQ(const QByteArray &params);
	quint8 id() const { return 0xCC; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 charID;
};

class OpcodeMMBUD : public Commande {
public:
	explicit OpcodeMMBUD(const QByteArray &params);
	quint8 id() const { return 0xCD; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 exists, charID;
};
// note: same struct as PC
class OpcodeMMBLK : public Commande {
public:
	explicit OpcodeMMBLK(const QByteArray &params);
	quint8 id() const { return 0xCE; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 charID;
};
// note: same struct as PC
class OpcodeMMBUK : public Commande {
public:
	explicit OpcodeMMBUK(const QByteArray &params);
	quint8 id() const { return 0xCF; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 charID;
};

class OpcodeLINE : public Commande {
public:
	explicit OpcodeLINE(const QByteArray &params);
	quint8 id() const { return 0xD0; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	qint16 targetX1, targetY1, targetZ1;
	qint16 targetX2, targetY2, targetZ2;
};

class OpcodeLINON : public Commande {
public:
	explicit OpcodeLINON(const QByteArray &params);
	quint8 id() const { return 0xD1; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 enabled;
};

class OpcodeMPJPO : public Commande {
public:
	explicit OpcodeMPJPO(const QByteArray &params);
	quint8 id() const { return 0xD2; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 prevent;
};

class OpcodeSLINE : public Commande {
public:
	explicit OpcodeSLINE(const QByteArray &params);
	quint8 id() const { return 0xD3; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[3];
	qint16 targetX1, targetY1, targetZ1;
	qint16 targetX2, targetY2, targetZ2;
};

class OpcodeSIN : public Commande {
public:
	explicit OpcodeSIN(const QByteArray &params);
	quint8 id() const { return 0xD4; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	quint16 value1, value2, value3;
	quint8 var;
};
// note: same struct as SIN
class OpcodeCOS : public Commande {
public:
	explicit OpcodeCOS(const QByteArray &params);
	quint8 id() const { return 0xD5; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[2];
	quint16 value1, value2, value3;
	quint8 var;
};

class OpcodeTLKR2 : public Commande {
public:
	explicit OpcodeTLKR2(const QByteArray &params);
	quint8 id() const { return 0xD6; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint16 distance;
};
// note: same struct as TLKR2
class OpcodeSLDR2 : public Commande {
public:
	explicit OpcodeSLDR2(const QByteArray &params);
	quint8 id() const { return 0xD7; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint16 distance;
};

class OpcodePMJMP : public Commande {
public:
	explicit OpcodePMJMP(const QByteArray &params);
	quint8 id() const { return 0xD8; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint16 fieldID;
};

class OpcodePMJMP2 : public Commande {
public:
	explicit OpcodePMJMP2();
	quint8 id() const { return 0xD9; }
	virtual QString toString() const;
};

class OpcodeAKAO2 : public Commande {
public:
	explicit OpcodeAKAO2(const QByteArray &params);
	quint8 id() const { return 0xDA; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[14];
};

class OpcodeFCFIX : public Commande {
public:
	explicit OpcodeFCFIX(const QByteArray &params);
	quint8 id() const { return 0xDB; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 disabled;
};

class OpcodeCCANM : public Commande {
public:
	explicit OpcodeCCANM(const QByteArray &params);
	quint8 id() const { return 0xDC; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 animID, speed, standWalkRun;
};

class OpcodeANIMB : public Commande {
public:
	explicit OpcodeANIMB();
	quint8 id() const { return 0xDD; }
	virtual QString toString() const;
};

class OpcodeTURNW : public Commande {
public:
	explicit OpcodeTURNW();
	quint8 id() const { return 0xDE; }
	virtual QString toString() const;
};

class OpcodeMPPAL : public Commande {
public:
	explicit OpcodeMPPAL(const QByteArray &params);
	quint8 id() const { return 0xDF; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[10];
};

class OpcodeBGON : public Commande {
public:
	explicit OpcodeBGON(const QByteArray &params);
	quint8 id() const { return 0xE0; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, paramID, stateID;
};
// note: same struct as BGON
class OpcodeBGOFF : public Commande {
public:
	explicit OpcodeBGOFF(const QByteArray &params);
	quint8 id() const { return 0xE1; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, paramID, stateID;
};

class OpcodeBGROL : public Commande {
public:
	explicit OpcodeBGROL(const QByteArray &params);
	quint8 id() const { return 0xE2; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, paramID;
};
// note: same struct as BGROL
class OpcodeBGROL2 : public Commande {
public:
	explicit OpcodeBGROL2(const QByteArray &params);
	quint8 id() const { return 0xE3; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, paramID;
};
// note: same struct as BGROL
class OpcodeBGCLR : public Commande {
public:
	explicit OpcodeBGCLR(const QByteArray &params);
	quint8 id() const { return 0xE4; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, paramID;
};

class OpcodeSTPAL : public Commande {
public:
	explicit OpcodeSTPAL(const QByteArray &params);
	quint8 id() const { return 0xE5; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[4];
};

class OpcodeLDPAL : public Commande {
public:
	explicit OpcodeLDPAL(const QByteArray &params);
	quint8 id() const { return 0xE6; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[4];
};

class OpcodeCPPAL : public Commande {
public:
	explicit OpcodeCPPAL(const QByteArray &params);
	quint8 id() const { return 0xE7; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[4];
};

class OpcodeRTPAL : public Commande {
public:
	explicit OpcodeRTPAL(const QByteArray &params);
	quint8 id() const { return 0xE8; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[6];
};

class OpcodeADPAL : public Commande {
public:
	explicit OpcodeADPAL(const QByteArray &params);
	quint8 id() const { return 0xE9; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[9];
};

class OpcodeMPPAL2 : public Commande {
public:
	explicit OpcodeMPPAL2(const QByteArray &params);
	quint8 id() const { return 0xEA; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks[3], unknown1[2], r, g, b, unknown2;
};

class OpcodeSTPLS : public Commande {
public:
	explicit OpcodeSTPLS(const QByteArray &params);
	quint8 id() const { return 0xEB; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[4];
};

class OpcodeLDPLS : public Commande {
public:
	explicit OpcodeLDPLS(const QByteArray &params);
	quint8 id() const { return 0xEC; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[4];
};

class OpcodeCPPAL2 : public Commande {
public:
	explicit OpcodeCPPAL2(const QByteArray &params);
	quint8 id() const { return 0xED; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[7];
};

class OpcodeRTPAL2 : public Commande {
public:
	explicit OpcodeRTPAL2(const QByteArray &params);
	quint8 id() const { return 0xEE; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[7];
};

class OpcodeADPAL2 : public Commande {
public:
	explicit OpcodeADPAL2(const QByteArray &params);
	quint8 id() const { return 0xEF; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[10];
};

class OpcodeMUSIC : public Commande {
public:
	explicit OpcodeMUSIC(const QByteArray &params);
	quint8 id() const { return 0xF0; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 musicID;
};

class OpcodeSOUND : public Commande {
public:
	explicit OpcodeSOUND(const QByteArray &params);
	quint8 id() const { return 0xF1; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks;
	quint16 soundID;
	quint8 position;
};

class OpcodeAKAO : public Commande {
public:
	explicit OpcodeAKAO(const QByteArray &params);
	quint8 id() const { return 0xF2; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[13];
};
// note: same struct as MUSIC
class OpcodeMUSVT : public Commande {
public:
	explicit OpcodeMUSVT(const QByteArray &params);
	quint8 id() const { return 0xF3; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 musicID;
};
// note: same struct as MUSIC
class OpcodeMUSVM : public Commande {
public:
	explicit OpcodeMUSVM(const QByteArray &params);
	quint8 id() const { return 0xF4; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 musicID;
};

class OpcodeMULCK : public Commande {
public:
	explicit OpcodeMULCK(const QByteArray &params);
	quint8 id() const { return 0xF5; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 locked;
};
// note: same struct as MUSIC
class OpcodeBMUSC : public Commande {
public:
	explicit OpcodeBMUSC(const QByteArray &params);
	quint8 id() const { return 0xF6; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 musicID;
};

class OpcodeCHMPH : public Commande {
public:
	explicit OpcodeCHMPH(const QByteArray &params);
	quint8 id() const { return 0xF7; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[3];
};

class OpcodePMVIE : public Commande {
public:
	explicit OpcodePMVIE(const QByteArray &params);
	quint8 id() const { return 0xF8; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 movieID;
};

class OpcodeMOVIE : public Commande {
public:
	explicit OpcodeMOVIE();
	quint8 id() const { return 0xF9; }
	virtual QString toString() const;
};

class OpcodeMVIEF : public Commande {
public:
	explicit OpcodeMVIEF(const QByteArray &params);
	quint8 id() const { return 0xFA; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, varCurMovieFrame;
};

class OpcodeMVCAM : public Commande {
public:
	explicit OpcodeMVCAM(const QByteArray &params);
	quint8 id() const { return 0xFB; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 movieCamID;
};

class OpcodeFMUSC : public Commande {
public:
	explicit OpcodeFMUSC(const QByteArray &params);
	quint8 id() const { return 0xFC; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown;
};

class OpcodeCMUSC : public Commande {
public:
	explicit OpcodeCMUSC(const QByteArray &params);
	quint8 id() const { return 0xFD; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 unknown[5];
};

class OpcodeCHMST : public Commande {
public:
	explicit OpcodeCHMST(const QByteArray &params);
	quint8 id() const { return 0xFE; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, var;
};

class OpcodeGAMEOVER : public Commande {
public:
	explicit OpcodeGAMEOVER();
	quint8 id() const { return 0xFF; }
	virtual QString toString() const;
};

#endif
