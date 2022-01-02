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
#include "Clipboard.h"
#include <QApplication>
#include <QClipboard>
#include <QDataStream>
#include <QByteArray>
#include <QIODevice>
#include <QMimeData>
#include "core/field/Script.h"
#include "core/field/GrpScript.h"

Clipboard *Clipboard::_instance = nullptr;

Clipboard *Clipboard::instance()
{
	if (!_instance) {
		_instance = new Clipboard();
	}

	return _instance;
}

bool Clipboard::hasData(const QString &mimeType)
{
	return QApplication::clipboard()->mimeData()->
	        hasFormat(mimeType);
}

QByteArray Clipboard::data(const QString &mimeType)
{
	const QMimeData *mimeData = QApplication::clipboard()->mimeData();
	if (mimeData && mimeData->hasFormat(mimeType)) {
		return mimeData->data(mimeType);
	}
	return QByteArray();
}

void Clipboard::setData(const QString &mimeType, const QByteArray &data)
{
	QMimeData *mimeData = new QMimeData();
	mimeData->setData(mimeType, data);
	QApplication::clipboard()->setMimeData(mimeData);
}

bool Clipboard::hasFf7FieldScriptOpcodes() const
{
	return hasData(MIME_FF7_FIELD_SCRIPT_OPCODES);
}

QList<Opcode> Clipboard::ff7FieldScriptOpcodes() const
{
	QList<Opcode> opcodes;

	QByteArray data = Clipboard::data(MIME_FF7_FIELD_SCRIPT_OPCODES);
	if (!data.isEmpty()) {
		QDataStream stream(data);
		quint16 version;
		stream >> version;
		// Check version
		if (version != VERSION_FF7_FIELD_SCRIPT_OPCODES) {
			return opcodes;
		}
		stream >> opcodes;
	}

	return opcodes;
}

void Clipboard::setFF7FieldScriptOpcodes(const QList<Opcode> &opcodes)
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	stream << quint16(VERSION_FF7_FIELD_SCRIPT_OPCODES)
	       << opcodes;

	setData(MIME_FF7_FIELD_SCRIPT_OPCODES, data);
}

bool Clipboard::hasFf7FieldScriptGroups() const
{
	return hasData(MIME_FF7_FIELD_SCRIPT_GROUPS);
}

QList<GrpScript> Clipboard::ff7FieldScriptGroups() const
{
	QList<GrpScript> groups;

	QByteArray data = Clipboard::data(MIME_FF7_FIELD_SCRIPT_GROUPS);
	if (!data.isEmpty()) {
		QDataStream stream(data);
		quint16 version;
		stream >> version;
		// Check version
		if (version != VERSION_FF7_FIELD_SCRIPT_GROUPS) {
			return groups;
		}
		stream >> groups;
	}

	return groups;
}

void Clipboard::setFF7FieldScriptGroups(const QList<GrpScript> &groups)
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	stream << quint16(VERSION_FF7_FIELD_SCRIPT_GROUPS)
	       << groups;
	setData(MIME_FF7_FIELD_SCRIPT_GROUPS, data);
}
