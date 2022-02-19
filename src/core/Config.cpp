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
#include "Config.h"

QSettings *Config::settings = nullptr;

QString Config::programResourceDir()
{
#if defined(Q_OS_MAC)
	return qApp->applicationDirPath().append("/../Resources");
#elif defined(Q_OS_UNIX)
	return qApp->applicationDirPath().startsWith("/usr/bin")
	       ? "/usr/share/makoureactor"
	       : qApp->applicationDirPath();
#else
	return qApp->applicationDirPath();
#endif
}

QString Config::programLanguagesDir()
{
#if defined(Q_OS_MAC) or defined(Q_OS_UNIX)
	return Config::programResourceDir();
#else
	return qApp->applicationDirPath() + "/languages";
#endif
}

void Config::set() {
	if (!settings) {
#ifdef Q_OS_WIN
		settings = new QSettings(qApp->applicationDirPath()
								 .append("/Makou_Reactor.ini"),
								 QSettings::IniFormat);
#else
		settings = new QSettings("makoureactor/settings");
#endif
		settings->setValue("varFile",
		                   QFileInfo(settings->fileName())
		                   .path().append("/vars.cfg"));
	}
}

void Config::remove() {
	if (settings)	delete settings;
}

QVariant Config::value(const QString &key, const QVariant &defaultValue)
{
	return settings->value(key, defaultValue);
}

void Config::setValue(const QString &key, const QVariant &value)
{
	settings->setValue(key, value);
}

void Config::append(const QString &key, const QVariant &value)
{
	QList<QVariant> list = settings->value(key).toList();
	list.append(value);
	settings->setValue(key, list);
}

void Config::remove(const QString &key)
{
	settings->remove(key);
}

void Config::flush()
{
	settings->sync();
}
