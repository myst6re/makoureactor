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
#include "Script.h"

Script::Script() :
	valid(true)
{
}

Script::Script(const QList<Opcode *> &opcodes) :
	_opcodes(opcodes), valid(true)
{
}

Script::Script(const QByteArray &script)
{
	valid = openScript(script, 0, script.size());
}

Script::Script(const QByteArray &script, int pos, int size)
{
	valid = openScript(script, pos, size);
}

Script::Script(const Script &other) :
	lastError(other.lastError), valid(other.valid)
{
	for (Opcode *opcode : other.opcodes()) {
		_opcodes.append(Script::copyOpcode(opcode));
	}
}

Script::~Script()
{
	qDeleteAll(_opcodes);
}

bool Script::openScript(const QByteArray &script, const int initPos, const int size)
{
	int pos = 0, scriptSize = qMin(script.size() - initPos, size), opcodeID=1;
	QList<int> positions;
	QMultiMap<int, OpcodeJump *> indents;

	while (pos < scriptSize) {
		Opcode *op = createOpcode(script, initPos + pos);
		_opcodes.append(op);
		positions.append(pos);
		if (op->isJump()) {
			indents.insert(pos + static_cast<OpcodeJump *>(op)->jump(), static_cast<OpcodeJump *>(op));
//			qDebug() << opcodeID << "jump to" << (pos + ((OpcodeJump *)op)->jump());
		}
		pos += op->size();
		++opcodeID;
	}
	positions.append(pos);

	QList<int> indentsKeys = indents.uniqueKeys();

	for (int i=indentsKeys.size()-1; i >= 0; --i) {
		int jump = indentsKeys.at(i);
		int index = positions.indexOf(jump);
		if (index != -1) {
			_opcodes.insert(index, new OpcodeLabel(i+1));
//			qDebug() << (index+1) << "label" << (i+1);
			for (OpcodeJump *opJump : indents.values(jump)) {
				opJump->setLabel(i+1);
			}
		} else {
			qWarning() << "Error" << jump << "label" << (i+1);
			for (OpcodeJump *opJump : indents.values(jump)) {
				opJump->setBadJump(true);
			}
//			return false;
			/*int opID=0;
			bool repaired=false;
			for (const int &pos : positions) {
				if (pos > jump) {
					// Repair bad jump: Insert label to the nearest valid position
					_opcodes.insert(opID, new OpcodeLabel(i+1));
//					qDebug() << (opID+1) << "repair label" << (i+1);
					repaired = true;
					break;
				}
				++opID;
			}
			if (!repaired) {
				// Repair bad jump: Put label at the end
				_opcodes.append(new OpcodeLabel(i+1));
//				qDebug() << "repair label" << (i+1);
			}*/
		}
	}

	return true;
}

