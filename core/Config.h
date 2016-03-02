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
#ifndef CONFIG_H
#define CONFIG_H

#include <QtCore>

class Config
{
public:
	static QString programResourceDir();
	static void set();
	static void remove();
	static QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
	static void setValue(const QString &key, const QVariant &value);
	static void append(const QString &key, const QVariant &value);
	static void remove(const QString &key);
	static void flush();
private:
	static QSettings *settings;
};

#endif // CONFIG_H
