/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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

#ifdef _MSC_VER
#	define PACK(...)            \
		__pragma(pack(push, 1)) \
		__VA_ARGS__             \
		__pragma(pack(pop))
#else
#	define PACK(...) __VA_ARGS__ Q_PACKED
#endif

#define B1(v)             ((v >> 4) & 0xF)
#define B2(v)             (v & 0xF)
#define BANK(b1, b2)      quint8(((b1 & 0xF) << 4) | (b2 & 0xF))
#define SCRIPT_ID(v)      (v & 0x1F)
#define PRIORITY(v)       ((v >> 5) & 0x7)
#define SCRIPT_AND_PRIORITY(s, p) \
	((s  & 0x1F) | quint8((p & 0x7) << 5))
#define OPERATORS_SIZE    11
#define NOWIN             255

struct FF7Window {
	qint16 x, y;
	quint16 w, h;
	quint16 ask_first, ask_last;
	quint8 type, mode, displayType;
	quint8 displayX, displayY;
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

struct FF7BinaryOperation {
	quint8 bank1;
	quint8 bank2;
	quint8 var; // bank 1
	quint16 value; // bank 2
	bool isLong;
};

struct FF7UnaryOperation {
	quint8 bank2;
	quint8 var; // bank 2
	bool isLong;
};

struct FF7BitOperation {
	quint8 bank1;
	quint8 bank2;
	quint8 var; // bank 1
	quint8 position; // bank 2
};

struct FF7Var {
	enum VarSize : quint8 {
		Byte, Word, SignedWord, Bit
	};
	enum Flag : quint8 {
		None = 0,
		Writable = 0x1
	};
	Q_DECLARE_FLAGS(Flags, Flag)

