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
#include "BackgroundFilePS.h"
#include "Palette.h"
#include "../PsColor.h"
#include "FieldPS.h"

MIM BackgroundFilePS::headerPal;
MIM BackgroundFilePS::headerImg;
MIM BackgroundFilePS::headerEffect;

BackgroundFilePS::BackgroundFilePS(FieldPS *field) :
	BackgroundFile(field)
{
}

quint16 BackgroundFilePS::textureWidth(const Tile &tile) const
{
	return tile.textureID2 ? headerEffect.w : headerImg.w;
}

quint8 BackgroundFilePS::depth(const Tile &tile) const
{
	return tile.depth;
}

quint32 BackgroundFilePS::originInData(const Tile &tile) const
{
	quint16 texID = tile.textureID - (tile.textureID2 ? headerEffect.x/64 : headerImg.x/64);
	return headerPal.size + 12 + (tile.textureID2 ? headerImg.size : 0) + tile.srcY*textureWidth(tile) + tile.srcX*tile.depth + texID*128;
}

QRgb BackgroundFilePS::directColor(quint16 color) const
{
	return PsColor::fromPsColor(color);
}

QList<Palette *> BackgroundFilePS::openPalettes(const QByteArray &data)
{
	const char *constMimData = data.constData();
	quint32 mimDataSize = data.size(), i;
	QList<Palette *> palettes;

	if(mimDataSize < 12) {
		return QList<Palette *>() << NULL;
	}

	memcpy(&headerPal, constMimData, 12);

	if(mimDataSize < quint32(12+headerPal.h*512)) {
		return QList<Palette *>() << NULL;
	}

	for(i=0 ; i<headerPal.h ; ++i) {
		palettes.append(new PalettePS(constMimData + 12 + i*512));
	}

	return palettes;
}

