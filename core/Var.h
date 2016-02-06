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
#ifndef DEF_VAR
#define DEF_VAR

#include <QtCore>

class Var
{
public:
	static bool load();
	static bool save(const QMap<quint16, QString> &);
	static QString name(quint8 bank, quint8 adress);
	static const QMap<quint16, QString> &get();
	static void set(quint8 bank, quint8 adress, const QString &name);
	static void del(quint8 bank, quint8 adress);
	static bool exists(quint8 bank, quint8 adress);

private:
	static QMap<quint16, QString> var_names;
};

#endif
