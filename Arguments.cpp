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
#include "Arguments.h"

Arguments::Arguments() :
      _command(None)
{
	_parser.addHelpOption();
	_parser.addVersionOption();
	_parser.addPositionalArgument(
	    "command", QCoreApplication::translate("Arguments", "Available commands: export"), "<command>"
	);
	_parser.addPositionalArgument("args", "", "[<args>]");
	_parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);

	parse();
}

bool Arguments::help() const
{
	return _parser.isSet("help");
}

void Arguments::parse()
{
	_parser.process(*qApp);

	QStringList args = _parser.positionalArguments();

	if (args.isEmpty()) {
		qWarning() << qPrintable(QCoreApplication::translate("Arguments", "Please specify a command"));
		return;
	}

	const QString &command = args.first();

	if (command == "export") {
		_command = Export;
	} else {
		qWarning() << qPrintable(QCoreApplication::translate("Arguments", "Unknown command type:")) << qPrintable(command);
		return;
	}
}
