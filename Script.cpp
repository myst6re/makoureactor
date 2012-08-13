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

Script::Script(const QByteArray &script)
{
	valid = openScript(script);
}

Script::~Script()
{
	foreach(Commande *commande, commandes)	delete commande;
}

bool Script::openScript(const QByteArray &script)
{
	int pos = 0, scriptSize = script.size();
	quint8 opcode;
	Commande *op;
	QList<int> positions;
	QMultiMap<int, OpcodeJump *> indents;

	int commandeID=0;
	while(pos < scriptSize)
	{
		opcode = (quint8)script.at(pos);
		op = createOpcode(script, pos);
		commandes.append(op);
		positions.append(pos);
		qDebug() << "pos" << pos << "index" << commandeID++;
		if(op->isJump()) {
			indents.insert(pos + ((OpcodeJump *)op)->jump(), (OpcodeJump *)op);
			qDebug() << "\tjump" << (pos + ((OpcodeJump *)op)->jump());
		}
		pos += op->size();
	}

	QList<int> indentsKeys = indents.uniqueKeys();

	qDebug() << "========";

	for(int i=indentsKeys.size()-1 ; i >= 0 ; --i) {
		int jump = indentsKeys.at(i);
		int index;
		if((index = positions.indexOf(jump)) != -1) {
			commandes.insert(index, new OpcodeLabel(i+1));
			qDebug() << "Jump" << jump << "index" << index << "label" << (i+1);

			foreach(OpcodeJump *opJump, indents.values(jump)) {
				qDebug() << "\tsetLabel" << (i+1);
				opJump->setLabel(i+1);
			}
		} else {
			qDebug() << "Error" << jump << "label" << (i+1);
//			return false;//TODO
		}
	}

	qDebug() << "_______";

	return true;
}

