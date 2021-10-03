/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#include "FieldPart.h"

class TutFile : public FieldPart
{
public:
	explicit TutFile(Field *field = nullptr);
	explicit TutFile(const QList<QByteArray> &tutos);
	bool open(const QByteArray &data) override;
	inline void clear() override {
		tutos.clear();
	}
	inline int size() const {
		return tutos.size();
	}
	virtual int maxTutCount() const=0;
	void removeTut(int tutID);
	bool insertTut(int tutID);
	inline const QByteArray &data(int tutID) const {
		return tutos.at(tutID);
	}
	void setData(int tutID, const QByteArray &data);
	bool insertData(int tutID, const QByteArray &data);
	bool insertData(int tutID, const QString &path);
	inline const QList<QByteArray> &dataList() const {
		return tutos;
	}
	virtual inline bool isTut(int tutID) const {
		Q_UNUSED(tutID)
		return true;
	}
	virtual inline bool isBroken(int tutID) const {
		Q_UNUSED(tutID)
		return false;
	}
	virtual inline bool canBeRepaired(int tutID) const {
		Q_UNUSED(tutID)
		return false;
	}
	virtual inline bool repair(int tutID) {
		return !isBroken(tutID);
	}
	virtual QString parseScripts(int tutID, bool *warnings = nullptr) const;
	virtual bool parseText(int tutID, const QString &tuto);
	static void testParsing();
protected:
	virtual QList<quint32> openPositions(const QByteArray &data) const=0;
	inline QByteArray &dataRef(int tutID) {
		return tutos[tutID];
	}
private:
	static inline QString parseScriptsUnknownString(quint8 value) {
		return QString("[%1]").arg(value, 2, 16, QChar('0'));
	}

	QList<QByteArray> tutos;
};
