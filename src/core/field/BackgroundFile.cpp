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
#include "BackgroundFile.h"
#include "Field.h"

BackgroundFile::BackgroundFile(Field *field) :
	FieldPart(field), _textures(nullptr)
{
}

BackgroundFile::BackgroundFile(const BackgroundFile &other) :
	FieldPart(other.field()), _textures(nullptr)
{
	setTiles(other.tiles());
}

BackgroundFile::~BackgroundFile()
{
	qDeleteAll(_palettes);
	if (_textures) {
		delete _textures;
	}
}

void BackgroundFile::clear()
{
	qDeleteAll(_palettes);
	_palettes.clear();
	if (_textures) {
		delete _textures;
		_textures = nullptr;
	}
	_tiles.clear();
}

void BackgroundFile::initEmpty()
{

}

QImage BackgroundFile::openBackground(bool transparent, bool *warning)
{
	// Search default background params
	QHash<quint8, quint8> paramActifs;
	qint16 z[] = {-1, -1};
	field()->scriptsAndTexts()->bgParamAndBgMove(paramActifs, z);
	return openBackground(&paramActifs, z, nullptr, nullptr, false, transparent, warning);
}

QImage BackgroundFile::openBackground(const QHash<quint8, quint8> *paramActifs, const qint16 *z,
                                      const bool *layers, const QSet<quint16> *IDs, bool onlyParams,
                                      bool transparent, bool *warning)
{
	return openBackground(tiles().filter(paramActifs, z, layers, IDs, nullptr, onlyParams), _tiles.rect(), transparent, warning);
}

QImage BackgroundFile::openBackground(const BackgroundTiles &tiles, const QRect &area, bool transparent, bool *warning)
{
	if (!isOpen() && !open()) {
		if (warning) {
			*warning = false;
		}
		return QImage();
	}

	return drawBackground(tiles, area, transparent, warning);
}

QImage BackgroundFile::drawBackground(const BackgroundTiles &tiles, const QRect &area, bool transparent, bool *warning) const
{
	if (tiles.isEmpty() || _textures == nullptr) {
		if (warning) {
			*warning = false;
		}
		return QImage();
	}

	QImage image(area.size(), QImage::Format_ARGB32);
	image.fill(transparent ? 0 : 0xFF000000);

	QRgb *pixels = reinterpret_cast<QRgb *>(image.bits());
	bool warned = false; // To prevent verbosity of warnings

	for (const Tile &tile : tiles) {
		QList<uint> indexOrColorList = _textures->tile(tile);

		if (indexOrColorList.isEmpty()) {
			if (!warned) {
				qWarning() << "Texture ID overflow" << tile.textureID;
				warned = true;
			}
			continue;
		}

		quint8 depth = _textures->depth(tile);
		Palette *palette = nullptr;

		if (depth <= 1) {
			if (tile.paletteID >= _palettes.size()) {
				if (!warned) {
					qWarning() << "Palette ID overflow" << tile.paletteID << _palettes.size();
					warned = true;
				}
				continue;
			}
			palette = _palettes.at(tile.paletteID);
		} else if (depth != 2) {
			if (!warned) {
				qWarning() << "Unknown depth" << _textures->depth(tile);
				warned = true;
			}
			continue;
		}

		quint8 right = 0;
		qint32 top = (area.y() + tile.dstY) * area.width();
		qint32 baseX = area.x() + tile.dstX;

		for (uint indexOrColor : qAsConst(indexOrColorList)) {
			if (palette == nullptr) {
				if (indexOrColor != 0) {
					pixels[baseX + right + top] = indexOrColor;
				}
			} else {
				if (palette->notZero(quint8(indexOrColor))) {
					if (tile.blending) {
						pixels[baseX + right + top] = blendColor(tile.typeTrans,
						                                         pixels[baseX + right + top],
						                                         palette->color(quint8(indexOrColor)));
					} else {
						pixels[baseX + right + top] = palette->color(quint8(indexOrColor));
					}
				}
			}

			if (++right == tile.size) {
				right = 0;
				top += area.width();
			}
		}
	}

	if (warning) {
		*warning = warned;
	}

	return image;
}

bool BackgroundFile::usedParams(QMap<LayerParam, quint8> &usedParams, bool *layerExists, QSet<quint16> *usedIDs, QList<QList<quint16> > *effectLayers)
{
	if (!isOpen() && !open()) {
		return false;
	}

	usedParams = tiles().usedParams(layerExists, usedIDs, effectLayers);

	return true;
}

