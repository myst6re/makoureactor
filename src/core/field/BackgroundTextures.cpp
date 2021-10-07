/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#include "BackgroundTextures.h"

BackgroundTextures::BackgroundTextures()
{
}

BackgroundTextures::~BackgroundTextures()
{
}

QVector<uint> BackgroundTextures::tile(const Tile &tile) const
{
	QVector<uint> indexOrRgbList;
	quint8 depth = this->depth(tile),  x = 0;
	quint8 multiplicator = depth == 0 ? 1 : depth * 2;

	int origin = originInData(tile);

	if(origin == -1) {
		return indexOrRgbList;
	}

	int texWidth = textureWidth(tile);
	int maxByte = data().size();
	if(depth == 2) {
		--maxByte;
	}
	int lastByte = qMin(origin + tile.size * texWidth, maxByte);

	for (int i = origin; i < lastByte; ++i) {
		if(depth == 0) {
			quint8 index = quint8(data().at(i));
			indexOrRgbList.append(index & 0xF);
			++x;
			indexOrRgbList.append(index >> 4);
		} else if(depth == 1) {
			indexOrRgbList.append(quint8(data().at(i)));
		} else if(depth == 2) {
			indexOrRgbList.append(pixel(quint32(i)));
			++i;
		}

		if(++x == tile.size) {
			x = 0;
			i += texWidth - tile.size * multiplicator / 2;
		}
	}

	return indexOrRgbList;
}

QRgb BackgroundTextures::pixel(quint32 pos) const
{
	quint16 color;
	memcpy(&color, data().constData() + pos, 2);
	return directColor(color);
}

BackgroundTexturesPC::BackgroundTexturesPC() :
	BackgroundTextures()
{
}

BackgroundTexturesPC::BackgroundTexturesPC(const QHash<quint8, BackgroundTexturesPCInfos> &texInfos) :
	BackgroundTextures()
{
	setTexInfos(texInfos);
}

bool BackgroundTexturesPC::hasTex(quint8 texID) const
{
	return _texInfos.contains(texID);
}

quint32 BackgroundTexturesPC::texPos(quint8 texID) const
{
	return _texInfos.value(texID).pos;
}

quint8 BackgroundTexturesPC::texDepth(quint8 texID) const
{
	return _texInfos.value(texID).depth;
}

quint8 BackgroundTexturesPC::texTileSize(quint8 texID) const
{
	return _texInfos.value(texID).size;
}

BackgroundTexturesPCInfos BackgroundTexturesPC::texInfos(quint8 texID) const
{
	return _texInfos.value(texID);
}

void BackgroundTexturesPC::addTexInfos(quint8 texID, const BackgroundTexturesPCInfos &infos)
{
	_texInfos.insert(texID, infos);
}

void BackgroundTexturesPC::clear()
{
	_texInfos.clear();
	BackgroundTextures::clear();
}

void BackgroundTexturesPC::setTexInfos(const QHash<quint8, BackgroundTexturesPCInfos> &texInfos)
{
	_texInfos = texInfos;
}

QList<uint> BackgroundTexturesPC::tex(quint8 texID) const
{
	QList<uint> indexOrRgbList;
	BackgroundTexturesPCInfos infos = texInfos(texID);
	int size = infos.depth == 0 ? 32768 : infos.depth * 65536;

	for (int i=0; i<size; ++i) {
		if(infos.depth == 0) {
			quint8 index = quint8(data().at(infos.pos + i));
			indexOrRgbList.append(index & 0xF);
			indexOrRgbList.append(index >> 4);
		} else if(infos.depth == 1) {
			indexOrRgbList.append(quint8(data().at(infos.pos + i)));
		} else if(infos.depth == 2) {
			indexOrRgbList.append(pixel(infos.pos + i));
			++i;
		}
	}

	return indexOrRgbList;
}

