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
#include "BackgroundFilePC.h"
#include "Palette.h"
#include "../PsColor.h"
#include "FieldPC.h"

QHash<quint8, quint32> BackgroundFilePC::posTextures;
QByteArray BackgroundFilePC::data;

BackgroundFilePC::BackgroundFilePC(FieldPC *field) :
	BackgroundFile(field)
{
}

quint16 BackgroundFilePC::textureWidth(const Tile &tile) const
{
	return depth(tile) * 256;
}

quint8 BackgroundFilePC::depth(const Tile &tile) const
{
	quint32 pos = posTextures.value(tile.textureID);
	return data.at(pos);
}

quint32 BackgroundFilePC::originInData(const Tile &tile) const
{
	quint32 pos = posTextures.value(tile.textureID);
	return pos + 2 + (tile.srcY * 256 + tile.srcX) * (quint8)data.at(pos);
}

QRgb BackgroundFilePC::directColor(quint16 color) const
{
	return qRgb( (color>>11)*COEFF_COLOR, (color>>6 & 31)*COEFF_COLOR, (color & 31)*COEFF_COLOR ); // special PC RGB16 color
}

QList<Palette *> BackgroundFilePC::openPalettes(const QByteArray &data, const QByteArray &palData)
{
	const char *constDataPal = palData.constData();
	quint32 dataPalSize = palData.size(), i;
	QList<Palette *> palettes;
	quint16 nb;

	if(dataPalSize < 12) {
		return QList<Palette *>() << NULL;
	}

	memcpy(&nb, constDataPal + 10, 2);//nbPalettes

	if(dataPalSize < quint32(12+nb*512)) {
		return QList<Palette *>() << NULL;
	}

	for(i=0 ; i<nb ; ++i) {
		palettes.append(new PalettePC(constDataPal + 12+i*512, data.at(12+i)));
	}

	return palettes;
}

QPixmap BackgroundFilePC::openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
	data = field()->sectionData(Field::Background);
	const char *constData = data.constData();
	quint32 dataSize = data.size(), i;
	QList<Palette *> palettes = openPalettes(data, field()->sectionData(Field::PalettePC));
	quint32 aTex = 44;
	quint16 nbTiles1, nbTiles2=0, nbTiles3=0, nbTiles4=0;
	bool exist2, exist3, exist4;

	if(palettes.size() == 1 && palettes.first() == NULL) {
		return QPixmap();
	}

	if(dataSize < aTex+2)	return QPixmap();

	memcpy(&nbTiles1, constData + aTex, 2);//nbTiles1
	aTex += 8+nbTiles1*52;
	if(dataSize < aTex+1)	return QPixmap();
	if((exist2 = (bool)data.at(aTex)))
	{
		if(dataSize < aTex+7)	return QPixmap();
		memcpy(&nbTiles2, constData + aTex+5, 2);//nbTiles2
		aTex += 26+nbTiles2*52;
	}
	aTex++;
	if(dataSize < aTex+1)	return QPixmap();
	if((exist3 = (bool)data.at(aTex)))
	{
		if(dataSize < aTex+7)	return QPixmap();
		memcpy(&nbTiles3, constData + aTex+5, 2);//nbTiles3
		aTex += 20+nbTiles3*52;
	}
	aTex++;
	if(dataSize < aTex+1)	return QPixmap();
	if((exist4 = (bool)data.at(aTex)))
	{
		if(dataSize < aTex+7)	return QPixmap();
		memcpy(&nbTiles4, constData + aTex+5, 2);//nbTiles4
		aTex += 20+nbTiles4*52;
	}

	aTex += 8;
	posTextures.clear();

	//Textures
	for(i=0 ; i<42 ; ++i)
	{
		if(dataSize < aTex+2)	return QPixmap();
		if((bool)data.at(aTex))
		{
			posTextures.insert(i, aTex+4);
			aTex += (quint8)data.at(aTex+4)*65536 + 4;
			if(dataSize < aTex)	return QPixmap();
		}
		aTex += 2;
	}

	aTex = 52;
	TilePC tile;
	QMultiMap<qint16, Tile> tiles;

	//BG 0
	for(i=0 ; i<nbTiles1 ; ++i) {
		memcpy(&tile, constData + aTex+i*52, 34);

		if(posTextures.contains(tile.textureID) && (tile.paletteID==0 || tile.paletteID < palettes.size())
			&& qAbs(tile.dstX) < 1000 && qAbs(tile.dstY) < 1000) {
			tile.size = 16;

			if(layers==NULL || layers[0])
				tiles.insert(1, tilePC2Tile(tile));
		}
	}
	aTex += nbTiles1*52 + 1;

	//BG 1
	if(exist2) {
		aTex += 26;
		for(i=0 ; i<nbTiles2 ; ++i) {
			memcpy(&tile, constData + aTex+i*52, 34);
			if(tile.textureID2>0) {
				tile.srcX = tile.srcX2;
				tile.srcY = tile.srcY2;
				tile.textureID = tile.textureID2;
			}

			if(posTextures.contains(tile.textureID) && (tile.paletteID==0 || tile.paletteID < palettes.size())
				&& qAbs(tile.dstX) < 1000 && qAbs(tile.dstY) < 1000) {
				tile.size = 16;

				if((tile.state==0 || paramActifs.value(tile.param, 0)&tile.state) && (layers==NULL || layers[1]))
					tiles.insert(4096-tile.ID, tilePC2Tile(tile));
			}
		}
	}
	aTex += nbTiles2*52 + 1;

	//BG 2
	if(exist3) {
		aTex += 20;
		for(i=0 ; i<nbTiles3 ; ++i) {
			memcpy(&tile, constData + aTex+i*52, 34);
			if(tile.textureID2>0) {
				tile.srcX = tile.srcX2;
				tile.srcY = tile.srcY2;
				tile.textureID = tile.textureID2;
			}

			if(posTextures.contains(tile.textureID) && (tile.paletteID==0 || tile.paletteID < palettes.size())
					&& qAbs(tile.dstX) < 1000 && qAbs(tile.dstY) < 1000) {
				tile.size = 32;

				if((tile.state==0 || paramActifs.value(tile.param, 0)&tile.state) && (layers==NULL || layers[2]))
					tiles.insert(4096-(z[0]!=-1 ? z[0] : tile.ID), tilePC2Tile(tile));
			}
		}
	}
	aTex += nbTiles3*52 + 1;

	//BG 3
	if(exist4) {
		aTex += 20;
		for(i=0 ; i<nbTiles4 ; ++i) {
			memcpy(&tile, constData + aTex+i*52, 34);
			if(tile.textureID2>0) {
				tile.srcX = tile.srcX2;
				tile.srcY = tile.srcY2;
				tile.textureID = tile.textureID2;
			}

			if(posTextures.contains(tile.textureID) && (tile.paletteID==0 || tile.paletteID < palettes.size())
					&& qAbs(tile.dstX) < 1000 && qAbs(tile.dstY) < 1000) {
				tile.size = 32;

				if((tile.state==0 || paramActifs.value(tile.param, 0)&tile.state) && (layers==NULL || layers[3]))
					tiles.insert(4096-(z[1]!=-1 ? z[1] : tile.ID), tilePC2Tile(tile));
			}
		}
	}

	return drawBackground(tiles, palettes, data);
}

