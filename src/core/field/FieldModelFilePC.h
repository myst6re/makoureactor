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

#include <QtGui>
#include "FieldModelFile.h"
#include "FieldModelTextureRefPC.h"

class CharArchive;

class FieldModelFilePC : public FieldModelFile
{
public:
	FieldModelFilePC();
	inline bool translateAfter() const override { return true; }
	void clear() override;
	quint8 load(CharArchive *charLgp, const QString &hrc, const QString &a, bool animate = true);
	quint8 loadPart(CharArchive *charLgp, const QString &rsd);
	quint8 load(const QString &hrc, const QString &a, bool animate = true);
	inline qsizetype loadedTextureCount() const {
		return _loadedTex.size();
	}
	inline QImage loadedTexture(FieldModelGroup *group) override {
		return _loadedTex.value(static_cast<FieldModelTextureRefPC *>(group->textureRef())->id());
	}
	inline void *textureIdForGroup(FieldModelGroup *group) const override {
		return reinterpret_cast<void *>(quint64(static_cast<FieldModelTextureRefPC *>(group->textureRef())->id()));
	}
	QHash<void *, QImage> loadedTextures() override;
private:
	Q_DISABLE_COPY(FieldModelFilePC)
	bool openSkeleton(const QString &hrcFileName, QMultiMap<int, QStringList> &rsdFiles);
	bool openAnimation(const QString &aFileName, bool animate = false);
	bool openMesh(QMultiMap<int, QStringList> &rsdFiles, QStringList &textureFiles);
	bool openPart(const QString &rsdFileName, int boneID, QStringList &textureFiles);
	void openTextures(const QStringList &textureFiles);
	QImage openTexture(const QString &texFileName);
	CharArchive *_charLgp;
	QHash<quint64, QImage> _loadedTex;
};
