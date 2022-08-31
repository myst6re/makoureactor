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

#include <QPalette>

#ifdef GUI
	#include <QApplication>
#endif

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
	QDir translationDir(QStringLiteral("%1").arg(QCoreApplication::applicationDirPath()));
	QStringList nameFilter{QStringLiteral("Makou_Reactor_*.qm")};
	if (translationDir.entryList(nameFilter, QDir::Files, QDir::Name).isEmpty()) {
		translationDir.setPath(QStringLiteral("%1/%2").arg(QCoreApplication::applicationDirPath(), QStringLiteral("translations")));
		if (translationDir.entryList(nameFilter, QDir::Files, QDir::Name).isEmpty()) {
			translationDir.setPath(QStringLiteral("%1/../translations").arg(QCoreApplication::applicationDirPath()));
			if (translationDir.entryList(nameFilter, QDir::Files, QDir::Name).isEmpty()) {
				translationDir.setPath(QStringLiteral("%1/../share/makoureactor/translations").arg(QCoreApplication::applicationDirPath()));
				if (translationDir.entryList(nameFilter, QDir::Files, QDir::Name).isEmpty()) {
					translationDir.setPath(QStringLiteral("%1/%2").arg(QDir::homePath(), QStringLiteral(".local/share/makoureactor/translations")));
					if (translationDir.entryList(nameFilter, QDir::Files, QDir::Name).isEmpty()) {
						translationDir.setPath(QStringLiteral("/usr/local/share/makourector/translations"));
						if (translationDir.entryList(nameFilter, QDir::Files, QDir::Name).isEmpty()) {
							translationDir.setPath(QStringLiteral("/usr/share/makoureactor/translations"));
						}
					}
				}
			}
		}
	}
	return translationDir.absolutePath();
}

QPalette Config::paletteForSetting()
{
	int index = value("color-scheme", 0).toInt();
	QPalette newPalette;
	if (index != 0) {
		newPalette.setColor(QPalette::Window, index == 1 ? darkWindow : lightWindow);
		newPalette.setColor(QPalette::Base, index == 1 ? darkButton : lightButton);
		newPalette.setColor(QPalette::Text, index == 1 ? darkText : lightText);
		newPalette.setColor(QPalette::AlternateBase, index == 1 ? darkWindow : lightWindow);
		newPalette.setColor(QPalette::WindowText, index == 1 ? darkText : lightText);
		newPalette.setColor(QPalette::Button, index == 1 ? darkButton : lightButton);
		newPalette.setColor(QPalette::ButtonText,index == 1 ? darkText : lightText);
		newPalette.setColor(QPalette::PlaceholderText, index == 1 ? darkPlaceholderText : lightPlaceholderText);
		newPalette.setColor(QPalette::Disabled, QPalette::Button, index == 1 ? darkDisableButton : lightDisableButton);
		newPalette.setColor(QPalette::Disabled, QPalette::Window, index == 1 ? darkDisableButton : lightDisableButton);
		newPalette.setColor(QPalette::Disabled, QPalette::WindowText, index == 1 ? darkInactiveText : lightInactiveText);
		newPalette.setColor(QPalette::Disabled,QPalette::ButtonText, index == 1 ? darkInactiveText : lightInactiveText);
		newPalette.setColor(QPalette::ToolTipBase, index == 1 ? darkText : lightText);
		newPalette.setColor(QPalette::ToolTipText, index == 1 ? darkWindow : lightWindow);
		newPalette.setColor(QPalette::Highlight,index == 1 ? darkHighlight : lightHighlight);
		newPalette.setColor(QPalette::HighlightedText, index == 1 ? darkWindow : lightWindow);
		newPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, index == 1 ? darkButton : lightButton);
		newPalette.setColor(QPalette::Active, QPalette::Text, index == 1 ? darkText : lightText);
		newPalette.setColor(QPalette::Link, index == 1 ? darkLink : lightLink);
		newPalette.setColor(QPalette::LinkVisited, index == 1 ? darkInactiveText : lightInactiveText);
	}
	return newPalette;
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
	return settings->value(key, key == "color-scheme" ? (value("dark_theme").toBool() ? 2 : defaultValue) : defaultValue);
}

void Config::setValue(const QString &key, const QVariant &value)
{
	if(value.isNull()) {
		settings->remove(key);
	}
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

bool Config::inDarkMode()
{
#ifndef GUI
	return false;
#else
	return qApp->palette().text().color().value() >= QColor(Qt::lightGray).value();
#endif
}

QString Config::iconThemeColor()
{
	return Config::inDarkMode() ? QStringLiteral("dark") : QStringLiteral("light");
}
