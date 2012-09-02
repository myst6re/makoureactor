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
#include "Config.h"
#include "FF7Text.h"
#include "TextHighlighter.h"

class TutFile
{
public:
	TutFile();
	explicit TutFile(const QByteArray &data, bool tutType=false);
	bool isOpen() const;
	bool isModified() const;
	void setModified(bool);
	bool open(const QByteArray &data, bool tutType=false);
	QByteArray save(QByteArray &toc, quint32 firstPos=0) const;
	int size() const;
	bool hasTut() const;
	bool isTut(int tutID) const;
	void removeTut(int tutID);
	bool insertTut(int tutID);
	bool insertAkao(int tutID, const QString &akaoPath);
	const QByteArray &data(int tutID) const;
	void setData(int tutID, const QByteArray &data);
	QString parseScripts(int tutID) const;
	void parseText(int tutID, const QString &tuto);
	int akaoID(int tutID) const;
	void setAkaoID(int tutID, quint16 akaoID);
private:
	bool _isOpen, _isModified, tutType;
	QList<QByteArray> tutos;
};

#endif // ENCOUNTERFILE_H
