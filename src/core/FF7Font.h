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

#include <WindowBinFile.h>
#include "FF7Text.h"

class FF7Font
{
public:
	FF7Font(WindowBinFile *windowBinFile, const QByteArray &txtFileData);
	WindowBinFile *windowBinFile() const;
	const QList<QStringList> &tables() const;
	void setTables(const QList<QStringList> &tables);
	void setChar(int tableId, int charId, const QString &c);
	bool isValid() const;
	bool isModified() const;
	void setModified(bool modified);
	const QString &name() const;
	void setName(const QString &name);
	void setPaths(const QString &txtPath, const QString &windowBinFilePath);
	const QString &txtPath() const;
	const QString &windowBinFilePath() const;
	void setReadOnly(bool ro);
	bool isReadOnly() const;
	QString saveTxt();

	static bool listFonts();
	static QStringList fontList();
	static FF7Font *font(QString name);
	static FF7Font *getCurrentConfigFont();
	static bool saveFonts();
	static bool addFont(const QString &name, const QString &from, const QString &displayName);
	static bool removeFont(const QString &name);
	static const QString &fontDirPath();
	static QSize calcSize(const QByteArray &ff7Text);
	static QSize calcSize(const QByteArray &ff7Text, QList<int> &pagesPos);
	static quint8 charW(int tableId, int charId);
	static quint8 leftPadding(int tableId, int charId);
	static quint8 charFullWidth(int tableId, int charId);
	static int calcFF7TextWidth(const FF7Text &ff7Text);
	static int biggestCharWidth;
	static quint8 charWidth[7][256];
	static const char *optimisedDuo[3];
private:
	void openTxt(const QString &data);
	// void print();
	WindowBinFile *_windowBinFile;
	QString _name;
	QString _txtPath, _windowBinFilePath;
	bool modified, readOnly;
	QList<QStringList> _tables;

	static FF7Font *openFont(const QString &windowBinFilePath, const QString &txtPath);
	static QString font_dirPath;
	static QMap<QString, FF7Font *> fonts;
};
