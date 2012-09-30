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
/***************************************************************
	4/6/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN		SCIENCE
		NIFTY-Serve	PAF01022
		CompuServe	74050,1022
**************************************************************/
#include "LZS.h"

qint32 LZS::match_length=0;//of longest match. These are set by the InsertNode() procedure.
qint32 LZS::match_position=0;
qint32 LZS::lson[4097];//left & right children & parents -- These constitute binary search trees.
qint32 LZS::rson[4353];
qint32 LZS::dad[4097];
unsigned char LZS::text_buf[4113];//ring buffer of size 4096, with extra 17 bytes to facilitate string comparison
QByteArray LZS::result;

const QByteArray &LZS::decompress(const QByteArray &data, int max)
{
	return decompress(data.constData(), data.size(), max);
}

const QByteArray &LZS::decompress(const char *data, int fileSize, int max)
{
	int curResult=0, sizeAlloc=max+10;
	quint16 curBuff=4078, adresse, premOctet=0, i, length;
	const quint8 *fileData = (const quint8 *)data;
	const quint8 *endFileData = &fileData[fileSize-1] + 1;

	if(result.size() < sizeAlloc)
		result.resize(sizeAlloc);

	memset(text_buf, 0, 4078);//Le buffer de 4096 octets est initialisé à 0

	forever
	{
		if(((premOctet >>= 1) & 256) == 0) {
			premOctet = *fileData++ | 0xff00;//On récupère le premier octet puis on avance d'un octet
		}

		if(fileData >= endFileData || curResult >= max) {
			result.truncate(curResult);
			return result;//Fini !
		}

		if(premOctet & 1)
		{
			result[curResult] = text_buf[curBuff] = *fileData++;//On récupère l'octet (qui n'est pas compressé) et on le sauvegarde dans la chaine finale (result) et dans le buffer. Et bien sûr on fait avancer les curseurs d'un octet.
			curBuff = (curBuff + 1) & 4095;//Le curseur du buffer doit toujours être entre 0 et 4095
			++curResult;
		}
		else
		{
//			quint16 twoBytes = *((const quint16 *)fileData);
//			adresse = (twoBytes & 0x00FF) | ((twoBytes & 0xF000) >> 4);
//			length = ((twoBytes & 0x0F00) >> 8) + 2 + adresse;
//			fileData += 2;

			adresse = *fileData++;
			length = *fileData++;
			adresse |= (length & 0xF0) << 4;//on récupère l'adresse dans les deux octets (qui sont "compressés")
			length = (length & 0xF) + 2 + adresse;

			for(i=adresse ; i<=length ; ++i)
			{
				result[curResult] = text_buf[curBuff] = text_buf[i & 4095];//On va chercher l'octet (qui est décompressé) dans le buffer à l'adresse indiquée puis on le sauvegarde dans la chaine finale et le buffer.
				curBuff = (curBuff + 1) & 4095;
				++curResult;
			}
		}
	}
}

const QByteArray &LZS::decompressAll(const QByteArray &data)
{
	return decompressAll(data.constData(), data.size());
}