void BackgroundTexturesPC::setTex(quint8 texID, const QList<uint> &indexOrRgbList, const BackgroundTexturesPCInfos &_infos)
{
	if(indexOrRgbList.size() != 256 * 256 || texID >= BACKGROUND_TEXTURE_PC_MAX_COUNT) {
		qWarning() << "BackgroundTexturesPC::setTex invalid arguments";
		return;
	}

	BackgroundTexturesPCInfos infos = _infos;
	QByteArray texData;

	for (uint indexOrRgb : indexOrRgbList) {
		if(infos.depth == 0 || infos.depth == 1) {
			texData.append(char(indexOrRgb));
		} else if(infos.depth == 2) {
			quint16 color = toPcColor(indexOrRgb);
			texData.append((char *)&color, 2);
		}
	}

	QByteArray _data = data();

	if(hasTex(texID)) {
		BackgroundTexturesPCInfos oldInfos = texInfos(texID);
		quint32 oldDataSize = oldInfos.depth == 0 ? 32768 : oldInfos.depth * 65536;
		setData(_data.replace(oldInfos.pos, oldDataSize, texData));
		int diff = texData.size() - oldDataSize;
		QHashIterator<quint8, BackgroundTexturesPCInfos> it(_texInfos);
		while (it.hasNext()) {
			it.next();

			if(it.value().pos > oldInfos.pos) {
				BackgroundTexturesPCInfos othInfos = it.value();
				othInfos.pos += diff;
				_texInfos.insert(it.key(), othInfos);
			}
		}
	} else {
		infos.pos = _data.size();
		setData(_data.append(texData));
	}

	_texInfos.insert(texID, infos);
}

quint16 BackgroundTexturesPC::textureWidth(const Tile &tile) const
{
	return depth(tile) == 0 ? 128 : depth(tile) * 256;
}

quint8 BackgroundTexturesPC::depth(const Tile &tile) const
{
	/* When tile.depth is used, it can be buggy,
	 * because the PC version doesn't understand
	 * depth = 0. */
	if(hasTex(tile.textureID)) {
		return texDepth(tile.textureID);
	}
	return BackgroundTextures::depth(tile);
}

int BackgroundTexturesPC::originInData(const Tile &tile) const
{
	if(hasTex(tile.textureID)) {
		return texPos(tile.textureID) + (tile.srcY * 256 + tile.srcX) * depth(tile);
	}
	return -1;
}

QRgb BackgroundTexturesPC::directColor(quint16 color) const
{
	quint8 b = color & 31,
	       g = (color >> 6) & 31,
	       r = color >> 11;

	// special PC RGB16 color
	return qRgba((r << 3) + (r >> 2), (g << 3) + (g >> 2), (b << 3) + (b >> 2), color == 0 ? 0 : 255);
}

quint16 BackgroundTexturesPC::toPcColor(const QRgb &color)
{
	// alpha ignored!
	return ((qRound(qRed(color) / COEFF_COLOR) & 31) << 11) |
			((qRound(qGreen(color) / COEFF_COLOR) & 31) << 6) |
			(qRound(qBlue(color) / COEFF_COLOR) & 31); // special PC RGB16 color
}

QImage BackgroundTexturesPC::toImage(quint8 texID) const
{
	QList<uint> indexOrRgbList = tex(texID);
	const BackgroundTexturesPCInfos &infos = texInfos(texID);
	QImage img(256, 256, QImage::Format_ARGB32);
	img.fill(Qt::black);
	QRgb *bits = (QRgb *)img.bits();

	for (uint indexOrRgb : qAsConst(indexOrRgbList)) {
		if(infos.depth != 2) {
			*bits = qRgb(indexOrRgb, indexOrRgb, indexOrRgb);
		} else {
			*bits = indexOrRgb;
		}
		bits++;
	}

	return img;
}

