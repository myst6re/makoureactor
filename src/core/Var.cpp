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
#include "Var.h"
#include "Config.h"

bool Var::load()
{
	// Loading file into Var::_varNames
	QFile fic(Config::value("varFile").toString());
	if (!fic.exists()) {
		fic.setFileName(":/vars.cfg"); // Get default values
	}

	if (fic.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QString line;
		QStringList list;
		bool ok;
		int address;
		forever {
			line = QString::fromUtf8(fic.readLine());
			if (line.isEmpty()) {
				break;
			}

			list = line.split(QChar('|'));
			if (list.size() != 3) {
				return false;
			}

			int bank = list.first().toInt(&ok);
			if (!ok) {
				return false;
			}
			if (bank < 1 || bank > 15) {
				continue;
			}

			address = list.at(1).toInt(&ok);
			if (!ok) {
				return false;
			}
			if (address < 0 || address > 255) {
				continue;
			}

			line = list.at(2);
			if (line.isEmpty()) {
				return false;
			}
			if (line.size() > 255) {
				line = line.left(255);
			}

			set(bank, address, line.simplified());
		}
		fic.close();

		return true;
	}

	return false;
}

bool Var::save(const QMap<quint16, QString> &varNames)
{
	// Saving varNames in the file
	QFile fic(Config::value("varFile").toString());
	if (fic.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		_varNames = varNames;
		QMapIterator<quint16, QString> i(_varNames);
		while (i.hasNext()) {
			i.next();
			fic.write(QString("%1|%2|%3\n")
			          .arg(i.key() >> 8)
			          .arg(i.key() & 255)
			          .arg(i.value()).toLocal8Bit());
		}
		fic.close();

		return true;
	}

	return false;
}

QMap<quint16, QString> Var::_varNames;

QString Var::name(quint8 bank, quint8 address)
{
	return _varNames.value(address | (bank << 8));
}

void Var::set(quint8 bank, quint8 address, const QString &name)
{
	_varNames.insert(address | (bank << 8), name);
}

void Var::del(quint8 bank, quint8 address)
{
	_varNames.remove(address | (bank << 8));
}

bool Var::exists(quint8 bank, quint8 address)
{
	return _varNames.contains(address | (bank << 8));
}