Opcode *Script::createOpcode(const QByteArray &script, int pos)
{
	quint8 opcode = (quint8)script.at(pos);
	quint8 size = Opcode::length[opcode] - 1; // length of arguments
	const char *data = script.constData() + pos + 1;

	if (script.size() < pos + 1 + size) {
		qWarning() << "unknown opcode" << opcode << script.size() << pos << size << (pos + 1 + size);
		return new OpcodeUnknown(opcode, script.mid(pos + 1));
	}

	switch(opcode)
	{
	case 0x00:	return new OpcodeRET();
	case 0x01:	return new OpcodeREQ(data, size);
	case 0x02:	return new OpcodeREQSW(data, size);
	case 0x03:	return new OpcodeREQEW(data, size);
	case 0x04:	return new OpcodePREQ(data, size);
	case 0x05:	return new OpcodePRQSW(data, size);
	case 0x06:	return new OpcodePRQEW(data, size);
	case 0x07:	return new OpcodeRETTO(data, size);
	case 0x08:	return new OpcodeJOIN(data, size);
	case 0x09:	return new OpcodeSPLIT(data, size);
	case 0x0A:	return new OpcodeSPTYE(data, size);
	case 0x0B:	return new OpcodeGTPYE(data, size);
	case 0x0E:	return new OpcodeDSKCG(data, size);
	case 0x0F://SPECIAL
		switch((quint8)script.at(pos+1)) {
		case 0xF5:case 0xF6:case 0xF7:case 0xFB:case 0xFC:
			size += 1;
			break;
		case 0xF8:case 0xFD:
			size += 2;
			break;
		}
		if (script.size() < pos + 1 + size) {
			qWarning() << "unknown opcode SPECIAL" << opcode << (script.size() - pos - 1);
			return new OpcodeUnknown(opcode, script.mid(pos + 1));
		}
		return new OpcodeSPECIAL(data, size);
	case 0x10:	return new OpcodeJMPF(data, size);
	case 0x11:	return new OpcodeJMPFL(data, size);
	case 0x12:	return new OpcodeJMPB(data, size);
	case 0x13:	return new OpcodeJMPBL(data, size);
	case 0x14:	return new OpcodeIFUB(data, size);
	case 0x15:	return new OpcodeIFUBL(data, size);
	case 0x16:	return new OpcodeIFSW(data, size);
	case 0x17:	return new OpcodeIFSWL(data, size);
	case 0x18:	return new OpcodeIFUW(data, size);
	case 0x19:	return new OpcodeIFUWL(data, size);
	case 0x1A:	return new Opcode1A(data, size);
	case 0x1B:	return new Opcode1B(data, size);
	case 0x20:	return new OpcodeMINIGAME(data, size);
	case 0x21:	return new OpcodeTUTOR(data, size);
	case 0x22:	return new OpcodeBTMD2(data, size);
	case 0x23:	return new OpcodeBTRLD(data, size);
	case 0x24:	return new OpcodeWAIT(data, size);
	case 0x25:	return new OpcodeNFADE(data, size);
	case 0x26:	return new OpcodeBLINK(data, size);
	case 0x27:	return new OpcodeBGMOVIE(data, size);
	case 0x28://KAWAI
		if (pos + 1 < script.size()) {
			if ((quint8)script.at(pos+1) == 0) {
				qWarning() << "unknown opcode KAWAI" << opcode << size << (script.size() - pos - 1);
				return new OpcodeUnknown(0x28, data, 1);
			} else {
				size = (quint8)script.at(pos+1) - 1;
				if (pos + 1 + size <= script.size()) {
					return new OpcodeKAWAI(data, size);
				}
			}
		}
		qWarning() << "unknown opcode KAWAI" << opcode << size << (script.size() - pos - 1);
		return new OpcodeUnknown(opcode, script.mid(pos + 1));
	case 0x29:	return new OpcodeKAWIW();
	case 0x2A:	return new OpcodePMOVA(data, size);
	case 0x2B:	return new OpcodeSLIP(data, size);
	case 0x2C:	return new OpcodeBGPDH(data, size);
	case 0x2D:	return new OpcodeBGSCR(data, size);
	case 0x2E:	return new OpcodeWCLS(data, size);
	case 0x2F:	return new OpcodeWSIZW(data, size);
	case 0x30:	return new OpcodeIFKEY(data, size);
	case 0x31:	return new OpcodeIFKEYON(data, size);
	case 0x32:	return new OpcodeIFKEYOFF(data, size);
	case 0x33:	return new OpcodeUC(data, size);
	case 0x34:	return new OpcodePDIRA(data, size);
	case 0x35:	return new OpcodePTURA(data, size);
	case 0x36:	return new OpcodeWSPCL(data, size);
	case 0x37:	return new OpcodeWNUMB(data, size);
	case 0x38:	return new OpcodeSTTIM(data, size);
	case 0x39:	return new OpcodeGOLDu(data, size);
	case 0x3A:	return new OpcodeGOLDd(data, size);
	case 0x3B:	return new OpcodeCHGLD(data, size);
	case 0x3C:	return new OpcodeHMPMAX1();
	case 0x3D:	return new OpcodeHMPMAX2();
	case 0x3E:	return new OpcodeMHMMX();
	case 0x3F:	return new OpcodeHMPMAX3();
	case 0x40:	return new OpcodeMESSAGE(data, size);
	case 0x41:	return new OpcodeMPARA(data, size);
	case 0x42:	return new OpcodeMPRA2(data, size);
	case 0x43:	return new OpcodeMPNAM(data, size);

	case 0x45:	return new OpcodeMPu(data, size);

	case 0x47:	return new OpcodeMPd(data, size);
	case 0x48:	return new OpcodeASK(data, size);
	case 0x49:	return new OpcodeMENU(data, size);
	case 0x4A:	return new OpcodeMENU2(data, size);
	case 0x4B:	return new OpcodeBTLTB(data, size);

	case 0x4D:	return new OpcodeHPu(data, size);

	case 0x4F:	return new OpcodeHPd(data, size);
	case 0x50:	return new OpcodeWINDOW(data, size);
	case 0x51:	return new OpcodeWMOVE(data, size);
	case 0x52:	return new OpcodeWMODE(data, size);
	case 0x53:	return new OpcodeWREST(data, size);
	case 0x54:	return new OpcodeWCLSE(data, size);
	case 0x55:	return new OpcodeWROW(data, size);
	case 0x56:	return new OpcodeGWCOL(data, size);
	case 0x57:	return new OpcodeSWCOL(data, size);
	case 0x58:	return new OpcodeSTITM(data, size);
	case 0x59:	return new OpcodeDLITM(data, size);
	case 0x5A:	return new OpcodeCKITM(data, size);
	case 0x5B:	return new OpcodeSMTRA(data, size);
	case 0x5C:	return new OpcodeDMTRA(data, size);
	case 0x5D:	return new OpcodeCMTRA(data, size);
	case 0x5E:	return new OpcodeSHAKE(data, size);
	case 0x5F:	return new OpcodeNOP();
	case 0x60:	return new OpcodeMAPJUMP(data, size);
	case 0x61:	return new OpcodeSCRLO(data, size);
	case 0x62:	return new OpcodeSCRLC(data, size);
	case 0x63:	return new OpcodeSCRLA(data, size);
	case 0x64:	return new OpcodeSCR2D(data, size);
	case 0x65:	return new OpcodeSCRCC();
	case 0x66:	return new OpcodeSCR2DC(data, size);
	case 0x67:	return new OpcodeSCRLW();
	case 0x68:	return new OpcodeSCR2DL(data, size);
	case 0x69:	return new OpcodeMPDSP(data, size);
	case 0x6A:	return new OpcodeVWOFT(data, size);
	case 0x6B:	return new OpcodeFADE(data, size);
	case 0x6C:	return new OpcodeFADEW();
	case 0x6D:	return new OpcodeIDLCK(data, size);
	case 0x6E:	return new OpcodeLSTMP(data, size);
	case 0x6F:	return new OpcodeSCRLP(data, size);
	case 0x70:	return new OpcodeBATTLE(data, size);
	case 0x71:	return new OpcodeBTLON(data, size);
	case 0x72:	return new OpcodeBTLMD(data, size);
	case 0x73:	return new OpcodePGTDR(data, size);
	case 0x74:	return new OpcodeGETPC(data, size);
	case 0x75:	return new OpcodePXYZI(data, size);
	case 0x76:	return new OpcodePLUSX(data, size);
	case 0x77:	return new OpcodePLUS2X(data, size);
	case 0x78:	return new OpcodeMINUSX(data, size);
	case 0x79:	return new OpcodeMINUS2X(data, size);
	case 0x7A:	return new OpcodeINCX(data, size);
	case 0x7B:	return new OpcodeINC2X(data, size);
	case 0x7C:	return new OpcodeDECX(data, size);
	case 0x7D:	return new OpcodeDEC2X(data, size);
	case 0x7E:	return new OpcodeTLKON(data, size);
	case 0x7F:	return new OpcodeRDMSD(data, size);
	case 0x80:	return new OpcodeSETBYTE(data, size);
	case 0x81:	return new OpcodeSETWORD(data, size);
	case 0x82:	return new OpcodeBITON(data, size);
	case 0x83:	return new OpcodeBITOFF(data, size);
	case 0x84:	return new OpcodeBITXOR(data, size);
	case 0x85:	return new OpcodePLUS(data, size);
	case 0x86:	return new OpcodePLUS2(data, size);
	case 0x87:	return new OpcodeMINUS(data, size);
	case 0x88:	return new OpcodeMINUS2(data, size);
	case 0x89:	return new OpcodeMUL(data, size);
	case 0x8A:	return new OpcodeMUL2(data, size);
	case 0x8B:	return new OpcodeDIV(data, size);
	case 0x8C:	return new OpcodeDIV2(data, size);
	case 0x8D:	return new OpcodeMOD(data, size);
	case 0x8E:	return new OpcodeMOD2(data, size);
	case 0x8F:	return new OpcodeAND(data, size);
	case 0x90:	return new OpcodeAND2(data, size);
	case 0x91:	return new OpcodeOR(data, size);
	case 0x92:	return new OpcodeOR2(data, size);
	case 0x93:	return new OpcodeXOR(data, size);
	case 0x94:	return new OpcodeXOR2(data, size);
	case 0x95:	return new OpcodeINC(data, size);
	case 0x96:	return new OpcodeINC2(data, size);
	case 0x97:	return new OpcodeDEC(data, size);
	case 0x98:	return new OpcodeDEC2(data, size);
	case 0x99:	return new OpcodeRANDOM(data, size);
	case 0x9A:	return new OpcodeLBYTE(data, size);
	case 0x9B:	return new OpcodeHBYTE(data, size);
	case 0x9C:	return new Opcode2BYTE(data, size);
	case 0x9D:	return new OpcodeSETX(data, size);
	case 0x9E:	return new OpcodeGETX(data, size);
	case 0x9F:	return new OpcodeSEARCHX(data, size);
	case 0xA0:	return new OpcodePC(data, size);
	case 0xA1:	return new OpcodeCHAR(data, size);
	case 0xA2:	return new OpcodeDFANM(data, size);
	case 0xA3:	return new OpcodeANIME1(data, size);
	case 0xA4:	return new OpcodeVISI(data, size);
	case 0xA5:	return new OpcodeXYZI(data, size);
	case 0xA6:	return new OpcodeXYI(data, size);
	case 0xA7:	return new OpcodeXYZ(data, size);
	case 0xA8:	return new OpcodeMOVE(data, size);
	case 0xA9:	return new OpcodeCMOVE(data, size);
	case 0xAA:	return new OpcodeMOVA(data, size);
	case 0xAB:	return new OpcodeTURA(data, size);
	case 0xAC:	return new OpcodeANIMW();
	case 0xAD:	return new OpcodeFMOVE(data, size);
	case 0xAE:	return new OpcodeANIME2(data, size);
	case 0xAF:	return new OpcodeANIMX1(data, size);
	case 0xB0:	return new OpcodeCANIM1(data, size);
	case 0xB1:	return new OpcodeCANMX1(data, size);
	case 0xB2:	return new OpcodeMSPED(data, size);
	case 0xB3:	return new OpcodeDIR(data, size);
	case 0xB4:	return new OpcodeTURNGEN(data, size);
	case 0xB5:	return new OpcodeTURN(data, size);
	case 0xB6:	return new OpcodeDIRA(data, size);
	case 0xB7:	return new OpcodeGETDIR(data, size);
	case 0xB8:	return new OpcodeGETAXY(data, size);
	case 0xB9:	return new OpcodeGETAI(data, size);
	case 0xBA:	return new OpcodeANIMX2(data, size);
	case 0xBB:	return new OpcodeCANIM2(data, size);
	case 0xBC:	return new OpcodeCANMX2(data, size);
	case 0xBD:	return new OpcodeASPED(data, size);

	case 0xBF:	return new OpcodeCC(data, size);
	case 0xC0:	return new OpcodeJUMP(data, size);
	case 0xC1:	return new OpcodeAXYZI(data, size);
	case 0xC2:	return new OpcodeLADER(data, size);
	case 0xC3:	return new OpcodeOFST(data, size);
	case 0xC4:	return new OpcodeOFSTW();
	case 0xC5:	return new OpcodeTALKR(data, size);
	case 0xC6:	return new OpcodeSLIDR(data, size);
	case 0xC7:	return new OpcodeSOLID(data, size);
	case 0xC8:	return new OpcodePRTYP(data, size);
	case 0xC9:	return new OpcodePRTYM(data, size);
	case 0xCA:	return new OpcodePRTYE(data, size);
	case 0xCB:	return new OpcodeIFPRTYQ(data, size);
	case 0xCC:	return new OpcodeIFMEMBQ(data, size);
	case 0xCD:	return new OpcodeMMBUD(data, size);
	case 0xCE:	return new OpcodeMMBLK(data, size);
	case 0xCF:	return new OpcodeMMBUK(data, size);
	case 0xD0:	return new OpcodeLINE(data, size);
	case 0xD1:	return new OpcodeLINON(data, size);
	case 0xD2:	return new OpcodeMPJPO(data, size);
	case 0xD3:	return new OpcodeSLINE(data, size);
	case 0xD4:	return new OpcodeSIN(data, size);
	case 0xD5:	return new OpcodeCOS(data, size);
	case 0xD6:	return new OpcodeTLKR2(data, size);
	case 0xD7:	return new OpcodeSLDR2(data, size);
	case 0xD8:	return new OpcodePMJMP(data, size);
	case 0xD9:	return new OpcodePMJMP2();
	case 0xDA:	return new OpcodeAKAO2(data, size);
	case 0xDB:	return new OpcodeFCFIX(data, size);
	case 0xDC:	return new OpcodeCCANM(data, size);
	case 0xDD:	return new OpcodeANIMB();
	case 0xDE:	return new OpcodeTURNW();
	case 0xDF:	return new OpcodeMPPAL(data, size);
	case 0xE0:	return new OpcodeBGON(data, size);
	case 0xE1:	return new OpcodeBGOFF(data, size);
	case 0xE2:	return new OpcodeBGROL(data, size);
	case 0xE3:	return new OpcodeBGROL2(data, size);
	case 0xE4:	return new OpcodeBGCLR(data, size);
	case 0xE5:	return new OpcodeSTPAL(data, size);
	case 0xE6:	return new OpcodeLDPAL(data, size);
	case 0xE7:	return new OpcodeCPPAL(data, size);
	case 0xE8:	return new OpcodeRTPAL(data, size);
	case 0xE9:	return new OpcodeADPAL(data, size);
	case 0xEA:	return new OpcodeMPPAL2(data, size);
	case 0xEB:	return new OpcodeSTPLS(data, size);
	case 0xEC:	return new OpcodeLDPLS(data, size);
	case 0xED:	return new OpcodeCPPAL2(data, size);
	case 0xEE:	return new OpcodeRTPAL2(data, size);
	case 0xEF:	return new OpcodeADPAL2(data, size);
	case 0xF0:	return new OpcodeMUSIC(data, size);
	case 0xF1:	return new OpcodeSOUND(data, size);
	case 0xF2:	return new OpcodeAKAO(data, size);
	case 0xF3:	return new OpcodeMUSVT(data, size);
	case 0xF4:	return new OpcodeMUSVM(data, size);
	case 0xF5:	return new OpcodeMULCK(data, size);
	case 0xF6:	return new OpcodeBMUSC(data, size);
	case 0xF7:	return new OpcodeCHMPH(data, size);
	case 0xF8:	return new OpcodePMVIE(data, size);
	case 0xF9:	return new OpcodeMOVIE();
	case 0xFA:	return new OpcodeMVIEF(data, size);
	case 0xFB:	return new OpcodeMVCAM(data, size);
	case 0xFC:	return new OpcodeFMUSC(data, size);
	case 0xFD:	return new OpcodeCMUSC(data, size);
	case 0xFE:	return new OpcodeCHMST(data, size);
	case 0xFF:	return new OpcodeGAMEOVER();
	default:
		qWarning() << "unknown opcode" << opcode << (script.size() - pos - 1);
		return new OpcodeUnknown(opcode, script.mid(pos + 1, 1));
	}
}

