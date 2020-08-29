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
#include "ArgumentsExport.h"

ArgumentsExport::ArgumentsExport()
{
	_parser.addHelpOption();

	_ADD_ARGUMENT("input-format", "Input format (lgp, fieldpc, iso, dat).", "input-format", "");
	_ADD_ARGUMENT("include", "Include field map name. Repeat this argument to include multiple names. "
	                         "If empty, all will be included by default.", "include", "");
	_ADD_ARGUMENT("exclude", "Exclude field map name. Repeat this argument to exclude multiple names. "
	                         "Exclude has the priority over the --include argument.", "exclude", "");
	_ADD_ARGUMENT("include-from", "Include field map names from file. The file format is one name per line.", "include", "");
	_ADD_ARGUMENT("exclude-from", "Exclude field map names from file. The file format is one name per line.", "exclude", "");
	_ADD_ARGUMENT("map-file", "Export map files. Possible values: dec (decompressed), lzs (compressed)", "map-file", "");
	_ADD_ARGUMENT("background", "Export flatten backgrounds. Possible values: png, jpg, bmp. "
	              "Cannot be specified with bg-layer option.", "background", "");
	_ADD_ARGUMENT("bg-layer", "Export backgrounds with multiple layers. Possible values: png, jpg, bmp. "
	                          "Cannot be specified with background option.", "bg-layer", "");
	_ADD_ARGUMENT("music", "Export musics. Possible values: psf, akao, snd (alias of akao)", "music", "");
	_ADD_ARGUMENT("text", "Export texts. Possible values: xml, txt", "text", "");
	_ADD_FLAG(_OPTION_NAMES("f", "force"),
	             "Overwrite destination file if exists.");

	_parser.addPositionalArgument("file", QCoreApplication::translate("ArgumentsExport", "Input file or directory."));
	_parser.addPositionalArgument("directory", QCoreApplication::translate("ArgumentsExport", "Output directory."));

	parse();
}

QString ArgumentsExport::inputFormat() const
{
	QString inputFormat = _parser.value("input-format");
	if(inputFormat.isEmpty() && !_path.isEmpty()) {
		int index = _path.lastIndexOf('.');
		if(index > -1) {
			return _path.mid(index + 1);
		}
		return "fieldpc";
	}
	return inputFormat;
}

QString ArgumentsExport::mapFileFormat() const
{
	return _parser.value("map-file");
}

QString ArgumentsExport::backgroundFormat() const
{
	QString ret = _parser.value("background");

	if (ret.isEmpty()) {
		ret = _parser.value("bg-layer") % "_";
	}

	return ret;
}

QString ArgumentsExport::soundFormat() const
{
	return _parser.value("music");
}

QString ArgumentsExport::textFormat() const
{
	return _parser.value("text");
}

QStringList ArgumentsExport::includes() const
{
	return _parser.values("include") + _includesFromFile;
}

QStringList ArgumentsExport::excludes() const
{
	return _parser.values("exclude") + _excludesFromFile;
}

bool ArgumentsExport::help() const
{
	return _parser.isSet("help");
}

bool ArgumentsExport::force() const
{
	return _parser.isSet("force");
}

void ArgumentsExport::parse()
{
	_parser.process(*qApp);

	if (_parser.isSet("background") && _parser.isSet("bg-layer")) {
		qWarning() << qPrintable(
		    QCoreApplication::translate("Arguments",
		                                "Error: you cannot specify both background and bg-layer parameters"));
		exit(1);
	}

	if (_parser.positionalArguments().size() > 3) {
		qWarning() << qPrintable(
		    QCoreApplication::translate("Arguments", "Error: too much parameters"));
		exit(1);
	}

	wilcardParse();
	mapNamesFromFiles();
}

QStringList ArgumentsExport::searchFiles(const QString &path)
{
	int index = path.lastIndexOf('/');
	QString dirname, filename;

	if (index > 0) {
		dirname = path.left(index);
		filename = path.mid(index + 1);
	} else {
		filename = path;
	}

	QDir dir(dirname);
	QStringList entryList = dir.entryList(QStringList(filename), QDir::Files);
	int i=0;
	for (const QString &entry: entryList) {
		entryList.replace(i++, dir.filePath(entry));
	}
	return entryList;
}

void ArgumentsExport::wilcardParse()
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

	if (!paths.isEmpty()) {
		// Output directory
		if (QDir(paths.last()).exists()) {
			_directory = paths.takeLast();
		} else {
			qWarning() << qPrintable(
			    QCoreApplication::translate("Arguments", "Error: target directory does not exist:"))
			           << qPrintable(paths.last());
			exit(1);
		}

		if (!paths.isEmpty()) {
			_path = paths.first();
		}
	}
}

QStringList ArgumentsExport::mapNamesFromFile(const QString &path)
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

void ArgumentsExport::mapNamesFromFiles()
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
