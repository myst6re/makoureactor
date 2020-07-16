/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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
#include "TutFilePC.h"

TutFilePC::TutFilePC() :
	TutFile(0)
{
}

bool TutFilePC::open()
{
	return false;
}

QList<quint32> TutFilePC::openPositions(const QByteArray &data) const
{
	const char *constData = data.constData();
	QList<quint32> positions;
	quint32 dataSize = data.size();

	quint16 posTut;
	for (int i=0; i<9; ++i) {
		memcpy(&posTut, constData + i*2, 2);
		if (posTut < 18)		posTut = 18;
		positions.append(posTut);
	}

	positions.append(dataSize);

	return positions;
}

QByteArray TutFilePC::save() const
{
	quint32 pos;
	QByteArray toc, ret;

	for (int i=0; i<9; ++i) {
		if (i < size()) {
			pos = 18 + ret.size();
			ret.append(data(i));
		} else {
			pos = 0xffff;
		}
		toc.append((char *)&pos, 2);
	}

	return toc.append(ret);
}