bool BackgroundFile::layerExists(int num)
{
	if (num == 0) {
		return true;
	}

	if (!isOpen() && !open()) {
		return false;
	}

	bool le[3];
	tiles().usedParams(le);

	return le[num - 1];
}

QRgb BackgroundFile::blendColor(quint8 type, QRgb color0, QRgb color1)
{
	int r, g, b;

	switch (type) {
	case 1:
		r = qRed(color0) + qRed(color1);
		if (r > 255)	r = 255;
		g = qGreen(color0) + qGreen(color1);
		if (g > 255)	g = 255;
		b = qBlue(color0) + qBlue(color1);
		if (b > 255)	b = 255;
		break;
	case 2:
		r = qRed(color0) - qRed(color1);
		if (r < 0)	r = 0;
		g = qGreen(color0) - qGreen(color1);
		if (g < 0)	g = 0;
		b = qBlue(color0) - qBlue(color1);
		if (b < 0)	b = 0;
		break;
	case 3:
		r = qRed(color0) + qRed(color1) / 4;
		if (r > 255)	r = 255;
		g = qGreen(color0) + qGreen(color1) / 4;
		if (g > 255)	g = 255;
		b = qBlue(color0) + qBlue(color1) / 4;
		if (b > 255)	b = 255;
		break;
	default://0
		r = (qRed(color0) + qRed(color1)) / 2;
		g = (qGreen(color0) + qGreen(color1)) / 2;
		b = (qBlue(color0) + qBlue(color1)) / 2;
		break;
	}

	return qRgb(r, g, b);
}

bool BackgroundFile::exportTiles(const QString &fileName, const BackgroundTiles &tiles) const
{
	return drawBackground(tiles, _tiles.rect()).save(fileName);
}

bool BackgroundFile::exportLayers(const QString &dirPath, const QString &extension) const
{
	QDir dir(dirPath);

	if (!dir.exists()) {
		dir.mkpath("./");
	}

	bool layerExists[3];
	QSet<quint16> usedIDs;
	tiles().usedParams(layerExists, &usedIDs);

	for (quint8 i = 0 ; i < 4; ++i) {
		if (i == 0 || layerExists[i - 1]) {
			QString fileName = QString("%1_%2_%3." % extension)
			                       .arg(field()->name())
			                       .arg(i == 0 ? 0 : i - 1);
			if (i == 1) {
				for (quint16 ID: qAsConst(usedIDs)) {
					exportTiles(dir.filePath(fileName.arg(ID)), tiles().tilesByID(ID));
				}
			} else {
				exportTiles(dir.filePath(fileName.arg(42)), tiles().tiles(i));
			}
		}
	}

	return true;
}

void BackgroundFile::addPalette(const char *data)
{
	_palettes.append(new Palette(data));

	setModified(true);
}

void BackgroundFile::removePalette(qsizetype palID)
{
	delete _palettes.takeAt(palID);
	_tiles.shiftPalettes(palID, -1);

	setModified(true);
}

void BackgroundFile::setZLayer1(quint16 oldZ, quint16 newZ)
{
	if (oldZ != newZ) {
		_tiles.setZLayer1(oldZ, newZ);
	
		setModified(field()->isPC());
	}
}

bool BackgroundFile::addTile(Tile &tile, uint colorOrIndex)
{
	Q_UNUSED(colorOrIndex)

	tile.tileID = tiles().size();
	qDebug() << "addTile" << tile.layerID << tile.tileID << tile.textureID << tile.srcX << tile.srcY << tile.depth << tile.size << tile.dstX << tile.dstY << tile.ID << tile.IDBig << tile.blending << tile.typeTrans;
	_tiles.insert(tile);
	setModified(field()->isPC());

	return true;
}

void BackgroundFile::createTiles(const QRect &rect, const Tile &model, uint colorOrIndex)
{
	for (int y = rect.top(); y <= rect.bottom(); y += model.size) {
		for (int x = rect.left(); x <= rect.right(); x += model.size) {
			Tile tile = model;
			tile.dstX = x;
			tile.dstY = y;
			addTile(tile, colorOrIndex);
		}
	}
}

bool BackgroundFile::setTile(Tile &tile)
{
	if (_tiles.replace(tile)) {
		setModified(field()->isPC());
		return true;
	}

	return false;
}

bool BackgroundFile::removeTile(const Tile &tile)
{
	if (_tiles.remove(tile)) {
		setModified(field()->isPC());
		return true;
	}

	return false;
}
