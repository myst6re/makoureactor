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
#pragma once

#include <QtCore>
#include "TutFile.h"

class Field;

class TutFileStandard : public TutFile
{
public:
	explicit TutFileStandard(Field *field);
	explicit TutFileStandard(const QList<QByteArray> &tutos);
	using TutFile::open; // open(QByteArray)
	bool open();
	QByteArray save(QByteArray &toc, quint32 firstPos) const;
	QByteArray save() const;
	inline int maxTutCount() const { return 255; }
	bool hasTut() const;
	bool isTut(int tutID) const;
	bool isAkao(int tutID) const;
	bool isBroken(int tutID) const;
	bool canBeRepaired(int tutID) const;
	bool repair(int tutID);
	int akaoID(int tutID) const;
	void setAkaoID(int tutID, quint16 akaoID);
	QString parseScripts(int tutID, bool *warnings = nullptr) const;
protected:
	QList<quint32> openPositions(const QByteArray &data) const;
	QByteArray save2(QByteArray &toc, quint32 firstPos) const;
};
