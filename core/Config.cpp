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
#include "Config.h"

QSettings *Config::settings = 0;

QString Config::programResourceDir()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	return qApp->applicationDirPath().startsWith("/usr/bin")
	       ? "/usr/share/makoureactor"
	       : qApp->applicationDirPath();
#else
	return qApp->applicationDirPath();
#endif
}

void Config::set() {
	if(!settings) {
#ifdef Q_OS_WIN
	settings = new QSettings(qApp->applicationDirPath()+"/Makou_Reactor.ini", QSettings::IniFormat);
#else
	settings = new QSettings("Makou_Reactor");
#endif
	}
}

void Config::remove() {
	if(settings)	delete settings;
}

QVariant Config::value(const QString &key, const QVariant &defaultValue)
{
	return settings->value(key, defaultValue);
}

void Config::setValue(const QString &key, const QVariant &value)
{
	settings->setValue(key, value);
}

void Config::remove(const QString &key)
{
	settings->remove(key);
}

void Config::flush()
{
	settings->sync();
}
