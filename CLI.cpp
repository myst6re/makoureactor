/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2020 Arzel Jérôme <myst6re@gmail.com>
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
#include "CLI.h"
#include "Arguments.h"
#include "ArgumentsExport.h"
#include "ArgumentsPatch.h"
#include "core/field/FieldArchivePS.h"
#include "core/field/FieldArchivePC.h"
#include <iostream>

void CLIObserver::setObserverValue(int value)
{
	quint8 percent = quint8(value * 100.0 / double(_maximum));

	if (percent != _lastPercent) {
		_lastPercent = percent;
		setPercent(percent);
	}
}

void CLIObserver::setPercent(quint8 percent)
{
	printf("[%d%%] %s\r", percent, qPrintable(_filename));
	fflush(stdout);
}

bool CLIObserver::observerRetry(const QString &message)
{
	std::cout << qPrintable(message) << std::endl;
	std::cout << qPrintable(QCoreApplication::translate("CLI", "Retry? [Yn] ")) << std::flush;
	std::string line;
	std::getline(std::cin, line);
	if (std::cin.eof()) {
		return false;
	}

	QString qtLine = QString::fromStdString(line);

	return qtLine.isEmpty() || qtLine.compare(QCoreApplication::translate("CLI", "y"), Qt::CaseInsensitive);
}

CLIObserver CLI::observer;

void CLI::commandExport()
{
	ArgumentsExport argsExport;
	if (argsExport.help() || argsExport.destination().isEmpty()) {
		argsExport.showHelp();
	}

	FieldArchive *fieldArchive = openFieldArchive(argsExport.inputFormat(), argsExport.path());
	if (fieldArchive == nullptr) {
		return;
	}

	PsfTags tags; // TODO: set by CLI
	QList<int> selectedFields;
	QList<QRegExp> includes, excludes;

	for (const QString &pattern: argsExport.includes()) {
		includes.append(QRegExp(pattern, Qt::CaseSensitive, QRegExp::Wildcard));
	}
	for (const QString &pattern: argsExport.excludes()) {
		excludes.append(QRegExp(pattern, Qt::CaseSensitive, QRegExp::Wildcard));
	}

	FieldArchiveIterator it(*fieldArchive);
	while (it.hasNext()) {
		const Field *field = it.next(false);
		if (field != nullptr) {
			bool found = includes.isEmpty();
			for (const QRegExp &regExp: includes) {
				if (regExp.exactMatch(field->name())) {
					found = true;
					break;
				}
			}
			for (const QRegExp &regExp: excludes) {
				if (regExp.exactMatch(field->name())) {
					found = false;
					break;
				}
			}

			if (found) {
				selectedFields.append(it.mapId());
			}
		}
	}

	QMap<FieldArchive::ExportType, QString> toExport;

	if (!argsExport.mapFileFormat().isEmpty()) {
		toExport.insert(FieldArchive::Fields, argsExport.mapFileFormat());
	}
	if (!argsExport.backgroundFormat().isEmpty()) {
		toExport.insert(FieldArchive::Backgrounds, argsExport.backgroundFormat());
	}
	if (!argsExport.soundFormat().isEmpty()) {
		toExport.insert(FieldArchive::Akaos, argsExport.soundFormat());
	}
	if (!argsExport.textFormat().isEmpty()) {
		toExport.insert(FieldArchive::Texts, argsExport.textFormat());
	}

	if (!fieldArchive->exportation(selectedFields, argsExport.destination(),
								   argsExport.force(), toExport, &tags)) {
		qWarning() << qPrintable(QCoreApplication::translate("CLI", "An error occured when exporting"));
	}

	delete fieldArchive;
}