	FF7Var(qint8 b, quint8 a, VarSize size, Flags flags = None)
		: bank(quint8(b)), address(a), size(size), flags(flags) {
		Q_ASSERT(b > 0 && b <= 0xF);
	}
	quint8 bank;
	quint8 address;
	VarSize size;
	Flags flags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FF7Var::Flags)

constexpr inline quint8 operator|(FF7Var::VarSize size, FF7Var::Flag flag) noexcept {
	return quint8(size) | quint8(quint8(flag) << 3);
}

constexpr inline quint8 operator|(FF7Var::VarSize size, FF7Var::Flags flags) noexcept {
	return quint8(size) | quint8(quint8(flags) << 3);
}

inline bool operator==(const FF7Var &v1, const FF7Var &v2) {
	return v1.bank == v2.bank && v1.address == v2.address;
}

inline bool operator<(const FF7Var &v1, const FF7Var &v2) {
	if (v1.bank == v2.bank) {
		return v1.address < v2.address;
	}
	return v1.bank < v2.bank;
}

struct FF7If {
	quint8 bank1;
	quint8 bank2;
	qint32 value1; // bank 1
	qint32 value2; // bank 2
	quint8 oper;
	FF7Var::VarSize size;
};

class Section1File;

#define OPCODE_GENERATE_LIST \
	op_fun(RET)op_first op_sep op_fun(REQ)op_sep op_fun(REQSW)op_sep op_fun(REQEW)op_sep \
	op_fun(PREQ)op_sep op_fun(PRQSW)op_sep op_fun(PRQEW)op_sep op_fun(RETTO)op_sep \
	op_fun(JOIN)op_sep op_fun(SPLIT)op_sep op_fun(SPTYE)op_sep op_fun(GTPYE)op_sep \
	op_fun(Unused0C)op_sep op_fun(Unused0D)op_sep op_fun(DSKCG)op_sep op_fun(SPECIAL)op_sep \
 \
	op_fun(JMPF)op_sep op_fun(JMPFL)op_sep op_fun(JMPB)op_sep op_fun(JMPBL)op_sep \
	op_fun(IFUB)op_sep op_fun(IFUBL)op_sep op_fun(IFSW)op_sep op_fun(IFSWL)op_sep \
	op_fun(IFUW)op_sep op_fun(IFUWL)op_sep op_fun(Unused1A)op_sep op_fun(Unused1B)op_sep \
	op_fun(Unused1C)op_sep op_fun(Unused1D)op_sep op_fun(Unused1E)op_sep op_fun(Unused1F)op_sep \
 \
	op_fun(MINIGAME)op_sep op_fun(TUTOR)op_sep op_fun(BTMD2)op_sep op_fun(BTRLD)op_sep \
	op_fun(WAIT)op_sep op_fun(NFADE)op_sep op_fun(BLINK)op_sep op_fun(BGMOVIE)op_sep \
	op_fun(KAWAI)op_sep op_fun(KAWIW)op_sep op_fun(PMOVA)op_sep op_fun(SLIP)op_sep \
	op_fun(BGPDH)op_sep op_fun(BGSCR)op_sep op_fun(WCLS)op_sep op_fun(WSIZW)op_sep \
 \
	op_fun(IFKEY)op_sep op_fun(IFKEYON)op_sep op_fun(IFKEYOFF)op_sep op_fun(UC)op_sep \
	op_fun(PDIRA)op_sep op_fun(PTURA)op_sep op_fun(WSPCL)op_sep op_fun(WNUMB)op_sep \
	op_fun(STTIM)op_sep op_fun(GOLDu)op_sep op_fun(GOLDd)op_sep op_fun(CHGLD)op_sep \
	op_fun(HMPMAX1)op_sep op_fun(HMPMAX2)op_sep op_fun(MHMMX)op_sep op_fun(HMPMAX3)op_sep \
 \
	op_fun(MESSAGE)op_sep op_fun(MPARA)op_sep op_fun(MPRA2)op_sep op_fun(MPNAM)op_sep \
	op_fun(Unused44)op_sep op_fun(MPu)op_sep op_fun(Unused46)op_sep op_fun(MPd)op_sep \
	op_fun(ASK)op_sep op_fun(MENU)op_sep op_fun(MENU2)op_sep op_fun(BTLTB)op_sep \
	op_fun(Unused4C)op_sep op_fun(HPu)op_sep op_fun(Unused4E)op_sep op_fun(HPd)op_sep \
 \
	op_fun(WINDOW)op_sep op_fun(WMOVE)op_sep op_fun(WMODE)op_sep op_fun(WREST)op_sep \
	op_fun(WCLSE)op_sep op_fun(WROW)op_sep op_fun(GWCOL)op_sep op_fun(SWCOL)op_sep \
	op_fun(STITM)op_sep op_fun(DLITM)op_sep op_fun(CKITM)op_sep op_fun(SMTRA)op_sep \
	op_fun(DMTRA)op_sep op_fun(CMTRA)op_sep op_fun(SHAKE)op_sep op_fun(NOP)op_sep \
 \
	op_fun(MAPJUMP)op_sep op_fun(SCRLO)op_sep op_fun(SCRLC)op_sep op_fun(SCRLA)op_sep \
	op_fun(SCR2D)op_sep op_fun(SCRCC)op_sep op_fun(SCR2DC)op_sep op_fun(SCRLW)op_sep \
	op_fun(SCR2DL)op_sep op_fun(MPDSP)op_sep op_fun(VWOFT)op_sep op_fun(FADE)op_sep \
	op_fun(FADEW)op_sep op_fun(IDLCK)op_sep op_fun(LSTMP)op_sep op_fun(SCRLP)op_sep \
 \
	op_fun(BATTLE)op_sep op_fun(BTLON)op_sep op_fun(BTLMD)op_sep op_fun(PGTDR)op_sep \
	op_fun(GETPC)op_sep op_fun(PXYZI)op_sep op_fun(PLUSX)op_sep op_fun(PLUS2X)op_sep \
	op_fun(MINUSX)op_sep op_fun(MINUS2X)op_sep op_fun(INCX)op_sep op_fun(INC2X)op_sep \
	op_fun(DECX)op_sep op_fun(DEC2X)op_sep op_fun(TLKON)op_sep op_fun(RDMSD)op_sep \
 \
	op_fun(SETBYTE)op_sep op_fun(SETWORD)op_sep op_fun(BITON)op_sep op_fun(BITOFF)op_sep \
	op_fun(BITXOR)op_sep op_fun(PLUS)op_sep op_fun(PLUS2)op_sep op_fun(MINUS)op_sep \
	op_fun(MINUS2)op_sep op_fun(MUL)op_sep op_fun(MUL2)op_sep op_fun(DIV)op_sep \
	op_fun(DIV2)op_sep op_fun(MOD)op_sep op_fun(MOD2)op_sep op_fun(AND)op_sep \
 \
	op_fun(AND2)op_sep op_fun(OR)op_sep op_fun(OR2)op_sep op_fun(XOR)op_sep \
	op_fun(XOR2)op_sep op_fun(INC)op_sep op_fun(INC2)op_sep op_fun(DEC)op_sep \
	op_fun(DEC2)op_sep op_fun(RANDOM)op_sep op_fun(LBYTE)op_sep op_fun(HBYTE)op_sep \
	op_fun(TOBYTE)op_sep op_fun(SETX)op_sep op_fun(GETX)op_sep op_fun(SEARCHX)op_sep \
 \
	op_fun(PC)op_sep op_fun(CHAR_)op_sep op_fun(DFANM)op_sep op_fun(ANIME1)op_sep \
	op_fun(VISI)op_sep op_fun(XYZI)op_sep op_fun(XYI)op_sep op_fun(XYZ)op_sep \
	op_fun(MOVE)op_sep op_fun(CMOVE)op_sep op_fun(MOVA)op_sep op_fun(TURA)op_sep \
	op_fun(ANIMW)op_sep op_fun(FMOVE)op_sep op_fun(ANIME2)op_sep op_fun(ANIMX1)op_sep \
 \
	op_fun(CANIM1)op_sep op_fun(CANMX1)op_sep op_fun(MSPED)op_sep op_fun(DIR)op_sep \
	op_fun(TURNGEN)op_sep op_fun(TURN)op_sep op_fun(DIRA)op_sep op_fun(GETDIR)op_sep \
	op_fun(GETAXY)op_sep op_fun(GETAI)op_sep op_fun(ANIMX2)op_sep op_fun(CANIM2)op_sep \
	op_fun(CANMX2)op_sep op_fun(ASPED)op_sep op_fun(UnusedBE)op_sep op_fun(CC)op_sep \
 \
	op_fun(JUMP)op_sep op_fun(AXYZI)op_sep op_fun(LADER)op_sep op_fun(OFST)op_sep \
	op_fun(OFSTW)op_sep op_fun(TALKR)op_sep op_fun(SLIDR)op_sep op_fun(SOLID)op_sep \
	op_fun(PRTYP)op_sep op_fun(PRTYM)op_sep op_fun(PRTYE)op_sep op_fun(IFPRTYQ)op_sep \
	op_fun(IFMEMBQ)op_sep op_fun(MMBud)op_sep op_fun(MMBLK)op_sep op_fun(MMBUK)op_sep \
 \
	op_fun(LINE)op_sep op_fun(LINON)op_sep op_fun(MPJPO)op_sep op_fun(SLINE)op_sep \
	op_fun(SIN)op_sep op_fun(COS)op_sep op_fun(TLKR2)op_sep op_fun(SLDR2)op_sep \
	op_fun(PMJMP)op_sep op_fun(PMJMP2)op_sep op_fun(AKAO2)op_sep op_fun(FCFIX)op_sep \
	op_fun(CCANM)op_sep op_fun(ANIMB)op_sep op_fun(TURNW)op_sep op_fun(MPPAL)op_sep \
 \
	op_fun(BGON)op_sep op_fun(BGOFF)op_sep op_fun(BGROL)op_sep op_fun(BGROL2)op_sep \
	op_fun(BGCLR)op_sep op_fun(STPAL)op_sep op_fun(LDPAL)op_sep op_fun(CPPAL)op_sep \
	op_fun(RTPAL)op_sep op_fun(ADPAL)op_sep op_fun(MPPAL2)op_sep op_fun(STPLS)op_sep \
	op_fun(LDPLS)op_sep op_fun(CPPAL2)op_sep op_fun(RTPAL2)op_sep op_fun(ADPAL2)op_sep \
 \
	op_fun(MUSIC)op_sep op_fun(SOUND)op_sep op_fun(AKAO)op_sep op_fun(MUSVT)op_sep \
	op_fun(MUSVM)op_sep op_fun(MULCK)op_sep op_fun(BMUSC)op_sep op_fun(CHMPH)op_sep \
	op_fun(PMVIE)op_sep op_fun(MOVIE)op_sep op_fun(MVIEF)op_sep op_fun(MVCAM)op_sep \
	op_fun(FMUSC)op_sep op_fun(CMUSC)op_sep op_fun(CHMST)op_sep op_fun(GAMEOVER)op_sep \
	op_fun(LABEL)op_sep

#define op_fun(name) name
#define op_sep ,
#define op_first = 0

enum OpcodeKey : quint16 {
	OPCODE_GENERATE_LIST
};

#undef op_fun
#undef op_sep
#undef op_first
#define op_first op_sep

#define OPCODE_GENERATE_SPECIAL_LIST \
	op_fun(ARROW)op_first \
	op_fun(PNAME)op_sep \
	op_fun(GMSPD)op_sep \
	op_fun(SMSPD)op_sep \
	op_fun(FLMAT)op_sep \
	op_fun(FLITM)op_sep \
	op_fun(BTLCK)op_sep \
	op_fun(MVLCK)op_sep \
	op_fun(SPCNM)op_sep \
	op_fun(RSGLB)op_sep \
	op_fun(CLITM)op_sep

enum OpcodeSpecialKey : quint8 {
	ARROW = 0xF5,
	PNAME = 0xF6,
	GMSPD = 0xF7,
	SMSPD = 0xF8,
	FLMAT = 0xF9,
	FLITM = 0xFA,
	BTLCK = 0xFB,
	MVLCK = 0xFC,
	SPCNM = 0xFD,
	RSGLB = 0xFE,
	CLITM = 0xFF
};

#define OPCODE_GENERATE_KAWAI_LIST \
	op_fun(EYETX)op_first \
	op_fun(TRNSP)op_sep \
	op_fun(AMBNT)op_sep \
	op_fun(UNKNOWN4)op_sep \
	op_fun(LIGHT)op_sep \
	op_fun(UNKNOWN7)op_sep \
	op_fun(UNKNOWN8)op_sep \
	op_fun(UNKNOWN9)op_sep \
	op_fun(SBOBJ)op_sep \
	op_fun(UNKNOWNB)op_sep \
	op_fun(UNKNOWNC)op_sep \
	op_fun(SHINE)op_sep \
	op_fun(RESET)op_sep

enum OpcodeKawaiKey : quint8 {
	EYETX = 0x00,
	TRNSP = 0x01,
	AMBNT = 0x02,
	UNKNOWN4 = 0x04,
	LIGHT = 0x06,
	UNKNOWN7 = 0x07,
	UNKNOWN8 = 0x08,
	UNKNOWN9 = 0x09,
	SBOBJ = 0x0A,
	UNKNOWNB = 0x0B,
	UNKNOWNC = 0x0C,
	SHINE = 0x0D,
	RESET = 0xFF
};

enum BadJumpError : quint8 {
	Ok,
	InsideInstruction,
	AfterScript,
	BeforeScript,
	ImpossibleBackward,
	ImpossibleLong
};

PACK(struct OpcodeBase {
	OpcodeKey id;
});

PACK(struct OpcodeRET : public OpcodeBase {});

PACK(struct OpcodeExec : public OpcodeBase {
	quint8 groupID;
	quint8 scriptIDAndPriority; // scriptID = scriptIDAndPriority & 0x1F, priority = (scriptIDAndPriority >> 5) & 7
});

PACK(struct OpcodeREQ : public OpcodeExec {});
PACK(struct OpcodeREQSW : public OpcodeExec {});
PACK(struct OpcodeREQEW : public OpcodeExec {});

PACK(struct OpcodeExecChar : public OpcodeBase {
	quint8 partyID;
	quint8 scriptIDAndPriority; // scriptID = scriptIDAndPriority & 0x1F, priority = (scriptIDAndPriority >> 5) & 7
});

PACK(struct OpcodePREQ : public OpcodeExecChar {});
PACK(struct OpcodePRQSW : public OpcodeExecChar {});
PACK(struct OpcodePRQEW : public OpcodeExecChar {});

PACK(struct OpcodeRETTO : public OpcodeBase {
	quint8 scriptIDAndPriority; // scriptID = scriptIDAndPriority & 0x1F, priority = (scriptIDAndPriority >> 5) & 7
});

PACK(struct OpcodeJOIN : public OpcodeBase {
	quint8 speed;
});

PACK(struct OpcodeSPLIT : public OpcodeBase {
	quint8 banks[3];
	qint16 targetX1; // bank 1 16-bit
	qint16 targetY1; // bank 2 16-bit
	quint8 direction1; // bank 3 8-bit
	qint16 targetX2; // bank 4 16-bit
	qint16 targetY2; // bank 5 16-bit
	quint8 direction2; // bank 6 8-bit
	quint8 speed;
});

PACK(struct OpcodeSPTYE : public OpcodeBase {
	quint8 banks[2];
	quint8 charID1; // bank 1 8-bit
	quint8 charID2; // bank 2 8-bit
	quint8 charID3; // bank 3 8-bit
});

PACK(struct OpcodeGTPYE : public OpcodeBase {
	quint8 banks[2];
	quint8 varCharID1; // bank 1 8-bit
	quint8 varCharID2; // bank 2 8-bit
	quint8 varCharID3; // bank 3 8-bit
});

PACK(struct OpcodeUnused0C : public OpcodeBase {});
PACK(struct OpcodeUnused0D : public OpcodeBase {});

PACK(struct OpcodeDSKCG : public OpcodeBase {
	quint8 diskID;
});

PACK(struct OpcodeSPECIAL : public OpcodeBase {
	quint8 subKey;
});

PACK(struct OpcodeSPECIALARROW : public OpcodeSPECIAL {
	quint8 disabled;
});

// FIXME: 4 parameters, not 1
PACK(struct OpcodeSPECIALPNAME : public OpcodeSPECIAL {
	quint8 banks; // bank 2
	quint8 varOrValue; // bank 1 8-bit
	quint8 unused;
	quint8 size;
});

// FIXME
PACK(struct OpcodeSPECIALGMSPD : public OpcodeSPECIAL {
	quint8 banks;
	quint8 varSpeed; // bank 2 8-bit
});

// FIXME
PACK(struct OpcodeSPECIALSMSPD : public OpcodeSPECIAL {
	quint8 banks;
	quint8 speed; // bank 2 8-bit
});

PACK(struct OpcodeSPECIALFLMAT : public OpcodeSPECIAL {});
PACK(struct OpcodeSPECIALFLITM : public OpcodeSPECIAL {});

PACK(struct OpcodeSpecialLock : public OpcodeSPECIAL {
	quint8 lock;
});

PACK(struct OpcodeSPECIALBTLCK : public OpcodeSpecialLock {});
PACK(struct OpcodeSPECIALMVLCK : public OpcodeSpecialLock {});

PACK(struct OpcodeSPECIALSPCNM : public OpcodeSPECIAL {
	quint8 charID;
	quint8 textID;
});

PACK(struct OpcodeSPECIALRSGLB : public OpcodeSPECIAL {});
PACK(struct OpcodeSPECIALCLITM : public OpcodeSPECIAL {});

PACK(struct OpcodeShortJump : public OpcodeBase {
	quint8 jump;
	quint16 _label;
	quint8 _badJump;
});

PACK(struct OpcodeLongJump : public OpcodeBase {
	quint16 jump;
	quint16 _label;
	quint8 _badJump;
});

PACK(struct OpcodeJMPF : public OpcodeShortJump {});
PACK(struct OpcodeJMPFL : public OpcodeLongJump {});
PACK(struct OpcodeJMPB : public OpcodeShortJump {});
PACK(struct OpcodeJMPBL : public OpcodeLongJump {});

PACK(struct OpcodeIFUB : public OpcodeBase {
	quint8 banks;
	quint8 value1; // bank 1
	quint8 value2; // bank 2
	quint8 oper;
	quint8 jump;
	quint16 _label;
	quint8 _badJump;
});

PACK(struct OpcodeIFUBL : public OpcodeBase {
	quint8 banks;
	quint8 value1; // bank 1
	quint8 value2; // bank 2
	quint8 oper;
	quint16 jump;
	quint16 _label;
	quint8 _badJump;
});

PACK(struct OpcodeIFSW : public OpcodeBase {
	quint8 banks;
	qint16 value1; // bank 1
	qint16 value2; // bank 2
	quint8 oper;
	quint8 jump;
	quint16 _label;
	quint8 _badJump;
});

PACK(struct OpcodeIFSWL : public OpcodeBase {
	quint8 banks;
	qint16 value1; // bank 1
	qint16 value2; // bank 2
	quint8 oper;
	quint16 jump;
	quint16 _label;
	quint8 _badJump;
});

PACK(struct OpcodeIFUW : public OpcodeBase {
	quint8 banks;
	quint16 value1; // bank 1
	quint16 value2; // bank 2
	quint8 oper;
	quint8 jump;
	quint16 _label;
	quint8 _badJump;
});

PACK(struct OpcodeIFUWL : public OpcodeBase {
	quint8 banks;
	quint16 value1; // bank 1
	quint16 value2; // bank 2
	quint8 oper;
	quint16 jump;
	quint16 _label;
	quint8 _badJump;
});

PACK(struct OpcodeUnused1A : public OpcodeBase {
	quint16 from;
	quint16 to;
	quint32 absValue;
	quint8 flag;
});

PACK(struct OpcodeUnused1B : public OpcodeLongJump {});
PACK(struct OpcodeUnused1C : public OpcodeBase {
	quint32 address;
	quint8 subSize;
	QByteArray *_data;
});

PACK(struct OpcodeUnused1D : public OpcodeBase {});
PACK(struct OpcodeUnused1E : public OpcodeBase {});
PACK(struct OpcodeUnused1F : public OpcodeBase {});

PACK(struct OpcodeMINIGAME : public OpcodeBase {
	quint16 mapID;
	qint16 targetX;
	qint16 targetY;
	quint16 targetI;
	quint8 minigameParam;
	quint8 minigameID;
});

PACK(struct OpcodeTUTOR : public OpcodeBase {
	quint8 tutoID;
});

PACK(struct OpcodeBTMD2 : public OpcodeBase {
	quint32 battleMode;
});

PACK(struct OpcodeBTRLD : public OpcodeBase {
	quint8 banks;
	quint8 var; // bank 2
});

PACK(struct OpcodeWAIT : public OpcodeBase {
	quint16 frameCount;
});

// FIXME: speed is 16 bits
PACK(struct OpcodeNFADE : public OpcodeBase {
	quint8 banks[2];
	quint8 type;
	quint8 r; // bank 1
	quint8 g; // bank 2
	quint8 b; // bank 3
	quint16 speed; // bank 4
});

PACK(struct OpcodeBLINK : public OpcodeBase {
	quint8 closed;
});

PACK(struct OpcodeDisable : public OpcodeBase {
	quint8 disabled;
});

PACK(struct OpcodeEnable : public OpcodeBase {
	quint8 enabled;
});

PACK(struct OpcodeBGMOVIE : public OpcodeDisable {});

PACK(struct OpcodeKawai : public OpcodeBase {
	quint8 opcodeSize;
	quint8 subKey;
});

PACK(struct OpcodeKAWAI : public OpcodeKawai {
	QByteArray *_data;
});

PACK(struct OpcodeKAWAIEYETX {
	quint8 eyeID1;
	quint8 eyeID2;
	quint8 mouthID;
	quint8 objectID;
});

PACK(struct OpcodeKAWAITRNSP {
	quint8 enableTransparency;
});

PACK(struct OpcodeKAWAIAMBNT {
	quint8 r1;
	quint8 r2;
	quint8 g1;
	quint8 g2;
	quint8 b1;
	quint8 b2;
	quint8 flags;
});

PACK(struct OpcodeKAWAILIGHT {});
PACK(struct OpcodeKAWAISBOBJ {});
PACK(struct OpcodeKAWAISHINE {});
PACK(struct OpcodeKAWAIRESET {});

PACK(struct OpcodeKAWAIUNKNOWN4 {});
PACK(struct OpcodeKAWAIUNKNOWN7 {});
PACK(struct OpcodeKAWAIUNKNOWN8 {});
PACK(struct OpcodeKAWAIUNKNOWN9 {});
PACK(struct OpcodeKAWAIUNKNOWNB {});
PACK(struct OpcodeKAWAIUNKNOWNC {});
PACK(struct OpcodeKAWAIUNKNOWN {});

PACK(struct OpcodeKAWIW : public OpcodeBase {});

PACK(struct OpcodeParty : public OpcodeBase {
	quint8 partyID;
});

PACK(struct OpcodePMOVA : public OpcodeParty {});
PACK(struct OpcodeSLIP : public OpcodeDisable {});

// FIXME: change bank
PACK(struct OpcodeBGPDH : public OpcodeBase {
	quint8 banks;
	quint8 layerID;
	qint16 targetZ; // bank 1
});

PACK(struct OpcodeBGSCR : public OpcodeBase {
	quint8 banks;
	quint8 layerID;
	qint16 targetX; // bank 1
	qint16 targetY; // bank 2
});

PACK(struct OpcodeWindowGet : public OpcodeBase {
	quint8 windowID;
});

PACK(struct OpcodeWindowEdit : public OpcodeWindowGet {
	qint16 targetX;
	qint16 targetY;
	quint16 width;
	quint16 height;
});

PACK(struct OpcodeWCLS : public OpcodeWindowGet {});
PACK(struct OpcodeWSIZW : public OpcodeWindowEdit {});

PACK(struct OpcodeIfKey : public OpcodeBase {
	quint16 keys;
	quint8 jump;
	quint16 _label;
	quint8 _badJump;
});

PACK(struct OpcodeIFKEY : public OpcodeIfKey {});
PACK(struct OpcodeIFKEYON : public OpcodeIfKey {});
PACK(struct OpcodeIFKEYOFF : public OpcodeIfKey {});
PACK(struct OpcodeUC : public OpcodeDisable {});
// TODO: check if really partyID
PACK(struct OpcodePDIRA : public OpcodeParty {});

PACK(struct OpcodePTURA : public OpcodeParty {
	quint8 speed;
	quint8 directionRotation;
});

PACK(struct OpcodeWSPCL : public OpcodeWindowGet {
	quint8 displayType;
	quint8 marginLeft;
	quint8 marginTop;
});

PACK(struct OpcodeWNUMB : public OpcodeBase {
	quint8 banks;
	quint8 windowID;
	qint32 value; // bank 1 and 2
	quint8 digitCount;
});

// FIXME: change bank for "s"
PACK(struct OpcodeSTTIM : public OpcodeBase {
	quint8 banks[2];
	quint8 h; // bank 1
	quint8 m; // bank 2
	quint8 s; // bank 4
});

PACK(struct OpcodeGold : public OpcodeBase {
	quint8 banks;
	qint32 value; // bank 1 and 2
});

PACK(struct OpcodeGOLDu : public OpcodeGold {});
PACK(struct OpcodeGOLDd : public OpcodeGold {});

PACK(struct OpcodeCHGLD : public OpcodeBase {
	quint8 banks;
	quint8 var1; // bank 1 16-bit
	quint8 var2; // bank 2 16-bit
});

PACK(struct OpcodeHMPMAX1 : public OpcodeBase {});
PACK(struct OpcodeHMPMAX2 : public OpcodeBase {});
PACK(struct OpcodeMHMMX : public OpcodeBase {});
PACK(struct OpcodeHMPMAX3 : public OpcodeBase {});

PACK(struct OpcodeMESSAGE : public OpcodeWindowGet {
	quint8 textID;
});

PACK(struct OpcodeMPARA : public OpcodeBase {
	quint8 banks;
	quint8 windowID;
	quint8 windowVarID;
	quint8 value; // bank 2 8-bit
});

// FIXME: signed?
PACK(struct OpcodeMPRA2 : public OpcodeBase {
	quint8 banks;
	quint8 windowID;
	quint8 windowVarID;
	quint16 value; // bank 2 16-bit
});

PACK(struct OpcodeMPNAM : public OpcodeBase {
	quint8 textID;
});

PACK(struct OpcodeHpMp : public OpcodeBase {
	quint8 banks;
	quint8 partyID;
	quint16 value; // bank 2 16-bit
});

PACK(struct OpcodeUnused44 : public OpcodeBase {});
PACK(struct OpcodeMPu : public OpcodeHpMp {});
PACK(struct OpcodeUnused46 : public OpcodeBase {});
PACK(struct OpcodeMPd : public OpcodeHpMp {});

PACK(struct OpcodeASK : public OpcodeBase {
	quint8 banks;
	quint8 windowID;
	quint8 textID;
	quint8 firstLine;
	quint8 lastLine;
	quint8 varAnswer; // bank 2 8-bit
});

PACK(struct OpcodeMENU : public OpcodeBase {
	quint8 banks;
	quint8 menuID;
	quint8 param; // bank 2
});

PACK(struct OpcodeMENU2 : public OpcodeDisable {});

PACK(struct OpcodeBTLTB : public OpcodeBase {
	quint8 battleTableID;
});

PACK(struct OpcodeUnused4C : public OpcodeBase {});
PACK(struct OpcodeHPu : public OpcodeHpMp {});
PACK(struct OpcodeUnused4E : public OpcodeBase {});
PACK(struct OpcodeHPd : public OpcodeHpMp {});
PACK(struct OpcodeWINDOW : public OpcodeWindowEdit {});

PACK(struct OpcodeWMOVE : public OpcodeWindowGet {
	qint16 relativeX;
	qint16 relativeY;
});

PACK(struct OpcodeWMODE : public OpcodeWindowGet {
	quint8 mode;
	quint8 preventClose;
});

PACK(struct OpcodeWREST : public OpcodeWindowGet {});
PACK(struct OpcodeWCLSE : public OpcodeWindowGet {});

PACK(struct OpcodeWROW : public OpcodeWindowGet {
	quint8 rowCount;
});

PACK(struct OpcodeGWCOL : public OpcodeBase {
	quint8 banks[2];
	quint8 corner; // bank 1
	quint8 varR; // bank 2
	quint8 varG; // bank 3
	quint8 varB; // bank 4
});

PACK(struct OpcodeSWCOL : public OpcodeBase {
	quint8 banks[2];
	quint8 corner; // bank 1
	quint8 r; // bank 2
	quint8 g; // bank 3
	quint8 b; // bank 4
});

PACK(struct OpcodeItem : public OpcodeBase {
	quint8 banks;
	quint16 itemID; // bank 1
	quint8 quantity; // bank 2
});

PACK(struct OpcodeSTITM : public OpcodeItem {});
PACK(struct OpcodeDLITM : public OpcodeItem {});
PACK(struct OpcodeCKITM : public OpcodeItem {});

PACK(struct OpcodeMateria : public OpcodeBase {
	quint8 banks[2];
	quint8 materiaID; // bank 1
	quint8 APCount[3]; // bank 2, bank 3, bank 4
});

PACK(struct OpcodeSMTRA : public OpcodeMateria {});

PACK(struct OpcodeDMTRA : public OpcodeMateria {
	quint8 quantity;
});

// FIXME: all reorganized, APCount + 1 byte, - 1 unknown, bank of var is 6, not 5
PACK(struct OpcodeCMTRA : public OpcodeBase {
	quint8 banks[3];
	quint8 APCount[4]; // bank 1, bank 2, bank 3, bank 4
	quint8 materiaID;
	quint8 varQuantity; // bank 6 8-bit??
});

// FIXME: add var
PACK(struct OpcodeSHAKE : public OpcodeBase {
	quint8 banks[2];
	quint8 type;
	quint8 xAmplitude; // bank 1
	quint8 xFrames; // bank 2
	quint8 yAmplitude; // bank 3
	quint8 yFrames; // bank 4
});

PACK(struct OpcodeNOP : public OpcodeBase {});

PACK(struct OpcodeMAPJUMP : public OpcodeBase {
	quint16 mapID;
	qint16 targetX;
	qint16 targetY;
	quint16 targetI;
	quint8 direction;
});

PACK(struct OpcodeSCRLO : public OpcodeBase {
	quint8 unknown;
});

// FIXME: new variable detected
PACK(struct OpcodeSCRLC : public OpcodeBase {
	quint8 banks;
	quint16 speed; // bank 2
	quint8 unknown; // Related to Scroll Wait
});

PACK(struct OpcodeSCRLA : public OpcodeBase {
	quint8 banks;
	quint16 speed; // bank 2
	quint8 groupID;
	quint8 scrollType;
});

PACK(struct OpcodeSCR2D : public OpcodeBase {
	quint8 banks;
	qint16 targetX; // bank 1
	qint16 targetY; // bank 2
});

PACK(struct OpcodeSCRCC : public OpcodeBase {});

PACK(struct OpcodeScrollWithSpeed : public OpcodeBase {
	quint8 banks[2];
	qint16 targetX; // bank 1
	qint16 targetY; // bank 2
	quint16 speed; // bank 4
});
PACK(struct OpcodeSCR2DC : public OpcodeScrollWithSpeed {});
PACK(struct OpcodeSCRLW : public OpcodeBase {});
PACK(struct OpcodeSCR2DL : public OpcodeScrollWithSpeed {});

PACK(struct OpcodeMPDSP : public OpcodeBase {
	quint8 unknown;
});

// FIXME: last parameter has more sense
PACK(struct OpcodeVWOFT : public OpcodeBase {
	quint8 banks;
	quint16 unknown1; // bank 1
	quint16 unknown2; // bank 2
	quint8 enable;
});

PACK(struct OpcodeFADE : public OpcodeBase {
	quint8 banks[2];
	quint8 r; // bank 1
	quint8 g; // bank 2
	quint8 b; // bank 4
	quint8 speed;
	quint8 fadeType;
	quint8 adjust;
});

PACK(struct OpcodeFADEW : public OpcodeBase {});

PACK(struct OpcodeIDLCK : public OpcodeBase {
	quint16 triangleID;
	quint8 locked;
});

PACK(struct OpcodeLSTMP : public OpcodeBase {
	quint8 banks;
	quint8 var; // bank 2 16-bit
});

PACK(struct OpcodeSCRLP : public OpcodeBase {
	quint8 banks;
	quint16 speed; // bank 2
	quint8 partyID;
	quint8 scrollType;
});

PACK(struct OpcodeBATTLE : public OpcodeBase {
	quint8 banks;
	quint16 battleID; // bank 2
});

PACK(struct OpcodeBTLON : public OpcodeDisable {});

PACK(struct OpcodeBTLMD : public OpcodeBase {
	quint16 battleMode;
});

PACK(struct OpcodePGTDR : public OpcodeBase {
	quint8 banks;
	quint8 partyID;
	quint8 varDir; // bank 2
});

PACK(struct OpcodeGETPC : public OpcodeBase {
	quint8 banks;
	quint8 partyID;
	quint8 varPC; // bank 2
});

PACK(struct OpcodePXYZI : public OpcodeBase {
	quint8 banks[2];
	quint8 partyID;
	quint8 varX; // bank 1
	quint8 varY; // bank 2
	quint8 varZ; // bank 3
	quint8 varI; // bank 4
});

PACK(struct OpcodeBinaryOperation : public OpcodeBase {
	quint8 banks;
	quint8 var; // bank 1
	quint8 value; // bank 2
});

PACK(struct OpcodeBinaryOperation2 : public OpcodeBase {
	quint8 banks;
	quint8 var; // bank 1
	quint16 value; // bank 2
});

PACK(struct OpcodeUnaryOperation : public OpcodeBase {
	quint8 banks;
	quint8 var; // bank 2 8-bit or 16-bit (8-bit for RANDOM)
});

PACK(struct OpcodeBitOperation : public OpcodeBase {
	quint8 banks;
	quint8 var; // bank 1 1-bit
	quint8 position; // bank 2
});

PACK(struct OpcodePLUSX : public OpcodeBinaryOperation {});
PACK(struct OpcodePLUS2X : public OpcodeBinaryOperation2 {});
PACK(struct OpcodeMINUSX : public OpcodeBinaryOperation {});
PACK(struct OpcodeMINUS2X : public OpcodeBinaryOperation2 {});
PACK(struct OpcodeINCX : public OpcodeUnaryOperation {});
PACK(struct OpcodeINC2X : public OpcodeUnaryOperation {});
PACK(struct OpcodeDECX : public OpcodeUnaryOperation {});
PACK(struct OpcodeDEC2X : public OpcodeUnaryOperation {});
PACK(struct OpcodeTLKON : public OpcodeDisable {});

PACK(struct OpcodeRDMSD : public OpcodeBase {
	quint8 banks;
	quint8 value; // bank 2 8-bit
});

PACK(struct OpcodeSETBYTE : public OpcodeBinaryOperation {});
PACK(struct OpcodeSETWORD : public OpcodeBinaryOperation2 {});
PACK(struct OpcodeBITON : public OpcodeBitOperation {});
PACK(struct OpcodeBITOFF : public OpcodeBitOperation {});
PACK(struct OpcodeBITXOR : public OpcodeBitOperation {});
PACK(struct OpcodePLUS : public OpcodeBinaryOperation {});
PACK(struct OpcodePLUS2 : public OpcodeBinaryOperation2 {});
PACK(struct OpcodeMINUS : public OpcodeBinaryOperation {});
PACK(struct OpcodeMINUS2 : public OpcodeBinaryOperation2 {});
PACK(struct OpcodeMUL : public OpcodeBinaryOperation {});
PACK(struct OpcodeMUL2 : public OpcodeBinaryOperation2 {});
PACK(struct OpcodeDIV : public OpcodeBinaryOperation {});
PACK(struct OpcodeDIV2 : public OpcodeBinaryOperation2 {});
PACK(struct OpcodeMOD : public OpcodeBinaryOperation {});
PACK(struct OpcodeMOD2 : public OpcodeBinaryOperation2 {});
PACK(struct OpcodeAND : public OpcodeBinaryOperation {});
PACK(struct OpcodeAND2 : public OpcodeBinaryOperation2 {});
PACK(struct OpcodeOR : public OpcodeBinaryOperation {});
PACK(struct OpcodeOR2 : public OpcodeBinaryOperation2 {});
PACK(struct OpcodeXOR : public OpcodeBinaryOperation {});
PACK(struct OpcodeXOR2 : public OpcodeBinaryOperation2 {});
PACK(struct OpcodeINC : public OpcodeUnaryOperation {});
PACK(struct OpcodeINC2 : public OpcodeUnaryOperation {});
PACK(struct OpcodeDEC : public OpcodeUnaryOperation {});
PACK(struct OpcodeDEC2 : public OpcodeUnaryOperation {});
PACK(struct OpcodeRANDOM : public OpcodeUnaryOperation {});
PACK(struct OpcodeLBYTE : public OpcodeBinaryOperation {});
PACK(struct OpcodeHBYTE : public OpcodeBinaryOperation2 {});

PACK(struct OpcodeTOBYTE : public OpcodeBase {
	quint8 banks[2];
	quint8 var; // bank 1
	quint8 value1; // bank 2
	quint8 value2; // bank 4
});

// FIXME: set[value + varOrValue1] = varOrValue2
PACK(struct OpcodeSETX : public OpcodeBase {
	quint8 banks[2]; // bank 1
	quint8 value;
	quint16 varOrValue1; // bank 2
	quint8 varOrValue2; // bank 4
});

// FIXME: var = get[value + varOrValue1]
PACK(struct OpcodeGETX : public OpcodeBase {
	quint8 banks[2]; // bank 1
	quint8 value;
	quint16 varOrValue1; // bank 2
	quint8 var; // bank 4 8-bit
});

// FIXME: varResult is 16-bit
PACK(struct OpcodeSEARCHX : public OpcodeBase {
	quint8 banks[3]; // bank 1
	quint8 searchStart;
	quint16 start; // bank 2
	quint16 end; // bank 3
	quint8 value; // bank 4
	quint8 varResult; // bank 6 16-bit
});

PACK(struct OpcodeChar : public OpcodeBase {
	quint8 charID;
});

PACK(struct OpcodePC : public OpcodeChar {});

PACK(struct OpcodeCHAR_ : public OpcodeBase {
	quint8 object3DID;
});

PACK(struct OpcodeAnimation : public OpcodeBase {
	quint8 animID;
	quint8 speed;
});

PACK(struct OpcodeDFANM : public OpcodeAnimation {});
PACK(struct OpcodeANIME1 : public OpcodeAnimation {});

PACK(struct OpcodeVISI : public OpcodeBase {
	quint8 show;
});

PACK(struct OpcodeXYZ : public OpcodeBase {
	quint8 banks[2];
	qint16 targetX; // bank 1
	qint16 targetY; // bank 2
	qint16 targetZ; // bank 3 (OK)
});

PACK(struct OpcodeXYZI : public OpcodeBase {
	quint8 banks[2];
	qint16 targetX; // bank 1
	qint16 targetY; // bank 2
	qint16 targetZ; // bank 3
	quint16 targetI; // bank 4
});

PACK(struct OpcodeXYI : public OpcodeBase {
	quint8 banks[2];
	qint16 targetX; // bank 1
	qint16 targetY; // bank 2
	quint16 targetI; // bank 3
});

PACK(struct OpcodeMove : public OpcodeBase {
	quint8 banks;
	qint16 targetX; // bank 1
	qint16 targetY; // bank 2
});

PACK(struct OpcodeMOVE : public OpcodeMove {});
PACK(struct OpcodeCMOVE : public OpcodeMove {});

PACK(struct OpcodeToGroup : public OpcodeBase {
	quint8 groupID;
});

PACK(struct OpcodeMOVA : public OpcodeToGroup {});

PACK(struct OpcodeTURA : public OpcodeToGroup {
	quint8 directionRotation;
	quint8 speed;
});

PACK(struct OpcodeANIMW : public OpcodeBase {});
PACK(struct OpcodeFMOVE : public OpcodeMove {});
PACK(struct OpcodeANIME2: public OpcodeAnimation {});
PACK(struct OpcodeANIMX1 : public OpcodeAnimation {});

PACK(struct OpcodeCAnimation : public OpcodeBase {
	quint8 animID;
	quint8 firstFrame;
	quint8 lastFrame;
	quint8 speed;
});

PACK(struct OpcodeCANIM1: public OpcodeCAnimation {});
PACK(struct OpcodeCANMX1 : public OpcodeCAnimation {});

PACK(struct OpcodeSpeed : public OpcodeBase {
	quint8 banks;
	quint16 speed; // bank 2
});

PACK(struct OpcodeMSPED : public OpcodeSpeed {});

PACK(struct OpcodeDIR : public OpcodeBase {
	quint8 banks;
	quint8 direction; // bank 2
});

PACK(struct OpcodeTurn : public OpcodeBase {
	quint8 banks;
	quint8 direction; // bank 2
	quint8 turnCount;
	quint8 speed;
	quint8 unknown;
});

PACK(struct OpcodeTURNGEN : public OpcodeTurn {});
PACK(struct OpcodeTURN : public OpcodeTurn {});
PACK(struct OpcodeDIRA : public OpcodeToGroup {});

PACK(struct OpcodeGETDIR : public OpcodeBase {
	quint8 banks;
	quint8 groupID;
	quint8 varDir; // bank 2
});

PACK(struct OpcodeGETAXY : public OpcodeBase {
	quint8 banks;
	quint8 groupID;
	quint8 varX; // bank 1 16-bit
	quint8 varY; // bank 2 16-bit
});

PACK(struct OpcodeGETAI : public OpcodeBase {
	quint8 banks;
	quint8 groupID;
	quint8 varI; // bank 2 16-bit
});

PACK(struct OpcodeANIMX2 : public OpcodeAnimation {});
PACK(struct OpcodeCANIM2 : public OpcodeCAnimation {});
PACK(struct OpcodeCANMX2 : public OpcodeCAnimation {});
PACK(struct OpcodeASPED : public OpcodeSpeed {});
PACK(struct OpcodeUnusedBE : public OpcodeBase {});
PACK(struct OpcodeCC : public OpcodeToGroup {});

PACK(struct OpcodeJUMP : public OpcodeBase {
	quint8 banks[2];
	qint16 targetX; // bank 1
	qint16 targetY; // bank 2
	quint16 targetI; // bank 3
	qint16 height; // bank 4
});

PACK(struct OpcodeAXYZI : public OpcodeBase {
	quint8 banks[2];
	quint8 groupID;
	quint8 varX; // bank 1
	quint8 varY; // bank 2
	quint8 varZ; // bank 3
	quint8 varI; // bank 4
});

PACK(struct OpcodeLADER : public OpcodeBase {
	quint8 banks[2];
	qint16 targetX; // bank 1
	qint16 targetY; // bank 2
	qint16 targetZ; // bank 3
	quint16 targetI; // bank 4
	quint8 way;
	quint8 animID;
	quint8 direction;
	quint8 speed;
});

PACK(struct OpcodeOFST : public OpcodeBase {
	quint8 banks[2];
	quint8 moveType;
	qint16 targetX; // bank 1
	qint16 targetY; // bank 2
	qint16 targetZ; // bank 3
	quint16 speed; // bank 4
});

PACK(struct OpcodeOFSTW : public OpcodeBase {});

PACK(struct OpcodeRange : public OpcodeBase {
	quint8 banks;
	quint8 range; // bank 2
});

PACK(struct OpcodeRange2 : public OpcodeBase {
	quint8 banks;
	quint16 range; // bank 2
});

PACK(struct OpcodeTALKR : public OpcodeRange {});
PACK(struct OpcodeSLIDR : public OpcodeRange {});
PACK(struct OpcodeSOLID : public OpcodeDisable {});
PACK(struct OpcodePRTYP : public OpcodeChar {});
PACK(struct OpcodePRTYM : public OpcodeChar {});

PACK(struct OpcodePRTYE : public OpcodeBase {
	quint8 charID[3];
});

PACK(struct OpcodeIfQ : public OpcodeBase {
	quint8 charID;
	quint8 jump;
	quint16 _label;
	quint8 _badJump;
});

PACK(struct OpcodeIFPRTYQ : public OpcodeIfQ {});
PACK(struct OpcodeIFMEMBQ : public OpcodeIfQ {});

PACK(struct OpcodeMMBud : public OpcodeBase {
	quint8 exists;
	quint8 charID;
});

PACK(struct OpcodeMMBLK : public OpcodeChar {});
PACK(struct OpcodeMMBUK : public OpcodeChar {});

PACK(struct OpcodeLINE : public OpcodeBase {
	qint16 targetX1;
	qint16 targetY1;
	qint16 targetZ1;
	qint16 targetX2;
	qint16 targetY2;
	qint16 targetZ2;
});

PACK(struct OpcodeLINON : public OpcodeEnable {});
PACK(struct OpcodeMPJPO : public OpcodeDisable {});

PACK(struct OpcodeSLINE : public OpcodeBase {
	quint8 banks[3];
	qint16 targetX1; // bank 1
	qint16 targetY1; // bank 2
	qint16 targetZ1; // bank 3
	qint16 targetX2; // bank 4
	qint16 targetY2; // bank 5
	qint16 targetZ2; // bank 6
});

PACK(struct OpcodeSinCos : public OpcodeBase {
	quint8 banks[2];
	qint16 value1; // bank 1
	qint16 value2; // bank 2
	qint16 value3; // bank 3
	quint8 var; // bank 4
});

PACK(struct OpcodeSIN : public OpcodeSinCos {});
PACK(struct OpcodeCOS : public OpcodeSinCos {});
PACK(struct OpcodeTLKR2 : public OpcodeRange2 {});
PACK(struct OpcodeSLDR2 : public OpcodeRange2 {});

PACK(struct OpcodePMJMP : public OpcodeBase {
	quint16 mapID;
});

PACK(struct OpcodePMJMP2 : public OpcodeBase {});

PACK(struct OpcodeAKAO2 : public OpcodeBase {
	quint8 banks[3];
	quint8 opcode;
	quint16 param1; // bank 1
	quint16 param2; // bank 2
	quint16 param3; // bank 3
	quint16 param4; // bank 4
	quint16 param5; // bank 6
});

PACK(struct OpcodeFCFIX : public OpcodeDisable {});

PACK(struct OpcodeCCANM : public OpcodeAnimation {
	quint8 standWalkRun;
});

PACK(struct OpcodeANIMB : public OpcodeBase {});
PACK(struct OpcodeTURNW : public OpcodeBase {});

PACK(struct OpcodeMPPAL : public OpcodeBase {
	quint8 banks[3];
	quint8 posSrc;
	quint8 posDst;
	quint8 start; // bank 1
	quint8 b; // bank 2
	quint8 g; // bank 3
	quint8 r; // bank 4
	quint8 colorCount; // bank 6
});

PACK(struct OpcodeBackgroundState : public OpcodeBase {
	quint8 banks;
	quint8 bgParamID; // bank 1
	quint8 bgStateID; // bank 2
});

PACK(struct OpcodeBGON : public OpcodeBackgroundState {});
PACK(struct OpcodeBGOFF : public OpcodeBackgroundState {});

PACK(struct OpcodeBackgroundRoll : public OpcodeBase {
	quint8 banks;
	quint8 bgParamID; // bank 2
});

PACK(struct OpcodeBGROL : public OpcodeBackgroundRoll {});
PACK(struct OpcodeBGROL2 : public OpcodeBackgroundRoll {});
PACK(struct OpcodeBGCLR : public OpcodeBackgroundRoll {});

PACK(struct OpcodeSTPAL : public OpcodeBase {
	quint8 banks;
	quint8 palID; // bank 1
	quint8 position; // bank 2
	quint8 colorCount;
});

PACK(struct OpcodeLDPAL : public OpcodeBase {
	quint8 banks;
	quint8 position; // bank 1
	quint8 palID; // bank 2
	quint8 colorCount;
});

PACK(struct OpcodeCPPAL : public OpcodeBase {
	quint8 banks;
	quint8 posSrc; // bank 1
	quint8 posDst; // bank 2
	quint8 colorCount;
});

PACK(struct OpcodeRTPAL : public OpcodeBase {
	quint8 banks[2];
	quint8 posSrc; // bank 1
	quint8 posDst; // bank 2
	quint8 start; // bank 4
	quint8 end;
});

PACK(struct OpcodeADPAL : public OpcodeBase {
	quint8 banks[3];
	quint8 posSrc; // bank 1
	quint8 posDst; // bank 2
	quint8 b; // bank 3
	quint8 g; // bank 4
	quint8 r; // bank 5
	quint8 colorCount;
});

PACK(struct OpcodeMPPAL2 : public OpcodeBase {
	quint8 banks[3];
	quint8 posSrc; // bank 1
	quint8 posDst; // bank 2
	quint8 b; // bank 3
	quint8 g; // bank 4
	quint8 r; // bank 5
	quint8 colorCount;
});

PACK(struct OpcodeSTPLS : public OpcodeBase {
	quint8 palID;
	quint8 posSrc;
	quint8 start;
	quint8 colorCount;
});

PACK(struct OpcodeLDPLS : public OpcodeBase {
	quint8 posSrc;
	quint8 palID;
	quint8 start;
	quint8 colorCount;
});

// FIXME
PACK(struct OpcodeCPPAL2 : public OpcodeBase {
	quint8 banks[2];
	quint8 posTileSrc;
	quint8 posTileDst;
	quint8 posSrc; // bank 1
	quint8 posDst; // bank 2
	quint8 colorCount; // bank 4
});

// FIXME
PACK(struct OpcodeRTPAL2 : public OpcodeBase {
	quint8 banks[2];
	quint8 posTileSrc;
	quint8 posTileDst;
	quint8 posSrc; // bank 1
	quint8 posDst; // bank 2
	quint8 start; // bank 4
});

PACK(struct OpcodeADPAL2 : public OpcodeBase {
	quint8 banks[3];
	quint8 posTileSrc;
	quint8 posTileDst;
	quint8 start; // bank 1
	quint8 b; // bank 2
	quint8 g; // bank 3
	quint8 r; // bank 4
	quint8 colorCount; // bank 6
});

PACK(struct OpcodeMusic : public OpcodeBase {
	quint8 musicID;
});

PACK(struct OpcodeMUSIC : public OpcodeMusic {});

PACK(struct OpcodeSOUND : public OpcodeBase {
	quint8 banks;
	quint16 soundID; // bank 1
	quint8 position; // bank 2
});

PACK(struct OpcodeAKAO : public OpcodeBase {
	quint8 banks[3];
	quint8 opcode;
	quint8 param1; // bank 1
	quint16 param2; // bank 2
	quint16 param3; // bank 3
	quint16 param4; // bank 4
	quint16 param5; // bank 6
});

PACK(struct OpcodeMUSVT : public OpcodeMusic {});
PACK(struct OpcodeMUSVM : public OpcodeMusic {});
PACK(struct OpcodeMULCK : public OpcodeDisable {});
PACK(struct OpcodeBMUSC : public OpcodeMusic {});

// FIXME: reverse this
PACK(struct OpcodeCHMPH : public OpcodeBase {
	quint8 banks;
	quint8 var1; // bank 1 8-bit??
	quint8 var2; // bank 2 8-bit??
});

PACK(struct OpcodeMovie : public OpcodeBase {
	quint8 movieID;
});

PACK(struct OpcodePMVIE : public OpcodeMovie {});
PACK(struct OpcodeMOVIE : public OpcodeBase {});

PACK(struct OpcodeMVIEF : public OpcodeBase {
	quint8 banks;
	quint8 varCurMovieFrame; // bank 2
});

PACK(struct OpcodeMVCAM : public OpcodeBase {
	quint8 movieCamID;
});

PACK(struct OpcodeFMUSC : public OpcodeMusic {});

PACK(struct OpcodeCMUSC : public OpcodeMusic {
	quint8 banks;
	quint8 opcode;
	quint16 param1; // bank 1
	quint16 param2; // bank 2
});

PACK(struct OpcodeCHMST : public OpcodeBase {
	quint8 banks;
	quint8 var; // bank 2 8-bit
});

PACK(struct OpcodeGAMEOVER : public OpcodeBase {});

PACK(struct OpcodeLABEL : public OpcodeBase {
	quint16 _label;
});

class Opcode
{
public:
	union OPCODE {
		OpcodeKey id;

#define op_fun(name) \
	Opcode##name opcode##name;
#define op_sep
	OPCODE_GENERATE_LIST
#undef op_fun
#undef op_sep

#define op_fun(name) \
	OpcodeSPECIAL##name opcodeSPECIAL##name;
#define op_sep
	OPCODE_GENERATE_SPECIAL_LIST
#undef op_fun
#undef op_sep
	};
	
