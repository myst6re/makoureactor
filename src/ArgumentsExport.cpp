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
#include "ArgumentsExport.h"

ArgumentsExport::ArgumentsExport() : CommonArguments()
{
	_ADD_ARGUMENT("map-file", "Export map files. Possible values: dec (decompressed), lzs (compressed)", "map-file", "");
	_ADD_ARGUMENT("background", "Export flatten backgrounds. Possible values: png, jpg, bmp. "
	              "Cannot be specified with bg-layer option.", "background", "");
	_ADD_ARGUMENT("bg-layer", "Export backgrounds with multiple layers. Possible values: png, jpg, bmp. "
	                          "Cannot be specified with background option.", "bg-layer", "");
	_ADD_ARGUMENT("music", "Export musics. Possible values: psf, minipsf, akao, snd (alias of akao)", "music", "");
	_ADD_ARGUMENT("text", "Export texts. Possible values: xml, txt", "text", "");
	_ADD_ARGUMENT("chunk", "Export field chunks. Possible value: chunk", "chunk", "");
	_ADD_ARGUMENT("psf-lib-path", "PSF lib path. Required only when --music psf/minipsf is set.", "psf-lib-path", "");
	_ADD_FLAG(_OPTION_NAMES("f", "force"),
	             "Overwrite destination file if exists.");

	_parser.addPositionalArgument("directory", QCoreApplication::translate("ArgumentsExport", "Output directory."));

	parse();
}

QString ArgumentsExport::mapFileFormat() const
{
	return _parser.value("map-file");
}

QString ArgumentsExport::backgroundFormat() const
{
	QString ret = _parser.value("background");

	if (ret.isEmpty()) {
		ret = _parser.value("bg-layer");

		if (!ret.isEmpty()) {
			ret = ret % "_";
		}
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

QString ArgumentsExport::chunkFormat() const
{
	return _parser.value("chunk");
}

PsfTags ArgumentsExport::psfTags() const
{
	return PsfTags(_parser.value("psf-lib-path"));
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

	if ((_parser.value("music") == "psf" || _parser.value("music") == "minipsf") && !_parser.isSet("psf-lib-path")) {
		qWarning() << qPrintable(
		    QCoreApplication::translate("Arguments", "Error: --psf-lib-path is required with --music psf/minipsf"));
		exit(1);
	}

	QStringList paths = wilcardParse();
	if (paths.size() == 2) {
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
	mapNamesFromFiles();
}