Commande *Script::createOpcode(const QByteArray &script, int pos)
{
	quint8 opcode = (quint8)script.at(pos);
	quint8 size = Opcode::length[opcode] - 1;

	switch(opcode)
	{
	case 0x00:	return new OpcodeRET();
	case 0x01:	return new OpcodeREQ(script.mid(pos+1, size));
	case 0x02:	return new OpcodeREQSW(script.mid(pos+1, size));
	case 0x03:	return new OpcodeREQEW(script.mid(pos+1, size));
	case 0x04:	return new OpcodePREQ(script.mid(pos+1, size));
	case 0x05:	return new OpcodePRQSW(script.mid(pos+1, size));
	case 0x06:	return new OpcodePRQEW(script.mid(pos+1, size));
	case 0x07:	return new OpcodeRETTO(script.mid(pos+1, size));
	case 0x08:	return new OpcodeJOIN(script.mid(pos+1, size));
	case 0x09:	return new OpcodeSPLIT(script.mid(pos+1, size));
	case 0x0A:	return new OpcodeSPTYE(script.mid(pos+1, size));
	case 0x0B:	return new OpcodeGTPYE(script.mid(pos+1, size));
	case 0x0E:	return new OpcodeDSKCG(script.mid(pos+1, size));
	case 0x0F://SPECIAL
		switch((quint8)script.at(pos+1)) {
		case 0xF5:case 0xF6:case 0xF7:case 0xFB:case 0xFC:
			size += 1;
			break;
		case 0xF8:case 0xFD:
			size += 2;
			break;
		}
		return new OpcodeSPECIAL(script.mid(pos+1, size));
	case 0x10:	return new OpcodeJMPF(script.mid(pos+1, size));
	case 0x11:	return new OpcodeJMPFL(script.mid(pos+1, size));
	case 0x12:	return new OpcodeJMPB(script.mid(pos+1, size));
	case 0x13:	return new OpcodeJMPBL(script.mid(pos+1, size));
	case 0x14:	return new OpcodeIFUB(script.mid(pos+1, size));
	case 0x15:	return new OpcodeIFUBL(script.mid(pos+1, size));
	case 0x16:	return new OpcodeIFSW(script.mid(pos+1, size));
	case 0x17:	return new OpcodeIFSWL(script.mid(pos+1, size));
	case 0x18:	return new OpcodeIFUW(script.mid(pos+1, size));
	case 0x19:	return new OpcodeIFUWL(script.mid(pos+1, size));
	case 0x20:	return new OpcodeMINIGAME(script.mid(pos+1, size));
	case 0x21:	return new OpcodeTUTOR(script.mid(pos+1, size));
	case 0x22:	return new OpcodeBTMD2(script.mid(pos+1, size));
	case 0x23:	return new OpcodeBTRLD(script.mid(pos+1, size));
	case 0x24:	return new OpcodeWAIT(script.mid(pos+1, size));
	case 0x25:	return new OpcodeNFADE(script.mid(pos+1, size));
	case 0x26:	return new OpcodeBLINK(script.mid(pos+1, size));
	case 0x27:	return new OpcodeBGMOVIE(script.mid(pos+1, size));
	case 0x28://KAWAI
		size += (quint8)script.at(pos+1);
		return new OpcodeKAWAI(script.mid(pos+1, size));
	case 0x29:	return new OpcodeKAWIW();
	case 0x2A:	return new OpcodePMOVA(script.mid(pos+1, size));
	case 0x2B:	return new OpcodeSLIP(script.mid(pos+1, size));
	case 0x2C:	return new OpcodeBGPDH(script.mid(pos+1, size));
	case 0x2D:	return new OpcodeBGSCR(script.mid(pos+1, size));
	case 0x2E:	return new OpcodeWCLS(script.mid(pos+1, size));
	case 0x2F:	return new OpcodeWSIZW(script.mid(pos+1, size));
	case 0x30:	return new OpcodeIFKEY(script.mid(pos+1, size));
	case 0x31:	return new OpcodeIFKEYON(script.mid(pos+1, size));
	case 0x32:	return new OpcodeIFKEYOFF(script.mid(pos+1, size));
	case 0x33:	return new OpcodeUC(script.mid(pos+1, size));
	case 0x34:	return new OpcodePDIRA(script.mid(pos+1, size));
	case 0x35:	return new OpcodePTURA(script.mid(pos+1, size));
	case 0x36:	return new OpcodeWSPCL(script.mid(pos+1, size));
	case 0x37:	return new OpcodeWNUMB(script.mid(pos+1, size));
	case 0x38:	return new OpcodeSTTIM(script.mid(pos+1, size));
	case 0x39:	return new OpcodeGOLDu(script.mid(pos+1, size));
	case 0x3A:	return new OpcodeGOLDd(script.mid(pos+1, size));
	case 0x3B:	return new OpcodeCHGLD(script.mid(pos+1, size));
	case 0x3C:	return new OpcodeHMPMAX1();
	case 0x3D:	return new OpcodeHMPMAX2();
	case 0x3E:	return new OpcodeMHMMX();
	case 0x3F:	return new OpcodeHMPMAX3();
	case 0x40:	return new OpcodeMESSAGE(script.mid(pos+1, size));
	case 0x41:	return new OpcodeMPARA(script.mid(pos+1, size));
	case 0x42:	return new OpcodeMPRA2(script.mid(pos+1, size));
	case 0x43:	return new OpcodeMPNAM(script.mid(pos+1, size));

	case 0x45:	return new OpcodeMPu(script.mid(pos+1, size));

	case 0x47:	return new OpcodeMPd(script.mid(pos+1, size));
	case 0x48:	return new OpcodeASK(script.mid(pos+1, size));
	case 0x49:	return new OpcodeMENU(script.mid(pos+1, size));
	case 0x4A:	return new OpcodeMENU2(script.mid(pos+1, size));
	case 0x4B:	return new OpcodeBTLTB(script.mid(pos+1, size));

	case 0x4D:	return new OpcodeHPu(script.mid(pos+1, size));

	case 0x4F:	return new OpcodeHPd(script.mid(pos+1, size));
	case 0x50:	return new OpcodeWINDOW(script.mid(pos+1, size));
	case 0x51:	return new OpcodeWMOVE(script.mid(pos+1, size));
	case 0x52:	return new OpcodeWMODE(script.mid(pos+1, size));
	case 0x53:	return new OpcodeWREST(script.mid(pos+1, size));
	case 0x54:	return new OpcodeWCLSE(script.mid(pos+1, size));
	case 0x55:	return new OpcodeWROW(script.mid(pos+1, size));
	case 0x56:	return new OpcodeGWCOL(script.mid(pos+1, size));
	case 0x57:	return new OpcodeSWCOL(script.mid(pos+1, size));
	case 0x58:	return new OpcodeSTITM(script.mid(pos+1, size));
	case 0x59:	return new OpcodeDLITM(script.mid(pos+1, size));
	case 0x5A:	return new OpcodeCKITM(script.mid(pos+1, size));
	case 0x5B:	return new OpcodeSMTRA(script.mid(pos+1, size));
	case 0x5C:	return new OpcodeDMTRA(script.mid(pos+1, size));
	case 0x5D:	return new OpcodeCMTRA(script.mid(pos+1, size));
	case 0x5E:	return new OpcodeSHAKE(script.mid(pos+1, size));
	case 0x5F:	return new OpcodeNOP();
	case 0x60:	return new OpcodeMAPJUMP(script.mid(pos+1, size));
	case 0x61:	return new OpcodeSCRLO(script.mid(pos+1, size));
	case 0x62:	return new OpcodeSCRLC(script.mid(pos+1, size));
	case 0x63:	return new OpcodeSCRLA(script.mid(pos+1, size));
	case 0x64:	return new OpcodeSCR2D(script.mid(pos+1, size));
	case 0x65:	return new OpcodeSCRCC();
	case 0x66:	return new OpcodeSCR2DC(script.mid(pos+1, size));
	case 0x67:	return new OpcodeSCRLW();
	case 0x68:	return new OpcodeSCR2DL(script.mid(pos+1, size));
	case 0x69:	return new OpcodeMPDSP(script.mid(pos+1, size));
	case 0x6A:	return new OpcodeVWOFT(script.mid(pos+1, size));
	case 0x6B:	return new OpcodeFADE(script.mid(pos+1, size));
	case 0x6C:	return new OpcodeFADEW();
	case 0x6D:	return new OpcodeIDLCK(script.mid(pos+1, size));
	case 0x6E:	return new OpcodeLSTMP(script.mid(pos+1, size));
	case 0x6F:	return new OpcodeSCRLP(script.mid(pos+1, size));
	case 0x70:	return new OpcodeBATTLE(script.mid(pos+1, size));
	case 0x71:	return new OpcodeBTLON(script.mid(pos+1, size));
	case 0x72:	return new OpcodeBTLMD(script.mid(pos+1, size));
	case 0x73:	return new OpcodePGTDR(script.mid(pos+1, size));
	case 0x74:	return new OpcodeGETPC(script.mid(pos+1, size));
	case 0x75:	return new OpcodePXYZI(script.mid(pos+1, size));
	case 0x76:	return new OpcodePLUSX(script.mid(pos+1, size));
	case 0x77:	return new OpcodePLUS2X(script.mid(pos+1, size));
	case 0x78:	return new OpcodeMINUSX(script.mid(pos+1, size));
	case 0x79:	return new OpcodeMINUS2X(script.mid(pos+1, size));
	case 0x7A:	return new OpcodeINCX(script.mid(pos+1, size));
	case 0x7B:	return new OpcodeINC2X(script.mid(pos+1, size));
	case 0x7C:	return new OpcodeDECX(script.mid(pos+1, size));
	case 0x7D:	return new OpcodeDEC2X(script.mid(pos+1, size));
	case 0x7E:	return new OpcodeTLKON(script.mid(pos+1, size));
	case 0x7F:	return new OpcodeRDMSD(script.mid(pos+1, size));
	case 0x80:	return new OpcodeSETBYTE(script.mid(pos+1, size));
	case 0x81:	return new OpcodeSETWORD(script.mid(pos+1, size));
	case 0x82:	return new OpcodeBITON(script.mid(pos+1, size));
	case 0x83:	return new OpcodeBITOFF(script.mid(pos+1, size));
	case 0x84:	return new OpcodeBITXOR(script.mid(pos+1, size));
	case 0x85:	return new OpcodePLUS(script.mid(pos+1, size));
	case 0x86:	return new OpcodePLUS2(script.mid(pos+1, size));
	case 0x87:	return new OpcodeMINUS(script.mid(pos+1, size));
	case 0x88:	return new OpcodeMINUS2(script.mid(pos+1, size));
	case 0x89:	return new OpcodeMUL(script.mid(pos+1, size));
	case 0x8A:	return new OpcodeMUL2(script.mid(pos+1, size));
	case 0x8B:	return new OpcodeDIV(script.mid(pos+1, size));
	case 0x8C:	return new OpcodeDIV2(script.mid(pos+1, size));
	case 0x8D:	return new OpcodeMOD(script.mid(pos+1, size));
	case 0x8E:	return new OpcodeMOD2(script.mid(pos+1, size));
	case 0x8F:	return new OpcodeAND(script.mid(pos+1, size));
	case 0x90:	return new OpcodeAND2(script.mid(pos+1, size));
	case 0x91:	return new OpcodeOR(script.mid(pos+1, size));
	case 0x92:	return new OpcodeOR2(script.mid(pos+1, size));
	case 0x93:	return new OpcodeXOR(script.mid(pos+1, size));
	case 0x94:	return new OpcodeXOR2(script.mid(pos+1, size));
	case 0x95:	return new OpcodeINC(script.mid(pos+1, size));
	case 0x96:	return new OpcodeINC2(script.mid(pos+1, size));
	case 0x97:	return new OpcodeDEC(script.mid(pos+1, size));
	case 0x98:	return new OpcodeDEC2(script.mid(pos+1, size));
	case 0x99:	return new OpcodeRANDOM(script.mid(pos+1, size));
	case 0x9A:	return new OpcodeLBYTE(script.mid(pos+1, size));
	case 0x9B:	return new OpcodeHBYTE(script.mid(pos+1, size));
	case 0x9C:	return new Opcode2BYTE(script.mid(pos+1, size));
	case 0x9D:	return new OpcodeSETX(script.mid(pos+1, size));
	case 0x9E:	return new OpcodeGETX(script.mid(pos+1, size));
	case 0x9F:	return new OpcodeSEARCHX(script.mid(pos+1, size));
	case 0xA0:	return new OpcodePC(script.mid(pos+1, size));
	case 0xA1:	return new OpcodeCHAR(script.mid(pos+1, size));
	case 0xA2:	return new OpcodeDFANM(script.mid(pos+1, size));
	case 0xA3:	return new OpcodeANIME1(script.mid(pos+1, size));
	case 0xA4:	return new OpcodeVISI(script.mid(pos+1, size));
	case 0xA5:	return new OpcodeXYZI(script.mid(pos+1, size));
	case 0xA6:	return new OpcodeXYI(script.mid(pos+1, size));
	case 0xA7:	return new OpcodeXYZ(script.mid(pos+1, size));
	case 0xA8:	return new OpcodeMOVE(script.mid(pos+1, size));
	case 0xA9:	return new OpcodeCMOVE(script.mid(pos+1, size));
	case 0xAA:	return new OpcodeMOVA(script.mid(pos+1, size));
	case 0xAB:	return new OpcodeTURA(script.mid(pos+1, size));
	case 0xAC:	return new OpcodeANIMW();
	case 0xAD:	return new OpcodeFMOVE(script.mid(pos+1, size));
	case 0xAE:	return new OpcodeANIME2(script.mid(pos+1, size));
	case 0xAF:	return new OpcodeANIMX1(script.mid(pos+1, size));
	case 0xB0:	return new OpcodeCANIM1(script.mid(pos+1, size));
	case 0xB1:	return new OpcodeCANMX1(script.mid(pos+1, size));
	case 0xB2:	return new OpcodeMSPED(script.mid(pos+1, size));
	case 0xB3:	return new OpcodeDIR(script.mid(pos+1, size));
	case 0xB4:	return new OpcodeTURNGEN(script.mid(pos+1, size));
	case 0xB5:	return new OpcodeTURN(script.mid(pos+1, size));
	case 0xB6:	return new OpcodeDIRA(script.mid(pos+1, size));
	case 0xB7:	return new OpcodeGETDIR(script.mid(pos+1, size));
	case 0xB8:	return new OpcodeGETAXY(script.mid(pos+1, size));
	case 0xB9:	return new OpcodeGETAI(script.mid(pos+1, size));
	case 0xBA:	return new OpcodeANIMX2(script.mid(pos+1, size));
	case 0xBB:	return new OpcodeCANIM2(script.mid(pos+1, size));
	case 0xBC:	return new OpcodeCANMX2(script.mid(pos+1, size));
	case 0xBD:	return new OpcodeASPED(script.mid(pos+1, size));

	case 0xBF:	return new OpcodeCC(script.mid(pos+1, size));
	case 0xC0:	return new OpcodeJUMP(script.mid(pos+1, size));
	case 0xC1:	return new OpcodeAXYZI(script.mid(pos+1, size));
	case 0xC2:	return new OpcodeLADER(script.mid(pos+1, size));
	case 0xC3:	return new OpcodeOFST(script.mid(pos+1, size));
	case 0xC4:	return new OpcodeOFSTW();
	case 0xC5:	return new OpcodeTALKR(script.mid(pos+1, size));
	case 0xC6:	return new OpcodeSLIDR(script.mid(pos+1, size));
	case 0xC7:	return new OpcodeSOLID(script.mid(pos+1, size));
	case 0xC8:	return new OpcodePRTYP(script.mid(pos+1, size));
	case 0xC9:	return new OpcodePRTYM(script.mid(pos+1, size));
	case 0xCA:	return new OpcodePRTYE(script.mid(pos+1, size));
	case 0xCB:	return new OpcodeIFPRTYQ(script.mid(pos+1, size));
	case 0xCC:	return new OpcodeIFMEMBQ(script.mid(pos+1, size));
	case 0xCD:	return new OpcodeMMBUD(script.mid(pos+1, size));
	case 0xCE:	return new OpcodeMMBLK(script.mid(pos+1, size));
	case 0xCF:	return new OpcodeMMBUK(script.mid(pos+1, size));
	case 0xD0:	return new OpcodeLINE(script.mid(pos+1, size));
	case 0xD1:	return new OpcodeLINON(script.mid(pos+1, size));
	case 0xD2:	return new OpcodeMPJPO(script.mid(pos+1, size));
	case 0xD3:	return new OpcodeSLINE(script.mid(pos+1, size));
	case 0xD4:	return new OpcodeSIN(script.mid(pos+1, size));
	case 0xD5:	return new OpcodeCOS(script.mid(pos+1, size));
	case 0xD6:	return new OpcodeTLKR2(script.mid(pos+1, size));
	case 0xD7:	return new OpcodeSLDR2(script.mid(pos+1, size));
	case 0xD8:	return new OpcodePMJMP(script.mid(pos+1, size));
	case 0xD9:	return new OpcodePMJMP2();
	case 0xDA:	return new OpcodeAKAO2(script.mid(pos+1, size));
	case 0xDB:	return new OpcodeFCFIX(script.mid(pos+1, size));
	case 0xDC:	return new OpcodeCCANM(script.mid(pos+1, size));
	case 0xDD:	return new OpcodeANIMB();
	case 0xDE:	return new OpcodeTURNW();
	case 0xDF:	return new OpcodeMPPAL(script.mid(pos+1, size));
	case 0xE0:	return new OpcodeBGON(script.mid(pos+1, size));
	case 0xE1:	return new OpcodeBGOFF(script.mid(pos+1, size));
	case 0xE2:	return new OpcodeBGROL(script.mid(pos+1, size));
	case 0xE3:	return new OpcodeBGROL2(script.mid(pos+1, size));
	case 0xE4:	return new OpcodeBGCLR(script.mid(pos+1, size));
	case 0xE5:	return new OpcodeSTPAL(script.mid(pos+1, size));
	case 0xE6:	return new OpcodeLDPAL(script.mid(pos+1, size));
	case 0xE7:	return new OpcodeCPPAL(script.mid(pos+1, size));
	case 0xE8:	return new OpcodeRTPAL(script.mid(pos+1, size));
	case 0xE9:	return new OpcodeADPAL(script.mid(pos+1, size));
	case 0xEA:	return new OpcodeMPPAL2(script.mid(pos+1, size));
	case 0xEB:	return new OpcodeSTPLS(script.mid(pos+1, size));
	case 0xEC:	return new OpcodeLDPLS(script.mid(pos+1, size));
	case 0xED:	return new OpcodeCPPAL2(script.mid(pos+1, size));
	case 0xEE:	return new OpcodeRTPAL2(script.mid(pos+1, size));
	case 0xEF:	return new OpcodeADPAL2(script.mid(pos+1, size));
	case 0xF0:	return new OpcodeMUSIC(script.mid(pos+1, size));
	case 0xF1:	return new OpcodeSOUND(script.mid(pos+1, size));
	case 0xF2:	return new OpcodeAKAO(script.mid(pos+1, size));
	case 0xF3:	return new OpcodeMUSVT(script.mid(pos+1, size));
	case 0xF4:	return new OpcodeMUSVM(script.mid(pos+1, size));
	case 0xF5:	return new OpcodeMULCK(script.mid(pos+1, size));
	case 0xF6:	return new OpcodeBMUSC(script.mid(pos+1, size));
	case 0xF7:	return new OpcodeCHMPH(script.mid(pos+1, size));
	case 0xF8:	return new OpcodePMVIE(script.mid(pos+1, size));
	case 0xF9:	return new OpcodeMOVIE();
	case 0xFA:	return new OpcodeMVIEF(script.mid(pos+1, size));
	case 0xFB:	return new OpcodeMVCAM(script.mid(pos+1, size));
	case 0xFC:	return new OpcodeFMUSC(script.mid(pos+1, size));
	case 0xFD:	return new OpcodeCMUSC(script.mid(pos+1, size));
	case 0xFE:	return new OpcodeCHMST(script.mid(pos+1, size));
	case 0xFF:	return new OpcodeGAMEOVER();
	default:	return new OpcodeUnknown(opcode, script.mid(pos+1, size));
	}
}

