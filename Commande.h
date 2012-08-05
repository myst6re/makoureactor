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
	static QString _materia(const QByteArray &param, quint8 bank);
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
	quint8 banks, windowID, textID, firstLine, lastLine, var;
};

class OpcodeMENU : public Commande {
public:
	explicit OpcodeMENU(const QByteArray &params);
	quint8 id() const { return 0x49; }
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
	quint8 banks[2], corner, var1, var2, var3;
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
	quint8 var;
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
	quint8 unknown, quantity;
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

class OpcodePLUSX : public Commande {
public:
	explicit OpcodePLUSX(const QByteArray &params);
	quint8 id() const { return 0x76; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, var, value;
};

class OpcodePLUS2X : public Commande {
public:
	explicit OpcodePLUS2X(const QByteArray &params);
	quint8 id() const { return 0x77; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, var;
	quint16 value;
};
// note: same struct as PLUSX
class OpcodeMINUSX : public Commande {
public:
	explicit OpcodeMINUSX(const QByteArray &params);
	quint8 id() const { return 0x78; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, var, value;
};
// note: same struct as PLUS2X
class OpcodeMINUS2X : public Commande {
public:
	explicit OpcodeMINUS2X(const QByteArray &params);
	quint8 id() const { return 0x79; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, var;
	quint16 value;
};

class OpcodeINCX : public Commande {
public:
	explicit OpcodeINCX(const QByteArray &params);
	quint8 id() const { return 0x7A; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, var;
};
// note: same struct as INCX
class OpcodeINC2X : public Commande {
public:
	explicit OpcodeINC2X(const QByteArray &params);
	quint8 id() const { return 0x7B; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, var;
};
// note: same struct as INCX
class OpcodeDECX : public Commande {
public:
	explicit OpcodeDECX(const QByteArray &params);
	quint8 id() const { return 0x7C; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, var;
};
// note: same struct as INCX
class OpcodeDEC2X : public Commande {
public:
	explicit OpcodeDECX(const QByteArray &params);
	quint8 id() const { return 0x7D; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, var;
};

class OpcodeTLKON : public Commande {
public:
	explicit OpcodeTLKON(const QByteArray &params);
	quint8 id() const { return 0x7E; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 disabled;
};

class OpcodeRDMSD : public Commande {
public:
	explicit OpcodeRDMSD(const QByteArray &params);
	quint8 id() const { return 0x7F; }
	virtual QString toString() const;
	virtual QByteArray params() const;
	quint8 banks, var;
};

#endif
