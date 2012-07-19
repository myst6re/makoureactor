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
#ifndef DEF_FF7TEXT
#define DEF_FF7TEXT

#include <QtCore>
#include "Config.h"

class FF7Text
{
public:
	FF7Text(const QByteArray &texte=QByteArray());
	FF7Text(const QString &texte, bool jp);
	const QByteArray &getData() const;
	QString getText(bool jp, bool simplified=false) const;
	QString getShortText(bool jp) const;
	void setText(const QString &text, bool jp);
	bool search(const QRegExp &texte) const;

private:
	QString getCaract(quint8 ord, quint8 table=0) const;
	static const char *caract[256];
	static const char *caract_jp[256];
	static const char *caract_jp_fa[256];
	static const char *caract_jp_fb[256];
	static const char *caract_jp_fc[256];
	static const char *caract_jp_fd[256];
	static const char *caract_jp_fe[256];
	QByteArray texte;
};

#endif