Opcode *Script::copyOpcode(Opcode *opcode)
{
	switch(opcode->id())
	{
	case 0x00:	return new OpcodeRET(*static_cast<OpcodeRET *>(opcode));
	case 0x01:	return new OpcodeREQ(*static_cast<OpcodeREQ *>(opcode));
	case 0x02:	return new OpcodeREQSW(*static_cast<OpcodeREQSW *>(opcode));
	case 0x03:	return new OpcodeREQEW(*static_cast<OpcodeREQEW *>(opcode));
	case 0x04:	return new OpcodePREQ(*static_cast<OpcodePREQ *>(opcode));
	case 0x05:	return new OpcodePRQSW(*static_cast<OpcodePRQSW *>(opcode));
	case 0x06:	return new OpcodePRQEW(*static_cast<OpcodePRQEW *>(opcode));
	case 0x07:	return new OpcodeRETTO(*static_cast<OpcodeRETTO *>(opcode));
	case 0x08:	return new OpcodeJOIN(*static_cast<OpcodeJOIN *>(opcode));
	case 0x09:	return new OpcodeSPLIT(*static_cast<OpcodeSPLIT *>(opcode));
	case 0x0A:	return new OpcodeSPTYE(*static_cast<OpcodeSPTYE *>(opcode));
	case 0x0B:	return new OpcodeGTPYE(*static_cast<OpcodeGTPYE *>(opcode));
	case 0x0E:	return new OpcodeDSKCG(*static_cast<OpcodeDSKCG *>(opcode));
	case 0x0F:	return new OpcodeSPECIAL(*static_cast<OpcodeSPECIAL *>(opcode));
	case 0x10:	return new OpcodeJMPF(*static_cast<OpcodeJMPF *>(opcode));
	case 0x11:	return new OpcodeJMPFL(*static_cast<OpcodeJMPFL *>(opcode));
	case 0x12:	return new OpcodeJMPB(*static_cast<OpcodeJMPB *>(opcode));
	case 0x13:	return new OpcodeJMPBL(*static_cast<OpcodeJMPBL *>(opcode));
	case 0x14:	return new OpcodeIFUB(*static_cast<OpcodeIFUB *>(opcode));
	case 0x15:	return new OpcodeIFUBL(*static_cast<OpcodeIFUBL *>(opcode));
	case 0x16:	return new OpcodeIFSW(*static_cast<OpcodeIFSW *>(opcode));
	case 0x17:	return new OpcodeIFSWL(*static_cast<OpcodeIFSWL *>(opcode));
	case 0x18:	return new OpcodeIFUW(*static_cast<OpcodeIFUW *>(opcode));
	case 0x19:	return new OpcodeIFUWL(*static_cast<OpcodeIFUWL *>(opcode));
	case 0x1A:	return new Opcode1A(*static_cast<Opcode1A *>(opcode));
	case 0x1B:	return new Opcode1B(*static_cast<Opcode1B *>(opcode));
	case 0x20:	return new OpcodeMINIGAME(*static_cast<OpcodeMINIGAME *>(opcode));
	case 0x21:	return new OpcodeTUTOR(*static_cast<OpcodeTUTOR *>(opcode));
	case 0x22:	return new OpcodeBTMD2(*static_cast<OpcodeBTMD2 *>(opcode));
	case 0x23:	return new OpcodeBTRLD(*static_cast<OpcodeBTRLD *>(opcode));
	case 0x24:	return new OpcodeWAIT(*static_cast<OpcodeWAIT *>(opcode));
	case 0x25:	return new OpcodeNFADE(*static_cast<OpcodeNFADE *>(opcode));
	case 0x26:	return new OpcodeBLINK(*static_cast<OpcodeBLINK *>(opcode));
	case 0x27:	return new OpcodeBGMOVIE(*static_cast<OpcodeBGMOVIE *>(opcode));
	case 0x28:	return new OpcodeKAWAI(*static_cast<OpcodeKAWAI *>(opcode));
	case 0x29:	return new OpcodeKAWIW(*static_cast<OpcodeKAWIW *>(opcode));
	case 0x2A:	return new OpcodePMOVA(*static_cast<OpcodePMOVA *>(opcode));
	case 0x2B:	return new OpcodeSLIP(*static_cast<OpcodeSLIP *>(opcode));
	case 0x2C:	return new OpcodeBGPDH(*static_cast<OpcodeBGPDH *>(opcode));
	case 0x2D:	return new OpcodeBGSCR(*static_cast<OpcodeBGSCR *>(opcode));
	case 0x2E:	return new OpcodeWCLS(*static_cast<OpcodeWCLS *>(opcode));
	case 0x2F:	return new OpcodeWSIZW(*static_cast<OpcodeWSIZW *>(opcode));
	case 0x30:	return new OpcodeIFKEY(*static_cast<OpcodeIFKEY *>(opcode));
	case 0x31:	return new OpcodeIFKEYON(*static_cast<OpcodeIFKEYON *>(opcode));
	case 0x32:	return new OpcodeIFKEYOFF(*static_cast<OpcodeIFKEYOFF *>(opcode));
	case 0x33:	return new OpcodeUC(*static_cast<OpcodeUC *>(opcode));
	case 0x34:	return new OpcodePDIRA(*static_cast<OpcodePDIRA *>(opcode));
	case 0x35:	return new OpcodePTURA(*static_cast<OpcodePTURA *>(opcode));
	case 0x36:	return new OpcodeWSPCL(*static_cast<OpcodeWSPCL *>(opcode));
	case 0x37:	return new OpcodeWNUMB(*static_cast<OpcodeWNUMB *>(opcode));
	case 0x38:	return new OpcodeSTTIM(*static_cast<OpcodeSTTIM *>(opcode));
	case 0x39:	return new OpcodeGOLDu(*static_cast<OpcodeGOLDu *>(opcode));
	case 0x3A:	return new OpcodeGOLDd(*static_cast<OpcodeGOLDd *>(opcode));
	case 0x3B:	return new OpcodeCHGLD(*static_cast<OpcodeCHGLD *>(opcode));
	case 0x3C:	return new OpcodeHMPMAX1(*static_cast<OpcodeHMPMAX1 *>(opcode));
	case 0x3D:	return new OpcodeHMPMAX2(*static_cast<OpcodeHMPMAX2 *>(opcode));
	case 0x3E:	return new OpcodeMHMMX(*static_cast<OpcodeMHMMX *>(opcode));
	case 0x3F:	return new OpcodeHMPMAX3(*static_cast<OpcodeHMPMAX3 *>(opcode));
	case 0x40:	return new OpcodeMESSAGE(*static_cast<OpcodeMESSAGE *>(opcode));
	case 0x41:	return new OpcodeMPARA(*static_cast<OpcodeMPARA *>(opcode));
	case 0x42:	return new OpcodeMPRA2(*static_cast<OpcodeMPRA2 *>(opcode));
	case 0x43:	return new OpcodeMPNAM(*static_cast<OpcodeMPNAM *>(opcode));

	case 0x45:	return new OpcodeMPu(*static_cast<OpcodeMPu *>(opcode));

	case 0x47:	return new OpcodeMPd(*static_cast<OpcodeMPd *>(opcode));
	case 0x48:	return new OpcodeASK(*static_cast<OpcodeASK *>(opcode));
	case 0x49:	return new OpcodeMENU(*static_cast<OpcodeMENU *>(opcode));
	case 0x4A:	return new OpcodeMENU2(*static_cast<OpcodeMENU2 *>(opcode));
	case 0x4B:	return new OpcodeBTLTB(*static_cast<OpcodeBTLTB *>(opcode));

	case 0x4D:	return new OpcodeHPu(*static_cast<OpcodeHPu *>(opcode));

	case 0x4F:	return new OpcodeHPd(*static_cast<OpcodeHPd *>(opcode));
	case 0x50:	return new OpcodeWINDOW(*static_cast<OpcodeWINDOW *>(opcode));
	case 0x51:	return new OpcodeWMOVE(*static_cast<OpcodeWMOVE *>(opcode));
	case 0x52:	return new OpcodeWMODE(*static_cast<OpcodeWMODE *>(opcode));
	case 0x53:	return new OpcodeWREST(*static_cast<OpcodeWREST *>(opcode));
	case 0x54:	return new OpcodeWCLSE(*static_cast<OpcodeWCLSE *>(opcode));
	case 0x55:	return new OpcodeWROW(*static_cast<OpcodeWROW *>(opcode));
	case 0x56:	return new OpcodeGWCOL(*static_cast<OpcodeGWCOL *>(opcode));
	case 0x57:	return new OpcodeSWCOL(*static_cast<OpcodeSWCOL *>(opcode));
	case 0x58:	return new OpcodeSTITM(*static_cast<OpcodeSTITM *>(opcode));
	case 0x59:	return new OpcodeDLITM(*static_cast<OpcodeDLITM *>(opcode));
	case 0x5A:	return new OpcodeCKITM(*static_cast<OpcodeCKITM *>(opcode));
	case 0x5B:	return new OpcodeSMTRA(*static_cast<OpcodeSMTRA *>(opcode));
	case 0x5C:	return new OpcodeDMTRA(*static_cast<OpcodeDMTRA *>(opcode));
	case 0x5D:	return new OpcodeCMTRA(*static_cast<OpcodeCMTRA *>(opcode));
	case 0x5E:	return new OpcodeSHAKE(*static_cast<OpcodeSHAKE *>(opcode));
	case 0x5F:	return new OpcodeNOP(*static_cast<OpcodeNOP *>(opcode));
	case 0x60:	return new OpcodeMAPJUMP(*static_cast<OpcodeMAPJUMP *>(opcode));
	case 0x61:	return new OpcodeSCRLO(*static_cast<OpcodeSCRLO *>(opcode));
	case 0x62:	return new OpcodeSCRLC(*static_cast<OpcodeSCRLC *>(opcode));
	case 0x63:	return new OpcodeSCRLA(*static_cast<OpcodeSCRLA *>(opcode));
	case 0x64:	return new OpcodeSCR2D(*static_cast<OpcodeSCR2D *>(opcode));
	case 0x65:	return new OpcodeSCRCC(*static_cast<OpcodeSCRCC *>(opcode));
	case 0x66:	return new OpcodeSCR2DC(*static_cast<OpcodeSCR2DC *>(opcode));
	case 0x67:	return new OpcodeSCRLW(*static_cast<OpcodeSCRLW *>(opcode));
	case 0x68:	return new OpcodeSCR2DL(*static_cast<OpcodeSCR2DL *>(opcode));
	case 0x69:	return new OpcodeMPDSP(*static_cast<OpcodeMPDSP *>(opcode));
	case 0x6A:	return new OpcodeVWOFT(*static_cast<OpcodeVWOFT *>(opcode));
	case 0x6B:	return new OpcodeFADE(*static_cast<OpcodeFADE *>(opcode));
	case 0x6C:	return new OpcodeFADEW(*static_cast<OpcodeFADEW *>(opcode));
	case 0x6D:	return new OpcodeIDLCK(*static_cast<OpcodeIDLCK *>(opcode));
	case 0x6E:	return new OpcodeLSTMP(*static_cast<OpcodeLSTMP *>(opcode));
	case 0x6F:	return new OpcodeSCRLP(*static_cast<OpcodeSCRLP *>(opcode));
	case 0x70:	return new OpcodeBATTLE(*static_cast<OpcodeBATTLE *>(opcode));
	case 0x71:	return new OpcodeBTLON(*static_cast<OpcodeBTLON *>(opcode));
	case 0x72:	return new OpcodeBTLMD(*static_cast<OpcodeBTLMD *>(opcode));
	case 0x73:	return new OpcodePGTDR(*static_cast<OpcodePGTDR *>(opcode));
	case 0x74:	return new OpcodeGETPC(*static_cast<OpcodeGETPC *>(opcode));
	case 0x75:	return new OpcodePXYZI(*static_cast<OpcodePXYZI *>(opcode));
	case 0x76:	return new OpcodePLUSX(*static_cast<OpcodePLUSX *>(opcode));
	case 0x77:	return new OpcodePLUS2X(*static_cast<OpcodePLUS2X *>(opcode));
	case 0x78:	return new OpcodeMINUSX(*static_cast<OpcodeMINUSX *>(opcode));
	case 0x79:	return new OpcodeMINUS2X(*static_cast<OpcodeMINUS2X *>(opcode));
	case 0x7A:	return new OpcodeINCX(*static_cast<OpcodeINCX *>(opcode));
	case 0x7B:	return new OpcodeINC2X(*static_cast<OpcodeINC2X *>(opcode));
	case 0x7C:	return new OpcodeDECX(*static_cast<OpcodeDECX *>(opcode));
	case 0x7D:	return new OpcodeDEC2X(*static_cast<OpcodeDEC2X *>(opcode));
	case 0x7E:	return new OpcodeTLKON(*static_cast<OpcodeTLKON *>(opcode));
	case 0x7F:	return new OpcodeRDMSD(*static_cast<OpcodeRDMSD *>(opcode));
	case 0x80:	return new OpcodeSETBYTE(*static_cast<OpcodeSETBYTE *>(opcode));
	case 0x81:	return new OpcodeSETWORD(*static_cast<OpcodeSETWORD *>(opcode));
	case 0x82:	return new OpcodeBITON(*static_cast<OpcodeBITON *>(opcode));
	case 0x83:	return new OpcodeBITOFF(*static_cast<OpcodeBITOFF *>(opcode));
	case 0x84:	return new OpcodeBITXOR(*static_cast<OpcodeBITXOR *>(opcode));
	case 0x85:	return new OpcodePLUS(*static_cast<OpcodePLUS *>(opcode));
	case 0x86:	return new OpcodePLUS2(*static_cast<OpcodePLUS2 *>(opcode));
	case 0x87:	return new OpcodeMINUS(*static_cast<OpcodeMINUS *>(opcode));
	case 0x88:	return new OpcodeMINUS2(*static_cast<OpcodeMINUS2 *>(opcode));
	case 0x89:	return new OpcodeMUL(*static_cast<OpcodeMUL *>(opcode));
	case 0x8A:	return new OpcodeMUL2(*static_cast<OpcodeMUL2 *>(opcode));
	case 0x8B:	return new OpcodeDIV(*static_cast<OpcodeDIV *>(opcode));
	case 0x8C:	return new OpcodeDIV2(*static_cast<OpcodeDIV2 *>(opcode));
	case 0x8D:	return new OpcodeMOD(*static_cast<OpcodeMOD *>(opcode));
	case 0x8E:	return new OpcodeMOD2(*static_cast<OpcodeMOD2 *>(opcode));
	case 0x8F:	return new OpcodeAND(*static_cast<OpcodeAND *>(opcode));
	case 0x90:	return new OpcodeAND2(*static_cast<OpcodeAND2 *>(opcode));
	case 0x91:	return new OpcodeOR(*static_cast<OpcodeOR *>(opcode));
	case 0x92:	return new OpcodeOR2(*static_cast<OpcodeOR2 *>(opcode));
	case 0x93:	return new OpcodeXOR(*static_cast<OpcodeXOR *>(opcode));
	case 0x94:	return new OpcodeXOR2(*static_cast<OpcodeXOR2 *>(opcode));
	case 0x95:	return new OpcodeINC(*static_cast<OpcodeINC *>(opcode));
	case 0x96:	return new OpcodeINC2(*static_cast<OpcodeINC2 *>(opcode));
	case 0x97:	return new OpcodeDEC(*static_cast<OpcodeDEC *>(opcode));
	case 0x98:	return new OpcodeDEC2(*static_cast<OpcodeDEC2 *>(opcode));
	case 0x99:	return new OpcodeRANDOM(*static_cast<OpcodeRANDOM *>(opcode));
	case 0x9A:	return new OpcodeLBYTE(*static_cast<OpcodeLBYTE *>(opcode));
	case 0x9B:	return new OpcodeHBYTE(*static_cast<OpcodeHBYTE *>(opcode));
	case 0x9C:	return new Opcode2BYTE(*static_cast<Opcode2BYTE *>(opcode));
	case 0x9D:	return new OpcodeSETX(*static_cast<OpcodeSETX *>(opcode));
	case 0x9E:	return new OpcodeGETX(*static_cast<OpcodeGETX *>(opcode));
	case 0x9F:	return new OpcodeSEARCHX(*static_cast<OpcodeSEARCHX *>(opcode));
	case 0xA0:	return new OpcodePC(*static_cast<OpcodePC *>(opcode));
	case 0xA1:	return new OpcodeCHAR(*static_cast<OpcodeCHAR *>(opcode));
	case 0xA2:	return new OpcodeDFANM(*static_cast<OpcodeDFANM *>(opcode));
	case 0xA3:	return new OpcodeANIME1(*static_cast<OpcodeANIME1 *>(opcode));
	case 0xA4:	return new OpcodeVISI(*static_cast<OpcodeVISI *>(opcode));
	case 0xA5:	return new OpcodeXYZI(*static_cast<OpcodeXYZI *>(opcode));
	case 0xA6:	return new OpcodeXYI(*static_cast<OpcodeXYI *>(opcode));
	case 0xA7:	return new OpcodeXYZ(*static_cast<OpcodeXYZ *>(opcode));
	case 0xA8:	return new OpcodeMOVE(*static_cast<OpcodeMOVE *>(opcode));
	case 0xA9:	return new OpcodeCMOVE(*static_cast<OpcodeCMOVE *>(opcode));
	case 0xAA:	return new OpcodeMOVA(*static_cast<OpcodeMOVA *>(opcode));
	case 0xAB:	return new OpcodeTURA(*static_cast<OpcodeTURA *>(opcode));
	case 0xAC:	return new OpcodeANIMW(*static_cast<OpcodeANIMW *>(opcode));
	case 0xAD:	return new OpcodeFMOVE(*static_cast<OpcodeFMOVE *>(opcode));
	case 0xAE:	return new OpcodeANIME2(*static_cast<OpcodeANIME2 *>(opcode));
	case 0xAF:	return new OpcodeANIMX1(*static_cast<OpcodeANIMX1 *>(opcode));
	case 0xB0:	return new OpcodeCANIM1(*static_cast<OpcodeCANIM1 *>(opcode));
	case 0xB1:	return new OpcodeCANMX1(*static_cast<OpcodeCANMX1 *>(opcode));
	case 0xB2:	return new OpcodeMSPED(*static_cast<OpcodeMSPED *>(opcode));
	case 0xB3:	return new OpcodeDIR(*static_cast<OpcodeDIR *>(opcode));
	case 0xB4:	return new OpcodeTURNGEN(*static_cast<OpcodeTURNGEN *>(opcode));
	case 0xB5:	return new OpcodeTURN(*static_cast<OpcodeTURN *>(opcode));
	case 0xB6:	return new OpcodeDIRA(*static_cast<OpcodeDIRA *>(opcode));
	case 0xB7:	return new OpcodeGETDIR(*static_cast<OpcodeGETDIR *>(opcode));
	case 0xB8:	return new OpcodeGETAXY(*static_cast<OpcodeGETAXY *>(opcode));
	case 0xB9:	return new OpcodeGETAI(*static_cast<OpcodeGETAI *>(opcode));
	case 0xBA:	return new OpcodeANIMX2(*static_cast<OpcodeANIMX2 *>(opcode));
	case 0xBB:	return new OpcodeCANIM2(*static_cast<OpcodeCANIM2 *>(opcode));
	case 0xBC:	return new OpcodeCANMX2(*static_cast<OpcodeCANMX2 *>(opcode));
	case 0xBD:	return new OpcodeASPED(*static_cast<OpcodeASPED *>(opcode));

	case 0xBF:	return new OpcodeCC(*static_cast<OpcodeCC *>(opcode));
	case 0xC0:	return new OpcodeJUMP(*static_cast<OpcodeJUMP *>(opcode));
	case 0xC1:	return new OpcodeAXYZI(*static_cast<OpcodeAXYZI *>(opcode));
	case 0xC2:	return new OpcodeLADER(*static_cast<OpcodeLADER *>(opcode));
	case 0xC3:	return new OpcodeOFST(*static_cast<OpcodeOFST *>(opcode));
	case 0xC4:	return new OpcodeOFSTW(*static_cast<OpcodeOFSTW *>(opcode));
	case 0xC5:	return new OpcodeTALKR(*static_cast<OpcodeTALKR *>(opcode));
	case 0xC6:	return new OpcodeSLIDR(*static_cast<OpcodeSLIDR *>(opcode));
	case 0xC7:	return new OpcodeSOLID(*static_cast<OpcodeSOLID *>(opcode));
	case 0xC8:	return new OpcodePRTYP(*static_cast<OpcodePRTYP *>(opcode));
	case 0xC9:	return new OpcodePRTYM(*static_cast<OpcodePRTYM *>(opcode));
	case 0xCA:	return new OpcodePRTYE(*static_cast<OpcodePRTYE *>(opcode));
	case 0xCB:	return new OpcodeIFPRTYQ(*static_cast<OpcodeIFPRTYQ *>(opcode));
	case 0xCC:	return new OpcodeIFMEMBQ(*static_cast<OpcodeIFMEMBQ *>(opcode));
	case 0xCD:	return new OpcodeMMBUD(*static_cast<OpcodeMMBUD *>(opcode));
	case 0xCE:	return new OpcodeMMBLK(*static_cast<OpcodeMMBLK *>(opcode));
	case 0xCF:	return new OpcodeMMBUK(*static_cast<OpcodeMMBUK *>(opcode));
	case 0xD0:	return new OpcodeLINE(*static_cast<OpcodeLINE *>(opcode));
	case 0xD1:	return new OpcodeLINON(*static_cast<OpcodeLINON *>(opcode));
	case 0xD2:	return new OpcodeMPJPO(*static_cast<OpcodeMPJPO *>(opcode));
	case 0xD3:	return new OpcodeSLINE(*static_cast<OpcodeSLINE *>(opcode));
	case 0xD4:	return new OpcodeSIN(*static_cast<OpcodeSIN *>(opcode));
	case 0xD5:	return new OpcodeCOS(*static_cast<OpcodeCOS *>(opcode));
	case 0xD6:	return new OpcodeTLKR2(*static_cast<OpcodeTLKR2 *>(opcode));
	case 0xD7:	return new OpcodeSLDR2(*static_cast<OpcodeSLDR2 *>(opcode));
	case 0xD8:	return new OpcodePMJMP(*static_cast<OpcodePMJMP *>(opcode));
	case 0xD9:	return new OpcodePMJMP2(*static_cast<OpcodePMJMP2 *>(opcode));
	case 0xDA:	return new OpcodeAKAO2(*static_cast<OpcodeAKAO2 *>(opcode));
	case 0xDB:	return new OpcodeFCFIX(*static_cast<OpcodeFCFIX *>(opcode));
	case 0xDC:	return new OpcodeCCANM(*static_cast<OpcodeCCANM *>(opcode));
	case 0xDD:	return new OpcodeANIMB(*static_cast<OpcodeANIMB *>(opcode));
	case 0xDE:	return new OpcodeTURNW(*static_cast<OpcodeTURNW *>(opcode));
	case 0xDF:	return new OpcodeMPPAL(*static_cast<OpcodeMPPAL *>(opcode));
	case 0xE0:	return new OpcodeBGON(*static_cast<OpcodeBGON *>(opcode));
	case 0xE1:	return new OpcodeBGOFF(*static_cast<OpcodeBGOFF *>(opcode));
	case 0xE2:	return new OpcodeBGROL(*static_cast<OpcodeBGROL *>(opcode));
	case 0xE3:	return new OpcodeBGROL2(*static_cast<OpcodeBGROL2 *>(opcode));
	case 0xE4:	return new OpcodeBGCLR(*static_cast<OpcodeBGCLR *>(opcode));
	case 0xE5:	return new OpcodeSTPAL(*static_cast<OpcodeSTPAL *>(opcode));
	case 0xE6:	return new OpcodeLDPAL(*static_cast<OpcodeLDPAL *>(opcode));
	case 0xE7:	return new OpcodeCPPAL(*static_cast<OpcodeCPPAL *>(opcode));
	case 0xE8:	return new OpcodeRTPAL(*static_cast<OpcodeRTPAL *>(opcode));
	case 0xE9:	return new OpcodeADPAL(*static_cast<OpcodeADPAL *>(opcode));
	case 0xEA:	return new OpcodeMPPAL2(*static_cast<OpcodeMPPAL2 *>(opcode));
	case 0xEB:	return new OpcodeSTPLS(*static_cast<OpcodeSTPLS *>(opcode));
	case 0xEC:	return new OpcodeLDPLS(*static_cast<OpcodeLDPLS *>(opcode));
	case 0xED:	return new OpcodeCPPAL2(*static_cast<OpcodeCPPAL2 *>(opcode));
	case 0xEE:	return new OpcodeRTPAL2(*static_cast<OpcodeRTPAL2 *>(opcode));
	case 0xEF:	return new OpcodeADPAL2(*static_cast<OpcodeADPAL2 *>(opcode));
	case 0xF0:	return new OpcodeMUSIC(*static_cast<OpcodeMUSIC *>(opcode));
	case 0xF1:	return new OpcodeSOUND(*static_cast<OpcodeSOUND *>(opcode));
	case 0xF2:	return new OpcodeAKAO(*static_cast<OpcodeAKAO *>(opcode));
	case 0xF3:	return new OpcodeMUSVT(*static_cast<OpcodeMUSVT *>(opcode));
	case 0xF4:	return new OpcodeMUSVM(*static_cast<OpcodeMUSVM *>(opcode));
	case 0xF5:	return new OpcodeMULCK(*static_cast<OpcodeMULCK *>(opcode));
	case 0xF6:	return new OpcodeBMUSC(*static_cast<OpcodeBMUSC *>(opcode));
	case 0xF7:	return new OpcodeCHMPH(*static_cast<OpcodeCHMPH *>(opcode));
	case 0xF8:	return new OpcodePMVIE(*static_cast<OpcodePMVIE *>(opcode));
	case 0xF9:	return new OpcodeMOVIE(*static_cast<OpcodeMOVIE *>(opcode));
	case 0xFA:	return new OpcodeMVIEF(*static_cast<OpcodeMVIEF *>(opcode));
	case 0xFB:	return new OpcodeMVCAM(*static_cast<OpcodeMVCAM *>(opcode));
	case 0xFC:	return new OpcodeFMUSC(*static_cast<OpcodeFMUSC *>(opcode));
	case 0xFD:	return new OpcodeCMUSC(*static_cast<OpcodeCMUSC *>(opcode));
	case 0xFE:	return new OpcodeCHMST(*static_cast<OpcodeCHMST *>(opcode));
	case 0xFF:	return new OpcodeGAMEOVER(*static_cast<OpcodeGAMEOVER *>(opcode));
	case 0x100:	return new OpcodeLabel(*static_cast<OpcodeLabel *>(opcode));
	default:	return new OpcodeUnknown(*static_cast<OpcodeUnknown *>(opcode));
	}
}