const QByteArray &LZS::decompressAll(const char *data, int fileSize)
{
	int curResult=0, sizeAlloc=fileSize*5;
	quint16 curBuff=4078, adresse, premOctet=0, i, length;
	const quint8 *fileData = (const quint8 *)data;
	const quint8 *endFileData = &fileData[fileSize-1] + 1;

	if(result.size() < sizeAlloc)
		result.resize(sizeAlloc);

	memset(text_buf, 0, 4078);//Le buffer de 4096 octets est initialisé à 0

	forever
	{
		if(((premOctet >>= 1) & 256) == 0) {
			premOctet = *fileData++ | 0xff00;//On récupère le premier octet puis on avance d'un octet
		}

		if(fileData >= endFileData) {
			result.truncate(curResult);
			return result;//Fini !
		}

		if(premOctet & 1)
		{
			result[curResult] = text_buf[curBuff] = *fileData++;//On récupère l'octet (qui n'est pas compressé) et on le sauvegarde dans la chaine finale (result) et dans le buffer. Et bien sûr on fait avancer les curseurs d'un octet.
			curBuff = (curBuff + 1) & 4095;//Le curseur du buffer doit toujours être entre 0 et 4095
			++curResult;
		}
		else
		{
//			quint16 twoBytes = *((const quint16 *)fileData);
//			adresse = (twoBytes & 0x00FF) | ((twoBytes & 0xF000) >> 4);
//			length = ((twoBytes & 0x0F00) >> 8) + 2 + adresse;
//			fileData += 2;

			adresse = *fileData++;
			length = *fileData++;
			adresse |= (length & 0xF0) << 4;//on récupère l'adresse dans les deux octets (qui sont "compressés")
			length = (length & 0xF) + 2 + adresse;

			for(i=adresse ; i<=length ; ++i)
			{
				result[curResult] = text_buf[curBuff] = text_buf[i & 4095];//On va chercher l'octet (qui est décompressé) dans le buffer à l'adresse indiquée puis on le sauvegarde dans la chaine finale et le buffer.
				curBuff = (curBuff + 1) & 4095;
				++curResult;
			}
		}
	}
}

void LZS::InsertNode(qint32 r)
{
	/* Inserts string of length 18, text_buf[r..r+18-1], into one of the trees (text_buf[r]'th tree) and returns the longest-match position and length via the global variables match_position and match_length.
	If match_length = 18, then removes the old node in favor of the new one, because the old one will be deleted sooner.
	Note r plays double role, as tree node and position in buffer. */
	
	qint32 i, p, cmp;
	unsigned char *key;

	cmp = 1;
	key = &text_buf[r];
	p = 4097 + key[0];

	rson[r] = lson[r] = 4096;
	match_length = 0;

	forever
	{
		if (cmp >= 0)
		{
			if(rson[p] != 4096)
			{
				p = rson[p];
			}
			else
			{
				rson[p] = r;
				dad[r] = p;
				return;
			}
		}
		else
		{
			if(lson[p] != 4096)
			{
				p = lson[p];
			}
			else
			{
				lson[p] = r;
				dad[r] = p;
				return;
			}
		}
		
		for(i=1 ; i<18 ; i++)
		{
			if((cmp = key[i] - text_buf[p + i]) != 0)	break;
		}

		if(i > match_length)
		{
			match_position = p;
			if((match_length = i) >= 18)	break;
		}
	}

	dad[r] = dad[p];
	lson[r] = lson[p];
	rson[r] = rson[p];

	dad[lson[p]] = r;
	dad[rson[p]] = r;

	if(rson[dad[p]] == p)
	{
		rson[dad[p]] = r;
	}
	else
	{
		lson[dad[p]] = r;
	}
	dad[p] = 4096;//remove p
}

