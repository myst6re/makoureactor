/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2012 Arzel J�r�me <myst6re@gmail.com>
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
	FieldPart(field)
{
}

BackgroundFile::~BackgroundFile()
{
}

QImage BackgroundFile::openBackground()
{
	// Search default background params
	QHash<quint8, quint8> paramActifs;
	qint16 z[] = {-1, -1};
	field()->scriptsAndTexts()->bgParamAndBgMove(paramActifs, z);
	return openBackground(paramActifs, z);
}

QImage BackgroundFile::drawBackground(const BackgroundTiles &tiles,
									   const QList<Palette *> &palettes,
									   const BackgroundTextures *textures) const
{
	if(tiles.isEmpty()) {
		foreach(Palette *pal, palettes)	delete pal;
		return QImage();
	}

	quint16 minWidth, minHeight;
	int width, height;
	_tiles.area(minWidth, minHeight, width, height);

	QImage image(width, height, QImage::Format_ARGB32);
	image.fill(0xFF000000);

	QRgb *pixels = (QRgb *)image.bits();
	bool warned = false;

	foreach(const Tile &tile, tiles) {
		QVector<uint> indexOrColorList = textures->tile(tile);

		if(indexOrColorList.isEmpty()) {
			if(!warned) {
				qWarning() << "Texture ID overflow" << tile.textureID << tile.textureID2;
				warned = true;
			}
			continue;
		}

		quint8 right = 0;
		quint32 top = (minHeight + tile.dstY) * width;
		quint16 baseX = minWidth + tile.dstX;
		Palette *palette = 0;

		if(textures->depth(tile) == 1
				|| textures->depth(tile) == 0) {
			if(tile.paletteID >= palettes.size()) {
				if(!warned) {
					qWarning() << "Palette ID overflow" << tile.paletteID << palettes.size();
					warned = true;
				}
				continue;
			}

			palette = palettes.at(tile.paletteID);
		} else if(textures->depth(tile) != 2) {
			if(!warned) {
				qWarning() << "Unknown depth" << textures->depth(tile);
				warned = true;
			}
			continue;
		}

		foreach(uint indexOrColor, indexOrColorList) {
			if(textures->depth(tile) == 2) {
				pixels[baseX + right + top] = indexOrColor;
			} else if(textures->depth(tile) == 1
					  || textures->depth(tile) == 0) {
				if(palette->notZero(indexOrColor)) {
					if(tile.blending) {
						pixels[baseX + right + top] = blendColor(tile.typeTrans,
																 pixels[baseX + right + top], palette->color(indexOrColor));
					} else {
						pixels[baseX + right + top] = palette->color(indexOrColor);
					}
				}
			}

			if(++right == tile.size) {
				right = 0;
				top += width;
			}
		}
	}

	foreach(Palette *pal, palettes)	delete pal;

	return image;
}

bool BackgroundFile::usedParams(QHash<quint8, quint8> &usedParams, bool *layerExists)
{
	if(tiles().isEmpty() && !openTiles(field()->sectionData(Field::Background))) {
		return false;
	}

	usedParams = tiles().usedParams(layerExists);

	return true;
}

QRgb BackgroundFile::blendColor(quint8 type, QRgb color0, QRgb color1)
{
	int r, g, b;

	switch(type) {
	case 1:
		r = qRed(color0) + qRed(color1);
		if(r>255)	r = 255;
		g = qGreen(color0) + qGreen(color1);
		if(g>255)	g = 255;
		b = qBlue(color0) + qBlue(color1);
		if(b>255)	b = 255;
		break;
	case 2:
		r = qRed(color0) - qRed(color1);
		if(r<0)	r = 0;
		g = qGreen(color0) - qGreen(color1);
		if(g<0)	g = 0;
		b = qBlue(color0) - qBlue(color1);
		if(b<0)	b = 0;
		break;
	case 3:
		r = qRed(color0) + 0.25*qRed(color1);
		if(r>255)	r = 255;
		g = qGreen(color0) + 0.25*qGreen(color1);
		if(g>255)	g = 255;
		b = qBlue(color0) + 0.25*qBlue(color1);
		if(b>255)	b = 255;
		break;
	default://0
		r = (qRed(color0) + qRed(color1))/2;
		g = (qGreen(color0) + qGreen(color1))/2;
		b = (qBlue(color0) + qBlue(color1))/2;
		break;
	}

	return qRgb(r, g, b);
}
