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
#ifndef BACKGROUNDFILE_H
#define BACKGROUNDFILE_H

#include <QtCore>
#include <QImage>
#include "FieldPart.h"
#include "Palette.h"
#include "BackgroundTiles.h"
#include "BackgroundTextures.h"

class BackgroundFile : public FieldPart
{
public:
	explicit BackgroundFile(Field *field);
	BackgroundFile(const BackgroundFile &other);
	virtual ~BackgroundFile();

	virtual void initEmpty();
	using FieldPart::open;
	using FieldPart::save;
	virtual inline bool canSave() const { return false; }
	void clear();
	QImage openBackground(bool *warning = NULL);
	QImage openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 z[2],
	                      const bool *layers = NULL, const QSet<quint16> *IDs = NULL,
	                      bool *warning = NULL);
	// Draw background tiles with ID
	QImage backgroundPart(quint16 ID, bool *warning = NULL);
	bool usedParams(QHash<quint8, quint8> &usedParams, bool *layerExists, QSet<quint16> *usedIDs);
	bool layerExists(int num);

	inline const BackgroundTiles &tiles() const {
		return _tiles;
	}
	inline void setTiles(const BackgroundTiles &tiles) {
		_tiles = tiles;
	}

	inline const Palettes &palettes() const {
		return _palettes;
	}
	inline void setPalettes(const Palettes &palettes) {
		qDeleteAll(_palettes);
		_palettes = palettes;
	}

	inline virtual BackgroundTextures *textures() const {
		return _textures;
	}
	inline void setTextures(BackgroundTextures *textures) {
		if(_textures) {
			delete _textures;
		}
		_textures = textures;
	}

	virtual inline bool repair() {
		return false;
	}

protected:
	QImage drawBackground(const BackgroundTiles &tiles, bool *warning = NULL) const;
	static QRgb blendColor(quint8 type, QRgb color0, QRgb color1);
	inline BackgroundTiles &tilesRef() {
		return _tiles;
	}

private:
	bool open(const QByteArray &data) {
		Q_UNUSED(data);
		return false;
	}
	BackgroundTiles _tiles;
	Palettes _palettes;
	BackgroundTextures *_textures;
};

#endif // BACKGROUNDFILE_H