Script *Script::splitScriptAtReturn()
{
	int gotoLabel = -1;
	int opcodeID = 0;

	for (Opcode *opcode : _opcodes) {
		if (opcode->isLabel()) {
			if (gotoLabel != -1 && static_cast<OpcodeLabel *>(opcode)->label() == (quint32)gotoLabel) {
				gotoLabel = -1;
			}
		} else if (gotoLabel == -1) {
			if (opcode->isJump() && !static_cast<OpcodeJump *>(opcode)->isBackJump()) {
				gotoLabel = static_cast<OpcodeJump *>(opcode)->label();
			} else if (opcode->id() == Opcode::RET || opcode->id() == Opcode::RETTO) {
				++opcodeID;
				break;
			}
		}
		++opcodeID;
	}

	Script *s = new Script(_opcodes.mid(opcodeID));
	int size = _opcodes.size();
	for ( ; opcodeID < size; ++opcodeID) {
		_opcodes.removeLast();
	}

	return s;
}

int Script::size() const
{
	return _opcodes.size();
}

bool Script::isEmpty() const
{
	return _opcodes.isEmpty();
}

bool Script::isValid() const
{
	return valid;
}

Opcode *Script::opcode(quint16 opcodeID) const
{
	return _opcodes.value(opcodeID);
}

