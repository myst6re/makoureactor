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
#include "PsfFile.h"

#include <GZIP.h>

PsfTags::PsfTags()
{
}

PsfTags::PsfTags(const QString &lib)
{
	setLib(lib);
}

bool PsfTags::open(const QString &config)
{
	_tags.clear();

	if (!config.startsWith("[TAG]")) {
		return false;
	}

	QStringList entries = config.mid(5).split('\n', Qt::SkipEmptyParts);
	for (const QString &entry : qAsConst(entries)) {
		qsizetype index = entry.indexOf('=');

		if (index >= 0) {
			QString name = entry.left(index),
			        value = entry.mid(index + 1);

			if (_tags.contains(name)) {
				_tags[name].append("\n").append(value);
			} else {
				_tags.insert(name, value);
			}
		} else {
			qWarning() << "PsfTags::open Unknown entry format";
		}
	}

	return true;
}

QString PsfTags::save() const
{
	QString config;
	config.append("[TAG]");

	QMapIterator<QString, QString> it(_tags);

	while (it.hasNext()) {
		it.next();

		QStringList values = it.value().split('\n');

		for (const QString &value : qAsConst(values)) {
			config.append(QString("%1=%2\n").arg(it.key(), value));
		}
	}

	return config;
}

PsfTags &PsfTags::setLengthS(quint32 seconds)
{
	return setCustom("length", QString("%1:%2").arg(seconds / 60).arg(seconds % 60));
}

PsfTags &PsfTags::setReplayGain(const QString &type, double gain_db, double peak)
{
	return setCustom(QString("replaygain_%1_gain").arg(type), QString("%1%2")
	                                                              .arg(gain_db < 0 ? QLatin1String("-") : QLatin1String("+"))
	                                                              .arg(gain_db))
	    .setCustom(QString("replaygain_%1_peak").arg(type), QString::number(peak));
}

PsfFile::PsfFile()
{

}

PsfFile::PsfFile(const QByteArray &data, const PsfTags &tags) :
      _data(data), _tags(tags)
{
}

PsfFile PsfFile::fromAkao(const QByteArray &akaoData, const PsfTags &tags)
{
	QByteArray data;

	data.append(QString("PS-X EXE").toLatin1())
	    .append(QByteArray(10, '\0'))
	    .append("\x01\x80", 2)
	    .append(QByteArray(6, '\0'))
	    .append("\x1D\x80\x00\x80", 4)
	    .append(QByteArray(18, '\0'))
	    .append("\xF0\xFF\x1F\x80", 4)
	    .append(QByteArray(0x7CC, '\0'))
	    .append(akaoData);

	return PsfFile(data, tags);
}

bool PsfFile::open(const QByteArray &data)
{
	if (data.size() < 16 || QString::fromLatin1(data.constData(), 3) != QLatin1String("PSF")) {
		qWarning() << "PsfFile::open" << "wrong header";
		return false;
	}

	quint8 version = quint8(data.at(3));

	if (version != 1) {
		qWarning() << "PsfFile::open" << "unsupported version" << version;
		return false;
	}

	quint32 sizes[3];

	memcpy(sizes, data.constData() + 4, 3 * sizeof(quint32));

	_special = data.mid(16, sizes[0]);
	_data = GZIP::decompressNoHeader(data.constData() + 16 + sizes[0], int(sizes[1]));

	return _tags.open(QString::fromLatin1(data.constData() + 16 + sizes[0] + sizes[1]));
}

QByteArray PsfFile::save() const
{
	QByteArray data;
	quint8 version = 1;
	data.append("PSF").append(char(version));

	QByteArray compressedData = GZIP::compressNoHeader(_data.constData(), int(_data.size()));

	quint32 specialSize = quint32(_special.size());
	data.append((char *)&specialSize, 4);
	quint32 dataSize = quint32(compressedData.size());
	data.append((char *)&dataSize, 4);
	quint32 crc = GZIP::crc(compressedData.constData(), int(compressedData.size()));
	data.append((char *)&crc, 4);

	data.append(_special);
	data.append(compressedData);
	data.append(_tags.save().toLatin1());

	return data;
}

QByteArray PsfFile::akao() const
{
	qsizetype index = _data.indexOf("AKAO");
	if (index >= 0) {
		return _data.mid(index);
	}

	return QByteArray();
}
