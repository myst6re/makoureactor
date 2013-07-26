/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef BACKGROUNDTEXTURES_H
#define BACKGROUNDTEXTURES_H

#include <QtCore>
#include "../PsColor.h"
#include "../TimFile.h"
#include "BackgroundTiles.h"
#include "Palette.h"

struct BackgroundTexturesPCInfos
{
	quint32 pos;
	quint8 depth, size;
};

//Sizeof : 12
typedef struct {
	quint32 size;// = 12 + w*2*h
	quint16 x, y;
	quint16 w, h;
} MIM;

struct BackgroundConversionTexture
{
	BackgroundConversionTexture() {}
	BackgroundConversionTexture(const QVector<uint> &data, const Tile &tile) :
		data(data), tile(tile) {}
	QVector<uint> data;
	Tile tile;
};

class BackgroundTextures
{
public:
	BackgroundTextures();
	virtual ~BackgroundTextures();
	inline const QByteArray &data() const {
		return _data;
	}
	inline void setData(const QByteArray &data) {
		_data = data;
	}
	virtual void clear() {
		_data.clear();
	}
	QVector<uint> tile(const Tile &tile) const;
	virtual inline quint8 depth(const Tile &tile) const {
		return tile.depth;
	}
protected:
	virtual quint16 textureWidth(const Tile &tile) const=0;
	virtual quint32 originInData(const Tile &tile) const=0;
	virtual QRgb directColor(quint16 color) const=0;
	QRgb pixel(quint32 pos) const;
private:
	QByteArray _data;
};

class BackgroundTexturesPC;
class BackgroundTexturesPS;

class BackgroundTexturesPC : public BackgroundTextures
{
public:
	BackgroundTexturesPC();
	explicit BackgroundTexturesPC(const QHash<quint8, BackgroundTexturesPCInfos> &texInfos);
	bool hasTex(quint8 texID) const;
	quint32 texPos(quint8 texID) const;
	quint8 texDepth(quint8 texID) const;
	quint8 texTileSize(quint8 texID) const;
	BackgroundTexturesPCInfos texInfos(quint8 texID) const;
	void addTexInfos(quint8 texID, const BackgroundTexturesPCInfos &infos);
	void clear();
	void setTexInfos(const QHash<quint8, BackgroundTexturesPCInfos> &texInfos);
	QList<uint> tex(quint8 texID) const;
	void setTex(quint8 texID, const QList<uint> &indexOrRgbList, const BackgroundTexturesPCInfos &infos);
	QImage toImage(quint8 texID) const;
	QImage toImage(quint8 texID, const BackgroundTiles &tiles, const Palettes &palettes) const;
	BackgroundTexturesPS toPS(const BackgroundTiles &pcTiles,
							  BackgroundTiles &psTiles,
							  const PalettesPS &palettesPS) const;
protected:
	quint16 textureWidth(const Tile &tile) const;
	quint8 depth(const Tile &tile) const;
	quint32 originInData(const Tile &tile) const;
	QRgb directColor(quint16 color) const;
private:
	static quint16 toPcColor(const QRgb &color);
	QHash<quint8, BackgroundTexturesPCInfos> _texInfos;
};

class BackgroundTexturesPS : public BackgroundTextures
{
public:
	BackgroundTexturesPS();
	void setDataPos(quint32 dataPos);
	void setHeaderImg(const MIM &headerImg);
	void setHeaderEffect(const MIM &headerEffect);
	QList<uint> tex(quint8 x, quint8 y, quint8 depth) const;
	TimFile tim(quint8 pageID, quint8 depth) const;
	BackgroundTexturesPC toPC(const BackgroundTiles &psTiles,
							  BackgroundTiles &pcTiles,
							  const PalettesPC &palettesPC) const;
protected:
	quint16 textureWidth(const Tile &tile) const;
	quint32 originInData(const Tile &tile) const;
	QRgb directColor(quint16 color) const;
private:
	quint32 pageDataPos(quint8 pageID) const;
	quint16 pageTexPos(quint8 pageID) const;
	quint16 pageTexWidth(quint8 pageID) const;
	quint32 texturePos(quint8 x, quint8 y) const;
	quint32 _dataPos;
	MIM _headerImg, _headerEffect;
};

#endif // BACKGROUNDTEXTURES_H