QImage BackgroundTexturesPC::toImage(quint8 texID, const BackgroundTiles &tiles, const Palettes &palettes) const
{
	const BackgroundTexturesPCInfos &infos = texInfos(texID);

	if(infos.depth == 2) {
		return toImage(texID);
	}

	QImage img(256, 256, QImage::Format_ARGB32);
	img.fill(Qt::black);
	QRgb *bits = (QRgb *)img.bits();

	for (const Tile &tile : tiles) {
		if(tile.textureID == texID) {
			QVector<uint> indexOrRgbList = this->tile(tile);
			Palette *palette = nullptr;
			quint16 pos = tile.srcY * 256 + tile.srcX;
			quint8 x = 0, y = 0;

			if(tile.paletteID < palettes.size()) {
				palette = palettes.at(tile.paletteID);
			}

			for (uint indexOrRgb : qAsConst(indexOrRgbList)) {
//				if(tile.depth == 0) {
//					bits[pos + y * 256 + x] = qRgb(0, 255, 0);
//				} else if(tile.depth == 1) {
//					bits[pos + y * 256 + x] = qRgb(0, 0, 255);
//				}
				if(palette) {
					bits[pos + y * 256 + x] = palette->color(indexOrRgb);
				} else {
					bits[pos + y * 256 + x] = qRgb(indexOrRgb, indexOrRgb, indexOrRgb);
				}

				if(++x >= tile.size) {
					x = 0;
					++y;
				}
			}
		}
	}

	return img;
}

BackgroundTexturesPS BackgroundTexturesPC::toPS(const BackgroundTiles &pcTiles,
												BackgroundTiles &psTiles,
												const PalettesPS &palettesPS) const
{
	Q_UNUSED(pcTiles)
	Q_UNUSED(psTiles)
	Q_UNUSED(palettesPS)

	BackgroundTexturesPS ret;

	// TODO: toPS

	return ret;
}

BackgroundTexturesPS::BackgroundTexturesPS() :
	BackgroundTextures(),
	_headerImg(MIM()), _headerEffect(MIM())
{
}

quint32 BackgroundTexturesPS::pageDataPos(quint8 pageID) const
{
	return 12 + (pageID ? _headerImg.size : 0);
}

quint16 BackgroundTexturesPS::pageTexPos(quint8 pageID) const
{
	return (pageID ? _headerEffect.x : _headerImg.x) / 64;
}

quint16 BackgroundTexturesPS::pageTexWidth(quint8 pageID) const
{
	return pageID ? _headerEffect.w : _headerImg.w;
}

void BackgroundTexturesPS::setHeaderImg(const MIM &headerImg)
{
	_headerImg = headerImg;
}

void BackgroundTexturesPS::setHeaderEffect(const MIM &headerEffect)
{
	_headerEffect = headerEffect;
}

QList<uint> BackgroundTexturesPS::tex(quint8 x, quint8 y, quint8 depth) const
{
	QList<uint> indexOrRgbList;
	quint32 pos = texturePos(x, y);
	int curPos;
	quint16 pageTexW = pageTexWidth(y);

	for (int texY=0; texY<256; ++texY) {
		for (int texX=0; texX<256; ++texX) {
			curPos = pos + texY * pageTexW + (depth == 0 ? texX/2 : texX * depth);

			if(depth == 0) {
				quint8 index = curPos < data().size() ? data().at(curPos) : 0;
				indexOrRgbList.append(index & 0xF);
				++texX;
				indexOrRgbList.append(index >> 4);
			} else if(depth == 1) {
				indexOrRgbList.append(
							curPos < data().size()
							? quint8(data().at(curPos))
							: 0);
			} else if(depth == 2) {
				indexOrRgbList.append(curPos + 1 < data().size()
									  ? pixel(curPos)
									  : 0);
			}
		}
	}

	return indexOrRgbList;
}

TimFile BackgroundTexturesPS::tim(quint8 pageID, quint8 depth) const
{
	QByteArray header("\x10\x00\x00\x00"
					  "\x00\x00\x00\x00", 8);
	header[4] = char(depth < 2 ? depth | 8 : 2);

	return TimFile(header + (pageID ?
					   data().left(int(pageDataPos(0)) - 12) // Palettes
					   + data().mid(int(pageDataPos(1)) - 12) // Page 2
					 : data().left(int(pageDataPos(1)) - 12))); // Palettes + Page 1
}

