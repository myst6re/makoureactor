#include "BackgroundTextures.h"

QByteArray BackgroundTextures::_data;

BackgroundTextures::BackgroundTextures()
{
}

BackgroundTextures::~BackgroundTextures()
{
}

QVector<uint> BackgroundTextures::tile(const Tile &tile) const
{
	QVector<uint> indexOrRgbList;
	quint8 depth = this->depth(tile);
	quint8 multiplicator = depth == 0 ? 1 : depth * 2;
	quint32 texWidth = textureWidth(tile);
	quint32 origin = originInData(tile);
	quint32 lastByte = origin + tile.size * texWidth;
	quint8 x = 0;

	if(origin == 0) {
		return indexOrRgbList;
	}

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
			indexOrRgbList.append(data().at(infos.pos + i));
		} else if(infos.depth == 2) {
			indexOrRgbList.append(pixel(infos.pos + i));
			++i;
		}
	}

	return indexOrRgbList;
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
	return 0;
}

QRgb BackgroundTexturesPC::directColor(quint16 color) const
{
	return qRgb(COEFF_COLOR * (color >> 11),
				COEFF_COLOR * (color >> 6 & 31),
				COEFF_COLOR * (color & 31)); // special PC RGB16 color
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

quint16 BackgroundTexturesPS::textureWidth(const Tile &tile) const
{
	return pageTexWidth(tile.textureID2);
}

quint32 BackgroundTexturesPS::originInData(const Tile &tile) const
{
	quint16 texID = tile.textureID - pageTexPos(tile.textureID2);
	quint32 dataStart = pageDataPos(tile.textureID2);
	quint32 textureStart = texID * 128;
	quint32 tileStart = tile.srcY * textureWidth(tile) + tile.srcX * (tile.depth == 0 ? 0.5 : tile.depth);
	return dataStart + textureStart + tileStart;
}

QRgb BackgroundTexturesPS::directColor(quint16 color) const
{
	return PsColor::fromPsColor(color);
}

BackgroundTexturesPC BackgroundTexturesPS::toPC(const BackgroundTiles &tiles) const
{
	BackgroundTexturesPC ret;
	quint8 texID1 = 0, texID2 = 15;
	BackgroundTexturesPCInfos infos;
	QSet<quint16> visitedPositions1, visitedPositions2;

	foreach(const Tile &tile, tiles) {
		if(tile.textureID2 == 0
				&& !visitedPositions1.contains(tile.textureID)
				&& _headerImg.w > 0) {
			infos.depth = tile.depth == 0 ? 1 : tile.depth;
			infos.size = tile.size == 32; // 0 = 16, 1 = 32
			infos.pos = 0; // TODO

			if(tile.depth == 0) {
				//TODO: depth conversion
			}



			ret.addTexInfos(texID1++, infos);
			visitedPositions1.insert(tile.textureID);

		} else if(tile.textureID2 == 1 && !visitedPositions2.contains(tile.textureID)) {
			infos.depth = tile.depth;
			infos.size = tile.size == 32; // 0 = 16, 1 = 32
			infos.pos = 0; // TODO

			if(_headerEffect.w > 0) {

				ret.addTexInfos(texID2++, infos);
				visitedPositions2.insert(tile.textureID);
			}


		}
	}

	return ret;
}
