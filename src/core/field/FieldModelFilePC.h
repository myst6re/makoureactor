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
#ifndef FIELDMODELFILEPC_H
#define FIELDMODELFILEPC_H

#include <QtGui>
#include "FieldModelFile.h"
#include "FieldModelTextureRefPC.h"

class CharArchive;

class FieldModelFilePC : public FieldModelFile
{
public:
	FieldModelFilePC();
	inline bool translateAfter() const { return true; }
	void clear();
	quint8 load(CharArchive *charLgp, const QString &hrc, const QString &a, bool animate = true);
	quint8 loadPart(CharArchive *charLgp, const QString &rsd);
	quint8 load(const QString &hrc, const QString &a, bool animate = true);
	inline int loadedTextureCount() const {
		return _loadedTex.size();
	}
	inline QImage loadedTexture(FieldModelGroup *group) {
		return _loadedTex.value(static_cast<FieldModelTextureRefPC *>(group->textureRef())->id());
	}
	inline void *textureIdForGroup(FieldModelGroup *group) const {
		return (void *)(ulong(static_cast<FieldModelTextureRefPC *>(group->textureRef())->id()));
	}
	QHash<void *, QImage> loadedTextures();
private:
	Q_DISABLE_COPY(FieldModelFilePC)
	bool openSkeleton(const QString &hrcFileName, QMultiMap<int, QStringList> &rsdFiles);
	bool openAnimation(const QString &aFileName, bool animate = false);
	bool openMesh(QMultiMap<int, QStringList> &rsdFiles, QStringList &textureFiles);
	bool openPart(const QString &rsdFileName, int boneID, QStringList &textureFiles);
	void openTextures(const QStringList &textureFiles);
	QImage openTexture(const QString &texFileName);
	CharArchive *_charLgp;
	QHash<quint32, QImage> _loadedTex;
};

#endif // FIELDMODELFILEPC_H