bool BackgroundFilePC::usedParams(QHash<quint8, quint8> &usedParams, bool *layerExists)
{
	data = field()->sectionData(Field::Background);
	if(data.isEmpty())	return false;

	const char *constData = data.constData();
	quint32 i, dataSize = data.size(), aTex = 44;
	quint16 nbTiles1, nbTiles2=0, nbTiles3=0, nbTiles4=0;
	bool exist2, exist3, exist4;

	if(dataSize < aTex + 2)	return false;

	memcpy(&nbTiles1, constData + aTex, 2);//nbTiles1
	aTex += 8+nbTiles1*52;
	if(dataSize < aTex + 1)	return false;
	if((exist2 = (bool)data.at(aTex)))
	{
		if(dataSize < aTex + 7)	return false;
		memcpy(&nbTiles2, constData + aTex+5, 2);//nbTiles2
		aTex += 26+nbTiles2*52;
	}
	aTex++;
	if(dataSize < aTex + 1)	return false;
	if((exist3 = (bool)data.at(aTex)))
	{
		if(dataSize < aTex + 7)	return false;
		memcpy(&nbTiles3, constData + aTex+5, 2);//nbTiles3
		aTex += 20+nbTiles3*52;
	}
	aTex++;
	if(dataSize < aTex + 1)	return false;
	if((exist4 = (bool)data.at(aTex)))
	{
		if(dataSize < aTex + 7)	return false;
		memcpy(&nbTiles4, constData + aTex+5, 2);//nbTiles4
		aTex += 20+nbTiles4*52;
	}

	layerExists[0] = exist2;
	layerExists[1] = exist3;
	layerExists[2] = exist4;

	TilePC tile;

	//BG 0
	aTex = 52 + nbTiles1*52 + 1;

	//BG 1
	if(exist2)
	{
		aTex += 26;
		if(dataSize < aTex + nbTiles2*52)	return false;
		for(i=0 ; i<nbTiles2 ; ++i)
		{
			memcpy(&tile, constData + aTex+i*52, 34);
			if(tile.param && qAbs(tile.dstX) < 1000 && qAbs(tile.dstY) < 1000)
			{
				usedParams.insert(tile.param, usedParams.value(tile.param) | tile.state);
			}
		}
	}
	aTex += nbTiles2*52 + 1;

	//BG 2
	if(exist3)
	{
		aTex += 20;
		if(dataSize < aTex + nbTiles3*52)	return false;
		for(i=0 ; i<nbTiles3 ; ++i)
		{
			memcpy(&tile, constData + aTex+i*52, 34);
			if(tile.param && qAbs(tile.dstX) < 1000 && qAbs(tile.dstY) < 1000)
			{
				usedParams.insert(tile.param, usedParams.value(tile.param) | tile.state);
			}
		}
	}
	aTex += nbTiles3*52 + 1;

	//BG 3
	if(exist4)
	{
		aTex += 20;
		if(dataSize < aTex + nbTiles4*52)	return false;
		for(i=0 ; i<nbTiles4 ; ++i)
		{
			memcpy(&tile, constData + aTex+i*52, 34);
			if(tile.param && qAbs(tile.dstX) < 1000 && qAbs(tile.dstY) < 1000)
			{
				usedParams.insert(tile.param, usedParams.value(tile.param) | tile.state);
			}
		}
	}
	return true;
}

Tile BackgroundFilePC::tilePC2Tile(const TilePC &tile)
{
	Tile ret;

	ret.dstX = tile.dstX;
	ret.dstY = tile.dstY;
	ret.srcX = tile.srcX;
	ret.srcY = tile.srcY;
	ret.paletteID = tile.paletteID;
	ret.ID = tile.ID;
	ret.param = tile.param;
	ret.state = tile.state;
	ret.blending = tile.blending;
	ret.typeTrans = tile.typeTrans;
	ret.size = tile.size;
	ret.textureID = tile.textureID;
	ret.textureID2 = tile.textureID2;
	ret.depth = tile.depth;

	return ret;
}
