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
#include "HrcFile.h"
#include "PFile.h"

HrcFile::HrcFile(QIODevice *io) :
	IO(io)
{
}

HrcFile::~HrcFile()
{
}

bool HrcFile::readHeader(int &boneCount) const
{
	if (!canRead()) {
		return false;
	}
	
	bool ok;
	QString line;
	boneCount = 0;
	
	do {
		line = QString::fromLatin1(device()->readLine()).trimmed();
		if (line.startsWith(":BONES ")) {
			boneCount = QStringView(line).mid(7).toInt(&ok);
			if (!ok || boneCount < 0) {
				qWarning() << "HrcFile::read :BONES not a number" << line;
				return false;
			}
			if (boneCount == 0) {
				boneCount = 1; // Null HRC fix
			}
			break;
		}
	} while (device()->canReadLine());
	
	return boneCount != 0;
}

bool HrcFile::read(FieldModelSkeleton &skeleton, QMultiMap<int, QStringList> &rsdFiles) const
{
	if (!canRead()) {
		return false;
	}
	int boneCount = 0;
	
	if (!readHeader(boneCount)) {
		return false;
	}

	bool ok;
	QString line;
	int nbP, lineType = 0;
	int boneID = 0;
	FieldModelBone bone(0, 0);
	QMap<QString, int> nameToId;
	QStringList rsdlist;
	nameToId.insert("root", -1);

	while (device()->canReadLine() && boneID < boneCount) {
		line = QString::fromLatin1(device()->readLine()).trimmed();
		if (line.isEmpty() || line.startsWith('#')) {
			if (!line.isEmpty()) {
				qDebug() << "commented line" << line;
			}
			continue;
		}

		switch (lineType) {
		case 0: // Current
			nameToId.insert(line, boneID);
			break;
		case 1: // Parent
			bone.setParent(nameToId.value(line, -1));
			break;
		case 2: // Length
			bone.setSize(-line.toFloat(&ok) / MODEL_SCALE_PC);
			if (!ok) {
				qWarning() << "HrcFile::read not a number" << line;
				return false;
			}
			skeleton.addBone(bone);
			break;
		case 3: // RSD list
			rsdlist = line.split(' ', Qt::SkipEmptyParts);
			if (rsdlist.size() < 1) {
				return false;
			}
			nbP = rsdlist.first().toInt(&ok);
			if (ok && nbP > 0) {
				rsdlist.removeFirst();
				if (rsdlist.size() == nbP) {
					rsdFiles.insert(boneID, rsdlist);
				}
			} else if (!ok) {
				qWarning() << "HrcFile::read not a number" << rsdlist;
			}
			++boneID;
			break;
		}
		lineType = (lineType + 1) % 4;
	}

	return boneID == boneCount;
}

bool HrcFile::write(const FieldModelSkeleton &skeleton) const
{
	Q_UNUSED(skeleton)
	return false; // TODO
}
