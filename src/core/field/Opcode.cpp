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
#include "Opcode.h"
#include "Section1File.h"
#include "core/Var.h"
#include "Data.h"
#include "Field.h"
#include "FieldArchiveIO.h"
#include "FieldArchive.h"

Opcode::Opcode() noexcept
{
}

Opcode::Opcode(const char *data, qsizetype size) noexcept
{
	if (size <= 0) {
		return;
	}

	_opcode.id = OpcodeKey(quint8(data[0]));
	setParams(data + 1, size - 1);
}

Opcode::Opcode(OpcodeKey id, const char *params, qsizetype size) noexcept
{
	_opcode.id = id;
	setParams(params, size);
}

Opcode::Opcode(const Opcode &other) noexcept :
    _opcode(other._opcode)
{
	clearResizableDataPointers();
	setResizableData(other.resizableData());
}

Opcode::~Opcode() noexcept
{
	deleteResizableData();
}

Opcode &Opcode::operator=(const Opcode &other) noexcept
{
	_opcode = other._opcode;
	clearResizableDataPointers();
	setResizableData(other.resizableData());

	return *this;
}

void Opcode::setParams(const char *params, qsizetype maxSize)
{
	quint8 fixedParamsSize = fixedSize() - 1;

	if (fixedParamsSize > maxSize) {
		qWarning() << "Opcode::setParams" << id() << "fixed size exceeded" << fixedParamsSize << maxSize;
		fixedParamsSize = quint8(maxSize);
	}

	qsizetype maxStructSize = sizeof(_opcode) - sizeof(_opcode.id);
	memcpy(reinterpret_cast<char *>(&_opcode) + sizeof(_opcode.id), params, size_t(fixedParamsSize));
	if (fixedParamsSize < maxStructSize) {
		memset(reinterpret_cast<char *>(&_opcode) + sizeof(_opcode.id) + fixedParamsSize, 0, size_t(maxStructSize - fixedParamsSize));
	}

	if (id() == OpcodeKey::Unused1C || id() == OpcodeKey::KAWAI) {
		quint8 dynamicParamsSize = size() - fixedParamsSize - 1;
		
		if (dynamicParamsSize > maxSize - fixedParamsSize) {
			qWarning() << "Opcode::setParams" << id() << "dynamic size exceeded" << fixedParamsSize << dynamicParamsSize << maxSize;
			dynamicParamsSize = quint8(maxSize - fixedParamsSize);
		}

		setResizableData(QByteArray(params + fixedParamsSize, dynamicParamsSize));
	}
}

QByteArray Opcode::params() const
{
	quint8 fixedParamsSize = fixedSize() - 1;
	QByteArray ret;

	return ret
	        .append(reinterpret_cast<const char *>(&_opcode) + sizeof(_opcode.id), fixedParamsSize)
	        .append(resizableData());
}

int Opcode::subParam(qsizetype cur, qsizetype sizeInBits) const
{
	QByteArray p = params();
	qsizetype size = sizeInBits % 8 != 0 ? sizeInBits / 8 + 1 : sizeInBits / 8;
	int value = 0;

	memcpy(&value, p.constData() + cur / 8, size_t(size));

	return (value >> ((size * 8 - cur % 8) - sizeInBits)) & (int(pow(2, sizeInBits)) - 1);
}

quint8 Opcode::fixedSize() const
{
	quint8 size = id() < 257 ? length[id()] : 1;

	if (id() == OpcodeKey::SPECIAL) {
		switch (_opcode.opcodeSPECIAL.subKey) {
		case OpcodeSpecialKey::ARROW:
		case OpcodeSpecialKey::PNAME:
		case OpcodeSpecialKey::GMSPD:
		case OpcodeSpecialKey::BTLCK:
		case OpcodeSpecialKey::MVLCK:
			size += 1;
			break;
		case OpcodeSpecialKey::SMSPD:
		case OpcodeSpecialKey::SPCNM:
			size += 2;
			break;
		}
	}

	return size;
}

quint8 Opcode::size() const
{
	quint8 size = fixedSize();

	if (id() == OpcodeKey::Unused1C) {
		size += std::min(_opcode.opcodeUnused1C.subSize, quint8(128));
	} else if (id() == OpcodeKey::KAWAI) {
		size = _opcode.opcodeKAWAI.opcodeSize;
	}

	return size;
}

QByteArray Opcode::toByteArray() const
{
	quint8 s = size();
	QByteArray ret;

	if (s > 0) {
		ret.reserve(s);
		ret.append(char(id()));
		ret.append(params());
	}

	return ret;
}

QByteArray Opcode::serialize() const
{
	QByteArray data = resizableData();
	QByteArray ret;
	ret.reserve(qsizetype(sizeof(_opcode)) + data.size());
	return ret
	        .append(reinterpret_cast<const char *>(&_opcode), sizeof(_opcode))
	        .append(resizableData());
}

Opcode Opcode::unserialize(const QByteArray &data)
{
	Opcode ret;
	ret._opcode = OPCODE();
	memcpy(&ret._opcode, data.constData(), std::min(sizeof(_opcode), size_t(data.size())));
	ret.setResizableData(data.mid(sizeof(_opcode)));

	return ret;
}

bool Opcode::isExec() const
{
	return id() >= OpcodeKey::REQ && id() <= OpcodeKey::PRQEW;
}

bool Opcode::isJump() const
{
	return (id() >= OpcodeKey::JMPF && id() <= OpcodeKey::JMPBL)
	        || id() == OpcodeKey::Unused1B
	        || isIf();
}

bool Opcode::isLongJump() const
{
	return id() == OpcodeKey::JMPFL
	        || id() == OpcodeKey::JMPBL
	        || id() == OpcodeKey::IFUBL
	        || id() == OpcodeKey::IFSWL
	        || id() == OpcodeKey::IFUWL
	        || id() == OpcodeKey::Unused1B;
}

bool Opcode::isBackJump() const
{
	return id() == OpcodeKey::JMPB || id() == OpcodeKey::JMPBL;
}

bool Opcode::isIf() const
{
	return (id() >= OpcodeKey::IFUB && id() <= OpcodeKey::IFUWL)
	       || (id() >= OpcodeKey::IFKEY && id() <= OpcodeKey::IFKEYOFF)
	        || (id() >= OpcodeKey::IFPRTYQ && id() <= OpcodeKey::IFMEMBQ);
}

bool Opcode::isVoid() const
{
	return id() == OpcodeKey::RET || id() == OpcodeKey::LABEL || isJump();
}

bool Opcode::searchVar(quint8 bank, quint16 address, Operation operation, int value) const
{
	// TODO: compare var with var
	const bool noValue = value > 0xFFFF,
	        noAddress = address > 0xFF;
	QList<FF7Var> vars;

	variables(vars);

	switch (operation) {
	case Assign:
	case AssignNotEqual:
	case AssignLessThan:
	case AssignLessThanEqual:
	case AssignGreaterThan:
	case AssignGreaterThanEqual:
		// Compare value if provided
		if (!noValue) {
			if (id() == SETBYTE || id() == SETWORD) {
				FF7BinaryOperation binaryOp;
				binaryOperation(binaryOp);
				
				if (bank1() == bank && (noAddress || binaryOp.var == address)
				        && bank2() == 0) {
					switch (operation) {
					case Assign:
						return binaryOp.value == value;
					case AssignNotEqual:
						return binaryOp.value != value;
					case AssignLessThan:
						return binaryOp.value < value;
					case AssignLessThanEqual:
						return binaryOp.value <= value;
					case AssignGreaterThan:
						return binaryOp.value > value;
					case AssignGreaterThanEqual:
						return binaryOp.value >= value;
					default:
						return false;
					}
				}
			}
		} else {
			// Every write vars
			for (const FF7Var &var : std::as_const(vars)) {
				if (var.bank == bank && (noAddress || var.address == address)
				        && var.flags.testFlag(FF7Var::Writable) && var.size != FF7Var::Bit) {
					return true;
				}
			}
		}
		return false;
	case BitAssign:
		if (id() == BITON
		        || id() == BITOFF
		        || id() == BITXOR) {
			const OpcodeBitOperation &bitOperation = _opcode.opcodeBITON;
			if (bank1() == bank && (noAddress || bitOperation.var == address)
			        && (noValue || (bank2() == 0 && bitOperation.position == value))) {
				return true;
			}
		}
		return false;
	case Compare:
	case BitCompare: {
		FF7If i;
		if (ifStruct(i) && i.oper >= Equal && i.bank1 == bank && (noAddress || i.value1 == address)) {
			if (noValue) {
				return true;
			} else if ((i.oper <= LowerThanEqual || i.oper >= BitOn)
			           && (i.bank2 == 0 && i.value2 == value)) {
				return true;
			} else if (i.oper >= BitAnd && i.oper <= BitOr
			           && (i.bank2 == 0 && (i.value2 >> value) & 1)) {
				return true;
			}
		}
	} return false;
	default:
		for (const FF7Var &var : std::as_const(vars)) {
			if (var.bank == bank && (noAddress || var.address == address)) {
				return true;
			}
		}
		return false;
	}
}

#define CaseOpcodeAttribute(op, attr) \
case OpcodeKey::op: \
    return _opcode.opcode##op.attr

