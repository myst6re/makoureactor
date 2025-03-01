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
#include "CsvFile.h"

CsvFile::CsvFile(QIODevice *io, const QChar &fieldSeparator, const QChar &quoteCharacter, CsvEncoding encoding) :
      _io(io), _fieldSeparator(fieldSeparator), _quoteCharacter(quoteCharacter), _encoding(encoding)
{
}

bool CsvFile::readLine(QStringList &line)
{
	QString field;
	line.clear();
	State state = Start;
	
	do {
		if (_io->atEnd()) {
			return false;
		}

		QByteArray lineData = _io->readLine();
		QString l;
		switch (_encoding) {
		case Utf8:
			l = QString::fromUtf8(lineData);
			break;
		}
		
		for (int i = 0; i < l.size(); ++i) {
			QChar c = l.at(i);
			
			switch (state) {
			case Start:
				if (c == _fieldSeparator) {
					state = NextField;
				} else if (c == '\n' || c == '\r') {
					state = End;
				} else if (c == _quoteCharacter) {
					state = FieldQuoted;
				} else {
					state = FieldUnquoted;
				}
				break;
			case End:
			case NextField:
				// Should not happen
				qWarning() << "Invalid state";
				break;
			case FieldQuoted:
				if (c == _quoteCharacter) {
					state = FieldQuotedEscape;
				} else {
					field.append(c);
				}
				break;
			case FieldQuotedEscape:
				if (c == _fieldSeparator) {
					state = NextField;
				} else if (c == '\n' || c == '\r') {
					state = End;
				} else if (c == _quoteCharacter) {
					field.append(c);
					state = FieldQuoted;
				} else {
					field.append(_quoteCharacter); // Abort escape, put the quote character of the previous iteration
					field.append(c);
					state = FieldQuoted;
				}
				break;
			case FieldUnquoted:
				if (c == _fieldSeparator) {
					state = NextField;
				} else if (c == '\n' || c == '\r') {
					state = End;
				} else {
					field.append(c);
					state = FieldUnquoted;
				}
				break;
			}
			
			if (state == NextField) {
				line.append(field);
				field = QString();
				if (i + 1 < l.size()) {
					state = Start;
				}
			} else if (state == End) {
				line.append(field);
				break;
			}
		}
		
		// Flush unfinished states
		switch (state) {
		case End:
		case NextField:
			// Nothing to do
			break;
		case FieldQuoted:
			// Will read another line
			break;
		case Start:
		case FieldQuotedEscape:
		case FieldUnquoted:
			line.append(field);
			break;
		}
	} while (state == FieldQuoted);
	
	return true;
}

bool CsvFile::writeLine(const QStringList &line)
{
	QString l, esc = QString().append(_quoteCharacter).append(_quoteCharacter);

	for (QString field: line) {
		l.append(_quoteCharacter)
		        .append(field.replace(_quoteCharacter, esc))
		        .append(_quoteCharacter)
		        .append(_fieldSeparator);
	}

	l[l.size() - 1] = '\r';
	l.append('\n');
	
	QByteArray lineData;
	
	switch (_encoding) {
	case Utf8:
		lineData = l.toUtf8();
		break;
	}
	
	return _io->write(lineData) == lineData.size();
}
