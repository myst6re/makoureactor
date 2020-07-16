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
#include "HrcFile.h"
#include "RsdFile.h"
#include "PFile.h"
#include "AFile.h"
#include "../TexFile.h"

FieldModelFilePC::FieldModelFilePC() :
	FieldModelFile(), _charLgp(0)
{
}

void FieldModelFilePC::clear()
{
	_loadedTex.clear();
	FieldModelFile::clear();
}

void FieldModelFilePC::openTextures(const QStringList &textureFiles)
{
	// Open all loaded tex
	int texID = 0;
	for (const QString &texName : textureFiles) {
		QImage tex = openTexture(texName % ".tex");
		if (!tex.isNull()) {
			_loadedTex.insert(texID, tex);
		} else {
			qWarning() << "FieldModelFilePC::load error texture" << texName;
		}
		++texID;
	}
}

quint8 FieldModelFilePC::load(CharArchive *charLgp, const QString &hrc, const QString &a, bool animate)
{
	if (hrc.isEmpty() || a.isEmpty() || !charLgp->isOpen()) {
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

			if (openAnimation(aFilename % ".a", animate) || _skeleton.boneCount() == 1) {
				openTextures(textureFiles);

				return true;
			} else {
				qWarning() << "FieldModelFilePC::load error animation" << hrcFilename << aFilename;
			}
		} else {
			qWarning() << "FieldModelFilePC::load error mesh" << hrcFilename;
		}
	} else {
		qWarning() << "FieldModelFilePC::load error skeleton" << hrcFilename;
	}

	return false;
}

quint8 FieldModelFilePC::loadPart(CharArchive *charLgp, const QString &rsd)
{
	if (rsd.isEmpty() || !charLgp->isOpen()) {
		return 2;
	}
	_charLgp = charLgp;
	_skeleton.addBone(FieldModelBone(0.0, -1));

	QStringList textureFiles;
	if (openPart(rsd.toLower(), 0, textureFiles)) {
		openTextures(textureFiles);

		return 0;
	}

	qWarning() << "FieldModelFilePC::openMesh cannot open part" << rsd;

	return 1;
}

quint8 FieldModelFilePC::load(const QString &hrc, const QString &a, bool animate)
{
	return load(CharArchive::instance(), hrc, a, animate);
}

QHash<void *, QImage> FieldModelFilePC::loadedTextures()
{
	QHash<void *, QImage> ret;
	QHashIterator<int, QImage> it(_loadedTex);

	while (it.hasNext()) {
		it.next();
		ret.insert((void *)it.key(), it.value());
	}

	return ret;
}

bool FieldModelFilePC::openSkeleton(const QString &hrcFileName, QMultiMap<int, QStringList> &rsdFiles)
{
	HrcFile io(_charLgp->fileIO(hrcFileName));
	return io.read(_skeleton, rsdFiles);
}

bool FieldModelFilePC::openAnimation(const QString &aFileName, bool animate)
{
	FieldModelAnimation animation;
	AFile io(_charLgp->fileIO(aFileName));
	if (io.read(animation, animate ? -1 : 1)) {
		_animations.append(animation);
		return true;
	}
	return false;
}

bool FieldModelFilePC::openMesh(QMultiMap<int, QStringList> &rsdFiles, QStringList &textureFiles)
{
	bool onePartOpened = false;
	QMapIterator<int, QStringList> itRsd(rsdFiles);
	while (itRsd.hasNext()) {
		itRsd.next();
		int boneID = itRsd.key();

		for (const QString &rsd : itRsd.value()) {
			if (openPart(rsd.toLower() % ".rsd", boneID, textureFiles)) {
				onePartOpened = true;
			} else {
				qWarning() << "FieldModelFilePC::openMesh cannot open part" << rsd;
			}
		}
	}

	return onePartOpened;
}

bool FieldModelFilePC::openPart(const QString &rsdFileName, int boneID, QStringList &textureFiles)
{
	RsdFile rsdIO(_charLgp->fileIO(rsdFileName));
	Rsd rsd;
	if (rsdIO.read(rsd, textureFiles)) {
		PFile partIO(_charLgp->fileIO(rsd.pFile() % ".p"));

		FieldModelPart *part = new FieldModelPart();
		if (partIO.read(part, rsd.textureIds())) {
			_skeleton[boneID].addPart(part);
			return true;
		} else {
			qWarning() << "FieldModelFilePC::openPart part error" << rsdFileName << rsd.pFile();
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
	if (!tex.isValid()) {
		return QImage();
	}
	return tex.image();
}