	enum Operation : quint8 {
		None, Assign, BitAssign, Compare, BitCompare,
		AssignNotEqual,
		AssignLessThan, AssignLessThanEqual,
		AssignGreaterThan, AssignGreaterThanEqual,
		CompareNotEqual,
		CompareLessThan, CompareLessThanEqual,
		CompareGreaterThan, CompareGreaterThanEqual
	};

	enum Operator : quint8 {
		Equal, NotEqual,
		UpperThan, LowerThan,
		UpperThanEqual, LowerThanEqual,
		BitAnd, BitXOr, BitOr,
		BitOn, BitOff
	};

	Opcode() noexcept;
	Opcode(const char *data, qsizetype size) noexcept;
	Opcode(OpcodeKey id, const char *params, qsizetype size) noexcept;
#define op_fun(name) \
	Opcode(Opcode##name o) noexcept { \
		o.id = OpcodeKey::name; \
		_opcode.opcode##name = o; \
	}
#define op_sep
	OPCODE_GENERATE_LIST
#undef op_fun
#undef op_sep
#define op_fun(name) \
	Opcode(OpcodeSPECIAL##name o) noexcept { \
		o.id = OpcodeKey::SPECIAL; \
		o.subKey = OpcodeSpecialKey::name; \
		_opcode.opcodeSPECIAL##name = o; \
	}
#define op_sep
	OPCODE_GENERATE_SPECIAL_LIST
#undef op_fun
#undef op_sep
	Opcode(const Opcode &other) noexcept;
	~Opcode() noexcept;

