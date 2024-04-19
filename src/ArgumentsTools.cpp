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
#include "ArgumentsTools.h"

ArgumentsTools::ArgumentsTools() : CommonArguments()
{
	_parser.addPositionalArgument("directory", QCoreApplication::translate("ArgumentsTools", "Input/Output directory."));

	parse();
}

QDir ArgumentsTools::dir() const
{
	return QDir(_directory);
}

void ArgumentsTools::parse()
{
	_parser.process(*qApp);

	if (_parser.positionalArguments().size() > 3) {
		qWarning() << qPrintable(
		    QCoreApplication::translate("Arguments", "Error: too much parameters"));
		exit(1);
	}

	QStringList paths = wilcardParse();
	if (paths.size() == 2) {
		// Input/Output directory
		if (QDir(paths.last()).exists()) {
			_directory = paths.takeLast();
		} else {
			qWarning() << qPrintable(
			    QCoreApplication::translate("Arguments", "Error: directory does not exist:"))
			           << qPrintable(paths.last());
			exit(1);
		}

		if (!paths.isEmpty()) {
			_path = paths.first();
		}
	}
	mapNamesFromFiles();
}