const QList<Opcode *> &Script::opcodes() const
{
	return _opcodes;
}

OpcodeJump *Script::convertOpcodeJumpDirection(OpcodeJump *opcodeJump, bool *ok) const
{
	// Warning: this not change the opcode size when converted, if short jump => keep short jump, if long jump => keep long jump
	qint32 jump = opcodeJump->jump();
	if (ok)	*ok = true;

//	qDebug() << "opcodeJump" << opcodeJump->name() << jump << "label" << opcodeJump->label();

	if (jump - opcodeJump->jumpPosData() < 0) {
		if (opcodeJump->isBackJump()) {
			// OK: jump back
//			qDebug() << "OK -> jump back" << jump << opcodeJump->jumpPosData();
		} else if (opcodeJump->id() == Opcode::JMPF) {
//			qDebug() << "convert" << opcodeJump->name() << "to JMPB";
			return new OpcodeJMPB(*opcodeJump);
		} else if (opcodeJump->id() == Opcode::JMPFL) {
//			qDebug() << "convert" << opcodeJump->name() << "to JMPBL";
			return new OpcodeJMPBL(*opcodeJump);
		} else {
			if (ok)	*ok = false;
			qWarning() << "Le label doit se trouver après la commande.";
//			lastError = QObject::tr("Le label doit se trouver après la commande.");
		}
	} else if (jump - opcodeJump->jumpPosData() > 0) {
		if (opcodeJump->id() == Opcode::JMPB) {
//			qDebug() << "convert" << opcodeJump->name() << "to JMPF";
			return new OpcodeJMPF(*opcodeJump);
		} else if (opcodeJump->id() == Opcode::JMPBL) {
//			qDebug() << "convert" << opcodeJump->name() << "to JMPFL";
			return new OpcodeJMPFL(*opcodeJump);
		} else {
//			qDebug() << "OK -> jump forward";
		}
	}

	return opcodeJump;
}