int Script::posReturn(const QByteArray &script)
{
	quint16 pos=0, longueur, param16;
	int scriptSize = script.size();
	quint8 clef;
	
	if(scriptSize <= 0)	return -1;
	
	while(pos < scriptSize)
	{
		longueur = 0;
		switch(clef = (quint8)script.at(pos))
		{
		case 0x10://Saut
			pos += (quint8)script.at(pos+1) - 1;
			break;
		case 0x11://Saut
			memcpy(&param16, script.mid(pos+1,2), 2);
			pos += param16 - 1;
			break;
		case 0x14://If -> Saut
			pos += (quint8)script.at(pos+5) - 1;
			break;
		case 0x15://If -> Saut
			memcpy(&param16, script.mid(pos+5,2), 2);
			pos += param16 - 1;
			break;
		case 0x16:case 0x18://If -> Saut
					pos += (quint8)script.at(pos+7) - 1;
			break;
		case 0x17:case 0x19://If -> Saut
					memcpy(&param16, script.mid(pos+7,2), 2);
			pos += param16 - 1;
			break;
		case 0x0F://SPECIAL
			switch((quint8)script.at(pos+1))
			{
			case 0xF5:case 0xF6:case 0xF7:case 0xFB:case 0xFC:
						longueur = 1;
				break;
			case 0xF8:case 0xFD:
						longueur = 2;
				break;
			}
			break;
		case 0x28://KAWAI
			longueur = (quint8)script.at(pos+1);
			break;
		}
		pos += Opcode::length[clef] + longueur;
		if(clef == 0x00 || clef == 0x07)	break;
	}
	
	return pos;
}

