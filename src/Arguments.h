/****************************************************************************
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

#define _ADD_ARGUMENT(names, description, valueName, defaultValue) \
	_parser.addOption(QCommandLineOption(names, description, valueName, defaultValue))

#define _ADD_FLAG(names, description) \
	_parser.addOption(QCommandLineOption(names, description))

#define _OPTION_NAMES(shortName, fullName) \
	(QStringList() << shortName << fullName)

#include <QtCore>

class HelpArguments
{
public:
	HelpArguments();
	[[ noreturn ]] void showHelp(int exitCode = EXIT_SUCCESS);
	bool help() const;
protected:
	QCommandLineParser _parser;
};

class CommonArguments : public HelpArguments
{
public:
	CommonArguments();
	inline QString path() const {
		return _path;
	}
	QString inputFormat() const;
	QStringList includes() const;
	QStringList excludes() const;
protected:
	QStringList wilcardParse();
	void mapNamesFromFiles();
	static QStringList mapNamesFromFile(const QString &path);
	static QStringList searchFiles(const QString &path);
	QString _path;
	QStringList _includesFromFile, _excludesFromFile;
};

class Arguments : public HelpArguments
{
public:
	enum Command {
		None,
		Export,
		//Import,
		Patch,
		Tools
	};
	Arguments();
	inline Command command() const {
		return _command;
	}
private:
	void parse();
	Command _command;
};
