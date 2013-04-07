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
#include "BackgroundFile.h"

BackgroundFile::BackgroundFile()
{
}

BackgroundFile::~BackgroundFile()
{
}

void BackgroundFile::area(const QMultiMap<qint16, Tile> &tiles,
						  quint16 &minWidth, quint16 &minHeight,
						  int &width, int &height)
{
	quint16 maxWidth=0, maxHeight=0;
	minWidth = minHeight = 0;

	foreach(const Tile &tile, tiles) {
		quint8 toAdd = tile.size - 16;
		if(tile.dstX >= 0 && tile.dstX+toAdd > maxWidth)
			maxWidth = tile.dstX+toAdd;
		else if(tile.dstX < 0 && -tile.dstX > minWidth)
			minWidth = -tile.dstX;
		if(tile.dstY >= 0 && tile.dstY+toAdd > maxHeight)
			maxHeight = tile.dstY+toAdd;
		else if(tile.dstY < 0 && -tile.dstY > minHeight)
			minHeight = -tile.dstY;
	}

	width = minWidth + maxWidth + 16;
	height = minHeight + maxHeight + 16;
}

QPixmap BackgroundFile::drawBackground(const QMultiMap<qint16, Tile> &tiles,
									   const QList<Palette *> &palettes,
									   const QByteArray &textureData) const
{
	if(tiles.isEmpty()) {
		foreach(Palette *pal, palettes)	delete pal;
		return QPixmap();
	}

	quint16 minWidth, minHeight;
	int width, height;
	area(tiles, minWidth, minHeight, width, height);

	const char *constTextureData = textureData.constData();
	QImage image(width, height, QImage::Format_ARGB32);
	image.fill(0xFF000000);

	quint32 origin, top, texWidth;
	quint16 color, baseX;
	quint8 index, right;
	QRgb *pixels = (QRgb *)image.bits();

	foreach(const Tile &tile, tiles) {
		texWidth = textureWidth(tile);
		origin = originInData(tile);
		right = 0;
		top = (minHeight + tile.dstY) * width;
		baseX = minWidth + tile.dstX;


		if(depth(tile) == 2) {

			for(quint16 j=0 ; j<tile.size*texWidth ; j+=2) {
				memcpy(&color, constTextureData + origin+j, 2);
				if(color != 0) {
					pixels[baseX + right + top] = directColor(color);
				}

				if(++right==tile.size) {
					right = 0;
					j += texWidth-tile.size*2;
					top += width;
				}
			}
		} else {

			Palette *palette = palettes.at(tile.paletteID);

			for(quint16 j=0 ; j<tile.size*texWidth ; ++j) {
				index = textureData.at(origin + j);

				if(palette->notZero(index)) {
					if(tile.blending) {
						pixels[baseX + right + top] = blendColor(tile.typeTrans, pixels[baseX + right + top], palette->color(index));
					} else {
						pixels[baseX + right + top] = palette->color(index);
					}
				}

				if(++right==tile.size) {
					right = 0;
					j += texWidth-tile.size;
					top += width;
				}
			}
		}
	}

	foreach(Palette *pal, palettes)	delete pal;

	return QPixmap::fromImage(image);
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
