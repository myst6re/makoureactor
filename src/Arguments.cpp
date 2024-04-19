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
#include "Arguments.h"

HelpArguments::HelpArguments()
{
	_ADD_FLAG(_OPTION_NAMES("h", "help"), "Displays help.");
}

bool HelpArguments::help() const
{
	return _parser.isSet("help");
}

[[ noreturn ]] void HelpArguments::showHelp(int exitCode)
{
	QRegularExpression usage("Usage: .* \\[options\\]");
	qInfo("%s", qPrintable(_parser.helpText().replace(usage, "Usage: %1 [options]")
	                     .arg(QFileInfo(qApp->arguments().first()).fileName())));
	::exit(exitCode);
}

CommonArguments::CommonArguments()
{
	_ADD_ARGUMENT("input-format", "Input format (lgp, fieldpc, iso, dat).", "input-format", "");
	_ADD_ARGUMENT("include", "Include field map name. Repeat this argument to include multiple names. "
	                         "If empty, all will be included by default.", "include", "");
	_ADD_ARGUMENT("exclude", "Exclude field map name. Repeat this argument to exclude multiple names. "
	                         "Exclude has the priority over the --include argument.", "exclude", "");
	_ADD_ARGUMENT("include-from", "Include field map names from file. The file format is one name per line.", "include", "");
	_ADD_ARGUMENT("exclude-from", "Exclude field map names from file. The file format is one name per line.", "exclude", "");

	_parser.addPositionalArgument("file", QCoreApplication::translate("Arguments", "Input file or directory."));
}

QString CommonArguments::inputFormat() const
{
	QString inputFormat = _parser.value("input-format");
	if(inputFormat.isEmpty() && !_path.isEmpty()) {
		qsizetype index = _path.lastIndexOf('.');
		if(index > -1) {
			return _path.mid(index + 1);
		}
		return "fieldpc";
	}
	return inputFormat;
}

QStringList CommonArguments::includes() const
{
	return _parser.values("include") + _includesFromFile;
}

QStringList CommonArguments::excludes() const
{
	return _parser.values("exclude") + _excludesFromFile;
}

QStringList CommonArguments::searchFiles(const QString &path)
{
	qsizetype index = path.lastIndexOf('/');
	QString dirname, filename;

	if (index > 0) {
		dirname = path.left(index);
		filename = path.mid(index + 1);
	} else {
		filename = path;
	}

	QDir dir(dirname);
	QStringList entryList = dir.entryList(QStringList(filename), QDir::Files);
	int i = 0;
	for (const QString &entry: entryList) {
		entryList.replace(i++, dir.filePath(entry));
	}
	return entryList;
}

QStringList CommonArguments::wilcardParse()
{
	QStringList paths, args = _parser.positionalArguments();

	args.removeFirst();

	for (const QString &path: args) {
		if (path.contains('*') || path.contains('?')) {
			paths << searchFiles(QDir::fromNativeSeparators(path));
		} else {
			paths << QDir::fromNativeSeparators(path);
		}
	}

	return paths;
}

QStringList CommonArguments::mapNamesFromFile(const QString &path)
{
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) {
		qWarning() << qPrintable(
		    QCoreApplication::translate("Arguments", "Warning: cannot open file"))
		           << qPrintable(path) << qPrintable(f.errorString());
		exit(1);
	}

	QStringList ret;

	while (f.canReadLine()) {
		ret.append(QString::fromUtf8(f.readLine()));
	}

	return ret;
}

void CommonArguments::mapNamesFromFiles()
{
	QStringList pathsInclude = _parser.values("include-from");

	for (const QString &path: pathsInclude) {
		_includesFromFile.append(mapNamesFromFile(path));
	}

	QStringList pathsExclude = _parser.values("exclude-from");

	for (const QString &path: pathsExclude) {
		_excludesFromFile.append(mapNamesFromFile(path));
	}
}

Arguments::Arguments() :
      _command(None)
{
	_parser.addVersionOption();
	_parser.addPositionalArgument(
	    "command", QCoreApplication::translate("Arguments", "Command to execute"), "<command>"
	);
	_parser.addPositionalArgument("args", "", "[<args>]");
	_parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);
	_parser.setApplicationDescription(
	    QCoreApplication::translate(
	        "Arguments",
	        "\nList of available commands:\n"
	        "  export           Export various assets from archive to files\n"
	        "  patch            Patch archive\n"
	        "  unpack-bg-mod    Unpack background mod packed with Palmer\n"
	        "\n"
	        "\"%1 export --help\" to see help of the specific subcommand"
	    ).arg(QFileInfo(qApp->arguments().first()).fileName())
	);

	parse();
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
	} else if (command == "patch") {
		_command = Patch;
	}  else if (command == "unpack-bg-mod") {
		_command = Tools;
	} else {
		qWarning() << qPrintable(QCoreApplication::translate("Arguments", "Unknown command type:")) << qPrintable(command);
		return;
	}
}
