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

#include <QtCore>
#include <PsColor>
#include <TimFile>
#include "BackgroundTiles.h"
#include "Palette.h"

#define BACKGROUND_TEXTURE_PC_MAX_COUNT 42

struct BackgroundTexturesPCInfos
{
	quint32 pos;
	quint8 depth, size;
};

//Sizeof : 12
struct MIM {
	quint32 size;// = 12 + w*2*h
	quint16 x, y;
	quint16 w, h;
};

struct BackgroundConversionTexture
{
	BackgroundConversionTexture() : tile(Tile()) {}
	BackgroundConversionTexture(const QList<uint> &data, const Tile &tile) :
	    data(data), tile(tile) {}
	QList<uint> data;
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
	QList<uint> tile(const Tile &tile) const;
	bool setTile(const Tile &tile, const QList<uint> &indexOrColor);
	virtual inline quint8 depth(const Tile &tile) const {
		return tile.depth;
	}
	QImage toImage(const Tile &tile, const Palettes &palettes) const;
	virtual QImage toImage(const BackgroundTiles &tiles, const Palettes &palettes) const=0;
protected:
	virtual quint16 textureWidth(const Tile &tile) const=0;
	virtual int originInData(const Tile &tile) const=0;
	virtual QRgb directColor(quint16 color) const=0;
	virtual quint16 fromQRgb(QRgb color) const=0;
	QRgb pixel(quint32 pos) const;
	QByteArray &data() {
		return _data;
	}
private:
	QByteArray _data;
};

class BackgroundTexturesPC;
class BackgroundTexturesPS;

struct UnusedSpaceInTexturePC {
	quint8 texID;
	quint8 x, y;
	bool valid, needsToCreateTex;
};

class BackgroundTexturesPC : public BackgroundTextures
{
public:
	BackgroundTexturesPC();
	explicit BackgroundTexturesPC(const QMap<quint8, BackgroundTexturesPCInfos> &texInfos);
	bool hasTex(quint8 texID) const;
	quint32 texPos(quint8 texID) const;
	quint8 texDepth(quint8 texID) const;
	quint8 texTileSize(quint8 texID) const;
	BackgroundTexturesPCInfos texInfos(quint8 texID) const;
	void addTexInfos(quint8 texID, const BackgroundTexturesPCInfos &infos);
	void clear() override;
	void setTexInfos(const QMap<quint8, BackgroundTexturesPCInfos> &texInfos);
	QList<uint> tex(quint8 texID) const;
	void setTex(quint8 texID, const QList<uint> &indexOrRgbList, const BackgroundTexturesPCInfos &infos);
	UnusedSpaceInTexturePC findFirstUnusedSpaceInTextures(const BackgroundTiles &tiles, quint8 depth, quint8 size);
	QImage toImage(const BackgroundTiles &tiles, const Palettes &palettes) const override;
	QImage toImage(const BackgroundTiles &tiles, const Palettes &palettes, QList<quint8> &texIdKeys) const;
	QImage toImage(quint8 texID) const;
	QImage toImage(quint8 texID, const BackgroundTiles &tiles, const Palettes &palettes) const;
	BackgroundTexturesPS toPS(const BackgroundTiles &pcTiles,
	                          BackgroundTiles &psTiles,
	                          const PalettesPS &palettesPS) const;
protected:
	quint16 textureWidth(const Tile &tile) const override;
	quint8 depth(const Tile &tile) const override;
	int originInData(const Tile &tile) const override;
	QRgb directColor(quint16 color) const override;
	quint16 fromQRgb(QRgb color) const override;
private:
	QMap<quint8, BackgroundTexturesPCInfos> _texInfos;
};

class BackgroundTexturesPS : public BackgroundTextures
{
public:
	BackgroundTexturesPS();
	void setHeaderImg(const MIM &headerImg);
	void setHeaderEffect(const MIM &headerEffect);
	QList<uint> tex(quint8 x, quint8 y, quint8 depth) const;
	TimFile tim(quint8 pageID, quint8 depth) const;
	QImage toImage(const BackgroundTiles &tiles, const Palettes &palettes) const override;
	BackgroundTexturesPC toPC(const BackgroundTiles &psTiles,
	                          BackgroundTiles &pcTiles,
	                          const PalettesPC &palettesPC) const;
protected:
	quint16 textureWidth(const Tile &tile) const override;
	int originInData(const Tile &tile) const override;
	QRgb directColor(quint16 color) const override;
	quint16 fromQRgb(QRgb color) const override;
private:
	quint32 pageDataPos(quint8 pageID) const;
	quint16 pageTexPos(quint8 pageID) const;
	quint16 pageTexWidth(quint8 pageID) const;
	quint32 texturePos(quint8 x, quint8 y) const;
	MIM _headerImg, _headerEffect;
};