quint16 BackgroundTexturesPS::textureWidth(const Tile &tile) const
{
	return pageTexWidth(tile.textureID2);
}

quint32 BackgroundTexturesPS::texturePos(quint8 x, quint8 y) const
{
	quint16 texID = x - pageTexPos(y);
	return pageDataPos(y) + texID * 128;
}

int BackgroundTexturesPS::originInData(const Tile &tile) const
{
	quint32 textureStart = texturePos(tile.textureID, tile.textureID2);
	quint32 tileStart = tile.srcY * textureWidth(tile) + (tile.depth == 0 ? tile.srcX / 2 : tile.srcX * tile.depth);
	return int(textureStart + tileStart);
}

QRgb BackgroundTexturesPS::directColor(quint16 color) const
{
	return PsColor::fromPsColor(color, true);
}

BackgroundTexturesPC BackgroundTexturesPS::toPC(const BackgroundTiles &psTiles,
												BackgroundTiles &pcTiles,
												const PalettesPC &palettesPC) const
{
	QMap<quint16, QList<BackgroundConversionTexture> > groupedTextures;
	QMap<qint32, Tile> sortedTiles = psTiles.sortedTiles();

	for (const Tile &tile : sortedTiles) {

		if((tile.textureID2 == 0 && _headerImg.w <= 0)
			 || (tile.textureID2 == 1 && _headerEffect.w <= 0)) {
			qWarning() << "BackgroundTexturesPS::toPC: unknown textureID2" << tile.textureID2
						  << _headerImg.w << _headerEffect.w;
			continue;
		}

		/* The key is used to group tiles by:
		 *  - blending (0 -> 1),
		 *  - size (16 -> 32),
		 *  - depth (1 -> 0 -> 2)
		 */

		quint8 depthKey = 0;
		switch (tile.depth & 3) {
		case 0:		depthKey = 1;	break;
		case 1:		depthKey = 0;	break;
		default:	depthKey = 2;   break;
		}

		quint16 key = quint16((tile.typeTrans << 4) |
		              (tile.blending << 3) |
		              (depthKey << 1) |
		              (tile.size == 32));

		QVector<uint> tileData = this->tile(tile); // Retrieve tile data

		if(tile.depth < 2) {
			PalettePC *palette = static_cast<PalettePC *>(palettesPC.value(tile.paletteID));
			if(palette && !palette->transparency()) {

				// Detection of transparency PC flag: true if one of used indexes is transparent
				const QList<bool> &areZero = palette->areZero();
				for (uint index : qAsConst(tileData)) {
					if(areZero.at(int(index))) {
						palette->setTransparency(true);
						break;
					}
				}
			}
		}

		groupedTextures[key].append(BackgroundConversionTexture(tileData, tile));
	}

	QMutableMapIterator<quint16, QList<BackgroundConversionTexture> > it(groupedTextures);
	// blending works only if textureID >= 15, and textureID >= 26 for depth = 2
	const quint8 startTexID = 0, startTexID2 = 15,
	        startTexID3 = 16, startTexID4 = 24,
	        startTexID5 = 26;
	quint8 curTexID = startTexID, curTexID2 = startTexID2,
	        curTexID3 = startTexID3, curTexID4 = startTexID4,
	        curTexID5 = startTexID5; 
	BackgroundTiles pcTiles2;
	BackgroundTexturesPC ret;

	while (it.hasNext()) {
		it.next();
		QList<BackgroundConversionTexture> &texture = it.value();
		BackgroundTexturesPCInfos info;
		quint8 texID;
		const Tile &representativeTile = texture.first().tile;

		info.depth = qMin(representativeTile.depth, quint8(1));
		info.size = representativeTile.size == 32; // 0 = 16, 1 = 32

		if(info.depth < 2) {

			/* On PC version, only the first palette color can be transparent
			 * So we need to change all indexes to a transparent color to 0
			 * And relocate when index=0 */
			for (int texConvId=0; texConvId < texture.size(); ++texConvId) {
				BackgroundConversionTexture &tileConversion = texture[texConvId];
				if (tileConversion.tile.depth >= 2) {
					continue; // No palette here
				}
				PalettePC *palette = static_cast<PalettePC *>(palettesPC.value(tileConversion.tile.paletteID));

				if(palette && palette->transparency()) {

					const QList<bool> &areZero = palette->areZero();
					bool firstIsZero = areZero.first();
					int indexOfFirstZero = areZero.indexOf(true, 1);
					if(firstIsZero || indexOfFirstZero >= 0) {
						int i = 0;
						for (const uint index : qAsConst(tileConversion.data)) {
							if(index > 0 && areZero.at(int(index))) {
								// When the index refer to a transparent color, change this index to 0

								tileConversion.data[i] = 0;
							} else if(!firstIsZero && index == 0) {
								// Reloc when index = 0 and not refer to a transparent color

								tileConversion.data[i] = uint(indexOfFirstZero);
							}
							++i;
						}
					} else {
						qWarning() << "BackgroundTexturesPS::toPC relloc color failed";
						break;
					}
				}
			}
		}

		// Textures fill

		const quint8 tileCount = quint8(256 / representativeTile.size);
		const quint16 tilesPerTexture = representativeTile.size == 16 ? 256 : 64;
		const quint8 textureCount = quint8(texture.size() / tilesPerTexture
				+ (texture.size() % tilesPerTexture != 0));
		QSet<quint8> usedTextures;
		usedTextures.reserve(BACKGROUND_TEXTURE_PC_MAX_COUNT);

		for (int i=0; i<textureCount; ++i) {

			if(representativeTile.depth == 2) {
				texID = curTexID5++;
			} else if(representativeTile.blending) {
				if(representativeTile.layerID == 3) {
					texID = curTexID3++;
				} else if(representativeTile.typeTrans == 0
				          || representativeTile.typeTrans == 2) {
					texID = curTexID4++;
				} else {
					texID = curTexID2++;
				}
			} else {
				texID = curTexID++;
			}
			if(usedTextures.contains(texID)) {
				for (quint8 j = 0; j < BACKGROUND_TEXTURE_PC_MAX_COUNT; ++j) {
					quint8 newTexID = (texID + j) % BACKGROUND_TEXTURE_PC_MAX_COUNT;
					if(!usedTextures.contains(newTexID)) {
						texID = newTexID;
						break;
					}
				}
				if(usedTextures.contains(texID)) {
					qWarning() << "BackgroundTexturesPS::toPC no more texture ID available!";
					break;
				}
			}
			usedTextures.insert(texID);

			QList<uint> flatten;

			for (quint8 tileY=0; tileY<tileCount; ++tileY) {
				for (quint8 y=0; y<representativeTile.size; ++y) {
					for (quint8 tileX=0; tileX<tileCount; ++tileX) {
						for (quint8 x=0; x<representativeTile.size; ++x) {
							flatten.append(texture.value(i * tilesPerTexture +
														 tileY * tileCount + tileX)
										   .data.value(y * representativeTile.size + x));
						}
					}
				}
			}

			ret.setTex(texID, flatten, info);

			// Update textureID and src in tiles
			for(quint16 tileID=0 ; tileID < tilesPerTexture
				&& i * tilesPerTexture + tileID < texture.size() ; ++tileID) {
				Tile tile = texture.at(i * tilesPerTexture + tileID).tile;

				tile.srcX = (tileID % tileCount) * tile.size;
				tile.srcY = quint8((tileID / tileCount) * tile.size);

				tile.textureID = texID;
				tile.textureID2 = tile.blending ? texID : 0;

				pcTiles2.insert(qint16(4096 - tile.ID), tile);
			}
		}
	}

	pcTiles = pcTiles2;

	return ret;
}
