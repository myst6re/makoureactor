/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2025 Arzel Jérôme <myst6re@gmail.com>
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

#include <QtCore>

class CsvFile
{
public:
	enum CsvEncoding {
		Utf8
	};
	explicit CsvFile(QIODevice *io, const QChar &fieldSeparator = QChar(','), const QChar &quoteCharacter = QChar('"'),
	                 CsvEncoding encoding = Utf8);
	bool readLine(QStringList &line);
	bool writeLine(const QStringList &line);
private:
	enum State {
		Start,
		End,
		NextField,
		FieldQuoted,
		FieldQuotedEscape,
		FieldUnquoted
	};
	QIODevice *_io;
	QChar _fieldSeparator, _quoteCharacter;
	CsvEncoding _encoding;
};