int Script::size() const
{
	return commandes.size();
}

bool Script::isEmpty() const
{
	return commandes.isEmpty();
}

bool Script::isValid() const
{
	return valid;
}

Commande *Script::getCommande(quint16 commandeID)
{
	// if(commandeID >= commandes.size())	return NULL;
	return commandes.at(commandeID);
}

QByteArray Script::toByteArray() const
{
	QByteArray ret;
	foreach(Commande *commande, commandes) {
		ret.append(commande->toByteArray());
	}
	return ret;
}

bool Script::isVoid() const
{
	foreach(Commande *commande, commandes)
		if(!commande->isVoid())	return false;
	return true;
}

void Script::setCommande(quint16 commandeID, Commande *commande)
{
	Commande *curCommand = commandes.at(commandeID);
//	int oldSize = curCommand->size();
	commandes.replace(commandeID, commande);
	delete curCommand;
//	shiftJumps(commandeID, commande->size()-oldSize);
}

void Script::delCommande(quint16 commandeID)
{
//	int oldSize = commandes.at(commandeID)->size();
	delete commandes.takeAt(commandeID);
//	shiftJumps(commandeID, -oldSize);
}

Commande *Script::removeCommande(quint16 commandeID)
{
//	int oldSize = commandes.at(commandeID)->size();
	Commande *commande = commandes.takeAt(commandeID);
//	shiftJumps(commandeID, -oldSize);
	return commande;
}

