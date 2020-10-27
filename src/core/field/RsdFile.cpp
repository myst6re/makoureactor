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
#include "RsdFile.h"

RsdFile::RsdFile(QIODevice *io) :
	IO(io)
{
}

bool RsdFile::read(Rsd &rsd, QStringList &textureNames) const
{
	if (!canRead()) {
		return false;
	}

	QString pname;
	QList<int> texIds;
	quint32 nTex = 0;
	int index;
	bool ok;

	while (device()->canReadLine()) {
		QString line = QString(device()->readLine()).trimmed();
		if (pname.isNull() && (line.startsWith(QString("PLY="))
							  || line.startsWith(QString("MAT="))
							  || line.startsWith(QString("GRP=")))) {
			index = line.lastIndexOf('.');
			if (index != -1) {
				line.truncate(index);
			}
			pname = line.mid(4).toLower();
		} else if (!pname.isNull() && nTex == 0
				  && line.startsWith(QString("NTEX="))) {
			nTex = line.mid(5).toUInt(&ok);
			if (!ok) {
				return false;
			}

			for (quint32 i = 0; i < nTex && device()->canReadLine(); ++i) {
				line = QString(device()->readLine()).trimmed();
				if (!line.startsWith(QString("TEX[%1]=").arg(i))) {
					return false;
				}

				index = line.lastIndexOf('.');
				if (index != -1) {
					line.truncate(index);
				}
				QString tex = line.mid(line.indexOf('=') + 1).toLower();

				index = textureNames.indexOf(tex);
				if (index > -1) {
					texIds.append(index);
				} else {
					texIds.append(textureNames.size());
					textureNames.append(tex);
				}
			}
		}
	}

	rsd.pName = pname;
	rsd.texIds = texIds;

	return true;
}

bool RsdFile::write(const Rsd &rsd, const QStringList &textureNames) const
{
	Q_UNUSED(rsd)
	Q_UNUSED(textureNames)
	// TODO
	return false;
}