void LZS::DeleteNode(qint32 p)//deletes node p from tree
{
	qint32 q;
	if(dad[p] == 4096)	return;//not in tree

	if(rson[p] == 4096)
	{
		q = lson[p];
	}
	else if(lson[p] == 4096)
	{
		q = rson[p];
	}
	else
	{
		q = lson[p];
		if(rson[q] != 4096)
		{
			do
			{
				q = rson[q];
			}
			while(rson[q] != 4096);
			
			rson[dad[q]] = lson[q];
			dad[lson[q]] = dad[q];
			lson[q] = lson[p];
			dad[lson[p]] = q;
		}
		rson[q] = rson[p];
		dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	
	if(rson[dad[p]] == p)
	{
		rson[dad[p]] = q;
	}
	else
	{
		lson[dad[p]] = q;
	}
	dad[p] = 4096;
}

const QByteArray &LZS::compress(const QByteArray &fileData)
{
	return compress(fileData.constData(), fileData.size());
}

const QByteArray &LZS::compress(const char *data, int sizeData)
{
	int i, c, len, r, s, last_match_length, code_buf_ptr,
			curResult = 0, sizeAlloc = sizeData / 2;
	unsigned char code_buf[17], mask;
	const char *dataEnd = &data[sizeData-1] + 1;

	if(result.size() < sizeAlloc) {
		result.resize(sizeAlloc);
	}
	
	/* quint32
		textsize = 0,//text size counter
		codesize = 0;//code size counter */

	//initialize trees
	/* For i = 0 to 4095, rson[i] and lson[i] will be the right and left children of node i. These nodes need not be initialized.
	Also, dad[i] is the parent of node i. These are initialized to 4096 which stands for 'not used.'
	For i = 0 to 255, rson[4097 + i] is the root of the tree for strings that begin with character i. These are initialized to 4096. Note there are 256 trees. */

	for(i=4097 ; i<=4352 ; ++i)	rson[i] = 4096;
	for(i=0 ; i<4096 ; ++i)	dad[i] = 4096;

	code_buf[0] = 0;//code_buf[1..16] saves eight units of code, and code_buf[0] works as eight flags, "1" representing that the unit is an unencoded letter (1 byte), "0" a position-and-length pair (2 bytes). Thus, eight units require at most 16 bytes of code.

	code_buf_ptr = mask = 1;

	s = 0;
	r = 4078;

//	for(i=s ; i<r ; ++i)
//		text_buf[i] = '\x0';//Clear the buffer with  any character that will appear often.
	memset(text_buf, 0, r);

	for(len=0 ; len<18 && data<dataEnd ; ++len)
		text_buf[r + len] = *data++;//Read 18 bytes into the last 18 bytes of the buffer
	if(/* (textsize =  */len/* ) */ == 0) {
		result.clear();
		return result;//text of size zero
	}

	for(i=1 ; i<=18 ; ++i)
		InsertNode(r - i);//Insert the 18 strings, each of which begins with one or more 'space' characters.  Note the order in which these strings are inserted.  This way, degenerate trees will be less likely to occur.
	
	InsertNode(r);//Finally, insert the whole string just read.  The global variables match_length and match_position are set.

	do
	{
		if(match_length > len)
			match_length = len;//match_length may be spuriously long near the end of text.
		
		if(match_length <= 2)
		{
			match_length = 1;//Not long enough match.  Send one byte.
			code_buf[0] |= mask;//'send one byte' flag
			code_buf[code_buf_ptr++] = text_buf[r];//Send uncoded.
		}
		else
		{
			code_buf[code_buf_ptr++] = (unsigned char) match_position;
			code_buf[code_buf_ptr++] = (unsigned char) (((match_position >> 4) & 0xf0) | (match_length - (2 + 1)));//Send position and length pair. Note match_length > 2.
		}
		
		if((mask <<= 1) == 0)//Shift mask left one bit.
		{
//			for(i=0 ; i<code_buf_ptr ; ++i)//Send at most 8 units of
//				result.append(code_buf[i]);//code together
			result.replace(curResult, code_buf_ptr, (char *)code_buf, code_buf_ptr);
			curResult += code_buf_ptr;
			code_buf[0] = 0;
			code_buf_ptr = mask = 1;
		}
		
		last_match_length = match_length;
		for(i=0 ; i < last_match_length && data<dataEnd ; ++i)
		{
			c = *data++;
			DeleteNode(s);//Delete old strings and
			text_buf[s] = c;//read new bytes
			
			if(s < 17)
				text_buf[s + 4096] = c;//If the position is near the end of buffer, extend the buffer to make string comparison easier.
			
			s = (s + 1) & (4095);
			r = (r + 1) & (4095);
			//Since this is a ring buffer, increment the position modulo 4096.
			InsertNode(r);//Register the string in text_buf[r..r+18-1]
		}

		while(i++ < last_match_length)//After the end of text,
		{
			DeleteNode(s);//no need to read, but
			s = (s + 1) & (4095);
			r = (r + 1) & (4095);
			if(--len)
				InsertNode(r);//buffer may not be empty.
		}

	}
	while(len > 0);//until length of string to be processed is zero

	if(code_buf_ptr > 1)//Send remaining code.
	{
//		for(i = 0; i < code_buf_ptr ; ++i)
//			result.append(code_buf[i]);
		result.replace(curResult, code_buf_ptr, (char *)code_buf, code_buf_ptr);
		curResult += code_buf_ptr;
	}
	result.truncate(curResult);
	return result;
}

void LZS::clear()
{
	result.clear();
}