void Script::insertCommande(quint16 commandeID, Commande *commande)
{
	commandes.insert(commandeID, commande);
//	shiftJumps(commandeID, commande->size());
}

bool Script::moveCommande(quint16 commandeID, bool direction)
{
	if(commandeID >= commandes.size())	return false;

	Commande *commande;
	
	if(direction) // DOWN
	{
		if(commandeID == commandes.size()-1)	return false;
		commande = removeCommande(commandeID);
		insertCommande(commandeID+1, commande);
//		shiftJumpsSwap(commande, -commandes.at(commandeID)->size());
	}
	else // UP
	{
		if(commandeID == 0)	return false;
		commande = removeCommande(commandeID);
		insertCommande(commandeID-1, commande);
//		shiftJumpsSwap(commande, commandes.at(commandeID)->size());
	}
	return true;
}

bool Script::rechercherOpCode(quint8 opCode, int &commandeID) const
{
	if(commandeID < 0)	commandeID = 0;

	int nbCommandes = commandes.size();
	while(commandeID < nbCommandes)
	{
		if(opCode == commandes.at(commandeID)->id())	return true;
		++commandeID;
		// qDebug() << "SCRIPT_commandeID " << commandeID;
	}
	return false;
}

bool Script::rechercherVar(quint8 bank, quint8 adress, int value, int &commandeID) const
{
	if(commandeID < 0)	commandeID = 0;

	int nbCommandes = commandes.size();
	while(commandeID < nbCommandes)
	{
		if(commandes.at(commandeID)->rechercherVar(bank, adress, value))	return true;
		++commandeID;
	}
	return false;
}

QList<FF7Var> Script::searchAllVars() const
{
	QList<FF7Var> vars;

	foreach(Commande *commande, commandes) {
		commande->getVariables(vars);
	}

	return vars;
}

bool Script::rechercherExec(quint8 group, quint8 script, int &commandeID) const
{
	if(commandeID < 0)	commandeID = 0;

	int nbCommandes = commandes.size();
	while(commandeID < nbCommandes)
	{
		if(commandes.at(commandeID)->rechercherExec(group, script))	return true;
		++commandeID;
	}
	return false;
}

bool Script::rechercherTexte(const QRegExp &texte, int &commandeID) const
{
	if(commandeID < 0)	commandeID = 0;

	int nbCommandes = commandes.size();
	while(commandeID < nbCommandes)
	{
		if(commandes.at(commandeID)->rechercherTexte(texte))	return true;
		++commandeID;
	}
	return false;
}

bool Script::rechercherOpCodeP(quint8 opCode, int &commandeID) const
{
	if(commandeID >= commandes.size())	commandeID = commandes.size()-1;
	
	while(commandeID >= 0)
	{
		if(opCode == commandes.at(commandeID)->id())	return true;
		--commandeID;
	}

	return false;
}

