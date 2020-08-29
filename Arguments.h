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
#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <QtCore>

class Arguments
{
public:
	enum Command {
		None,
		Export
	};
	Arguments();
	inline void showHelp(int exitCode = 0) {
		_parser.showHelp(exitCode);
	}
	inline Command command() const {
		return _command;
	}
	bool help() const;
private:
	void parse();
	Command _command;
	QCommandLineParser _parser;
};

#endif // ARGUMENTS_H
