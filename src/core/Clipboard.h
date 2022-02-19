/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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

#include <QList>

#define VERSION_FF7_FIELD_SCRIPT_OPCODES 1
#define MIME_FF7_FIELD_SCRIPT_OPCODES "application/x-makoureactor-ff7-field-script-opcodes"
#define VERSION_FF7_FIELD_SCRIPT_GROUPS 1
#define MIME_FF7_FIELD_SCRIPT_GROUPS "application/x-makoureactor-ff7-field-script-groups"

class Opcode;
class GrpScript;
class QClipboard;

class Clipboard
{
public:
	static Clipboard *instance();
	bool hasFf7FieldScriptOpcodes() const;
	QList<Opcode> ff7FieldScriptOpcodes() const;
	void setFF7FieldScriptOpcodes(const QList<Opcode> &opcodes);
	bool hasFf7FieldScriptGroups() const;
	QList<GrpScript> ff7FieldScriptGroups() const;
	void setFF7FieldScriptGroups(const QList<GrpScript> &groups);
private:
	Clipboard() {}
	static bool hasData(const QString &mimeType);
	static QByteArray data(const QString &mimeType);
	static void setData(const QString &mimeType, const QByteArray &data);

	static Clipboard *_instance;
	
};