bool Script::rechercherVarP(quint8 bank, quint8 adress, int value, int &commandeID) const
{
	if(commandeID >= commandes.size())	commandeID = commandes.size()-1;
	
	while(commandeID >= 0)
	{
		if(commandes.at(commandeID)->rechercherVar(bank, adress, value))	return true;
		--commandeID;
	}

	return false;
}

bool Script::rechercherExecP(quint8 group, quint8 script, int &commandeID) const
{
	if(commandeID >= commandes.size())	commandeID = commandes.size()-1;

	while(commandeID >= 0)
	{
		if(commandes.at(commandeID)->rechercherExec(group, script))	return true;
		--commandeID;
	}

	return false;
}

bool Script::rechercherTexteP(const QRegExp &texte, int &commandeID) const
{
	if(commandeID >= commandes.size())	commandeID = commandes.size()-1;
	
	while(commandeID >= 0)
	{
		if(commandes.at(commandeID)->rechercherTexte(texte))	return true;
		--commandeID;
	}

	return false;
}

void Script::listUsedTexts(QSet<quint8> &usedTexts) const
{
	foreach(Commande *commande, commandes)
		commande->listUsedTexts(usedTexts);
}

void Script::listUsedTuts(QSet<quint8> &usedTuts) const
{
	foreach(Commande *commande, commandes)
		commande->listUsedTuts(usedTuts);
}

void Script::shiftTextIds(int textId, int steps)
{
	foreach(Commande *commande, commandes)
		commande->shiftTextIds(textId, steps);
}

void Script::shiftTutIds(int tutId, int steps)
{
	foreach(Commande *commande, commandes)
		commande->shiftTutIds(tutId, steps);
}

void Script::listWindows(QMultiMap<quint8, FF7Window> &windows, QMultiMap<quint8, quint8> &text2win) const
{
	foreach(Commande *commande, commandes)
		commande->listWindows(windows, text2win);
}

void Script::getBgParams(QHash<quint8, quint8> &paramActifs) const
{
	foreach(Commande *commande, commandes)
		commande->getBgParams(paramActifs);
}

void Script::getBgMove(qint16 z[2], qint16 *x, qint16 *y) const
{
	foreach(Commande *commande, commandes)
		commande->getBgMove(z, x, y);
}

