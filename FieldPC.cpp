/****************************************************************************
 ** Makou Reactor Final Fantasy VII FieldPC Script Editor
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
#include "FieldPC.h"

FieldPC::FieldPC() :
	Field(), position(0)
{
}

FieldPC::FieldPC(quint32 position, const QString &name) :
	Field(name), position(position)
{
}

FieldPC::FieldPC(const Field &field) :
	Field(field), position(0)
{
}

FieldPC::~FieldPC()
{
}

qint8 FieldPC::open(const QByteArray &fileData)
{
	quint32 size;
	const char *fileDataConst = fileData.constData();
	memcpy(&size, fileDataConst, 4);
	if(size+4 != (quint32)fileData.size()) 	return -1;

	QByteArray contenu = QByteArray(&fileDataConst[4], size);

	const QByteArray &contenuDec = LZS::decompress(contenu, 14);//décompression partielle
	if(contenuDec.size() < 14)	return -1;
	const char *contenu_const = contenuDec.constData();

	int debutSection2;
	memcpy(&debutSection2, &contenu_const[10], 4);

	return openSection1(LZS::decompress(contenu, debutSection2), 46);
}

QPixmap FieldPC::openModelAndBackground(const QByteArray &data)
{
	QHash<quint8, quint8> paramActifs;
	qint16 z[] = {-1, -1};
	getBgParamAndBgMove(paramActifs, z);

	FieldModelLoaderPC *fieldModelLoader = getFieldModelLoader();

	if(!fieldModelLoader->isLoaded())
		fieldModelLoader->load(data, this->name);

//	Data::currentCharNames = model_nameChar;
	Data::currentHrcNames = &fieldModelLoader->model_nameHRC;
	Data::currentAnimNames = &fieldModelLoader->model_anims;

	return ouvrirBackground(data, paramActifs, z);
}

bool FieldPC::getUsedParams(const QByteArray &contenu, QHash<quint8, quint8> &usedParams, bool *layerExists) const
{
	const char *constContenu = contenu.constData();
	quint32 debutSection9, i;

	if(contenu.isEmpty())	return false;

	memcpy(&debutSection9, &constContenu[38], 4);//Adresse Section 9

	if((quint32)contenu.size() <= debutSection9+153)	return false;

	quint32 aTex = debutSection9+48;
	quint16 nbTiles1, nbTiles2=0, nbTiles3=0, nbTiles4=0;
	bool exist2, exist3, exist4;

	memcpy(&nbTiles1, &constContenu[aTex], 2);//nbTiles1
	aTex += 8+nbTiles1*52;
	if((exist2 = (bool)contenu.at(aTex)))
	{
		memcpy(&nbTiles2, &constContenu[aTex+5], 2);//nbTiles2
		aTex += 26+nbTiles2*52;
	}
	aTex++;
	if((exist3 = (bool)contenu.at(aTex)))
	{
		memcpy(&nbTiles3, &constContenu[aTex+5], 2);//nbTiles3
		aTex += 20+nbTiles3*52;
	}
	aTex++;
	if((exist4 = (bool)contenu.at(aTex)))
	{
		memcpy(&nbTiles4, &constContenu[aTex+5], 2);//nbTiles4
		aTex += 20+nbTiles4*52;
	}

	layerExists[0] = exist2;
	layerExists[1] = exist3;
	layerExists[2] = exist4;

	Tile tile;

	//BG 0
	aTex = debutSection9+56 + nbTiles1*52 + 1;

	//BG 1
	if(exist2)
	{
		aTex += 26;
		for(i=0 ; i<nbTiles2 ; ++i)
		{
			memcpy(&tile, &constContenu[aTex+i*52], 34);
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
		for(i=0 ; i<nbTiles3 ; ++i)
		{
			memcpy(&tile, &constContenu[aTex+i*52], 34);
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
		for(i=0 ; i<nbTiles4 ; ++i)
		{
			memcpy(&tile, &constContenu[aTex+i*52], 34);
			if(tile.param && qAbs(tile.cibleX) < 1000 && qAbs(tile.cibleY) < 1000)
			{
				usedParams.insert(tile.param, usedParams.value(tile.param) | tile.state);
			}
		}
	}
	return true;
}

QPixmap FieldPC::ouvrirBackground(const QByteArray &contenu, const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers) const
{
	const char *constContenu = contenu.constData();
	QList<Palette> palettes;
	quint32 debutSection4, debutSection9, i, j;
	quint16 nb;

	memcpy(&debutSection4, &constContenu[18], 4);//Adresse Section 4
	memcpy(&debutSection9, &constContenu[38], 4);//Adresse Section 9

	if((quint32)contenu.size() <= debutSection9+153)	return QPixmap();

	memcpy(&nb, &constContenu[debutSection4+14], 2);//nbPalettes
	
	for(i=0 ; i<nb ; ++i)
		palettes.append(Palette(&constContenu[debutSection4+16+i*512], contenu.at(debutSection9+16+i)));

	quint32 aTex = debutSection9+48;
	quint16 nbTiles1, nbTiles2=0, nbTiles3=0, nbTiles4=0;
	bool exist2, exist3, exist4;
	
	memcpy(&nbTiles1, &constContenu[aTex], 2);//nbTiles1
	aTex += 8+nbTiles1*52;
	if((exist2 = (bool)contenu.at(aTex)))
	{
		memcpy(&nbTiles2, &constContenu[aTex+5], 2);//nbTiles2
		aTex += 26+nbTiles2*52;
	}
	aTex++;
	if((exist3 = (bool)contenu.at(aTex)))
	{
		memcpy(&nbTiles3, &constContenu[aTex+5], 2);//nbTiles3
		aTex += 20+nbTiles3*52;
	}
	aTex++;
	if((exist4 = (bool)contenu.at(aTex)))
	{
		memcpy(&nbTiles4, &constContenu[aTex+5], 2);//nbTiles4
		aTex += 20+nbTiles4*52;
	}
	
	aTex += 8;
	QHash<quint8, quint32> posTextures;
	
	//Textures
	for(i=0 ; i<42 ; ++i)
	{
		if((bool)contenu.at(aTex))
		{
			posTextures.insert(i, aTex+4);
			aTex += (quint8)contenu.at(aTex+4)*65536 + 4;
		}
		aTex += 2;
	}

	aTex = debutSection9+56;
	QMultiMap<qint16, Tile> tiles;
	Tile tile;
	quint16 largeurMax=0, largeurMin=0, hauteurMax=0, hauteurMin=0;
	
	//BG 0
	for(i=0 ; i<nbTiles1 ; ++i) {
		memcpy(&tile, &constContenu[aTex+i*52], 34);

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
			memcpy(&tile, &constContenu[aTex+i*52], 34);
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
			memcpy(&tile, &constContenu[aTex+i*52], 34);
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
			memcpy(&tile, &constContenu[aTex+i*52], 34);
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

		origin = pos + 2 + (tile.srcY * 256 + tile.srcX) * (quint8)contenu.at(pos);
		
		if((quint8)contenu.at(pos) == 2)
		{
			for(j=0 ; j<tile.size*512 ; j+=2)
			{
				memcpy(&deuxOctets, &constContenu[origin+j], 2);
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
				index = (quint8)contenu.at(origin+j);
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
	
	return QPixmap::fromImage(image);
}

FieldModelLoaderPC *FieldPC::getFieldModelLoader()
{
	if(modelLoader)	return (FieldModelLoaderPC *)modelLoader;
	return (FieldModelLoaderPC *)(modelLoader = new FieldModelLoaderPC());
}

quint32 FieldPC::getPosition() const
{
	return position;
}

void FieldPC::setPosition(quint32 position)
{
	if(this->position != 0)
		this->position = position;
}

QByteArray FieldPC::save(const QByteArray &fileData, bool compress)
{
	QByteArray decompresse = LZS::decompress(fileData), newData, section, toc;
	const char *decompresseData = decompresse.constData();
	quint32 debutSections[9], size, section_size;

	debutSections[0] = 42;
	for(quint8 i=1 ; i<9 ; ++i)
		memcpy(&debutSections[i], &decompresseData[6+4*i], 4);

	// Header + pos section 1
	toc.append(decompresseData, 10);

	// Section 1 (scripts + textes + akaos/tutos)
	section = saveSection1(decompresse.mid(debutSections[0]+4, debutSections[1]-debutSections[0]-4));
	section_size = section.size();

	newData.append((char *)&section_size, 4).append(section);

	size = 42 + newData.size();
	toc.append((char *)&size, 4);

	// Section 2 (camera)
	section = QByteArray();
	if(ca && ca->isModified() && ca->save(section)) {
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(&decompresseData[debutSections[1]], debutSections[2]-debutSections[1]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4);

	// Section 3 (model loader PC)
	section = getFieldModelLoader()->save(decompresse.mid(debutSections[2]+4, debutSections[3]-debutSections[2]-4), this->name);
	section_size = section.size();

	newData.append((char *)&section_size, 4).append(section);

	size = 42 + newData.size();
	toc.append((char *)&size, 4);
	
	// Section 4 (background palette PC)
	newData.append(&decompresseData[debutSections[3]], debutSections[4]-debutSections[3]);

	size = 42 + newData.size();
	toc.append((char *)&size, 4);

	// Section 5 (walkmesh)
	section = QByteArray();
	if(id && id->isModified() && id->save(section)) {
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(&decompresseData[debutSections[4]], debutSections[5]-debutSections[4]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4);

	// Section 6 (background tileMap -unused-)
	newData.append(&decompresseData[debutSections[5]], debutSections[6]-debutSections[5]);

	size = 42 + newData.size();
	toc.append((char *)&size, 4);

	// Section 7 (encounter)
	if(encounter && encounter->isModified()) {
		section = encounter->save();
		section_size = section.size();

		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(&decompresseData[debutSections[6]], debutSections[7]-debutSections[6]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4);

	// Section 8 (trigger)
	if(inf && inf->isModified()) {
		section = inf->save();
		section_size = section.size();

		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(&decompresseData[debutSections[7]], debutSections[8]-debutSections[7]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4);

	// Section 9 (background PC)
	newData.append(decompresse.mid(debutSections[8]));

	newData.prepend(toc);

//	if(decompresse != newData) {
//		QFile fic("test_"+name+"_nouveau");
//		fic.open(QIODevice::WriteOnly);
//		fic.write(newData);
//		fic.close();
//		QFile fic2("test_"+name+"_original");
//		fic2.open(QIODevice::WriteOnly);
//		fic2.write(decompresse);
//		fic2.close();
//		qDebug() << name << " : ERROR";
////		newData = decompresse;
//	}

	if(compress)
	{
		QByteArray compresse = LZS::compress(newData);
		quint32 taille = compresse.size();
		return QByteArray((char *)&taille, 4).append(compresse);
	}

	return newData;
}

qint8 FieldPC::importer(const QByteArray &data, bool isDat, FieldParts part)
{
	if(part.testFlag(HrcLoader)) {
		if(!isDat) {
			getFieldModelLoader()->load(data, this->name);
		}
	}

	return Field::importer(data, part);
}
