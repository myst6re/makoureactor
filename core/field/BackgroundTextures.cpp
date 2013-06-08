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
	quint32 texWidth, origin, maxByte, lastByte;

	origin = originInData(tile);

	if(origin == quint32(-1)) {
		return indexOrRgbList;
	}

	texWidth = textureWidth(tile);
	maxByte = data().size();
	if(depth == 2) {
		--maxByte;
	}
	lastByte = qMin(origin + tile.size * texWidth, maxByte);

	for(quint32 i=origin ; i<lastByte ; ++i) {
		if(depth == 0) {
			quint8 index = data().at(i);
			indexOrRgbList.append(index & 0xF);
			++x;
			indexOrRgbList.append(index >> 4);
		} else if(depth == 1) {
			indexOrRgbList.append(quint8(data().at(i)));
		} else if(depth == 2) {
			indexOrRgbList.append(pixel(i));
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

	for(int i=0 ; i<size ; ++i) {
		if(infos.depth == 0) {
			quint8 index = data().at(infos.pos + i);
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
	if(indexOrRgbList.size() != 256 * 256 || texID >= 42) {
		qWarning() << "BackgroundTexturesPC::setTex invalid arguments";
		return;
	}

	BackgroundTexturesPCInfos infos = _infos;
	QByteArray texData;

	foreach(uint indexOrRgb, indexOrRgbList) {
		if(infos.depth == 0 || infos.depth == 1) {
			texData.append(quint8(indexOrRgb));
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
		while(it.hasNext()) {
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

quint32 BackgroundTexturesPC::originInData(const Tile &tile) const
{
	if(hasTex(tile.textureID)) {
		return texPos(tile.textureID) + (tile.srcY * 256 + tile.srcX) * depth(tile);
	}
	return quint32(-1);
}

QRgb BackgroundTexturesPC::directColor(quint16 color) const
{
	return qRgb(qRound(COEFF_COLOR * (color >> 11)),
				qRound(COEFF_COLOR * (color >> 6 & 31)),
				qRound(COEFF_COLOR * (color & 31))); // special PC RGB16 color
}

quint16 BackgroundTexturesPC::toPcColor(const QRgb &color)
{
	// alpha ignored!
	return ((qRound(qRed(color) / COEFF_COLOR) & 31) << 11) |
				((qRound(qGreen(color) / COEFF_COLOR) & 31) << 6) |
				(qRound(qBlue(color) / COEFF_COLOR) & 31); // special PC RGB16 color
}

BackgroundTexturesPS BackgroundTexturesPC::toPS() const
{
	BackgroundTexturesPS ret;

	// TODO: toPS

	return ret;
}

BackgroundTexturesPS::BackgroundTexturesPS() :
	BackgroundTextures(), _dataPos(0),
	_headerImg(MIM()), _headerEffect(MIM())
{
}

quint32 BackgroundTexturesPS::pageDataPos(quint8 pageID) const
{
	return _dataPos + (pageID ? _headerImg.size : 0);
}

quint16 BackgroundTexturesPS::pageTexPos(quint8 pageID) const
{
	return (pageID ? _headerEffect.x : _headerImg.x) / 64;
}

quint16 BackgroundTexturesPS::pageTexWidth(quint8 pageID) const
{
	return pageID ? _headerEffect.w : _headerImg.w;
}

void BackgroundTexturesPS::setDataPos(quint32 dataPos)
{
	_dataPos = dataPos + 12;
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

	for(int texY=0 ; texY<256 ; ++texY) {
		for(int texX=0 ; texX<256 ; ++texX) {
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
	header[4] = depth < 2 ? (depth | 8) : 2;

	return TimFile(header + (pageID ?
					   data().left(pageDataPos(0) - 12) // Palettes
					   + data().mid(pageDataPos(1) - 12) // Page 2
					 : data().left(pageDataPos(1) - 12))); // Palettes + Page 1
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

quint32 BackgroundTexturesPS::originInData(const Tile &tile) const
{
	quint32 textureStart = texturePos(tile.textureID, tile.textureID2);
	quint32 tileStart = tile.srcY * textureWidth(tile) + tile.srcX * (tile.depth == 0 ? 0.5 : tile.depth);
	return textureStart + tileStart;
}

QRgb BackgroundTexturesPS::directColor(quint16 color) const
{
	return PsColor::fromPsColor(color);
}

BackgroundTexturesPC BackgroundTexturesPS::toPC(const BackgroundTiles &psTiles,
												BackgroundTiles &pcTiles,
												const QList< QList<quint8> > &relocateZeroTable) const
{
	BackgroundTexturesPC ret;
	QMap<quint16, quint8> texIdConversionTable;
	QMap<quint16, BackgroundTexturesPCInfos> infos;
	QMap<quint16, QList< QVector<uint> > > textures;

	foreach(const Tile &tile, psTiles) {

		if((tile.textureID2 == 0 && _headerImg.w <= 0)
			 || (tile.textureID2 == 1 && _headerEffect.w <= 0)
			 || tile.textureID2 > 1) {
			qWarning() << "BackgroundTexturesPS::toPC: unknown textureID2" << tile.textureID2
						  << _headerImg.w << _headerEffect.w;
			continue;
		}

		quint16 key = (tile.textureID << 2) |
				(tile.textureID2 << 10) |
				(tile.size == 32) |
				(tile.depth << 1);
		Tile pcTile = tile;

		QList< QVector<uint> > texture = textures.value(key);

		quint8 tileCount = 256 / pcTile.size;
		pcTile.srcX = (texture.size() % tileCount) * pcTile.size;
		pcTile.srcY = (texture.size() / tileCount) * pcTile.size;

		QVector<uint> tileData = this->tile(tile);
		 // On PC version, only the first palette color can be transparent
		if(tile.depth < 2) {
			QList<quint8> relocateZero = relocateZeroTable.value(tile.paletteID);
			if(!relocateZero.isEmpty()) {
				int i=0;
				foreach(uint index, tileData) {
					if(relocateZero.contains(index)) {
						tileData[i] = 0;
					}
					++i;
				}
			}
		}

		texture.append(tileData);
		textures.insert(key, texture);
		pcTiles.insert(4096 - pcTile.ID, pcTile);
	}

	QMapIterator<quint16, QList< QVector<uint> > > it(textures);
	quint8 curTexID = 0;

	while(it.hasNext()) {
		it.next();
		const QList< QVector<uint> > &texture = it.value();
		BackgroundTexturesPCInfos info;
		QList<uint> flatten;
		quint16 key = it.key();
		quint8 texID;

		if(!texIdConversionTable.contains(key)) {
			texID = curTexID++;
			if(texID >= 42) {
				qWarning() << "BackgroundTexturesPS::toPC: Too many textures!";
				break;
			}
			texIdConversionTable.insert(key, texID);
		} else {
			qWarning() << "BackgroundTexturesPS::toPC: texIdConversionTable broken";
			break;
		}

		info.depth = (key >> 1) & 1;
		info.size = key & 1; // 0 = 16, 1 = 32
		infos.insert(key, info);

		quint8 tileSize = info.size ? 32 : 16;
		quint8 tileCount = 256 / tileSize;

		for(quint8 tileY=0 ; tileY<tileCount ; ++tileY) {
			for(quint8 y=0 ; y<tileSize ; ++y) {
				for(quint8 tileX=0 ; tileX<tileCount ; ++tileX) {
					for(quint8 x=0 ; x<tileSize ; ++x) {
						flatten.append(texture
									   .value(tileY * tileCount + tileX)
									   .value(y * tileSize + x));
					}
				}
			}
		}

		ret.setTex(texID, flatten, info);
	}

	foreach(Tile tile, pcTiles) {
		quint16 key = (tile.textureID << 2) |
				(tile.textureID2 << 10) |
				(tile.size == 32) |
				(tile.depth << 1);

		quint8 texID = texIdConversionTable.value(key);

		tile.textureID = texID;
		tile.textureID2 = texID;

		pcTiles.insert(4096 - tile.ID, tile);
	}

	return ret;
}
