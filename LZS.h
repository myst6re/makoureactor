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
#ifndef DEF_LZS
#define DEF_LZS

#include <QtCore>

#define MAX 1000000

class LZS
{
public:
	static const QByteArray &decompress(const QByteArray &fileData, int max=MAX);
	static QByteArray compress(const QByteArray &fileData);

private:
	static void InsertNode(qint32 r);
	static void DeleteNode(qint32 p);
	static qint32 match_length;//of longest match. These are set by the InsertNode() procedure.
	static qint32 match_position;
	static qint32 lson[4097];//left & right children & parents -- These constitute binary search trees.
	static qint32 rson[4353];
	static qint32 dad[4097];
	static unsigned char text_buf[4113];//ring buffer of size 4096, with extra 17 bytes to facilitate string comparison
	static QByteArray result;
};

#endif