QPixmap BackgroundFilePS::openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
	/*--- OUVERTURE DU MIM ---*/
	QByteArray mimDataDec = ((FieldPS *)field())->io()->mimData(field());
	const char *constMimData = mimDataDec.constData();
	quint32 mimDataSize = mimDataDec.size(), i;

	QList<Palette *> palettes = openPalettes(mimDataDec);
	if(palettes.size() == 1 && palettes.first() == NULL) {
		return QPixmap();
	}

	if(mimDataSize < headerPal.size + 12)	return QPixmap();

	memcpy(&headerImg, constMimData + headerPal.size, 12);

	headerImg.w *= 2;

	if(headerPal.size+headerImg.size+12 < mimDataSize)
	{
		memcpy(&headerEffect, constMimData + headerPal.size+headerImg.size, 12);
		headerEffect.w *= 2;
	}
	else
	{
		headerEffect.size = 4;
		headerEffect.w = 0;
		headerEffect.x = 0;
	}

	/*--- OUVERTURE DU DAT ---*/
	QByteArray datDataDec = field()->sectionData(Field::Background);
	const char *constDatData = datDataDec.constData();
	quint32 datDataSize = datDataDec.size();
	quint32 debut1, debut2, debut3, debut4;

	if(datDataSize < 16)	return QPixmap();

	memcpy(&debut1, constDatData, 4);
	memcpy(&debut2, constDatData + 4, 4);
	memcpy(&debut3, constDatData + 8, 4);
	memcpy(&debut4, constDatData + 12, 4);

	quint16 tilePos=0, tileCount=0;
	QList<quint32> nbTilesTex, nbTilesLayer;
	quint8 layerID=0;
	qint16 type;
	i = 16;
	while(i<debut1)
	{
		if(datDataSize < i+2)	return QPixmap();

		memcpy(&type, constDatData + i, 2);

		if(type==0x7FFF)
		{
			nbTilesLayer.append(tilePos+tileCount);
			++layerID;
		}
		else
		{
			if(type==0x7FFE)
			{
				memcpy(&tilePos, constDatData + i-4, 2);
				memcpy(&tileCount, constDatData + i-2, 2);

				nbTilesTex.append(tilePos+tileCount);
			}
			else {
				if(datDataSize < i+6)	return QPixmap();

				memcpy(&tilePos, constDatData + i+2, 2);
				memcpy(&tileCount, constDatData + i+4, 2);
			}
			i+=4;
		}
		i+=2;
	}

	QList<layer2Tile> tiles2;
	layer1Tile tile1;
	layer2Tile tile2;
	layer3Tile tile4;
	paramTile tile3;
	QMultiMap<qint16, Tile> tiles;
	Tile tile;
	quint16 texID=0;
	quint32 size, tileID=0;

	size = (debut3-debut2)/2;

	if(datDataSize < debut2+size*2)	return QPixmap();

	for(i=0 ; i<size ; ++i) {
		memcpy(&tile2, constDatData + debut2+i*2, 2);
		tiles2.append(tile2);
	}
	if(tiles2.isEmpty()) {
		return QPixmap();
	}
	tile2 = tiles2.first();

	size = (debut2-debut1)/8;

	if(datDataSize < debut1+size*8)	return QPixmap();

	for(i=0 ; i<size ; ++i)
	{
		memcpy(&tile1, constDatData + debut1+i*8, 8);
		if(qAbs(tile1.dstX) < 1000 && qAbs(tile1.dstY) < 1000) {
			tile.dstX = tile1.dstX;
			tile.dstY = tile1.dstY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = tile1.palID;

			if(texID+1<tiles2.size() && texID+1<nbTilesTex.size() && tileID>=nbTilesTex.at(texID)) {
				++texID;
				tile2 = tiles2.at(texID);
			}

			tile.textureID = tile2.page_x;
			tile.textureID2 = tile2.page_y;
			tile.depth = tile2.depth;
			tile.typeTrans = tile2.typeTrans;

			tile.param = tile.state = tile.blending = 0;
			tile.ID = 4095;

			tile.size = 16;

			if(layers==NULL || layers[0])
				tiles.insert(4096-tile.ID, tile);
		}
		++tileID;
	}

	size = (debut4-debut3)/14;

	if(datDataSize < debut3+size*14)	return QPixmap();

	for(i=0 ; i<size ; ++i)
	{
		memcpy(&tile1, constDatData + debut3+i*14, 8);
		if(qAbs(tile1.dstX) < 1000 || qAbs(tile1.dstY) < 1000) {
			tile.dstX = tile1.dstX;
			tile.dstY = tile1.dstY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = tile1.palID;

			memcpy(&tile2, constDatData + debut3+i*14+8, 2);
			memcpy(&tile3, constDatData + debut3+i*14+10, 4);

			tile.param = tile3.param;
			tile.state = tile3.state;

			if(tile.state==0 || paramActifs.value(tile.param, 0)&tile.state) {
				tile.textureID = tile2.page_x;
				tile.textureID2 = tile2.page_y;
				tile.depth = tile2.depth;
				tile.typeTrans = tile2.typeTrans;

				tile.blending = tile3.blending;
				tile.ID = tile3.group;

				tile.size = 16;

				if(layers==NULL || layers[1])
					tiles.insert(4096-tile.ID, tile);
			}
		}
		++tileID;
	}

	layerID = 2;

	size = (datDataSize-debut4)/10;

	if(datDataSize < debut4+size*10)	return QPixmap();

	for(i=0 ; i<size ; ++i)
	{
		memcpy(&tile1, constDatData + debut4+i*10, 8);
		if(qAbs(tile1.dstX) < 1000 || qAbs(tile1.dstY) < 1000) {
			tile.dstX = tile1.dstX;
			tile.dstY = tile1.dstY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = tile1.palID;

			memcpy(&tile4, constDatData + debut4+i*10+8, 2);

			tile.param = tile4.param;
			tile.state = tile4.state;

			if(texID+1<tiles2.size() && texID+1<nbTilesTex.size() && tileID>=nbTilesTex.at(texID)) {
				++texID;
				tile2 = tiles2.at(texID);
			}

			if(layerID+1<nbTilesLayer.size() && tileID>=nbTilesLayer.at(layerID)) {
				++layerID;
			}

			if(tile.state==0 || paramActifs.value(tile.param, 0)&tile.state) {
				tile.blending = tile4.blending;

				tile.textureID = tile2.page_x;
				tile.textureID2 = tile2.page_y;
				tile.depth = tile2.depth;
				tile.typeTrans = tile2.typeTrans;

				tile.ID = layerID==2 ? 4096 : 0;
				tile.size = 32;

				if(layers==NULL || layerID>3 || layers[layerID])
					tiles.insert(4096-(z[layerID!=2]!=-1 ? z[layerID!=2] : tile.ID), tile);
			}
		}
		++tileID;
	}

	return drawBackground(tiles, palettes, mimDataDec);
}