void CLI::commandPatch()
{
	ArgumentsPatch argsPatch;
	if (argsPatch.help() || argsPatch.path().isEmpty()) {
		argsPatch.showHelp();
	}

	FieldArchive *fieldArchive = openFieldArchive(argsPatch.inputFormat(), argsPatch.path());
	if (fieldArchive == nullptr) {
		return;
	}

	QList<int> selectedFields;
	QList<QRegExp> includes, excludes;

	for (const QString &pattern: argsPatch.includes()) {
		includes.append(QRegExp(pattern, Qt::CaseSensitive, QRegExp::Wildcard));
	}
	for (const QString &pattern: argsPatch.excludes()) {
		excludes.append(QRegExp(pattern, Qt::CaseSensitive, QRegExp::Wildcard));
	}

	FieldArchiveIterator it(*fieldArchive);
	while (it.hasNext()) {
		const Field *field = it.next(false);
		if (field != nullptr) {
			bool found = includes.isEmpty();
			for (const QRegExp &regExp: includes) {
				if (regExp.exactMatch(field->name())) {
					found = true;
					break;
				}
			}
			for (const QRegExp &regExp: excludes) {
				if (regExp.exactMatch(field->name())) {
					found = false;
					break;
				}
			}

			if (found) {
				selectedFields.append(it.mapId());
			}
		}
	}

	delete fieldArchive;
}

FieldArchive *CLI::openFieldArchive(const QString &ext, const QString &path)
{
	bool isPS;
	FieldArchiveIO::Type type;

	if (ext == "iso" || ext == "bin" || ext == "img") {
		isPS = true;
		type = FieldArchiveIO::Iso;
	} else {
		if (ext == "dat") {
			isPS = true;
			type = FieldArchiveIO::File;
		} else if (ext == "lgp") {
			isPS = false;
			type = FieldArchiveIO::Lgp;
		} else {
			isPS = false;
			type = FieldArchiveIO::File;
		}
	}

	FieldArchive *fieldArchive;

	if (isPS) {
		fieldArchive = new FieldArchivePS(path, type);
	} else {
		fieldArchive = new FieldArchivePC(path, type);
	}
	fieldArchive->setObserver(&observer);

	FieldArchiveIO::ErrorCode error = fieldArchive->open();

	QString out;
	switch (error)
	{
	case FieldArchiveIO::Ok:
	case FieldArchiveIO::Aborted:
		break;
	case FieldArchiveIO::FieldNotFound:
		out = QCoreApplication::translate("CLI", "Nothing found!");
		break;
	case FieldArchiveIO::FieldExists:
		out = QCoreApplication::translate("CLI", "The file already exists");
		break;
	case FieldArchiveIO::ErrorOpening:
		out = QCoreApplication::translate("CLI", "The file is inaccessible");
		break;
	case FieldArchiveIO::ErrorOpeningTemp:
		out = QCoreApplication::translate("CLI", "Can not create temporary file");
		break;
	case FieldArchiveIO::ErrorRemoving:
		out = QCoreApplication::translate("CLI", "Unable to remove the file, check write permissions.");
		break;
	case FieldArchiveIO::ErrorRenaming:
		out = QCoreApplication::translate("CLI", "Failed to rename the file, check write permissions.");
		break;
	case FieldArchiveIO::ErrorCopying:
		out = QCoreApplication::translate("CLI", "Failed to copy the file, check write permissions.");
		break;
	case FieldArchiveIO::Invalid:
		out = QCoreApplication::translate("CLI", "Invalid file");
		break;
	case FieldArchiveIO::NotImplemented:
		out = QCoreApplication::translate("CLI", "This error should not appear, thank you for reporting it");
		break;
	}

	if (!out.isEmpty()) {
		qWarning() << qPrintable(QCoreApplication::translate("CLI", "Error")) << qPrintable(out);
		delete fieldArchive;
		return nullptr;
	}

	return fieldArchive;
}

void CLI::exec()
{
	Arguments args;
	if (args.help()) {
		args.showHelp();
	}

	switch (args.command()) {
	case Arguments::None:
		args.showHelp();
		break;
	case Arguments::Export:
		commandExport();
		break;
	case Arguments::Patch:
		commandPatch();
		break;
	}
}