	Opcode &operator=(const Opcode &other) noexcept;

	inline OpcodeKey id() const {
		return _opcode.id;
	}
	inline OPCODE op() const {
		return _opcode;
	}
	inline OPCODE &op() {
		return _opcode;
	}
	QString toString(const Section1File *scriptsAndTexts) const;
	void setParams(const char *params, qsizetype maxSize);
	QByteArray params() const;
	int subParam(qsizetype cur, qsizetype sizeInBits) const;
	quint8 size() const;
	inline const char *name() const {
		return id() < 257 ? names[id()] : names[0];
	}
	static Opcode fromData(const char *data, qsizetype size);
	QByteArray toByteArray() const;
	// Unserializable byte array (include OpcodeLABEL case)
	QByteArray serialize() const;
	static Opcode unserialize(const QByteArray &data);
	bool isExec() const;
	bool isJump() const;
	bool isLongJump() const;
	bool isBackJump() const;
	bool isIf() const;
	bool isVoid() const;

	bool searchVar(quint8 bank, quint16 address, Operation operation = None, int value = 65536) const;

	qint16 groupID() const;
	void setGroupID(quint8 groupID);
	qint8 scriptID() const;
	qint8 priority() const;
	void setScriptIDAndPriority(quint8 scriptID, quint8 priority);
	qint16 partyID() const;
	void setPartyID(quint8 partyID);
	qint16 charID() const;
	void setCharID(quint8 charID);
	qint16 textID() const;
	void setTextID(quint8 textID);
	qint16 tutoID() const;
	void setTutoID(quint8 tutoID);
	int mapID() const;
	void setMapID(quint16 mapID);
	qint16 subKey() const;
	qint32 keys() const;
	void setKeys(quint16 keys);
	bool ifStruct(FF7If &i) const;
	void setIfStruct(const FF7If &i);
	bool binaryOperation(FF7BinaryOperation &o) const;
	void setBinaryOperation(const FF7BinaryOperation &o);
	bool unaryOperation(FF7UnaryOperation &o) const;
	void setUnaryOperation(const FF7UnaryOperation &o);
	bool bitOperation(FF7BitOperation &o) const;
	void setBitOperation(const FF7BitOperation &o);
	qint8 bank1() const;
	qint8 bank2() const;
	qint8 bank3() const;
	qint8 bank4() const;
	qint8 bank5() const;
	qint8 bank6() const;
	qint16 windowID() const;
	void setWindowID(quint8 windowID);
	bool window(FF7Window &window) const;
	void setWindow(const FF7Window &window);
	QByteArray resizableData() const;
	void setResizableData(const QByteArray &data);
	bool variables(QList<FF7Var> &vars) const;
	bool modelPosition(FF7Position &position) const;
	bool linePosition(FF7Position position[2]) const;