bool Script::canConvertOpcodeJumpRangeToLong(OpcodeJump *opcodeJump) const
{
	bool ok = false;

	OpcodeJump *op = convertOpcodeJumpRangeToLong(opcodeJump, &ok);
	if (ok) {
		delete op;

		return true;
	}

	return false;
}

OpcodeJump *Script::convertOpcodeJumpRangeToLong(OpcodeJump *opcodeJump, bool *ok) const
{
	if (ok)	*ok = true;

	if (opcodeJump->isLongJump()) {
		return opcodeJump;
	}

	if (opcodeJump->id() == Opcode::JMPF) {
		return new OpcodeJMPFL(*opcodeJump);
	} else if (opcodeJump->id() == Opcode::JMPB) {
		return new OpcodeJMPBL(*opcodeJump);
	} else if (opcodeJump->id() == Opcode::IFUB) {
		return new OpcodeIFUBL(*(OpcodeIf *)opcodeJump);
	} else if (opcodeJump->id() == Opcode::IFSW) {
		return new OpcodeIFSWL(*(OpcodeIf *)opcodeJump);
	} else if (opcodeJump->id() == Opcode::IFUW) {
		return new OpcodeIFUWL(*(OpcodeIf *)opcodeJump);
	}

	if (ok)	*ok = false;

	return opcodeJump;
}
/*
bool Script::verifyOpcodeJumpRange(OpcodeJump *opcodeJump, QString &errorStr) const
{
	// Warning: this can change the opcode size when converted
	qint32 jump = opcodeJump->jump();
	errorStr = QString();

	qDebug() << "opcodeJump" << opcodeJump->name() << jump << "label" << opcodeJump->label();

	// if this is a long jump and opcode is a short jump
	if (!opcodeJump->isLongJump() && jump > opcodeJump->maxJump()) {
//		if (jump < 65535) {
//			switch((Opcode::Keys)opcodeJump->id()) {
//			case Opcode::JMPF:
//				return opcodeJump;
////				qDebug() << "convert" << opcodeJump->name() << "to JMPFL because" << jump << "<=" << 65535;
////				return new OpcodeJMPFL(*opcodeJump);
//			case Opcode::JMPB:
//				return opcodeJump;
////				qDebug() << "convert" << opcodeJump->name() << "to JMPBL because" << jump << "<=" << 65535;
////				return new OpcodeJMPBL(*opcodeJump);
//			case Opcode::IFUB:
//				return opcodeJump;
////				qDebug() << "convert" << opcodeJump->name() << "to IFUBL because" << jump << "<=" << 65535;
////				return new OpcodeIFUBL(*(OpcodeIf *)opcodeJump);
//			case Opcode::IFSW:
//				return opcodeJump;
////				qDebug() << "convert" << opcodeJump->name() << "to IFSW because" << jump << "<=" << 65535;
////				return new OpcodeIFSWL(*(OpcodeIf *)opcodeJump);
//			case Opcode::IFUW:
//				return opcodeJump;
////				qDebug() << "convert" << opcodeJump->name() << "to IFUW because" << jump << "<=" << 65535;
////				return new OpcodeIFUWL(*(OpcodeIf *)opcodeJump);
//			default:
//				qWarning() << "\"Aller à\" trop grand, le label est inaccessible.";
//				errorStr = QObject::tr("\"Aller à\" trop grand, le label est inaccessible.");
//				break;
//			}
//		} else {
			qWarning() << "\"Aller à\" trop grand, le label est inaccessible.";
			errorStr = QObject::tr("Label %1 is unreachable, please use a long jump.").arg(opcodeJump->label());
			return false;
		//}
	}

	return true;

	// Optimization: if this is a short jump and opcode is a long jump
//	if (opcodeJump->isLongJump() && jump - opcodeJump->jumpPosData() <= 240) {
//		switch((Opcode::Keys)opcodeJump->id()) {
//		case Opcode::JMPFL:
//			qDebug() << "convert" << opcodeJump->name() << "to JMPF because" << jump << "<=" << 240;
//			return new OpcodeJMPF(*opcodeJump);
//		case Opcode::JMPBL:
//			qDebug() << "convert" << opcodeJump->name() << "to JMPB because" << jump << "<=" << 240;
//			return new OpcodeJMPB(*opcodeJump);
//		case Opcode::IFUBL:
//			qDebug() << "convert" << opcodeJump->name() << "to IFUB because" << jump << "<=" << 240;
//			return new OpcodeIFUB(*(OpcodeIf *)opcodeJump);
//		case Opcode::IFSWL:
//			qDebug() << "convert" << opcodeJump->name() << "to IFSW because" << jump << "<=" << 240;
//			return new OpcodeIFSW(*(OpcodeIf *)opcodeJump);
//		case Opcode::IFUWL:
//			qDebug() << "convert" << opcodeJump->name() << "to IFUW because" << jump << "<=" << 240;
//			return new OpcodeIFUW(*(OpcodeIf *)opcodeJump);
//		default:
//			break;
//		}
//	}

//	return true;
}*/

