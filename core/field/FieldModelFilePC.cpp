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
#include "FieldModelFilePC.h"
#include "CharArchive.h"
#include "FieldModelSkeletonIOPC.h"
#include "FieldModelPartIOPC.h"
#include "FieldModelAnimationIOPC.h"
#include "../TexFile.h"

FieldModelFilePC::FieldModelFilePC() :
	FieldModelFile()
{
}

quint8 FieldModelFilePC::load(const QString &hrc, const QString &a, bool animate)
{
	if (hrc.isEmpty() || a.isEmpty()) {
		return 2;
	}

	CharArchive *charLgp = CharArchive::instance();
	if (!charLgp->isOpen()) {
		return 2;
	}
	_charLgp = charLgp;

	QString hrcFilename, aFilename;
	int index;

	index = hrc.lastIndexOf('.');
	hrcFilename = index > -1 ? hrc.left(index) : hrc;
	index = a.lastIndexOf('.');
	aFilename = index > -1 ? a.left(index) : a;

	clear();

	QMultiMap<int, QStringList> rsdFiles;

	if (openSkeleton(hrcFilename % ".hrc", rsdFiles)) {
		QStringList textureFiles;

		if (openMesh(rsdFiles, textureFiles)) {

			if (openAnimation(aFilename % ".a", animate)) {
				// Open all loaded tex
				int texID = 0;
				foreach(const QString &texName, textureFiles) {
					QImage tex = openTexture(texName % ".tex");
					if (!tex.isNull()) {
						_loadedTex.insert(texID, tex);
					}
					++texID;
				}

				dataLoaded = true;
			}
		}
	}

	return dataLoaded;
}

bool FieldModelFilePC::openSkeleton(const QString &hrcFileName, QMultiMap<int, QStringList> &rsdFiles)
{
	FieldModelSkeletonIOPC io(_charLgp->fileIO(hrcFileName));
	return io.read(_skeleton, rsdFiles);
}

bool FieldModelFilePC::openAnimation(const QString &aFileName, bool animate)
{
	FieldModelAnimationIOPC io(_charLgp->fileIO(aFileName));
	return io.read(_animation, animate ? -1 : 1);
}

bool FieldModelFilePC::openMesh(QMultiMap<int, QStringList> &rsdFiles, QStringList &textureFiles)
{
	QMapIterator<int, QStringList> itRsd(rsdFiles);
	while (itRsd.hasNext()) {
		itRsd.next();
		int boneID = itRsd.key();

		foreach (const QString &rsd, itRsd.value()) {
			openPart(rsd.toLower() % ".rsd", boneID, textureFiles);
		}
	}

	return !_parts.isEmpty();
}

bool FieldModelFilePC::openPart(const QString &rsdFileName, int boneID, QStringList &textureFiles)
{
	FieldModelPartIOPC partIO(_charLgp->fileIO(rsdFileName));
	Rsd rsd;
	if (partIO.readRsd(rsd, textureFiles)) {
		QIODevice *pFile = _charLgp->fileIO(rsd.pFile() % ".p");
		partIO.setDevice(pFile);

		FieldModelPart *part = new FieldModelPart();
		if (partIO.read(part, rsd.textureIds())) {
			_parts.insert(boneID, part);
			return true;
		} else {
			delete part;
		}
	}
	return false;
}

QImage FieldModelFilePC::openTexture(const QString &texFileName)
{
	QIODevice *texFile = _charLgp->fileIO(texFileName);
	if (!texFile || !texFile->open(QIODevice::ReadOnly)) {
		return QImage();
	}
	TexFile tex(texFile->readAll());
	if(!tex.isValid()) {
		return QImage();
	}
	return tex.image();
}