bool BackgroundFilePS::usedParams(QHash<quint8, quint8> &usedParams, bool *layerExists)
{
	/*--- OUVERTURE DU DAT ---*/
	QByteArray datDataDec = field()->sectionData(Field::Background);
	const char *constDatData = datDataDec.constData();
	quint32 datDataSize = datDataDec.size();
	quint32 i, debut1, debut2, debut3, debut4;

	memcpy(&debut1, constDatData, 4);
	memcpy(&debut2, constDatData + 4, 4);
	memcpy(&debut3, constDatData + 8, 4);
	memcpy(&debut4, constDatData + 12, 4);

	quint16 tilePos=0, tileCount=0;
	QList<quint32> nbTilesLayer;
	quint8 layerID=0;
	qint16 type;
	i = 16;
	while(i<debut1)
	{
		memcpy(&type, constDatData + i, 2);

		if(type==0x7FFF)
		{
			nbTilesLayer.append(tilePos+tileCount);
		}
		else
		{
			if(type==0x7FFE)
			{
				memcpy(&tilePos, constDatData + i-4, 2);
				memcpy(&tileCount, constDatData + i-2, 2);
			}
			else {
				memcpy(&tilePos, constDatData + i+2, 2);
				memcpy(&tileCount, constDatData + i+4, 2);
			}
			i+=4;
		}
		i+=2;
	}

	layer1Tile tile1;
	layer3Tile tile4;
	paramTile tile3;
	quint32 size, tileID=0;

	size = (debut2-debut1)/8;
	tileID += size;

	size = (debut4-debut3)/14;
	layerExists[0] = size > 0;
	for(i=0 ; i<size ; ++i)
	{
		memcpy(&tile1, constDatData + debut3+i*14, 8);
		if(qAbs(tile1.dstX) < 1000 || qAbs(tile1.dstY) < 1000) {
			memcpy(&tile3, constDatData + debut3+i*14+10, 4);
			if(tile3.param)
			{
				usedParams.insert(tile3.param, usedParams.value(tile3.param) | tile3.state);
			}
		}
		++tileID;
	}

	layerID = 2;
	layerExists[1] = layerExists[2] = false;

	size = (datDataSize-debut4)/10;
	for(i=0 ; i<size ; ++i)
	{
		memcpy(&tile1, constDatData + debut4+i*10, 8);
		if(qAbs(tile1.dstX) < 1000 || qAbs(tile1.dstY) < 1000) {
			memcpy(&tile4, constDatData + debut4+i*10+8, 2);
			if(tile4.param)
			{
				usedParams.insert(tile4.param, usedParams.value(tile4.param) | tile4.state);
			}

			if(layerID+1<nbTilesLayer.size() && tileID>=nbTilesLayer.at(layerID)) {
				++layerID;
			}

			if(layerID-1 < 3)
				layerExists[layerID-1] = true;
		}
		++tileID;
	}

	return true;
}