bool Script::compile(int &opcodeID, QString &errorStr)
{
	quint32 pos=0;
	QHash<quint32, quint32> labelPositions;// Each label is unique

	// Search labels
	opcodeID = 0;
	for (Opcode *opcode : _opcodes) {
		if (opcode->isLabel()) {
			if (!labelPositions.contains(static_cast<OpcodeLabel *>(opcode)->label())) {
				labelPositions.insert(static_cast<OpcodeLabel *>(opcode)->label(), pos);
			} else {
				errorStr = QObject::tr("Label %1 is declared several times.")
				           .arg(static_cast<OpcodeLabel *>(opcode)->label());
				return false;
			}
		} else {
			pos += opcode->size();
		}
		++opcodeID;
	}

	// Search jumps
	opcodeID = pos = 0;
	for (Opcode *opcode : _opcodes) {
		if (opcode->isJump()) {
			OpcodeJump *opcodeJump = static_cast<OpcodeJump *>(opcode);
			if (opcodeJump->isBadJump()) {
				qWarning() << "invalid jump" << opcodeID;
				//errorStr = QObject::tr("Ce saut est invalide, veuillez mettre un label valide.");
				//return false;
			} else {
				qint32 jump = labelPositions.value(opcodeJump->label()) - pos;
				opcodeJump->setJump(jump);

				if (!opcodeJump->isLongJump() && quint32(qAbs(jump)) > opcodeJump->maxJump()) {
					if (!canConvertOpcodeJumpRangeToLong(opcodeJump)) {
						errorStr = QObject::tr("Label %1 is unreachable, please bring this instruction closer.").arg(opcodeJump->label());
						return false;
					}
				}
				if (opcodeJump->isLongJump() && quint32(qAbs(jump)) > opcodeJump->maxJump()) {
					errorStr = QObject::tr("Label %1 is unreachable because your script exceeds 65535 bytes, please reduce the size of the script.").arg(opcodeJump->label());
					return false;
				}
				if (opcodeJump->id() != Opcode::JMPF
						&& opcodeJump->id() != Opcode::JMPFL
						&& opcodeJump->id() != Opcode::JMPB
						&& opcodeJump->id() != Opcode::JMPBL
						&& jump - opcodeJump->jumpPosData() < 0) {
					errorStr = QObject::tr("The label %1 is unreachable because it is located before the opcode.").arg(opcodeJump->label());
					return false;
				}
			}
		}
		pos += opcode->size();
		++opcodeID;
	}

	if (pos > 65535) {
		errorStr = QObject::tr("Script too big, it should not exceed 65535 bytes. Actual size: %1.").arg(pos);
		return false;
	}

	return true;
}

QByteArray Script::toByteArray() const
{
	quint32 pos=0;
	QHash<quint32, quint32> labelPositions;// Each label is unique
	QByteArray ret;

	// Search labels
	for (const Opcode *opcode : _opcodes) {
		if (opcode->isLabel()) {
			labelPositions.insert(static_cast<const OpcodeLabel *>(opcode)->label(), pos);
		} else {
			pos += opcode->size();
		}
	}

	pos = 0;
	for (Opcode *opcode : _opcodes) {
		QList<Opcode *> delPlease;
		if (opcode->isJump()) {
			OpcodeJump *opcodeJump = static_cast<OpcodeJump *>(opcode);
			opcodeJump->setJump(labelPositions.value(opcodeJump->label()) - pos);

			if (!opcodeJump->isLongJump() && quint32(qAbs(opcodeJump->jump())) > opcodeJump->maxJump()) {
				bool ok = false;
				opcodeJump = convertOpcodeJumpRangeToLong(opcodeJump, &ok);

				if (opcodeJump != opcode) {
					delPlease.append(opcodeJump);
				}
			}

			opcode = convertOpcodeJumpDirection(opcodeJump);
			if (opcode != opcodeJump) {
				delPlease.append(opcode);
			}
		}
		ret.append(opcode->toByteArray());
		pos += opcode->size();
		qDeleteAll(delPlease);
	}

	return ret;
}

bool Script::isVoid() const
{
	for (const Opcode *opcode : _opcodes) {
		if (!opcode->isVoid())	return false;
	}
	return true;
}

void Script::setOpcode(quint16 opcodeID, Opcode *opcode)
{
	Opcode *curOpcode = _opcodes.at(opcodeID);
	_opcodes.replace(opcodeID, opcode);
	delete curOpcode;
}

void Script::delOpcode(quint16 opcodeID)
{
	delete _opcodes.takeAt(opcodeID);
}

Opcode *Script::removeOpcode(quint16 opcodeID)
{
	Opcode *opcode = _opcodes.takeAt(opcodeID);
	return opcode;
}

void Script::insertOpcode(quint16 opcodeID, Opcode *opcode)
{
	_opcodes.insert(opcodeID, opcode);
}

bool Script::moveOpcode(quint16 opcodeID, MoveDirection direction)
{
	if (opcodeID >= _opcodes.size())	return false;
	
	if (direction == Down)
	{
		if (opcodeID == _opcodes.size()-1)	return false;
		_opcodes.swap(opcodeID, opcodeID+1);
	}
	else
	{
		if (opcodeID == 0)	return false;
		_opcodes.swap(opcodeID, opcodeID-1);
	}
	return true;
}

