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
#ifndef DEF_OPCODE
#define DEF_OPCODE

#include <QtGui>
#include "FF7Text.h"
#include "Var.h"
#include "Data.h"

#define B1(v)		((v>>4)&0xF)
#define B2(v)		(v&0xF)

typedef struct {
	quint16 opcode;
	qint16 x, y;
	quint16 w, h;
	quint16 ask_first, ask_last;
	quint8 type;
	quint16 groupID, scriptID, opcodeID;
} FF7Window;

typedef struct _ff7Var {
	_ff7Var(quint8 b, quint8 a) {
		bank = b;
		adress = a;
	}
	quint8 bank;
	quint8 adress;
} FF7Var;

inline bool operator==(FF7Var v1, FF7Var v2) { return v1.bank == v2.bank && v1.adress == v2.adress; }

class Opcode
{
public:
	enum Keys {
		RET=0, REQ, REQSW, REQEW,
		PREQ, PRQSW, PRQEW, RETTO,
		JOIN, SPLIT, SPTYE, GTPYE,
		Unknown1, Unknown2, DSKCG, SPECIAL,

		JMPF, JMPFL, JMPB, JMPBL,
		IFUB, IFUBL, IFSW, IFSWL,
		IFUW, IFUWL, Unknown3, Unknown4,
		Unknown5, Unknown6, Unknown7, Unknown8,

		MINIGAME, TUTOR, BTMD2, BTRLD,
		WAIT, NFADE, BLINK, BGMOVIE,
		KAWAI, KAWIW, PMOVA, SLIP,
		BGPDH, BGSCR, WCLS, WSIZW,

		IFKEY, IFKEYON, IFKEYOFF, UC,
		PDIRA, PTURA, WSPCL, WNUMB,
		STTIM, GOLDu, GOLDd, CHGLD,
		HMPMAX1, HMPMAX2, MHMMX, HMPMAX3,

		MESSAGE, MPARA, MPRA2, MPNAM,
		Unknown9, MPu, Unknown10, MPd,
		ASK, MENU, MENU2, BTLTB,
		Unknown11, HPu, Unknown12, HPd,

		WINDOW, WMOVE, WMODE, WREST,
		WCLSE, WROW, GWCOL, SWCOL,
		STITM, DLITM, CKITM, SMTRA,
		DMTRA, CMTRA, SHAKE, NOP,

		MAPJUMP, SCRLO, SCRLC, SCRLA,
		SCR2D, SCRCC, SCR2DC, SCRLW,
		SCR2DL, MPDSP, VWOFT, FADE,
		FADEW, IDLCK, LSTMP, SCRLP,

		BATTLE, BTLON, BTLMD, PGTDR,
		GETPC, PXYZI, PLUSX, PLUS2X,
		MINUSX, MINUS2X, INCX, INC2X,
		DECX, DEC2X, TLKON, RDMSD,

		SETBYTE, SETWORD, BITON, BITOFF,
		BITXOR, PLUS, PLUS2, MINUS,
		MINUS2, MUL, MUL2, DIV,
		DIV2, MOD, MOD2, AND,

		AND2, OR, OR2, XOR,
		XOR2, INC, INC2, DEC,
		DEC2, RANDOM, LBYTE, HBYTE,
		TOBYTE, SETX, GETX, SEARCHX,

		PC, CHAR, DFANM, ANIME1,
		VISI, XYZI, XYI, XYZ,
		MOVE, CMOVE, MOVA, TURA,
		ANIMW, FMOVE, ANIME2, ANIMX1,

		CANIM1, CANMX1, MSPED, DIR,
		TURNGEN, TURN, DIRA, GETDIR,
		GETAXY, GETAI, ANIMX2, CANIM2,
		CANMX2, ASPED, Unknown13, CC,

		JUMP, AXYZI, LADER, OFST,
		OFSTW, TALKR, SLIDR, SOLID,
		PRTYP, PRTYM, PRTYE, IFPRTYQ,
		IFMEMBQ, MMBud, MMBLK, MMBUK,

		LINE, LINON, MPJPO, SLINE,
		SIN, COS, TLKR2, SLDR2,
		PMJMP, PMJMP2, AKAO2, FCFIX,
		CCANM, ANIMB, TURNW, MPPAL,

		BGON, BGOFF, BGROL, BGROL2,
		BGCLR, STPAL, LDPAL, CPPAL,
		RTPAL, ADPAL, MPPAL2, STPLS,
		LDPLS, CPPAL2, RTPAL2, ADPAL2,

		MUSIC, SOUND, AKAO, MUSVT,
		MUSVM, MULCK, BMUSC, CHMPH,
		PMVIE, MOVIE, MVIEF, MVCAM,
		FMUSC, CMUSC, CHMST, GAMEOVER,
		LABEL
	};

	Opcode();

	virtual int id() const=0;
	virtual QString toString() const=0;
	virtual void setParams(const QByteArray &params);
	virtual QByteArray params() const;
	virtual quint8 size() const;
	bool hasParams() const;
	virtual const QString &name() const;
	virtual QByteArray toByteArray() const;
	virtual bool isJump() const;
	virtual bool isLabel() const;
	int subParam(int cur, int paramSize) const;

	virtual bool isVoid() const;

	bool searchVar(quint8 bank, quint8 adress, int value=65536) const;

	virtual int getTextID() const;
	virtual void setTextID(quint8 textID);
	virtual int getTutoID() const;
	virtual void setTutoID(quint8 tutoID);
	virtual int getWindowID() const;
	virtual void setWindowID(quint8 windowID);
	virtual bool getWindow(FF7Window &window) const;
	virtual void setWindow(const FF7Window &window);
	virtual void getVariables(QList<FF7Var> &vars) const;
	bool searchExec(quint8 group, quint8 script) const;
	bool searchMapJump(quint16 fieldID) const;
	bool searchTextInScripts(const QRegExp &text) const;
	void listUsedTexts(QSet<quint8> &usedTexts) const;
	void listUsedTuts(QSet<quint8> &usedTuts) const;
	void shiftTextIds(int textId, int steps);
	void shiftTutIds(int tutId, int steps);
	void listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const;
	void getBgParams(QHash<quint8, quint8> &enabledParams) const;
	void getBgMove(qint16 z[2], qint16 *x, qint16 *y) const;

	static QString _personnage(quint8 persoID);

	static const quint8 length[257];
	static const QString names[257];
protected:
	static QString _script(quint8 param);
	static QString _text(quint8 textID);
	static QString _item(quint16 itemID, quint8 bank);
	static QString _materia(quint8 materiaID, quint8 bank);
	static QString _field(quint16 fieldID);
	static QString _movie(quint8 movieID);
	// static QString _objet3D(quint8 objet3D_ID);

	static QString _bank(quint8 adress, quint8 bank);
	static QString _var(int value, quint8 bank);
	static QString _var(int value, quint8 bank1, quint8 bank2);
	static QString _var(int value, quint8 bank1, quint8 bank2, quint8 bank3);

	static QString _operateur(quint8 param);
	static QString _windowCorner(quint8 param, quint8 bank);
	static QString _sensRotation(quint8 param);
};

class OpcodeUnknown : public Opcode {
public:
	explicit OpcodeUnknown(quint8 id, const QByteArray &params=QByteArray());
	int id() const;
	quint8 size() const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 _id;
	QByteArray unknown;
};

class OpcodeRET : public Opcode {
public:
	explicit OpcodeRET();
	int id() const { return 0x00; }
	QString toString() const;
	bool isVoid() const { return true; }
};

class OpcodeExec : public Opcode {
public:
	explicit OpcodeExec(const QByteArray &params);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 groupID;
	quint8 scriptID;
	quint8 priority;
};

class OpcodeREQ : public OpcodeExec {
public:
	explicit OpcodeREQ(const QByteArray &params);
	explicit OpcodeREQ(const OpcodeExec &op);
	int id() const { return 0x01; }
	QString toString() const;
};

class OpcodeREQSW : public OpcodeExec {
public:
	explicit OpcodeREQSW(const QByteArray &params);
	explicit OpcodeREQSW(const OpcodeExec &op);
	int id() const { return 0x02; }
	QString toString() const;
};

class OpcodeREQEW : public OpcodeExec {
public:
	explicit OpcodeREQEW(const QByteArray &params);
	explicit OpcodeREQEW(const OpcodeExec &op);
	int id() const { return 0x03; }
	QString toString() const;
};

