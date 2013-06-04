#include "BackgroundTextures.h"

BackgroundTextures::BackgroundTextures()
{
}

BackgroundTextures::~BackgroundTextures()
{
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

void BackgroundTexturesPC::addTexInfos(quint8 texID, const BackgroundTexturesPCInfos &infos)
{
	_texInfos.insert(texID, infos);
}

void BackgroundTexturesPC::clear()
{
	_texInfos.clear();
}

void BackgroundTexturesPC::setTexInfos(const QHash<quint8, BackgroundTexturesPCInfos> &texInfos)
{
	_texInfos = texInfos;
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