bool Script::searchOpcode(int opcode, int &opcodeID) const
{
	if (opcodeID < 0) {
		opcodeID = 0;
	}

	if (opcodeID >= _opcodes.size()) {
		return false;
	}

	if (_opcodes.at(opcodeID)->searchOpcode(opcode)) {
		return true;
	}

	return searchOpcode(opcode, ++opcodeID);
}

bool Script::searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &opcodeID) const
{
	if (opcodeID < 0) {
		opcodeID = 0;
	}

	if (opcodeID >= _opcodes.size()) {
		return false;
	}

	if (_opcodes.at(opcodeID)->searchVar(bank, address, op, value)) {
		return true;
	}

	return searchVar(bank, address, op, value, ++opcodeID);
}

void Script::searchAllVars(QList<FF7Var> &vars) const
{
	for (Opcode *opcode : _opcodes) {
		opcode->getVariables(vars);
	}
}

bool Script::searchExec(quint8 group, quint8 script, int &opcodeID) const
{
	if (opcodeID < 0) {
		opcodeID = 0;
	}

	if (opcodeID >= _opcodes.size()) {
		return false;
	}

	if (_opcodes.at(opcodeID)->searchExec(group, script)) {
		return true;
	}

	return searchExec(group, script, ++opcodeID);
}

bool Script::searchMapJump(quint16 field, int &opcodeID) const
{
	if (opcodeID < 0) {
		opcodeID = 0;
	}

	if (opcodeID >= _opcodes.size()) {
		return false;
	}

	if (_opcodes.at(opcodeID)->searchMapJump(field)) {
		return true;
	}

	return searchMapJump(field, ++opcodeID);
}

bool Script::searchTextInScripts(const QRegExp &text, int &opcodeID, const Section1File *scriptsAndTexts) const
{
	if (opcodeID < 0) {
		opcodeID = 0;
	}

	if (opcodeID >= _opcodes.size()) {
		return false;
	}

	if (_opcodes.at(opcodeID)->searchTextInScripts(text, scriptsAndTexts)) {
		return true;
	}

	return searchTextInScripts(text, ++opcodeID, scriptsAndTexts);
}

bool Script::searchOpcodeP(int opcode, int &opcodeID) const
{
	if (opcodeID >= _opcodes.size()) {
		opcodeID = _opcodes.size() - 1;
	}

	if (opcodeID < 0) {
		return false;
	}

	if (_opcodes.at(opcodeID)->searchOpcode(opcode)) {
		return true;
	}

	return searchOpcodeP(opcode, --opcodeID);
}

bool Script::searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &opcodeID) const
{
	if (opcodeID >= _opcodes.size()) {
		opcodeID = _opcodes.size()-1;
	}

	if (opcodeID < 0) {
		return false;
	}

	if (_opcodes.at(opcodeID)->searchVar(bank, address, op, value)) {
		return true;
	}

	return searchVarP(bank, address, op, value, --opcodeID);
}

bool Script::searchExecP(quint8 group, quint8 script, int &opcodeID) const
{
	if (opcodeID >= _opcodes.size()) {
		opcodeID = _opcodes.size()-1;
	}

	if (opcodeID < 0) {
		return false;
	}

	if (_opcodes.at(opcodeID)->searchExec(group, script)) {
		return true;
	}

	return searchExecP(group, script, --opcodeID);
}

bool Script::searchMapJumpP(quint16 field, int &opcodeID) const
{
	if (opcodeID >= _opcodes.size()) {
		opcodeID = _opcodes.size()-1;
	}

	if (opcodeID < 0) {
		return false;
	}

	if (_opcodes.at(opcodeID)->searchMapJump(field)) {
		return true;
	}

	return searchMapJumpP(field, --opcodeID);
}

bool Script::searchTextInScriptsP(const QRegExp &text, int &opcodeID, const Section1File *scriptsAndTexts) const
{
	if (opcodeID >= _opcodes.size()) {
		opcodeID = _opcodes.size()-1;
	}

	if (opcodeID < 0) {
		return false;
	}

	if (_opcodes.at(opcodeID)->searchTextInScripts(text, scriptsAndTexts)) {
		return true;
	}

	return searchTextInScriptsP(text, --opcodeID, scriptsAndTexts);
}

void Script::listUsedTexts(QSet<quint8> &usedTexts) const
{
	for (Opcode *opcode : _opcodes)
		opcode->listUsedTexts(usedTexts);
}

void Script::listUsedTuts(QSet<quint8> &usedTuts) const
{
	for (Opcode *opcode : _opcodes)
		opcode->listUsedTuts(usedTuts);
}

void Script::shiftGroupIds(int groupId, int steps)
{
	for (Opcode *opcode : _opcodes)
		opcode->shiftGroupIds(groupId, steps);
}

void Script::shiftTextIds(int textId, int steps)
{
	for (Opcode *opcode : _opcodes)
		opcode->shiftTextIds(textId, steps);
}

void Script::shiftTutIds(int tutId, int steps)
{
	for (Opcode *opcode : _opcodes)
		opcode->shiftTutIds(tutId, steps);
}

void Script::swapGroupIds(int groupId1, int groupId2)
{
	for (Opcode *opcode : _opcodes)
		opcode->swapGroupIds(groupId1, groupId2);
}

void Script::setWindow(const FF7Window &win)
{
	if (win.opcodeID < _opcodes.size()) {
		_opcodes.at(win.opcodeID)->setWindow(win);
	}
}

int Script::opcodePositionInBytes(quint16 opcodeID)
{
	int pos=0, i=0;
	for (Opcode *op : _opcodes) {
		if (i == opcodeID) {
			return pos;
		}
		pos += op->size();
		++i;
	}
	return pos;
}

void Script::listWindows(int groupID, int scriptID, QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const
{
	int opcodeID = 0;
	for (Opcode *opcode : _opcodes) {
		opcode->listWindows(groupID, scriptID, opcodeID++, windows, text2win);
	}
}

void Script::listWindows(int groupID, int scriptID, int textID, QList<FF7Window> &windows) const
{
	int opcodeID = 0;
	QMap<int, FF7Window> lastWinPerWindowID;
	for (Opcode *opcode : _opcodes) {
		if (opcode->isJump() || opcode->id() <= Opcode::RETTO) {
			lastWinPerWindowID.clear();
		} else {
			FF7Window win = FF7Window();
			win.groupID = groupID;
			win.scriptID = scriptID;
			win.opcodeID = opcodeID;
			if (opcode->getWindow(win)) {
				win.type = opcode->id();
				lastWinPerWindowID.insert(opcode->getWindowID(), win);
			} else if (opcode->getTextID() == textID
			           && lastWinPerWindowID.contains(opcode->getWindowID())) {
				windows.append(
				    lastWinPerWindowID.value(opcode->getWindowID())
				);
			}
		}

		opcodeID += 1;
	}
}

void Script::listModelPositions(QList<FF7Position> &positions) const
{
	for (Opcode *opcode : _opcodes) {
		opcode->listModelPositions(positions);
	}
}

bool Script::linePosition(FF7Position position[2]) const
{
	for (Opcode *opcode : _opcodes) {
		if (opcode->linePosition(position)) {
			return true;
		}
	}
	return false;
}

//void Script::searchWindows() const
//{
//	for (Opcode *opcode : _opcodes) {

//	}
//}

void Script::backgroundParams(QHash<quint8, quint8> &paramActifs) const
{
	for (Opcode *opcode : _opcodes) {
		opcode->backgroundParams(paramActifs);
	}
}

void Script::backgroundMove(qint16 z[2], qint16 *x, qint16 *y) const
{
	for (Opcode *opcode : _opcodes) {
		opcode->backgroundMove(z, x, y);
	}
}

bool Script::removeTexts()
{
	bool modified = false;
	for (Opcode *opcode : _opcodes) {
		if (opcode->id() != Opcode::ASK
				&& opcode->id() != Opcode::MPNAM
				&& opcode->getTextID() != -1) {
			_opcodes.removeOne(opcode);
			modified = true;
		}
	}

	return modified;
}

QString Script::toString(Field *field) const
{
	QString ret;

	for (Opcode *opcode : _opcodes) {
		ret.append(opcode->toString(field));
		ret.append("\n");
	}

	return ret;
}

QDataStream &operator<<(QDataStream &stream, const QList<Opcode *> &script)
{
	stream << script.size();

	for (Opcode *opcode : script) {
		stream << opcode->serialize();
	}

	return stream;
}

QDataStream &operator>>(QDataStream &stream, QList<Opcode *> &script)
{
	int size;
	stream >> size;

	for (int i = 0; i < size; ++i) {
		QByteArray data;
		stream >> data;

		Opcode *op = Opcode::unserialize(data);
		if (op) {
			script.append(op);
		}
	}

	return stream;
}
