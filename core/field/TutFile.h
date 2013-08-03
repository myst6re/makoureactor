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
#ifndef TUTFILE_H
#define TUTFILE_H

#include <QtCore>
#include "FieldPart.h"

class TutFile : public FieldPart
{
public:
	explicit TutFile(Field *field);
	bool open(const QByteArray &data);
	void clear();
	int size() const;
	virtual int maxTutCount() const=0;
	void removeTut(int tutID);
	bool insertTut(int tutID);
	const QByteArray &data(int tutID) const;
	void setData(int tutID, const QByteArray &data);
	bool insertData(int tutID, const QByteArray &data);
	bool insertData(int tutID, const QString &path);
	const QList<QByteArray> &dataList() const;
	virtual bool isTut(int tutID) const;
	virtual QString parseScripts(int tutID) const;
	virtual bool parseText(int tutID, const QString &tuto);
protected:
	virtual QList<quint32> openPositions(const QByteArray &data) const=0;
	QByteArray &dataRef(int tutID);
private:
	QList<QByteArray> tutos;
};

#endif // ENCOUNTERFILE_H
