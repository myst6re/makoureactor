/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
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
#include "TextHighlighter.h"
#include "../Data.h"

TextHighlighter::TextHighlighter(QTextDocument *parent, bool tut)
	: QSyntaxHighlighter(parent)
{
	HighlightingRule rule;

	if (tut) {
		rule.pattern = QRegExp("^TEXT\\(\"(.*)\"\\)$", Qt::CaseInsensitive);
		rule.color = Data::color(Data::ColorGreenForeground);
		highlightingRules.append(rule);
	}

	rule.pattern = QRegExp("\\{x[\\da-fA-F]{2}\\}");
	rule.color = Data::color(Data::ColorRedForeground);
	highlightingRules.append(rule);

	QStringList names;
	names << "\\{CLOUD\\}" << "\\{BARRET\\}" << "\\{TIFA\\}" << "\\{AERITH\\}" << "\\{AERIS\\}" << "\\{RED XIII\\}" << "\\{YUFFIE\\}" <<
			"\\{CAIT SITH\\}" << "\\{VINCENT\\}" << "\\{CID\\}" << "\\{MEMBER 1\\}" << "\\{MEMBER 2\\}" << "\\{MEMBER 3\\}";

	for (const QString &name : qAsConst(names)) {
		rule.pattern = QRegExp(name, Qt::CaseInsensitive);
		rule.color = Data::color(Data::ColorGreenForeground);
		highlightingRules.append(rule);
	}

	QStringList syst;
	syst << "\\{GREY\\}" << "\\{BLUE\\}" << "\\{RED\\}" << "\\{PURPLE\\}" << "\\{GREEN\\}" << "\\{CYAN\\}" << "\\{YELLOW\\}" <<
			"\\{WHITE\\}" << "\\{BLINK\\}" << "\\{MULTICOLOUR\\}" << "\\{VARDEC\\}" << "\\{VARHEX\\}" << "\\{SCROLLING\\}" << "\\{VARDECR\\}" <<
			"\\{SPACED CHARACTERS\\}" << "\\{PAUSE\\}" << "\\{PAUSE\\d\\d\\d\\}" << "\\{MEMORY:var\\[(\\d+)\\]\\[(\\d+)\\];size=(\\d+)\\}" <<
			"\\{CHOICE\\}" << "^\\{NEW PAGE\\}$" << "^\\{NEW PAGE 2\\}$" << "\\{CIRCLE\\}" << "\\{TRIANGLE\\}" << "\\{SQUARE\\}" << "\\{CROSS\\}";

	for (const QString &s : qAsConst(syst)) {
		rule.pattern = QRegExp(s, Qt::CaseInsensitive);
		rule.color = Data::color(Data::ColorBlueForeground);
		highlightingRules.append(rule);
	}

	QStringList dchar;
	dchar << "\\{, \\}" << "\\{\\.\"\\}" << QString::fromUtf8("\\{\xe2\x80\xa6\"\\}");//\xe2\x80\xa6 = "..."

	for (const QString &s : qAsConst(dchar)) {
		rule.pattern = QRegExp(s);
		rule.color = Data::color(Data::ColorGreyForeground);
		highlightingRules.append(rule);
	}

	if (tut) {
		QStringList keys;
		keys << "^\\[UP\\]$" << "^\\[DOWN\\]$" << "^\\[LEFT\\]$" << "^\\[RIGHT\\]$" << "^\\[MENU\\]$" << "^\\[CANCEL\\]$" <<
				"^\\[CHANGE\\]$" << "^\\[OK\\]$" << "^\\[R1\\]$" << "^\\[R2\\]$" << "^\\[L1\\]$" << "^\\[L2\\]$" << "^\\[START\\]$" << "^\\[SELECT\\]$";

		for (const QString &s : qAsConst(keys)) {
			rule.pattern = QRegExp(s, Qt::CaseInsensitive);
			rule.color = Data::color(Data::ColorPurpleForeground);
			highlightingRules.append(rule);
		}

		QStringList op;
		op << "^\\{FINISH\\}$" << "^\\{NOP\\}$" << "^MOVE\\((\\d+),(\\d+)\\)$" << "^PAUSE\\((\\d+)\\)$";

		for (const QString &s : qAsConst(op)) {
			rule.pattern = QRegExp(s, Qt::CaseInsensitive);
			rule.color = Data::color(Data::ColorGreenForeground);
			highlightingRules.append(rule);
		}
	}
}

void TextHighlighter::highlightBlock(const QString &text)
{
	for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
		QRegExp expression(rule.pattern);
		int index = 0;

		do {
			index = expression.indexIn(text, index);

			if (index < 0) {
				break;
			}

			int length, lastIndex = index + expression.matchedLength();
			int firstCapturePos, capNb = 1, capLength;

			do {
				firstCapturePos = expression.pos(capNb);
				if (firstCapturePos == -1) {
					length = lastIndex - index;
					capLength = 0;
				} else {
					length = firstCapturePos - index;
					capLength = expression.cap(capNb).size();
				}
				setFormat(index, length, rule.color);

				index += length + capLength;
				capNb++;
			} while (firstCapturePos >= 0);
		} while (index >= 0);
	}
}