#define CaseOpcodeAttrConvert(op, attr, conv) \
case OpcodeKey::op: \
    return conv(_opcode.opcode##op.attr)

#define CaseOpcodeSetAttribute(op, attr) \
case OpcodeKey::op: \
    _opcode.opcode##op.attr = attr; \
    break

qint16 Opcode::groupID() const
{
	switch (id()) {
		CaseOpcodeAttribute(REQ, groupID);
		CaseOpcodeAttribute(REQSW, groupID);
		CaseOpcodeAttribute(REQEW, groupID);
		CaseOpcodeAttribute(SCRLA, groupID);
		CaseOpcodeAttribute(TURA, groupID);
		CaseOpcodeAttribute(MOVA, groupID);
		CaseOpcodeAttribute(DIRA, groupID);
		CaseOpcodeAttribute(GETDIR, groupID);
		CaseOpcodeAttribute(GETAXY, groupID);
		CaseOpcodeAttribute(GETAI, groupID);
		CaseOpcodeAttribute(AXYZI, groupID);
		CaseOpcodeAttribute(CC, groupID);
	default:
		break;
	}

	return -1;
}

void Opcode::setGroupID(quint8 groupID)
{
	switch (id()) {
		CaseOpcodeSetAttribute(REQ, groupID);
		CaseOpcodeSetAttribute(REQSW, groupID);
		CaseOpcodeSetAttribute(REQEW, groupID);
		CaseOpcodeSetAttribute(SCRLA, groupID);
		CaseOpcodeSetAttribute(TURA, groupID);
		CaseOpcodeSetAttribute(MOVA, groupID);
		CaseOpcodeSetAttribute(DIRA, groupID);
		CaseOpcodeSetAttribute(GETDIR, groupID);
		CaseOpcodeSetAttribute(GETAXY, groupID);
		CaseOpcodeSetAttribute(GETAI, groupID);
		CaseOpcodeSetAttribute(AXYZI, groupID);
		CaseOpcodeSetAttribute(CC, groupID);
	default:
		break;
	}
}

qint8 Opcode::scriptID() const
{
	switch (id()) {
		CaseOpcodeAttribute(REQ, scriptIDAndPriority & 0x1F);
		CaseOpcodeAttribute(REQSW, scriptIDAndPriority & 0x1F);
		CaseOpcodeAttribute(REQEW, scriptIDAndPriority & 0x1F);
		CaseOpcodeAttribute(PREQ, scriptIDAndPriority & 0x1F);
		CaseOpcodeAttribute(PRQSW, scriptIDAndPriority & 0x1F);
		CaseOpcodeAttribute(PRQEW, scriptIDAndPriority & 0x1F);
		CaseOpcodeAttribute(RETTO, scriptIDAndPriority & 0x1F);
	default:
		break;
	}

	return -1;
}

qint8 Opcode::priority() const
{
	switch (id()) {
		CaseOpcodeAttribute(REQ, scriptIDAndPriority >> 5);
		CaseOpcodeAttribute(REQSW, scriptIDAndPriority >> 5);
		CaseOpcodeAttribute(REQEW, scriptIDAndPriority >> 5);
		CaseOpcodeAttribute(PREQ, scriptIDAndPriority >> 5);
		CaseOpcodeAttribute(PRQSW, scriptIDAndPriority >> 5);
		CaseOpcodeAttribute(PRQEW, scriptIDAndPriority >> 5);
		CaseOpcodeAttribute(RETTO, scriptIDAndPriority >> 5);
	default:
		break;
	}

	return -1;
}

void Opcode::setScriptIDAndPriority(quint8 scriptID, quint8 priority)
{
	quint8 scriptIDAndPriority = SCRIPT_AND_PRIORITY(scriptID, priority);

	switch (id()) {
		CaseOpcodeSetAttribute(REQ, scriptIDAndPriority);
		CaseOpcodeSetAttribute(REQSW, scriptIDAndPriority);
		CaseOpcodeSetAttribute(REQEW, scriptIDAndPriority);
		CaseOpcodeSetAttribute(PREQ, scriptIDAndPriority);
		CaseOpcodeSetAttribute(PRQSW, scriptIDAndPriority);
		CaseOpcodeSetAttribute(PRQEW, scriptIDAndPriority);
		CaseOpcodeSetAttribute(RETTO, scriptIDAndPriority);
	default:
		break;
	}
}

qint16 Opcode::partyID() const
{
	switch (id()) {
		CaseOpcodeAttribute(PREQ, partyID);
		CaseOpcodeAttribute(PRQSW, partyID);
		CaseOpcodeAttribute(PRQEW, partyID);
		CaseOpcodeAttribute(PMOVA, partyID);
		CaseOpcodeAttribute(PDIRA, partyID);
		CaseOpcodeAttribute(PTURA, partyID);
		CaseOpcodeAttribute(MPu, partyID);
		CaseOpcodeAttribute(MPd, partyID);
		CaseOpcodeAttribute(HPu, partyID);
		CaseOpcodeAttribute(HPd, partyID);
		CaseOpcodeAttribute(SCRLP, partyID);
		CaseOpcodeAttribute(PGTDR, partyID);
		CaseOpcodeAttribute(GETPC, partyID);
		CaseOpcodeAttribute(PXYZI, partyID);
	default:
		break;
	}

	return -1;
}

void Opcode::setPartyID(quint8 partyID)
{
	switch (id()) {
		CaseOpcodeSetAttribute(PREQ, partyID);
		CaseOpcodeSetAttribute(PRQSW, partyID);
		CaseOpcodeSetAttribute(PRQEW, partyID);
		CaseOpcodeSetAttribute(PMOVA, partyID);
		CaseOpcodeSetAttribute(PDIRA, partyID);
		CaseOpcodeSetAttribute(PTURA, partyID);
		CaseOpcodeSetAttribute(MPu, partyID);
		CaseOpcodeSetAttribute(MPd, partyID);
		CaseOpcodeSetAttribute(HPu, partyID);
		CaseOpcodeSetAttribute(HPd, partyID);
		CaseOpcodeSetAttribute(SCRLP, partyID);
		CaseOpcodeSetAttribute(PGTDR, partyID);
		CaseOpcodeSetAttribute(GETPC, partyID);
		CaseOpcodeSetAttribute(PXYZI, partyID);
	default:
		break;
	}
}

qint16 Opcode::charID() const
{
	switch (id()) {
	case OpcodeKey::SPECIAL:
		if (_opcode.opcodeSPECIALSPCNM.subKey == OpcodeSpecialKey::SPCNM) {
			return _opcode.opcodeSPECIALSPCNM.charID;
		}
		break;
		CaseOpcodeAttribute(PC, charID);
		CaseOpcodeAttribute(PRTYP, charID);
		CaseOpcodeAttribute(PRTYM, charID);
		CaseOpcodeAttribute(MMBLK, charID);
		CaseOpcodeAttribute(MMBUK, charID);
		CaseOpcodeAttribute(IFPRTYQ, charID);
		CaseOpcodeAttribute(IFMEMBQ, charID);
		CaseOpcodeAttribute(MMBud, charID);
	default:
		break;
	}

	return -1;
}

void Opcode::setCharID(quint8 charID)
{
	switch (id()) {
	case OpcodeKey::SPECIAL:
		if (_opcode.opcodeSPECIALSPCNM.subKey == OpcodeSpecialKey::SPCNM) {
			_opcode.opcodeSPECIALSPCNM.charID = charID;
		}
		break;
		CaseOpcodeSetAttribute(PC, charID);
		CaseOpcodeSetAttribute(PRTYP, charID);
		CaseOpcodeSetAttribute(PRTYM, charID);
		CaseOpcodeSetAttribute(MMBLK, charID);
		CaseOpcodeSetAttribute(MMBUK, charID);
		CaseOpcodeSetAttribute(IFPRTYQ, charID);
		CaseOpcodeSetAttribute(IFMEMBQ, charID);
		CaseOpcodeSetAttribute(MMBud, charID);
	default:
		break;
	}
}

qint16 Opcode::textID() const
{
	switch (id()) {
	case OpcodeKey::SPECIAL:
		if (_opcode.opcodeSPECIALSPCNM.subKey == OpcodeSpecialKey::SPCNM) {
			return _opcode.opcodeSPECIALSPCNM.textID;
		}
		break;
		CaseOpcodeAttribute(MESSAGE, textID);
		CaseOpcodeAttribute(MPNAM, textID);
		CaseOpcodeAttribute(ASK, textID);
	default:
		break;
	}

	return -1;
}

void Opcode::setTextID(quint8 textID)
{
	switch (id()) {
	case OpcodeKey::SPECIAL:
		if (_opcode.opcodeSPECIALSPCNM.subKey == OpcodeSpecialKey::SPCNM) {
			_opcode.opcodeSPECIALSPCNM.textID = textID;
		}
		break;
		CaseOpcodeSetAttribute(MESSAGE, textID);
		CaseOpcodeSetAttribute(MPNAM, textID);
		CaseOpcodeSetAttribute(ASK, textID);
	default:
		break;
	}
}

qint16 Opcode::tutoID() const
{
	switch (id()) {
		CaseOpcodeAttribute(TUTOR, tutoID);
	default:
		break;
	}

	return -1;
}

void Opcode::setTutoID(quint8 tutoID)
{
	switch (id()) {
		CaseOpcodeSetAttribute(TUTOR, tutoID);
	default:
		break;
	}
}

qint16 Opcode::paletteID() const
{
	switch (id()) {
	case OpcodeKey::STPAL:
		if (B1(_opcode.opcodeSTPAL.banks) == 0) {
			return _opcode.opcodeSTPAL.palID;
		}
		break;
	case OpcodeKey::LDPAL:
		if (B2(_opcode.opcodeLDPAL.banks) == 0) {
			return _opcode.opcodeLDPAL.palID;
		}
		break;
		CaseOpcodeAttribute(STPLS, palID);
		CaseOpcodeAttribute(LDPLS, palID);
	default:
		break;
	}

	return -1;
}

void Opcode::setPaletteID(quint8 palID)
{
	switch (id()) {
	case OpcodeKey::STPAL:
		if (B1(_opcode.opcodeSTPAL.banks) == 0) {
			_opcode.opcodeSTPAL.palID = palID;
		}
		break;
	case OpcodeKey::LDPAL:
		if (B2(_opcode.opcodeLDPAL.banks) == 0) {
			_opcode.opcodeLDPAL.palID = palID;
		}
		break;
		CaseOpcodeSetAttribute(STPLS, palID);
		CaseOpcodeSetAttribute(LDPLS, palID);
	default:
		break;
	}
}

int Opcode::mapID() const
{
	switch (id()) {
		CaseOpcodeAttribute(MINIGAME, mapID);
		CaseOpcodeAttribute(MAPJUMP, mapID);
		CaseOpcodeAttribute(PMJMP, mapID);
	default:
		break;
	}

	return -1;
}

void Opcode::setMapID(quint16 mapID)
{
	switch (id()) {
		CaseOpcodeSetAttribute(MINIGAME, mapID);
		CaseOpcodeSetAttribute(MAPJUMP, mapID);
		CaseOpcodeSetAttribute(PMJMP, mapID);
	default:
		break;
	}
}

qint16 Opcode::subKey() const
{
	switch (id()) {
		CaseOpcodeAttribute(SPECIAL, subKey);
		CaseOpcodeAttribute(KAWAI, subKey);
	default:
		break;
	}

	return -1;
}

qint32 Opcode::keys() const
{
	switch (id()) {
		CaseOpcodeAttribute(IFKEY, keys);
		CaseOpcodeAttribute(IFKEYOFF, keys);
		CaseOpcodeAttribute(IFKEYON, keys);
	default:
		break;
	}

	return -1;
}

void Opcode::setKeys(quint16 keys)
{
	switch (id()) {
		CaseOpcodeSetAttribute(IFKEY, keys);
		CaseOpcodeSetAttribute(IFKEYOFF, keys);
		CaseOpcodeSetAttribute(IFKEYON, keys);
	default:
		break;
	}
}

#define CaseOpcodeIf(op, s) \
case OpcodeKey::op: \
    i.bank1 = B1(_opcode.opcode##op.banks); \
    i.bank2 = B2(_opcode.opcode##op.banks); \
    i.value1 = qint32(_opcode.opcode##op.value1); \
    i.value2 = qint32(_opcode.opcode##op.value2); \
    i.oper = _opcode.opcode##op.oper; \
    i.size = s; \
    return true

#define CaseOpcodeSetIf(op, conv) \
case OpcodeKey::op: \
    _opcode.opcode##op.banks = BANK(i.bank1, i.bank2); \
    _opcode.opcode##op.value1 = conv(i.value1); \
    _opcode.opcode##op.value2 = conv(i.value2); \
    _opcode.opcode##op.oper = i.oper; \
    break

bool Opcode::ifStruct(FF7If &i) const
{
	switch (id()) {
		CaseOpcodeIf(IFUB, FF7Var::Byte);
		CaseOpcodeIf(IFUBL, FF7Var::Byte);
		CaseOpcodeIf(IFSW, FF7Var::SignedWord);
		CaseOpcodeIf(IFSWL, FF7Var::SignedWord);
		CaseOpcodeIf(IFUW, FF7Var::Word);
		CaseOpcodeIf(IFUWL, FF7Var::Word);
	default:
		break;
	}

	return false;
}

void Opcode::setIfStruct(const FF7If &i)
{
	FF7If oldI;

	if (!ifStruct(oldI)) {
		return;
	}

	if (oldI.size != i.size) {
		qint32 j = jump();
		qint32 l = label();
		BadJumpError bje = badJump();
		switch (i.size) {
		case FF7Var::Byte:
			_opcode.id = isLongJump() ? OpcodeKey::IFUBL : OpcodeKey::IFUB;
			break;
		case FF7Var::SignedWord:
			_opcode.id = isLongJump() ? OpcodeKey::IFSWL : OpcodeKey::IFSW;
			break;
		case FF7Var::Word:
			_opcode.id = isLongJump() ? OpcodeKey::IFUWL : OpcodeKey::IFUW;
			break;
		default:
			return;
		}
		setJump(j);
		setLabel(quint16(l));
		setBadJump(bje);
	}

	switch (id()) {
		CaseOpcodeSetIf(IFUB, quint8);
		CaseOpcodeSetIf(IFUBL, quint8);
		CaseOpcodeSetIf(IFSW, qint16);
		CaseOpcodeSetIf(IFSWL, qint16);
		CaseOpcodeSetIf(IFUW, quint16);
		CaseOpcodeSetIf(IFUWL, quint16);
	default:
		break;
	}
}

#define CaseOpcodeBinaryOp(op, l) \
case OpcodeKey::op: \
    o.bank1 = B1(_opcode.opcode##op.banks); \
    o.bank2 = B2(_opcode.opcode##op.banks); \
    o.var = _opcode.opcode##op.var; \
    o.value = _opcode.opcode##op.value; \
    o.isLong = l; \
    return true

#define CaseOpcodeSetBinaryOp(op, conv) \
case OpcodeKey::op: \
    _opcode.opcode##op.banks = BANK(o.bank1, o.bank2); \
    _opcode.opcode##op.var = o.var; \
    _opcode.opcode##op.value = conv(o.value); \
    break

bool Opcode::binaryOperation(FF7BinaryOperation &o) const
{
	switch (id()) {
		CaseOpcodeBinaryOp(PLUSX, false);
		CaseOpcodeBinaryOp(PLUS2X, true);
		CaseOpcodeBinaryOp(MINUSX, false);
		CaseOpcodeBinaryOp(MINUS2X, true);
		CaseOpcodeBinaryOp(SETBYTE, false);
		CaseOpcodeBinaryOp(SETWORD, true);
		CaseOpcodeBinaryOp(PLUS, false);
		CaseOpcodeBinaryOp(PLUS2, true);
		CaseOpcodeBinaryOp(MINUS, false);
		CaseOpcodeBinaryOp(MINUS2, true);
		CaseOpcodeBinaryOp(MUL, false);
		CaseOpcodeBinaryOp(MUL2, true);
		CaseOpcodeBinaryOp(DIV, false);
		CaseOpcodeBinaryOp(DIV2, true);
		CaseOpcodeBinaryOp(MOD, false);
		CaseOpcodeBinaryOp(MOD2, true);
		CaseOpcodeBinaryOp(AND, false);
		CaseOpcodeBinaryOp(AND2, true);
		CaseOpcodeBinaryOp(OR, false);
		CaseOpcodeBinaryOp(OR2, true);
		CaseOpcodeBinaryOp(XOR, false);
		CaseOpcodeBinaryOp(XOR2, true);
		CaseOpcodeBinaryOp(LBYTE, false);
		CaseOpcodeBinaryOp(HBYTE, true);
	default:
		break;
	}

	return false;
}

void Opcode::setBinaryOperation(const FF7BinaryOperation &o)
{
	switch (id()) {
		CaseOpcodeSetBinaryOp(PLUSX, quint8);
		CaseOpcodeSetBinaryOp(PLUS2X, quint16);
		CaseOpcodeSetBinaryOp(MINUSX, quint8);
		CaseOpcodeSetBinaryOp(MINUS2X, quint16);
		CaseOpcodeSetBinaryOp(SETBYTE, quint8);
		CaseOpcodeSetBinaryOp(SETWORD, quint16);
		CaseOpcodeSetBinaryOp(PLUS, quint8);
		CaseOpcodeSetBinaryOp(PLUS2, quint16);
		CaseOpcodeSetBinaryOp(MINUS, quint8);
		CaseOpcodeSetBinaryOp(MINUS2, quint16);
		CaseOpcodeSetBinaryOp(MUL, quint8);
		CaseOpcodeSetBinaryOp(MUL2, quint16);
		CaseOpcodeSetBinaryOp(DIV, quint8);
		CaseOpcodeSetBinaryOp(DIV2, quint16);
		CaseOpcodeSetBinaryOp(MOD, quint8);
		CaseOpcodeSetBinaryOp(MOD2, quint16);
		CaseOpcodeSetBinaryOp(AND, quint8);
		CaseOpcodeSetBinaryOp(AND2, quint16);
		CaseOpcodeSetBinaryOp(OR, quint8);
		CaseOpcodeSetBinaryOp(OR2, quint16);
		CaseOpcodeSetBinaryOp(XOR, quint8);
		CaseOpcodeSetBinaryOp(XOR2, quint16);
		CaseOpcodeSetBinaryOp(LBYTE, quint8);
		CaseOpcodeSetBinaryOp(HBYTE, quint16);
	default:
		break;
	}
}

#define CaseOpcodeUnaryOp(op, l) \
case OpcodeKey::op: \
    o.bank2 = B2(_opcode.opcode##op.banks); \
    o.var = _opcode.opcode##op.var; \
    o.isLong = l; \
    return true

#define CaseOpcodeSetUnaryOp(op) \
case OpcodeKey::op: \
    _opcode.opcode##op.banks = BANK(0, o.bank2); \
    _opcode.opcode##op.var = o.var; \
    break

bool Opcode::unaryOperation(FF7UnaryOperation &o) const
{
	switch (id()) {
		CaseOpcodeUnaryOp(INCX, false);
		CaseOpcodeUnaryOp(INC2X, true);
		CaseOpcodeUnaryOp(DECX, false);
		CaseOpcodeUnaryOp(DEC2X, true);
		CaseOpcodeUnaryOp(INC, false);
		CaseOpcodeUnaryOp(INC2, true);
		CaseOpcodeUnaryOp(DEC, false);
		CaseOpcodeUnaryOp(DEC2, true);
		CaseOpcodeUnaryOp(RANDOM, false);
	default:
		break;
	}

	return false;
}

void Opcode::setUnaryOperation(const FF7UnaryOperation &o)
{
	switch (id()) {
	CaseOpcodeSetUnaryOp(INCX);
	CaseOpcodeSetUnaryOp(INC2X);
	CaseOpcodeSetUnaryOp(DECX);
	CaseOpcodeSetUnaryOp(DEC2X);
	CaseOpcodeSetUnaryOp(INC);
	CaseOpcodeSetUnaryOp(INC2);
	CaseOpcodeSetUnaryOp(DEC);
	CaseOpcodeSetUnaryOp(DEC2);
	CaseOpcodeSetUnaryOp(RANDOM);
	default:
		break;
	}
}

#define CaseOpcodeBitOp(op) \
case OpcodeKey::op: \
    o.bank1 = B1(_opcode.opcode##op.banks); \
    o.bank2 = B2(_opcode.opcode##op.banks); \
    o.var = _opcode.opcode##op.var; \
    o.position = _opcode.opcode##op.position; \
    return true

#define CaseOpcodeSetBitOp(op) \
case OpcodeKey::op: \
    _opcode.opcode##op.banks = BANK(o.bank1, o.bank2); \
    _opcode.opcode##op.var = o.var; \
	_opcode.opcode##op.position = o.position; \
    break

bool Opcode::bitOperation(FF7BitOperation &o) const
{
	switch (id()) {
		CaseOpcodeBitOp(BITON);
		CaseOpcodeBitOp(BITOFF);
		CaseOpcodeBitOp(BITXOR);
	default:
		break;
	}

	return false;
}

void Opcode::setBitOperation(const FF7BitOperation &o)
{
	switch (id()) {
	CaseOpcodeSetBitOp(BITON);
	CaseOpcodeSetBitOp(BITOFF);
	CaseOpcodeSetBitOp(BITXOR);
	default:
		break;
	}
}

int Opcode::label() const
{
	switch (id()) {
		CaseOpcodeAttribute(JMPF, _label);
		CaseOpcodeAttribute(JMPFL, _label);
		CaseOpcodeAttribute(JMPB, _label);
		CaseOpcodeAttribute(JMPBL, _label);
		CaseOpcodeAttribute(IFUB, _label);
		CaseOpcodeAttribute(IFUBL, _label);
		CaseOpcodeAttribute(IFSW, _label);
		CaseOpcodeAttribute(IFSWL, _label);
		CaseOpcodeAttribute(IFUW, _label);
		CaseOpcodeAttribute(IFUWL, _label);
		CaseOpcodeAttribute(Unused1B, _label);
		CaseOpcodeAttribute(IFKEY, _label);
		CaseOpcodeAttribute(IFKEYON, _label);
		CaseOpcodeAttribute(IFKEYOFF, _label);
		CaseOpcodeAttribute(IFPRTYQ, _label);
		CaseOpcodeAttribute(IFMEMBQ, _label);
		CaseOpcodeAttribute(LABEL, _label);
	default:
		break;
	}

	return -1;
}

void Opcode::setLabel(quint16 _label)
{
	switch (id()) {
		CaseOpcodeSetAttribute(JMPF, _label);
		CaseOpcodeSetAttribute(JMPFL, _label);
		CaseOpcodeSetAttribute(JMPB, _label);
		CaseOpcodeSetAttribute(JMPBL, _label);
		CaseOpcodeSetAttribute(IFUB, _label);
		CaseOpcodeSetAttribute(IFUBL, _label);
		CaseOpcodeSetAttribute(IFSW, _label);
		CaseOpcodeSetAttribute(IFSWL, _label);
		CaseOpcodeSetAttribute(IFUW, _label);
		CaseOpcodeSetAttribute(IFUWL, _label);
		CaseOpcodeSetAttribute(Unused1B, _label);
		CaseOpcodeSetAttribute(IFKEY, _label);
		CaseOpcodeSetAttribute(IFKEYON, _label);
		CaseOpcodeSetAttribute(IFKEYOFF, _label);
		CaseOpcodeSetAttribute(IFPRTYQ, _label);
		CaseOpcodeSetAttribute(IFMEMBQ, _label);
		CaseOpcodeSetAttribute(LABEL, _label);
	default:
		break;
	}
}

#define getBadJump() \
	_badJump & 0x7F

BadJumpError Opcode::badJump() const
{
	switch (id()) {
		CaseOpcodeAttrConvert(JMPF, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(JMPFL, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(JMPB, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(JMPBL, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(IFUB, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(IFUBL, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(IFSW, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(IFSWL, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(IFUW, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(IFUWL, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(Unused1B, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(IFKEY, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(IFKEYON, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(IFKEYOFF, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(IFPRTYQ, getBadJump(), BadJumpError);
		CaseOpcodeAttrConvert(IFMEMBQ, getBadJump(), BadJumpError);
	default:
		break;
	}

	return BadJumpError::Ok;
}

void Opcode::setBadJump(BadJumpError badJump)
{
	quint8 _badJump = badJump | quint8(itemIsExpanded() << 7);

	switch (id()) {
		CaseOpcodeSetAttribute(JMPF, _badJump);
		CaseOpcodeSetAttribute(JMPFL, _badJump);
		CaseOpcodeSetAttribute(JMPB, _badJump);
		CaseOpcodeSetAttribute(JMPBL, _badJump);
		CaseOpcodeSetAttribute(IFUB, _badJump);
		CaseOpcodeSetAttribute(IFUBL, _badJump);
		CaseOpcodeSetAttribute(IFSW, _badJump);
		CaseOpcodeSetAttribute(IFSWL, _badJump);
		CaseOpcodeSetAttribute(IFUW, _badJump);
		CaseOpcodeSetAttribute(IFUWL, _badJump);
		CaseOpcodeSetAttribute(Unused1B, _badJump);
		CaseOpcodeSetAttribute(IFKEY, _badJump);
		CaseOpcodeSetAttribute(IFKEYON, _badJump);
		CaseOpcodeSetAttribute(IFKEYOFF, _badJump);
		CaseOpcodeSetAttribute(IFPRTYQ, _badJump);
		CaseOpcodeSetAttribute(IFMEMBQ, _badJump);
	default:
		break;
	}
}

bool Opcode::itemIsExpanded() const
{
	switch (id()) {
		CaseOpcodeAttribute(IFUB, _badJump >> 7);
		CaseOpcodeAttribute(IFUBL, _badJump >> 7);
		CaseOpcodeAttribute(IFSW, _badJump >> 7);
		CaseOpcodeAttribute(IFSWL, _badJump >> 7);
		CaseOpcodeAttribute(IFUW, _badJump >> 7);
		CaseOpcodeAttribute(IFUWL, _badJump >> 7);
		CaseOpcodeAttribute(Unused1B, _badJump >> 7);
		CaseOpcodeAttribute(IFKEY, _badJump >> 7);
		CaseOpcodeAttribute(IFKEYON, _badJump >> 7);
		CaseOpcodeAttribute(IFKEYOFF, _badJump >> 7);
		CaseOpcodeAttribute(IFPRTYQ, _badJump >> 7);
		CaseOpcodeAttribute(IFMEMBQ, _badJump >> 7);
	default:
		break;
	}

	return BadJumpError::Ok;
}

void Opcode::setItemIsExpanded(bool isExpanded)
{
	quint8 _badJump = badJump() | quint8(isExpanded << 7);

	switch (id()) {
		CaseOpcodeSetAttribute(IFUB, _badJump);
		CaseOpcodeSetAttribute(IFUBL, _badJump);
		CaseOpcodeSetAttribute(IFSW, _badJump);
		CaseOpcodeSetAttribute(IFSWL, _badJump);
		CaseOpcodeSetAttribute(IFUW, _badJump);
		CaseOpcodeSetAttribute(IFUWL, _badJump);
		CaseOpcodeSetAttribute(Unused1B, _badJump);
		CaseOpcodeSetAttribute(IFKEY, _badJump);
		CaseOpcodeSetAttribute(IFKEYON, _badJump);
		CaseOpcodeSetAttribute(IFKEYOFF, _badJump);
		CaseOpcodeSetAttribute(IFPRTYQ, _badJump);
		CaseOpcodeSetAttribute(IFMEMBQ, _badJump);
	default:
		break;
	}
}

qint16 Opcode::shortJump() const
{
	switch (id()) {
		CaseOpcodeAttribute(JMPF, jump);
		CaseOpcodeAttribute(JMPB, jump);
		CaseOpcodeAttribute(IFUB, jump);
		CaseOpcodeAttribute(IFSW, jump);
		CaseOpcodeAttribute(IFUW, jump);
		CaseOpcodeAttribute(IFKEY, jump);
		CaseOpcodeAttribute(IFKEYON, jump);
		CaseOpcodeAttribute(IFKEYOFF, jump);
		CaseOpcodeAttribute(IFPRTYQ, jump);
		CaseOpcodeAttribute(IFMEMBQ, jump);
	default:
		break;
	}

	return -1;
}

qint32 Opcode::longJump() const
{
	switch (id()) {
		CaseOpcodeAttribute(JMPFL, jump);
		CaseOpcodeAttribute(JMPBL, jump);
		CaseOpcodeAttribute(IFUBL, jump);
		CaseOpcodeAttribute(IFSWL, jump);
		CaseOpcodeAttribute(IFUWL, jump);
		CaseOpcodeAttribute(Unused1B, jump);
	default:
		break;
	}

	return -1;
}

qint32 Opcode::jump() const
{
	qint32 jump = isLongJump() ? longJump() : shortJump();

	return isBackJump() ? -jump : jump + jumpShift();
}

bool Opcode::setShortJump(quint8 jump)
{
	switch (id()) {
		CaseOpcodeSetAttribute(JMPF, jump);
		CaseOpcodeSetAttribute(JMPB, jump);
		CaseOpcodeSetAttribute(IFUB, jump);
		CaseOpcodeSetAttribute(IFSW, jump);
		CaseOpcodeSetAttribute(IFUW, jump);
		CaseOpcodeSetAttribute(IFKEY, jump);
		CaseOpcodeSetAttribute(IFKEYON, jump);
		CaseOpcodeSetAttribute(IFKEYOFF, jump);
		CaseOpcodeSetAttribute(IFPRTYQ, jump);
		CaseOpcodeSetAttribute(IFMEMBQ, jump);
		CaseOpcodeSetAttribute(Unused1B, jump);
	case OpcodeKey::JMPFL: {
		OpcodeJMPFL op = _opcode.opcodeJMPFL;
		_opcode.opcodeJMPF.id = OpcodeKey::JMPF;
		_opcode.opcodeJMPF.jump = jump;
		_opcode.opcodeJMPF._label = op._label;
		_opcode.opcodeJMPF._badJump = op._badJump;
	}	return true;
	case OpcodeKey::JMPBL: {
		OpcodeJMPBL op = _opcode.opcodeJMPBL;
		_opcode.opcodeJMPB.id = OpcodeKey::JMPB;
		_opcode.opcodeJMPB.jump = jump;
		_opcode.opcodeJMPB._label = op._label;
		_opcode.opcodeJMPB._badJump = op._badJump;
	}	return true;
	case OpcodeKey::IFUBL: {
		OpcodeIFUBL op = _opcode.opcodeIFUBL;
		_opcode.opcodeIFUB.id = OpcodeKey::IFUB;
		_opcode.opcodeIFUB.banks = op.banks;
		_opcode.opcodeIFUB.value1 = op.value1;
		_opcode.opcodeIFUB.value2 = op.value2;
		_opcode.opcodeIFUB.oper = op.oper;
		_opcode.opcodeIFUB.jump = jump;
		_opcode.opcodeIFUB._label = op._label;
		_opcode.opcodeIFUB._badJump = op._badJump;
	}	return true;
	case OpcodeKey::IFSWL: {
		OpcodeIFSWL op = _opcode.opcodeIFSWL;
		_opcode.opcodeIFSW.id = OpcodeKey::IFSW;
		_opcode.opcodeIFSW.banks = op.banks;
		_opcode.opcodeIFSW.value1 = op.value1;
		_opcode.opcodeIFSW.value2 = op.value2;
		_opcode.opcodeIFSW.oper = op.oper;
		_opcode.opcodeIFSW.jump = jump;
		_opcode.opcodeIFSW._label = op._label;
		_opcode.opcodeIFSW._badJump = op._badJump;
	}	return true;
	case OpcodeKey::IFUWL: {
		OpcodeIFUWL op = _opcode.opcodeIFUWL;
		_opcode.opcodeIFUW.id = OpcodeKey::IFUW;
		_opcode.opcodeIFUW.banks = op.banks;
		_opcode.opcodeIFUW.value1 = op.value1;
		_opcode.opcodeIFUW.value2 = op.value2;
		_opcode.opcodeIFUW.oper = op.oper;
		_opcode.opcodeIFUW.jump = jump;
		_opcode.opcodeIFUW._label = op._label;
		_opcode.opcodeIFUW._badJump = op._badJump;
	}	return true;
	default:
		break;
	}

	return false;
}

bool Opcode::setLongJump(quint16 jump)
{
	switch (id()) {
		CaseOpcodeSetAttribute(JMPFL, jump);
		CaseOpcodeSetAttribute(JMPBL, jump);
		CaseOpcodeSetAttribute(IFUBL, jump);
		CaseOpcodeSetAttribute(IFSWL, jump);
		CaseOpcodeSetAttribute(IFUWL, jump);
		CaseOpcodeSetAttribute(Unused1B, jump);
	case OpcodeKey::JMPF: {
		OpcodeJMPF op = _opcode.opcodeJMPF;
		_opcode.opcodeJMPFL.id = OpcodeKey::JMPFL;
		_opcode.opcodeJMPFL.jump = jump;
		_opcode.opcodeJMPFL._label = op._label;
		_opcode.opcodeJMPFL._badJump = op._badJump;
	}	return true;
	case OpcodeKey::JMPB: {
		OpcodeJMPB op = _opcode.opcodeJMPB;
		_opcode.opcodeJMPBL.id = OpcodeKey::JMPBL;
		_opcode.opcodeJMPBL.jump = jump;
		_opcode.opcodeJMPBL._label = op._label;
		_opcode.opcodeJMPBL._badJump = op._badJump;
	}	return true;
	case OpcodeKey::IFUB: {
		OpcodeIFUB op = _opcode.opcodeIFUB;
		_opcode.opcodeIFUBL.id = OpcodeKey::IFUBL;
		_opcode.opcodeIFUBL.banks = op.banks;
		_opcode.opcodeIFUBL.value1 = op.value1;
		_opcode.opcodeIFUBL.value2 = op.value2;
		_opcode.opcodeIFUBL.oper = op.oper;
		_opcode.opcodeIFUBL.jump = jump;
		_opcode.opcodeIFUBL._label = op._label;
		_opcode.opcodeIFUBL._badJump = op._badJump;
	}	return true;
	case OpcodeKey::IFSW: {
		OpcodeIFSW op = _opcode.opcodeIFSW;
		_opcode.opcodeIFSWL.id = OpcodeKey::IFSWL;
		_opcode.opcodeIFSWL.banks = op.banks;
		_opcode.opcodeIFSWL.value1 = op.value1;
		_opcode.opcodeIFSWL.value2 = op.value2;
		_opcode.opcodeIFSWL.oper = op.oper;
		_opcode.opcodeIFSWL.jump = jump;
		_opcode.opcodeIFSWL._label = op._label;
		_opcode.opcodeIFSWL._badJump = op._badJump;
	}	return true;
	case OpcodeKey::IFUW: {
		OpcodeIFUW op = _opcode.opcodeIFUW;
		_opcode.opcodeIFUWL.id = OpcodeKey::IFUWL;
		_opcode.opcodeIFUWL.banks = op.banks;
		_opcode.opcodeIFUWL.value1 = op.value1;
		_opcode.opcodeIFUWL.value2 = op.value2;
		_opcode.opcodeIFUWL.oper = op.oper;
		_opcode.opcodeIFUWL.jump = jump;
		_opcode.opcodeIFUWL._label = op._label;
		_opcode.opcodeIFUWL._badJump = op._badJump;
	}	return true;
	case OpcodeKey::IFKEY:
	case OpcodeKey::IFKEYON:
	case OpcodeKey::IFKEYOFF:
	case OpcodeKey::IFPRTYQ:
	case OpcodeKey::IFMEMBQ:
		setBadJump(BadJumpError::ImpossibleLong);
		return false;
	default:
		break;
	}

	return false;
}

quint8 Opcode::jumpShift() const
{
	switch (id()) {
	case OpcodeKey::JMPF:
	case OpcodeKey::JMPFL:
	case OpcodeKey::Unused1B:
		return 1;
	case OpcodeKey::JMPB:
	case OpcodeKey::JMPBL:
		return 0;
	case OpcodeKey::IFUB:
	case OpcodeKey::IFUBL:
		return 5;
	case OpcodeKey::IFSW:
	case OpcodeKey::IFSWL:
	case OpcodeKey::IFUW:
	case OpcodeKey::IFUWL:
		return 7;
	case OpcodeKey::IFKEY:
	case OpcodeKey::IFKEYON:
	case OpcodeKey::IFKEYOFF:
		return 3;
	case OpcodeKey::IFPRTYQ:
	case OpcodeKey::IFMEMBQ:
		return 2;
	default:
		break;
	}

	return 0;
}

bool Opcode::setJump(qint32 jump)
{
	bool oldIsBackJump = isBackJump(),
	        newIsBackJump = jump < 0;

	if (oldIsBackJump && !newIsBackJump) {
		// convert to forward
		switch (id()) {
		case OpcodeKey::JMPB: {
			OpcodeJMPB op = _opcode.opcodeJMPB;
			_opcode.opcodeJMPF.id = OpcodeKey::JMPF;
			_opcode.opcodeJMPF.jump = op.jump;
			_opcode.opcodeJMPF._label = op._label;
			_opcode.opcodeJMPF._badJump = op._badJump;
		}	break;
		case OpcodeKey::JMPBL: {
			OpcodeJMPBL op = _opcode.opcodeJMPBL;
			_opcode.opcodeJMPFL.id = OpcodeKey::JMPFL;
			_opcode.opcodeJMPFL.jump = op.jump;
			_opcode.opcodeJMPFL._label = op._label;
			_opcode.opcodeJMPFL._badJump = op._badJump;
		}	break;
		default:
			break;
		}
	} else if (!oldIsBackJump && newIsBackJump) {
		// convert to back
		switch (id()) {
		case OpcodeKey::JMPF: {
			OpcodeJMPF op = _opcode.opcodeJMPF;
			_opcode.opcodeJMPB.id = OpcodeKey::JMPB;
			_opcode.opcodeJMPB.jump = op.jump;
			_opcode.opcodeJMPB._label = op._label;
			_opcode.opcodeJMPB._badJump = op._badJump;
		}	break;
		case OpcodeKey::JMPFL: {
			OpcodeJMPFL op = _opcode.opcodeJMPFL;
			_opcode.opcodeJMPBL.id = OpcodeKey::JMPBL;
			_opcode.opcodeJMPBL.jump = op.jump;
			_opcode.opcodeJMPBL._label = op._label;
			_opcode.opcodeJMPBL._badJump = op._badJump;
		}	break;
		default:
			setBadJump(BadJumpError::ImpossibleBackward);
			return false;
		}
	}

	qint32 realJump = isBackJump() ? -jump : jump - jumpShift();

	setBadJump(realJump < 0 ? BadJumpError::BeforeScript : (realJump > 65535 ? BadJumpError::AfterScript : BadJumpError::Ok));

	if (realJump > 255) {
		return setLongJump(quint16(std::min(realJump, 65535)));
	}
	
	return setShortJump(quint8(std::max(realJump, 0)));
}

#define CaseOpcodeBank(op, attr, F) \
case OpcodeKey::op: \
	return F(_opcode.opcode##op.attr);

#define CaseOpcodeSpecialBank(op, attr, F) \
case OpcodeSpecialKey::op: \
	return F(_opcode.opcodeSPECIAL##op.attr);

#define SwitchOpcodeBankLen3(i, F) \
	CaseOpcodeBank(SPLIT, banks[i], F) \
	CaseOpcodeBank(CMTRA, banks[i], F) \
	CaseOpcodeBank(SEARCHX, banks[i], F) \
	CaseOpcodeBank(SLINE, banks[i], F) \
	CaseOpcodeBank(AKAO2, banks[i], F) \
	CaseOpcodeBank(MPPAL, banks[i], F) \
	CaseOpcodeBank(ADPAL, banks[i], F) \
	CaseOpcodeBank(MPPAL2, banks[i], F) \
	CaseOpcodeBank(ADPAL2, banks[i], F) \
	CaseOpcodeBank(AKAO, banks[i], F)

#define SwitchOpcodeBankLen2(i, F) \
	CaseOpcodeBank(SPTYE, banks[i], F) \
	CaseOpcodeBank(GTPYE, banks[i], F) \
	CaseOpcodeBank(NFADE, banks[i], F) \
	CaseOpcodeBank(STTIM, banks[i], F) \
	CaseOpcodeBank(GWCOL, banks[i], F) \
	CaseOpcodeBank(SWCOL, banks[i], F) \
	CaseOpcodeBank(SMTRA, banks[i], F) \
	CaseOpcodeBank(DMTRA, banks[i], F) \
	CaseOpcodeBank(SHAKE, banks[i], F) \
	CaseOpcodeBank(SCR2DC, banks[i], F) \
	CaseOpcodeBank(SCR2DL, banks[i], F) \
	CaseOpcodeBank(FADE, banks[i], F) \
	CaseOpcodeBank(PXYZI, banks[i], F) \
	CaseOpcodeBank(TOBYTE, banks[i], F) \
	CaseOpcodeBank(SETX, banks[i], F) \
	CaseOpcodeBank(GETX, banks[i], F) \
	CaseOpcodeBank(XYZ, banks[i], F) \
	CaseOpcodeBank(XYZI, banks[i], F) \
	CaseOpcodeBank(JUMP, banks[i], F) \
	CaseOpcodeBank(AXYZI, banks[i], F) \
	CaseOpcodeBank(LADER, banks[i], F) \
	CaseOpcodeBank(OFST, banks[i], F) \
	CaseOpcodeBank(SIN, banks[i], F) \
	CaseOpcodeBank(COS, banks[i], F) \
	CaseOpcodeBank(RTPAL, banks[i], F) \
	CaseOpcodeBank(CPPAL2, banks[i], F) \
	CaseOpcodeBank(RTPAL2, banks[i], F) \
	SwitchOpcodeBankLen3(i, F)

#define SwitchOpcodeBankLen1(i, F) \
	case OpcodeKey::SPECIAL: \
		switch (_opcode.opcodeSPECIAL.subKey) { \
			CaseOpcodeSpecialBank(PNAME, banks, F) \
			CaseOpcodeSpecialBank(GMSPD, banks, F) \
			CaseOpcodeSpecialBank(SMSPD, banks, F) \
		} \
	break; \
	CaseOpcodeBank(IFUB, banks, F) \
	CaseOpcodeBank(IFUBL, banks, F) \
	CaseOpcodeBank(IFSW, banks, F) \
	CaseOpcodeBank(IFSWL, banks, F) \
	CaseOpcodeBank(IFUW, banks, F) \
	CaseOpcodeBank(IFUWL, banks, F) \
	CaseOpcodeBank(BTRLD, banks, F) \
	CaseOpcodeBank(BGPDH, banks, F) \
	CaseOpcodeBank(BGSCR, banks, F) \
	CaseOpcodeBank(WNUMB, banks, F) \
	CaseOpcodeBank(GOLDu, banks, F) \
	CaseOpcodeBank(GOLDd, banks, F) \
	CaseOpcodeBank(CHGLD, banks, F) \
	CaseOpcodeBank(MPARA, banks, F) \
	CaseOpcodeBank(MPRA2, banks, F) \
	CaseOpcodeBank(MPu, banks, F) \
	CaseOpcodeBank(MPd, banks, F) \
	CaseOpcodeBank(HPu, banks, F) \
	CaseOpcodeBank(HPd, banks, F) \
	CaseOpcodeBank(ASK, banks, F) \
	CaseOpcodeBank(MENU, banks, F) \
	CaseOpcodeBank(STITM, banks, F) \
	CaseOpcodeBank(DLITM, banks, F) \
	CaseOpcodeBank(CKITM, banks, F) \
	CaseOpcodeBank(SCRLC, banks, F) \
	CaseOpcodeBank(SCRLA, banks, F) \
	CaseOpcodeBank(SCR2D, banks, F) \
	CaseOpcodeBank(VWOFT, banks, F) \
	CaseOpcodeBank(LSTMP, banks, F) \
	CaseOpcodeBank(SCRLP, banks, F) \
	CaseOpcodeBank(BATTLE, banks, F) \
	CaseOpcodeBank(PGTDR, banks, F) \
	CaseOpcodeBank(GETPC, banks, F) \
	CaseOpcodeBank(PLUSX, banks, F) \
	CaseOpcodeBank(PLUS2X, banks, F) \
	CaseOpcodeBank(MINUSX, banks, F) \
	CaseOpcodeBank(MINUS2X, banks, F) \
	CaseOpcodeBank(INCX, banks, F) \
	CaseOpcodeBank(INC2X, banks, F) \
	CaseOpcodeBank(DECX, banks, F) \
	CaseOpcodeBank(DEC2X, banks, F) \
	CaseOpcodeBank(RDMSD, banks, F) \
	CaseOpcodeBank(SETBYTE, banks, F) \
	CaseOpcodeBank(SETWORD, banks, F) \
	CaseOpcodeBank(BITON, banks, F) \
	CaseOpcodeBank(BITOFF, banks, F) \
	CaseOpcodeBank(BITXOR, banks, F) \
	CaseOpcodeBank(PLUS, banks, F) \
	CaseOpcodeBank(PLUS2, banks, F) \
	CaseOpcodeBank(MINUS, banks, F) \
	CaseOpcodeBank(MINUS2, banks, F) \
	CaseOpcodeBank(MUL, banks, F) \
	CaseOpcodeBank(MUL2, banks, F) \
	CaseOpcodeBank(DIV, banks, F) \
	CaseOpcodeBank(DIV2, banks, F) \
	CaseOpcodeBank(MOD, banks, F) \
	CaseOpcodeBank(MOD2, banks, F) \
	CaseOpcodeBank(AND, banks, F) \
	CaseOpcodeBank(AND2, banks, F) \
	CaseOpcodeBank(OR, banks, F) \
	CaseOpcodeBank(OR2, banks, F) \
	CaseOpcodeBank(XOR, banks, F) \
	CaseOpcodeBank(XOR2, banks, F) \
	CaseOpcodeBank(INC, banks, F) \
	CaseOpcodeBank(INC2, banks, F) \
	CaseOpcodeBank(DEC, banks, F) \
	CaseOpcodeBank(DEC2, banks, F) \
	CaseOpcodeBank(RANDOM, banks, F) \
	CaseOpcodeBank(LBYTE, banks, F) \
	CaseOpcodeBank(HBYTE, banks, F) \
	CaseOpcodeBank(MOVE, banks, F) \
	CaseOpcodeBank(CMOVE, banks, F) \
	CaseOpcodeBank(FMOVE, banks, F) \
	CaseOpcodeBank(MSPED, banks, F) \
	CaseOpcodeBank(DIR, banks, F) \
	CaseOpcodeBank(TURNGEN, banks, F) \
	CaseOpcodeBank(TURN, banks, F) \
	CaseOpcodeBank(GETDIR, banks, F) \
	CaseOpcodeBank(GETAXY, banks, F) \
	CaseOpcodeBank(GETAI, banks, F) \
	CaseOpcodeBank(ASPED, banks, F) \
	CaseOpcodeBank(TALKR, banks, F) \
	CaseOpcodeBank(SLIDR, banks, F) \
	CaseOpcodeBank(TLKR2, banks, F) \
	CaseOpcodeBank(SLDR2, banks, F) \
	CaseOpcodeBank(BGON, banks, F) \
	CaseOpcodeBank(BGOFF, banks, F) \
	CaseOpcodeBank(BGROL, banks, F) \
	CaseOpcodeBank(BGROL2, banks, F) \
	CaseOpcodeBank(BGCLR, banks, F) \
	CaseOpcodeBank(STPAL, banks, F) \
	CaseOpcodeBank(LDPAL, banks, F) \
	CaseOpcodeBank(CPPAL, banks, F) \
	CaseOpcodeBank(CHMPH, banks, F) \
	CaseOpcodeBank(MVIEF, banks, F) \
	CaseOpcodeBank(CMUSC, banks, F) \
	CaseOpcodeBank(CHMST, banks, F) \
	SwitchOpcodeBankLen2(i, F)

qint8 Opcode::bank1() const
{
	switch (id()) {
		SwitchOpcodeBankLen1(0, B1)
	default:
		return -1;
	}
	return -1;
}

qint8 Opcode::bank2() const
{
	switch (id()) {
		SwitchOpcodeBankLen1(0, B2)
	default:
		return -1;
	}
	return -1;
}

qint8 Opcode::bank3() const
{
	switch (id()) {
		SwitchOpcodeBankLen2(1, B1)
	default:
		break;
	}
	return -1;
}

qint8 Opcode::bank4() const
{
	switch (id()) {
		SwitchOpcodeBankLen2(1, B2)
	default:
		break;
	}
	return -1;
}

qint8 Opcode::bank5() const
{
	switch (id()) {
		SwitchOpcodeBankLen3(2, B1)
	default:
		break;
	}
	return -1;
}

qint8 Opcode::bank6() const
{
	switch (id()) {
		SwitchOpcodeBankLen3(2, B2)
	default:
		break;
	}
	return -1;
}

qint16 Opcode::windowID() const
{
	switch (id()) {
		CaseOpcodeAttribute(WCLS, windowID);
		CaseOpcodeAttribute(WSIZW, windowID);
		CaseOpcodeAttribute(WSPCL, windowID);
		CaseOpcodeAttribute(WNUMB, windowID);
		CaseOpcodeAttribute(MESSAGE, windowID);
		CaseOpcodeAttribute(MPARA, windowID);
		CaseOpcodeAttribute(MPRA2, windowID);
		CaseOpcodeAttribute(ASK, windowID);
		CaseOpcodeAttribute(WINDOW, windowID);
		CaseOpcodeAttribute(WMOVE, windowID);
		CaseOpcodeAttribute(WMODE, windowID);
		CaseOpcodeAttribute(WREST, windowID);
		CaseOpcodeAttribute(WCLSE, windowID);
		CaseOpcodeAttribute(WROW, windowID);
	default:
		break;
	}
	return -1;
}

void Opcode::setWindowID(quint8 windowID)
{
	switch (id()) {
		CaseOpcodeSetAttribute(WCLS, windowID);
		CaseOpcodeSetAttribute(WSIZW, windowID);
		CaseOpcodeSetAttribute(WSPCL, windowID);
		CaseOpcodeSetAttribute(WNUMB, windowID);
		CaseOpcodeSetAttribute(MESSAGE, windowID);
		CaseOpcodeSetAttribute(MPARA, windowID);
		CaseOpcodeSetAttribute(MPRA2, windowID);
		CaseOpcodeSetAttribute(ASK, windowID);
		CaseOpcodeSetAttribute(WINDOW, windowID);
		CaseOpcodeSetAttribute(WMOVE, windowID);
		CaseOpcodeSetAttribute(WMODE, windowID);
		CaseOpcodeSetAttribute(WREST, windowID);
		CaseOpcodeSetAttribute(WCLSE, windowID);
		CaseOpcodeSetAttribute(WROW, windowID);
	default:
		break;
	}
}

#define CaseOpcodeWindow(op, window) \
case OpcodeKey::op: \
    window.x = _opcode.opcode##op.targetX; \
    window.y = _opcode.opcode##op.targetY; \
    window.w = _opcode.opcode##op.width; \
    window.h = _opcode.opcode##op.height; \
    return true

#define CaseOpcodeSetWindow(op, window) \
case OpcodeKey::op: \
    _opcode.opcode##op.targetX = window.x; \
    _opcode.opcode##op.targetY = window.y; \
    _opcode.opcode##op.width = window.w; \
    _opcode.opcode##op.height = window.h; \
    break

bool Opcode::window(FF7Window &window) const
{
	switch (id()) {
	case OpcodeKey::WREST:
		window.x = 8;
		window.y = 149;
		window.w = 304;
		window.h = 74;

		return true;
		CaseOpcodeWindow(WSIZW, window);
		CaseOpcodeWindow(WINDOW, window);
	default:
		break;
	}

	return false;
}

void Opcode::setWindow(const FF7Window &window)
{
	switch (id()) {
		CaseOpcodeSetWindow(WSIZW, window);
		CaseOpcodeSetWindow(WINDOW, window);
	default:
		break;
	}
}

#define CaseOpcodeData(op, attr) \
case OpcodeKey::op: \
    return _opcode.opcode##op.attr ? *_opcode.opcode##op.attr : QByteArray()

#define CaseOpcodeSetData(op, attr) \
case OpcodeKey::op: \
	if (_opcode.opcode##op.attr != nullptr) { \
		*_opcode.opcode##op.attr = attr; \
	} else { \
		_opcode.opcode##op.attr = new (std::nothrow) QByteArray(attr); \
	} \
	break

#define CaseOpcodeClearData(op, attr) \
case OpcodeKey::op: \
	_opcode.opcode##op.attr = nullptr; \
	break

#define CaseOpcodeDeleteData(op, attr) \
case OpcodeKey::op: \
	if (_opcode.opcode##op.attr != nullptr) { \
		delete _opcode.opcode##op.attr; \
		_opcode.opcode##op.attr = nullptr; \
	} \
	break

QByteArray Opcode::resizableData() const
{
	switch (id()) {
		CaseOpcodeData(KAWAI, _data);
		CaseOpcodeData(Unused1C, _data);
	default:
		break;
	}
	return QByteArray();
}

void Opcode::setResizableData(const QByteArray &_data)
{
	switch (id()) {
		CaseOpcodeSetData(KAWAI, _data);
		CaseOpcodeSetData(Unused1C, _data);
	default:
		break;
	}
}

void Opcode::clearResizableDataPointers()
{
	switch (id()) {
		CaseOpcodeClearData(KAWAI, _data);
		CaseOpcodeClearData(Unused1C, _data);
	default:
		break;
	}
}

void Opcode::deleteResizableData()
{
	switch (id()) {
		CaseOpcodeDeleteData(KAWAI, _data);
		CaseOpcodeDeleteData(Unused1C, _data);
	default:
		break;
	}
}

#define OpcodeAddVar(op, bnk, attr, type) \
	b = bank##bnk(); \
	if (b > 0) { \
		vars.append(FF7Var(b, (_opcode.opcode##op.attr) & 0xFF, FF7Var::VarSize((FF7Var::type) & 0x7), FF7Var::Flag((FF7Var::type) >> 3))); \
	}

#define CaseOpcodeAddVar1(op, attr, type, bnk) \
	case OpcodeKey::op: \
		OpcodeAddVar(op, bnk, attr, type) \
		return true

#define CaseOpcodeAddVarSpecial1(op, attr, type, bnk) \
	case OpcodeSpecialKey::op: \
		OpcodeAddVar(SPECIAL##op, bnk, attr, type) \
		return true

#define CaseOpcodeAddVar2(op, attr1, type1, attr2, type2) \
	case OpcodeKey::op: \
		OpcodeAddVar(op, 1, attr1, type1) \
		OpcodeAddVar(op, 2, attr2, type2) \
		return true

#define CaseOpcodeAddVar3(op, attr1, type1, attr2, type2, attr3, type3, bnk3) \
	case OpcodeKey::op: \
		OpcodeAddVar(op, 1, attr1, type1) \
		OpcodeAddVar(op, 2, attr2, type2) \
		OpcodeAddVar(op, bnk3, attr3, type3) \
		return true

#define CaseOpcodeAddVarX3(op, attr2, type2, attr3, type3) \
	case OpcodeKey::op: \
		OpcodeAddVar(op, 2, attr2, type2) \
		OpcodeAddVar(op, 4, attr3, type3) \
		return true

#define CaseOpcodeAddVar4(op, attr1, type1, attr2, type2, attr3, type3, attr4, type4) \
	case OpcodeKey::op: \
		OpcodeAddVar(op, 1, attr1, type1) \
		OpcodeAddVar(op, 2, attr2, type2) \
		OpcodeAddVar(op, 3, attr3, type3) \
		OpcodeAddVar(op, 4, attr4, type4) \
		return true

#define CaseOpcodeAddVar5(op, attr1, type1, attr2, type2, attr3, type3, attr4, type4, attr5, type5, bnk5) \
	case OpcodeKey::op: \
		OpcodeAddVar(op, 1, attr1, type1) \
		OpcodeAddVar(op, 2, attr2, type2) \
		OpcodeAddVar(op, 3, attr3, type3) \
		OpcodeAddVar(op, 4, attr4, type4) \
		OpcodeAddVar(op, bnk5, attr5, type5) \
		return true

#define CaseOpcodeAddVar6(op, attr1, type1, attr2, type2, attr3, type3, attr4, type4, attr5, type5, attr6, type6) \
	case OpcodeKey::op: \
		OpcodeAddVar(op, 1, attr1, type1) \
		OpcodeAddVar(op, 2, attr2, type2) \
		OpcodeAddVar(op, 3, attr3, type3) \
		OpcodeAddVar(op, 4, attr4, type4) \
		OpcodeAddVar(op, 5, attr4, type5) \
		OpcodeAddVar(op, 6, attr4, type6) \
		return true

#define CaseOpcodeAddVarX6(op, attr2, type2, attr3, type3, attr4, type4, attr6, type6) \
	case OpcodeKey::op: \
		OpcodeAddVar(op, 2, attr2, type2) \
		OpcodeAddVar(op, 3, attr3, type3) \
		OpcodeAddVar(op, 4, attr4, type4) \
		OpcodeAddVar(op, 6, attr6, type6) \
		return true

#define IsWritable FF7Var::Writable

bool Opcode::variables(QList<FF7Var> &vars) const
{
	qint8 b;

	switch (id()) {
		CaseOpcodeAddVar6(SPLIT,
		                  targetX1, SignedWord,
		                  targetY1, SignedWord,
		                  direction1, Byte,
		                  targetX2, SignedWord,
		                  targetY2, SignedWord,
		                  direction2, Byte);
		CaseOpcodeAddVar3(SPTYE,
		                  charID1, Byte,
		                  charID2, Byte,
		                  charID3, Byte, 3);
		CaseOpcodeAddVar3(GTPYE,
		                  varCharID1, Byte | IsWritable,
		                  varCharID2, Byte | IsWritable,
		                  varCharID3, Byte | IsWritable, 3);
	case OpcodeKey::SPECIAL:
		switch (_opcode.opcodeSPECIAL.subKey) {
		CaseOpcodeAddVarSpecial1(PNAME, varOrValue, Byte, 2);
		CaseOpcodeAddVarSpecial1(GMSPD, varSpeed, Byte | IsWritable, 2);
		CaseOpcodeAddVarSpecial1(SMSPD, speed, Byte, 2);
		}
	break;
		CaseOpcodeAddVar2(IFUB,
		                  value1, Byte,
		                  value2, Byte);
		CaseOpcodeAddVar2(IFUBL,
		                  value1, Byte,
		                  value2, Byte);
		CaseOpcodeAddVar2(IFSW,
		                  value1, SignedWord,
		                  value2, SignedWord);
		CaseOpcodeAddVar2(IFSWL,
		                  value1, SignedWord,
		                  value2, SignedWord);
		CaseOpcodeAddVar2(IFUW,
		                  value1, Word,
		                  value2, Word);
		CaseOpcodeAddVar2(IFUWL,
		                  value1, Word,
		                  value2, Word);
		CaseOpcodeAddVar1(BTRLD, var, Byte, 2);
		CaseOpcodeAddVar4(NFADE, r, Byte, g, Byte, b, Byte, speed, Word);
		CaseOpcodeAddVar1(BGPDH, targetZ, SignedWord, 1);
		CaseOpcodeAddVar2(BGSCR, targetX, SignedWord, targetY, SignedWord);
		CaseOpcodeAddVar2(WNUMB, value, Word, value >> 16, Word);
		CaseOpcodeAddVar3(STTIM, h, Byte, m, Byte, s, Byte, 4);
		CaseOpcodeAddVar2(GOLDu, value, Word, value >> 16, Word);
		CaseOpcodeAddVar2(GOLDd, value, Word, value >> 16, Word);
		CaseOpcodeAddVar2(CHGLD, var1, Word | IsWritable, var2, Word | IsWritable);
		CaseOpcodeAddVar1(MPARA, value, Byte, 2);
		CaseOpcodeAddVar1(MPRA2, value, Word, 2);
		CaseOpcodeAddVar1(MPu, value, Word, 2);
		CaseOpcodeAddVar1(MPd, value, Word, 2);
		CaseOpcodeAddVar1(ASK, varAnswer, Byte | IsWritable, 2);
		CaseOpcodeAddVar1(HPu, value, Word, 2);
		CaseOpcodeAddVar1(HPd, value, Word, 2);
		CaseOpcodeAddVar1(MENU, param, Byte, 2);
		CaseOpcodeAddVar4(GWCOL, corner, Byte, varR, Byte | IsWritable, varG, Byte | IsWritable, varB, Byte | IsWritable);
		CaseOpcodeAddVar4(SWCOL, corner, Byte, r, Byte, g, Byte, b, Byte);
		CaseOpcodeAddVar2(STITM, itemID, Word, quantity, Byte);
		CaseOpcodeAddVar2(DLITM, itemID, Word, quantity, Byte);
		CaseOpcodeAddVar2(CKITM, itemID, Word, quantity, Byte);
		CaseOpcodeAddVar4(SMTRA, materiaID, Byte, APCount[0], Byte, APCount[1], Byte, APCount[2], Byte);
		CaseOpcodeAddVar4(DMTRA, materiaID, Byte, APCount[0], Byte, APCount[1], Byte, APCount[2], Byte);
		CaseOpcodeAddVar6(CMTRA, materiaID, Byte, APCount[0], Byte, APCount[1], Byte, APCount[2], Byte, APCount[3], Byte, varQuantity, Byte | IsWritable);
		CaseOpcodeAddVar4(SHAKE, xAmplitude, Byte, xFrames, Byte, yAmplitude, Byte, yFrames, Byte);
		CaseOpcodeAddVar1(SCRLC, speed, Word, 2);
		CaseOpcodeAddVar1(SCRLA, speed, Word, 2);
		CaseOpcodeAddVar2(SCR2D, targetX, SignedWord, targetY, SignedWord);
		CaseOpcodeAddVar3(SCR2DC, targetX, SignedWord, targetY, SignedWord, speed, Word, 4);
		CaseOpcodeAddVar3(SCR2DL, targetX, SignedWord, targetY, SignedWord, speed, Word, 4);
		CaseOpcodeAddVar2(VWOFT, unknown1, Word, unknown2, Word);
		CaseOpcodeAddVar3(FADE, r, Byte, g, Byte, b, Byte, 4);
		CaseOpcodeAddVar1(LSTMP, var, Word | IsWritable, 2);
		CaseOpcodeAddVar1(SCRLP, speed, Word, 2);
		CaseOpcodeAddVar1(BATTLE, battleID, Word, 2);
		CaseOpcodeAddVar1(PGTDR, varDir, Byte | IsWritable, 2);
		CaseOpcodeAddVar1(GETPC, varPC, Byte | IsWritable, 2);
		CaseOpcodeAddVar4(PXYZI, varX, SignedWord | IsWritable, varY, SignedWord | IsWritable, varZ, SignedWord | IsWritable, varI, Word | IsWritable);
		CaseOpcodeAddVar2(PLUSX, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(PLUS2X, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar2(MINUSX, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(MINUS2X, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar1(INCX, var, Byte | IsWritable, 2);
		CaseOpcodeAddVar1(INC2X, var, Word | IsWritable, 2);
		CaseOpcodeAddVar1(DECX, var, Byte | IsWritable, 2);
		CaseOpcodeAddVar1(DEC2X, var, Word | IsWritable, 2);
		CaseOpcodeAddVar1(RDMSD, value, Word, 2);
		CaseOpcodeAddVar2(SETBYTE, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(SETWORD, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar2(BITON, var, Bit | IsWritable, position, Byte);
		CaseOpcodeAddVar2(BITOFF, var, Bit | IsWritable, position, Byte);
		CaseOpcodeAddVar2(BITXOR, var, Bit | IsWritable, position, Byte);
		CaseOpcodeAddVar2(PLUS, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(PLUS2, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar2(MINUS, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(MINUS2, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar2(MUL, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(MUL2, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar2(DIV, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(DIV2, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar2(MOD, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(MOD2, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar2(AND, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(AND2, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar2(OR, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(OR2, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar2(XOR, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(XOR2, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar1(INC, var, Byte | IsWritable, 2);
		CaseOpcodeAddVar1(INC2, var, Word | IsWritable, 2);
		CaseOpcodeAddVar1(DEC, var, Byte | IsWritable, 2);
		CaseOpcodeAddVar1(DEC2, var, Word | IsWritable, 2);
		CaseOpcodeAddVar1(RANDOM, var, Byte | IsWritable, 2);
		CaseOpcodeAddVar2(LBYTE, var, Byte | IsWritable, value, Byte);
		CaseOpcodeAddVar2(HBYTE, var, Word | IsWritable, value, Word);
		CaseOpcodeAddVar3(TOBYTE, var, Word | IsWritable, value1, Byte, value2, Byte, 4);
		CaseOpcodeAddVarX3(SETX, varOrValue1, Word, varOrValue2, Byte);
		CaseOpcodeAddVarX3(GETX, varOrValue1, Word, var | IsWritable, Byte);
		CaseOpcodeAddVarX6(SEARCHX, start, Word, end, Word, value, Byte, varResult | IsWritable, Word);
		CaseOpcodeAddVar3(XYZ, targetX, SignedWord, targetY, SignedWord, targetZ, SignedWord, 3);
		CaseOpcodeAddVar4(XYZI, targetX, SignedWord, targetY, SignedWord, targetZ, SignedWord, targetI, Word);
		CaseOpcodeAddVar3(XYI, targetX, SignedWord, targetY, SignedWord, targetI, Word, 3);
		CaseOpcodeAddVar2(MOVE, targetX, SignedWord, targetY, SignedWord);
		CaseOpcodeAddVar2(CMOVE, targetX, SignedWord, targetY, SignedWord);
		CaseOpcodeAddVar2(FMOVE, targetX, SignedWord, targetY, SignedWord);
		CaseOpcodeAddVar1(MSPED, speed, Word, 2);
		CaseOpcodeAddVar1(DIR, direction, Byte, 2);
		CaseOpcodeAddVar1(TURNGEN, speed, Byte, 2);
		CaseOpcodeAddVar1(TURN, speed, Byte, 2);
		CaseOpcodeAddVar1(GETDIR, varDir, Byte | IsWritable, 2);
		CaseOpcodeAddVar2(GETAXY, varX, SignedWord | IsWritable, varY, SignedWord | IsWritable);
		CaseOpcodeAddVar1(GETAI, varI, Word | IsWritable, 2);
		CaseOpcodeAddVar1(ASPED, speed, Word, 2);
		CaseOpcodeAddVar4(JUMP, targetX, SignedWord, targetY, SignedWord, targetI, Word, height, SignedWord);
		CaseOpcodeAddVar4(AXYZI, varX | IsWritable, SignedWord, varY | IsWritable, SignedWord, varZ | IsWritable, SignedWord, varI | IsWritable, Word);
		CaseOpcodeAddVar4(LADER, targetX, SignedWord, targetY, SignedWord, targetZ, SignedWord, targetI, Word);
		CaseOpcodeAddVar4(OFST, targetX, SignedWord, targetY, SignedWord, targetZ, SignedWord, speed, Word);
		CaseOpcodeAddVar1(TALKR, range, Byte, 2);
		CaseOpcodeAddVar1(SLIDR, range, Byte, 2);
		CaseOpcodeAddVar1(TLKR2, range, Word, 2);
		CaseOpcodeAddVar1(SLDR2, range, Word, 2);
		CaseOpcodeAddVar6(SLINE, targetX1, SignedWord, targetY1, SignedWord, targetZ1, SignedWord, targetX2, SignedWord, targetY2, SignedWord, targetZ2, SignedWord);
		CaseOpcodeAddVar4(SIN, value1, SignedWord, value2, SignedWord, value3, SignedWord, var, SignedWord | IsWritable);
		CaseOpcodeAddVar4(COS, value1, SignedWord, value2, SignedWord, value3, SignedWord, var, SignedWord | IsWritable);
		CaseOpcodeAddVar5(AKAO2, param1, Word, param2, Word, param3, Word, param4, Word, param5, Word, 6);
		CaseOpcodeAddVar5(MPPAL, start, Byte, b, Byte, g, Byte, r, Byte, colorCount, Word, 6);
		CaseOpcodeAddVar2(BGON, bgParamID, Byte, bgStateID, Byte);
		CaseOpcodeAddVar2(BGOFF, bgParamID, Byte, bgStateID, Byte);
		CaseOpcodeAddVar1(BGROL, bgParamID, Byte, 2);
		CaseOpcodeAddVar1(BGROL2, bgParamID, Byte, 2);
		CaseOpcodeAddVar1(BGCLR, bgParamID, Byte, 2);
		CaseOpcodeAddVar2(STPAL, palID, Byte, position, Byte);
		CaseOpcodeAddVar2(LDPAL, position, Byte, palID, Byte);
		CaseOpcodeAddVar2(CPPAL, posSrc, Byte, posDst, Byte);
		CaseOpcodeAddVar3(RTPAL, posSrc, Byte, posDst, Byte, start, Byte, 4);
		CaseOpcodeAddVar5(ADPAL, posSrc, Byte, posDst, Byte, b, Byte, g, Byte, r, Byte, 5);
		CaseOpcodeAddVar5(MPPAL2, posSrc, Byte, posDst, Byte, b, Byte, g, Byte, r, Byte, 5);
		CaseOpcodeAddVar3(CPPAL2, posSrc, Byte, posDst, Byte, colorCount, Byte, 4);
		CaseOpcodeAddVar3(RTPAL2, posSrc, Byte, posDst, Byte, start, Byte, 4);
		CaseOpcodeAddVar5(ADPAL2, start, Byte, b, Byte, g, Byte, r, Byte, colorCount, Byte, 6);
		CaseOpcodeAddVar2(SOUND, soundID, Word, position, Byte);
		CaseOpcodeAddVar5(AKAO, param1, Byte, param2, Word, param3, Word, param4, Word, param5, Word, 6);
		CaseOpcodeAddVar2(CHMPH, var1, Byte | IsWritable, var2, Byte | IsWritable);
		CaseOpcodeAddVar1(MVIEF, varCurMovieFrame, Byte, 2);
		CaseOpcodeAddVar2(CMUSC, param1, Byte, param2, Byte);
		CaseOpcodeAddVar1(CHMST, var, Byte | IsWritable, 2);
	default:
		break;
	}

	return false;
}

#undef IsWritable

bool Opcode::modelPosition(FF7Position &position) const
{
	switch (id()) {
	case OpcodeKey::XYZI:
		if (_opcode.opcodeXYZI.banks[0] || _opcode.opcodeXYZI.banks[1]) {
			return false;
		}
		position.x = _opcode.opcodeXYZI.targetX;
		position.y = _opcode.opcodeXYZI.targetY;
		position.z = _opcode.opcodeXYZI.targetZ;
		position.id = _opcode.opcodeXYZI.targetI;
		position.hasId = true;
		position.hasZ = true;

		return true;
	case OpcodeKey::XYI:
		if (_opcode.opcodeXYI.banks[0] || _opcode.opcodeXYI.banks[1]) {
			return false;
		}
		position.x = _opcode.opcodeXYI.targetX;
		position.y = _opcode.opcodeXYI.targetY;
		position.id = _opcode.opcodeXYI.targetI;
		position.hasId = true;
		position.hasZ = false;

		return true;
	case OpcodeKey::XYZ:
		if (_opcode.opcodeXYZ.banks[0] || _opcode.opcodeXYZ.banks[1]) {
			return false;
		}
		position.x = _opcode.opcodeXYZ.targetX;
		position.y = _opcode.opcodeXYZ.targetY;
		position.z = _opcode.opcodeXYZ.targetZ;
		position.hasId = false;
		position.hasZ = true;

		return true;
	case OpcodeKey::MOVE:
		if (_opcode.opcodeMOVE.banks) {
			return false;
		}
		position.x = _opcode.opcodeMOVE.targetX;
		position.y = _opcode.opcodeMOVE.targetY;
		position.hasId = false;
		position.hasZ = false;

		return true;
	default:
		break;
	}

	return false;
}

bool Opcode::linePosition(FF7Position position[2]) const
{
	switch (id()) {
	case OpcodeKey::LINE:
		position[0].x = _opcode.opcodeLINE.targetX1;
		position[0].y = _opcode.opcodeLINE.targetY1;
		position[0].z = _opcode.opcodeLINE.targetZ1;
		position[0].hasId = false;
		position[0].hasZ = true;
		position[1].x = _opcode.opcodeLINE.targetX2;
		position[1].y = _opcode.opcodeLINE.targetY2;
		position[1].z = _opcode.opcodeLINE.targetZ2;
		position[1].hasId = false;
		position[1].hasZ = true;

		return true;
	case OpcodeKey::SLINE:
		if (_opcode.opcodeSLINE.banks[0] || _opcode.opcodeSLINE.banks[1] || _opcode.opcodeSLINE.banks[2]) {
			return false;
		}
		position[0].x = _opcode.opcodeSLINE.targetX1;
		position[0].y = _opcode.opcodeSLINE.targetY1;
		position[0].z = _opcode.opcodeSLINE.targetZ1;
		position[0].hasId = false;
		position[0].hasZ = true;
		position[1].x = _opcode.opcodeSLINE.targetX2;
		position[1].y = _opcode.opcodeSLINE.targetY2;
		position[1].z = _opcode.opcodeSLINE.targetZ2;
		position[1].hasId = false;
		position[1].hasZ = true;

		return true;
	default:
		break;
	}

	return false;
}

#define CaseToString(op) \
	case OpcodeKey::op: \
		return toString##op(scriptsAndTexts, _opcode.opcode##op)
#define CaseToStringSpecial(op) \
	case OpcodeSpecialKey::op: \
		return toStringSPECIAL##op(scriptsAndTexts, _opcode.opcodeSPECIAL##op)
#define CaseToStringKawai(op) \
	case OpcodeKawaiKey::op: { \
		OpcodeKAWAI##op kawai = OpcodeKAWAI##op(); \
		if (_opcode.opcodeKAWAI._data != nullptr && !_opcode.opcodeKAWAI._data->isEmpty()) { \
			memcpy(&kawai, _opcode.opcodeKAWAI._data->constData(), std::min(sizeof(OpcodeKAWAI##op), size_t(_opcode.opcodeKAWAI._data->size()))); \
		} \
		return toStringKAWAI##op(scriptsAndTexts, kawai); \
	}

QString Opcode::toString(const Section1File *scriptsAndTexts) const
{
#define op_fun(name) \
	CaseToString(name);
#define op_sep

	switch (id()) {
	OPCODE_GENERATE_LIST
	}

#undef op_fun
#undef op_sep

	return QString();
}

QString Opcode::toStringRET(const Section1File *scriptsAndTexts, const OpcodeRET &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Return");
}

QString Opcode::toStringREQ(const Section1File *scriptsAndTexts, const OpcodeREQ &opcode) const
{
	return Opcode::tr("Execute script #%3 in extern group %1 (priority %2/6) - Only if the script is not already running")
	        .arg(_groupScript(opcode.groupID, scriptsAndTexts))
	        .arg(PRIORITY(opcode.scriptIDAndPriority))
	        .arg(SCRIPT_ID(opcode.scriptIDAndPriority));
}

QString Opcode::toStringREQSW(const Section1File *scriptsAndTexts, const OpcodeREQSW &opcode) const
{
	return Opcode::tr("Execute script #%3 in extern group %1 (priority %2/6)")
	        .arg(_groupScript(opcode.groupID, scriptsAndTexts))
	        .arg(PRIORITY(opcode.scriptIDAndPriority))
	        .arg(SCRIPT_ID(opcode.scriptIDAndPriority));
}

QString Opcode::toStringREQEW(const Section1File *scriptsAndTexts, const OpcodeREQEW &opcode) const
{
	return Opcode::tr("Execute script #%3 in group %1 (priority %2/6) - Waiting for end of execution to continue")
	        .arg(_groupScript(opcode.groupID, scriptsAndTexts))
	        .arg(PRIORITY(opcode.scriptIDAndPriority))
	        .arg(SCRIPT_ID(opcode.scriptIDAndPriority));
}

QString Opcode::toStringPREQ(const Section1File *scriptsAndTexts, const OpcodePREQ &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Execute script #%3 in extern group associated with the character #%1 in the current party (priority %2/6) - Only if the script is not already running")
	        .arg(opcode.partyID)
	        .arg(PRIORITY(opcode.scriptIDAndPriority))
	        .arg(SCRIPT_ID(opcode.scriptIDAndPriority));
}

QString Opcode::toStringPRQSW(const Section1File *scriptsAndTexts, const OpcodePRQSW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Execute script #%3 in extern group associated with the character #%1 in the current party (priority %2/6)")
	        .arg(opcode.partyID)
	        .arg(PRIORITY(opcode.scriptIDAndPriority))
	        .arg(SCRIPT_ID(opcode.scriptIDAndPriority));
}

QString Opcode::toStringPRQEW(const Section1File *scriptsAndTexts, const OpcodePRQEW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Execute script #%3 in group associated with the character #%1 in the current party (priority %2/6) - Waiting for end of execution to continue")
	        .arg(opcode.partyID)
	        .arg(PRIORITY(opcode.scriptIDAndPriority))
	        .arg(SCRIPT_ID(opcode.scriptIDAndPriority));
}

QString Opcode::toStringRETTO(const Section1File *scriptsAndTexts, const OpcodeRETTO &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Return and execute script #%2 from the current entity (Priority %1/6)")
	        .arg(PRIORITY(opcode.scriptIDAndPriority))
	        .arg(SCRIPT_ID(opcode.scriptIDAndPriority));
}

QString Opcode::toStringJOIN(const Section1File *scriptsAndTexts, const OpcodeJOIN &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Join party field (speed=%1)")
	        .arg(opcode.speed);
}

QString Opcode::toStringSPLIT(const Section1File *scriptsAndTexts, const OpcodeSPLIT &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Split party field (member 1: X=%1, Y=%2, dir=%3 ; member 2 : X=%4, Y=%5, dir=%6) (speed %7)")
	        .arg(
	            _var(opcode.targetX1, B1(opcode.banks[0])),
	            _var(opcode.targetY1, B2(opcode.banks[0])),
	            _var(opcode.direction1, B1(opcode.banks[1])),
	            _var(opcode.targetX2, B2(opcode.banks[1])),
	            _var(opcode.targetY2, B1(opcode.banks[2])),
	            _var(opcode.direction2, B2(opcode.banks[2]))
	        )
	        .arg(opcode.speed);
}

QString Opcode::toStringSPTYE(const Section1File *scriptsAndTexts, const OpcodeSPTYE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set party from memory: %1 | %2 | %3")
	        .arg(
	            _var(opcode.charID1, B1(opcode.banks[0])),
	            _var(opcode.charID2, B2(opcode.banks[0])),
	            _var(opcode.charID3, B1(opcode.banks[1]))
	        );
}

QString Opcode::toStringGTPYE(const Section1File *scriptsAndTexts, const OpcodeGTPYE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Get party to memory: %1 | %2 | %3")
	        .arg(
	            _bank(opcode.varCharID1, B1(opcode.banks[0])),
	            _bank(opcode.varCharID2, B2(opcode.banks[0])),
	            _bank(opcode.varCharID3, B1(opcode.banks[1]))
	        );
}

QString Opcode::toStringUnused0C(const Section1File *scriptsAndTexts, const OpcodeUnused0C &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Unused opcode 0x%1").arg(opcode.id, 2, 16, QChar('0'));
}

QString Opcode::toStringUnused0D(const Section1File *scriptsAndTexts, const OpcodeUnused0D &opcode) const
{
	Q_UNUSED(opcode)
	return toStringUnused0C(scriptsAndTexts, _opcode.opcodeUnused0C);
}

QString Opcode::toStringDSKCG(const Section1File *scriptsAndTexts, const OpcodeDSKCG &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Ask for disc %1")
	        .arg(opcode.diskID);
}

QString Opcode::toStringSPECIALARROW(const Section1File *scriptsAndTexts, const OpcodeSPECIALARROW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 arrow")
	        .arg(opcode.disabled == 0 ? Opcode::tr("Display") : Opcode::tr("Hide"));
}

QString Opcode::toStringSPECIALPNAME(const Section1File *scriptsAndTexts, const OpcodeSPECIALPNAME &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("PNAME - Disable right menu (%1, bank=%2, size=%3)")
	        .arg(_var(opcode.varOrValue, B1(opcode.banks)))
	        .arg(B2(opcode.banks))
	        .arg(opcode.size);
}

QString Opcode::toStringSPECIALGMSPD(const Section1File *scriptsAndTexts, const OpcodeSPECIALGMSPD &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = Field message speed")
	        .arg(_var(opcode.varSpeed, B2(opcode.banks)));
}

QString Opcode::toStringSPECIALSMSPD(const Section1File *scriptsAndTexts, const OpcodeSPECIALSMSPD &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set field message speed: %1")
	        .arg(_var(opcode.speed, B2(opcode.banks)));
}

QString Opcode::toStringSPECIALFLMAT(const Section1File *scriptsAndTexts, const OpcodeSPECIALFLMAT &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Fill materia menu with all materias in full quantity");
}

QString Opcode::toStringSPECIALFLITM(const Section1File *scriptsAndTexts, const OpcodeSPECIALFLITM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Fills all available item entries in full quantity");
}


QString Opcode::toStringSPECIALBTLCK(const Section1File *scriptsAndTexts, const OpcodeSPECIALBTLCK &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 battles")
	        .arg(opcode.lock == 0 ? Opcode::tr("Activate") : Opcode::tr("Deactivate"));
}

QString Opcode::toStringSPECIALMVLCK(const Section1File *scriptsAndTexts, const OpcodeSPECIALMVLCK &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 movies")
	        .arg(opcode.lock == 0 ? Opcode::tr("Activate") : Opcode::tr("Deactivate"));
}

QString Opcode::toStringSPECIALSPCNM(const Section1File *scriptsAndTexts, const OpcodeSPECIALSPCNM &opcode) const
{
	return Opcode::tr("Change name of %1 by text %2")
	        .arg(
	            character(opcode.charID),
	            _text(opcode.textID, scriptsAndTexts)
	        );
}

QString Opcode::toStringSPECIALRSGLB(const Section1File *scriptsAndTexts, const OpcodeSPECIALRSGLB &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Set game time to 0, unlock \"PHS\" and Save menu. New party: Cloud | (empty) | (empty)");
}

QString Opcode::toStringSPECIALCLITM(const Section1File *scriptsAndTexts, const OpcodeSPECIALCLITM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Remove all items");
}

QString Opcode::toStringSPECIAL(const Section1File *scriptsAndTexts, const OpcodeSPECIAL &opcode) const
{
	switch (OpcodeSpecialKey(opcode.subKey)) {
	CaseToStringSpecial(ARROW);
	CaseToStringSpecial(PNAME);
	CaseToStringSpecial(GMSPD);
	CaseToStringSpecial(SMSPD);
	CaseToStringSpecial(FLMAT);
	CaseToStringSpecial(FLITM);
	CaseToStringSpecial(BTLCK);
	CaseToStringSpecial(MVLCK);
	CaseToStringSpecial(SPCNM);
	CaseToStringSpecial(RSGLB);
	CaseToStringSpecial(CLITM);
	}

	return Opcode::tr("SPECIAL - Unknown %1")
	        .arg(opcode.subKey, 2, 16, QLatin1Char('0'));
}

QString Opcode::toStringLABEL(const Section1File *scriptsAndTexts, const OpcodeLABEL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Label %1")
	        .arg(opcode._label);
}

QString Opcode::toStringJMPF(const Section1File *scriptsAndTexts, const OpcodeJMPF &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return opcode.getBadJump() ? Opcode::tr("Forward %n byte(s)", "With plural", opcode.jump)
	                       : Opcode::tr("Goto label %1")
	                         .arg(opcode._label);
}

QString Opcode::toStringJMPFL(const Section1File *scriptsAndTexts, const OpcodeJMPFL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return opcode.getBadJump() ? Opcode::tr("Forward %n byte(s)", "With plural", opcode.jump)
	                       : Opcode::tr("Goto label %1")
	                         .arg(opcode._label);
}

QString Opcode::toStringJMPB(const Section1File *scriptsAndTexts, const OpcodeJMPB &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return opcode.getBadJump() ? Opcode::tr("Back %n byte(s)", "With plural", -opcode.jump)
	                       : Opcode::tr("Goto label %1")
	                         .arg(opcode._label);
}

QString Opcode::toStringJMPBL(const Section1File *scriptsAndTexts, const OpcodeJMPBL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return opcode.getBadJump() ? Opcode::tr("Back %n byte(s)", "With plural", -opcode.jump)
	                       : Opcode::tr("Goto label %1")
	                         .arg(opcode._label);
}

QString Opcode::toStringIFUB(const Section1File *scriptsAndTexts, const OpcodeIFUB &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If %1 %3 %2 (%4)")
	        .arg(
	            _var(opcode.value1, B1(opcode.banks)),
	            _var(opcode.value2, B2(opcode.banks)),
	            _operateur(opcode.oper),
	            opcode.getBadJump()
	            ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	            : Opcode::tr("else goto label %1").arg(opcode._label)
	        );
}

QString Opcode::toStringIFUBL(const Section1File *scriptsAndTexts, const OpcodeIFUBL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If %1 %3 %2 (%4)")
	        .arg(
	            _var(opcode.value1, B1(opcode.banks)),
	            _var(opcode.value2, B2(opcode.banks)),
	            _operateur(opcode.oper),
	            opcode.getBadJump()
	             ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	             : Opcode::tr("else goto label %1").arg(opcode._label)
	        );
}

QString Opcode::toStringIFSW(const Section1File *scriptsAndTexts, const OpcodeIFSW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If %1 %3 %2 (%4)")
	        .arg(
	            _var(opcode.value1, B1(opcode.banks)),
	            _var(opcode.value2, B2(opcode.banks)),
	            _operateur(opcode.oper),
	            opcode.getBadJump()
	            ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	            : Opcode::tr("else goto label %1").arg(opcode._label)
	        );
}

QString Opcode::toStringIFSWL(const Section1File *scriptsAndTexts, const OpcodeIFSWL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If %1 %3 %2 (%4)")
	        .arg(
	            _var(opcode.value1, B1(opcode.banks)),
	            _var(opcode.value2, B2(opcode.banks)),
	            _operateur(opcode.oper),
	            opcode.getBadJump()
	            ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	            : Opcode::tr("else goto label %1").arg(opcode._label)
	        );
}

QString Opcode::toStringIFUW(const Section1File *scriptsAndTexts, const OpcodeIFUW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If %1 %3 %2 (%4)")
	        .arg(
	            _var(opcode.value1, B1(opcode.banks)),
	            _var(opcode.value2, B2(opcode.banks)),
	            _operateur(opcode.oper),
	            opcode.getBadJump()
	             ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	             : Opcode::tr("else goto label %1").arg(opcode._label)
	        );
}

QString Opcode::toStringIFUWL(const Section1File *scriptsAndTexts, const OpcodeIFUWL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If %1 %3 %2 (%4)")
	        .arg(
	            _var(opcode.value1, B1(opcode.banks)),
	            _var(opcode.value2, B2(opcode.banks)),
	            _operateur(opcode.oper),
	            opcode.getBadJump()
	            ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	            : Opcode::tr("else goto label %1").arg(opcode._label)
	        );
}

QString Opcode::toStringUnused1A(const Section1File *scriptsAndTexts, const OpcodeUnused1A &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	QStringList flags;
	
	switch (opcode.flag & 0x7) {
	case 0:
		flags.append(Opcode::tr("8 bit"));
		break;
	case 1:
		flags.append(Opcode::tr("16 bit"));
		break;
	case 2:
		flags.append(Opcode::tr("24 bit"));
		break;
	case 3:
		flags.append(Opcode::tr("32 bit"));
		break;
	default:
		break;
	}
	
	if (opcode.flag & 0x10) {
		flags.append(Opcode::tr("From is a pointer"));
	}
	
	if (opcode.flag & 0x20) {
		flags.append(Opcode::tr("To is a pointer"));
	}
	
	return Opcode::tr("Write/Read entire savemap (from=%1, to=%2, absValue=%3, flags={%4})")
	        .arg(opcode.from, 0, 16)
	        .arg(opcode.to, 0, 16)
	        .arg(opcode.absValue)
	        .arg(flags.join(", "));
}

QString Opcode::toStringUnused1B(const Section1File *scriptsAndTexts, const OpcodeUnused1B &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If Red XIII is named Nanaki (%2)")
	        .arg(opcode.getBadJump()
	             ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	             : Opcode::tr("else goto label %1").arg(opcode._label));
}

QString Opcode::toStringUnused1C(const Section1File *scriptsAndTexts, const OpcodeUnused1C &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Write bytes to address 0x%1 (length=%2)")
	        .arg(opcode.address, 0, 16)
	        .arg(opcode.subSize);
}

QString Opcode::toStringUnused1D(const Section1File *scriptsAndTexts, const OpcodeUnused1D &opcode) const
{
	Q_UNUSED(opcode)
	return toStringUnused0C(scriptsAndTexts, _opcode.opcodeUnused0C);
}

QString Opcode::toStringUnused1E(const Section1File *scriptsAndTexts, const OpcodeUnused1E &opcode) const
{
	Q_UNUSED(opcode)
	return toStringUnused0C(scriptsAndTexts, _opcode.opcodeUnused0C);
}

QString Opcode::toStringUnused1F(const Section1File *scriptsAndTexts, const OpcodeUnused1F &opcode) const
{
	Q_UNUSED(opcode)
	return toStringUnused0C(scriptsAndTexts, _opcode.opcodeUnused0C);
}

QString Opcode::toStringMINIGAME(const Section1File *scriptsAndTexts, const OpcodeMINIGAME &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	QString miniGame;

	switch (opcode.minigameID) {
	case 0x00:
		miniGame = Opcode::tr("Bike (parameter %1)")
		        .arg(opcode.minigameParam);
		break;
	case 0x01:
		miniGame = Opcode::tr("Chocobo Races (parameter %1)")
		        .arg(opcode.minigameParam);
		break;
	case 0x02:
		miniGame = Opcode::tr("Snowboard -normal mode- (parameter %1)")
		        .arg(opcode.minigameParam);
		break;
	case 0x03:
		miniGame = Opcode::tr("Fort Condor (parameter %1)")
		        .arg(opcode.minigameParam);
		break;
	case 0x04:
		miniGame = Opcode::tr("Submarine (parameter %1)")
		        .arg(opcode.minigameParam);
		break;
	case 0x05:
		miniGame = Opcode::tr("Speed Square (parameter %1)")
		        .arg(opcode.minigameParam);
		break;
	case 0x06:
		miniGame = Opcode::tr("Snowboard -Gold Saucer mode- (parameter %1)")
		        .arg(opcode.minigameParam);
		break;
	default:
		miniGame = Opcode::tr("%1? (parameter %2)")
		        .arg(opcode.minigameID)
		        .arg(opcode.minigameParam);
		break;
	}
	
	return Opcode::tr("Mini-game: %5 (After the game goto field %1 (X=%2, Y=%3, triangle ID=%4))")
	        .arg(_field(opcode.mapID, scriptsAndTexts))
	        .arg(opcode.targetX)
	        .arg(opcode.targetY)
	        .arg(opcode.targetI)
	        .arg(miniGame);
}

QString Opcode::toStringTUTOR(const Section1File *scriptsAndTexts, const OpcodeTUTOR &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Tutorial #%1")
	        .arg(opcode.tutoID);
}

QString Opcode::toStringBTMD2(const Section1File *scriptsAndTexts, const OpcodeBTMD2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	QStringList modes;

	for (quint8 i = 0; i < 32; ++i) {
		if ((opcode.battleMode >> i) & 1) {
			switch (i) {
			case 1:
				modes.append(Opcode::tr("Countdown"));
				break;
			case 2:
				modes.append(Opcode::tr("Pre-emptive attack"));
				break;
			case 3:
				modes.append(Opcode::tr("The party cannot escape the battle"));
				break;
			case 5:
				modes.append(Opcode::tr("Do not play the battle victory music"));
				break;
			case 6:
				modes.append(Opcode::tr("Activates the battle arena"));
				break;
			case 7:
				modes.append(Opcode::tr("Do not show battle rewards"));
				break;
			case 8:
				modes.append(Opcode::tr("The party members do not perform their victory celebrations at the end of battle"));
				break;
			case 16:
				modes.append(Opcode::tr("Disable game over"));
				break;
			default:
				modes.append(QString("%1?").arg(i));
				break;
			}
		}
	}

	return Opcode::tr("Battle mode: %1")
	        .arg(modes.isEmpty() ? Opcode::tr("None") : modes.join(", "));
}

QString Opcode::toStringBTRLD(const Section1File *scriptsAndTexts, const OpcodeBTRLD &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Stores the result of the last battle in %1")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringWAIT(const Section1File *scriptsAndTexts, const OpcodeWAIT &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Wait %1 frame")
	        .arg(opcode.frameCount);
}

QString Opcode::toStringNFADE(const Section1File *scriptsAndTexts, const OpcodeNFADE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Fades the screen to the colour RGB(%2, %3, %4) (speed=%5, type=%1)")
	        .arg(opcode.type)
	        .arg(
	            _var(opcode.r, B1(opcode.banks[0])),
	            _var(opcode.g, B2(opcode.banks[0])),
	            _var(opcode.b, B1(opcode.banks[1])),
	            _var(opcode.speed, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringBLINK(const Section1File *scriptsAndTexts, const OpcodeBLINK &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 Field Model blinking")
	        .arg(opcode.closed == 0 ? Opcode::tr("Enable") : Opcode::tr("Disable"));
}

QString Opcode::toStringBGMOVIE(const Section1File *scriptsAndTexts, const OpcodeBGMOVIE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("BGMOVIE : %1")
	        .arg(opcode.disabled == 0 ? Opcode::tr("ON") : Opcode::tr("OFF"));
}

QString Opcode::toStringKAWAIEYETX(const Section1File *scriptsAndTexts, const OpcodeKAWAIEYETX &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Field Model graphic filter - Change the state of the eye/mouth texture (eye 1=%1, eye 2=%2, mouth=%3, 3D object ID=%4)")
	        .arg(opcode.eyeID1)
	        .arg(opcode.eyeID2)
	        .arg(opcode.mouthID)
	        .arg(opcode.objectID);
}

QString Opcode::toStringKAWAITRNSP(const Section1File *scriptsAndTexts, const OpcodeKAWAITRNSP &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Field Model graphic filter - %1 blending")
	        .arg(opcode.enableTransparency == 0 ? Opcode::tr("Deactivate") : Opcode::tr("Activate"));
}

QString Opcode::toStringKAWAIAMBNT(const Section1File *scriptsAndTexts, const OpcodeKAWAIAMBNT &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Field Model graphic filter - Change the ambient color of the model: RGB(%1, %2, %3) RGB(%4, %5, %6) (flags=%7)")
	        .arg(opcode.r1)
	        .arg(opcode.g1)
	        .arg(opcode.b1)
	        .arg(opcode.r2)
	        .arg(opcode.g2)
	        .arg(opcode.b2)
	        .arg(opcode.flags);
}

QString Opcode::toStringKAWAIUNKNOWN4(const Section1File *scriptsAndTexts, const OpcodeKAWAIUNKNOWN4 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Field Model graphic filter - UNKNOWN4");
}

QString Opcode::toStringKAWAILIGHT(const Section1File *scriptsAndTexts, const OpcodeKAWAILIGHT &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Field Model graphic filter - LIGHT");
}

QString Opcode::toStringKAWAIUNKNOWN7(const Section1File *scriptsAndTexts, const OpcodeKAWAIUNKNOWN7 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Field Model graphic filter - UNKNOWN7");
}

QString Opcode::toStringKAWAIUNKNOWN8(const Section1File *scriptsAndTexts, const OpcodeKAWAIUNKNOWN8 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Field Model graphic filter - UNKNOWN8");
}

QString Opcode::toStringKAWAIUNKNOWN9(const Section1File *scriptsAndTexts, const OpcodeKAWAIUNKNOWN9 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Field Model graphic filter - UNKNOWN9");
}

QString Opcode::toStringKAWAISBOBJ(const Section1File *scriptsAndTexts, const OpcodeKAWAISBOBJ &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Field Model graphic filter - SBOBJ");
}

QString Opcode::toStringKAWAIUNKNOWNB(const Section1File *scriptsAndTexts, const OpcodeKAWAIUNKNOWNB &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Field Model graphic filter - UNKNOWNB");
}

QString Opcode::toStringKAWAIUNKNOWNC(const Section1File *scriptsAndTexts, const OpcodeKAWAIUNKNOWNC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Field Model graphic filter - UNKNOWNC");
}

QString Opcode::toStringKAWAISHINE(const Section1File *scriptsAndTexts, const OpcodeKAWAISHINE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Field Model graphic filter - SHINE");
}

QString Opcode::toStringKAWAIRESET(const Section1File *scriptsAndTexts, const OpcodeKAWAIRESET &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Field Model graphic filter - RESET");
}

QString Opcode::toStringKAWAI(const Section1File *scriptsAndTexts, const OpcodeKAWAI &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	switch (OpcodeKawaiKey(opcode.subKey)) {
	CaseToStringKawai(EYETX)
	CaseToStringKawai(TRNSP)
	CaseToStringKawai(AMBNT)
	CaseToStringKawai(UNKNOWN4)
	CaseToStringKawai(LIGHT)
	CaseToStringKawai(UNKNOWN7)
	CaseToStringKawai(UNKNOWN8)
	CaseToStringKawai(UNKNOWN9)
	CaseToStringKawai(SBOBJ)
	CaseToStringKawai(UNKNOWNB)
	CaseToStringKawai(UNKNOWNC)
	CaseToStringKawai(SHINE)
	CaseToStringKawai(RESET)
	}

	return Opcode::tr("Field Model graphic filter - Unknown %1")
	        .arg(opcode.subKey, 2, 16, QLatin1Char('0'));
}

QString Opcode::toStringKAWIW(const Section1File *scriptsAndTexts, const OpcodeKAWIW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Wait for Field Model graphic filter");
}

QString Opcode::toStringPMOVA(const Section1File *scriptsAndTexts, const OpcodePMOVA &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Move Field Model to Party Member #%1")
	        .arg(opcode.partyID);
}

QString Opcode::toStringSLIP(const Section1File *scriptsAndTexts, const OpcodeSLIP &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("SLIP : %1")
	        .arg(opcode.disabled == 0 ? Opcode::tr("ON") : Opcode::tr("OFF"));
}

QString Opcode::toStringBGPDH(const Section1File *scriptsAndTexts, const OpcodeBGPDH &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set Z-deph for the background layer #%1 (Z=%2)")
	        .arg(opcode.layerID)
	        .arg(_var(opcode.targetZ, B2(opcode.banks)));
}

QString Opcode::toStringBGSCR(const Section1File *scriptsAndTexts, const OpcodeBGSCR &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Animate the background layer #%1 (Horizontally=%2, Vertically=%3)")
	        .arg(opcode.layerID)
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks)),
	            _var(opcode.targetY, B2(opcode.banks))
	        );
}

QString Opcode::toStringWCLS(const Section1File *scriptsAndTexts, const OpcodeWCLS &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Close the window #%1")
	        .arg(opcode.windowID);
}

QString Opcode::toStringWSIZW(const Section1File *scriptsAndTexts, const OpcodeWSIZW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Resizes/Repositions the window #%1 (X=%2, Y=%3, width=%4, height=%5)")
	        .arg(opcode.windowID)
	        .arg(opcode.targetX)
	        .arg(opcode.targetY)
	        .arg(opcode.width)
	        .arg(opcode.height);
}

QString Opcode::toStringIFKEY(const Section1File *scriptsAndTexts, const OpcodeIFKEY &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If key %1 pressed (%2)")
	        .arg(
	            _keys(opcode.keys),
	            opcode.getBadJump()
	            ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	            : Opcode::tr("else goto label %1")
	              .arg(opcode._label)
	        );
}

QString Opcode::toStringIFKEYON(const Section1File *scriptsAndTexts, const OpcodeIFKEYON &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If key %1 pressed once (%2)")
	        .arg(
	            _keys(opcode.keys),
	            opcode.getBadJump()
	            ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	            : Opcode::tr("else goto label %1")
	              .arg(opcode._label)
	        );
}

QString Opcode::toStringIFKEYOFF(const Section1File *scriptsAndTexts, const OpcodeIFKEYOFF &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If key %1 released once (%2)")
	        .arg(
	            _keys(opcode.keys),
	            opcode.getBadJump()
	            ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	            : Opcode::tr("else goto label %1").arg(opcode._label)
	        );
}

QString Opcode::toStringUC(const Section1File *scriptsAndTexts, const OpcodeUC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 the movability of the playable character")
	        .arg(opcode.disabled == 0 ? Opcode::tr("Activate") : Opcode::tr("Deactivate"));
}

QString Opcode::toStringPDIRA(const Section1File *scriptsAndTexts, const OpcodePDIRA &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Instantly turns the field model to face the party member #%1")
	        .arg(opcode.partyID);
}

QString Opcode::toStringPTURA(const Section1File *scriptsAndTexts, const OpcodePTURA &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Turns the field model to face the party member #%1 (Speed=%2, Rotation=%3)")
	        .arg(opcode.partyID)
	        .arg(opcode.speed)
	        .arg(_sensRotation(opcode.directionRotation));
}

QString Opcode::toStringWSPCL(const Section1File *scriptsAndTexts, const OpcodeWSPCL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	QString windowNum;
	switch (opcode.displayType) {
	case 0x00:
		windowNum = Opcode::tr("(none)");
		break;
	case 0x01:
		windowNum = Opcode::tr("Clock (00:00)");
		break;
	case 0x02:
		windowNum = Opcode::tr("Numeric (00)");
		break;
	default:
		windowNum = QString("%1?")
		        .arg(opcode.displayType);
		break;
	}
	
	return Opcode::tr("%2 in the window #%1 (left=%3, top=%4)")
	        .arg(opcode.windowID)
	        .arg(windowNum)
	        .arg(opcode.marginLeft)
	        .arg(opcode.marginTop);
}

QString Opcode::toStringWNUMB(const Section1File *scriptsAndTexts, const OpcodeWNUMB &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Sets %2 in window #%1 (show %3 digits)")
	        .arg(opcode.windowID)
	        .arg(_var(opcode.value, B1(opcode.banks), B2(opcode.banks)))
	        .arg(opcode.digitCount);
}

QString Opcode::toStringSTTIM(const Section1File *scriptsAndTexts, const OpcodeSTTIM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set Timer (H=%1, M=%2, S=%3)")
	        .arg(
	            _var(opcode.h, B1(opcode.banks[0])),
	            _var(opcode.m, B2(opcode.banks[0])),
	            _var(opcode.s, B1(opcode.banks[1]))
	        );
}

QString Opcode::toStringGOLDu(const Section1File *scriptsAndTexts, const OpcodeGOLDu &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Add %1 gil to the party ")
	        .arg(_var(opcode.value, B1(opcode.banks), B2(opcode.banks)));
}

QString Opcode::toStringGOLDd(const Section1File *scriptsAndTexts, const OpcodeGOLDd &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Remove %1 gils from the party")
	        .arg(_var(opcode.value, B1(opcode.banks), B2(opcode.banks)));
}

QString Opcode::toStringCHGLD(const Section1File *scriptsAndTexts, const OpcodeCHGLD &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Copies the amount of gil in %1 and %2")
	        .arg(
	            _bank(opcode.var1, B1(opcode.banks)),
	            _bank(opcode.var2, B2(opcode.banks))
	        );
}

QString Opcode::toStringHMPMAX1(const Section1File *scriptsAndTexts, const OpcodeHMPMAX1 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Restores full HP and MP of every party member");
}

QString Opcode::toStringHMPMAX2(const Section1File *scriptsAndTexts, const OpcodeHMPMAX2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Restores full HP and MP of every party member");
}

QString Opcode::toStringMHMMX(const Section1File *scriptsAndTexts, const OpcodeMHMMX &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Restores full HP and MP of every available character and removing status effects");
}

QString Opcode::toStringHMPMAX3(const Section1File *scriptsAndTexts, const OpcodeHMPMAX3 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Restores full HP and MP of every party member");
}

QString Opcode::toStringMESSAGE(const Section1File *scriptsAndTexts, const OpcodeMESSAGE &opcode) const
{
	return Opcode::tr("Displays the dialog %2 in the window #%1")
	        .arg(opcode.windowID)
	        .arg(_text(opcode.textID, scriptsAndTexts));
}

QString Opcode::toStringMPARA(const Section1File *scriptsAndTexts, const OpcodeMPARA &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set %3 to the variable #%2 in the window #%1")
	        .arg(opcode.windowID)
	        .arg(opcode.windowVarID)
	        .arg(_var(opcode.value, B2(opcode.banks)));
}

QString Opcode::toStringMPRA2(const Section1File *scriptsAndTexts, const OpcodeMPRA2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set %3 to the variable #%2 in the window #%1")
	        .arg(opcode.windowID)
	        .arg(opcode.windowVarID)
	        .arg(_var(opcode.value, B2(opcode.banks)));
}

QString Opcode::toStringMPNAM(const Section1File *scriptsAndTexts, const OpcodeMPNAM &opcode) const
{
	return Opcode::tr("Display %1 in the main menu")
	        .arg(_text(opcode.textID, scriptsAndTexts));
}

QString Opcode::toStringUnused44(const Section1File *scriptsAndTexts, const OpcodeUnused44 &opcode) const
{
	Q_UNUSED(opcode)
	return toStringUnused0C(scriptsAndTexts, _opcode.opcodeUnused0C);
}

QString Opcode::toStringMPu(const Section1File *scriptsAndTexts, const OpcodeMPu &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Add %2 MP to party member #%1")
	        .arg(opcode.partyID)
	        .arg(_var(opcode.value, B2(opcode.banks)));
}

QString Opcode::toStringUnused46(const Section1File *scriptsAndTexts, const OpcodeUnused46 &opcode) const
{
	Q_UNUSED(opcode)
	return toStringUnused0C(scriptsAndTexts, _opcode.opcodeUnused0C);
}

QString Opcode::toStringMPd(const Section1File *scriptsAndTexts, const OpcodeMPd &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Remove %2 MP to party member #%1")
	        .arg(opcode.partyID)
	        .arg(_var(opcode.value, B2(opcode.banks)));
}

QString Opcode::toStringASK(const Section1File *scriptsAndTexts, const OpcodeASK &opcode) const
{
	return Opcode::tr("Ask Question %2 in the window #%1 (and put selected answer in %5) first line=%3, last line=%4")
	        .arg(opcode.windowID)
	        .arg(_text(opcode.textID, scriptsAndTexts))
	        .arg(opcode.firstLine)
	        .arg(opcode.lastLine)
	        .arg(_bank(opcode.varAnswer, B2(opcode.banks)));
}

QString Opcode::toStringMENU(const Section1File *scriptsAndTexts, const OpcodeMENU &opcode) const
{
	Q_UNUSED(scriptsAndTexts)

	QString menu, subMenu;
	QString param = _var(opcode.param, B2(opcode.banks));
	switch (opcode.menuID) {
	case 1:
		menu = Opcode::tr("Exit program (parameter %1)")
		        .arg(param);
		break;
	case 2:
		menu = Opcode::tr("Encount Error (parameter %1)")
		        .arg(param);
		break;
	case 5:
		menu = Opcode::tr("ff7 credits (parameter %1)")
		        .arg(param);
		break;
	case 6:
		menu = Opcode::tr("Change name of %1")
		        .arg(B2(opcode.banks)
		             ? Opcode::tr("%1 (char ID)").arg(param)
		             : character(opcode.param)); // Parameter: char ID
		break;
	case 7:
		menu = Opcode::tr("Change party (parameter %1)")
		        .arg(param);
		break;
	case 8:
		menu = Opcode::tr("Shop No%1")
		        .arg(param); // Parameter: shop ID
		break;
	case 9:
		menu = Opcode::tr("main (parameter %1)")
		        .arg(param);
		break;
	case 12:
		menu = Opcode::tr("Bike (parameter %1)")
		        .arg(param);
		break;
	case 14:
		menu = Opcode::tr("Save (parameter %1)")
		        .arg(param);
		break;
	case 15:
		menu = Opcode::tr("Remove all materias (parameter %1)")
		        .arg(param);
		break;
	case 16:
		menu = Opcode::tr("Restore all materias (parameter %1)")
		        .arg(param);
		break;
	case 17:
		menu = Opcode::tr("Remove %1's Materia")
		        .arg(B2(opcode.banks)
		             ? Opcode::tr("%1 (char ID)").arg(param)
		             : character(opcode.param)); // Parameter: char ID
		break;
	case 18:
		menu = Opcode::tr("Clear Cloud's materias (parameter %1)")
		        .arg(param);
		break;
	case 19:
		menu = Opcode::tr("Restore Cloud's materias (parameter %1)")
		        .arg(param);
		break;
	case 20:
		menu = Opcode::tr("? (parameter %1)")
		        .arg(param); // TODO
		break;
	case 21:
		menu = Opcode::tr("HP to 1 (parameter %1)")
		        .arg(param);
		break;
	case 22:
		switch (opcode.param) {
		case 0:
			subMenu = Opcode::tr("all magic materias are present and mastered");
			break;
		case 1:
			subMenu = Opcode::tr("all summon materias are present and mastered");
			break;
		case 2:
			subMenu = Opcode::tr("necessary command materias are present and mastered");
			break;
		case 3:
			subMenu = Opcode::tr("Bahamut and Neo Bahamut materias are present");
			break;
		default:
			subMenu = Opcode::tr("22? (parameter %1)")
			        .arg(param);
			break;
		}
		menu = Opcode::tr("Check if %1 and store the result in var[15][111]")
		        .arg(subMenu);
		break;
	case 23:
		switch (opcode.param) {
		case 0:
			menu = Opcode::tr("Remove mastered magic materias and add Master Magic");
			break;
		case 1:
			menu = Opcode::tr("Remove mastered summon materias and add Master Summon");
			break;
		case 2:
			menu = Opcode::tr("Remove mastered meaning command materias and add Master Command");
			break;
		case 3:
			menu = Opcode::tr("Add Bahamut Zero to the inventory");
			break;
		default:
			menu = Opcode::tr("23? (parameter %1)")
			        .arg(param);
			break;
		}
		break;
	case 24:
		menu = Opcode::tr("? (parameter %1)")
		        .arg(param);
		break;
	case 25:
		menu = Opcode::tr("? (parameter %1)")
		        .arg(param);
		break;
	default:
		menu = Opcode::tr("%1? (parameter %2)")
		        .arg(opcode.menuID)
		        .arg(param);
		break;
	}

	return Opcode::tr("Show menu %1")
	        .arg(menu);
}

QString Opcode::toStringMENU2(const Section1File *scriptsAndTexts, const OpcodeMENU2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 access to the main menu")
	        .arg(opcode.disabled == 0 ? Opcode::tr("Enables") : Opcode::tr("Disables"));
}

QString Opcode::toStringBTLTB(const Section1File *scriptsAndTexts, const OpcodeBTLTB &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set battle table: %1")
	        .arg(opcode.battleTableID);
}

QString Opcode::toStringUnused4C(const Section1File *scriptsAndTexts, const OpcodeUnused4C &opcode) const
{
	Q_UNUSED(opcode)
	return toStringUnused0C(scriptsAndTexts, _opcode.opcodeUnused0C);
}

QString Opcode::toStringHPu(const Section1File *scriptsAndTexts, const OpcodeHPu &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Add %2 HP to party member #%1")
	        .arg(opcode.partyID)
	        .arg(_var(opcode.value, B2(opcode.banks)));
}

QString Opcode::toStringUnused4E(const Section1File *scriptsAndTexts, const OpcodeUnused4E &opcode) const
{
	Q_UNUSED(opcode)
	return toStringUnused0C(scriptsAndTexts, _opcode.opcodeUnused0C);
}

QString Opcode::toStringHPd(const Section1File *scriptsAndTexts, const OpcodeHPd &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Remove %2 HP to party member #%1")
	        .arg(opcode.partyID)
	        .arg(_var(opcode.value, B2(opcode.banks)));
}

QString Opcode::toStringWINDOW(const Section1File *scriptsAndTexts, const OpcodeWINDOW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Create window #%1 (X=%2, Y=%3, Width=%4, Height=%5)")
	        .arg(opcode.windowID)
	        .arg(opcode.targetX)
	        .arg(opcode.targetY)
	        .arg(opcode.width)
	        .arg(opcode.height);
}

QString Opcode::toStringWMOVE(const Section1File *scriptsAndTexts, const OpcodeWMOVE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Move the window #%1 (Move : X=%2, Y=%3)")
	        .arg(opcode.windowID)
	        .arg(opcode.relativeX)
	        .arg(opcode.relativeY);
}

QString Opcode::toStringWMODE(const Section1File *scriptsAndTexts, const OpcodeWMODE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)

	QString typeStr;
	switch (opcode.mode) {
	case 0x00:
		typeStr = Opcode::tr("Normal");
		break;
	case 0x01:
		typeStr = Opcode::tr("No Background/Border");
		break;
	case 0x02:
		typeStr = Opcode::tr("Transparent Background");
		break;
	default:
		typeStr = QString("%1?")
		        .arg(opcode.mode);
		break;
	}
	
	return Opcode::tr("Set the window #%1 mode: %2 (%3 the closing of the window by the player)")
	        .arg(opcode.windowID)
	        .arg(
	            typeStr,
	            opcode.preventClose == 0 ? Opcode::tr("Authorize") : Opcode::tr("prevent")
	        );
}

QString Opcode::toStringWREST(const Section1File *scriptsAndTexts, const OpcodeWREST &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Reset the window #%1")
	        .arg(opcode.windowID);
}

QString Opcode::toStringWCLSE(const Section1File *scriptsAndTexts, const OpcodeWCLSE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Close the window #%1 (stronger)")
	        .arg(opcode.windowID);
}

QString Opcode::toStringWROW(const Section1File *scriptsAndTexts, const OpcodeWROW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Number of row in the window #%1 = %2")
	        .arg(opcode.windowID)
	        .arg(opcode.rowCount);
}

QString Opcode::toStringGWCOL(const Section1File *scriptsAndTexts, const OpcodeGWCOL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Get windows %1 color to %2 (R), %3 (G) and %4 (B)")
	        .arg(
	            _windowCorner(opcode.corner, B1(opcode.banks[0])),
	            _bank(opcode.varR, B2(opcode.banks[0])),
	            _bank(opcode.varG, B1(opcode.banks[1])),
	            _bank(opcode.varB, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringSWCOL(const Section1File *scriptsAndTexts, const OpcodeSWCOL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set windows %1 color: RGB(%2, %3, %4)")
	        .arg(
	            _windowCorner(opcode.corner, B1(opcode.banks[0])),
	            _var(opcode.r, B2(opcode.banks[0])),
	            _var(opcode.g, B1(opcode.banks[1])),
	            _var(opcode.b, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringSTITM(const Section1File *scriptsAndTexts, const OpcodeSTITM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Add %2 item(s) %1 to the inventory")
	        .arg(_item(opcode.itemID, B1(opcode.banks)),
	             _var(opcode.quantity, B2(opcode.banks))
	        );
}

QString Opcode::toStringDLITM(const Section1File *scriptsAndTexts, const OpcodeDLITM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Remove %2 item(s) %1 from the inventory")
	        .arg(_item(opcode.itemID, B1(opcode.banks)),
	             _var(opcode.quantity, B2(opcode.banks))
	        );
}

QString Opcode::toStringCKITM(const Section1File *scriptsAndTexts, const OpcodeCKITM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%2 = amount of item %1 in the inventory")
	        .arg(_item(opcode.itemID, B1(opcode.banks)),
	             _bank(opcode.quantity, B2(opcode.banks))
	        );
}

QString Opcode::toStringSMTRA(const Section1File *scriptsAndTexts, const OpcodeSMTRA &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Add %1 materia to the inventory (AP=%2)")
	        .arg(_materia(opcode.materiaID, B1(opcode.banks[0])),
	             _var(opcode.APCount, B2(opcode.banks[0]), B1(opcode.banks[1]), B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringDMTRA(const Section1File *scriptsAndTexts, const OpcodeDMTRA &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Remove %3 materia(s) %1 from the inventory (AP=%2)")
	        .arg(_materia(opcode.materiaID, B1(opcode.banks[0])),
	             _var(opcode.APCount, B2(opcode.banks[0]), B1(opcode.banks[1]), B2(opcode.banks[1])))
	        .arg(opcode.quantity);
}

QString Opcode::toStringCMTRA(const Section1File *scriptsAndTexts, const OpcodeCMTRA &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%3 = amount of materia %1 in the inventory (AP=%2)")
	        .arg(
	            _materia(opcode.materiaID, 0),
	            _var(opcode.APCount, B1(opcode.banks[0]), B2(opcode.banks[0]), B1(opcode.banks[1]), B2(opcode.banks[1])),
	            _bank(opcode.varQuantity, B2(opcode.banks[2]))
	        );
}

QString Opcode::toStringSHAKE(const Section1File *scriptsAndTexts, const OpcodeSHAKE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Shake (type=%1, xAmplitude=%2, xFrames=%3, yAmplitude=%4, yFrames=%5)")
	        .arg(opcode.type)
	        .arg(opcode.xAmplitude)
	        .arg(opcode.xFrames)
	        .arg(opcode.yAmplitude)
	        .arg(opcode.yFrames);
}

QString Opcode::toStringNOP(const Section1File *scriptsAndTexts, const OpcodeNOP &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Perform no operation");
}

QString Opcode::toStringMAPJUMP(const Section1File *scriptsAndTexts, const OpcodeMAPJUMP &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Jump to map %1 (X=%2, Y=%3, triangle ID=%4, direction=%5)")
	        .arg(_field(opcode.mapID, scriptsAndTexts))
	        .arg(opcode.targetX)
	        .arg(opcode.targetY)
	        .arg(opcode.targetI)
	        .arg(opcode.direction);
}

QString Opcode::toStringSCRLO(const Section1File *scriptsAndTexts, const OpcodeSCRLO &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("SCRLO (?=%1)")
	        .arg(opcode.unknown);
}

QString Opcode::toStringSCRLC(const Section1File *scriptsAndTexts, const OpcodeSCRLC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("SCRLC (?=%1)")
	        .arg(opcode.unknown);
}

QString Opcode::toStringSCRLA(const Section1File *scriptsAndTexts, const OpcodeSCRLA &opcode) const
{
	return Opcode::tr("Scroll to group %2 (speed=%1, type=%3)")
	        .arg(
	            _var(opcode.speed, B2(opcode.banks)),
	            _groupScript(opcode.groupID, scriptsAndTexts)
	        )
	        .arg(opcode.scrollType);
}

QString Opcode::toStringSCR2D(const Section1File *scriptsAndTexts, const OpcodeSCR2D &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Scroll to location (X=%1, Y=%2)")
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks)),
	            _var(opcode.targetY, B2(opcode.banks))
	        );
}

QString Opcode::toStringSCRCC(const Section1File *scriptsAndTexts, const OpcodeSCRCC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Scroll to playable character");
}

QString Opcode::toStringSCR2DC(const Section1File *scriptsAndTexts, const OpcodeSCR2DC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Scroll to location (X=%1, Y=%2, speed=%3)")
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks[0])),
	            _var(opcode.targetY, B2(opcode.banks[0])),
	            _var(opcode.speed, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringSCRLW(const Section1File *scriptsAndTexts, const OpcodeSCRLW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Wait for scroll");
}

QString Opcode::toStringSCR2DL(const Section1File *scriptsAndTexts, const OpcodeSCR2DL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Scroll to location (X=%1, Y=%2, speed=%3)")
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks[0])),
	            _var(opcode.targetY, B2(opcode.banks[0])),
	            _var(opcode.speed, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringMPDSP(const Section1File *scriptsAndTexts, const OpcodeMPDSP &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("MPDSP : %1")
	        .arg(opcode.unknown);
}

QString Opcode::toStringVWOFT(const Section1File *scriptsAndTexts, const OpcodeVWOFT &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Scroll to location (?=%1, ?=%2, enable=%3)")
	        .arg(
	            _var(opcode.unknown1, B1(opcode.banks)),
	            _var(opcode.unknown2, B2(opcode.banks))
	        )
	        .arg(opcode.enable);
}

QString Opcode::toStringFADE(const Section1File *scriptsAndTexts, const OpcodeFADE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Fades the screen to the colour RGB(%1, %2, %3) (speed=%4, type=%5, adjust=%6)")
	        .arg(
	            _var(opcode.r, B1(opcode.banks[0])),
	            _var(opcode.g, B2(opcode.banks[0])),
	            _var(opcode.b, B2(opcode.banks[1]))
	        )
	        .arg(opcode.speed)
	        .arg(opcode.fadeType)
	        .arg(opcode.adjust);
}

QString Opcode::toStringFADEW(const Section1File *scriptsAndTexts, const OpcodeFADEW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Wait for fade");
}

QString Opcode::toStringIDLCK(const Section1File *scriptsAndTexts, const OpcodeIDLCK &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%2 the triangle #%1")
	        .arg(opcode.triangleID)
	        .arg(opcode.locked == 0 ? Opcode::tr("Activate") : Opcode::tr("Deactivate"));
}

QString Opcode::toStringLSTMP(const Section1File *scriptsAndTexts, const OpcodeLSTMP &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Retrieves the field ID number of the last field in %1")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringSCRLP(const Section1File *scriptsAndTexts, const OpcodeSCRLP &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Scroll to party member #%2 (speed=%1 frames, type=%3)")
	        .arg(_var(opcode.speed, B2(opcode.banks)))
	        .arg(opcode.partyID)
	        .arg(opcode.scrollType);
}

QString Opcode::toStringBATTLE(const Section1File *scriptsAndTexts, const OpcodeBATTLE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Start battle #%1")
	        .arg(_var(opcode.battleID, B2(opcode.banks)));
}

QString Opcode::toStringBTLON(const Section1File *scriptsAndTexts, const OpcodeBTLON &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 random battle")
	        .arg(opcode.disabled == 0 ? Opcode::tr("Activate") : Opcode::tr("Deactivate"));
}

QString Opcode::toStringBTLMD(const Section1File *scriptsAndTexts, const OpcodeBTLMD &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	QStringList modes;
	for (quint8 i = 0; i < 16; ++i) {
		if ((opcode.battleMode >> i) & 1) {
			switch (i) {
			case 1:
				modes.append(Opcode::tr("Countdown"));
				break;
			case 2:
				modes.append(Opcode::tr("Pre-emptive attack"));
				break;
			case 3:
				modes.append(Opcode::tr("The party cannot escape the battle"));
				break;
			case 5:
				modes.append(Opcode::tr("Do not play the battle victory music"));
				break;
			case 6:
				modes.append(Opcode::tr("Activates the battle arena"));
				break;
			case 7:
				modes.append(Opcode::tr("Do not show battle rewards"));
				break;
			case 8:
				modes.append(Opcode::tr("Disable game over"));
				break;
			default:
				modes.append(QString("%1?").arg(i));
				break;
			}
		}
	}
	
	return Opcode::tr("Battle mode: %1")
	        .arg(modes.isEmpty() ? Opcode::tr("None") : modes.join(", "));
}

QString Opcode::toStringPGTDR(const Section1File *scriptsAndTexts, const OpcodePGTDR &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Get direction of the party member #%1 to %2")
	        .arg(opcode.partyID)
	        .arg(_bank(opcode.varDir, B2(opcode.banks)));
}

QString Opcode::toStringGETPC(const Section1File *scriptsAndTexts, const OpcodeGETPC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Get character ID of the party member #%1 to %2")
	        .arg(opcode.partyID)
	        .arg(_bank(opcode.varPC, B2(opcode.banks)));
}

QString Opcode::toStringPXYZI(const Section1File *scriptsAndTexts, const OpcodePXYZI &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Get coordinates of the party member #%1 (store : X in %2, Y in %3, Z in %4 and triangle ID in %5)")
	        .arg(opcode.partyID)
	        .arg(
	            _bank(opcode.varX, B1(opcode.banks[0])),
	            _bank(opcode.varY, B2(opcode.banks[0])),
	            _bank(opcode.varZ, B1(opcode.banks[1])),
	            _bank(opcode.varI, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringPLUSX(const Section1File *scriptsAndTexts, const OpcodePLUSX &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 + %2 (8 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringPLUS2X(const Section1File *scriptsAndTexts, const OpcodePLUS2X &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 + %2 (16 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringMINUSX(const Section1File *scriptsAndTexts, const OpcodeMINUSX &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 - %2 (8 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringMINUS2X(const Section1File *scriptsAndTexts, const OpcodeMINUS2X &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 - %2 (16 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringINCX(const Section1File *scriptsAndTexts, const OpcodeINCX &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 + 1 (8 bit)")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringINC2X(const Section1File *scriptsAndTexts, const OpcodeINC2X &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 + 1 (16 bit)")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringDECX(const Section1File *scriptsAndTexts, const OpcodeDECX &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 - 1 (8 bit)")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringDEC2X(const Section1File *scriptsAndTexts, const OpcodeDEC2X &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 - 1 (16 bit)")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringTLKON(const Section1File *scriptsAndTexts, const OpcodeTLKON &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 talk script for the current field model")
	        .arg(opcode.disabled == 0 ? Opcode::tr("Activate") : Opcode::tr("Deactivate"));
}

QString Opcode::toStringRDMSD(const Section1File *scriptsAndTexts, const OpcodeRDMSD &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Seed Random Generator : %1")
	        .arg(_var(opcode.value, B2(opcode.banks)));
}

QString Opcode::toStringSETBYTE(const Section1File *scriptsAndTexts, const OpcodeSETBYTE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %2 (8 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringSETWORD(const Section1File *scriptsAndTexts, const OpcodeSETWORD &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %2 (16 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringBITON(const Section1File *scriptsAndTexts, const OpcodeBITON &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Bit %2 ON in %1")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.position, B2(opcode.banks))
	        );
}

QString Opcode::toStringBITOFF(const Section1File *scriptsAndTexts, const OpcodeBITOFF &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Bit %2 OFF in %1")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.position, B2(opcode.banks))
	        );
}

QString Opcode::toStringBITXOR(const Section1File *scriptsAndTexts, const OpcodeBITXOR &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Toggle bit %2 in %1")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.position, B2(opcode.banks))
	        );
}

QString Opcode::toStringPLUS(const Section1File *scriptsAndTexts, const OpcodePLUS &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 + %2 (8 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringPLUS2(const Section1File *scriptsAndTexts, const OpcodePLUS2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 + %2 (16 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringMINUS(const Section1File *scriptsAndTexts, const OpcodeMINUS &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 - %2 (8 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringMINUS2(const Section1File *scriptsAndTexts, const OpcodeMINUS2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 - %2 (16 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringMUL(const Section1File *scriptsAndTexts, const OpcodeMUL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 * %2 (8 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringMUL2(const Section1File *scriptsAndTexts, const OpcodeMUL2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 * %2 (16 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringDIV(const Section1File *scriptsAndTexts, const OpcodeDIV &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 / %2 (8 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringDIV2(const Section1File *scriptsAndTexts, const OpcodeDIV2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 / %2 (16 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringMOD(const Section1File *scriptsAndTexts, const OpcodeMOD &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 mod %2 (8 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringMOD2(const Section1File *scriptsAndTexts, const OpcodeMOD2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 mod %2 (16 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringAND(const Section1File *scriptsAndTexts, const OpcodeAND &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 & %2 (8 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringAND2(const Section1File *scriptsAndTexts, const OpcodeAND2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 & %2 (16 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringOR(const Section1File *scriptsAndTexts, const OpcodeOR &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 | %2 (8 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringOR2(const Section1File *scriptsAndTexts, const OpcodeOR2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 | %2 (16 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringXOR(const Section1File *scriptsAndTexts, const OpcodeXOR &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 ^ %2 (8 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringXOR2(const Section1File *scriptsAndTexts, const OpcodeXOR2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 ^ %2 (16 bit)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringINC(const Section1File *scriptsAndTexts, const OpcodeINC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 + 1 (8 bit)")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringINC2(const Section1File *scriptsAndTexts, const OpcodeINC2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 + 1 (16 bit)")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringDEC(const Section1File *scriptsAndTexts, const OpcodeDEC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 - 1 (8 bit)")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringDEC2(const Section1File *scriptsAndTexts, const OpcodeDEC2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %1 - 1 (16 bit)")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringRANDOM(const Section1File *scriptsAndTexts, const OpcodeRANDOM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set random value to %1 (8-bit)")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringLBYTE(const Section1File *scriptsAndTexts, const OpcodeLBYTE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = %2 & 0xFF (low byte)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringHBYTE(const Section1File *scriptsAndTexts, const OpcodeHBYTE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = (%2 >> 8) & 0xFF (high byte)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks)),
	            _var(opcode.value, B2(opcode.banks))
	        );
}

QString Opcode::toStringTOBYTE(const Section1File *scriptsAndTexts, const OpcodeTOBYTE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = (%2 & 0xFF) | ((%3 & 0xFF) << 8)")
	        .arg(
	            _bank(opcode.var, B1(opcode.banks[0])),
	            _var(opcode.value1, B2(opcode.banks[0])),
	            _var(opcode.value2, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringSETX(const Section1File *scriptsAndTexts, const OpcodeSETX &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("SETX[%1][%2 + %3] = %4")
	        .arg(B1(opcode.banks[0]))
	        .arg(opcode.value)
	        .arg(
	            _var(opcode.varOrValue1, B2(opcode.banks[0])),
	            _var(opcode.varOrValue2, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringGETX(const Section1File *scriptsAndTexts, const OpcodeGETX &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 = GETX[%2][%3 + %4]")
	        .arg(_bank(opcode.var, B2(opcode.banks[1])))
	        .arg(B1(opcode.banks[0]))
	        .arg(opcode.value)
	        .arg(_var(opcode.varOrValue1, B2(opcode.banks[0])));
}

QString Opcode::toStringSEARCHX(const Section1File *scriptsAndTexts, const OpcodeSEARCHX &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Search the value %5 in the memory (bank=%1, start=%2+%3, end=%2+%4) and put the position in %6")
	        .arg(B1(opcode.banks[0]))
	        .arg(opcode.searchStart)
	        .arg(
	            _var(opcode.start, B2(opcode.banks[0])),
	            _var(opcode.end, B1(opcode.banks[1])),
	            _var(opcode.value, B2(opcode.banks[1])),
	            _var(opcode.varResult, B2(opcode.banks[2]))
	        );
}

QString Opcode::toStringPC(const Section1File *scriptsAndTexts, const OpcodePC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Field model is playable and it is %1")
	        .arg(character(opcode.charID));
}

QString Opcode::toStringCHAR_(const Section1File *scriptsAndTexts, const OpcodeCHAR_ &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("This group is a field model (ID=%1)")
	        .arg(opcode.object3DID);
}

QString Opcode::toStringDFANM(const Section1File *scriptsAndTexts, const OpcodeDFANM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play loop animation #%1 of the field model (speed=%2)")
	        .arg(opcode.animID)
	        .arg(opcode.speed);
}

QString Opcode::toStringANIME1(const Section1File *scriptsAndTexts, const OpcodeANIME1 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play animation #%1 of the field model and reset to previous state (speed=%2)")
	        .arg(opcode.animID)
	        .arg(opcode.speed);
}

QString Opcode::toStringVISI(const Section1File *scriptsAndTexts, const OpcodeVISI &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 field model")
	        .arg(opcode.show == 0 ? Opcode::tr("Hide") : Opcode::tr("Display"));
}

QString Opcode::toStringXYZI(const Section1File *scriptsAndTexts, const OpcodeXYZI &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Place field Model (X=%1, Y=%2, Z=%3, triangle ID=%4)")
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks[0])),
	            _var(opcode.targetY, B2(opcode.banks[0])),
	            _var(opcode.targetZ, B1(opcode.banks[1])),
	            _var(opcode.targetI, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringXYI(const Section1File *scriptsAndTexts, const OpcodeXYI &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Place field Model (X=%1, Y=%2, triangle ID=%4)")
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks[0])),
	            _var(opcode.targetY, B2(opcode.banks[0])),
	            _var(opcode.targetI, B1(opcode.banks[1]))
	        );
}

QString Opcode::toStringXYZ(const Section1File *scriptsAndTexts, const OpcodeXYZ &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Place field Model (X=%1, Y=%2, Z=%3)")
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks[0])),
	            _var(opcode.targetY, B2(opcode.banks[0])),
	            _var(opcode.targetZ, B1(opcode.banks[1]))
	        );
}

QString Opcode::toStringMOVE(const Section1File *scriptsAndTexts, const OpcodeMOVE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Move field Model (X=%1, Y=%2)")
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks)),
	            _var(opcode.targetY, B2(opcode.banks))
	        );
}

QString Opcode::toStringCMOVE(const Section1File *scriptsAndTexts, const OpcodeCMOVE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Place field Model without animation (X=%1, Y=%2)")
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks)),
	            _var(opcode.targetY, B2(opcode.banks))
	        );
}

QString Opcode::toStringMOVA(const Section1File *scriptsAndTexts, const OpcodeMOVA &opcode) const
{
	return Opcode::tr("Move field Model to the group %1")
	        .arg(_groupScript(opcode.groupID, scriptsAndTexts));
}

QString Opcode::toStringTURA(const Section1File *scriptsAndTexts, const OpcodeTURA &opcode) const
{
	return Opcode::tr("Rotation of the field model to group %1 (Speed=%3, Rotation=%2)")
	        .arg(
	            _groupScript(opcode.groupID, scriptsAndTexts),
	            _sensRotation(opcode.directionRotation)
	        )
	        .arg(opcode.speed);
}

QString Opcode::toStringANIMW(const Section1File *scriptsAndTexts, const OpcodeANIMW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Wait for animation");
}

QString Opcode::toStringFMOVE(const Section1File *scriptsAndTexts, const OpcodeFMOVE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Place field Model without animation (X=%1, Y=%2)")
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks)),
	            _var(opcode.targetY, B2(opcode.banks))
	        );
}

QString Opcode::toStringANIME2(const Section1File *scriptsAndTexts, const OpcodeANIME2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play animation #%1 of the field model and reset to previous state (speed=%2)")
	        .arg(opcode.animID)
	        .arg(opcode.speed);
}

QString Opcode::toStringANIMX1(const Section1File *scriptsAndTexts, const OpcodeANIMX1 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play animation #%1 of the field model (speed=%2, type=1)")
	        .arg(opcode.animID)
	        .arg(opcode.speed);
}

QString Opcode::toStringCANIM1(const Section1File *scriptsAndTexts, const OpcodeCANIM1 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play partially the animation #%1 of the field model and reset to initial state (first frame=%2, last frame=%3, speed=%4)")
	        .arg(opcode.animID)
	        .arg(opcode.firstFrame)
	        .arg(opcode.lastFrame)
	        .arg(opcode.speed);
}

QString Opcode::toStringCANMX1(const Section1File *scriptsAndTexts, const OpcodeCANMX1 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play partially the animation #%1 of the field model (first frame=%2, last frame=%3, speed=%4)")
	        .arg(opcode.animID)
	        .arg(opcode.firstFrame)
	        .arg(opcode.lastFrame)
	        .arg(opcode.speed);
}

QString Opcode::toStringMSPED(const Section1File *scriptsAndTexts, const OpcodeMSPED &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set the field model move speed: %1")
	        .arg(_var(opcode.speed, B2(opcode.banks)));
}

QString Opcode::toStringDIR(const Section1File *scriptsAndTexts, const OpcodeDIR &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set field model direction: %1")
	        .arg(_var(opcode.direction, B2(opcode.banks)));
}

QString Opcode::toStringTURNGEN(const Section1File *scriptsAndTexts, const OpcodeTURNGEN &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Rotation (direction=%1, nbRevolution=%2, speed=%3, ?=%4)")
	        .arg(_var(opcode.direction, B2(opcode.banks)))
	        .arg(opcode.turnCount)
	        .arg(opcode.speed)
	        .arg(opcode.unknown);
}

QString Opcode::toStringTURN(const Section1File *scriptsAndTexts, const OpcodeTURN &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Inversed rotation (direction=%1, nbRevolution=%2, speed=%3, ?=%4)")
	        .arg(_var(opcode.direction, B2(opcode.banks)))
	        .arg(opcode.turnCount)
	        .arg(opcode.speed)
	        .arg(opcode.unknown);
}

QString Opcode::toStringDIRA(const Section1File *scriptsAndTexts, const OpcodeDIRA &opcode) const
{
	return Opcode::tr("Direct the field model towards the group %1")
	        .arg(_groupScript(opcode.groupID, scriptsAndTexts));
}

QString Opcode::toStringGETDIR(const Section1File *scriptsAndTexts, const OpcodeGETDIR &opcode) const
{
	return Opcode::tr("Store direction of the group %1 in %2")
	        .arg(
	            _groupScript(opcode.groupID, scriptsAndTexts),
	            _bank(opcode.varDir, B2(opcode.banks))
	        );
}

QString Opcode::toStringGETAXY(const Section1File *scriptsAndTexts, const OpcodeGETAXY &opcode) const
{
	return Opcode::tr("Store position of the group %1 in %2 (X) and %3 (Y)")
	        .arg(
	            _groupScript(opcode.groupID, scriptsAndTexts),
	            _bank(opcode.varX, B1(opcode.banks)),
	            _bank(opcode.varY, B2(opcode.banks))
	        );
}

QString Opcode::toStringGETAI(const Section1File *scriptsAndTexts, const OpcodeGETAI &opcode) const
{
	return Opcode::tr("Store triangle ID of the group %1 in %2")
	        .arg(
	            _groupScript(opcode.groupID, scriptsAndTexts),
	            _bank(opcode.varI, B2(opcode.banks))
	        );
}

QString Opcode::toStringANIMX2(const Section1File *scriptsAndTexts, const OpcodeANIMX2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play animation #%1 of the field model (speed=%2, type=2)")
	        .arg(opcode.animID)
	        .arg(opcode.speed);
}

QString Opcode::toStringCANIM2(const Section1File *scriptsAndTexts, const OpcodeCANIM2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play partially the animation #%1 of the field model and reset to initial state (first frame=%2, last frame=%3, speed=%4)")
	        .arg(opcode.animID)
	        .arg(opcode.firstFrame)
	        .arg(opcode.lastFrame)
	        .arg(opcode.speed);
}

QString Opcode::toStringCANMX2(const Section1File *scriptsAndTexts, const OpcodeCANMX2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play partially the animation #%1 of the field model (first frame=%2, last frame=%3, speed=%4)")
	        .arg(opcode.animID)
	        .arg(opcode.firstFrame)
	        .arg(opcode.lastFrame)
	        .arg(opcode.speed);
}

QString Opcode::toStringASPED(const Section1File *scriptsAndTexts, const OpcodeASPED &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set the field model animations speed: %1")
	        .arg(_var(opcode.speed, B2(opcode.banks)));
}

QString Opcode::toStringUnusedBE(const Section1File *scriptsAndTexts, const OpcodeUnusedBE &opcode) const
{
	Q_UNUSED(opcode)
	return toStringUnused0C(scriptsAndTexts, _opcode.opcodeUnused0C);
}

QString Opcode::toStringCC(const Section1File *scriptsAndTexts, const OpcodeCC &opcode) const
{
	return Opcode::tr("Control the group %1")
	        .arg(_groupScript(opcode.groupID, scriptsAndTexts));
}

QString Opcode::toStringJUMP(const Section1File *scriptsAndTexts, const OpcodeJUMP &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Field model jump (X=%1, Y=%2, triangle ID=%3, Steps=%4)")
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks[0])),
	            _var(opcode.targetY, B2(opcode.banks[0])),
	            _var(opcode.targetI, B1(opcode.banks[1])),
	            _var(opcode.height, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringAXYZI(const Section1File *scriptsAndTexts, const OpcodeAXYZI &opcode) const
{
	return Opcode::tr("Store position of the group %1 in %2 (X), %3 (Y), %4 (Z) and %5 (triangle ID)")
	        .arg(
	            _groupScript(opcode.groupID, scriptsAndTexts),
	            _bank(opcode.varX, B1(opcode.banks[0])),
	            _bank(opcode.varY, B2(opcode.banks[0])),
	            _bank(opcode.varZ, B1(opcode.banks[1])),
	            _bank(opcode.varI, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringLADER(const Section1File *scriptsAndTexts, const OpcodeLADER &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Climb a ladder with the animation #%6 (X=%1, Y=%2, Z=%3, triangle ID=%4, direction1=%5, direction2=%7, speed=%8)")
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks[0])),
	           _var(opcode.targetY, B2(opcode.banks[0])),
	           _var(opcode.targetZ, B1(opcode.banks[1])),
	           _var(opcode.targetI, B2(opcode.banks[1]))
	        )
	        .arg(opcode.way)
	        .arg(opcode.animID)
	        .arg(opcode.direction)
	        .arg(opcode.speed);
}

QString Opcode::toStringOFST(const Section1File *scriptsAndTexts, const OpcodeOFST &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Offset Object (movement=%1, X=%2, Y=%3, Z=%4, speed=%5)")
	        .arg(opcode.moveType)
	        .arg(
	            _var(opcode.targetX, B1(opcode.banks[0])),
	            _var(opcode.targetY, B2(opcode.banks[0])),
	            _var(opcode.targetZ, B1(opcode.banks[1])),
	            _var(opcode.speed, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringOFSTW(const Section1File *scriptsAndTexts, const OpcodeOFSTW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Wait for offset object");
}

QString Opcode::toStringTALKR(const Section1File *scriptsAndTexts, const OpcodeTALKR &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set range of the talk circle for the field model: %1")
	        .arg(_var(opcode.range, B2(opcode.banks)));
}

QString Opcode::toStringSLIDR(const Section1File *scriptsAndTexts, const OpcodeSLIDR &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set range of the contact circle for the field model: %1")
	        .arg(_var(opcode.range, B2(opcode.banks)));
}

QString Opcode::toStringSOLID(const Section1File *scriptsAndTexts, const OpcodeSOLID &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 contact with field model")
	        .arg(opcode.disabled == 0 ? Opcode::tr("Activate") : Opcode::tr("Deactivate"));
}

QString Opcode::toStringPRTYP(const Section1File *scriptsAndTexts, const OpcodePRTYP &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Add %1 to the current party")
	        .arg(character(opcode.charID));
}

QString Opcode::toStringPRTYM(const Section1File *scriptsAndTexts, const OpcodePRTYM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Remove %1 from the current party")
	        .arg(character(opcode.charID));
}

QString Opcode::toStringPRTYE(const Section1File *scriptsAndTexts, const OpcodePRTYE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("New party: %1 | %2 | %3")
	        .arg(
	            character(opcode.charID[0]),
	            character(opcode.charID[1]),
	            character(opcode.charID[2])
	        );
}

QString Opcode::toStringIFPRTYQ(const Section1File *scriptsAndTexts, const OpcodeIFPRTYQ &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If %1 is in the current party (%2)")
	        .arg(
	            character(opcode.charID),
	            opcode.getBadJump()
	             ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	             : Opcode::tr("else goto label %1").arg(opcode._label)
	        );
}

QString Opcode::toStringIFMEMBQ(const Section1File *scriptsAndTexts, const OpcodeIFMEMBQ &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If %1 exists (%2)")
	        .arg(
	           character(opcode.charID),
	           opcode.getBadJump()
	             ? Opcode::tr("else forward %n byte(s)", "With plural", opcode.jump)
	             : Opcode::tr("else goto label %1").arg(opcode._label)
	        );
}

QString Opcode::toStringMMBud(const Section1File *scriptsAndTexts, const OpcodeMMBud &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%2 %1")
	        .arg(
	            opcode.exists == 0 ? Opcode::tr("not available") : Opcode::tr("available"),
	            character(opcode.charID)
	        );
}

QString Opcode::toStringMMBLK(const Section1File *scriptsAndTexts, const OpcodeMMBLK &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Locks %1 in PHS menu")
	        .arg(character(opcode.charID));
}

QString Opcode::toStringMMBUK(const Section1File *scriptsAndTexts, const OpcodeMMBUK &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Unlock %1 in PHS menu")
	        .arg(character(opcode.charID));
}

QString Opcode::toStringLINE(const Section1File *scriptsAndTexts, const OpcodeLINE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Create line (X1=%1, Y1=%2, Z1=%3, X2=%4, Y2=%5, Z2=%6)")
	        .arg(opcode.targetX1)
	        .arg(opcode.targetY1)
	        .arg(opcode.targetZ1)
	        .arg(opcode.targetX2)
	        .arg(opcode.targetY2)
	        .arg(opcode.targetZ2);
}

QString Opcode::toStringLINON(const Section1File *scriptsAndTexts, const OpcodeLINON &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 line")
	        .arg(opcode.enabled != 0 ? Opcode::tr("Enable") : Opcode::tr("Disable"));
}

QString Opcode::toStringMPJPO(const Section1File *scriptsAndTexts, const OpcodeMPJPO &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Gateways %1")
	        .arg(opcode.disabled == 0 ? Opcode::tr("ON") : Opcode::tr("OFF"));
}

QString Opcode::toStringSLINE(const Section1File *scriptsAndTexts, const OpcodeSLINE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set line (X1=%1, Y1=%2, Z1=%3, X2=%4, Y2=%5, Z2=%6)")
	        .arg(
	            _var(opcode.targetX1, B1(opcode.banks[0])),
	            _var(opcode.targetY1, B2(opcode.banks[0])),
	            _var(opcode.targetZ1, B1(opcode.banks[1])),
	            _var(opcode.targetX2, B2(opcode.banks[1])),
	            _var(opcode.targetY2, B1(opcode.banks[2])),
	            _var(opcode.targetZ2, B2(opcode.banks[2]))
	        );
}

QString Opcode::toStringSIN(const Section1File *scriptsAndTexts, const OpcodeSIN &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%4 = ((Sinus(%1) * %2) + %3) >> 12")
	        .arg(
	            _var(opcode.value1, B1(opcode.banks[0])),
	            _var(opcode.value2, B2(opcode.banks[0])),
	            _var(opcode.value3, B1(opcode.banks[1])),
	            _bank(opcode.var, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringCOS(const Section1File *scriptsAndTexts, const OpcodeCOS &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%4 = ((Cosinus(%1) * %2) + %3) >> 12")
	        .arg(
	            _var(opcode.value1, B1(opcode.banks[0])),
	            _var(opcode.value2, B2(opcode.banks[0])),
	            _var(opcode.value3, B1(opcode.banks[1])),
	            _bank(opcode.var, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringTLKR2(const Section1File *scriptsAndTexts, const OpcodeTLKR2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set range of the talk circle for the field model: %1")
	        .arg(_var(opcode.range, B2(opcode.banks)));
}

QString Opcode::toStringSLDR2(const Section1File *scriptsAndTexts, const OpcodeSLDR2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set range of the contact circle for the field model: %1")
	        .arg(_var(opcode.range, B2(opcode.banks)));
}

QString Opcode::toStringPMJMP(const Section1File *scriptsAndTexts, const OpcodePMJMP &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Preload the field map %1")
	        .arg(_field(opcode.mapID, scriptsAndTexts));
}

QString Opcode::toStringPMJMP2(const Section1File *scriptsAndTexts, const OpcodePMJMP2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("PMJMP2");
}

QString Opcode::toStringAKAO2(const Section1File *scriptsAndTexts, const OpcodeAKAO2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 (param1=%2, param2=%3, param3=%4, param4=%5, param5=%6)")
	        .arg(
	            akao(opcode.opcode),
	            _var(opcode.param1, B1(opcode.banks[0])),
	            _var(opcode.param2, B2(opcode.banks[0])),
	            _var(opcode.param3, B1(opcode.banks[1])),
	            _var(opcode.param4, B2(opcode.banks[1])),
	            _var(opcode.param5, B2(opcode.banks[2]))
	        );
}

QString Opcode::toStringFCFIX(const Section1File *scriptsAndTexts, const OpcodeFCFIX &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 rotation")
	        .arg(opcode.disabled == 0 ? Opcode::tr("Activate") : Opcode::tr("Deactivate"));
}

QString Opcode::toStringCCANM(const Section1File *scriptsAndTexts, const OpcodeCCANM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play animation #%1 for '%3' (speed=%2)")
	        .arg(opcode.animID)
	        .arg(opcode.speed)
	        .arg(opcode.standWalkRun == 0
	             ? Opcode::tr("stay")
	             : (opcode.standWalkRun == 1
	                ? Opcode::tr("walk")
	                : Opcode::tr("run")));
}

QString Opcode::toStringANIMB(const Section1File *scriptsAndTexts, const OpcodeANIMB &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Break field model animation");
}

QString Opcode::toStringTURNW(const Section1File *scriptsAndTexts, const OpcodeTURNW &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Wait for rotation");
}

QString Opcode::toStringMPPAL(const Section1File *scriptsAndTexts, const OpcodeMPPAL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Multiply RGB(%6, %5, %4) on the colors in a palette (sourcePal=%1, targetPal=%2, first color=%3, color count=%7+1)")
	        .arg(opcode.posSrc)
	        .arg(opcode.posDst)
	        .arg(
	            _var(opcode.start, B1(opcode.banks[0])),
	            _var(opcode.b, B2(opcode.banks[0])),
	            _var(opcode.g, B1(opcode.banks[1])),
	            _var(opcode.r, B2(opcode.banks[1])),
	            _var(opcode.colorCount, B2(opcode.banks[2]))
	        );
}

QString Opcode::toStringBGON(const Section1File *scriptsAndTexts, const OpcodeBGON &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Show the state #%2 of the background parameter #%1")
	        .arg(
	            _var(opcode.bgParamID, B1(opcode.banks)),
	            _var(opcode.bgStateID, B2(opcode.banks))
	        );
}

QString Opcode::toStringBGOFF(const Section1File *scriptsAndTexts, const OpcodeBGOFF &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Hide the state #%2 of the background parameter #%1")
	        .arg(
	            _var(opcode.bgParamID, B1(opcode.banks)),
	            _var(opcode.bgStateID, B2(opcode.banks))
	        );
}

QString Opcode::toStringBGROL(const Section1File *scriptsAndTexts, const OpcodeBGROL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Show next state of the background parameter #%1")
	        .arg(_var(opcode.bgParamID, B2(opcode.banks)));
}

QString Opcode::toStringBGROL2(const Section1File *scriptsAndTexts, const OpcodeBGROL2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Show previous state of the background parameter #%1")
	        .arg(_var(opcode.bgParamID, B2(opcode.banks)));
}

QString Opcode::toStringBGCLR(const Section1File *scriptsAndTexts, const OpcodeBGCLR &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Hide background parameter #%1")
	        .arg(_var(opcode.bgParamID, B2(opcode.banks)));
}

QString Opcode::toStringSTPAL(const Section1File *scriptsAndTexts, const OpcodeSTPAL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Load the palette #%1 in the position %2 (color count=%3)")
	        .arg(
	            _var(opcode.palID, B1(opcode.banks)),
	            _var(opcode.position, B2(opcode.banks))
	        )
	        .arg(opcode.colorCount + 1);
}

QString Opcode::toStringLDPAL(const Section1File *scriptsAndTexts, const OpcodeLDPAL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Load the position %1 in the palette #%2 (color count=%3)")
	        .arg(
	            _var(opcode.position, B1(opcode.banks)),
	            _var(opcode.palID, B2(opcode.banks))
	        )
	        .arg(opcode.colorCount + 1);
}

QString Opcode::toStringCPPAL(const Section1File *scriptsAndTexts, const OpcodeCPPAL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Copy palette (sourcePal=%1, targetPal=%2, color count=%3)")
	        .arg(
	            _var(opcode.posSrc, B1(opcode.banks)),
	            _var(opcode.posDst, B2(opcode.banks))
	        )
	        .arg(opcode.colorCount + 1);
}

QString Opcode::toStringRTPAL(const Section1File *scriptsAndTexts, const OpcodeRTPAL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Copy partially palette (sourcePal=%1, targetPal=%2, first color=%3, color count=%4)")
	        .arg(
	            _var(opcode.posSrc, B1(opcode.banks[0])),
	            _var(opcode.posDst, B2(opcode.banks[0])),
	            _var(opcode.start, B2(opcode.banks[1]))
	        )
	        .arg(opcode.end + 1);
}

QString Opcode::toStringADPAL(const Section1File *scriptsAndTexts, const OpcodeADPAL &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Add RGB(%5, %4, %3) on the colors in a palette (sourcePal=%1, targetPal=%2, color count=%6)")
	        .arg(
	            _var(opcode.posSrc, B1(opcode.banks[0])),
	            _var(opcode.posDst, B2(opcode.banks[0])),
	            _var(opcode.b, B1(opcode.banks[1])),
	            _var(opcode.g, B2(opcode.banks[1])),
	            _var(opcode.r, B1(opcode.banks[2]))
	        )
	        .arg(opcode.colorCount + 1);
}

QString Opcode::toStringMPPAL2(const Section1File *scriptsAndTexts, const OpcodeMPPAL2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Multiply RGB(%5, %4, %3) on the colors in a palette (sourcePal=%1, targetPal=%2, color count=%6)")
	        .arg(
	            _var(opcode.posSrc, B1(opcode.banks[0])),
	            _var(opcode.posDst, B2(opcode.banks[0])),
	            _var(opcode.b, B1(opcode.banks[1])),
	            _var(opcode.g, B2(opcode.banks[1])),
	            _var(opcode.r, B1(opcode.banks[2]))
	        )
	        .arg(opcode.colorCount + 1);
}

QString Opcode::toStringSTPLS(const Section1File *scriptsAndTexts, const OpcodeSTPLS &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Load the palette #%1 in the position %2 (first color=%3, color count=%4)")
	        .arg(opcode.palID)
	        .arg(opcode.posSrc)
	        .arg(opcode.start)
	        .arg(opcode.colorCount + 1);
}

QString Opcode::toStringLDPLS(const Section1File *scriptsAndTexts, const OpcodeLDPLS &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Load the position %1 in the palette #%2 (first color=%3, color count=%4)")
	        .arg(opcode.posSrc)
	        .arg(opcode.palID)
	        .arg(opcode.start)
	        .arg(opcode.colorCount + 1);
}

QString Opcode::toStringCPPAL2(const Section1File *scriptsAndTexts, const OpcodeCPPAL2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	QString colorCount = B2(opcode.banks[1])
	    ? QString("%1 + 1").arg(_var(opcode.colorCount, B2(opcode.banks[1])))
	    : QString::number(opcode.colorCount + 1);
	return Opcode::tr("Copy palette (2) (sourceTile=%1, targetTile=%2, sourcePal=%3, targetPal=%4, color count=%5 + 1)")
	        .arg(opcode.posTileSrc)
	        .arg(opcode.posTileDst)
	        .arg(
	            _var(opcode.posSrc, B1(opcode.banks[0])),
	            _var(opcode.posDst, B1(opcode.banks[1])),
	            colorCount
	        );
}

QString Opcode::toStringRTPAL2(const Section1File *scriptsAndTexts, const OpcodeRTPAL2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Copy partially palette (2) (sourceTile=%1, targetTile=%2, sourcePal=%3, targetPal=%4, first color=%5)")
	        .arg(opcode.posTileSrc)
	        .arg(opcode.posTileDst)
	        .arg(
	            _var(opcode.posSrc, B1(opcode.banks[0])),
	            _var(opcode.posDst, B2(opcode.banks[0])),
	            _var(opcode.start, B2(opcode.banks[1]))
	        );
}

QString Opcode::toStringADPAL2(const Section1File *scriptsAndTexts, const OpcodeADPAL2 &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	QString colorCount = B2(opcode.banks[2])
	    ? QString("%1 + 1").arg(_var(opcode.colorCount, B2(opcode.banks[2])))
	    : QString::number(opcode.colorCount + 1);
	return Opcode::tr("Add RGB(%6, %5, %4) on the colors in a palette (2) (sourceTile=%1, targetTile=%2, first color=%3, color count=%7)")
	        .arg(opcode.posTileSrc)
	        .arg(opcode.posTileDst)
	        .arg(
	           _var(opcode.start, B1(opcode.banks[0])),
	           _var(opcode.b, B1(opcode.banks[1])),
	           _var(opcode.g, B2(opcode.banks[1])),
	           _var(opcode.r, B1(opcode.banks[2])),
	           colorCount
	        );
}

QString Opcode::toStringMUSIC(const Section1File *scriptsAndTexts, const OpcodeMUSIC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play music #%1")
	        .arg(opcode.musicID);
}

QString Opcode::toStringSOUND(const Section1File *scriptsAndTexts, const OpcodeSOUND &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play sound #%1 (position=%2/127)")
	        .arg(
	           _var(opcode.soundID, B1(opcode.banks)),
	           _var(opcode.position, B2(opcode.banks))
	        );
}

QString Opcode::toStringAKAO(const Section1File *scriptsAndTexts, const OpcodeAKAO &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 (param1 (8-bit)=%2, param2=%3, param3=%4, param4=%5, param5=%6)")
	        .arg(
	            akao(opcode.opcode),
	            _var(opcode.param1, B1(opcode.banks[0])),
	            _var(opcode.param2, B2(opcode.banks[0])),
	            _var(opcode.param3, B1(opcode.banks[1])),
	            _var(opcode.param4, B2(opcode.banks[1])),
	            _var(opcode.param5, B2(opcode.banks[2]))
	        );
}

QString Opcode::toStringMUSVT(const Section1File *scriptsAndTexts, const OpcodeMUSVT &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Play temporary music #%1")
	        .arg(opcode.musicID);
}

QString Opcode::toStringMUSVM(const Section1File *scriptsAndTexts, const OpcodeMUSVM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("MUSVM (music #%1)")
	        .arg(opcode.musicID);
}

QString Opcode::toStringMULCK(const Section1File *scriptsAndTexts, const OpcodeMULCK &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("%1 music")
	        .arg(opcode.disabled == 0 ? Opcode::tr("Unlock") : Opcode::tr("Lock", "test"));
}

QString Opcode::toStringBMUSC(const Section1File *scriptsAndTexts, const OpcodeBMUSC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set the music #%1 for next battle")
	        .arg(opcode.musicID);
}

QString Opcode::toStringCHMPH(const Section1File *scriptsAndTexts, const OpcodeCHMPH &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("CHMPH: Save (unknown) in %1 and (unknown) in %2")
	        .arg(
	            _bank(opcode.var1, B1(opcode.banks)),
	            _bank(opcode.var2, B2(opcode.banks))
	        );
}

QString Opcode::toStringPMVIE(const Section1File *scriptsAndTexts, const OpcodePMVIE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set next movie: %1")
	        .arg(_movie(opcode.movieID));
}

QString Opcode::toStringMOVIE(const Section1File *scriptsAndTexts, const OpcodeMOVIE &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Play movie");
}

QString Opcode::toStringMVIEF(const Section1File *scriptsAndTexts, const OpcodeMVIEF &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Save Movie frame in %1")
	        .arg(_bank(opcode.varCurMovieFrame, B2(opcode.banks)));
}

QString Opcode::toStringMVCAM(const Section1File *scriptsAndTexts, const OpcodeMVCAM &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Camera Movie: %1")
	        .arg(opcode.movieCamID);
}

QString Opcode::toStringFMUSC(const Section1File *scriptsAndTexts, const OpcodeFMUSC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("Set next field music for when we will be back to the map: #%1")
	        .arg(opcode.musicID);
}

QString Opcode::toStringCMUSC(const Section1File *scriptsAndTexts, const OpcodeCMUSC &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("CMUSC (music #%1, operation=%2, param1=%3, param2=%4)")
	        .arg(opcode.musicID)
	        .arg(
	            akao(opcode.opcode),
	            _var(opcode.param1, B1(opcode.banks)),
	            _var(opcode.param2, B2(opcode.banks))
	        );
}

QString Opcode::toStringCHMST(const Section1File *scriptsAndTexts, const OpcodeCHMST &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	return Opcode::tr("If music is currently playing set %1 to 1")
	        .arg(_bank(opcode.var, B2(opcode.banks)));
}

QString Opcode::toStringGAMEOVER(const Section1File *scriptsAndTexts, const OpcodeGAMEOVER &opcode) const
{
	Q_UNUSED(scriptsAndTexts)
	Q_UNUSED(opcode)
	return Opcode::tr("Game Over");
}

QString Opcode::_groupScript(quint8 param, const Section1File *scriptsAndTexts)
{
	if (param < scriptsAndTexts->grpScriptCount()) {
		return scriptsAndTexts->grpScript(param).name() + Opcode::tr(" (No%1)").arg(param);
	}
	return Opcode::tr("? (No%1)").arg(param);
}

QString Opcode::_text(quint8 textID, const Section1File *scriptsAndTexts)
{
	if (textID < scriptsAndTexts->textCount()) {
		QString t = scriptsAndTexts->text(textID).text().simplified();
		if (t.size() > 70) {
			t = t.left(35) % QString("...") % t.right(35);
		}
		return "\"" + t + "\"";
	}
	return Opcode::tr("(no text)");
}

QString Opcode::_item(quint16 itemID, quint8 bank)
{
	if (bank > 0) {
		return Opcode::tr("No%1").arg(_bank(itemID & 0xFF, bank));
	}
	
	if (itemID < 128) {
		if (!Data::item_names.isEmpty() && itemID < Data::item_names.size())
			return Data::item_names.at(itemID);
	} else if (itemID < 256) {
		if (!Data::weapon_names.isEmpty() && itemID-128 < Data::weapon_names.size())
			return Data::weapon_names.at(itemID-128);
	} else if (itemID < 288) {
		if (!Data::armor_names.isEmpty() && itemID-256 < Data::armor_names.size())
			return Data::armor_names.at(itemID-256);
	} else if (itemID < 320) {
		if (!Data::accessory_names.isEmpty() && itemID-288 < Data::accessory_names.size())
			return Data::accessory_names.at(itemID-288);
	}
	return Opcode::tr("No%1").arg(itemID);
}

QString Opcode::_materia(quint8 materiaID, quint8 bank)
{
	if (bank > 0) {
		return Opcode::tr("No%1").arg(_bank(materiaID, bank));
	}
	
	if (materiaID < Data::materia_names.size()) {
		return Data::materia_names.at(materiaID);
	}
	return Opcode::tr("No%1").arg(materiaID);
}

QString Opcode::_field(quint16 mapID, const Section1File *scriptsAndTexts)
{
	const QStringList *mapNames = nullptr;

	if (scriptsAndTexts != nullptr
	    && scriptsAndTexts->field()->io() != nullptr
	    && scriptsAndTexts->field()->io()->fieldArchive() != nullptr) {
		mapNames = &(scriptsAndTexts->field()->io()->fieldArchive()->mapList().mapNames());
	} else if (mapID < Data::maplist(false).size()) {
		mapNames = &Data::maplist(false);
	}

	if (mapNames != nullptr && mapID < mapNames->size()) {
		return Opcode::tr("%1 (#%2)")
		    .arg(mapNames->at(mapID))
		    .arg(mapID);
	}
	return Opcode::tr("No%1").arg(mapID);
}

QString Opcode::_movie(quint8 movieID)
{
	// Movie names are different according to the current disc
	QString dflt = Opcode::tr("No%1").arg(movieID);
	QStringList cds;
	cds << Data::movie_names_cd1.value(movieID, dflt)
	    << Data::movie_names_cd2.value(movieID, dflt)
	    << Data::movie_names_cd3.value(movieID, dflt);
	
	QStringList out;
	for (int discID = 0; discID < 3; ++discID) {
		out.append(Opcode::tr("%1 (disc %2)").arg(cds.at(discID)).arg(discID + 1));
	}
	
	return out.join(", ");
}

/* QString Opcode::_objet3D(quint8 objet3D_ID)
{
	if (objet3D_ID < Data::currentCharNames.size())
		return QString("%1 (%2)").arg(Data::currentCharNames.at(objet3D_ID), Data::currentHrcNames.at(objet3D_ID));
	return Opcode::tr("No%1").arg(objet3D_ID);
} */

QString Opcode::akao(quint8 akaoOp, bool *ok)
{
	if (ok) {
		*ok = true;
	}

	switch (akaoOp) {
	case 0x10:
	case 0x14:
		return Opcode::tr("Play music [param1: music ID, 0-based]");
	case 0x18:
	case 0x19:
		return Opcode::tr("Play music and resume from last position [param1: music ID, 0-based]");
	case 0x24:
	case 0x20:
		return Opcode::tr("Play a sound effect (will be terminated if "
		                   "another effect is played on channel) "
		                   "[param1: panning, param2: effect ID]");
	case 0x25:
	case 0x21:
		return Opcode::tr("Play a sound effect (will be terminated if "
		                   "another effect is played on channel) "
		                   "[param1: panning, param2: effect ID, param3: ?]");
	case 0x26:
	case 0x22:
		return Opcode::tr("Play a sound effect (will be terminated if "
		                   "another effect is played on channel) "
		                   "[param1: panning, param2: effect ID, param3: ?, param4: ?]");
	case 0x27:
	case 0x23:
		return Opcode::tr("Play a sound effect (will be terminated if "
		                   "another effect is played on channel) "
		                   "[param1: panning, param2: effect ID, param3: ?, param4: ?, param5: ?]");
	case 0x28:
		return Opcode::tr("Play a sound effect on channel #1 [param1: panning, param2: effect ID]");
	case 0x29:
		return Opcode::tr("Play a sound effect on channel #2 [param1: panning, param2: effect ID]");
	case 0x2A:
		return Opcode::tr("Play a sound effect on channel #3 [param1: panning, param2: effect ID]");
	case 0x2B:
		return Opcode::tr("Play a sound effect on channel #4 [param1: panning, param2: effect ID]");
	case 0x30:
		return Opcode::tr("Play a sound effect (cannot be stopped) [param1: effect ID]");
	case 0x98:
		return Opcode::tr("Resumes music and sound effects");
	case 0x99:
		return Opcode::tr("Pauses music and sound effects");
	case 0x9A:
		return Opcode::tr("Resumes only the music");
	case 0x9B:
		return Opcode::tr("Pauses only the music");
	case 0x9C:
		return Opcode::tr("Resumes only sound effects");
	case 0x9D:
		return Opcode::tr("Pauses only sound effects");
	case 0xA0:
		return Opcode::tr("Volume control (channel #1) [param1: volume]");
	case 0xA1:
		return Opcode::tr("Volume control (channel #2) [param1: volume]");
	case 0xA2:
		return Opcode::tr("Volume control (channel #3) [param1: volume]");
	case 0xA3:
		return Opcode::tr("Volume control (channel #4) [param1: volume]");
	case 0xA4:
		return Opcode::tr("Volume transitions (channel #1) [param1: transition time, param2: target volume]");
	case 0xA5:
		return Opcode::tr("Volume transitions (channel #2) [param1: transition time, param2: target volume]");
	case 0xA6:
		return Opcode::tr("Volume transitions (channel #3) [param1: transition time, param2: target volume]");
	case 0xA7:
		return Opcode::tr("Volume transitions (channel #4) [param1: transition time, param2: target volume]");
	case 0xA8:
		return Opcode::tr("Pan control (channel #1)");
	case 0xA9:
		return Opcode::tr("Pan control (channel #2)");
	case 0xAA:
		return Opcode::tr("Pan control (channel #3)");
	case 0xAB:
		return Opcode::tr("Pan control (channel #4)");
	case 0xAC:
		return Opcode::tr("Pan transitions (channel #1)");
	case 0xAD:
		return Opcode::tr("Pan transitions (channel #2)");
	case 0xAE:
		return Opcode::tr("Pan transitions (channel #3)");
	case 0xAF:
		return Opcode::tr("Pan transitions (channel #4)");
	case 0xB0:
		return Opcode::tr("Tempo control (channel #1)");
	case 0xB1:
		return Opcode::tr("Tempo control (channel #2)");
	case 0xB2:
		return Opcode::tr("Tempo control (channel #3)");
	case 0xB3:
		return Opcode::tr("Tempo control (channel #4)");
	case 0xB4:
		return Opcode::tr("Tempo transitions (channel #1)");
	case 0xB5:
		return Opcode::tr("Tempo transitions (channel #2)");
	case 0xB6:
		return Opcode::tr("Tempo transitions (channel #3)");
	case 0xB7:
		return Opcode::tr("Tempo transitions (channel #4)");
	case 0xB8:
		return Opcode::tr("Volume control for all channels [param1: volume]");
	case 0xB9:
		return Opcode::tr("Volume transitions for all channels [param1: transition time, param2: target volume]");
	case 0xBA:
		return Opcode::tr("Pan control for all channels");
	case 0xBB:
		return Opcode::tr("Pan transitions for all channels");
	case 0xBC:
		return Opcode::tr("Tempo control for all channels");
	case 0xBD:
		return Opcode::tr("Tempo transitions for all channels");
	case 0xC0:
		return Opcode::tr("Set music volume [param1: volume]");
	case 0xC1:
		return Opcode::tr("Music volume transition [param1: transition time, param2: target volume]");
	case 0xC2:
		return Opcode::tr("Fade music volume");
	case 0xC8:
		return Opcode::tr("Set music pan (noop in PC version)");
	case 0xC9:
		return Opcode::tr("Music pan transition (noop in PC version)");
	case 0xCA:
		return Opcode::tr("Music pan fade (noop in PC version)");
	case 0xD0:
		return Opcode::tr("Set music tempo [param1: tempo, 0x20 is normal]");
	case 0xD1:
		return Opcode::tr("Music tempo transition");
	case 0xD2:
		return Opcode::tr("Music tempo fade");
	case 0xDA:
		return Opcode::tr("Stop music-like (noop in PC version)");
	case 0xF0:
		return Opcode::tr("Stop music");
	case 0xF1:
		return Opcode::tr("Stop sound effects");
	default:
		if (ok) {
			*ok = false;
		}
		
		return Opcode::tr("AKAO: %1?").arg(akaoOp);
	}
}

QString Opcode::_bank(quint8 address, quint8 bank)
{
	if (!Var::name(bank, address).isEmpty()) {
		return Var::name(bank, address);
	}
	if (bank == 0) {
		return QString("?");
	}
	return QString("Var[%1][%2]").arg(bank).arg(address);
}

QString Opcode::_var(int value, quint8 bank)
{
	if (bank > 0) {
		return _bank(value & 0xFF, bank);
	}
	return QString::number(value);
}

QString Opcode::_var(int value, quint8 bank1, quint8 bank2)
{
	if (bank1 > 0 || bank2 > 0) {
		QString ret = _var(value & 0xFFFF, bank1);
		if (bank2 != 0 || ((value >> 16) & 0xFFFF) != 0) {
			ret += QString(" + %1 * 65536").arg(_var((value >> 16) & 0xFFFF, bank2));
		}
		return ret;
	}
	return QString::number(value);
}

QString Opcode::_var(const quint8 value[3], quint8 bank1, quint8 bank2, quint8 bank3)
{
	if (bank1 > 0 || bank2 > 0 || bank3 > 0) {
		return Opcode::tr("%1 and %2 and %3")
		        .arg(
		            _bank(value[0], bank1),
		            _bank(value[1], bank2),
		            _bank(value[2], bank3)
		        );
	}
	return QString::number(value[0] | (value[1] << 8) | (value[2] << 16));
}

QString Opcode::_var(const quint8 value[4], quint8 bank1, quint8 bank2, quint8 bank3, quint8 bank4)
{
	if (bank1 > 0 || bank2 > 0 || bank3 > 0 || bank4 > 0) {
		return Opcode::tr("%1 and %2 and %3 and %4")
		        .arg(
		            _bank(value[0], bank1),
		            _bank(value[1], bank2),
		            _bank(value[2], bank3),
		            _bank(value[3], bank4)
		        );
	}
	return QString::number(value[0] | (value[1] << 8) | (value[2] << 16) | (value[3] << 24));
}

QString Opcode::character(quint8 persoID)
{
	if (persoID < Data::char_names.size()) {
		return Data::char_names.at(persoID);
	}
	if (persoID >= 254) {
		return Opcode::tr("(Empty)");
	}
	if (persoID >= 100) {
		return Data::char_names.last();
	}
	return QString("%1?").arg(persoID);
}

QString Opcode::_windowCorner(quint8 param, quint8 bank)
{
	if (bank > 0) {
		return _bank(param, bank);
	}
	
	switch (param) {
	case 0:     return Opcode::tr("Top Left");
	case 1:     return Opcode::tr("Bottom Left");
	case 2:     return Opcode::tr("Top Right");
	case 3:     return Opcode::tr("Bottom Right");
	default:    return QString("%1?").arg(param);
	}
}

QString Opcode::_sensRotation(quint8 param)
{
	switch (param) {
	case 1:     return Opcode::tr("reverse");
	case 2:     return Opcode::tr("reverse");
	default:    return Opcode::tr("normal");
	}
}

QString Opcode::_keys(quint16 keys)
{
	QStringList ret;
	for (quint8 i = 0; i < 16; ++i) {
		if ((keys >> i) & 1) {
			ret.append(Data::key_names.at(i));
		}
	}
	if (ret.isEmpty()) {
		return Opcode::tr("(no key)");
	}
	return ret.join(Opcode::tr(" or "));
}

QString Opcode::_operateur(quint8 param)
{
	return param < OPERATORS_SIZE ? QString::fromLatin1(operators[param]) : QString("%1?").arg(param);
}

const char *Opcode::operators[OPERATORS_SIZE] = {
    "==", "!=", ">", "<", ">=", "<=", "&", "^", "|", "bitON", "bitOFF"
};

const quint8 Opcode::length[257] =
{
    /* 00  RET      */    1,
    /* 01  REQ      */    3,
    /* 02  REQSW    */    3,
    /* 03  REQEW    */    3,
    /* 04  PREQ     */    3,
    /* 05  PRQSW    */    3,
    /* 06  PRQEW    */    3,
    /* 07  RETTO    */    2,
    /* 08  JOIN     */    2,
    /* 09  SPLIT    */    15,
    /* 0a  SPTYE    */    6,
    /* 0b  GTPYE    */    6,
    /* 0c           */    1,
    /* 0d           */    1,
    /* 0e  DSKCG    */    2,
    /* 0f  SPECIAL  */    2,
    
    /* 10  JMPF     */    2,
    /* 11  JMPFL    */    3,
    /* 12  JMPB     */    2,
    /* 13  JMPBL    */    3,
    /* 14  IFUB     */    6,
    /* 15  IFUBL    */    7,
    /* 16  IFSW     */    8,
    /* 17  IFSWL    */    9,
    /* 18  IFUW     */    8,
    /* 19  IFUWL    */    9,
    /* 1a           */    10,
    /* 1b           */    3,
    /* 1c           */    6,
    /* 1d           */    1,
    /* 1e           */    1,
    /* 1f           */    1,
    
    /* 20  MINIGAME */    11,
    /* 21  TUTOR    */    2,
    /* 22  BTMD2    */    5,
    /* 23  BTRLD    */    3,
    /* 24  WAIT     */    3,
    /* 25  NFADE    */    9,
    /* 26  BLINK    */    2,
    /* 27  BGMOVIE  */    2,
    /* 28  KAWAI    */    3,
    /* 29  KAWIW    */    1,
    /* 2a  PMOVA    */    2,
    /* 2b  SLIP     */    2,
    /* 2c  BGPDH    */    5,
    /* 2d  BGSCR    */    7,
    /* 2e  WCLS     */    2,
    /* 2f  WSIZW    */    10,
    
    /* 30  IFKEY    */    4,
    /* 31  IFKEYON  */    4,
    /* 32  IFKEYOFF */    4,
    /* 33  UC       */    2,
    /* 34  PDIRA    */    2,
    /* 35  PTURA    */    4,
    /* 36  WSPCL    */    5,
    /* 37  WNUMB    */    8,
    /* 38  STTIM    */    6,
    /* 39  GOLDu    */    6,
    /* 3a  GOLDd    */    6,
    /* 3b  CHGLD    */    4,
    /* 3c  HMPMAX1  */    1,
    /* 3d  HMPMAX2  */    1,
    /* 3e  MHMMX    */    1,
    /* 3f  HMPMAX3  */    1,
    
    /* 40  MESSAGE  */    3,
    /* 41  MPARA    */    5,
    /* 42  MPRA2    */    6,
    /* 43  MPNAM    */    2,
    /* 44           */    1,
    /* 45  MPu      */    5,
    /* 46           */    1,
    /* 47  MPd      */    5,
    /* 48  ASK      */    7,
    /* 49  MENU     */    4,
    /* 4a  MENU2    */    2,
    /* 4b  BTLTB    */    2,
    /* 4c           */    1,
    /* 4d  HPu      */    5,
    /* 4e           */    1,
    /* 4f  HPd      */    5,
    
    /* 50  WINDOW   */    10,
    /* 51  WMOVE    */    6,
    /* 52  WMODE    */    4,
    /* 53  WREST    */    2,
    /* 54  WCLSE    */    2,
    /* 55  WROW     */    3,
    /* 56  GWCOL    */    7,
    /* 57  SWCOL    */    7,
    /* 58  STITM    */    5,
    /* 59  DLITM    */    5,
    /* 5a  CKITM    */    5,
    /* 5b  SMTRA    */    7,
    /* 5c  DMTRA    */    8,
    /* 5d  CMTRA    */    10,
    /* 5e  SHAKE    */    8,
    /* 5f  NOP      */    1,
    
    /* 60  MAPJUMP  */    10,
    /* 61  SCRLO    */    2,
    /* 62  SCRLC    */    5,
    /* 63  SCRLA    */    6,
    /* 64  SCR2D    */    6,
    /* 65  SCRCC    */    1,
    /* 66  SCR2DC   */    9,
    /* 67  SCRLW    */    1,
    /* 68  SCR2DL   */    9,
    /* 69  MPDSP    */    2,
    /* 6a  VWOFT    */    7,
    /* 6b  FADE     */    9,
    /* 6c  FADEW    */    1,
    /* 6d  IDLCK    */    4,
    /* 6e  LSTMP    */    3,
    /* 6f  SCRLP    */    6,
    
    /* 70  BATTLE   */    4,
    /* 71  BTLON    */    2,
    /* 72  BTLMD    */    3,
    /* 73  PGTDR    */    4,
    /* 74  GETPC    */    4,
    /* 75  PXYZI    */    8,
    /* 76  PLUS!    */    4,
    /* 77  PLUS2!   */    5,
    /* 78  MINUS!   */    4,
    /* 79  MINUS2!  */    5,
    /* 7a  INC!     */    3,
    /* 7b  INC2!    */    3,
    /* 7c  DEC!     */    3,
    /* 7d  DEC2!    */    3,
    /* 7e  TLKON    */    2,
    /* 7f  RDMSD    */    3,
    
    /* 80  SETBYTE  */    4,
    /* 81  SETWORD  */    5,
    /* 82  BITON    */    4,
    /* 83  BITOFF   */    4,
    /* 84  BITXOR   */    4,
    /* 85  PLUS     */    4,
    /* 86  PLUS2    */    5,
    /* 87  MINUS    */    4,
    /* 88  MINUS2   */    5,
    /* 89  MUL      */    4,
    /* 8a  MUL2     */    5,
    /* 8b  DIV      */    4,
    /* 8c  DIV2     */    5,
    /* 8d  MOD      */    4,
    /* 8e  MOD2     */    5,
    /* 8f  AND      */    4,
    
    /* 90  AND2     */    5,
    /* 91  OR       */    4,
    /* 92  OR2      */    5,
    /* 93  XOR      */    4,
    /* 94  XOR2     */    5,
    /* 95  INC      */    3,
    /* 96  INC2     */    3,
    /* 97  DEC      */    3,
    /* 98  DEC2     */    3,
    /* 99  RANDOM   */    3,
    /* 9a  LBYTE    */    4,
    /* 9b  HBYTE    */    5,
    /* 9c  2BYTE    */    6,
    /* 9d  SETX     */    7,
    /* 9e  GETX     */    7,
    /* 9f  SEARCHX  */    11,
    
    /* a0  PC       */    2,
    /* a1  CHAR     */    2,
    /* a2  DFANM    */    3,
    /* a3  ANIME1   */    3,
    /* a4  VISI     */    2,
    /* a5  XYZI     */    11,
    /* a6  XYI      */    9,
    /* a7  XYZ      */    9,
    /* a8  MOVE     */    6,
    /* a9  CMOVE    */    6,
    /* aa  MOVA     */    2,
    /* ab  TURA     */    4,
    /* ac  ANIMW    */    1,
    /* ad  FMOVE    */    6,
    /* ae  ANIME2   */    3,
    /* af  ANIM!1   */    3,
    
    /* b0  CANIM1   */    5,
    /* b1  CANM!1   */    5,
    /* b2  MSPED    */    4,
    /* b3  DIR      */    3,
    /* b4  TURNGEN  */    6,
    /* b5  TURN     */    6,
    /* b6  DIRA     */    2,
    /* b7  GETDIR   */    4,
    /* b8  GETAXY   */    5,
    /* b9  GETAI    */    4,
    /* ba  ANIM!2   */    3,
    /* bb  CANIM2   */    5,
    /* bc  CANM!2   */    5,
    /* bd  ASPED    */    4,
    /* be           */    1,
    /* bf  CC       */    2,
    
    /* c0  JUMP     */    11,
    /* c1  AXYZI    */    8,
    /* c2  LADER    */    15,
    /* c3  OFST     */    12,
    /* c4  OFSTW    */    1,
    /* c5  TALKR    */    3,
    /* c6  SLIDR    */    3,
    /* c7  SOLID    */    2,
    /* c8  PRTYP    */    2,
    /* c9  PRTYM    */    2,
    /* ca  PRTYE    */    4,
    /* cb  IFPRTYQ  */    3,
    /* cc  IFMEMBQ  */    3,
    /* cd  MMBud    */    3,
    /* ce  MMBLK    */    2,
    /* cf  MMBUK    */    2,
    
    /* d0  LINE     */    13,
    /* d1  LINON    */    2,
    /* d2  MPJPO    */    2,
    /* d3  SLINE    */    16,
    /* d4  SIN      */    10,
    /* d5  COS      */    10,
    /* d6  TLKR2    */    4,
    /* d7  SLDR2    */    4,
    /* d8  PMJMP    */    3,
    /* d9  PMJMP2   */    1,
    /* da  AKAO2    */    15,
    /* db  FCFIX    */    2,
    /* dc  CCANM    */    4,
    /* dd  ANIMB    */    1,
    /* de  TURNW    */    1,
    /* df  MPPAL    */    11,
    
    /* e0  BGON     */    4,
    /* e1  BGOFF    */    4,
    /* e2  BGROL    */    3,
    /* e3  BGROL2   */    3,
    /* e4  BGCLR    */    3,
    /* e5  STPAL    */    5,
    /* e6  LDPAL    */    5,
    /* e7  CPPAL    */    5,
    /* e8  RTPAL    */    7,
    /* e9  ADPAL    */    10,
    /* ea  MPPAL2   */    10,
    /* eb  STPLS    */    5,
    /* ec  LDPLS    */    5,
    /* ed  CPPAL2   */    8,
    /* ee  RTPAL2   */    8,
    /* ef  ADPAL2   */    11,
    
    /* f0  MUSIC    */    2,
    /* f1  SOUND    */    5,
    /* f2  AKAO     */    14,
    /* f3  MUSVT    */    2,
    /* f4  MUSVM    */    2,
    /* f5  MULCK    */    2,
    /* f6  BMUSC    */    2,
    /* f7  CHMPH    */    4,
    /* f8  PMVIE    */    2,
    /* f9  MOVIE    */    1,
    /* fa  MVIEF    */    3,
    /* fb  MVCAM    */    2,
    /* fc  FMUSC    */    2,
    /* fd  CMUSC    */    8,
    /* fe  CHMST    */    3,
    /* ff  GAMEOVER */    1,
    /* 100 LABEL    */     0
};

const char *Opcode::names[257] =
{
    /* 00 */    "RET",
    /* 01 */    "REQ",
    /* 02 */    "REQSW",
    /* 03 */    "REQEW",
    /* 04 */    "PREQ",
    /* 05 */    "PRQSW",
    /* 06 */    "PRQEW",
    /* 07 */    "RETTO",
    /* 08 */    "JOIN",
    /* 09 */    "SPLIT",
    /* 0a */    "SPTYE",
    /* 0b */    "GTPYE",
    /* 0c */    "Unknown1",
    /* 0d */    "Unknown2",
    /* 0e */    "DSKCG",
    /* 0f */    "SPECIAL",
    
    /* 10 */    "JMPF",
    /* 11 */    "JMPFL",
    /* 12 */    "JMPB",
    /* 13 */    "JMPBL",
    /* 14 */    "IFUB",
    /* 15 */    "IFUBL",
    /* 16 */    "IFSW",
    /* 17 */    "IFSWL",
    /* 18 */    "IFUW",
    /* 19 */    "IFUWL",
    /* 1a */    "Unknown3",
    /* 1b */    "Unknown4",
    /* 1c */    "Unknown5",
    /* 1d */    "Unknown6",
    /* 1e */    "Unknown7",
    /* 1f */    "Unknown8",
    
    /* 20 */    "MINIGAME",
    /* 21 */    "TUTOR",
    /* 22 */    "BTMD2",
    /* 23 */    "BTRLD",
    /* 24 */    "WAIT",
    /* 25 */    "NFADE",
    /* 26 */    "BLINK",
    /* 27 */    "BGMOVIE",
    /* 28 */    "KAWAI",
    /* 29 */    "KAWIW",
    /* 2a */    "PMOVA",
    /* 2b */    "SLIP",
    /* 2c */    "BGPDH",
    /* 2d */    "BGSCR",
    /* 2e */    "WCLS",
    /* 2f */    "WSIZW",
    
    /* 30 */    "IFKEY",
    /* 31 */    "IFKEYON",
    /* 32 */    "IFKEYOFF",
    /* 33 */    "UC",
    /* 34 */    "PDIRA",
    /* 35 */    "PTURA",
    /* 36 */    "WSPCL",
    /* 37 */    "WNUMB",
    /* 38 */    "STTIM",
    /* 39 */    "GOLDu",
    /* 3a */    "GOLDd",
    /* 3b */    "CHGLD",
    /* 3c */    "HMPMAX1",
    /* 3d */    "HMPMAX2",
    /* 3e */    "MHMMX",
    /* 3f */    "HMPMAX3",
    
    /* 40 */    "MESSAGE",
    /* 41 */    "MPARA",
    /* 42 */    "MPRA2",
    /* 43 */    "MPNAM",
    /* 44 */    "Unknown9",
    /* 45 */    "MPu",
    /* 46 */    "Unknown10",
    /* 47 */    "MPd",
    /* 48 */    "ASK",
    /* 49 */    "MENU",
    /* 4a */    "MENU2",
    /* 4b */    "BTLTB",
    /* 4c */    "Unknown11",
    /* 4d */    "HPu",
    /* 4e */    "Unknown12",
    /* 4f */    "HPd",
    
    /* 50 */    "WINDOW",
    /* 51 */    "WMOVE",
    /* 52 */    "WMODE",
    /* 53 */    "WREST",
    /* 54 */    "WCLSE",
    /* 55 */    "WROW",
    /* 56 */    "GWCOL",
    /* 57 */    "SWCOL",
    /* 58 */    "STITM",
    /* 59 */    "DLITM",
    /* 5a */    "CKITM",
    /* 5b */    "SMTRA",
    /* 5c */    "DMTRA",
    /* 5d */    "CMTRA",
    /* 5e */    "SHAKE",
    /* 5f */    "NOP",
    
    /* 60 */    "MAPJUMP",
    /* 61 */    "SCRLO",
    /* 62 */    "SCRLC",
    /* 63 */    "SCRLA",
    /* 64 */    "SCR2D",
    /* 65 */    "SCRCC",
    /* 66 */    "SCR2DC",
    /* 67 */    "SCRLW",
    /* 68 */    "SCR2DL",
    /* 69 */    "MPDSP",
    /* 6a */    "VWOFT",
    /* 6b */    "FADE",
    /* 6c */    "FADEW",
    /* 6d */    "IDLCK",
    /* 6e */    "LSTMP",
    /* 6f */    "SCRLP",
    
    /* 70 */    "BATTLE",
    /* 71 */    "BTLON",
    /* 72 */    "BTLMD",
    /* 73 */    "PGTDR",
    /* 74 */    "GETPC",
    /* 75 */    "PXYZI",
    /* 76 */    "PLUS!",
    /* 77 */    "PLUS2!",
    /* 78 */    "MINUS!",
    /* 79 */    "MINUS2!",
    /* 7a */    "INC!",
    /* 7b */    "INC2!",
    /* 7c */    "DEC!",
    /* 7d */    "DEC2!",
    /* 7e */    "TLKON",
    /* 7f */    "RDMSD",
    
    /* 80 */    "SETBYTE",
    /* 81 */    "SETWORD",
    /* 82 */    "BITON",
    /* 83 */    "BITOFF",
    /* 84 */    "BITXOR",
    /* 85 */    "PLUS",
    /* 86 */    "PLUS2",
    /* 87 */    "MINUS",
    /* 88 */    "MINUS2",
    /* 89 */    "MUL",
    /* 8a */    "MUL2",
    /* 8b */    "DIV",
    /* 8c */    "DIV2",
    /* 8d */    "MOD",
    /* 8e */    "MOD2",
    /* 8f */    "AND",
    
    /* 90 */    "AND2",
    /* 91 */    "OR",
    /* 92 */    "OR2",
    /* 93 */    "XOR",
    /* 94 */    "XOR2",
    /* 95 */    "INC",
    /* 96 */    "INC2",
    /* 97 */    "DEC",
    /* 98 */    "DEC2",
    /* 99 */    "RANDOM",
    /* 9a */    "LBYTE",
    /* 9b */    "HBYTE",
    /* 9c */    "2BYTE",
    /* 9d */    "SETX",
    /* 9e */    "GETX",
    /* 9f */    "SEARCHX",
    
    /* a0 */    "PC",
    /* a1 */    "CHAR",
    /* a2 */    "DFANM",
    /* a3 */    "ANIME1",
    /* a4 */    "VISI",
    /* a5 */    "XYZI",
    /* a6 */    "XYI",
    /* a7 */    "XYZ",
    /* a8 */    "MOVE",
    /* a9 */    "CMOVE",
    /* aa */    "MOVA",
    /* ab */    "TURA",
    /* ac */    "ANIMW",
    /* ad */    "FMOVE",
    /* ae */    "ANIME2",
    /* af */    "ANIM!1",
    
    /* b0 */    "CANIM1",
    /* b1 */    "CANM!1",
    /* b2 */    "MSPED",
    /* b3 */    "DIR",
    /* b4 */    "TURNGEN",
    /* b5 */    "TURN",
    /* b6 */    "DIRA",
    /* b7 */    "GETDIR",
    /* b8 */    "GETAXY",
    /* b9 */    "GETAI",
    /* ba */    "ANIM!2",
    /* bb */    "CANIM2",
    /* bc */    "CANM!2",
    /* bd */    "ASPED",
    /* be */    "Unknown13",
    /* bf */    "CC",
    
    /* c0 */    "JUMP",
    /* c1 */    "AXYZI",
    /* c2 */    "LADER",
    /* c3 */    "OFST",
    /* c4 */    "OFSTW",
    /* c5 */    "TALKR",
    /* c6 */    "SLIDR",
    /* c7 */    "SOLID",
    /* c8 */    "PRTYP",
    /* c9 */    "PRTYM",
    /* ca */    "PRTYE",
    /* cb */    "IFPRTYQ",
    /* cc */    "IFMEMBQ",
    /* cd */    "MMBud",
    /* ce */    "MMBLK",
    /* cf */    "MMBUK",
    
    /* d0 */    "LINE",
    /* d1 */    "LINON",
    /* d2 */    "MPJPO",
    /* d3 */    "SLINE",
    /* d4 */    "SIN",
    /* d5 */    "COS",
    /* d6 */    "TLKR2",
    /* d7 */    "SLDR2",
    /* d8 */    "PMJMP",
    /* d9 */    "PMJMP2",
    /* da */    "AKAO2",
    /* db */    "FCFIX",
    /* dc */    "CCANM",
    /* dd */    "ANIMB",
    /* de */    "TURNW",
    /* df */    "MPPAL",
    
    /* e0 */    "BGON",
    /* e1 */    "BGOFF",
    /* e2 */    "BGROL",
    /* e3 */    "BGROL2",
    /* e4 */    "BGCLR",
    /* e5 */    "STPAL",
    /* e6 */    "LDPAL",
    /* e7 */    "CPPAL",
    /* e8 */    "RTPAL",
    /* e9 */    "ADPAL",
    /* ea */    "MPPAL2",
    /* eb */    "STPLS",
    /* ec */    "LDPLS",
    /* ed */    "CPPAL2",
    /* ee */    "RTPAL2",
    /* ef */    "ADPAL2",
    
    /* f0 */    "MUSIC",
    /* f1 */    "SOUND",
    /* f2 */    "AKAO",
    /* f3 */    "MUSVT",
    /* f4 */    "MUSVM",
    /* f5 */    "MULCK",
    /* f6 */    "BMUSC",
    /* f7 */    "CHMPH",
    /* f8 */    "PMVIE",
    /* f9 */    "MOVIE",
    /* fa */    "MVIEF",
    /* fb */    "MVCAM",
    /* fc */    "FMUSC",
    /* fd */    "CMUSC",
    /* fe */    "CHMST",
    /* ff */    "GAMEOVER",
    /* 100 */   "LABEL"
};