	// Jump
	qint16 shortJump() const;
	qint32 longJump() const;
	qint32 jump() const;
	// Return true if opcode was converted to long
	bool setShortJump(quint8 jump);
	// Return true if opcode was converted to short
	bool setLongJump(quint16 jump);
	// Return true if opcode was converted to short or long
	bool setJump(qint32 jump);
	int label() const;
	void setLabel(quint16 label);
	BadJumpError badJump() const;
	void setBadJump(BadJumpError badJump);
	bool itemIsExpanded() const;
	void setItemIsExpanded(bool isExpanded);

	static QString character(quint8 persoID);
	static QString akao(quint8 akaoOp, bool *ok = nullptr);

	static const char *operators[OPERATORS_SIZE];
	static const quint8 length[257];
	static const char *names[257];
private:
	quint8 fixedSize() const;
	void clearResizableDataPointers();
	void deleteResizableData();
	quint8 jumpShift() const;
	inline static QString tr(const char *sourceText, const char *disambiguation = nullptr, int n = -1) {
		return QCoreApplication::translate("Opcode", sourceText, disambiguation, n);
	}

#define op_fun(name) \
	QString toString##name(const Section1File *scriptsAndTexts, const Opcode##name &opcode) const;
#define op_sep
	OPCODE_GENERATE_LIST
#undef op_fun
#undef op_sep
	
#define op_fun(name) \
	QString toStringSPECIAL##name(const Section1File *scriptsAndTexts, const OpcodeSPECIAL##name &opcode) const;
#define op_sep
	OPCODE_GENERATE_SPECIAL_LIST
#undef op_fun
#undef op_sep
	
#define op_fun(name) \
	QString toStringKAWAI##name(const Section1File *scriptsAndTexts, const OpcodeKAWAI##name &opcode) const;
#define op_sep
	OPCODE_GENERATE_KAWAI_LIST
#undef op_fun
#undef op_sep

	static QString _groupScript(quint8 param, const Section1File *scriptsAndTexts);
	static QString _text(quint8 textID, const Section1File *scriptsAndTexts);
	static QString _item(quint16 itemID, quint8 bank);
	static QString _materia(quint8 materiaID, quint8 bank);
	static QString _field(quint16 fieldID);
	static QString _movie(quint8 movieID);
	// static QString _objet3D(quint8 objet3D_ID);

	static QString _bank(quint8 address, quint8 bank);
	static QString _var(int value, quint8 bank);
	static QString _var(int value, quint8 bank1, quint8 bank2);
	static QString _var(const quint8 value[3], quint8 bank1, quint8 bank2, quint8 bank3);
	static QString _var(const quint8 value[4], quint8 bank1, quint8 bank2, quint8 bank3, quint8 bank4);

	static QString _windowCorner(quint8 param, quint8 bank);
	static QString _sensRotation(quint8 param);
	static QString _keys(quint16 keys);
	static QString _operateur(quint8 param);

	OPCODE _opcode;
};
