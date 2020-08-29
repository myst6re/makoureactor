/****************************************************************************
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
#ifndef ARGUMENTS_EXPORT_H
#define ARGUMENTS_EXPORT_H

#include <QtCore>

#define _ADD_ARGUMENT(names, description, valueName, defaultValue) \
	_parser.addOption(QCommandLineOption(names, description, valueName, defaultValue));

#define _ADD_FLAG(names, description) \
	_parser.addOption(QCommandLineOption(names, description));

#define _OPTION_NAMES(shortName, fullName) \
	(QStringList() << shortName << fullName)

class ArgumentsExport
{
public:
	ArgumentsExport();
	inline void showHelp(int exitCode = 0) {
		_parser.showHelp(exitCode);
	}

	inline QString path() const {
		return _path;
	}
	QString inputFormat() const;
	inline QString destination() const {
		return _directory;
	}
	QString mapFileFormat() const;
	QString backgroundFormat() const;
	QString soundFormat() const;
	QString textFormat() const;
	QStringList includes() const;
	QStringList excludes() const;
	bool help() const;
	bool force() const;
private:
	bool exportAll() const;
	void parse();
	void wilcardParse();
	void mapNamesFromFiles();
	static QStringList mapNamesFromFile(const QString &path);
	static QStringList searchFiles(const QString &path);
	QString _path, _directory;
	QStringList _includesFromFile, _excludesFromFile;
	QCommandLineParser _parser;
};

#endif // ARGUMENTS_EXPORT_H
