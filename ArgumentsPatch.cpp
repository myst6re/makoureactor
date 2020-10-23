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
#include "ArgumentsPatch.h"

ArgumentsPatch::ArgumentsPatch()
{
	_ADD_FLAG("empty-unused-texts", "Empty unused texts.");
	_ADD_FLAG("remove-dialogs", "Remove in-game dialogs.");
	_ADD_FLAG("remove-encounters", "Remove random encounters, but not scripted encounters.");
	_ADD_FLAG("autosize-text-windows", "Autosize windows when the content can be guessed.");
	_ADD_FLAG("clean-model-loader", "Clean model loader section format (PC format only)");
	_ADD_FLAG("remove-tiles-section", "Remove the unused tiles section (PC format only)");
	_ADD_FLAG("repair-backgrounds", "Repair lastmap (completely) and fr_e (partially) backgrounds. "
	                                "Use `--include lastmap` to repair only one background.");

	_parser.addPositionalArgument(
	    "target_file", QCoreApplication::translate("Arguments", "Output file (optional)."), "[target_file]"
	);

	parse();
}

void ArgumentsPatch::parse()
{
	_parser.process(*qApp);

	if (_parser.positionalArguments().size() > 3) {
		qWarning() << qPrintable(
		    QCoreApplication::translate("Arguments", "Error: too much parameters"));
		exit(1);
	}

	QStringList paths = wilcardParse();
	if (!paths.isEmpty()) {
		_path = paths.first();
		if (paths.size() > 1) {
			_target_file = paths.at(1);
		}
	}
	mapNamesFromFiles();
}
