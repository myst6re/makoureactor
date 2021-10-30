/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#pragma once

#include <QtCore>

class OpcodeBox;

#define B1(v)		((v>>4)&0xF)
#define B2(v)		(v&0xF)
#define OPERATORS_SIZE	11
#define NOWIN		255

struct FF7Window {
	qint16 x, y;
	quint16 w, h;
	quint16 ask_first, ask_last;
	quint8 type, mode, displayType;
	quint16 groupID, scriptID, opcodeID;

	QPoint realPos() {
		if (this->type == NOWIN) {
			return QPoint();
		}

		int windowX = this->x, windowY = this->y;

		if (windowX + this->w > 312) {
			windowX = 312 - this->w;
		}

		if (windowY + this->h > 223) {
			windowY = 223 - this->h;
		}

		if (windowX < 8) {
			windowX = 8;
		}

		if (windowY < 8) {
			windowY = 8;
		}

		return QPoint(windowX, windowY);
	}
};

struct FF7Position {
	qint16 x, y, z;
	quint16 id;
	bool hasZ, hasId;
};

struct FF7Var {
	enum VarSize {
		Byte, Word, SignedWord, Bit
	};

	FF7Var(quint8 b, quint8 a, VarSize size = Byte, bool w = false)
		: bank(b), address(a), size(size), write(w) {}
	quint8 bank;
	quint8 address;
	VarSize size;
	bool write;
};

inline bool operator==(const FF7Var &v1, const FF7Var &v2) {
	return v1.bank == v2.bank && v1.address == v2.address;
}

inline bool operator<(const FF7Var &v1, const FF7Var &v2) {
	if (v1.bank == v2.bank) {
		return v1.address < v2.address;
	}
	return v1.bank < v2.bank;
}

class Section1File;

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

	enum Operation {
		None, Assign, BitAssign, Compare, BitCompare,
		AssignNotEqual,
		AssignLessThan, AssignLessThanEqual,
		AssignGreaterThan, AssignGreaterThanEqual,
		CompareNotEqual,
		CompareLessThan, CompareLessThanEqual,
		CompareGreaterThan, CompareGreaterThanEqual
	};

	enum Operator {
		Equal, NotEqual,
		UpperThan, LowerThan,
		UpperThanEqual, LowerThanEqual,
		BitAnd, BitXOr, BitOr,
		BitOn, BitOff
	};

	static const char *operators[OPERATORS_SIZE];

	Opcode();
	virtual ~Opcode();
	virtual int id() const=0;
	virtual QString toString(const Section1File *scriptsAndTexts) const=0;
	inline virtual void setParams(const char *params, int size) { Q_UNUSED(params) Q_UNUSED(size) }
	inline virtual QByteArray params() const { return QByteArray(); }
	inline virtual quint8 size() const { return Opcode::length[id()]; }
	inline bool hasParams() const { return size() > 1; }
	inline const QString &name() const { return Opcode::names[id()]; }
	virtual QByteArray toByteArray() const;
	// Unserializable byte array (include OpcodeLABEL case)
	virtual QByteArray serialize() const;
	static OpcodeBox unserialize(const QByteArray &data);
	inline virtual bool isExec() const { return false; }
	inline virtual bool isJump() const { return false; }
	inline virtual bool isIf() const { return false; }
	inline virtual bool isLabel() const { return false; }
	int subParam(int cur, int paramSize) const;

	inline virtual bool isVoid() const { return false; }

	bool searchVar(quint8 bank, quint16 address, Operation op = None, int value=65536) const;

	inline virtual int getGroupID() const { return -1; }
	inline virtual void setGroupID(quint8 groupID) { Q_UNUSED(groupID) }
	inline virtual int getTextID() const { return -1; }
	inline virtual void setTextID(quint8 textID) { Q_UNUSED(textID) }
	inline virtual int getTutoID() const { return -1; }
	inline virtual void setTutoID(quint8 tutoID) { Q_UNUSED(tutoID) }
	inline virtual int getWindowID() const { return -1; }
	inline virtual void setWindowID(quint8 windowID) { Q_UNUSED(windowID) }
	inline virtual bool getWindow(FF7Window &window) const { Q_UNUSED(window) return false; }
	inline virtual void setWindow(const FF7Window &window) { Q_UNUSED(window) }
	inline virtual void getVariables(QList<FF7Var> &vars) const { Q_UNUSED(vars) }
	virtual bool searchOpcode(int opcode) const;
	bool searchExec(quint8 group, quint8 script) const;
	bool searchMapJump(quint16 fieldID) const;
	bool searchTextInScripts(const QRegularExpression &text, const Section1File *scriptsAndTexts) const;
	void listUsedTexts(QSet<quint8> &usedTexts) const;
	void listUsedTuts(QSet<quint8> &usedTuts) const;
	void shiftGroupIds(int groupId, int steps);
	void shiftTextIds(int textId, int steps);
	void shiftTutIds(int tutId, int steps);
	void swapGroupIds(int groupId1, int groupId2);
	void listWindows(int groupID, int scriptID, int opcodeID, QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const;
	virtual void listModelPositions(QList<FF7Position> &positions) const;
	virtual bool linePosition(FF7Position position[2]) const;
	void backgroundParams(QHash<quint8, quint8> &enabledParams) const;
	void backgroundMove(qint16 z[2], qint16 *x, qint16 *y) const;

	static QString character(quint8 persoID);
	static QString akao(quint8 akaoOp, bool *ok = nullptr);

	static const quint8 length[257];
	static const QString names[257];
protected:
	static QString _script(quint8 param, const Section1File *scriptsAndTexts);
	static QString _text(quint8 textID, const Section1File *scriptsAndTexts);
	static QString _item(quint16 itemID, quint8 bank);
	static QString _materia(quint8 materiaID, quint8 bank);
	static QString _field(quint16 fieldID);
	static QString _movie(quint8 movieID);
	// static QString _objet3D(quint8 objet3D_ID);

	static QString _bank(quint8 address, quint8 bank);
	static QString _var(int value, quint8 bank);
	static QString _var(int value, quint8 bank1, quint8 bank2);
	static QString _var(int value, quint8 bank1, quint8 bank2, quint8 bank3);

	static QString _operateur(quint8 param);
	static QString _windowCorner(quint8 param, quint8 bank);
	static QString _sensRotation(quint8 param);
};

class OpcodeUnknown : public Opcode {
public:
	explicit OpcodeUnknown(quint8 id, const QByteArray &params = QByteArray());
	OpcodeUnknown(quint8 id, const char *params, int size);
	int id() const override;
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	QByteArray unknown;
	quint8 _id;
};