class OpcodeExecChar : public Opcode {
public:
	explicit OpcodeExecChar(const QByteArray &params);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 partyID;
	quint8 scriptID;
	quint8 priority;
};

class OpcodePREQ : public OpcodeExecChar {
public:
	explicit OpcodePREQ(const QByteArray &params);
	explicit OpcodePREQ(const OpcodeExecChar &op);
	int id() const { return 0x04; }
	QString toString() const;
};

class OpcodePRQSW : public OpcodeExecChar {
public:
	explicit OpcodePRQSW(const QByteArray &params);
	explicit OpcodePRQSW(const OpcodeExecChar &op);
	int id() const { return 0x05; }
	QString toString() const;
};

class OpcodePRQEW : public OpcodeExecChar {
public:
	explicit OpcodePRQEW(const QByteArray &params);
	explicit OpcodePRQEW(const OpcodeExecChar &op);
	int id() const { return 0x06; }
	QString toString() const;
};

class OpcodeRETTO : public Opcode {
public:
	explicit OpcodeRETTO(const QByteArray &params);
	int id() const { return 0x07; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 scriptID;
	quint8 priority;
};

class OpcodeJOIN : public Opcode {
public:
	explicit OpcodeJOIN(const QByteArray &params);
	int id() const { return 0x08; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 speed;
};

class OpcodeSPLIT : public Opcode {
public:
	explicit OpcodeSPLIT(const QByteArray &params);
	int id() const { return 0x09; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[3];
	qint16 targetX1, targetY1, targetX2, targetY2;
	quint8 direction1, direction2;
	quint8 speed;
};

class OpcodePartyE : public Opcode {
public:
	explicit OpcodePartyE(const QByteArray &params);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	quint8 party1, party2, party3;
};

class OpcodeSPTYE : public OpcodePartyE {
public:
	explicit OpcodeSPTYE(const QByteArray &params);
	explicit OpcodeSPTYE(const OpcodePartyE &op);
	int id() const { return 0x0A; }
	QString toString() const;
};

class OpcodeGTPYE : public OpcodePartyE {
public:
	explicit OpcodeGTPYE(const QByteArray &params);
	explicit OpcodeGTPYE(const OpcodePartyE &op);
	int id() const { return 0x0B; }
	QString toString() const;
};

class OpcodeDSKCG : public Opcode {
public:
	explicit OpcodeDSKCG(const QByteArray &params);
	int id() const { return 0x0E; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 diskID;
};

class OpcodeSPECIALARROW : public Opcode {
public:
	explicit OpcodeSPECIALARROW(const QByteArray &params);
	int id() const { return 0xF5; }
	quint8 size() const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 hide;
};

class OpcodeSPECIALPNAME : public Opcode {
public:
	explicit OpcodeSPECIALPNAME(const QByteArray &params);
	int id() const { return 0xF6; }
	quint8 size() const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown;
};

class OpcodeSPECIALGMSPD : public Opcode {
public:
	explicit OpcodeSPECIALGMSPD(const QByteArray &params);
	int id() const { return 0xF7; }
	quint8 size() const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 speed;
};

class OpcodeSPECIALSMSPD : public Opcode {
public:
	explicit OpcodeSPECIALSMSPD(const QByteArray &params);
	int id() const { return 0xF8; }
	quint8 size() const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown, speed;
};

class OpcodeSPECIALFLMAT : public Opcode {
public:
	explicit OpcodeSPECIALFLMAT();
	int id() const { return 0xF9; }
	quint8 size() const;
	QString toString() const;
};

class OpcodeSPECIALFLITM : public Opcode {
public:
	explicit OpcodeSPECIALFLITM();
	int id() const { return 0xFA; }
	quint8 size() const;
	QString toString() const;
};

class OpcodeSPECIALBTLCK : public Opcode {
public:
	explicit OpcodeSPECIALBTLCK(const QByteArray &params);
	int id() const { return 0xFB; }
	quint8 size() const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 lock;
};

class OpcodeSPECIALMVLCK : public Opcode {
public:
	explicit OpcodeSPECIALMVLCK(const QByteArray &params);
	int id() const { return 0xFC; }
	quint8 size() const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 lock;
};

class OpcodeSPECIALSPCNM : public Opcode {
public:
	explicit OpcodeSPECIALSPCNM(const QByteArray &params);
	int id() const { return 0xFD; }
	quint8 size() const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getTextID() const;
	void setTextID(quint8 textID);
	quint8 charID, textID;
};

class OpcodeSPECIALRSGLB : public Opcode {
public:
	explicit OpcodeSPECIALRSGLB();
	int id() const { return 0xFE; }
	quint8 size() const;
	QString toString() const;
};

class OpcodeSPECIALCLITM : public Opcode {
public:
	explicit OpcodeSPECIALCLITM();
	int id() const { return 0xFF; }
	quint8 size() const;
	QString toString() const;
};

class OpcodeSPECIAL : public Opcode {
public:
	explicit OpcodeSPECIAL(const QByteArray &params);
	virtual ~OpcodeSPECIAL();
	int id() const { return 0x0F; }
	quint8 size() const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getTextID() const;
	void setTextID(quint8 textID);
	Opcode *opcode;
};

class OpcodeJump : public Opcode {
public:
	explicit OpcodeJump();
	qint32 jump() const;
	void setJump(qint32 jump);
	quint32 label() const;
	void setLabel(quint32 label);
	bool isJump() const;
	virtual bool isLongJump() const=0;
	virtual quint8 jumpPosData() const=0;
protected:
	qint32 _jump;
	quint32 _label;
};

class OpcodeLabel : public Opcode {
public:
	explicit OpcodeLabel(quint32 label);
	int id() const { return 0x100; } // fake id
	QByteArray toByteArray() const { return QByteArray(); }
	QString toString() const;
	bool isLabel() const;
	bool isVoid() const { return true; }
	quint32 label() const;
	void setLabel(quint32 label);
	quint32 _label;
};

class OpcodeJMPF : public OpcodeJump {
public:
	explicit OpcodeJMPF(const QByteArray &params);
	explicit OpcodeJMPF(const OpcodeJump &op);
	int id() const { return 0x10; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	bool isVoid() const { return true; }
	bool isLongJump() const { return false; }
	quint8 jumpPosData() const { return 1; }
};

class OpcodeJMPFL : public OpcodeJump {
public:
	explicit OpcodeJMPFL(const QByteArray &params);
	explicit OpcodeJMPFL(const OpcodeJump &op);
	int id() const { return 0x11; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	bool isVoid() const { return true; }
	bool isLongJump() const { return true; }
	quint8 jumpPosData() const { return 1; }
};

class OpcodeJMPB : public OpcodeJump {
public:
	explicit OpcodeJMPB(const QByteArray &params);
	explicit OpcodeJMPB(const OpcodeJump &op);
	int id() const { return 0x12; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	bool isVoid() const { return true; }
	bool isLongJump() const { return false; }
	quint8 jumpPosData() const { return 0; }
};

class OpcodeJMPBL : public OpcodeJump {
public:
	explicit OpcodeJMPBL(const QByteArray &params);
	explicit OpcodeJMPBL(const OpcodeJump &op);
	int id() const { return 0x13; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	bool isVoid() const { return true; }
	bool isLongJump() const { return true; }
	quint8 jumpPosData() const { return 0; }
};

class OpcodeIf : public OpcodeJump {
public:
	explicit OpcodeIf();
	explicit OpcodeIf(const OpcodeJump &op);
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	qint32 value1, value2;
	quint8 oper;
};

class OpcodeIFUB : public OpcodeIf {
public:
	explicit OpcodeIFUB(const QByteArray &params);
	explicit OpcodeIFUB(const OpcodeIf &op);
	int id() const { return 0x14; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	bool isLongJump() const { return false; }
	quint8 jumpPosData() const { return 5; }
};

class OpcodeIFUBL : public OpcodeIf {
public:
	explicit OpcodeIFUBL(const QByteArray &params);
	explicit OpcodeIFUBL(const OpcodeIf &op);
	int id() const { return 0x15; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	bool isLongJump() const { return true; }
	quint8 jumpPosData() const { return 5; }
};

class OpcodeIFSW : public OpcodeIf {
public:
	explicit OpcodeIFSW(const QByteArray &params);
	explicit OpcodeIFSW(const OpcodeIf &op);
	int id() const { return 0x16; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	bool isLongJump() const { return false; }
	quint8 jumpPosData() const { return 7; }
};

class OpcodeIFSWL : public OpcodeIf {
public:
	explicit OpcodeIFSWL(const QByteArray &params);
	explicit OpcodeIFSWL(const OpcodeIf &op);
	int id() const { return 0x17; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	bool isLongJump() const { return true; }
	quint8 jumpPosData() const { return 7; }
};

class OpcodeIFUW : public OpcodeIf {
public:
	explicit OpcodeIFUW(const QByteArray &params);
	explicit OpcodeIFUW(const OpcodeIf &op);
	int id() const { return 0x18; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	bool isLongJump() const { return false; }
	quint8 jumpPosData() const { return 7; }
};

class OpcodeIFUWL : public OpcodeIf {
public:
	explicit OpcodeIFUWL(const QByteArray &params);
	explicit OpcodeIFUWL(const OpcodeIf &op);
	int id() const { return 0x19; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	bool isLongJump() const { return true; }
	quint8 jumpPosData() const { return 7; }
};

class OpcodeMINIGAME : public Opcode {
public:
	explicit OpcodeMINIGAME(const QByteArray &params);
	int id() const { return 0x20; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint16 fieldID;
	qint16 targetX;
	qint16 targetY;
	quint16 targetI;
	quint8 minigameParam;
	quint8 minigameID;
};

class OpcodeTUTOR : public Opcode {
public:
	explicit OpcodeTUTOR(const QByteArray &params);
	int id() const { return 0x21; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getTutoID() const;
	void setTutoID(quint8 tutoID);
	quint8 tutoID;
};

class OpcodeBTMD2 : public Opcode {
public:
	explicit OpcodeBTMD2(const QByteArray &params);
	int id() const { return 0x22; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint32 battleMode;
};
//note: same struct as UnaryOperation
class OpcodeBTRLD : public Opcode {
public:
	explicit OpcodeBTRLD(const QByteArray &params);
	int id() const { return 0x23; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, var;
};

class OpcodeWAIT : public Opcode {
public:
	explicit OpcodeWAIT(const QByteArray &params);
	int id() const { return 0x24; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint16 frameCount;
};

class OpcodeNFADE : public Opcode {
public:
	explicit OpcodeNFADE(const QByteArray &params);
	int id() const { return 0x25; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	quint8 r, g, b;
	quint8 unknown1, unknown2, unknown3;
};

class OpcodeBLINK : public Opcode {
public:
	explicit OpcodeBLINK(const QByteArray &params);
	int id() const { return 0x26; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 closed;
};

class OpcodeBGMOVIE : public Opcode {
public:
	explicit OpcodeBGMOVIE(const QByteArray &params);
	int id() const { return 0x27; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 disabled;
};

class OpcodeKAWAIEYETX : public OpcodeUnknown {
public:
	explicit OpcodeKAWAIEYETX(const QByteArray &params);
	QString toString() const;
};

class OpcodeKAWAITRNSP : public Opcode {
public:
	explicit OpcodeKAWAITRNSP(const QByteArray &params);
	int id() const { return 0x01; }
	quint8 size() const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 enableTransparency;
	QByteArray data;
};

class OpcodeKAWAIAMBNT : public OpcodeUnknown {
public:
	explicit OpcodeKAWAIAMBNT(const QByteArray &params);
	QString toString() const;
};

class OpcodeKAWAILIGHT : public OpcodeUnknown {
public:
	explicit OpcodeKAWAILIGHT(const QByteArray &params);
	QString toString() const;
};

class OpcodeKAWAISBOBJ : public OpcodeUnknown {
public:
	explicit OpcodeKAWAISBOBJ(const QByteArray &params);
	QString toString() const;
};

class OpcodeKAWAISHINE : public OpcodeUnknown {
public:
	explicit OpcodeKAWAISHINE(const QByteArray &params);
	QString toString() const;
};

class OpcodeKAWAIRESET : public OpcodeUnknown {
public:
	explicit OpcodeKAWAIRESET(const QByteArray &params);
	QString toString() const;
};

class OpcodeKAWAI : public Opcode {
public:
	explicit OpcodeKAWAI(const QByteArray &params);
	virtual ~OpcodeKAWAI();
	int id() const { return 0x28; }
	quint8 size() const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	Opcode *opcode;
};

class OpcodeKAWIW : public Opcode {
public:
	explicit OpcodeKAWIW();
	int id() const { return 0x29; }
	QString toString() const;
};

class OpcodePMOVA : public Opcode {
public:
	explicit OpcodePMOVA(const QByteArray &params);
	int id() const { return 0x2A; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 partyID;
};

class OpcodeSLIP : public Opcode {
public:
	explicit OpcodeSLIP(const QByteArray &params);
	int id() const { return 0x2B; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 off;
};

class OpcodeBGPDH : public Opcode {
public:
	explicit OpcodeBGPDH(const QByteArray &params);
	int id() const { return 0x2C; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint8 layerID;
	qint16 targetZ;
};

class OpcodeBGSCR : public Opcode {
public:
	explicit OpcodeBGSCR(const QByteArray &params);
	int id() const { return 0x2D; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint8 layerID;
	qint16 targetX, targetY;
};

class OpcodeWCLS : public Opcode {
public:
	explicit OpcodeWCLS(const QByteArray &params);
	int id() const { return 0x2E; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	quint8 windowID;
};

class OpcodeWindow : public Opcode {
public:
	explicit OpcodeWindow(const QByteArray &params);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	bool getWindow(FF7Window &window) const;
	void setWindow(const FF7Window &window);
	quint8 windowID;
	quint16 targetX, targetY;
	quint16 width, height;
};

class OpcodeWSIZW : public OpcodeWindow {
public:
	explicit OpcodeWSIZW(const QByteArray &params);
	explicit OpcodeWSIZW(const OpcodeWindow &op);
	int id() const { return 0x2F; }
	QString toString() const;
};

class OpcodeIfKey : public OpcodeJump {
public:
	explicit OpcodeIfKey(const QByteArray &params);
	explicit OpcodeIfKey(const OpcodeJump &op);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	QString keyString() const;
	bool isLongJump() const { return false; }
	quint8 jumpPosData() const { return 3; }
	quint16 keys;
};

class OpcodeIFKEY : public OpcodeIfKey {
public:
	explicit OpcodeIFKEY(const QByteArray &params);
	explicit OpcodeIFKEY(const OpcodeIfKey &op);
	int id() const { return 0x30; }
	QString toString() const;
};

class OpcodeIFKEYON : public OpcodeIfKey {
public:
	explicit OpcodeIFKEYON(const QByteArray &params);
	explicit OpcodeIFKEYON(const OpcodeIfKey &op);
	int id() const { return 0x31; }
	QString toString() const;
};

class OpcodeIFKEYOFF : public OpcodeIfKey {
public:
	explicit OpcodeIFKEYOFF(const QByteArray &params);
	explicit OpcodeIFKEYOFF(const OpcodeIfKey &op);
	int id() const { return 0x32; }
	QString toString() const;
};

class OpcodeUC : public Opcode {
public:
	explicit OpcodeUC(const QByteArray &params);
	int id() const { return 0x33; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 disabled;
};

class OpcodePDIRA : public Opcode {
public:
	explicit OpcodePDIRA(const QByteArray &params);
	int id() const { return 0x34; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 partyID;
};

class OpcodePTURA : public Opcode {
public:
	explicit OpcodePTURA(const QByteArray &params);
	int id() const { return 0x35; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 partyID;
	quint8 speed;
	quint8 directionRotation;
};

class OpcodeWSPCL : public Opcode {
public:
	explicit OpcodeWSPCL(const QByteArray &params);
	int id() const { return 0x36; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	quint8 windowID;
	quint8 displayType;
	quint8 marginLeft, marginTop;
};

class OpcodeWNUMB : public Opcode {
public:
	explicit OpcodeWNUMB(const QByteArray &params);
	int id() const { return 0x37; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint8 windowID;
	qint32 value;
	quint8 digitCount;
};

class OpcodeSTTIM : public Opcode {
public:
	explicit OpcodeSTTIM(const QByteArray &params);
	int id() const { return 0x38; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	quint8 h, m, s;
};

class OpcodeGOLD : public Opcode {
public:
	explicit OpcodeGOLD(const QByteArray &params);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	qint32 value;
};

class OpcodeGOLDu : public OpcodeGOLD {
public:
	explicit OpcodeGOLDu(const QByteArray &params);
	explicit OpcodeGOLDu(const OpcodeGOLD &op);
	int id() const { return 0x39; }
	QString toString() const;
};

class OpcodeGOLDd : public OpcodeGOLD {
public:
	explicit OpcodeGOLDd(const QByteArray &params);
	explicit OpcodeGOLDd(const OpcodeGOLD &op);
	int id() const { return 0x3A; }
	QString toString() const;
};

class OpcodeCHGLD : public Opcode {
public:
	explicit OpcodeCHGLD(const QByteArray &params);
	int id() const { return 0x3B; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint8 var1, var2;
};

class OpcodeHMPMAX1 : public Opcode {
public:
	explicit OpcodeHMPMAX1();
	int id() const { return 0x3C; }
	QString toString() const;
};

class OpcodeHMPMAX2 : public Opcode {
public:
	explicit OpcodeHMPMAX2();
	int id() const { return 0x3D; }
	QString toString() const;
};

class OpcodeMHMMX : public Opcode {
public:
	explicit OpcodeMHMMX();
	int id() const { return 0x3E; }
	QString toString() const;
};

class OpcodeHMPMAX3 : public Opcode {
public:
	explicit OpcodeHMPMAX3();
	int id() const { return 0x3F; }
	QString toString() const;
};

class OpcodeMESSAGE : public Opcode {
public:
	explicit OpcodeMESSAGE(const QByteArray &params);
	int id() const { return 0x40; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getTextID() const;
	void setTextID(quint8 textID);
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	quint8 windowID, textID;
};

class OpcodeMPARA : public Opcode {
public:
	explicit OpcodeMPARA(const QByteArray &params);
	int id() const { return 0x41; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, windowID, windowVarID, value;
};

class OpcodeMPRA2 : public Opcode {
public:
	explicit OpcodeMPRA2(const QByteArray &params);
	int id() const { return 0x42; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, windowID, windowVarID;
	quint16 value;
};

class OpcodeMPNAM : public Opcode {
public:
	explicit OpcodeMPNAM(const QByteArray &params);
	int id() const { return 0x43; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getTextID() const;
	void setTextID(quint8 textID);
	quint8 textID;
};

class OpcodeHPMP : public Opcode {
public:
	explicit OpcodeHPMP(const QByteArray &params);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, partyID;
	quint16 value;
};

class OpcodeMPu : public OpcodeHPMP {
public:
	explicit OpcodeMPu(const QByteArray &params);
	explicit OpcodeMPu(const OpcodeHPMP &op);
	int id() const { return 0x45; }
	QString toString() const;
};

class OpcodeMPd : public OpcodeHPMP {
public:
	explicit OpcodeMPd(const QByteArray &params);
	explicit OpcodeMPd(const OpcodeHPMP &op);
	int id() const { return 0x47; }
	QString toString() const;
};

class OpcodeASK : public Opcode {
public:
	explicit OpcodeASK(const QByteArray &params);
	int id() const { return 0x48; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getTextID() const;
	void setTextID(quint8 textID);
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, windowID, textID, firstLine, lastLine, varAnswer;
};

class OpcodeMENU : public Opcode {
public:
	explicit OpcodeMENU(const QByteArray &params);
	int id() const { return 0x49; }
	QString menu(const QString &param) const;
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, menuID, param;
};

class OpcodeMENU2 : public Opcode {
public:
	explicit OpcodeMENU2(const QByteArray &params);
	int id() const { return 0x4A; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 disabled;
};

class OpcodeBTLTB : public Opcode {
public:
	explicit OpcodeBTLTB(const QByteArray &params);
	int id() const { return 0x4B; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 battleTableID;
};

class OpcodeHPu : public OpcodeHPMP {
public:
	explicit OpcodeHPu(const QByteArray &params);
	explicit OpcodeHPu(const OpcodeHPMP &params);
	int id() const { return 0x4D; }
	QString toString() const;
};

class OpcodeHPd : public OpcodeHPMP {
public:
	explicit OpcodeHPd(const QByteArray &params);
	explicit OpcodeHPd(const OpcodeHPMP &params);
	int id() const { return 0x4F; }
	QString toString() const;
};

class OpcodeWINDOW : public OpcodeWindow {
public:
	explicit OpcodeWINDOW(const QByteArray &params);
	explicit OpcodeWINDOW(const OpcodeWindow &op);
	int id() const { return 0x50; }
	QString toString() const;
};

class OpcodeWMOVE : public Opcode {
public:
	explicit OpcodeWMOVE(const QByteArray &params);
	int id() const { return 0x51; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	bool getWindow(FF7Window &window) const;
	void setWindow(const FF7Window &window);
	quint8 windowID;
	quint16 targetX, targetY;
};

class OpcodeWMODE : public Opcode {
public:
	explicit OpcodeWMODE(const QByteArray &params);
	int id() const { return 0x52; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	quint8 windowID, mode;
	quint8 preventClose;
};

class OpcodeWREST : public Opcode {
public:
	explicit OpcodeWREST(const QByteArray &params);
	int id() const { return 0x53; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	quint8 windowID;
};
// note: same struct as WREST
class OpcodeWCLSE : public Opcode {
public:
	explicit OpcodeWCLSE(const QByteArray &params);
	int id() const { return 0x54; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	quint8 windowID;
};

class OpcodeWROW : public Opcode {
public:
	explicit OpcodeWROW(const QByteArray &params);
	int id() const { return 0x55; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	int getWindowID() const;
	void setWindowID(quint8 windowID);
	quint8 windowID, rowCount;
};

class OpcodeWCOL : public Opcode {
public:
	explicit OpcodeWCOL(const QByteArray &params);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2], corner, r, g, b;
};

class OpcodeGWCOL : public OpcodeWCOL {
public:
	explicit OpcodeGWCOL(const QByteArray &params);
	explicit OpcodeGWCOL(const OpcodeWCOL &op);
	int id() const { return 0x56; }
	QString toString() const;
};

class OpcodeSWCOL : public OpcodeWCOL {
public:
	explicit OpcodeSWCOL(const QByteArray &params);
	explicit OpcodeSWCOL(const OpcodeWCOL &op);
	int id() const { return 0x57; }
	QString toString() const;
};

class OpcodeItem : public Opcode {
public:
	explicit OpcodeItem(const QByteArray &params);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint16 itemID;
	quint8 quantity;
};

class OpcodeSTITM : public OpcodeItem {
public:
	explicit OpcodeSTITM(const QByteArray &params);
	explicit OpcodeSTITM(const OpcodeItem &op);
	int id() const { return 0x58; }
	QString toString() const;
};

class OpcodeDLITM : public OpcodeItem {
public:
	explicit OpcodeDLITM(const QByteArray &params);
	explicit OpcodeDLITM(const OpcodeItem &op);
	int id() const { return 0x59; }
	QString toString() const;
};

class OpcodeCKITM : public OpcodeItem {
public:
	explicit OpcodeCKITM(const QByteArray &params);
	explicit OpcodeCKITM(const OpcodeItem &op);
	int id() const { return 0x5A; }
	QString toString() const;
};

class OpcodeSMTRA : public Opcode {
public:
	explicit OpcodeSMTRA(const QByteArray &params);
	int id() const { return 0x5B; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	quint8 materiaID;
	quint32 APCount;
};

class OpcodeDMTRA : public Opcode {
public:
	explicit OpcodeDMTRA(const QByteArray &params);
	int id() const { return 0x5C; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	quint8 materiaID;
	quint32 APCount;
	quint8 quantity;
};

class OpcodeCMTRA : public Opcode {
public:
	explicit OpcodeCMTRA(const QByteArray &params);
	int id() const { return 0x5D; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[3];
	quint8 materiaID;
	quint32 APCount;
	quint8 unknown, varQuantity;
};

class OpcodeSHAKE : public Opcode {
public:
	explicit OpcodeSHAKE(const QByteArray &params);
	int id() const { return 0x5E; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown1, unknown2, shakeCount, unknown3, unknown4;
	quint8 amplitude, speed;
};

class OpcodeNOP : public Opcode {
public:
	explicit OpcodeNOP();
	int id() const { return 0x5F; }
	QString toString() const;
};

class OpcodeMAPJUMP : public Opcode {
public:
	explicit OpcodeMAPJUMP(const QByteArray &params);
	int id() const { return 0x60; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint16 fieldID;
	qint16 targetX, targetY;
	quint16 targetI;
	quint8 direction;
};

class OpcodeSCRLO : public Opcode {
public:
	explicit OpcodeSCRLO(const QByteArray &params);
	int id() const { return 0x61; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown;
};

class OpcodeSCRLC : public Opcode {
public:
	explicit OpcodeSCRLC(const QByteArray &params);
	int id() const { return 0x62; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint32 unknown;
};

class OpcodeSCRLA : public Opcode {
public:
	explicit OpcodeSCRLA(const QByteArray &params);
	int id() const { return 0x63; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint16 speed;
	quint8 groupID, scrollType;
};

class OpcodeSCR2D : public Opcode {
public:
	explicit OpcodeSCR2D(const QByteArray &params);
	int id() const { return 0x64; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	qint16 targetX, targetY;
};

class OpcodeSCRCC : public Opcode {
public:
	explicit OpcodeSCRCC();
	int id() const { return 0x65; }
	QString toString() const;
};

class OpcodeSCR2DC : public Opcode {
public:
	explicit OpcodeSCR2DC(const QByteArray &params);
	int id() const { return 0x66; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 speed;
};

class OpcodeSCRLW : public Opcode {
public:
	explicit OpcodeSCRLW();
	int id() const { return 0x67; }
	QString toString() const;
};
// note: same struct as SCR2DC
class OpcodeSCR2DL : public Opcode {
public:
	explicit OpcodeSCR2DL(const QByteArray &params);
	int id() const { return 0x68; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 speed;
};

class OpcodeMPDSP : public Opcode {
public:
	explicit OpcodeMPDSP(const QByteArray &params);
	int id() const { return 0x69; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown;
};

class OpcodeVWOFT : public Opcode {
public:
	explicit OpcodeVWOFT(const QByteArray &params);
	int id() const { return 0x6A; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	qint16 unknown1, unknown2;
	quint8 unknown3;
};

class OpcodeFADE : public Opcode {
public:
	explicit OpcodeFADE(const QByteArray &params);
	int id() const { return 0x6B; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	quint8 r, g, b, speed, fadeType, adjust;
};

class OpcodeFADEW : public Opcode {
public:
	explicit OpcodeFADEW();
	int id() const { return 0x6C; }
	QString toString() const;
};

class OpcodeIDLCK : public Opcode {
public:
	explicit OpcodeIDLCK(const QByteArray &params);
	int id() const { return 0x6D; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint16 triangleID;
	quint8 locked;
};
//note: same struct as UnaryOperation
class OpcodeLSTMP : public Opcode {
public:
	explicit OpcodeLSTMP(const QByteArray &params);
	int id() const { return 0x6E; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, var;
};

class OpcodeSCRLP : public Opcode {
public:
	explicit OpcodeSCRLP(const QByteArray &params);
	int id() const { return 0x6F; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint16 speed;
	quint8 partyID, scrollType;
};

class OpcodeBATTLE : public Opcode {
public:
	explicit OpcodeBATTLE(const QByteArray &params);
	int id() const { return 0x70; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint16 battleID;
};

class OpcodeBTLON : public Opcode {
public:
	explicit OpcodeBTLON(const QByteArray &params);
	int id() const { return 0x71; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 disabled;
};

class OpcodeBTLMD : public Opcode {
public:
	explicit OpcodeBTLMD(const QByteArray &params);
	int id() const { return 0x72; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint16 unknown;
};

class OpcodePGTDR : public Opcode {
public:
	explicit OpcodePGTDR(const QByteArray &params);
	int id() const { return 0x73; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, partyID, varDir;
};
// note: same struct as PGTDR
class OpcodeGETPC : public Opcode {
public:
	explicit OpcodeGETPC(const QByteArray &params);
	int id() const { return 0x74; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, partyID, varPC;
};

class OpcodePXYZI : public Opcode {
public:
	explicit OpcodePXYZI(const QByteArray &params);
	int id() const { return 0x75; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2], partyID, varX, varY, varZ, varI;
};

class OpcodeBinaryOperation : public Opcode {
public:
	explicit OpcodeBinaryOperation();
	virtual bool isLong() const=0;
	quint8 banks, var;
	quint16 value;
};

class OpcodeOperation : public OpcodeBinaryOperation {
public:
	explicit OpcodeOperation(const QByteArray &params);
	explicit OpcodeOperation(const OpcodeBinaryOperation &op);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	bool isLong() const { return false; }
};

class OpcodeOperation2 : public OpcodeBinaryOperation {
public:
	explicit OpcodeOperation2(const QByteArray &params);
	explicit OpcodeOperation2(const OpcodeBinaryOperation &op);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	bool isLong() const { return true; }
};

class OpcodeUnaryOperation : public Opcode {
public:
	explicit OpcodeUnaryOperation(const QByteArray &params);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	virtual bool isLong() const=0;
	quint8 banks, var;
};

class OpcodePLUSX : public OpcodeOperation {
public:
	explicit OpcodePLUSX(const QByteArray &params);
	explicit OpcodePLUSX(const OpcodeBinaryOperation &op);
	int id() const { return 0x76; }
	QString toString() const;
};

class OpcodePLUS2X : public OpcodeOperation2 {
public:
	explicit OpcodePLUS2X(const QByteArray &params);
	explicit OpcodePLUS2X(const OpcodeBinaryOperation &op);
	int id() const { return 0x77; }
	QString toString() const;
};

class OpcodeMINUSX : public OpcodeOperation {
public:
	explicit OpcodeMINUSX(const QByteArray &params);
	explicit OpcodeMINUSX(const OpcodeBinaryOperation &op);
	int id() const { return 0x78; }
	QString toString() const;
};

class OpcodeMINUS2X : public OpcodeOperation2 {
public:
	explicit OpcodeMINUS2X(const QByteArray &params);
	explicit OpcodeMINUS2X(const OpcodeBinaryOperation &op);
	int id() const { return 0x79; }
	QString toString() const;
};

class OpcodeINCX : public OpcodeUnaryOperation {
public:
	explicit OpcodeINCX(const QByteArray &params);
	explicit OpcodeINCX(const OpcodeUnaryOperation &op);
	int id() const { return 0x7A; }
	QString toString() const;
	bool isLong() const { return false; }
};

class OpcodeINC2X : public OpcodeUnaryOperation {
public:
	explicit OpcodeINC2X(const QByteArray &params);
	explicit OpcodeINC2X(const OpcodeUnaryOperation &op);
	int id() const { return 0x7B; }
	QString toString() const;
	bool isLong() const { return true; }
};

class OpcodeDECX : public OpcodeUnaryOperation {
public:
	explicit OpcodeDECX(const QByteArray &params);
	explicit OpcodeDECX(const OpcodeUnaryOperation &op);
	int id() const { return 0x7C; }
	QString toString() const;
	bool isLong() const { return false; }
};

class OpcodeDEC2X : public OpcodeUnaryOperation {
public:
	explicit OpcodeDEC2X(const QByteArray &params);
	explicit OpcodeDEC2X(const OpcodeUnaryOperation &op);
	int id() const { return 0x7D; }
	QString toString() const;
	bool isLong() const { return true; }
};

class OpcodeTLKON : public Opcode {
public:
	explicit OpcodeTLKON(const QByteArray &params);
	int id() const { return 0x7E; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 disabled;
};

class OpcodeRDMSD : public Opcode {
public:
	explicit OpcodeRDMSD(const QByteArray &params);
	int id() const { return 0x7F; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 banks, value;
};

class OpcodeSETBYTE : public OpcodeOperation {
public:
	explicit OpcodeSETBYTE(const QByteArray &params);
	explicit OpcodeSETBYTE(const OpcodeBinaryOperation &op);
	int id() const { return 0x80; }
	QString toString() const;
};

class OpcodeSETWORD : public OpcodeOperation2 {
public:
	explicit OpcodeSETWORD(const QByteArray &params);
	explicit OpcodeSETWORD(const OpcodeBinaryOperation &op);
	int id() const { return 0x81; }
	QString toString() const;
};

class OpcodeBitOperation : public Opcode {
public:
	explicit OpcodeBitOperation(const QByteArray &params);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, var, position;
};

class OpcodeBITON : public OpcodeBitOperation {
public:
	explicit OpcodeBITON(const QByteArray &params);
	explicit OpcodeBITON(const OpcodeBitOperation &op);
	int id() const { return 0x82; }
	QString toString() const;
};

class OpcodeBITOFF : public OpcodeBitOperation {
public:
	explicit OpcodeBITOFF(const QByteArray &params);
	explicit OpcodeBITOFF(const OpcodeBitOperation &op);
	int id() const { return 0x83; }
	QString toString() const;
};

class OpcodeBITXOR : public OpcodeBitOperation {
public:
	explicit OpcodeBITXOR(const QByteArray &params);
	explicit OpcodeBITXOR(const OpcodeBitOperation &op);
	int id() const { return 0x84; }
	QString toString() const;
};

class OpcodePLUS : public OpcodeOperation {
public:
	explicit OpcodePLUS(const QByteArray &params);
	explicit OpcodePLUS(const OpcodeBinaryOperation &op);
	int id() const { return 0x85; }
	QString toString() const;
};

class OpcodePLUS2 : public OpcodeOperation2 {
public:
	explicit OpcodePLUS2(const QByteArray &params);
	explicit OpcodePLUS2(const OpcodeBinaryOperation &op);
	int id() const { return 0x86; }
	QString toString() const;
};

class OpcodeMINUS : public OpcodeOperation {
public:
	explicit OpcodeMINUS(const QByteArray &params);
	explicit OpcodeMINUS(const OpcodeBinaryOperation &op);
	int id() const { return 0x87; }
	QString toString() const;
};

class OpcodeMINUS2 : public OpcodeOperation2 {
public:
	explicit OpcodeMINUS2(const QByteArray &params);
	explicit OpcodeMINUS2(const OpcodeBinaryOperation &op);
	int id() const { return 0x88; }
	QString toString() const;
};

class OpcodeMUL : public OpcodeOperation {
public:
	explicit OpcodeMUL(const QByteArray &params);
	explicit OpcodeMUL(const OpcodeBinaryOperation &op);
	int id() const { return 0x89; }
	QString toString() const;
};

class OpcodeMUL2 : public OpcodeOperation2 {
public:
	explicit OpcodeMUL2(const QByteArray &params);
	explicit OpcodeMUL2(const OpcodeBinaryOperation &op);
	int id() const { return 0x8A; }
	QString toString() const;
};

class OpcodeDIV : public OpcodeOperation {
public:
	explicit OpcodeDIV(const QByteArray &params);
	explicit OpcodeDIV(const OpcodeBinaryOperation &op);
	int id() const { return 0x8B; }
	QString toString() const;
};

class OpcodeDIV2 : public OpcodeOperation2 {
public:
	explicit OpcodeDIV2(const QByteArray &params);
	explicit OpcodeDIV2(const OpcodeBinaryOperation &op);
	int id() const { return 0x8C; }
	QString toString() const;
};

class OpcodeMOD : public OpcodeOperation {
public:
	explicit OpcodeMOD(const QByteArray &params);
	explicit OpcodeMOD(const OpcodeBinaryOperation &op);
	int id() const { return 0x8D; }
	QString toString() const;
};

class OpcodeMOD2 : public OpcodeOperation2 {
public:
	explicit OpcodeMOD2(const QByteArray &params);
	explicit OpcodeMOD2(const OpcodeBinaryOperation &op);
	int id() const { return 0x8E; }
	QString toString() const;
};

class OpcodeAND : public OpcodeOperation {
public:
	explicit OpcodeAND(const QByteArray &params);
	explicit OpcodeAND(const OpcodeBinaryOperation &op);
	int id() const { return 0x8F; }
	QString toString() const;
};

class OpcodeAND2 : public OpcodeOperation2 {
public:
	explicit OpcodeAND2(const QByteArray &params);
	explicit OpcodeAND2(const OpcodeBinaryOperation &op);
	int id() const { return 0x90; }
	QString toString() const;
};

class OpcodeOR : public OpcodeOperation {
public:
	explicit OpcodeOR(const QByteArray &params);
	explicit OpcodeOR(const OpcodeBinaryOperation &op);
	int id() const { return 0x91; }
	QString toString() const;
};

class OpcodeOR2 : public OpcodeOperation2 {
public:
	explicit OpcodeOR2(const QByteArray &params);
	explicit OpcodeOR2(const OpcodeBinaryOperation &op);
	int id() const { return 0x92; }
	QString toString() const;
};

class OpcodeXOR : public OpcodeOperation {
public:
	explicit OpcodeXOR(const QByteArray &params);
	explicit OpcodeXOR(const OpcodeBinaryOperation &op);
	int id() const { return 0x93; }
	QString toString() const;
};

class OpcodeXOR2 : public OpcodeOperation2 {
public:
	explicit OpcodeXOR2(const QByteArray &params);
	explicit OpcodeXOR2(const OpcodeBinaryOperation &op);
	int id() const { return 0x94; }
	QString toString() const;
};

class OpcodeINC : public OpcodeUnaryOperation {
public:
	explicit OpcodeINC(const QByteArray &params);
	explicit OpcodeINC(const OpcodeUnaryOperation &op);
	int id() const { return 0x95; }
	QString toString() const;
	bool isLong() const { return false; }
};

class OpcodeINC2 : public OpcodeUnaryOperation {
public:
	explicit OpcodeINC2(const QByteArray &params);
	explicit OpcodeINC2(const OpcodeUnaryOperation &op);
	int id() const { return 0x96; }
	QString toString() const;
	bool isLong() const { return true; }
};

class OpcodeDEC : public OpcodeUnaryOperation {
public:
	explicit OpcodeDEC(const QByteArray &params);
	explicit OpcodeDEC(const OpcodeUnaryOperation &op);
	int id() const { return 0x97; }
	QString toString() const;
	bool isLong() const { return false; }
};

class OpcodeDEC2 : public OpcodeUnaryOperation {
public:
	explicit OpcodeDEC2(const QByteArray &params);
	explicit OpcodeDEC2(const OpcodeUnaryOperation &op);
	int id() const { return 0x98; }
	QString toString() const;
	bool isLong() const { return true; }
};

class OpcodeRANDOM : public OpcodeUnaryOperation {
public:
	explicit OpcodeRANDOM(const QByteArray &params);
	explicit OpcodeRANDOM(const OpcodeUnaryOperation &op);
	int id() const { return 0x99; }
	QString toString() const;
	bool isLong() const { return false; }
};

class OpcodeLBYTE : public OpcodeOperation {
public:
	explicit OpcodeLBYTE(const QByteArray &params);
	explicit OpcodeLBYTE(const OpcodeBinaryOperation &op);
	int id() const { return 0x9A; }
	QString toString() const;
};

class OpcodeHBYTE : public OpcodeOperation2 {
public:
	explicit OpcodeHBYTE(const QByteArray &params);
	explicit OpcodeHBYTE(const OpcodeBinaryOperation &op);
	int id() const { return 0x9B; }
	QString toString() const;
};

class Opcode2BYTE : public Opcode {
public:
	explicit Opcode2BYTE(const QByteArray &params);
	int id() const { return 0x9C; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2], var, value1, value2;
};

class OpcodeSETX : public Opcode {
public:
	explicit OpcodeSETX(const QByteArray &params);
	int id() const { return 0x9D; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown[6];
};

class OpcodeGETX : public Opcode {
public:
	explicit OpcodeGETX(const QByteArray &params);
	int id() const { return 0x9E; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown[6];
};

class OpcodeSEARCHX : public Opcode {
public:
	explicit OpcodeSEARCHX(const QByteArray &params);
	int id() const { return 0x9F; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown[10];
};

class OpcodePC : public Opcode {
public:
	explicit OpcodePC(const QByteArray &params);
	int id() const { return 0xA0; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 charID;
};

class OpcodeCHAR : public Opcode {
public:
	explicit OpcodeCHAR(const QByteArray &params);
	int id() const { return 0xA1; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 objectID;
};

class OpcodeDFANM : public Opcode {
public:
	explicit OpcodeDFANM(const QByteArray &params);
	int id() const { return 0xA2; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 animID, speed;
};
// note: same struct as DFANM
class OpcodeANIME1 : public Opcode {
public:
	explicit OpcodeANIME1(const QByteArray &params);
	int id() const { return 0xA3; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 animID, speed;
};

class OpcodeVISI : public Opcode {
public:
	explicit OpcodeVISI(const QByteArray &params);
	int id() const { return 0xA4; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 show;
};

class OpcodeXYZI : public Opcode {
public:
	explicit OpcodeXYZI(const QByteArray &params);
	int id() const { return 0xA5; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	qint16 targetX, targetY, targetZ;
	quint16 targetI;
};

class OpcodeXYI : public Opcode {
public:
	explicit OpcodeXYI(const QByteArray &params);
	int id() const { return 0xA6; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 targetI;
};

class OpcodeXYZ : public Opcode {
public:
	explicit OpcodeXYZ(const QByteArray &params);
	int id() const { return 0xA7; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	qint16 targetX, targetY, targetZ;
};

class OpcodeMOVE : public Opcode {
public:
	explicit OpcodeMOVE(const QByteArray &params);
	int id() const { return 0xA8; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	qint16 targetX, targetY;
};
// note: same struct as MOVE
class OpcodeCMOVE : public Opcode {
public:
	explicit OpcodeCMOVE(const QByteArray &params);
	int id() const { return 0xA9; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	qint16 targetX, targetY;
};

class OpcodeMOVA : public Opcode {
public:
	explicit OpcodeMOVA(const QByteArray &params);
	int id() const { return 0xAA; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 groupID;
};

class OpcodeTURA : public Opcode {
public:
	explicit OpcodeTURA(const QByteArray &params);
	int id() const { return 0xAB; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 groupID, directionRotation, speed;
};

class OpcodeANIMW : public Opcode {
public:
	explicit OpcodeANIMW();
	int id() const { return 0xAC; }
	QString toString() const;
};

class OpcodeFMOVE : public Opcode {
public:
	explicit OpcodeFMOVE(const QByteArray &params);
	int id() const { return 0xAD; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	qint16 targetX, targetY;
};

class OpcodeANIME2 : public Opcode {
public:
	explicit OpcodeANIME2(const QByteArray &params);
	int id() const { return 0xAE; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 animID, speed;
};
// note: same struct as ANIME2
class OpcodeANIMX1 : public Opcode {
public:
	explicit OpcodeANIMX1(const QByteArray &params);
	int id() const { return 0xAF; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 animID, speed;
};

class OpcodeCANIM1 : public Opcode {
public:
	explicit OpcodeCANIM1(const QByteArray &params);
	int id() const { return 0xB0; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 animID, firstFrame, lastFrame, speed;
};
// note: same struct as CANIM1
class OpcodeCANMX1 : public Opcode {
public:
	explicit OpcodeCANMX1(const QByteArray &params);
	int id() const { return 0xB1; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 animID, firstFrame, lastFrame, speed;
};

class OpcodeMSPED : public Opcode {
public:
	explicit OpcodeMSPED(const QByteArray &params);
	int id() const { return 0xB2; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint16 speed;
};

class OpcodeDIR : public Opcode {
public:
	explicit OpcodeDIR(const QByteArray &params);
	int id() const { return 0xB3; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, direction;
};

class OpcodeTURNGEN : public Opcode {
public:
	explicit OpcodeTURNGEN(const QByteArray &params);
	int id() const { return 0xB4; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, direction, turnCount, speed, unknown;
};
// note: same struct as TURNGEN
class OpcodeTURN : public Opcode {
public:
	explicit OpcodeTURN(const QByteArray &params);
	int id() const { return 0xB5; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, direction, turnCount, speed, unknown;
};
// note: same struct as MOVA
class OpcodeDIRA : public Opcode {
public:
	explicit OpcodeDIRA(const QByteArray &params);
	int id() const { return 0xB6; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 groupID;
};

class OpcodeGETDIR : public Opcode {
public:
	explicit OpcodeGETDIR(const QByteArray &params);
	int id() const { return 0xB7; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, groupID, varDir;
};

class OpcodeGETAXY : public Opcode {
public:
	explicit OpcodeGETAXY(const QByteArray &params);
	int id() const { return 0xB8; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, groupID, varX, varY;
};

class OpcodeGETAI : public Opcode {
public:
	explicit OpcodeGETAI(const QByteArray &params);
	int id() const { return 0xB9; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, groupID, varI;
};
// note: same struct as ANIME2
class OpcodeANIMX2 : public Opcode {
public:
	explicit OpcodeANIMX2(const QByteArray &params);
	int id() const { return 0xBA; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 animID, speed;
};
// note: same struct as CANIM1
class OpcodeCANIM2 : public Opcode {
public:
	explicit OpcodeCANIM2(const QByteArray &params);
	int id() const { return 0xBB; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 animID, firstFrame, lastFrame, speed;
};
// note: same struct as CANIM1
class OpcodeCANMX2 : public Opcode {
public:
	explicit OpcodeCANMX2(const QByteArray &params);
	int id() const { return 0xBC; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 animID, firstFrame, lastFrame, speed;
};

class OpcodeASPED : public Opcode {
public:
	explicit OpcodeASPED(const QByteArray &params);
	int id() const { return 0xBD; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint16 speed;
};
// note: same struct as MOVA
class OpcodeCC : public Opcode {
public:
	explicit OpcodeCC(const QByteArray &params);
	int id() const { return 0xBF; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 groupID;
};

class OpcodeJUMP : public Opcode {
public:
	explicit OpcodeJUMP(const QByteArray &params);
	int id() const { return 0xC0; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 targetI;
	qint16 height;
};

class OpcodeAXYZI : public Opcode {
public:
	explicit OpcodeAXYZI(const QByteArray &params);
	int id() const { return 0xC1; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2], groupID, varX, varY, varZ, varI;
};

class OpcodeLADER : public Opcode {
public:
	explicit OpcodeLADER(const QByteArray &params);
	int id() const { return 0xC2; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	qint16 targetX, targetY, targetZ;
	quint16 targetI;
	quint8 way, animID, direction, speed;
};

class OpcodeOFST : public Opcode {
public:
	explicit OpcodeOFST(const QByteArray &params);
	int id() const { return 0xC3; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2], moveType;
	qint16 targetX, targetY, targetZ;
	quint16 speed;
};

class OpcodeOFSTW : public Opcode {
public:
	explicit OpcodeOFSTW();
	int id() const { return 0xC4; }
	QString toString() const;
};

class OpcodeTALKR : public Opcode {
public:
	explicit OpcodeTALKR(const QByteArray &params);
	int id() const { return 0xC5; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, distance;
};
// note: same struct as TALKR
class OpcodeSLIDR : public Opcode {
public:
	explicit OpcodeSLIDR(const QByteArray &params);
	int id() const { return 0xC6; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, distance;
};

class OpcodeSOLID : public Opcode {
public:
	explicit OpcodeSOLID(const QByteArray &params);
	int id() const { return 0xC7; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 disabled;
};
// note: same struct as PC
class OpcodePRTYP : public Opcode {
public:
	explicit OpcodePRTYP(const QByteArray &params);
	int id() const { return 0xC8; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 charID;
};
// note: same struct as PC
class OpcodePRTYM : public Opcode {
public:
	explicit OpcodePRTYM(const QByteArray &params);
	int id() const { return 0xC9; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 charID;
};

class OpcodePRTYE : public Opcode {
public:
	explicit OpcodePRTYE(const QByteArray &params);
	int id() const { return 0xCA; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 charID[3];
};

class OpcodeIfQ : public OpcodeJump {
public:
	explicit OpcodeIfQ(const QByteArray &params);
	void setParams(const QByteArray &params);
	QByteArray params() const;
	bool isLongJump() const { return false; }
	quint8 jumpPosData() const { return 2; }
	quint8 charID;
};

class OpcodeIFPRTYQ : public OpcodeIfQ {
public:
	explicit OpcodeIFPRTYQ(const QByteArray &params);
	int id() const { return 0xCB; }
	QString toString() const;
};

class OpcodeIFMEMBQ : public OpcodeIfQ {
public:
	explicit OpcodeIFMEMBQ(const QByteArray &params);
	int id() const { return 0xCC; }
	QString toString() const;
};

class OpcodeMMBUD : public Opcode {
public:
	explicit OpcodeMMBUD(const QByteArray &params);
	int id() const { return 0xCD; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 exists, charID;
};
// note: same struct as PC
class OpcodeMMBLK : public Opcode {
public:
	explicit OpcodeMMBLK(const QByteArray &params);
	int id() const { return 0xCE; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 charID;
};
// note: same struct as PC
class OpcodeMMBUK : public Opcode {
public:
	explicit OpcodeMMBUK(const QByteArray &params);
	int id() const { return 0xCF; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 charID;
};

class OpcodeLINE : public Opcode {
public:
	explicit OpcodeLINE(const QByteArray &params);
	int id() const { return 0xD0; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	qint16 targetX1, targetY1, targetZ1;
	qint16 targetX2, targetY2, targetZ2;
};

class OpcodeLINON : public Opcode {
public:
	explicit OpcodeLINON(const QByteArray &params);
	int id() const { return 0xD1; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 enabled;
};

class OpcodeMPJPO : public Opcode {
public:
	explicit OpcodeMPJPO(const QByteArray &params);
	int id() const { return 0xD2; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 prevent;
};

class OpcodeSLINE : public Opcode {
public:
	explicit OpcodeSLINE(const QByteArray &params);
	int id() const { return 0xD3; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[3];
	qint16 targetX1, targetY1, targetZ1;
	qint16 targetX2, targetY2, targetZ2;
};

class OpcodeSIN : public Opcode {
public:
	explicit OpcodeSIN(const QByteArray &params);
	int id() const { return 0xD4; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	quint16 value1, value2, value3;
	quint8 var;
};
// note: same struct as SIN
class OpcodeCOS : public Opcode {
public:
	explicit OpcodeCOS(const QByteArray &params);
	int id() const { return 0xD5; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2];
	quint16 value1, value2, value3;
	quint8 var;
};

class OpcodeTLKR2 : public Opcode {
public:
	explicit OpcodeTLKR2(const QByteArray &params);
	int id() const { return 0xD6; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint16 distance;
};
// note: same struct as TLKR2
class OpcodeSLDR2 : public Opcode {
public:
	explicit OpcodeSLDR2(const QByteArray &params);
	int id() const { return 0xD7; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint16 distance;
};

class OpcodePMJMP : public Opcode {
public:
	explicit OpcodePMJMP(const QByteArray &params);
	int id() const { return 0xD8; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint16 fieldID;
};

class OpcodePMJMP2 : public Opcode {
public:
	explicit OpcodePMJMP2();
	int id() const { return 0xD9; }
	QString toString() const;
};

class OpcodeAKAO2 : public Opcode {
public:
	explicit OpcodeAKAO2(const QByteArray &params);
	int id() const { return 0xDA; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown[14];
};

class OpcodeFCFIX : public Opcode {
public:
	explicit OpcodeFCFIX(const QByteArray &params);
	int id() const { return 0xDB; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 disabled;
};

class OpcodeCCANM : public Opcode {
public:
	explicit OpcodeCCANM(const QByteArray &params);
	int id() const { return 0xDC; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 animID, speed, standWalkRun;
};

class OpcodeANIMB : public Opcode {
public:
	explicit OpcodeANIMB();
	int id() const { return 0xDD; }
	QString toString() const;
};

class OpcodeTURNW : public Opcode {
public:
	explicit OpcodeTURNW();
	int id() const { return 0xDE; }
	QString toString() const;
};

class OpcodeMPPAL : public Opcode {
public:
	explicit OpcodeMPPAL(const QByteArray &params);
	int id() const { return 0xDF; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2], u0, posSrc, posDst, a, r, g, b, colorCount;
};

class OpcodeBGON : public Opcode {
public:
	explicit OpcodeBGON(const QByteArray &params);
	int id() const { return 0xE0; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, paramID, stateID;
};
// note: same struct as BGON
class OpcodeBGOFF : public Opcode {
public:
	explicit OpcodeBGOFF(const QByteArray &params);
	int id() const { return 0xE1; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, paramID, stateID;
};

class OpcodeBGROL : public Opcode {
public:
	explicit OpcodeBGROL(const QByteArray &params);
	int id() const { return 0xE2; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, paramID;
};
// note: same struct as BGROL
class OpcodeBGROL2 : public Opcode {
public:
	explicit OpcodeBGROL2(const QByteArray &params);
	int id() const { return 0xE3; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, paramID;
};
// note: same struct as BGROL
class OpcodeBGCLR : public Opcode {
public:
	explicit OpcodeBGCLR(const QByteArray &params);
	int id() const { return 0xE4; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, paramID;
};

class OpcodeSTPAL : public Opcode {
public:
	explicit OpcodeSTPAL(const QByteArray &params);
	int id() const { return 0xE5; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, palID, position, colorCount;
};

class OpcodeLDPAL : public Opcode {
public:
	explicit OpcodeLDPAL(const QByteArray &params);
	int id() const { return 0xE6; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, position, palID, colorCount;
};

class OpcodeCPPAL : public Opcode {
public:
	explicit OpcodeCPPAL(const QByteArray &params);
	int id() const { return 0xE7; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, posSrc, posDst, colorCount;
};

class OpcodeRTPAL : public Opcode {
public:
	explicit OpcodeRTPAL(const QByteArray &params);
	int id() const { return 0xE8; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[2], u1, u2, u3, u5;
};

class OpcodeADPAL : public Opcode {
public:
	explicit OpcodeADPAL(const QByteArray &params);
	int id() const { return 0xE9; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[3], posSrc, posDst, r, g, b, colorCount;
};
// note: same struct as ADPAL
class OpcodeMPPAL2 : public Opcode {
public:
	explicit OpcodeMPPAL2(const QByteArray &params);
	int id() const { return 0xEA; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks[3], posSrc, posDst, r, g, b, colorCount;
};

class OpcodeSTPLS : public Opcode {
public:
	explicit OpcodeSTPLS(const QByteArray &params);
	int id() const { return 0xEB; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 palID, u1, u6, colorCount;
};

class OpcodeLDPLS : public Opcode {
public:
	explicit OpcodeLDPLS(const QByteArray &params);
	int id() const { return 0xEC; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 u2, palID, u6, colorCount;
};

class OpcodeCPPAL2 : public Opcode {
public:
	explicit OpcodeCPPAL2(const QByteArray &params);
	int id() const { return 0xED; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown[7];
};

class OpcodeRTPAL2 : public Opcode {
public:
	explicit OpcodeRTPAL2(const QByteArray &params);
	int id() const { return 0xEE; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown[7];
};

class OpcodeADPAL2 : public Opcode {
public:
	explicit OpcodeADPAL2(const QByteArray &params);
	int id() const { return 0xEF; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown[10];
};

class OpcodeMUSIC : public Opcode {
public:
	explicit OpcodeMUSIC(const QByteArray &params);
	int id() const { return 0xF0; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 musicID;
};

class OpcodeSOUND : public Opcode {
public:
	explicit OpcodeSOUND(const QByteArray &params);
	int id() const { return 0xF1; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks;
	quint16 soundID;
	quint8 position;
};

class OpcodeAKAO : public Opcode {
public:
	explicit OpcodeAKAO(const QByteArray &params);
	int id() const { return 0xF2; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown[13];
};
// note: same struct as MUSIC
class OpcodeMUSVT : public Opcode {
public:
	explicit OpcodeMUSVT(const QByteArray &params);
	int id() const { return 0xF3; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 musicID;
};
// note: same struct as MUSIC
class OpcodeMUSVM : public Opcode {
public:
	explicit OpcodeMUSVM(const QByteArray &params);
	int id() const { return 0xF4; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 musicID;
};

class OpcodeMULCK : public Opcode {
public:
	explicit OpcodeMULCK(const QByteArray &params);
	int id() const { return 0xF5; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 locked;
};
// note: same struct as MUSIC
class OpcodeBMUSC : public Opcode {
public:
	explicit OpcodeBMUSC(const QByteArray &params);
	int id() const { return 0xF6; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 musicID;
};

class OpcodeCHMPH : public Opcode {
public:
	explicit OpcodeCHMPH(const QByteArray &params);
	int id() const { return 0xF7; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown[3];
};

class OpcodePMVIE : public Opcode {
public:
	explicit OpcodePMVIE(const QByteArray &params);
	int id() const { return 0xF8; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 movieID;
};

class OpcodeMOVIE : public Opcode {
public:
	explicit OpcodeMOVIE();
	int id() const { return 0xF9; }
	QString toString() const;
};

class OpcodeMVIEF : public Opcode {
public:
	explicit OpcodeMVIEF(const QByteArray &params);
	int id() const { return 0xFA; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, varCurMovieFrame;
};

class OpcodeMVCAM : public Opcode {
public:
	explicit OpcodeMVCAM(const QByteArray &params);
	int id() const { return 0xFB; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 movieCamID;
};

class OpcodeFMUSC : public Opcode {
public:
	explicit OpcodeFMUSC(const QByteArray &params);
	int id() const { return 0xFC; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown;
};

class OpcodeCMUSC : public Opcode {
public:
	explicit OpcodeCMUSC(const QByteArray &params);
	int id() const { return 0xFD; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	quint8 unknown[5];
};
// same struct as unaryOperation
class OpcodeCHMST : public Opcode {
public:
	explicit OpcodeCHMST(const QByteArray &params);
	int id() const { return 0xFE; }
	QString toString() const;
	void setParams(const QByteArray &params);
	QByteArray params() const;
	void getVariables(QList<FF7Var> &vars) const;
	quint8 banks, var;
};

class OpcodeGAMEOVER : public Opcode {
public:
	explicit OpcodeGAMEOVER();
	int id() const { return 0xFF; }
	QString toString() const;
};

#endif
