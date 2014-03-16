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
	valid = openScript(script);
}

Script::Script(const Script &other) :
	valid(true)
{
	foreach(Opcode *opcode, other.opcodes()) {
		_opcodes.append(Script::copyOpcode(opcode));
	}
}

Script::~Script()
{
	foreach(Opcode *opcode, _opcodes)	delete opcode;
}

bool Script::openScript(const QByteArray &script)
{
	int pos = 0, scriptSize = script.size(), opcodeID=1;
	Opcode *op;
	QList<int> positions;
	QMultiMap<int, OpcodeJump *> indents;

	while(pos < scriptSize)
	{
		op = createOpcode(script, pos);
		_opcodes.append(op);
		positions.append(pos);
		if(op->isJump()) {
			indents.insert(pos + ((OpcodeJump *)op)->jump(), (OpcodeJump *)op);
//			qDebug() << opcodeID << "jump to" << (pos + ((OpcodeJump *)op)->jump());
		}
		pos += op->size();
		++opcodeID;
	}
	positions.append(pos);

	QList<int> indentsKeys = indents.uniqueKeys();

	for(int i=indentsKeys.size()-1 ; i >= 0 ; --i) {
		int jump = indentsKeys.at(i);
		int index = positions.indexOf(jump);
		if(index != -1) {
			_opcodes.insert(index, new OpcodeLabel(i+1));
//			qDebug() << (index+1) << "label" << (i+1);
			foreach(OpcodeJump *opJump, indents.values(jump)) {
				opJump->setLabel(i+1);
			}
		} else {
			qWarning() << "Error" << jump << "label" << (i+1);
			foreach(OpcodeJump *opJump, indents.values(jump)) {
				opJump->setBadJump(true);
			}
//			return false;
			/*int opID=0;
			bool repaired=false;
			foreach(const int &pos, positions) {
				if(pos > jump) {
					// Repair bad jump: Insert label to the nearest valid position
					_opcodes.insert(opID, new OpcodeLabel(i+1));
//					qDebug() << (opID+1) << "repair label" << (i+1);
					repaired = true;
					break;
				}
				++opID;
			}
			if(!repaired) {
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

	if(script.size() < pos + 1 + size) {
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
		if(script.size() < pos + 1 + size) {
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
	case 0x20:	return new OpcodeMINIGAME(data, size);
	case 0x21:	return new OpcodeTUTOR(data, size);
	case 0x22:	return new OpcodeBTMD2(data, size);
	case 0x23:	return new OpcodeBTRLD(data, size);
	case 0x24:	return new OpcodeWAIT(data, size);
	case 0x25:	return new OpcodeNFADE(data, size);
	case 0x26:	return new OpcodeBLINK(data, size);
	case 0x27:	return new OpcodeBGMOVIE(data, size);
	case 0x28://KAWAI
		if(pos + 1 < script.size()) {
			size = (quint8)script.at(pos+1) - 1;
			if(pos + 1 + size <= script.size()) {
				return new OpcodeKAWAI(data, size);
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
	case 0x00:	return new OpcodeRET(*(OpcodeRET *)opcode);
	case 0x01:	return new OpcodeREQ(*(OpcodeREQ *)opcode);
	case 0x02:	return new OpcodeREQSW(*(OpcodeREQSW *)opcode);
	case 0x03:	return new OpcodeREQEW(*(OpcodeREQEW *)opcode);
	case 0x04:	return new OpcodePREQ(*(OpcodePREQ *)opcode);
	case 0x05:	return new OpcodePRQSW(*(OpcodePRQSW *)opcode);
	case 0x06:	return new OpcodePRQEW(*(OpcodePRQEW *)opcode);
	case 0x07:	return new OpcodeRETTO(*(OpcodeRETTO *)opcode);
	case 0x08:	return new OpcodeJOIN(*(OpcodeJOIN *)opcode);
	case 0x09:	return new OpcodeSPLIT(*(OpcodeSPLIT *)opcode);
	case 0x0A:	return new OpcodeSPTYE(*(OpcodeSPTYE *)opcode);
	case 0x0B:	return new OpcodeGTPYE(*(OpcodeGTPYE *)opcode);
	case 0x0E:	return new OpcodeDSKCG(*(OpcodeDSKCG *)opcode);
	case 0x0F:	return new OpcodeSPECIAL(*(OpcodeSPECIAL *)opcode);
	case 0x10:	return new OpcodeJMPF(*(OpcodeJMPF *)opcode);
	case 0x11:	return new OpcodeJMPFL(*(OpcodeJMPFL *)opcode);
	case 0x12:	return new OpcodeJMPB(*(OpcodeJMPB *)opcode);
	case 0x13:	return new OpcodeJMPBL(*(OpcodeJMPBL *)opcode);
	case 0x14:	return new OpcodeIFUB(*(OpcodeIFUB *)opcode);
	case 0x15:	return new OpcodeIFUBL(*(OpcodeIFUBL *)opcode);
	case 0x16:	return new OpcodeIFSW(*(OpcodeIFSW *)opcode);
	case 0x17:	return new OpcodeIFSWL(*(OpcodeIFSWL *)opcode);
	case 0x18:	return new OpcodeIFUW(*(OpcodeIFUW *)opcode);
	case 0x19:	return new OpcodeIFUWL(*(OpcodeIFUWL *)opcode);
	case 0x20:	return new OpcodeMINIGAME(*(OpcodeMINIGAME *)opcode);
	case 0x21:	return new OpcodeTUTOR(*(OpcodeTUTOR *)opcode);
	case 0x22:	return new OpcodeBTMD2(*(OpcodeBTMD2 *)opcode);
	case 0x23:	return new OpcodeBTRLD(*(OpcodeBTRLD *)opcode);
	case 0x24:	return new OpcodeWAIT(*(OpcodeWAIT *)opcode);
	case 0x25:	return new OpcodeNFADE(*(OpcodeNFADE *)opcode);
	case 0x26:	return new OpcodeBLINK(*(OpcodeBLINK *)opcode);
	case 0x27:	return new OpcodeBGMOVIE(*(OpcodeBGMOVIE *)opcode);
	case 0x28:	return new OpcodeKAWAI(*(OpcodeKAWAI *)opcode);
	case 0x29:	return new OpcodeKAWIW(*(OpcodeKAWIW *)opcode);
	case 0x2A:	return new OpcodePMOVA(*(OpcodePMOVA *)opcode);
	case 0x2B:	return new OpcodeSLIP(*(OpcodeSLIP *)opcode);
	case 0x2C:	return new OpcodeBGPDH(*(OpcodeBGPDH *)opcode);
	case 0x2D:	return new OpcodeBGSCR(*(OpcodeBGSCR *)opcode);
	case 0x2E:	return new OpcodeWCLS(*(OpcodeWCLS *)opcode);
	case 0x2F:	return new OpcodeWSIZW(*(OpcodeWSIZW *)opcode);
	case 0x30:	return new OpcodeIFKEY(*(OpcodeIFKEY *)opcode);
	case 0x31:	return new OpcodeIFKEYON(*(OpcodeIFKEYON *)opcode);
	case 0x32:	return new OpcodeIFKEYOFF(*(OpcodeIFKEYOFF *)opcode);
	case 0x33:	return new OpcodeUC(*(OpcodeUC *)opcode);
	case 0x34:	return new OpcodePDIRA(*(OpcodePDIRA *)opcode);
	case 0x35:	return new OpcodePTURA(*(OpcodePTURA *)opcode);
	case 0x36:	return new OpcodeWSPCL(*(OpcodeWSPCL *)opcode);
	case 0x37:	return new OpcodeWNUMB(*(OpcodeWNUMB *)opcode);
	case 0x38:	return new OpcodeSTTIM(*(OpcodeSTTIM *)opcode);
	case 0x39:	return new OpcodeGOLDu(*(OpcodeGOLDu *)opcode);
	case 0x3A:	return new OpcodeGOLDd(*(OpcodeGOLDd *)opcode);
	case 0x3B:	return new OpcodeCHGLD(*(OpcodeCHGLD *)opcode);
	case 0x3C:	return new OpcodeHMPMAX1(*(OpcodeHMPMAX1 *)opcode);
	case 0x3D:	return new OpcodeHMPMAX2(*(OpcodeHMPMAX2 *)opcode);
	case 0x3E:	return new OpcodeMHMMX(*(OpcodeMHMMX *)opcode);
	case 0x3F:	return new OpcodeHMPMAX3(*(OpcodeHMPMAX3 *)opcode);
	case 0x40:	return new OpcodeMESSAGE(*(OpcodeMESSAGE *)opcode);
	case 0x41:	return new OpcodeMPARA(*(OpcodeMPARA *)opcode);
	case 0x42:	return new OpcodeMPRA2(*(OpcodeMPRA2 *)opcode);
	case 0x43:	return new OpcodeMPNAM(*(OpcodeMPNAM *)opcode);

	case 0x45:	return new OpcodeMPu(*(OpcodeMPu *)opcode);

	case 0x47:	return new OpcodeMPd(*(OpcodeMPd *)opcode);
	case 0x48:	return new OpcodeASK(*(OpcodeASK *)opcode);
	case 0x49:	return new OpcodeMENU(*(OpcodeMENU *)opcode);
	case 0x4A:	return new OpcodeMENU2(*(OpcodeMENU2 *)opcode);
	case 0x4B:	return new OpcodeBTLTB(*(OpcodeBTLTB *)opcode);

	case 0x4D:	return new OpcodeHPu(*(OpcodeHPu *)opcode);

	case 0x4F:	return new OpcodeHPd(*(OpcodeHPd *)opcode);
	case 0x50:	return new OpcodeWINDOW(*(OpcodeWINDOW *)opcode);
	case 0x51:	return new OpcodeWMOVE(*(OpcodeWMOVE *)opcode);
	case 0x52:	return new OpcodeWMODE(*(OpcodeWMODE *)opcode);
	case 0x53:	return new OpcodeWREST(*(OpcodeWREST *)opcode);
	case 0x54:	return new OpcodeWCLSE(*(OpcodeWCLSE *)opcode);
	case 0x55:	return new OpcodeWROW(*(OpcodeWROW *)opcode);
	case 0x56:	return new OpcodeGWCOL(*(OpcodeGWCOL *)opcode);
	case 0x57:	return new OpcodeSWCOL(*(OpcodeSWCOL *)opcode);
	case 0x58:	return new OpcodeSTITM(*(OpcodeSTITM *)opcode);
	case 0x59:	return new OpcodeDLITM(*(OpcodeDLITM *)opcode);
	case 0x5A:	return new OpcodeCKITM(*(OpcodeCKITM *)opcode);
	case 0x5B:	return new OpcodeSMTRA(*(OpcodeSMTRA *)opcode);
	case 0x5C:	return new OpcodeDMTRA(*(OpcodeDMTRA *)opcode);
	case 0x5D:	return new OpcodeCMTRA(*(OpcodeCMTRA *)opcode);
	case 0x5E:	return new OpcodeSHAKE(*(OpcodeSHAKE *)opcode);
	case 0x5F:	return new OpcodeNOP(*(OpcodeNOP *)opcode);
	case 0x60:	return new OpcodeMAPJUMP(*(OpcodeMAPJUMP *)opcode);
	case 0x61:	return new OpcodeSCRLO(*(OpcodeSCRLO *)opcode);
	case 0x62:	return new OpcodeSCRLC(*(OpcodeSCRLC *)opcode);
	case 0x63:	return new OpcodeSCRLA(*(OpcodeSCRLA *)opcode);
	case 0x64:	return new OpcodeSCR2D(*(OpcodeSCR2D *)opcode);
	case 0x65:	return new OpcodeSCRCC(*(OpcodeSCRCC *)opcode);
	case 0x66:	return new OpcodeSCR2DC(*(OpcodeSCR2DC *)opcode);
	case 0x67:	return new OpcodeSCRLW(*(OpcodeSCRLW *)opcode);
	case 0x68:	return new OpcodeSCR2DL(*(OpcodeSCR2DL *)opcode);
	case 0x69:	return new OpcodeMPDSP(*(OpcodeMPDSP *)opcode);
	case 0x6A:	return new OpcodeVWOFT(*(OpcodeVWOFT *)opcode);
	case 0x6B:	return new OpcodeFADE(*(OpcodeFADE *)opcode);
	case 0x6C:	return new OpcodeFADEW(*(OpcodeFADEW *)opcode);
	case 0x6D:	return new OpcodeIDLCK(*(OpcodeIDLCK *)opcode);
	case 0x6E:	return new OpcodeLSTMP(*(OpcodeLSTMP *)opcode);
	case 0x6F:	return new OpcodeSCRLP(*(OpcodeSCRLP *)opcode);
	case 0x70:	return new OpcodeBATTLE(*(OpcodeBATTLE *)opcode);
	case 0x71:	return new OpcodeBTLON(*(OpcodeBTLON *)opcode);
	case 0x72:	return new OpcodeBTLMD(*(OpcodeBTLMD *)opcode);
	case 0x73:	return new OpcodePGTDR(*(OpcodePGTDR *)opcode);
	case 0x74:	return new OpcodeGETPC(*(OpcodeGETPC *)opcode);
	case 0x75:	return new OpcodePXYZI(*(OpcodePXYZI *)opcode);
	case 0x76:	return new OpcodePLUSX(*(OpcodePLUSX *)opcode);
	case 0x77:	return new OpcodePLUS2X(*(OpcodePLUS2X *)opcode);
	case 0x78:	return new OpcodeMINUSX(*(OpcodeMINUSX *)opcode);
	case 0x79:	return new OpcodeMINUS2X(*(OpcodeMINUS2X *)opcode);
	case 0x7A:	return new OpcodeINCX(*(OpcodeINCX *)opcode);
	case 0x7B:	return new OpcodeINC2X(*(OpcodeINC2X *)opcode);
	case 0x7C:	return new OpcodeDECX(*(OpcodeDECX *)opcode);
	case 0x7D:	return new OpcodeDEC2X(*(OpcodeDEC2X *)opcode);
	case 0x7E:	return new OpcodeTLKON(*(OpcodeTLKON *)opcode);
	case 0x7F:	return new OpcodeRDMSD(*(OpcodeRDMSD *)opcode);
	case 0x80:	return new OpcodeSETBYTE(*(OpcodeSETBYTE *)opcode);
	case 0x81:	return new OpcodeSETWORD(*(OpcodeSETWORD *)opcode);
	case 0x82:	return new OpcodeBITON(*(OpcodeBITON *)opcode);
	case 0x83:	return new OpcodeBITOFF(*(OpcodeBITOFF *)opcode);
	case 0x84:	return new OpcodeBITXOR(*(OpcodeBITXOR *)opcode);
	case 0x85:	return new OpcodePLUS(*(OpcodePLUS *)opcode);
	case 0x86:	return new OpcodePLUS2(*(OpcodePLUS2 *)opcode);
	case 0x87:	return new OpcodeMINUS(*(OpcodeMINUS *)opcode);
	case 0x88:	return new OpcodeMINUS2(*(OpcodeMINUS2 *)opcode);
	case 0x89:	return new OpcodeMUL(*(OpcodeMUL *)opcode);
	case 0x8A:	return new OpcodeMUL2(*(OpcodeMUL2 *)opcode);
	case 0x8B:	return new OpcodeDIV(*(OpcodeDIV *)opcode);
	case 0x8C:	return new OpcodeDIV2(*(OpcodeDIV2 *)opcode);
	case 0x8D:	return new OpcodeMOD(*(OpcodeMOD *)opcode);
	case 0x8E:	return new OpcodeMOD2(*(OpcodeMOD2 *)opcode);
	case 0x8F:	return new OpcodeAND(*(OpcodeAND *)opcode);
	case 0x90:	return new OpcodeAND2(*(OpcodeAND2 *)opcode);
	case 0x91:	return new OpcodeOR(*(OpcodeOR *)opcode);
	case 0x92:	return new OpcodeOR2(*(OpcodeOR2 *)opcode);
	case 0x93:	return new OpcodeXOR(*(OpcodeXOR *)opcode);
	case 0x94:	return new OpcodeXOR2(*(OpcodeXOR2 *)opcode);
	case 0x95:	return new OpcodeINC(*(OpcodeINC *)opcode);
	case 0x96:	return new OpcodeINC2(*(OpcodeINC2 *)opcode);
	case 0x97:	return new OpcodeDEC(*(OpcodeDEC *)opcode);
	case 0x98:	return new OpcodeDEC2(*(OpcodeDEC2 *)opcode);
	case 0x99:	return new OpcodeRANDOM(*(OpcodeRANDOM *)opcode);
	case 0x9A:	return new OpcodeLBYTE(*(OpcodeLBYTE *)opcode);
	case 0x9B:	return new OpcodeHBYTE(*(OpcodeHBYTE *)opcode);
	case 0x9C:	return new Opcode2BYTE(*(Opcode2BYTE *)opcode);
	case 0x9D:	return new OpcodeSETX(*(OpcodeSETX *)opcode);
	case 0x9E:	return new OpcodeGETX(*(OpcodeGETX *)opcode);
	case 0x9F:	return new OpcodeSEARCHX(*(OpcodeSEARCHX *)opcode);
	case 0xA0:	return new OpcodePC(*(OpcodePC *)opcode);
	case 0xA1:	return new OpcodeCHAR(*(OpcodeCHAR *)opcode);
	case 0xA2:	return new OpcodeDFANM(*(OpcodeDFANM *)opcode);
	case 0xA3:	return new OpcodeANIME1(*(OpcodeANIME1 *)opcode);
	case 0xA4:	return new OpcodeVISI(*(OpcodeVISI *)opcode);
	case 0xA5:	return new OpcodeXYZI(*(OpcodeXYZI *)opcode);
	case 0xA6:	return new OpcodeXYI(*(OpcodeXYI *)opcode);
	case 0xA7:	return new OpcodeXYZ(*(OpcodeXYZ *)opcode);
	case 0xA8:	return new OpcodeMOVE(*(OpcodeMOVE *)opcode);
	case 0xA9:	return new OpcodeCMOVE(*(OpcodeCMOVE *)opcode);
	case 0xAA:	return new OpcodeMOVA(*(OpcodeMOVA *)opcode);
	case 0xAB:	return new OpcodeTURA(*(OpcodeTURA *)opcode);
	case 0xAC:	return new OpcodeANIMW(*(OpcodeANIMW *)opcode);
	case 0xAD:	return new OpcodeFMOVE(*(OpcodeFMOVE *)opcode);
	case 0xAE:	return new OpcodeANIME2(*(OpcodeANIME2 *)opcode);
	case 0xAF:	return new OpcodeANIMX1(*(OpcodeANIMX1 *)opcode);
	case 0xB0:	return new OpcodeCANIM1(*(OpcodeCANIM1 *)opcode);
	case 0xB1:	return new OpcodeCANMX1(*(OpcodeCANMX1 *)opcode);
	case 0xB2:	return new OpcodeMSPED(*(OpcodeMSPED *)opcode);
	case 0xB3:	return new OpcodeDIR(*(OpcodeDIR *)opcode);
	case 0xB4:	return new OpcodeTURNGEN(*(OpcodeTURNGEN *)opcode);
	case 0xB5:	return new OpcodeTURN(*(OpcodeTURN *)opcode);
	case 0xB6:	return new OpcodeDIRA(*(OpcodeDIRA *)opcode);
	case 0xB7:	return new OpcodeGETDIR(*(OpcodeGETDIR *)opcode);
	case 0xB8:	return new OpcodeGETAXY(*(OpcodeGETAXY *)opcode);
	case 0xB9:	return new OpcodeGETAI(*(OpcodeGETAI *)opcode);
	case 0xBA:	return new OpcodeANIMX2(*(OpcodeANIMX2 *)opcode);
	case 0xBB:	return new OpcodeCANIM2(*(OpcodeCANIM2 *)opcode);
	case 0xBC:	return new OpcodeCANMX2(*(OpcodeCANMX2 *)opcode);
	case 0xBD:	return new OpcodeASPED(*(OpcodeASPED *)opcode);

	case 0xBF:	return new OpcodeCC(*(OpcodeCC *)opcode);
	case 0xC0:	return new OpcodeJUMP(*(OpcodeJUMP *)opcode);
	case 0xC1:	return new OpcodeAXYZI(*(OpcodeAXYZI *)opcode);
	case 0xC2:	return new OpcodeLADER(*(OpcodeLADER *)opcode);
	case 0xC3:	return new OpcodeOFST(*(OpcodeOFST *)opcode);
	case 0xC4:	return new OpcodeOFSTW(*(OpcodeOFSTW *)opcode);
	case 0xC5:	return new OpcodeTALKR(*(OpcodeTALKR *)opcode);
	case 0xC6:	return new OpcodeSLIDR(*(OpcodeSLIDR *)opcode);
	case 0xC7:	return new OpcodeSOLID(*(OpcodeSOLID *)opcode);
	case 0xC8:	return new OpcodePRTYP(*(OpcodePRTYP *)opcode);
	case 0xC9:	return new OpcodePRTYM(*(OpcodePRTYM *)opcode);
	case 0xCA:	return new OpcodePRTYE(*(OpcodePRTYE *)opcode);
	case 0xCB:	return new OpcodeIFPRTYQ(*(OpcodeIFPRTYQ *)opcode);
	case 0xCC:	return new OpcodeIFMEMBQ(*(OpcodeIFMEMBQ *)opcode);
	case 0xCD:	return new OpcodeMMBUD(*(OpcodeMMBUD *)opcode);
	case 0xCE:	return new OpcodeMMBLK(*(OpcodeMMBLK *)opcode);
	case 0xCF:	return new OpcodeMMBUK(*(OpcodeMMBUK *)opcode);
	case 0xD0:	return new OpcodeLINE(*(OpcodeLINE *)opcode);
	case 0xD1:	return new OpcodeLINON(*(OpcodeLINON *)opcode);
	case 0xD2:	return new OpcodeMPJPO(*(OpcodeMPJPO *)opcode);
	case 0xD3:	return new OpcodeSLINE(*(OpcodeSLINE *)opcode);
	case 0xD4:	return new OpcodeSIN(*(OpcodeSIN *)opcode);
	case 0xD5:	return new OpcodeCOS(*(OpcodeCOS *)opcode);
	case 0xD6:	return new OpcodeTLKR2(*(OpcodeTLKR2 *)opcode);
	case 0xD7:	return new OpcodeSLDR2(*(OpcodeSLDR2 *)opcode);
	case 0xD8:	return new OpcodePMJMP(*(OpcodePMJMP *)opcode);
	case 0xD9:	return new OpcodePMJMP2(*(OpcodePMJMP2 *)opcode);
	case 0xDA:	return new OpcodeAKAO2(*(OpcodeAKAO2 *)opcode);
	case 0xDB:	return new OpcodeFCFIX(*(OpcodeFCFIX *)opcode);
	case 0xDC:	return new OpcodeCCANM(*(OpcodeCCANM *)opcode);
	case 0xDD:	return new OpcodeANIMB(*(OpcodeANIMB *)opcode);
	case 0xDE:	return new OpcodeTURNW(*(OpcodeTURNW *)opcode);
	case 0xDF:	return new OpcodeMPPAL(*(OpcodeMPPAL *)opcode);
	case 0xE0:	return new OpcodeBGON(*(OpcodeBGON *)opcode);
	case 0xE1:	return new OpcodeBGOFF(*(OpcodeBGOFF *)opcode);
	case 0xE2:	return new OpcodeBGROL(*(OpcodeBGROL *)opcode);
	case 0xE3:	return new OpcodeBGROL2(*(OpcodeBGROL2 *)opcode);
	case 0xE4:	return new OpcodeBGCLR(*(OpcodeBGCLR *)opcode);
	case 0xE5:	return new OpcodeSTPAL(*(OpcodeSTPAL *)opcode);
	case 0xE6:	return new OpcodeLDPAL(*(OpcodeLDPAL *)opcode);
	case 0xE7:	return new OpcodeCPPAL(*(OpcodeCPPAL *)opcode);
	case 0xE8:	return new OpcodeRTPAL(*(OpcodeRTPAL *)opcode);
	case 0xE9:	return new OpcodeADPAL(*(OpcodeADPAL *)opcode);
	case 0xEA:	return new OpcodeMPPAL2(*(OpcodeMPPAL2 *)opcode);
	case 0xEB:	return new OpcodeSTPLS(*(OpcodeSTPLS *)opcode);
	case 0xEC:	return new OpcodeLDPLS(*(OpcodeLDPLS *)opcode);
	case 0xED:	return new OpcodeCPPAL2(*(OpcodeCPPAL2 *)opcode);
	case 0xEE:	return new OpcodeRTPAL2(*(OpcodeRTPAL2 *)opcode);
	case 0xEF:	return new OpcodeADPAL2(*(OpcodeADPAL2 *)opcode);
	case 0xF0:	return new OpcodeMUSIC(*(OpcodeMUSIC *)opcode);
	case 0xF1:	return new OpcodeSOUND(*(OpcodeSOUND *)opcode);
	case 0xF2:	return new OpcodeAKAO(*(OpcodeAKAO *)opcode);
	case 0xF3:	return new OpcodeMUSVT(*(OpcodeMUSVT *)opcode);
	case 0xF4:	return new OpcodeMUSVM(*(OpcodeMUSVM *)opcode);
	case 0xF5:	return new OpcodeMULCK(*(OpcodeMULCK *)opcode);
	case 0xF6:	return new OpcodeBMUSC(*(OpcodeBMUSC *)opcode);
	case 0xF7:	return new OpcodeCHMPH(*(OpcodeCHMPH *)opcode);
	case 0xF8:	return new OpcodePMVIE(*(OpcodePMVIE *)opcode);
	case 0xF9:	return new OpcodeMOVIE(*(OpcodeMOVIE *)opcode);
	case 0xFA:	return new OpcodeMVIEF(*(OpcodeMVIEF *)opcode);
	case 0xFB:	return new OpcodeMVCAM(*(OpcodeMVCAM *)opcode);
	case 0xFC:	return new OpcodeFMUSC(*(OpcodeFMUSC *)opcode);
	case 0xFD:	return new OpcodeCMUSC(*(OpcodeCMUSC *)opcode);
	case 0xFE:	return new OpcodeCHMST(*(OpcodeCHMST *)opcode);
	case 0xFF:	return new OpcodeGAMEOVER(*(OpcodeGAMEOVER *)opcode);
	case 0x100:	return new OpcodeLabel(*(OpcodeLabel *)opcode);
	default:	return new OpcodeUnknown(*(OpcodeUnknown *)opcode);
	}
}

Script *Script::splitScriptAtReturn()
{
	int gotoLabel = -1;
	int opcodeID = 0;

	foreach(Opcode *opcode, _opcodes) {
		if(opcode->isLabel()) {
			if(gotoLabel != -1 && ((OpcodeLabel *)opcode)->label() == (quint32)gotoLabel) {
				gotoLabel = -1;
			}
		} else if(gotoLabel == -1) {
			if(opcode->isJump() && !((OpcodeJump *)opcode)->isBackJump()) {
				gotoLabel = ((OpcodeJump *)opcode)->label();
			} else if(opcode->id() == Opcode::RET || opcode->id() == Opcode::RETTO) {
				++opcodeID;
				break;
			}
		}
		++opcodeID;
	}

	Script *s = new Script(_opcodes.mid(opcodeID));
	int size = _opcodes.size();
	for( ; opcodeID < size ; ++opcodeID) {
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
	if(ok)	*ok = true;

//	qDebug() << "opcodeJump" << opcodeJump->name() << jump << "label" << opcodeJump->label();

	if(jump - opcodeJump->jumpPosData() < 0) {
		if(opcodeJump->isBackJump()) {
			// OK: jump back
//			qDebug() << "OK -> jump back" << jump << opcodeJump->jumpPosData();
		} else if(opcodeJump->id() == Opcode::JMPF) {
//			qDebug() << "convert" << opcodeJump->name() << "to JMPB";
			return new OpcodeJMPB(*opcodeJump);
		} else if(opcodeJump->id() == Opcode::JMPFL) {
//			qDebug() << "convert" << opcodeJump->name() << "to JMPBL";
			return new OpcodeJMPBL(*opcodeJump);
		} else {
			if(ok)	*ok = false;
			qWarning() << "Le label doit se trouver après la commande.";
//			lastError = QObject::tr("Le label doit se trouver après la commande.");
		}
	} else if(jump - opcodeJump->jumpPosData() > 0) {
		if(opcodeJump->id() == Opcode::JMPB) {
//			qDebug() << "convert" << opcodeJump->name() << "to JMPF";
			return new OpcodeJMPF(*opcodeJump);
		} else if(opcodeJump->id() == Opcode::JMPBL) {
//			qDebug() << "convert" << opcodeJump->name() << "to JMPFL";
			return new OpcodeJMPFL(*opcodeJump);
		} else {
//			qDebug() << "OK -> jump forward";
		}
	}

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
	if(!opcodeJump->isLongJump() && jump > opcodeJump->maxJump()) {
//		if(jump < 65535) {
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
			errorStr = QObject::tr("Le label %1 est inaccessible, veuillez utiliser un saut long.").arg(opcodeJump->label());
			return false;
		//}
	}

	return true;

	// Optimization: if this is a short jump and opcode is a long jump
//	if(opcodeJump->isLongJump() && jump - opcodeJump->jumpPosData() <= 240) {
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
	foreach(Opcode *opcode, _opcodes) {
		if(opcode->isLabel()) {
			if(!labelPositions.contains(((OpcodeLabel *)opcode)->label())) {
				labelPositions.insert(((OpcodeLabel *)opcode)->label(), pos);
			} else {
				errorStr = QObject::tr("Le label %1 est déclaré plusieurs fois.").arg(((OpcodeLabel *)opcode)->label());
				return false;
			}
		} else {
			pos += opcode->size();
		}
		++opcodeID;
	}

	// Search jumps
	opcodeID = pos = 0;
	foreach(Opcode *opcode, _opcodes) {
		if(opcode->isJump()) {
			OpcodeJump *opcodeJump = (OpcodeJump *)opcode;
			if(opcodeJump->isBadJump()) {
				qWarning() << "invalid jump" << opcodeID;
				//errorStr = QObject::tr("Ce saut est invalide, veuillez mettre un label valide.");
				//return false;
			} else {
				qint32 jump = labelPositions.value(opcodeJump->label()) - pos;
				opcodeJump->setJump(jump);

				if(!opcodeJump->isLongJump() && quint32(qAbs(jump)) > opcodeJump->maxJump()) {
					errorStr = QObject::tr("Le label %1 est inaccessible, veuillez utiliser un saut long.").arg(opcodeJump->label());
					return false;
				}
				if(opcodeJump->isLongJump() && quint32(qAbs(jump)) > opcodeJump->maxJump()) {
					errorStr = QObject::tr("Le label %1 est inaccessible car votre script dépasse 65535 octets, veuillez réduire la taille du script.").arg(opcodeJump->label());
					return false;
				}
				if(opcodeJump->id() != Opcode::JMPF
						&& opcodeJump->id() != Opcode::JMPFL
						&& opcodeJump->id() != Opcode::JMPB
						&& opcodeJump->id() != Opcode::JMPBL
						&& jump - opcodeJump->jumpPosData() < 0) {
					errorStr = QObject::tr("Le label %1 est inaccessible car il se trouve avant la commande.").arg(opcodeJump->label());
					return false;
				}
			}
		}
		pos += opcode->size();
		++opcodeID;
	}

	if(pos > 65535) {
		errorStr = QObject::tr("Script trop grand, il ne doit pas dépasser les 65535 octets. Taille actuelle : %1.").arg(pos);
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
	foreach(Opcode *opcode, _opcodes) {
		if(opcode->isLabel()) {
			labelPositions.insert(((OpcodeLabel *)opcode)->label(), pos);
		} else {
			pos += opcode->size();
		}
	}

	pos = 0;
	foreach(const Opcode *opcode, _opcodes) {
		bool delPlease = false;
		if(opcode->isJump()) {
			OpcodeJump *opcodeJump = (OpcodeJump *)opcode;
			opcodeJump->setJump(labelPositions.value(opcodeJump->label()) - pos);

			opcode = convertOpcodeJumpDirection(opcodeJump);
			delPlease = opcode != opcodeJump;
		}
		ret.append(opcode->toByteArray());
		pos += opcode->size();

		if(delPlease) {
			delete opcode;
		}
	}

	return ret;
}

bool Script::isVoid() const
{
	foreach(Opcode *opcode, _opcodes)
		if(!opcode->isVoid())	return false;
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
	if(opcodeID >= _opcodes.size())	return false;
	
	if(direction == Down)
	{
		if(opcodeID == _opcodes.size()-1)	return false;
		_opcodes.swap(opcodeID, opcodeID+1);
	}
	else
	{
		if(opcodeID == 0)	return false;
		_opcodes.swap(opcodeID, opcodeID-1);
	}
	return true;
}

bool Script::searchOpcode(int opcode, int &opcodeID) const
{
	if(opcodeID < 0) 	opcodeID = 0;
	if(opcodeID >= _opcodes.size())				return false;
	if(opcode == _opcodes.at(opcodeID)->id())	return true;

	return searchOpcode(opcode, ++opcodeID);
}

bool Script::searchVar(quint8 bank, quint8 adress, int value, int &opcodeID) const
{
	if(opcodeID < 0) 	opcodeID = 0;
	if(opcodeID >= _opcodes.size())								return false;
	if(_opcodes.at(opcodeID)->searchVar(bank, adress, value))	return true;

	return searchVar(bank, adress, value, ++opcodeID);
}

void Script::searchAllVars(QList<FF7Var> &vars) const
{
	foreach(Opcode *opcode, _opcodes) {
		opcode->getVariables(vars);
	}
}

bool Script::searchExec(quint8 group, quint8 script, int &opcodeID) const
{
	if(opcodeID < 0) 	opcodeID = 0;
	if(opcodeID >= _opcodes.size())						return false;
	if(_opcodes.at(opcodeID)->searchExec(group, script))	return true;

	return searchExec(group, script, ++opcodeID);
}

bool Script::searchMapJump(quint16 field, int &opcodeID) const
{
	if(opcodeID < 0) 	opcodeID = 0;
	if(opcodeID >= _opcodes.size())					return false;
	if(_opcodes.at(opcodeID)->searchMapJump(field))	return true;

	return searchMapJump(field, ++opcodeID);
}

bool Script::searchTextInScripts(const QRegExp &text, int &opcodeID, const Section1File *scriptsAndTexts) const
{
	if(opcodeID < 0) 	opcodeID = 0;
	if(opcodeID >= _opcodes.size())					return false;
	if(_opcodes.at(opcodeID)->searchTextInScripts(text, scriptsAndTexts))		return true;

	return searchTextInScripts(text, ++opcodeID, scriptsAndTexts);
}

bool Script::searchOpcodeP(int opcode, int &opcodeID) const
{
	if(opcodeID >= _opcodes.size()) opcodeID = _opcodes.size()-1;
	if(opcodeID < 0)							return false;
	if(opcode == _opcodes.at(opcodeID)->id())	return true;

	return searchOpcodeP(opcode, --opcodeID);
}

bool Script::searchVarP(quint8 bank, quint8 adress, int value, int &opcodeID) const
{
	if(opcodeID >= _opcodes.size()) opcodeID = _opcodes.size()-1;
	if(opcodeID < 0)											return false;
	if(_opcodes.at(opcodeID)->searchVar(bank, adress, value))	return true;

	return searchVarP(bank, adress, value, --opcodeID);
}

bool Script::searchExecP(quint8 group, quint8 script, int &opcodeID) const
{
	if(opcodeID >= _opcodes.size()) opcodeID = _opcodes.size()-1;
	if(opcodeID < 0)										return false;
	if(_opcodes.at(opcodeID)->searchExec(group, script))	return true;

	return searchExecP(group, script, --opcodeID);
}

bool Script::searchMapJumpP(quint16 field, int &opcodeID) const
{
	if(opcodeID >= _opcodes.size()) opcodeID = _opcodes.size()-1;
	if(opcodeID < 0)								return false;
	if(_opcodes.at(opcodeID)->searchMapJump(field))	return true;

	return searchMapJumpP(field, --opcodeID);
}

bool Script::searchTextInScriptsP(const QRegExp &text, int &opcodeID, const Section1File *scriptsAndTexts) const
{
	if(opcodeID >= _opcodes.size()) opcodeID = _opcodes.size()-1;
	if(opcodeID < 0)								return false;
	if(_opcodes.at(opcodeID)->searchTextInScripts(text, scriptsAndTexts))		return true;

	return searchTextInScriptsP(text, --opcodeID, scriptsAndTexts);
}

void Script::listUsedTexts(QSet<quint8> &usedTexts) const
{
	foreach(Opcode *opcode, _opcodes)
		opcode->listUsedTexts(usedTexts);
}

void Script::listUsedTuts(QSet<quint8> &usedTuts) const
{
	foreach(Opcode *opcode, _opcodes)
		opcode->listUsedTuts(usedTuts);
}

void Script::shiftGroupIds(int groupId, int steps)
{
	foreach(Opcode *opcode, _opcodes)
		opcode->shiftGroupIds(groupId, steps);
}

void Script::shiftTextIds(int textId, int steps)
{
	foreach(Opcode *opcode, _opcodes)
		opcode->shiftTextIds(textId, steps);
}

void Script::shiftTutIds(int tutId, int steps)
{
	foreach(Opcode *opcode, _opcodes)
		opcode->shiftTutIds(tutId, steps);
}

void Script::swapGroupIds(int groupId1, int groupId2)
{
	foreach(Opcode *opcode, _opcodes)
		opcode->swapGroupIds(groupId1, groupId2);
}

void Script::setWindow(const FF7Window &win)
{
	if(win.opcodeID < _opcodes.size()) {
		_opcodes.at(win.opcodeID)->setWindow(win);
	}
}

int Script::opcodePositionInBytes(quint16 opcodeID)
{
	int pos=0, i=0;
	foreach(Opcode *op, _opcodes) {
		if(i == opcodeID) {
			return pos;
		}
		pos += op->size();
		++i;
	}
	return pos;
}

void Script::listWindows(int groupID, int scriptID, QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const
{
	int opcodeID=0;
	foreach(Opcode *opcode, _opcodes)
		opcode->listWindows(groupID, scriptID, opcodeID++, windows, text2win);
}

void Script::listModelPositions(QList<FF7Position> &positions) const
{
	foreach(Opcode *opcode, _opcodes)
		opcode->listModelPositions(positions);
}

bool Script::linePosition(FF7Position position[2]) const
{
	foreach(Opcode *opcode, _opcodes) {
		if(opcode->linePosition(position)) {
			return true;
		}
	}
	return false;
}

//void Script::searchWindows() const
//{
//	foreach(Opcode *opcode, _opcodes) {

//	}
//}

void Script::backgroundParams(QHash<quint8, quint8> &paramActifs) const
{
	foreach(Opcode *opcode, _opcodes)
		opcode->backgroundParams(paramActifs);
}

void Script::backgroundMove(qint16 z[2], qint16 *x, qint16 *y) const
{
	foreach(Opcode *opcode, _opcodes)
		opcode->backgroundMove(z, x, y);
}

bool Script::removeTexts()
{
	bool modified = false;
	foreach(Opcode *opcode, _opcodes) {
		if(opcode->id() != Opcode::ASK
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

	foreach(Opcode *opcode, _opcodes) {
		ret.append(opcode->toString(field));
		ret.append("\n");
	}

	return ret;
}