class OpcodeRET : public Opcode {
public:
	explicit OpcodeRET();
	inline int id() const override { return 0x00; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	inline bool isVoid() const override { return true; }
};

class OpcodeExec : public Opcode {
public:
	explicit OpcodeExec(const char *params, int size);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isExec() const override { return true; }
	inline virtual int getGroupID() const override { return groupID; }
	inline virtual void setGroupID(quint8 groupID) override { this->groupID = groupID; }
	quint8 groupID;
	quint8 scriptID;
	quint8 priority;
};

class OpcodeREQ : public OpcodeExec {
public:
	explicit OpcodeREQ(const char *params, int size);
	explicit OpcodeREQ(const OpcodeExec &op);
	inline int id() const override { return 0x01; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeREQSW : public OpcodeExec {
public:
	explicit OpcodeREQSW(const char *params, int size);
	explicit OpcodeREQSW(const OpcodeExec &op);
	inline int id() const override { return 0x02; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeREQEW : public OpcodeExec {
public:
	explicit OpcodeREQEW(const char *params, int size);
	explicit OpcodeREQEW(const OpcodeExec &op);
	inline int id() const override { return 0x03; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeExecChar : public Opcode {
public:
	explicit OpcodeExecChar(const char *params, int size);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 partyID;
	quint8 scriptID;
	quint8 priority;
};

class OpcodePREQ : public OpcodeExecChar {
public:
	explicit OpcodePREQ(const char *params, int size);
	explicit OpcodePREQ(const OpcodeExecChar &op);
	inline int id() const override { return 0x04; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodePRQSW : public OpcodeExecChar {
public:
	explicit OpcodePRQSW(const char *params, int size);
	explicit OpcodePRQSW(const OpcodeExecChar &op);
	inline int id() const override { return 0x05; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodePRQEW : public OpcodeExecChar {
public:
	explicit OpcodePRQEW(const char *params, int size);
	explicit OpcodePRQEW(const OpcodeExecChar &op);
	inline int id() const override { return 0x06; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeRETTO : public Opcode {
public:
	explicit OpcodeRETTO(const char *params, int size);
	inline int id() const override { return 0x07; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 scriptID;
	quint8 priority;
};

class OpcodeJOIN : public Opcode {
public:
	explicit OpcodeJOIN(const char *params, int size);
	inline int id() const override { return 0x08; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 speed;
};

class OpcodeSPLIT : public Opcode {
public:
	explicit OpcodeSPLIT(const char *params, int size);
	inline int id() const override { return 0x09; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[3];
	quint8 speed;
	qint16 targetX1, targetY1, targetX2, targetY2;
	quint8 direction1, direction2;
};

class OpcodePartyE : public Opcode {
public:
	explicit OpcodePartyE(const char *params, int size);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	virtual void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2];
	quint8 party1, party2, party3;
};

class OpcodeSPTYE : public OpcodePartyE {
public:
	explicit OpcodeSPTYE(const char *params, int size);
	explicit OpcodeSPTYE(const OpcodePartyE &op);
	inline int id() const override { return 0x0A; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeGTPYE : public OpcodePartyE {
public:
	explicit OpcodeGTPYE(const char *params, int size);
	explicit OpcodeGTPYE(const OpcodePartyE &op);
	inline int id() const override { return 0x0B; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void getVariables(QList<FF7Var> &vars) const override;
};

class OpcodeDSKCG : public Opcode {
public:
	explicit OpcodeDSKCG(const char *params, int size);
	inline int id() const override { return 0x0E; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 diskID;
};

class OpcodeSPECIALARROW : public Opcode {
public:
	explicit OpcodeSPECIALARROW(const char *params, int size);
	inline int id() const override { return 0xF5; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 hide;
};

class OpcodeSPECIALPNAME : public Opcode {
public:
	explicit OpcodeSPECIALPNAME(const char *params, int size);
	inline int id() const override { return 0xF6; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 unknown;
};

class OpcodeSPECIALGMSPD : public Opcode {
public:
	explicit OpcodeSPECIALGMSPD(const char *params, int size);
	inline int id() const override { return 0xF7; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 speed;
};

class OpcodeSPECIALSMSPD : public Opcode {
public:
	explicit OpcodeSPECIALSMSPD(const char *params, int size);
	inline int id() const override { return 0xF8; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 unknown, speed;
};

class OpcodeSPECIALFLMAT : public Opcode {
public:
	explicit OpcodeSPECIALFLMAT();
	inline int id() const override { return 0xF9; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeSPECIALFLITM : public Opcode {
public:
	explicit OpcodeSPECIALFLITM();
	inline int id() const override { return 0xFA; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeSPECIALBTLCK : public Opcode {
public:
	explicit OpcodeSPECIALBTLCK(const char *params, int size);
	inline int id() const override { return 0xFB; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 lock;
};

class OpcodeSPECIALMVLCK : public Opcode {
public:
	explicit OpcodeSPECIALMVLCK(const char *params, int size);
	inline int id() const override { return 0xFC; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 lock;
};

class OpcodeSPECIALSPCNM : public Opcode {
public:
	explicit OpcodeSPECIALSPCNM(const char *params, int size);
	inline int id() const override { return 0xFD; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getTextID() const override;
	void setTextID(quint8 textID) override;
	quint8 charID, textID;
};

class OpcodeSPECIALRSGLB : public Opcode {
public:
	explicit OpcodeSPECIALRSGLB();
	inline int id() const override { return 0xFE; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeSPECIALCLITM : public Opcode {
public:
	explicit OpcodeSPECIALCLITM();
	inline int id() const override { return 0xFF; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeSPECIAL : public Opcode {
public:
	explicit OpcodeSPECIAL(const char *params, int size);
	OpcodeSPECIAL(const OpcodeSPECIAL &other);
	virtual ~OpcodeSPECIAL() override;
	inline int id() const override { return 0x0F; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getTextID() const override;
	void setTextID(quint8 textID) override;
	Opcode *opcode;
};

class OpcodeJump : public Opcode {
public:
	explicit OpcodeJump();
	qint32 jump() const;
	void setJump(qint32 jump);
	quint32 label() const;
	void setLabel(quint32 label);
	bool isBadJump() const;
	void setBadJump(bool badJump);
	inline bool isJump() const override { return true; }
	virtual bool isLongJump() const=0;
	inline virtual bool isBackJump() const { return false; }
	quint32 maxJump() const;
	virtual quint8 jumpPosData() const=0;
	QByteArray serialize() const override;
protected:
	qint32 _jump;
	quint32 _label;
	bool _badJump;
};

class OpcodeLabel : public Opcode {
public:
	explicit OpcodeLabel(quint32 label);
	inline int id() const override { return 0x100; } // fake id
	QByteArray toByteArray() const override { return QByteArray(); }
	QByteArray serialize() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	inline bool isLabel() const override { return true; }
	inline bool isVoid() const override { return true; }
	quint32 label() const;
	void setLabel(quint32 label);
	quint32 _label;
};

class OpcodeJMPF : public OpcodeJump {
public:
	explicit OpcodeJMPF(const char *params, int size);
	explicit OpcodeJMPF(const OpcodeJump &op);
	inline int id() const override { return 0x10; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isVoid() const override { return true; }
	inline bool isLongJump() const override { return false; }
	inline quint8 jumpPosData() const override { return 1; }
//	inline static quint8 jumpPosData() { return 1; }
};

class OpcodeJMPFL : public OpcodeJump {
public:
	explicit OpcodeJMPFL(const char *params, int size);
	explicit OpcodeJMPFL(const OpcodeJump &op);
	inline int id() const override { return 0x11; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isVoid() const override { return true; }
	inline bool isLongJump() const override { return true; }
	inline quint8 jumpPosData() const override { return 1; }
};

class OpcodeJMPB : public OpcodeJump {
public:
	explicit OpcodeJMPB(const char *params, int size);
	explicit OpcodeJMPB(const OpcodeJump &op);
	inline int id() const override { return 0x12; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isVoid() const override { return true; }
	inline bool isLongJump() const override { return false; }
	inline bool isBackJump() const override { return true; }
	inline quint8 jumpPosData() const override { return 0; }
};

class OpcodeJMPBL : public OpcodeJump {
public:
	explicit OpcodeJMPBL(const char *params, int size);
	explicit OpcodeJMPBL(const OpcodeJump &op);
	inline int id() const override { return 0x13; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isVoid() const override { return true; }
	inline bool isLongJump() const override { return true; }
	inline bool isBackJump() const override { return true; }
	inline quint8 jumpPosData() const override { return 0; }
};

class OpcodeIf : public OpcodeJump {
public:
	explicit OpcodeIf();
	explicit OpcodeIf(const OpcodeJump &op);
	virtual void getVariables(QList<FF7Var> &vars) const override;
	inline virtual bool isIf() const override { return true; }
	quint8 banks;
	quint8 oper;
	bool itemIsExpanded;
	qint32 value1, value2;
};

class OpcodeIFUB : public OpcodeIf {
public:
	explicit OpcodeIFUB(const char *params, int size);
	explicit OpcodeIFUB(const OpcodeIf &op);
	inline int id() const override { return 0x14; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isLongJump() const override { return false; }
	inline quint8 jumpPosData() const override { return 5; }
};

class OpcodeIFUBL : public OpcodeIf {
public:
	explicit OpcodeIFUBL(const char *params, int size);
	explicit OpcodeIFUBL(const OpcodeIf &op);
	inline int id() const override { return 0x15; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isLongJump() const override { return true; }
	inline quint8 jumpPosData() const override { return 5; }
};

class OpcodeIFSW : public OpcodeIf {
public:
	explicit OpcodeIFSW(const char *params, int size);
	explicit OpcodeIFSW(const OpcodeIf &op);
	inline int id() const override { return 0x16; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isLongJump() const override { return false; }
	inline quint8 jumpPosData() const override { return 7; }
	void getVariables(QList<FF7Var> &vars) const override;
};

class OpcodeIFSWL : public OpcodeIf {
public:
	explicit OpcodeIFSWL(const char *params, int size);
	explicit OpcodeIFSWL(const OpcodeIf &op);
	inline int id() const override { return 0x17; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isLongJump() const override { return true; }
	inline quint8 jumpPosData() const override { return 7; }
	void getVariables(QList<FF7Var> &vars) const override;
};

class OpcodeIFUW : public OpcodeIf {
public:
	explicit OpcodeIFUW(const char *params, int size);
	explicit OpcodeIFUW(const OpcodeIf &op);
	inline int id() const override { return 0x18; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isLongJump() const override { return false; }
	inline quint8 jumpPosData() const override { return 7; }
	void getVariables(QList<FF7Var> &vars) const override;
};

class OpcodeIFUWL : public OpcodeIf {
public:
	explicit OpcodeIFUWL(const char *params, int size);
	explicit OpcodeIFUWL(const OpcodeIf &op);
	inline int id() const override { return 0x19; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isLongJump() const override { return true; }
	inline quint8 jumpPosData() const override { return 7; }
	void getVariables(QList<FF7Var> &vars) const override;
};

class Opcode1A : public Opcode {
public:
	explicit Opcode1A(const char *params, int size);
	inline int id() const override { return 0x1A; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint16 from, to;
	quint32 absValue;
	quint8 flag;
};

class Opcode1B : public OpcodeJMPFL {
public:
	explicit Opcode1B(const char *params, int size);
	explicit Opcode1B(const OpcodeJump &op);
	inline int id() const override { return 0x1B; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class Opcode1C : public Opcode {
public:
	explicit Opcode1C(const char *params, int size);
	inline int id() const override { return 0x1C; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	quint8 size() const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	QByteArray bytes;
	quint32 address;
};

class OpcodeMINIGAME : public Opcode {
public:
	explicit OpcodeMINIGAME(const char *params, int size);
	inline int id() const override { return 0x20; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint16 fieldID;
	qint16 targetX;
	qint16 targetY;
	quint16 targetI;
	quint8 minigameParam;
	quint8 minigameID;
};

class OpcodeTUTOR : public Opcode {
public:
	explicit OpcodeTUTOR(const char *params, int size);
	inline int id() const override { return 0x21; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getTutoID() const override;
	void setTutoID(quint8 tutoID) override;
	quint8 tutoID;
};

class OpcodeBTMD2 : public Opcode {
public:
	explicit OpcodeBTMD2(const char *params, int size);
	inline int id() const override { return 0x22; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint32 battleMode;
};
//note: same struct as UnaryOperation
class OpcodeBTRLD : public Opcode {
public:
	explicit OpcodeBTRLD(const char *params, int size);
	inline int id() const override { return 0x23; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, var;
};

class OpcodeWAIT : public Opcode {
public:
	explicit OpcodeWAIT(const char *params, int size);
	inline int id() const override { return 0x24; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint16 frameCount;
};

class OpcodeNFADE : public Opcode {
public:
	explicit OpcodeNFADE(const char *params, int size);
	inline int id() const override { return 0x25; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2];
	quint8 r, g, b;
	quint8 type, speed, unused;
};

class OpcodeBLINK : public Opcode {
public:
	explicit OpcodeBLINK(const char *params, int size);
	inline int id() const override { return 0x26; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 closed;
};

class OpcodeBGMOVIE : public Opcode {
public:
	explicit OpcodeBGMOVIE(const char *params, int size);
	inline int id() const override { return 0x27; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 disabled;
};

class OpcodeKAWAIEYETX : public Opcode {
public:
	explicit OpcodeKAWAIEYETX(const char *params, int size);
	inline int id() const override { return 0x00; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	QByteArray data;
	quint8 eyeID1, eyeID2, mouthID, objectID;
};

class OpcodeKAWAITRNSP : public Opcode {
public:
	explicit OpcodeKAWAITRNSP(const char *params, int size);
	inline int id() const override { return 0x01; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	QByteArray data;
	quint8 enableTransparency;
};

class OpcodeKAWAIAMBNT : public Opcode {
public:
	explicit OpcodeKAWAIAMBNT(const char *params, int size);
	inline int id() const override { return 0x02; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	QByteArray data;
	quint8 r1, r2, g1, g2, b1, b2;
	quint8 flags;
};

class OpcodeKAWAIUNKNOWN4 : public OpcodeUnknown {
public:
	explicit OpcodeKAWAIUNKNOWN4(const char *params, int size);
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeKAWAILIGHT : public OpcodeUnknown {
public:
	explicit OpcodeKAWAILIGHT(const char *params, int size);
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeKAWAIUNKNOWN7 : public OpcodeUnknown {
public:
	explicit OpcodeKAWAIUNKNOWN7(const char *params, int size);
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeKAWAIUNKNOWN8 : public OpcodeUnknown {
public:
	explicit OpcodeKAWAIUNKNOWN8(const char *params, int size);
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeKAWAIUNKNOWN9 : public OpcodeUnknown {
public:
	explicit OpcodeKAWAIUNKNOWN9(const char *params, int size);
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeKAWAISBOBJ : public OpcodeUnknown {
public:
	explicit OpcodeKAWAISBOBJ(const char *params, int size);
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeKAWAIUNKNOWNB : public OpcodeUnknown {
public:
	explicit OpcodeKAWAIUNKNOWNB(const char *params, int size);
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeKAWAIUNKNOWNC : public OpcodeUnknown {
public:
	explicit OpcodeKAWAIUNKNOWNC(const char *params, int size);
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeKAWAISHINE : public OpcodeUnknown {
public:
	explicit OpcodeKAWAISHINE(const char *params, int size);
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeKAWAIRESET : public OpcodeUnknown {
public:
	explicit OpcodeKAWAIRESET(const char *params, int size);
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeKAWAI : public Opcode {
public:
	explicit OpcodeKAWAI(const char *params, int size);
	OpcodeKAWAI(const OpcodeKAWAI &other);
	virtual ~OpcodeKAWAI() override;
	inline int id() const override { return 0x28; }
	quint8 size() const override;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	Opcode *opcode;
};

class OpcodeKAWIW : public Opcode {
public:
	explicit OpcodeKAWIW();
	inline int id() const override { return 0x29; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodePMOVA : public Opcode {
public:
	explicit OpcodePMOVA(const char *params, int size);
	inline int id() const override { return 0x2A; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 partyID;
};

class OpcodeSLIP : public Opcode {
public:
	explicit OpcodeSLIP(const char *params, int size);
	inline int id() const override { return 0x2B; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 off;
};

class OpcodeBGPDH : public Opcode {
public:
	explicit OpcodeBGPDH(const char *params, int size);
	inline int id() const override { return 0x2C; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks;
	quint8 layerID;
	qint16 targetZ;
};

class OpcodeBGSCR : public Opcode {
public:
	explicit OpcodeBGSCR(const char *params, int size);
	inline int id() const override { return 0x2D; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks;
	quint8 layerID;
	qint16 targetX, targetY;
};

class OpcodeWCLS : public Opcode {
public:
	explicit OpcodeWCLS(const char *params, int size);
	inline int id() const override { return 0x2E; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	quint8 windowID;
};

class OpcodeWindow : public Opcode {
public:
	explicit OpcodeWindow(const char *params, int size);
	explicit OpcodeWindow(quint8 windowID, quint16 targetX,
						  quint16 targetY, quint16 width,
						  quint16 height);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	bool getWindow(FF7Window &window) const override;
	void setWindow(const FF7Window &window) override;
	quint16 targetX, targetY;
	quint16 width, height;
	quint8 windowID;
};

class OpcodeWSIZW : public OpcodeWindow {
public:
	explicit OpcodeWSIZW(const char *params, int size);
	explicit OpcodeWSIZW(const OpcodeWindow &op);
	inline int id() const override { return 0x2F; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeIfKey : public OpcodeJump {
public:
	explicit OpcodeIfKey(const char *params, int size);
	explicit OpcodeIfKey(const OpcodeJump &op);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	QString keyString() const;
	inline bool isLongJump() const override { return false; }
	inline quint8 jumpPosData() const override { return 3; }
	inline virtual bool isIf() const override { return true; }
	quint16 keys;
};

class OpcodeIFKEY : public OpcodeIfKey {
public:
	explicit OpcodeIFKEY(const char *params, int size);
	explicit OpcodeIFKEY(const OpcodeIfKey &op);
	inline int id() const override { return 0x30; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeIFKEYON : public OpcodeIfKey {
public:
	explicit OpcodeIFKEYON(const char *params, int size);
	explicit OpcodeIFKEYON(const OpcodeIfKey &op);
	inline int id() const override { return 0x31; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeIFKEYOFF : public OpcodeIfKey {
public:
	explicit OpcodeIFKEYOFF(const char *params, int size);
	explicit OpcodeIFKEYOFF(const OpcodeIfKey &op);
	inline int id() const override { return 0x32; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeUC : public Opcode {
public:
	explicit OpcodeUC(const char *params, int size);
	inline int id() const override { return 0x33; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 disabled;
};

class OpcodePDIRA : public Opcode {
public:
	explicit OpcodePDIRA(const char *params, int size);
	inline int id() const override { return 0x34; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 partyID;
};

class OpcodePTURA : public Opcode {
public:
	explicit OpcodePTURA(const char *params, int size);
	inline int id() const override { return 0x35; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 partyID;
	quint8 speed;
	quint8 directionRotation;
};

class OpcodeWSPCL : public Opcode {
public:
	explicit OpcodeWSPCL(const char *params, int size);
	inline int id() const override { return 0x36; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	quint8 windowID;
	quint8 displayType;
	quint8 marginLeft, marginTop;
};

class OpcodeWNUMB : public Opcode {
public:
	explicit OpcodeWNUMB(const char *params, int size);
	inline int id() const override { return 0x37; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	void getVariables(QList<FF7Var> &vars) const override;
	qint32 value;
	quint8 banks;
	quint8 windowID;
	quint8 digitCount;
};

class OpcodeSTTIM : public Opcode {
public:
	explicit OpcodeSTTIM(const char *params, int size);
	inline int id() const override { return 0x38; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2];
	quint8 h, m, s;
};

class OpcodeGOLD : public Opcode {
public:
	explicit OpcodeGOLD(const char *params, int size);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	qint32 value;
	quint8 banks;
};

class OpcodeGOLDu : public OpcodeGOLD {
public:
	explicit OpcodeGOLDu(const char *params, int size);
	explicit OpcodeGOLDu(const OpcodeGOLD &op);
	inline int id() const override { return 0x39; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeGOLDd : public OpcodeGOLD {
public:
	explicit OpcodeGOLDd(const char *params, int size);
	explicit OpcodeGOLDd(const OpcodeGOLD &op);
	inline int id() const override { return 0x3A; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeCHGLD : public Opcode {
public:
	explicit OpcodeCHGLD(const char *params, int size);
	inline int id() const override { return 0x3B; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks;
	quint8 var1, var2;
};

class OpcodeHMPMAX1 : public Opcode {
public:
	explicit OpcodeHMPMAX1();
	inline int id() const override { return 0x3C; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeHMPMAX2 : public Opcode {
public:
	explicit OpcodeHMPMAX2();
	inline int id() const override { return 0x3D; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMHMMX : public Opcode {
public:
	explicit OpcodeMHMMX();
	inline int id() const override { return 0x3E; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeHMPMAX3 : public Opcode {
public:
	explicit OpcodeHMPMAX3();
	inline int id() const override { return 0x3F; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMESSAGE : public Opcode {
public:
	explicit OpcodeMESSAGE(const char *params, int size);
	explicit OpcodeMESSAGE(quint8 windowID, quint8 textID);
	inline int id() const override { return 0x40; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getTextID() const override;
	void setTextID(quint8 textID) override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	quint8 windowID, textID;
};

class OpcodeMPARA : public Opcode {
public:
	explicit OpcodeMPARA(const char *params, int size);
	inline int id() const override { return 0x41; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, windowID, windowVarID, value;
};

class OpcodeMPRA2 : public Opcode {
public:
	explicit OpcodeMPRA2(const char *params, int size);
	inline int id() const override { return 0x42; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint16 value;
	quint8 banks, windowID, windowVarID;
};

class OpcodeMPNAM : public Opcode {
public:
	explicit OpcodeMPNAM(const char *params, int size);
	explicit OpcodeMPNAM(quint8 textID);
	inline int id() const override { return 0x43; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getTextID() const override;
	void setTextID(quint8 textID) override;
	quint8 textID;
};

class OpcodeHPMP : public Opcode {
public:
	explicit OpcodeHPMP(const char *params, int size);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, partyID;
	quint16 value;
};

class OpcodeMPu : public OpcodeHPMP {
public:
	explicit OpcodeMPu(const char *params, int size);
	explicit OpcodeMPu(const OpcodeHPMP &op);
	inline int id() const override { return 0x45; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMPd : public OpcodeHPMP {
public:
	explicit OpcodeMPd(const char *params, int size);
	explicit OpcodeMPd(const OpcodeHPMP &op);
	inline int id() const override { return 0x47; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeASK : public Opcode {
public:
	explicit OpcodeASK(const char *params, int size);
	inline int id() const override { return 0x48; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getTextID() const override;
	void setTextID(quint8 textID) override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, windowID, textID, firstLine, lastLine, varAnswer;
};

class OpcodeMENU : public Opcode {
public:
	explicit OpcodeMENU(const char *params, int size);
	inline int id() const override { return 0x49; }
	static QString menu22(const QString &param);
	static QString menu23(const QString &param);
	QString menu(const QString &param) const;
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, menuID, param;
};

class OpcodeMENU2 : public Opcode {
public:
	explicit OpcodeMENU2(const char *params, int size);
	inline int id() const override { return 0x4A; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 disabled;
};

class OpcodeBTLTB : public Opcode {
public:
	explicit OpcodeBTLTB(const char *params, int size);
	inline int id() const override { return 0x4B; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 battleTableID;
};

class OpcodeHPu : public OpcodeHPMP {
public:
	explicit OpcodeHPu(const char *params, int size);
	explicit OpcodeHPu(const OpcodeHPMP &params);
	inline int id() const override { return 0x4D; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeHPd : public OpcodeHPMP {
public:
	explicit OpcodeHPd(const char *params, int size);
	explicit OpcodeHPd(const OpcodeHPMP &params);
	inline int id() const override { return 0x4F; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeWINDOW : public OpcodeWindow {
public:
	explicit OpcodeWINDOW(const char *params, int size);
	explicit OpcodeWINDOW(quint8 windowID, quint16 targetX,
						  quint16 targetY, quint16 width,
						  quint16 height);
	explicit OpcodeWINDOW(const OpcodeWindow &op);
	inline int id() const override { return 0x50; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeWMOVE : public Opcode {
public:
	explicit OpcodeWMOVE(const char *params, int size);
	inline int id() const override { return 0x51; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	qint16 relativeX, relativeY;
	quint8 windowID;
};

class OpcodeWMODE : public Opcode {
public:
	explicit OpcodeWMODE(const char *params, int size);
	inline int id() const override { return 0x52; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	quint8 windowID, mode;
	quint8 preventClose;
};

class OpcodeWREST : public Opcode {
public:
	explicit OpcodeWREST(const char *params, int size);
	inline int id() const override { return 0x53; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	bool getWindow(FF7Window &window) const override;
	void setWindow(const FF7Window &window) override;
	quint8 windowID;
};
// note: same struct as WREST
class OpcodeWCLSE : public Opcode {
public:
	explicit OpcodeWCLSE(const char *params, int size);
	inline int id() const override { return 0x54; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	quint8 windowID;
};

class OpcodeWROW : public Opcode {
public:
	explicit OpcodeWROW(const char *params, int size);
	inline int id() const override { return 0x55; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	int getWindowID() const override;
	void setWindowID(quint8 windowID) override;
	quint8 windowID, rowCount;
};

class OpcodeWCOL : public Opcode {
public:
	explicit OpcodeWCOL(const char *params, int size);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 banks[2], corner, r, g, b;
};

class OpcodeGWCOL : public OpcodeWCOL {
public:
	explicit OpcodeGWCOL(const char *params, int size);
	explicit OpcodeGWCOL(const OpcodeWCOL &op);
	inline int id() const override { return 0x56; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void getVariables(QList<FF7Var> &vars) const override;
};

class OpcodeSWCOL : public OpcodeWCOL {
public:
	explicit OpcodeSWCOL(const char *params, int size);
	explicit OpcodeSWCOL(const OpcodeWCOL &op);
	inline int id() const override { return 0x57; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void getVariables(QList<FF7Var> &vars) const override;
};

class OpcodeItem : public Opcode {
public:
	explicit OpcodeItem(const char *params, int size);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint16 itemID;
	quint8 banks;
	quint8 quantity;
};

class OpcodeSTITM : public OpcodeItem {
public:
	explicit OpcodeSTITM(const char *params, int size);
	explicit OpcodeSTITM(const OpcodeItem &op);
	inline int id() const override { return 0x58; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeDLITM : public OpcodeItem {
public:
	explicit OpcodeDLITM(const char *params, int size);
	explicit OpcodeDLITM(const OpcodeItem &op);
	inline int id() const override { return 0x59; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeCKITM : public OpcodeItem {
public:
	explicit OpcodeCKITM(const char *params, int size);
	explicit OpcodeCKITM(const OpcodeItem &op);
	inline int id() const override { return 0x5A; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void getVariables(QList<FF7Var> &vars) const override;
};

class OpcodeSMTRA : public Opcode {
public:
	explicit OpcodeSMTRA(const char *params, int size);
	inline int id() const override { return 0x5B; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint32 APCount;
	quint8 banks[2];
	quint8 materiaID;
};

class OpcodeDMTRA : public Opcode {
public:
	explicit OpcodeDMTRA(const char *params, int size);
	inline int id() const override { return 0x5C; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint32 APCount;
	quint8 banks[2];
	quint8 materiaID;
	quint8 quantity;
};

class OpcodeCMTRA : public Opcode {
public:
	explicit OpcodeCMTRA(const char *params, int size);
	inline int id() const override { return 0x5D; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[3];
	quint8 materiaID;
	quint32 APCount;
	quint8 unknown, varQuantity;
};

class OpcodeSHAKE : public Opcode {
public:
	explicit OpcodeSHAKE(const char *params, int size);
	inline int id() const override { return 0x5E; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 unknown1, unknown2, type, xAmplitude, xFrames;
	quint8 yAmplitude, yFrames;
};

class OpcodeNOP : public Opcode {
public:
	explicit OpcodeNOP();
	inline int id() const override { return 0x5F; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMAPJUMP : public Opcode {
public:
	explicit OpcodeMAPJUMP(const char *params, int size);
	inline int id() const override { return 0x60; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint16 fieldID;
	qint16 targetX, targetY;
	quint16 targetI;
	quint8 direction;
};

class OpcodeSCRLO : public Opcode {
public:
	explicit OpcodeSCRLO(const char *params, int size);
	inline int id() const override { return 0x61; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 unknown;
};

class OpcodeSCRLC : public Opcode {
public:
	explicit OpcodeSCRLC(const char *params, int size);
	inline int id() const override { return 0x62; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint32 unknown;
};

class OpcodeSCRLA : public Opcode {
public:
	explicit OpcodeSCRLA(const char *params, int size);
	inline int id() const override { return 0x63; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	inline virtual int getGroupID() const override { return groupID; }
	inline virtual void setGroupID(quint8 groupID) override { this->groupID = groupID; }
	quint16 speed;
	quint8 banks;
	quint8 groupID, scrollType;
};

class OpcodeSCR2D : public Opcode {
public:
	explicit OpcodeSCR2D(const char *params, int size);
	inline int id() const override { return 0x64; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	qint16 targetX, targetY;
	quint8 banks;
};

class OpcodeSCRCC : public Opcode {
public:
	explicit OpcodeSCRCC();
	inline int id() const override { return 0x65; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeSCR2DC : public Opcode {
public:
	explicit OpcodeSCR2DC(const char *params, int size);
	inline int id() const override { return 0x66; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 speed;
};

class OpcodeSCRLW : public Opcode {
public:
	explicit OpcodeSCRLW();
	inline int id() const override { return 0x67; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};
// note: same struct as SCR2DC
class OpcodeSCR2DL : public Opcode {
public:
	explicit OpcodeSCR2DL(const char *params, int size);
	inline int id() const override { return 0x68; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 speed;
};

class OpcodeMPDSP : public Opcode {
public:
	explicit OpcodeMPDSP(const char *params, int size);
	inline int id() const override { return 0x69; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 unknown;
};

class OpcodeVWOFT : public Opcode {
public:
	explicit OpcodeVWOFT(const char *params, int size);
	inline int id() const override { return 0x6A; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	qint16 unknown1, unknown2;
	quint8 banks;
	quint8 unknown3;
};

class OpcodeFADE : public Opcode {
public:
	explicit OpcodeFADE(const char *params, int size);
	inline int id() const override { return 0x6B; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2];
	quint8 r, g, b, speed, fadeType, adjust;
};

class OpcodeFADEW : public Opcode {
public:
	explicit OpcodeFADEW();
	inline int id() const override { return 0x6C; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeIDLCK : public Opcode {
public:
	explicit OpcodeIDLCK(const char *params, int size);
	inline int id() const override { return 0x6D; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint16 triangleID;
	quint8 locked;
};
//note: same struct as UnaryOperation
class OpcodeLSTMP : public Opcode {
public:
	explicit OpcodeLSTMP(const char *params, int size);
	inline int id() const override { return 0x6E; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, var;
};

class OpcodeSCRLP : public Opcode {
public:
	explicit OpcodeSCRLP(const char *params, int size);
	inline int id() const override { return 0x6F; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint16 speed;
	quint8 banks;
	quint8 partyID, scrollType;
};

class OpcodeBATTLE : public Opcode {
public:
	explicit OpcodeBATTLE(const char *params, int size);
	inline int id() const override { return 0x70; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint16 battleID;
	quint8 banks;
};

class OpcodeBTLON : public Opcode {
public:
	explicit OpcodeBTLON(const char *params, int size);
	inline int id() const override { return 0x71; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 disabled;
};

class OpcodeBTLMD : public Opcode {
public:
	explicit OpcodeBTLMD(const char *params, int size);
	inline int id() const override { return 0x72; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint16 battleMode;
};

class OpcodePGTDR : public Opcode {
public:
	explicit OpcodePGTDR(const char *params, int size);
	inline int id() const override { return 0x73; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, partyID, varDir;
};
// note: same struct as PGTDR
class OpcodeGETPC : public Opcode {
public:
	explicit OpcodeGETPC(const char *params, int size);
	inline int id() const override { return 0x74; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, partyID, varPC;
};

class OpcodePXYZI : public Opcode {
public:
	explicit OpcodePXYZI(const char *params, int size);
	inline int id() const override { return 0x75; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
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
	explicit OpcodeOperation(const char *params, int size);
	explicit OpcodeOperation(const OpcodeBinaryOperation &op);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	inline bool isLong() const override { return false; }
};

class OpcodeOperation2 : public OpcodeBinaryOperation {
public:
	explicit OpcodeOperation2(const char *params, int size);
	explicit OpcodeOperation2(const OpcodeBinaryOperation &op);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	inline bool isLong() const override { return true; }
};

class OpcodeUnaryOperation : public Opcode {
public:
	explicit OpcodeUnaryOperation(const char *params, int size);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	virtual void getVariables(QList<FF7Var> &vars) const override;
	virtual bool isLong() const { return false; }
	quint8 banks, var;
};

class OpcodeUnaryOperation2 : public OpcodeUnaryOperation {
public:
	explicit OpcodeUnaryOperation2(const char *params, int size);
	explicit OpcodeUnaryOperation2(const OpcodeUnaryOperation &other);
	void getVariables(QList<FF7Var> &vars) const override;
	bool isLong() const override { return true; }
};

class OpcodePLUSX : public OpcodeOperation {
public:
	explicit OpcodePLUSX(const char *params, int size);
	explicit OpcodePLUSX(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x76; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodePLUS2X : public OpcodeOperation2 {
public:
	explicit OpcodePLUS2X(const char *params, int size);
	explicit OpcodePLUS2X(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x77; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMINUSX : public OpcodeOperation {
public:
	explicit OpcodeMINUSX(const char *params, int size);
	explicit OpcodeMINUSX(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x78; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMINUS2X : public OpcodeOperation2 {
public:
	explicit OpcodeMINUS2X(const char *params, int size);
	explicit OpcodeMINUS2X(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x79; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeINCX : public OpcodeUnaryOperation {
public:
	explicit OpcodeINCX(const char *params, int size);
	explicit OpcodeINCX(const OpcodeUnaryOperation &op);
	inline int id() const override { return 0x7A; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeINC2X : public OpcodeUnaryOperation2 {
public:
	explicit OpcodeINC2X(const char *params, int size);
	explicit OpcodeINC2X(const OpcodeUnaryOperation &op);
	inline int id() const override { return 0x7B; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeDECX : public OpcodeUnaryOperation {
public:
	explicit OpcodeDECX(const char *params, int size);
	explicit OpcodeDECX(const OpcodeUnaryOperation &op);
	inline int id() const override { return 0x7C; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeDEC2X : public OpcodeUnaryOperation2 {
public:
	explicit OpcodeDEC2X(const char *params, int size);
	explicit OpcodeDEC2X(const OpcodeUnaryOperation &op);
	inline int id() const override { return 0x7D; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeTLKON : public Opcode {
public:
	explicit OpcodeTLKON(const char *params, int size);
	inline int id() const override { return 0x7E; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 disabled;
};

class OpcodeRDMSD : public Opcode {
public:
	explicit OpcodeRDMSD(const char *params, int size);
	inline int id() const override { return 0x7F; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 banks, value;
};

class OpcodeSETBYTE : public OpcodeOperation {
public:
	explicit OpcodeSETBYTE(const char *params, int size);
	explicit OpcodeSETBYTE(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x80; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeSETWORD : public OpcodeOperation2 {
public:
	explicit OpcodeSETWORD(const char *params, int size);
	explicit OpcodeSETWORD(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x81; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeBitOperation : public Opcode {
public:
	explicit OpcodeBitOperation(const char *params, int size);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, var, position;
};

class OpcodeBITON : public OpcodeBitOperation {
public:
	explicit OpcodeBITON(const char *params, int size);
	explicit OpcodeBITON(const OpcodeBitOperation &op);
	inline int id() const override { return 0x82; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeBITOFF : public OpcodeBitOperation {
public:
	explicit OpcodeBITOFF(const char *params, int size);
	explicit OpcodeBITOFF(const OpcodeBitOperation &op);
	inline int id() const override { return 0x83; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeBITXOR : public OpcodeBitOperation {
public:
	explicit OpcodeBITXOR(const char *params, int size);
	explicit OpcodeBITXOR(const OpcodeBitOperation &op);
	inline int id() const override { return 0x84; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodePLUS : public OpcodeOperation {
public:
	explicit OpcodePLUS(const char *params, int size);
	explicit OpcodePLUS(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x85; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodePLUS2 : public OpcodeOperation2 {
public:
	explicit OpcodePLUS2(const char *params, int size);
	explicit OpcodePLUS2(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x86; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMINUS : public OpcodeOperation {
public:
	explicit OpcodeMINUS(const char *params, int size);
	explicit OpcodeMINUS(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x87; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMINUS2 : public OpcodeOperation2 {
public:
	explicit OpcodeMINUS2(const char *params, int size);
	explicit OpcodeMINUS2(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x88; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMUL : public OpcodeOperation {
public:
	explicit OpcodeMUL(const char *params, int size);
	explicit OpcodeMUL(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x89; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMUL2 : public OpcodeOperation2 {
public:
	explicit OpcodeMUL2(const char *params, int size);
	explicit OpcodeMUL2(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x8A; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeDIV : public OpcodeOperation {
public:
	explicit OpcodeDIV(const char *params, int size);
	explicit OpcodeDIV(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x8B; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeDIV2 : public OpcodeOperation2 {
public:
	explicit OpcodeDIV2(const char *params, int size);
	explicit OpcodeDIV2(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x8C; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMOD : public OpcodeOperation {
public:
	explicit OpcodeMOD(const char *params, int size);
	explicit OpcodeMOD(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x8D; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMOD2 : public OpcodeOperation2 {
public:
	explicit OpcodeMOD2(const char *params, int size);
	explicit OpcodeMOD2(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x8E; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeAND : public OpcodeOperation {
public:
	explicit OpcodeAND(const char *params, int size);
	explicit OpcodeAND(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x8F; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeAND2 : public OpcodeOperation2 {
public:
	explicit OpcodeAND2(const char *params, int size);
	explicit OpcodeAND2(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x90; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeOR : public OpcodeOperation {
public:
	explicit OpcodeOR(const char *params, int size);
	explicit OpcodeOR(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x91; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeOR2 : public OpcodeOperation2 {
public:
	explicit OpcodeOR2(const char *params, int size);
	explicit OpcodeOR2(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x92; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeXOR : public OpcodeOperation {
public:
	explicit OpcodeXOR(const char *params, int size);
	explicit OpcodeXOR(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x93; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeXOR2 : public OpcodeOperation2 {
public:
	explicit OpcodeXOR2(const char *params, int size);
	explicit OpcodeXOR2(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x94; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeINC : public OpcodeUnaryOperation {
public:
	explicit OpcodeINC(const char *params, int size);
	explicit OpcodeINC(const OpcodeUnaryOperation &op);
	inline int id() const override { return 0x95; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeINC2 : public OpcodeUnaryOperation2 {
public:
	explicit OpcodeINC2(const char *params, int size);
	explicit OpcodeINC2(const OpcodeUnaryOperation &op);
	inline int id() const override { return 0x96; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeDEC : public OpcodeUnaryOperation {
public:
	explicit OpcodeDEC(const char *params, int size);
	explicit OpcodeDEC(const OpcodeUnaryOperation &op);
	inline int id() const override { return 0x97; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeDEC2 : public OpcodeUnaryOperation2 {
public:
	explicit OpcodeDEC2(const char *params, int size);
	explicit OpcodeDEC2(const OpcodeUnaryOperation &op);
	inline int id() const override { return 0x98; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeRANDOM : public OpcodeUnaryOperation {
public:
	explicit OpcodeRANDOM(const char *params, int size);
	explicit OpcodeRANDOM(const OpcodeUnaryOperation &op);
	inline int id() const override { return 0x99; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeLBYTE : public OpcodeOperation {
public:
	explicit OpcodeLBYTE(const char *params, int size);
	explicit OpcodeLBYTE(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x9A; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeHBYTE : public OpcodeOperation2 {
public:
	explicit OpcodeHBYTE(const char *params, int size);
	explicit OpcodeHBYTE(const OpcodeBinaryOperation &op);
	inline int id() const override { return 0x9B; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class Opcode2BYTE : public Opcode {
public:
	explicit Opcode2BYTE(const char *params, int size);
	inline int id() const override { return 0x9C; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2], var, value1, value2;
};

class OpcodeSETX : public Opcode {
public:
	explicit OpcodeSETX(const char *params, int size);
	inline int id() const override { return 0x9D; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 unknown[6];
};

class OpcodeGETX : public Opcode {
public:
	explicit OpcodeGETX(const char *params, int size);
	inline int id() const override { return 0x9E; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 unknown[6];
};

class OpcodeSEARCHX : public Opcode {
public:
	explicit OpcodeSEARCHX(const char *params, int size);
	inline int id() const override { return 0x9F; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[3], searchStart;
	quint16 start, end;
	quint8 value, varResult;
};

class OpcodePC : public Opcode {
public:
	explicit OpcodePC(const char *params, int size);
	explicit OpcodePC(quint8 charID);
	inline int id() const override { return 0xA0; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 charID;
};

class OpcodeCHAR : public Opcode {
public:
	explicit OpcodeCHAR(const char *params, int size);
	explicit OpcodeCHAR(quint8 objectID);
	inline int id() const override { return 0xA1; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 objectID;
};

class OpcodeDFANM : public Opcode {
public:
	explicit OpcodeDFANM(const char *params, int size);
	inline int id() const override { return 0xA2; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 animID, speed;
};
// note: same struct as DFANM
class OpcodeANIME1 : public Opcode {
public:
	explicit OpcodeANIME1(const char *params, int size);
	inline int id() const override { return 0xA3; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 animID, speed;
};

class OpcodeVISI : public Opcode {
public:
	explicit OpcodeVISI(const char *params, int size);
	inline int id() const override { return 0xA4; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 show;
};

class OpcodeXYZI : public Opcode {
public:
	explicit OpcodeXYZI(const char *params, int size);
	inline int id() const override { return 0xA5; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	void listModelPositions(QList<FF7Position> &positions) const override;
	quint8 banks[2];
	qint16 targetX, targetY, targetZ;
	quint16 targetI;
};

class OpcodeXYI : public Opcode {
public:
	explicit OpcodeXYI(const char *params, int size);
	inline int id() const override { return 0xA6; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	void listModelPositions(QList<FF7Position> &positions) const override;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 targetI;
};

class OpcodeXYZ : public Opcode {
public:
	explicit OpcodeXYZ(const char *params, int size);
	inline int id() const override { return 0xA7; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	void listModelPositions(QList<FF7Position> &positions) const override;
	quint8 banks[2];
	qint16 targetX, targetY, targetZ;
};

class OpcodeMOVE : public Opcode {
public:
	explicit OpcodeMOVE(const char *params, int size);
	inline int id() const override { return 0xA8; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	qint16 targetX, targetY;
	quint8 banks;
};
// note: same struct as MOVE
class OpcodeCMOVE : public Opcode {
public:
	explicit OpcodeCMOVE(const char *params, int size);
	inline int id() const override { return 0xA9; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	qint16 targetX, targetY;
	quint8 banks;
};

class OpcodeMOVA : public Opcode {
public:
	explicit OpcodeMOVA(const char *params, int size);
	inline int id() const override { return 0xAA; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline virtual int getGroupID() const override { return groupID; }
	inline virtual void setGroupID(quint8 groupID) override { this->groupID = groupID; }
	quint8 groupID;
};

class OpcodeTURA : public Opcode {
public:
	explicit OpcodeTURA(const char *params, int size);
	inline int id() const override { return 0xAB; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline virtual int getGroupID() const override { return groupID; }
	inline virtual void setGroupID(quint8 groupID) override { this->groupID = groupID; }
	quint8 groupID, directionRotation, speed;
};

class OpcodeANIMW : public Opcode {
public:
	explicit OpcodeANIMW();
	inline int id() const override { return 0xAC; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeFMOVE : public Opcode {
public:
	explicit OpcodeFMOVE(const char *params, int size);
	inline int id() const override { return 0xAD; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	qint16 targetX, targetY;
	quint8 banks;
};

class OpcodeANIME2 : public Opcode {
public:
	explicit OpcodeANIME2(const char *params, int size);
	inline int id() const override { return 0xAE; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 animID, speed;
};
// note: same struct as ANIME2
class OpcodeANIMX1 : public Opcode {
public:
	explicit OpcodeANIMX1(const char *params, int size);
	inline int id() const override { return 0xAF; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 animID, speed;
};

class OpcodeCANIM1 : public Opcode {
public:
	explicit OpcodeCANIM1(const char *params, int size);
	inline int id() const override { return 0xB0; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 animID, firstFrame, lastFrame, speed;
};
// note: same struct as CANIM1
class OpcodeCANMX1 : public Opcode {
public:
	explicit OpcodeCANMX1(const char *params, int size);
	inline int id() const override { return 0xB1; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 animID, firstFrame, lastFrame, speed;
};

class OpcodeMSPED : public Opcode {
public:
	explicit OpcodeMSPED(const char *params, int size);
	inline int id() const override { return 0xB2; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint16 speed;
	quint8 banks;
};

class OpcodeDIR : public Opcode {
public:
	explicit OpcodeDIR(const char *params, int size);
	inline int id() const override { return 0xB3; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, direction;
};

class OpcodeTURNGEN : public Opcode {
public:
	explicit OpcodeTURNGEN(const char *params, int size);
	inline int id() const override { return 0xB4; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, direction, turnCount, speed, unknown;
};
// note: same struct as TURNGEN
class OpcodeTURN : public Opcode {
public:
	explicit OpcodeTURN(const char *params, int size);
	inline int id() const override { return 0xB5; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, direction, turnCount, speed, unknown;
};
// note: same struct as MOVA
class OpcodeDIRA : public Opcode {
public:
	explicit OpcodeDIRA(const char *params, int size);
	inline int id() const override { return 0xB6; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline virtual int getGroupID() const override { return groupID; }
	inline virtual void setGroupID(quint8 groupID) override { this->groupID = groupID; }
	quint8 groupID;
};

class OpcodeGETDIR : public Opcode {
public:
	explicit OpcodeGETDIR(const char *params, int size);
	inline int id() const override { return 0xB7; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	inline virtual int getGroupID() const override { return groupID; }
	inline virtual void setGroupID(quint8 groupID) override { this->groupID = groupID; }
	quint8 banks, groupID, varDir;
};

class OpcodeGETAXY : public Opcode {
public:
	explicit OpcodeGETAXY(const char *params, int size);
	inline int id() const override { return 0xB8; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	inline virtual int getGroupID() const override { return groupID; }
	inline virtual void setGroupID(quint8 groupID) override { this->groupID = groupID; }
	quint8 banks, groupID, varX, varY;
};

class OpcodeGETAI : public Opcode {
public:
	explicit OpcodeGETAI(const char *params, int size);
	inline int id() const override { return 0xB9; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	inline virtual int getGroupID() const override { return groupID; }
	inline virtual void setGroupID(quint8 groupID) override { this->groupID = groupID; }
	quint8 banks, groupID, varI;
};
// note: same struct as ANIME2
class OpcodeANIMX2 : public Opcode {
public:
	explicit OpcodeANIMX2(const char *params, int size);
	inline int id() const override { return 0xBA; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 animID, speed;
};
// note: same struct as CANIM1
class OpcodeCANIM2 : public Opcode {
public:
	explicit OpcodeCANIM2(const char *params, int size);
	inline int id() const override { return 0xBB; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 animID, firstFrame, lastFrame, speed;
};
// note: same struct as CANIM1
class OpcodeCANMX2 : public Opcode {
public:
	explicit OpcodeCANMX2(const char *params, int size);
	inline int id() const override { return 0xBC; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 animID, firstFrame, lastFrame, speed;
};

class OpcodeASPED : public Opcode {
public:
	explicit OpcodeASPED(const char *params, int size);
	inline int id() const override { return 0xBD; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint16 speed;
	quint8 banks;
};
// note: same struct as MOVA
class OpcodeCC : public Opcode {
public:
	explicit OpcodeCC(const char *params, int size);
	inline int id() const override { return 0xBF; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline virtual int getGroupID() const override { return groupID; }
	inline virtual void setGroupID(quint8 groupID) override { this->groupID = groupID; }
	quint8 groupID;
};

class OpcodeJUMP : public Opcode {
public:
	explicit OpcodeJUMP(const char *params, int size);
	inline int id() const override { return 0xC0; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2];
	qint16 targetX, targetY;
	quint16 targetI;
	qint16 height;
};

class OpcodeAXYZI : public Opcode {
public:
	explicit OpcodeAXYZI(const char *params, int size);
	inline int id() const override { return 0xC1; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	inline virtual int getGroupID() const override { return groupID; }
	inline virtual void setGroupID(quint8 groupID) override { this->groupID = groupID; }
	quint8 banks[2], groupID, varX, varY, varZ, varI;
};

class OpcodeLADER : public Opcode {
public:
	explicit OpcodeLADER(const char *params, int size);
	inline int id() const override { return 0xC2; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2];
	qint16 targetX, targetY, targetZ;
	quint16 targetI;
	quint8 way, animID, direction, speed;
};

class OpcodeOFST : public Opcode {
public:
	explicit OpcodeOFST(const char *params, int size);
	inline int id() const override { return 0xC3; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	qint16 targetX, targetY, targetZ;
	quint16 speed;
	quint8 banks[2], moveType;
};

class OpcodeOFSTW : public Opcode {
public:
	explicit OpcodeOFSTW();
	inline int id() const override { return 0xC4; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeTALKR : public Opcode {
public:
	explicit OpcodeTALKR(const char *params, int size);
	inline int id() const override { return 0xC5; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, distance;
};
// note: same struct as TALKR
class OpcodeSLIDR : public Opcode {
public:
	explicit OpcodeSLIDR(const char *params, int size);
	inline int id() const override { return 0xC6; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, distance;
};

class OpcodeSOLID : public Opcode {
public:
	explicit OpcodeSOLID(const char *params, int size);
	inline int id() const override { return 0xC7; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 disabled;
};
// note: same struct as PC
class OpcodePRTYP : public Opcode {
public:
	explicit OpcodePRTYP(const char *params, int size);
	inline int id() const override { return 0xC8; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 charID;
};
// note: same struct as PC
class OpcodePRTYM : public Opcode {
public:
	explicit OpcodePRTYM(const char *params, int size);
	inline int id() const override { return 0xC9; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 charID;
};

class OpcodePRTYE : public Opcode {
public:
	explicit OpcodePRTYE(const char *params, int size);
	inline int id() const override { return 0xCA; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 charID[3];
};

class OpcodeIfQ : public OpcodeJump {
public:
	explicit OpcodeIfQ(const char *params, int size);
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	inline bool isLongJump() const override { return false; }
	inline quint8 jumpPosData() const override { return 2; }
	inline virtual bool isIf() const override { return true; }
	quint8 charID;
};

class OpcodeIFPRTYQ : public OpcodeIfQ {
public:
	explicit OpcodeIFPRTYQ(const char *params, int size);
	inline int id() const override { return 0xCB; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeIFMEMBQ : public OpcodeIfQ {
public:
	explicit OpcodeIFMEMBQ(const char *params, int size);
	inline int id() const override { return 0xCC; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMMBUD : public Opcode {
public:
	explicit OpcodeMMBUD(const char *params, int size);
	inline int id() const override { return 0xCD; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 exists, charID;
};
// note: same struct as PC
class OpcodeMMBLK : public Opcode {
public:
	explicit OpcodeMMBLK(const char *params, int size);
	inline int id() const override { return 0xCE; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 charID;
};
// note: same struct as PC
class OpcodeMMBUK : public Opcode {
public:
	explicit OpcodeMMBUK(const char *params, int size);
	inline int id() const override { return 0xCF; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 charID;
};

class OpcodeLINE : public Opcode {
public:
	explicit OpcodeLINE(const char *params, int size);
	inline int id() const override { return 0xD0; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	bool linePosition(FF7Position position[2]) const override;
	qint16 targetX1, targetY1, targetZ1;
	qint16 targetX2, targetY2, targetZ2;
};

class OpcodeLINON : public Opcode {
public:
	explicit OpcodeLINON(const char *params, int size);
	inline int id() const override { return 0xD1; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 enabled;
};

class OpcodeMPJPO : public Opcode {
public:
	explicit OpcodeMPJPO(const char *params, int size);
	inline int id() const override { return 0xD2; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 prevent;
};

class OpcodeSLINE : public Opcode {
public:
	explicit OpcodeSLINE(const char *params, int size);
	inline int id() const override { return 0xD3; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	qint16 targetX1, targetY1, targetZ1;
	qint16 targetX2, targetY2, targetZ2;
	quint8 banks[3];
};

class OpcodeSIN : public Opcode {
public:
	explicit OpcodeSIN(const char *params, int size);
	inline int id() const override { return 0xD4; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2];
	quint16 value1, value2, value3;
	quint8 var;
};
// note: same struct as SIN
class OpcodeCOS : public Opcode {
public:
	explicit OpcodeCOS(const char *params, int size);
	inline int id() const override { return 0xD5; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2];
	quint16 value1, value2, value3;
	quint8 var;
};

class OpcodeTLKR2 : public Opcode {
public:
	explicit OpcodeTLKR2(const char *params, int size);
	inline int id() const override { return 0xD6; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint16 distance;
	quint8 banks;
};
// note: same struct as TLKR2
class OpcodeSLDR2 : public Opcode {
public:
	explicit OpcodeSLDR2(const char *params, int size);
	inline int id() const override { return 0xD7; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint16 distance;
	quint8 banks;
};

class OpcodePMJMP : public Opcode {
public:
	explicit OpcodePMJMP(const char *params, int size);
	inline int id() const override { return 0xD8; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint16 fieldID;
};

class OpcodePMJMP2 : public Opcode {
public:
	explicit OpcodePMJMP2();
	inline int id() const override { return 0xD9; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeAKAO2 : public Opcode {
public:
	explicit OpcodeAKAO2(const char *params, int size);
	inline int id() const override { return 0xDA; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	bool searchOpcode(int opcode) const override;
	quint8 banks[3], opcode;
	quint16 param1, param2, param3, param4, param5;
};

class OpcodeFCFIX : public Opcode {
public:
	explicit OpcodeFCFIX(const char *params, int size);
	inline int id() const override { return 0xDB; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 disabled;
};

class OpcodeCCANM : public Opcode {
public:
	explicit OpcodeCCANM(const char *params, int size);
	inline int id() const override { return 0xDC; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 animID, speed, standWalkRun;
};

class OpcodeANIMB : public Opcode {
public:
	explicit OpcodeANIMB();
	inline int id() const override { return 0xDD; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeTURNW : public Opcode {
public:
	explicit OpcodeTURNW();
	inline int id() const override { return 0xDE; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMPPAL : public Opcode {
public:
	explicit OpcodeMPPAL(const char *params, int size);
	inline int id() const override { return 0xDF; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[3], posSrc, posDst, start, b, g, r, colorCount;
};

class OpcodeBGON : public Opcode {
public:
	explicit OpcodeBGON(const char *params, int size);
	inline int id() const override { return 0xE0; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, paramID, stateID;
};
// note: same struct as BGON
class OpcodeBGOFF : public Opcode {
public:
	explicit OpcodeBGOFF(const char *params, int size);
	inline int id() const override { return 0xE1; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, paramID, stateID;
};

class OpcodeBGROL : public Opcode {
public:
	explicit OpcodeBGROL(const char *params, int size);
	inline int id() const override { return 0xE2; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, paramID;
};
// note: same struct as BGROL
class OpcodeBGROL2 : public Opcode {
public:
	explicit OpcodeBGROL2(const char *params, int size);
	inline int id() const override { return 0xE3; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, paramID;
};
// note: same struct as BGROL
class OpcodeBGCLR : public Opcode {
public:
	explicit OpcodeBGCLR(const char *params, int size);
	inline int id() const override { return 0xE4; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, paramID;
};

class OpcodeSTPAL : public Opcode {
public:
	explicit OpcodeSTPAL(const char *params, int size);
	inline int id() const override { return 0xE5; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, palID, position, colorCount;
};

class OpcodeLDPAL : public Opcode {
public:
	explicit OpcodeLDPAL(const char *params, int size);
	inline int id() const override { return 0xE6; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, position, palID, colorCount;
};

class OpcodeCPPAL : public Opcode {
public:
	explicit OpcodeCPPAL(const char *params, int size);
	inline int id() const override { return 0xE7; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, posSrc, posDst, colorCount;
};

class OpcodeRTPAL : public Opcode {
public:
	explicit OpcodeRTPAL(const char *params, int size);
	inline int id() const override { return 0xE8; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[2], posSrc, posDst, start, end;
};

class OpcodeADPAL : public Opcode {
public:
	explicit OpcodeADPAL(const char *params, int size);
	inline int id() const override { return 0xE9; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[3], posSrc, posDst;
	qint8 b, g, r;
	quint8 colorCount;
};
// note: same struct as ADPAL
class OpcodeMPPAL2 : public Opcode {
public:
	explicit OpcodeMPPAL2(const char *params, int size);
	inline int id() const override { return 0xEA; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks[3], posSrc, posDst, b, g, r, colorCount;
};

class OpcodeSTPLS : public Opcode {
public:
	explicit OpcodeSTPLS(const char *params, int size);
	inline int id() const override { return 0xEB; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 palID, posSrc, start, colorCount;
};

class OpcodeLDPLS : public Opcode {
public:
	explicit OpcodeLDPLS(const char *params, int size);
	inline int id() const override { return 0xEC; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 posSrc, palID, start, colorCount;
};

class OpcodeCPPAL2 : public Opcode {
public:
	explicit OpcodeCPPAL2(const char *params, int size);
	inline int id() const override { return 0xED; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 unknown[7];
};

class OpcodeRTPAL2 : public Opcode {
public:
	explicit OpcodeRTPAL2(const char *params, int size);
	inline int id() const override { return 0xEE; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 unknown[7];
};

class OpcodeADPAL2 : public Opcode {
public:
	explicit OpcodeADPAL2(const char *params, int size);
	inline int id() const override { return 0xEF; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 unknown[10];
};

class OpcodeMUSIC : public Opcode {
public:
	explicit OpcodeMUSIC(const char *params, int size);
	inline int id() const override { return 0xF0; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 musicID;
};

class OpcodeSOUND : public Opcode {
public:
	explicit OpcodeSOUND(const char *params, int size);
	inline int id() const override { return 0xF1; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint16 soundID;
	quint8 banks;
	quint8 position;
};

class OpcodeAKAO : public Opcode {
public:
	explicit OpcodeAKAO(const char *params, int size);
	inline int id() const override { return 0xF2; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	bool searchOpcode(int opcode) const override;
	quint16 param2, param3, param4, param5;
	quint8 banks[3], opcode, param1;
};
// note: same struct as MUSIC
class OpcodeMUSVT : public Opcode {
public:
	explicit OpcodeMUSVT(const char *params, int size);
	inline int id() const override { return 0xF3; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 musicID;
};
// note: same struct as MUSIC
class OpcodeMUSVM : public Opcode {
public:
	explicit OpcodeMUSVM(const char *params, int size);
	inline int id() const override { return 0xF4; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 musicID;
};

class OpcodeMULCK : public Opcode {
public:
	explicit OpcodeMULCK(const char *params, int size);
	inline int id() const override { return 0xF5; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 locked;
};
// note: same struct as MUSIC
class OpcodeBMUSC : public Opcode {
public:
	explicit OpcodeBMUSC(const char *params, int size);
	inline int id() const override { return 0xF6; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 musicID;
};

class OpcodeCHMPH : public Opcode {
public:
	explicit OpcodeCHMPH(const char *params, int size);
	inline int id() const override { return 0xF7; }
	QString toString(const Section1File *scriptsAndTexts) const override;//TODO: unknown
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, var1, var2;
};

class OpcodePMVIE : public Opcode {
public:
	explicit OpcodePMVIE(const char *params, int size);
	inline int id() const override { return 0xF8; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 movieID;
};

class OpcodeMOVIE : public Opcode {
public:
	explicit OpcodeMOVIE();
	inline int id() const override { return 0xF9; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};

class OpcodeMVIEF : public Opcode {
public:
	explicit OpcodeMVIEF(const char *params, int size);
	inline int id() const override { return 0xFA; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, varCurMovieFrame;
};

class OpcodeMVCAM : public Opcode {
public:
	explicit OpcodeMVCAM(const char *params, int size);
	inline int id() const override { return 0xFB; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 movieCamID;
};

class OpcodeFMUSC : public Opcode {
public:
	explicit OpcodeFMUSC(const char *params, int size);
	inline int id() const override { return 0xFC; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint8 musicID;
};

class OpcodeCMUSC : public Opcode {
public:
	explicit OpcodeCMUSC(const char *params, int size);
	inline int id() const override { return 0xFD; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	quint16 param1, param2;
	quint8 musicID, banks, opcode;
};
// same struct as unaryOperation
class OpcodeCHMST : public Opcode {
public:
	explicit OpcodeCHMST(const char *params, int size);
	inline int id() const override { return 0xFE; }
	QString toString(const Section1File *scriptsAndTexts) const override;
	void setParams(const char *params, int size) override;
	QByteArray params() const override;
	void getVariables(QList<FF7Var> &vars) const override;
	quint8 banks, var;
};

class OpcodeGAMEOVER : public Opcode {
public:
	explicit OpcodeGAMEOVER();
	inline int id() const override { return 0xFF; }
	QString toString(const Section1File *scriptsAndTexts) const override;
};
