#include "BackgroundFilePC.h"
#include "Palette.h"

BackgroundFilePC::BackgroundFilePC() :
	BackgroundFile()
{
}

QPixmap BackgroundFilePC::openBackground(const QByteArray &data, const QByteArray &palData, const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
//	qDebug() << "FieldPC::openBackground";
	const char *constData = data.constData(), *constDataPal = palData.constData();
	quint32 dataSize = data.size(), dataPalSize = palData.size(), i, j;
	QList<Palette> palettes;
	quint16 nb;

	if(dataPalSize < 12)	return QPixmap();

	memcpy(&nb, constDataPal + 10, 2);//nbPalettes

	if(dataPalSize < quint32(12+nb*512))	return QPixmap();

	for(i=0 ; i<nb ; ++i)
		palettes.append(Palette(constDataPal + 12+i*512, data.at(12+i)));

	quint32 aTex = 44;
	quint16 nbTiles1, nbTiles2=0, nbTiles3=0, nbTiles4=0;
	bool exist2, exist3, exist4;

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
	QHash<quint8, quint32> posTextures;

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
	QMultiMap<qint16, Tile> tiles;
	Tile tile;
	quint16 largeurMax=0, largeurMin=0, hauteurMax=0, hauteurMin=0;

	//BG 0
	for(i=0 ; i<nbTiles1 ; ++i) {
		memcpy(&tile, constData + aTex+i*52, 34);

		if(posTextures.contains(tile.textureID) && (tile.paletteID==0 || tile.paletteID < palettes.size())
			&& qAbs(tile.cibleX) < 1000 && qAbs(tile.cibleY) < 1000) {
			tile.size = 16;

			if(tile.cibleX >= 0 && tile.cibleX > largeurMax)
				largeurMax = tile.cibleX;
			else if(tile.cibleX < 0 && -tile.cibleX > largeurMin)
				largeurMin = -tile.cibleX;
			if(tile.cibleY >= 0 && tile.cibleY > hauteurMax)
				hauteurMax = tile.cibleY;
			else if(tile.cibleY < 0 && -tile.cibleY > hauteurMin)
				hauteurMin = -tile.cibleY;

			if(layers==NULL || layers[0])
				tiles.insert(1, tile);
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
				&& qAbs(tile.cibleX) < 1000 && qAbs(tile.cibleY) < 1000) {
				tile.size = 16;

				if(tile.cibleX >= 0 && tile.cibleX > largeurMax)
					largeurMax = tile.cibleX;
				else if(tile.cibleX < 0 && -tile.cibleX > largeurMin)
					largeurMin = -tile.cibleX;
				if(tile.cibleY >= 0 && tile.cibleY > hauteurMax)
					hauteurMax = tile.cibleY;
				else if(tile.cibleY < 0 && -tile.cibleY > hauteurMin)
					hauteurMin = -tile.cibleY;

				if((tile.state==0 || paramActifs.value(tile.param, 0)&tile.state) && (layers==NULL || layers[1]))
					tiles.insert(4096-tile.ID, tile);
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
					&& qAbs(tile.cibleX) < 1000 && qAbs(tile.cibleY) < 1000) {
				tile.size = 32;

				if(tile.cibleX >= 0 && tile.cibleX+16 > largeurMax)
					largeurMax = tile.cibleX+16;
				else if(tile.cibleX < 0 && -tile.cibleX > largeurMin)
					largeurMin = -tile.cibleX;
				if(tile.cibleY >= 0 && tile.cibleY+16 > hauteurMax)
					hauteurMax = tile.cibleY+16;
				else if(tile.cibleY < 0 && -tile.cibleY > hauteurMin)
					hauteurMin = -tile.cibleY;

				if((tile.state==0 || paramActifs.value(tile.param, 0)&tile.state) && (layers==NULL || layers[2]))
					tiles.insert(4096-(z[0]!=-1 ? z[0] : tile.ID), tile);
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
					&& qAbs(tile.cibleX) < 1000 && qAbs(tile.cibleY) < 1000) {
				tile.size = 32;

				if(tile.cibleX >= 0 && tile.cibleX+16 > largeurMax)
					largeurMax = tile.cibleX+16;
				else if(tile.cibleX < 0 && -tile.cibleX > largeurMin)
					largeurMin = -tile.cibleX;
				if(tile.cibleY >= 0 && tile.cibleY+16 > hauteurMax)
					hauteurMax = tile.cibleY+16;
				else if(tile.cibleY < 0 && -tile.cibleY > hauteurMin)
					hauteurMin = -tile.cibleY;

				if((tile.state==0 || paramActifs.value(tile.param, 0)&tile.state) && (layers==NULL || layers[3]))
					tiles.insert(4096-(z[1]!=-1 ? z[1] : tile.ID), tile);
			}
		}
	}

	if(tiles.isEmpty())	return QPixmap();

	int width = largeurMin + largeurMax + 16;
	QImage image(width, hauteurMin + hauteurMax + 16, QImage::Format_ARGB32);
	image.fill(0xFF000000);

	quint32 pos, origin, top;
	quint16 deuxOctets, baseX;
	quint8 index, right;
	QRgb *pixels = (QRgb *)image.bits();

	foreach(const Tile &tile, tiles)
	{
		pos = posTextures.value(tile.textureID);
		right = 0;
		top = (hauteurMin + tile.cibleY) * width;
		baseX = largeurMin + tile.cibleX;

		origin = pos + 2 + (tile.srcY * 256 + tile.srcX) * (quint8)data.at(pos);

		if((quint8)data.at(pos) == 2)
		{
			for(j=0 ; j<tile.size*512 ; j+=2)
			{
				memcpy(&deuxOctets, constData + origin+j, 2);
				if(deuxOctets!=0)
					pixels[baseX + right + top] = qRgb( (deuxOctets>>11)*COEFF_COLOR, (deuxOctets>>6 & 31)*COEFF_COLOR, (deuxOctets & 31)*COEFF_COLOR ); // special PC RGB16 color

				if(++right==tile.size)
				{
					right = 0;
					j += 512-tile.size*2;
					top += width;
				}
			}
		}
		else
		{
			const Palette &palette = palettes.at(tile.paletteID);

			for(j=0 ; j<tile.size*256 ; ++j)
			{
				index = (quint8)data.at(origin+j);
				if(index!=0 || !palette.transparency) {
					if(tile.blending) {
						pixels[baseX + right + top] = blendColor(tile.typeTrans, pixels[baseX + right + top], palette.couleurs.at(index));
					}
					else {
						pixels[baseX + right + top] = palette.couleurs.at(index);
					}
				}

				if(++right==tile.size)
				{
					right = 0;
					j += 256-tile.size;
					top += width;
				}
			}
		}
	}

//	qDebug() << "/FieldPC::openBackground";

	return QPixmap::fromImage(image);
}

bool BackgroundFilePC::usedParams(const QByteArray &data, QHash<quint8, quint8> &usedParams, bool *layerExists)
{
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

	Tile tile;

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
			if(tile.param && qAbs(tile.cibleX) < 1000 && qAbs(tile.cibleY) < 1000)
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
			if(tile.param && qAbs(tile.cibleX) < 1000 && qAbs(tile.cibleY) < 1000)
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
			if(tile.param && qAbs(tile.cibleX) < 1000 && qAbs(tile.cibleY) < 1000)
			{
				usedParams.insert(tile.param, usedParams.value(tile.param) | tile.state);
			}
		}
	}
	return true;
}
