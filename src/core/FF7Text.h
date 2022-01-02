/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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

class FF7Text
{
public:
	explicit FF7Text(const QByteArray &data=QByteArray());
	FF7Text(const QString &text, bool jp);
	const QByteArray &data() const;
	QString text(bool jp, bool simplified=false) const;
	void setText(const QString &text, bool jp);
	bool contains(const QRegularExpression &regExp) const;
	qsizetype indexOf(const QRegularExpression &regExp, qsizetype from, qsizetype &size) const;
	qsizetype lastIndexOf(const QRegularExpression &regExp, qsizetype &from, qsizetype &size) const;
	inline bool operator ==(const FF7Text &t2) const {
		return data() == t2.data();
	}
	inline bool operator !=(const FF7Text &t2) const {
		return data() != t2.data();
	}

private:
	static QString getCaract(quint8 ord, quint8 table=0);
	static const char *caract[256];
	static const char *caract_jp[256];
	static const char *caract_jp_fa[256];
	static const char *caract_jp_fb[256];
	static const char *caract_jp_fc[256];
	static const char *caract_jp_fd[256];
	static const char *caract_jp_fe[256];
	QByteArray _data;
};
