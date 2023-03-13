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
#include <QColor>

class Config
{
public:
	static QString programResourceDir();
	static QString programLanguagesDir();
	static QPalette paletteForSetting();
	static void set();
	static void remove();
	static QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
	static void setValue(const QString &key, const QVariant &value);
	static void append(const QString &key, const QVariant &value);
	static void remove(const QString &key);
	static void flush();
	static bool inDarkMode();
	static QString iconThemeColor();
private:
	static QSettings *settings;
	//Theme Colors
	inline static const QColor lightWindow = QColor(252, 252, 252);
	inline static const QColor lightDisableWindow = QColor(192, 192, 192);
	inline static const QColor lightText = QColor(35, 38, 39);
	inline static const QColor lightButton = QColor(239, 240, 241);
	inline static const QColor lightDisableButton = QColor(180, 181, 182);
	inline static const QColor lightHighlight = QColor(61,174,233);
	inline static const QColor lightInactiveText = QColor(127,140,141);
	inline static const QColor lightLink = QColor(41, 128, 185);
	inline static const QColor lightPlaceholderText = QColor(96, 96, 96);
	inline static const QColor darkWindow = QColor(35, 38, 41);
	inline static const QColor darkDisableWindow = QColor(100, 100, 100);
	inline static const QColor darkText = QColor(239, 240, 241);
	inline static const QColor darkDisableButton = QColor(67, 74, 81);
	inline static const QColor darkButton = QColor(49, 54, 59);
	inline static const QColor darkHighlight = QColor(61,174,233);
	inline static const QColor darkInactiveText = QColor(189, 195, 199);
	inline static const QColor darkLink = QColor(41, 128, 185);
	inline static const QColor darkPlaceholderText = QColor(196, 196, 196);
};