/*int Script::posOfCommand(int commandID) const
{
	int currentCommandID=0, pos=0;

	foreach(Commande *commande, commandes)
	{
		if(commandID==currentCommandID)
		{
			return pos;
		}
		++currentCommandID;
		pos += commande->size();
	}

	return pos;
}

void Script::shiftJumps(int commandID, int shift)
{
	if(shift == 0)	return;

	int currentCommandID=0, pos=posOfCommand(commandID), currentPos=0, jump;

	foreach(Commande *commande, commandes)
	{
		if(currentCommandID < commandID)
		{
			switch(commande->id())
			{
			case 0x10:// JMPF (Jump)
				jump = commande->getConstParams().at(0);
				if(currentPos + 1 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[0] = jump;
//						qDebug() << "JMPF" << (jump-shift) << jump;
					} else if(jump >= 0 && jump < 65536)// -> JMPFL
					{
						commande->setCommande(QByteArray().append('\x11').append((char *)&jump, 2));
//						qDebug() << "JMPF -> JMPFL" << (jump-shift) << jump;
					}
				}
				break;
			case 0x11:// JMPFL (Long Jump)
				jump=0;
				memcpy(&jump, commande->getConstParams().mid(0,2).constData(), 2);
				if(currentPos + 1 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 65536) {
						memcpy(commande->getParams().data(), &jump, 2);
//						qDebug() << "JMPFL" << (jump-shift) << jump;
					}
				}
				break;
			case 0x14:// IFUB (Jump)
				jump = commande->getConstParams().at(4);
				if(currentPos + 5 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[4] = jump;
//						qDebug() << "IFUB" << (jump-shift) << jump;
					}
					else if(jump >= 0 && jump < 65536)// -> IFUBL
					{
						commande->setCommande(QByteArray().append('\x15').append(commande->getConstParams().left(4)).append((char *)&jump, 2));
//						qDebug() << "IFUB -> IFUBL" << (jump-shift) << jump;
					}
				}
				break;
			case 0x15:// IFUBL (Long Jump)
				jump=0;
				memcpy(&jump, commande->getConstParams().mid(4,2).constData(), 2);
				if(currentPos + 5 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 65536) {
						memcpy(commande->getParams().data() + 4, &jump, 2);
//						qDebug() << "IFUBL" << (jump-shift) << jump;
					}
				}
				break;
			case 0x16:// IFSW (Jump)
			case 0x18:// IFUW (Jump)
				jump = commande->getConstParams().at(6);
				if(currentPos + 7 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[6] = jump;
//						qDebug() << (commande->id()==0x16 ? "IFSW" : "IFUW") << (jump-shift) << jump;
					} else if(jump >= 0 && jump < 65536)// -> IFSWL/IFUWL
					{
						commande->setCommande(QByteArray().append(commande->id()==0x16 ? '\x17' : '\x19').append(commande->getConstParams().left(6)).append((char *)&jump, 2));
//						qDebug() << (commande->id()==0x16 ? "IFSW -> IFSWL" : "IFUW -> IFUWL") << (jump-shift) << jump;
					}
				}
				break;
			case 0x17:// IFSWL (Long Jump)
			case 0x19:// IFUWL (Long Jump)
				jump=0;
				memcpy(&jump, commande->getConstParams().mid(6,2).constData(), 2);
				if(currentPos + 7 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 65536) {
						memcpy(commande->getParams().data() + 6, &jump, 2);
//						qDebug() << (commande->id()==0x17 ? "IFSWL" : "IFUWL") << (jump-shift) << jump;
					}
				}
				break;
			case 0x30:// IFKEY (Jump)
			case 0x31:// IFKEYON (Jump)
			case 0x32:// IFKEYOFF (Jump)
				jump = commande->getConstParams().at(2);
				if(currentPos + 3 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[2] = jump;
//						qDebug() << (commande->id()==0x30 ? "IFKEY" : (commande->id()==0x31 ? "IFKEYON" : "IFKEYOFF")) << (jump-shift) << jump;
					}
				}
				break;
			case 0xCB:// IFPRTYQ (Jump)
			case 0xCC:// IFMEMBQ (Jump)
				jump = commande->getConstParams().at(1);
				if(currentPos + 2 + jump > pos) {
					jump += shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[1] = jump;
//						qDebug() << (commande->id()==0xCB ? "IFPRTYQ" : "IFMEMBQ") << (jump-shift) << jump;
					}
				}
				break;
			}
		}
		else
		{
			switch(commande->id())
			{
			case 0x12:// JMPB (Jump Forward)
				jump = commande->getConstParams().at(0);
				if(currentPos - jump > pos) {
					jump -= shift;
					if(jump >= 0 && jump < 256) {
						commande->getParams()[0] = jump;
//						qDebug() << "JMPB" << (jump-shift) << jump;
					}
					else if(jump >= 0 && jump < 65536)// -> JMPBL
					{
						commande->setCommande(QByteArray().append('\x13').append((char *)&jump, 2));
//						qDebug() << "JMPB -> JMPBL" << (jump-shift) << jump;
					}
				}
				break;
			case 0x13:// JMPBL (Long Jump Forward)
				jump=0;
				memcpy(&jump, commande->getConstParams().mid(0,2).constData(), 2);
				if(currentPos - jump > pos) {
					jump -= shift;
					if(jump >= 0 && jump < 65536) {
						memcpy(commande->getParams().data(), &jump, 2);
//						qDebug() << "JMPBL" << (jump-shift) << jump;
					}
				}
				break;
			}
		}
		++currentCommandID;
		currentPos += commande->size();
	}
}

void Script::shiftJumpsSwap(Commande *commande, int shift)
{
	if(shift == 0)	return;

	int jump;

	switch(commande->id())
	{
	case 0x10:// JMPF (Jump)
		jump = commande->getConstParams().at(0) + shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[0] = jump;
//			qDebug() << "JMPF" << (jump-shift) << jump;
		} else if(jump >= 0 && jump < 65536)// -> JMPFL
		{
			commande->setCommande(QByteArray().append('\x11').append((char *)&jump, 2));
//			qDebug() << "JMPF -> JMPFL" << (jump-shift) << jump;
		}
		break;
	case 0x11:// JMPFL (Long Jump)
		jump=0;
		memcpy(&jump, commande->getConstParams().mid(0,2).constData(), 2);
		jump += shift;
		if(jump >= 0 && jump < 65536) {
			memcpy(commande->getParams().data(), &jump, 2);
//			qDebug() << "JMPFL" << (jump-shift) << jump;
		}
		break;
	case 0x12:// JMPB (Jump Forward)
		jump = commande->getConstParams().at(0) - shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[0] = jump;
//			qDebug() << "JMPB" << (jump-shift) << jump;
		}
		else if(jump >= 0 && jump < 65536)// -> JMPBL
		{
			commande->setCommande(QByteArray().append('\x13').append((char *)&jump, 2));
//			qDebug() << "JMPB -> JMPBL" << (jump-shift) << jump;
		}
		break;
	case 0x13:// JMPBL (Long Jump Forward)
		jump=0;
		memcpy(&jump, commande->getConstParams().mid(0,2).constData(), 2);
		jump -= shift;
		if(jump >= 0 && jump < 65536) {
			memcpy(commande->getParams().data(), &jump, 2);
//			qDebug() << "JMPBL" << (jump-shift) << jump;
		}
		break;
	case 0x14:// IFUB (Jump)
		jump = commande->getConstParams().at(4) + shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[4] = jump;
//			qDebug() << "IFUB" << (jump-shift) << jump;
		}
		else if(jump >= 0 && jump < 65536)// -> IFUBL
		{
			commande->setCommande(QByteArray().append('\x15').append(commande->getConstParams().left(4)).append((char *)&jump, 2));
//			qDebug() << "IFUB -> IFUBL" << (jump-shift) << jump;
		}
		break;
	case 0x15:// IFUBL (Long Jump)
		jump=0;
		memcpy(&jump, commande->getConstParams().mid(4,2).constData(), 2);
		jump += shift;
		if(jump >= 0 && jump < 65536) {
			memcpy(commande->getParams().data() + 4, &jump, 2);
//			qDebug() << "IFUBL" << (jump-shift) << jump;
		}
		break;
	case 0x16:// IFSW (Jump)
	case 0x18:// IFUW (Jump)
		jump = commande->getConstParams().at(6) + shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[6] = jump;
//			qDebug() << (commande->id()==0x16 ? "IFSW" : "IFUW") << (jump-shift) << jump;
		} else if(jump >= 0 && jump < 65536)// -> IFSWL/IFUWL
		{
			commande->setCommande(QByteArray().append(commande->id()==0x16 ? '\x17' : '\x19').append(commande->getConstParams().left(6)).append((char *)&jump, 2));
//			qDebug() << (commande->id()==0x16 ? "IFSW -> IFSWL" : "IFUW -> IFUWL") << (jump-shift) << jump;
		}
		break;
	case 0x17:// IFSWL (Long Jump)
	case 0x19:// IFUWL (Long Jump)
		jump=0;
		memcpy(&jump, commande->getConstParams().mid(6,2).constData(), 2);
		jump += shift;
		if(jump >= 0 && jump < 65536) {
			memcpy(commande->getParams().data() + 6, &jump, 2);
//			qDebug() << (commande->id()==0x17 ? "IFSWL" : "IFUWL") << (jump-shift) << jump;
		}
		break;
	case 0x30:// IFKEY (Jump)
	case 0x31:// IFKEYON (Jump)
	case 0x32:// IFKEYOFF (Jump)
		jump = commande->getConstParams().at(2) + shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[2] = jump;
//			qDebug() << (commande->id()==0x30 ? "IFKEY" : (commande->id()==0x31 ? "IFKEYON" : "IFKEYOFF")) << (jump-shift) << jump;
		}
		break;
	case 0xCB:// IFPRTYQ (Jump)
	case 0xCC:// IFMEMBQ (Jump)
		jump = commande->getConstParams().at(1) + shift;
		if(jump >= 0 && jump < 256) {
			commande->getParams()[1] = jump;
//			qDebug() << (commande->id()==0xCB ? "IFPRTYQ" : "IFMEMBQ") << (jump-shift) << jump;
		}
		break;
	}
}*/

void Script::lecture(QTreeWidget *zoneScript)
{
	QList<quint16> indent;
	QList<QTreeWidgetItem *> row;
	QTreeWidgetItem *parentItem = 0;
	QString description;
	quint8 clef;
	quint16 commandeID = 0, nbCommandes = commandes.size();
	Commande *curCommande;
	QPixmap fontPixmap(":/images/chiffres.png");
	bool error = false;
	
	while(commandeID < nbCommandes)
	{
		curCommande = commandes.at(commandeID);

		if(curCommande->isLabel()) {
			while(!indent.isEmpty() &&
				  ((OpcodeLabel *)curCommande)->label() >= indent.last())
			{
				if(((OpcodeLabel *)curCommande)->label() > indent.last())
					error = true;
				indent.removeLast();
				parentItem = parentItem->parent();
			}
		}
		
		description = curCommande->toString();
		clef = curCommande->id();
		
		/* description += QString(" | %1 || ").arg(clef,0,16);
		for(quint8 cur=0 ; cur<curCommande->getConstParams().size() ; ++cur)
		{
			description += QString("%1 | ").arg((quint8)curCommande->getConstParams().at(cur));
		} */
		
		/* description += " | ";
		for(quint8 cur=0 ; cur<indent.size() ; ++cur)
		{
			description += QString("%1 | ").arg(indent.at(cur));
		} */ //DEBUG
		QTreeWidgetItem *item = new QTreeWidgetItem(parentItem, QStringList() << description << QString::number(commandeID));
		row.append(item);
		
		// pos += Data::opcode_length[clef];
		
		//ERREUR
		/* if(pos>script.size())
		{
			description += " " + QObject::tr("Erreur : dépassement") + "";
			item->setBackground(0, QColor(0xFF,0xCC,0xCC));
		} */
		QPixmap wordPixmap(32,11);
		item->setIcon(0, QIcon(posNumber(commandeID, fontPixmap, wordPixmap)));
		item->setToolTip(0, curCommande->name());
		if((clef>=0x14 && clef<=0x19) || (clef>=0x30 && clef<=0x32) || clef==0xcb || clef==0xcc)
		{
			item->setForeground(0, QColor(0x00,0x66,0xcc));
			indent.append(((OpcodeJump *)curCommande)->label());
			parentItem = item;
		}
		else if(clef>=0x01 && clef<=0x07)
			item->setForeground(0, QColor(0xcc,0x66,0x00));
		else if(clef>=0x10 && clef<=0x13)
			item->setForeground(0, QColor(0x66,0xcc,0x00));
		else if(clef==0x00 || clef==0x07 || curCommande->isLabel())
			item->setForeground(0, QColor(0x66,0x66,0x66));

		if(error)
		{
			item->setBackground(0, QColor(0xFF,0xCC,0xCC));
			error = false;
		}

		++commandeID;
	}

	zoneScript->addTopLevelItems(row);

	commandeID = 0;
	foreach(QTreeWidgetItem *item, row) {
		if(expandedItems.contains(commandes.at(commandeID))) {
			item->setExpanded(true);
		}
		++commandeID;
	}
}

void Script::setExpandedItems(const QList<Commande *> &expandedItems)
{
	this->expandedItems = expandedItems;
}

QPixmap &Script::posNumber(int num, const QPixmap &fontPixmap, QPixmap &wordPixmap)
{
	QString strNum = QString("%1").arg(num, 5, 10, QChar(' '));
	wordPixmap.fill(QColor(0,0,0,0));
	QPainter painter(&wordPixmap);
	
	if(strNum.at(0)!=' ')
		painter.drawTiledPixmap(1, 1, 5, 9, fontPixmap, 5*strNum.mid(0,1).toInt(), 0);
	if(strNum.at(1)!=' ')
		painter.drawTiledPixmap(7, 1, 5, 9, fontPixmap, 5*strNum.mid(1,1).toInt(), 0);
	if(strNum.at(2)!=' ')
		painter.drawTiledPixmap(13, 1, 5, 9, fontPixmap, 5*strNum.mid(2,1).toInt(), 0);
	if(strNum.at(3)!=' ')
		painter.drawTiledPixmap(19, 1, 5, 9, fontPixmap, 5*strNum.mid(3,1).toInt(), 0);
	if(strNum.at(4)!=' ')
		painter.drawTiledPixmap(25, 1, 5, 9, fontPixmap, 5*strNum.mid(4,1).toInt(), 0);

	painter.end();
	return wordPixmap;
}
