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
#include "Field.h"

Field::Field()
	: _isOpen(false), _isModified(false), name(QString()), position(0), encounter(0), tut(0), id(0), ca(0), inf(0), modelLoader(0)
{
}

Field::Field(const QString &name)
	: _isOpen(false), _isModified(false), name(name), position(0), encounter(0), tut(0), id(0), ca(0), inf(0), modelLoader(0)
{
}

Field::Field(quint32 position, const QString &name)
	: _isOpen(false), _isModified(false), name(name), position(position), encounter(0), tut(0), id(0), ca(0), inf(0), modelLoader(0)
{
}

Field::~Field()
{
	foreach(GrpScript *grpScript, grpScripts)	delete grpScript;
	foreach(FF7Text *texte, textes)	delete texte;
	if(encounter)		delete encounter;
	if(tut)				delete tut;
	if(id)				delete id;
	if(ca)				delete ca;
	if(inf)				delete inf;
	if(modelLoader)		delete modelLoader;
}

void Field::close()
{
	foreach(GrpScript *grpScript, grpScripts)	delete grpScript;
	foreach(FF7Text *texte, textes)	delete texte;
	grpScripts.clear();
	textes.clear();
	author.clear();

	_isOpen = false;
}

bool Field::isOpen() const
{
	return _isOpen;
}

bool Field::isModified() const
{
	return _isModified;
}

void Field::setModified(bool modified)
{
	_isModified = modified;
}

qint8 Field::open(const QByteArray &fileData)
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

qint8 Field::open2(const QByteArray &fileData)
{
	quint32 size;
	const char *fileDataConst = fileData.constData();
	memcpy(&size, fileDataConst, 4);
	if(size+4 != (quint32)fileData.size()) 	return -1;

	QByteArray contenu = QByteArray(&fileDataConst[4], size);

	const QByteArray &contenuDec = LZS::decompress(contenu, 8);//décompression partielle
	if(contenuDec.size() < 8) 	return -1;
	const char *contenu_const = contenuDec.constData();

	quint32 debutSection1, debutSection2;
	memcpy(&debutSection1, contenu_const, 4);
	memcpy(&debutSection2, &contenu_const[4], 4);
	debutSection2 = debutSection2 - debutSection1 + 28;

	return openSection1(LZS::decompress(contenu, debutSection2), 28);
}

qint8 Field::openSection1(const QByteArray &contenu, int posStart)
{
	quint16 posTextes;
	int contenuSize = contenu.size();
	const char *constContenu = contenu.constData();

	if(contenuSize < 32)	return -1;

	memcpy(&posTextes, &constContenu[posStart+4], 2);//posTextes (et fin des scripts)
	posTextes += posStart;
	if((quint32)contenuSize < posTextes || posTextes < 32)	return -1;

	/* ---------- SCRIPTS ---------- */

	quint32 posAKAO = 0;
	quint16 nbAKAO, posScripts, pos;
	quint8 j, k, grpVides=0, nbScripts = (quint8)contenu.at(posStart+2);

	GrpScript *grpScript;

	memcpy(&nbAKAO, &constContenu[posStart+6], 2);//nbAKAO
	posScripts = posStart+32+8*nbScripts+4*nbAKAO;

	if(posTextes < posScripts+64*nbScripts)	return -1;
	this->author = contenu.mid(posStart+16, 8);
	//this->nbObjets3D = (quint8)contenu.at(posStart+3);
	memcpy(&this->scale, &constContenu[posStart+8], 2);
	//QString name2 = contenu.mid(posStart+24, 8);

	quint16 positions[33];

	for(quint8 i=0 ; i<nbScripts ; ++i)
	{
		grpScript = new GrpScript(QString(contenu.mid(posStart+32+8*i,8)));
		if(grpVides > 1)
		{
			for(int j=0 ; j<32 ; ++j)	grpScript->addScript();
			grpScripts.append(grpScript);
			grpVides--;
			continue;
		}

		//Listage des positions de départ
		memcpy(positions, &constContenu[posScripts+64*i], 64);

		//Ajout de la position de fin
		if(i==nbScripts-1)	positions[32] = posTextes - posStart;
		else
		{
			memcpy(&pos, &constContenu[posScripts+64*i+64], 2);

			if(pos > positions[31])	positions[32] = pos;
			else
			{
				grpVides = 1;
				while(pos <= positions[31] && i+grpVides<nbScripts-1)
				{
					memcpy(&pos, &constContenu[posScripts+64*(i+grpVides)+64], 2);
					grpVides++;
				}
				if(i+grpVides==nbScripts)	positions[32] = posTextes - posStart;
				else	positions[32] = pos;
			}
		}

		k=0;
		for(j=0 ; j<32 ; ++j)
		{
			if(positions[j+1] > positions[j])
			{
				grpScript->addScript(contenu.mid(posStart + positions[j], positions[j+1]-positions[j]));
				for(int l=k ; l<j ; ++l)	grpScript->addScript();
				k=j+1;
			}
		}
		for(int l=k ; l<32 ; ++l)	grpScript->addScript();
		grpScripts.append(grpScript);
	}

	if(nbAKAO>0)
	{
		//INTERGRITY TEST
		/*QString out;
		bool pasok = false;
		for(int i=0 ; i<nbAKAO ; ++i) {
			memcpy(&posAKAO, &constContenu[posStart+32+8*nbScripts+i*4], 4);
			posAKAO += posStart;
			out.append(QString("%1 %2 %3 %4 (%5)\n").arg(i).arg(name).arg(posAKAO-posStart).arg(QString(contenu.mid(posAKAO, 4))).arg(QString(contenu.mid(posAKAO-4, 8).toHex())));
			if(contenu.mid(posAKAO, 4) != "AKAO" && contenu.at(posAKAO) != '\x12') {
				pasok = true;
			}
		}
		if(pasok) {
			qDebug() << out;
		}*/

		memcpy(&posAKAO, &constContenu[posStart+32+8*nbScripts], 4);//posAKAO
		posAKAO += posStart;
	}
	else
	{
		posAKAO = contenuSize;
	}

	/* ---------- TEXTES ---------- */

	if((posAKAO -= posTextes) > 4)//Si il y a des textes
	{
		quint16 posDeb, posFin, nbTextes;
		if(contenuSize < posTextes+2)	return -1;
		memcpy(&posDeb, &constContenu[posTextes+2], 2);
		nbTextes = posDeb/2 - 1;

		for(quint16 i=1 ; i<nbTextes ; ++i)
		{
			memcpy(&posFin, &constContenu[posTextes+2+i*2], 2);

			if(contenuSize < posTextes+posFin)	return -1;

			textes.append(new FF7Text(contenu.mid(posTextes+posDeb, posFin-posDeb)));
			posDeb = posFin;
		}
		if((quint32)contenuSize < posAKAO)	return -1;
		textes.append(new FF7Text(contenu.mid(posTextes+posDeb, posAKAO-posDeb)));
	}

	_isOpen = true;

	return 0;
}

int Field::getModelID(quint8 grpScriptID) const
{
	if(grpScripts.at(grpScriptID)->getTypeID()!=1)	return -1;

	int ID=0;

	for(int i=0 ; i<grpScriptID ; ++i)
	{
		if(grpScripts.at(i)->getTypeID()==1)
			++ID;
	}
	return ID;
}

QString Field::HRCName(int modelID)
{
	return getFieldModelLoader()->model_nameHRC.value(modelID);
}

QString Field::AName(int modelID, int numA)
{
	return getFieldModelLoader()->model_anims.value(modelID).value(numA);
}

void Field::getBgParamAndBgMove(QHash<quint8, quint8> &paramActifs, qint16 *z, qint16 *x, qint16 *y) const
{
	foreach(GrpScript *grpScript, grpScripts) {
		grpScript->getBgParams(paramActifs);
		if(z)	grpScript->getBgMove(z, x, y);
	}
}

QPixmap Field::openModelAndBackground(const QByteArray &contenu)
{
	QHash<quint8, quint8> paramActifs;
	qint16 z[] = {-1, -1};
	Field::getBgParamAndBgMove(paramActifs, z);

	getFieldModelLoader();

	if(!modelLoader->isLoaded())
		modelLoader->load(contenu, this->name);

//	Data::currentCharNames = model_nameChar;
	Data::currentHrcNames = &modelLoader->model_nameHRC;
	Data::currentAnimNames = &modelLoader->model_anims;

	return ouvrirBackgroundPC(contenu, paramActifs, z);
}

bool Field::getUsedParamsPC(const QByteArray &contenu, QHash<quint8, quint8> &usedParams, bool *layerExists) const
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

bool Field::getUsedParamsPS(const QByteArray &datDataDec, QHash<quint8, quint8> &usedParams, bool *layerExists) const
{
	/*--- OUVERTURE DU DAT ---*/
	const char *constDatData = datDataDec.constData();
	quint32 i, start, debutSection3, debutSection4, debut1, debut2, debut3, debut4, debut5;

	memcpy(&start, &constDatData[0], 4);
	memcpy(&debutSection3, &constDatData[8], 4);
	memcpy(&debutSection4, &constDatData[12], 4);

	start = debutSection3 - start + 28;
	memcpy(&debut1, &constDatData[start], 4);
	memcpy(&debut2, &constDatData[start+4], 4);
	memcpy(&debut3, &constDatData[start+8], 4);
	memcpy(&debut4, &constDatData[start+12], 4);
	debut5 = debutSection4 - debutSection3;

	quint16 tilePos=0, tileCount=0;
	QList<quint32> nbTilesLayer;
	quint8 layerID=0;
	qint16 type;
	i = 16;
	while(i<debut1)
	{
		memcpy(&type, &constDatData[start+i], 2);

		if(type==0x7FFF)
		{
			nbTilesLayer.append(tilePos+tileCount);
		}
		else
		{
			if(type==0x7FFE)
			{
				memcpy(&tilePos, &constDatData[start+i-4], 2);
				memcpy(&tileCount, &constDatData[start+i-2], 2);
			}
			else {
				memcpy(&tilePos, &constDatData[start+i+2], 2);
				memcpy(&tileCount, &constDatData[start+i+4], 2);
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
		memcpy(&tile1, &constDatData[start+debut3+i*14], 8);
		if(qAbs(tile1.cibleX) < 1000 || qAbs(tile1.cibleY) < 1000) {
			memcpy(&tile3, &constDatData[start+debut3+i*14+10], 4);
			if(tile3.param)
			{
				usedParams.insert(tile3.param, usedParams.value(tile3.param) | tile3.state);
			}
		}
		++tileID;
	}

	layerID = 2;
	layerExists[1] = layerExists[2] = false;

	size = (debut5-debut4)/10;
	for(i=0 ; i<size ; ++i)
	{
		memcpy(&tile1, &constDatData[start+debut4+i*10], 8);
		if(qAbs(tile1.cibleX) < 1000 || qAbs(tile1.cibleY) < 1000) {
			memcpy(&tile4, &constDatData[start+debut4+i*10+8], 2);
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

QPixmap Field::ouvrirBackgroundPC(const QByteArray &contenu, const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers) const
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

QPixmap Field::ouvrirBackgroundPS(const QByteArray &mimDataDec, const QByteArray &datDataDec) const
{
	if(mimDataDec.isEmpty() || datDataDec.isEmpty())	return QPixmap();
	QHash<quint8, quint8> paramActifs;
	qint16 z[] = {-1, -1};
	getBgParamAndBgMove(paramActifs, z);
	return ouvrirBackgroundPS(mimDataDec, datDataDec, paramActifs, z);
}

QPixmap Field::ouvrirBackgroundPS(const QByteArray &mimDataDec, const QByteArray &datDataDec, const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers) const
{
	if(mimDataDec.isEmpty() || datDataDec.isEmpty())	return QPixmap();
	
	/*--- OUVERTURE DU MIM ---*/
	const char *constMimData = mimDataDec.constData();
	int mimDataSize = mimDataDec.size();
	MIM headerPal, headerImg, headerEffect;
	quint32 i;
	
	memcpy(&headerPal, constMimData, 12);
	
	QList<Palette> palettes;
	for(i=0 ; i<headerPal.h ; ++i)
		palettes.append(Palette(mimDataDec.mid(12+i*512,512)));

	memcpy(&headerImg, &constMimData[headerPal.size], 12);
	
	headerImg.w *= 2;

	if((int)(headerPal.size+headerImg.size+12) < mimDataSize)
	{
		memcpy(&headerEffect, &constMimData[headerPal.size+headerImg.size], 12);
		headerEffect.w *= 2;
	}
	else
	{
		headerEffect.size = 4;
		headerEffect.w = 0;
		headerEffect.x = 0;
	}
	
	/*--- OUVERTURE DU DAT ---*/
	const char *constDatData = datDataDec.constData();
	quint32 start, debutSection3, debutSection4, debut1, debut2, debut3, debut4, debut5;

	memcpy(&start, &constDatData[0], 4);
	memcpy(&debutSection3, &constDatData[8], 4);
	memcpy(&debutSection4, &constDatData[12], 4);

	/* // MODEL SECTION
	quint32 debutSection7, start2 = debutSection7 - start + 28;
	quint16 size7, nb_model7;

	memcpy(&debutSection7, &constDatData[24], 4);
	memcpy(&size7, &constDatData[start2], 2);
	memcpy(&nb_model7, &constDatData[start2+2], 2);
	qDebug() << QString("Size = %1 | nbModels = %2").arg(size7).arg(nb_model7);
	for(i=0 ; i<nb_model7 ; ++i) {
		qDebug() << QString("??? = %1 | ??? = %2 | ??? = %3 | nbAnims = %4").arg((quint8)datData.at(start2+4+i*8)).arg((quint8)datData.at(start2+5+i*8)).arg((quint8)datData.at(start2+6+i*8)).arg((quint8)datData.at(start2+7+i*8));
		qDebug() << QString("??? = %1 | ??? = %2 | ??? = %3 | ??? = %4").arg((quint8)datData.at(start2+8+i*8)).arg((quint8)datData.at(start2+9+i*8)).arg((quint8)datData.at(start2+10+i*8)).arg((quint8)datData.at(start2+11+i*8));
	}
	fdebug.write(datData.mid(start2));
	fdebug.close(); */
	
	start = debutSection3 - start + 28;
	memcpy(&debut1, &constDatData[start], 4);
	memcpy(&debut2, &constDatData[start+4], 4);
	memcpy(&debut3, &constDatData[start+8], 4);
	memcpy(&debut4, &constDatData[start+12], 4);
	debut5 = debutSection4 - debutSection3;
	
	quint16 tilePos=0, tileCount=0;
	QList<quint32> nbTilesTex, nbTilesLayer;
	quint8 layerID=0;
	qint16 type;
	i = 16;
	while(i<debut1)
	{
		memcpy(&type, &constDatData[start+i], 2);
		
		if(type==0x7FFF)
		{
			nbTilesLayer.append(tilePos+tileCount);
			++layerID;
		}
		else
		{
			if(type==0x7FFE)
			{
				memcpy(&tilePos, &constDatData[start+i-4], 2);
				memcpy(&tileCount, &constDatData[start+i-2], 2);
				
				nbTilesTex.append(tilePos+tileCount);
			}
			else {
				memcpy(&tilePos, &constDatData[start+i+2], 2);
				memcpy(&tileCount, &constDatData[start+i+4], 2);
			}
			i+=4;
		}
		i+=2;
	}
	
	QList<layer2Tile> tiles2;
	QMultiMap<qint16, Tile> tiles;
	layer1Tile tile1;
	layer2Tile tile2;
	layer3Tile tile4;
	paramTile tile3;
	Tile tile;
	quint16 texID=0, largeurMax=0, largeurMin=0, hauteurMax=0, hauteurMin=0;
	quint32 size, tileID=0;

	size = (debut3-debut2)/2;
	for(i=0 ; i<size ; ++i) {
		memcpy(&tile2, &constDatData[start+debut2+i*2], 2);
		tiles2.append(tile2);
	}
	if(tiles2.isEmpty()) {
		return QPixmap();
	}
	tile2 = tiles2.first();

	size = (debut2-debut1)/8;
	for(i=0 ; i<size ; ++i)
	{
		memcpy(&tile1, &constDatData[start+debut1+i*8], 8);
		if(qAbs(tile1.cibleX) < 1000 && qAbs(tile1.cibleY) < 1000) {
			if(tile1.cibleX >= 0 && tile1.cibleX > largeurMax)
				largeurMax = tile1.cibleX;
			else if(tile1.cibleX < 0 && -tile1.cibleX > largeurMin)
				largeurMin = -tile1.cibleX;
			if(tile1.cibleY >= 0 && tile1.cibleY > hauteurMax)
				hauteurMax = tile1.cibleY;
			else if(tile1.cibleY < 0 && -tile1.cibleY > hauteurMin)
				hauteurMin = -tile1.cibleY;

			tile.cibleX = tile1.cibleX;
			tile.cibleY = tile1.cibleY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = tile1.palID;

			if(texID+1<tiles2.size() && texID+1<nbTilesTex.size() && tileID>=nbTilesTex.at(texID)) {
				++texID;
				tile2 = tiles2.at(texID);
			}

			tile.textureID = tile2.page_x;
			tile.textureID2 = tile2.page_y;
			tile.deph = tile2.deph;
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
	for(i=0 ; i<size ; ++i)
	{
		memcpy(&tile1, &constDatData[start+debut3+i*14], 8);
		if(qAbs(tile1.cibleX) < 1000 || qAbs(tile1.cibleY) < 1000) {
			if(tile1.cibleX >= 0 && tile1.cibleX > largeurMax)
				largeurMax = tile1.cibleX;
			else if(tile1.cibleX < 0 && -tile1.cibleX > largeurMin)
				largeurMin = -tile1.cibleX;
			if(tile1.cibleY >= 0 && tile1.cibleY > hauteurMax)
				hauteurMax = tile1.cibleY;
			else if(tile1.cibleY < 0 && -tile1.cibleY > hauteurMin)
				hauteurMin = -tile1.cibleY;

			tile.cibleX = tile1.cibleX;
			tile.cibleY = tile1.cibleY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = tile1.palID;

			memcpy(&tile2, &constDatData[start+debut3+i*14+8], 2);
			memcpy(&tile3, &constDatData[start+debut3+i*14+10], 4);

			tile.param = tile3.param;
			tile.state = tile3.state;

			if(tile.state==0 || paramActifs.value(tile.param, 0)&tile.state) {
				tile.textureID = tile2.page_x;
				tile.textureID2 = tile2.page_y;
				tile.deph = tile2.deph;
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

	size = (debut5-debut4)/10;
	for(i=0 ; i<size ; ++i)
	{
		memcpy(&tile1, &constDatData[start+debut4+i*10], 8);
		if(qAbs(tile1.cibleX) < 1000 || qAbs(tile1.cibleY) < 1000) {
			if(tile1.cibleX >= 0 && tile1.cibleX+16 > largeurMax)
				largeurMax = tile1.cibleX+16;
			else if(tile1.cibleX < 0 && -tile1.cibleX > largeurMin)
				largeurMin = -tile1.cibleX;
			if(tile1.cibleY >= 0 && tile1.cibleY+16 > hauteurMax)
				hauteurMax = tile1.cibleY+16;
			else if(tile1.cibleY < 0 && -tile1.cibleY > hauteurMin)
				hauteurMin = -tile1.cibleY;

			tile.cibleX = tile1.cibleX;
			tile.cibleY = tile1.cibleY;
			tile.srcX = tile1.srcX;
			tile.srcY = tile1.srcY;
			tile.paletteID = tile1.palID;

			memcpy(&tile4, &constDatData[start+debut4+i*10+8], 2);

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
				tile.deph = tile2.deph;
				tile.typeTrans = tile2.typeTrans;

				tile.ID = layerID==2 ? 4096 : 0;
				tile.size = 32;

				if(layers==NULL || layerID>3 || layers[layerID])
					tiles.insert(4096-(z[layerID!=2]!=-1 ? z[layerID!=2] : tile.ID), tile);
			}
		}
		++tileID;
	}
	
	int imageWidth = largeurMin + largeurMax + 16;
	QImage image(imageWidth, hauteurMin + hauteurMax + 16, QImage::Format_ARGB32);
	QRgb *pixels = (QRgb *)image.bits();
	quint32 origin, top;
	quint16 width, deuxOctets, baseX;
	quint8 index, right;

	image.fill(0xFF000000);

	foreach(const Tile &tile, tiles)
	{
		width = tile.textureID2 ? headerEffect.w : headerImg.w;
		texID = tile.textureID - (tile.textureID2 ? headerEffect.x/64 : headerImg.x/64);
		origin = headerPal.size + 12 + (tile.textureID2 ? headerImg.size : 0) + tile.srcY*width + tile.srcX*tile.deph + texID*128;
		right = 0;
		top = (hauteurMin + tile.cibleY) * imageWidth;
		baseX = largeurMin + tile.cibleX;

		if(tile.deph == 2)
		{
			for(quint16 j=0 ; j<width*tile.size ; j+=2)
			{
				memcpy(&deuxOctets, &constMimData[origin+j], 2);
				if(deuxOctets!=0)
					pixels[baseX + right + top] = PsColor::fromPsColor(deuxOctets);

				if(++right==tile.size)
				{
					right = 0;
					j += width-tile.size*2;
					top += imageWidth;
				}
			}
		}
		else if(tile.deph == 1)
		{
			const Palette &palette = palettes.at(tile.paletteID);

			for(quint16 j=0 ; j<width*tile.size ; ++j)
			{
				index = (quint8)mimDataDec.at(origin+j);

				if(!palette.isZero.at(index)) {
					if(tile.blending) {
						pixels[baseX + right + top] = blendColor(tile.typeTrans, pixels[baseX + right + top], palette.couleurs.at(index));
					} else {
						pixels[baseX + right + top] = palette.couleurs.at(index);
					}
				}

				if(++right==tile.size)
				{
					right = 0;
					j += width-tile.size;
					top += imageWidth;
				}
			}
		}
	}

	return QPixmap::fromImage(image);
}

QRgb Field::blendColor(quint8 type, QRgb color0, QRgb color1)
{
	int r, g, b;

	switch(type) {
	case 1:
		r = qRed(color0) + qRed(color1);
		if(r>255)	r = 255;
		g = qGreen(color0) + qGreen(color1);
		if(g>255)	g = 255;
		b = qBlue(color0) + qBlue(color1);
		if(b>255)	b = 255;
		break;
	case 2:
		r = qRed(color0) - qRed(color1);
		if(r<0)	r = 0;
		g = qGreen(color0) - qGreen(color1);
		if(g<0)	g = 0;
		b = qBlue(color0) - qBlue(color1);
		if(b<0)	b = 0;
		break;
	case 3:
		r = qRed(color0) + 0.25*qRed(color1);
		if(r>255)	r = 255;
		g = qGreen(color0) + 0.25*qGreen(color1);
		if(g>255)	g = 255;
		b = qBlue(color0) + 0.25*qBlue(color1);
		if(b>255)	b = 255;
		break;
	default://0
		r = (qRed(color0) + qRed(color1))/2;
		g = (qGreen(color0) + qGreen(color1))/2;
		b = (qBlue(color0) + qBlue(color1))/2;
		break;
	}

	return qRgb(r, g, b);
}

void Field::insertGrpScript(int row)
{
	grpScripts.insert(row, new GrpScript);
	_isModified = true;
}

void Field::insertGrpScript(int row, GrpScript *grpScript)
{
	GrpScript *newGrpScript = new GrpScript(grpScript->getName());
	for(int i=0 ; i<grpScript->size() ; i++)
		newGrpScript->addScript(grpScript->getScript(i)->toByteArray(), false);
	grpScripts.insert(row, newGrpScript);
	_isModified = true;
}

void Field::deleteGrpScript(int row)
{
	if(row < grpScripts.size()) {
		delete grpScripts.takeAt(row);
		_isModified = true;
	}
}

void Field::removeGrpScript(int row)
{
	if(row < grpScripts.size()) {
		grpScripts.removeAt(row);
		_isModified = true;
	}
}

bool Field::moveGrpScript(int row, bool direction)
{
	if(row >= grpScripts.size())	return false;
	
	if(direction)
	{
		if(row == grpScripts.size()-1)	return false;
		grpScripts.swap(row, row+1);
		_isModified = true;
	}
	else
	{
		if(row == 0)	return false;
		grpScripts.swap(row, row-1);
		_isModified = true;
	}
	return true;
}

QList<FF7Var> Field::searchAllVars() const
{
	QList<FF7Var> vars;

	foreach(GrpScript *group, grpScripts)
		vars.append(group->searchAllVars());

	return vars;
}

bool Field::rechercherOpcode(int opcode, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0)	groupID = 0;

	int nbGroups = grpScripts.size();

	while(groupID < nbGroups)
	{
		if(grpScripts.at(groupID)->rechercherOpcode(opcode, scriptID, opcodeID))	return true;
		++groupID;
		scriptID = opcodeID = 0;
	}
	return false;
}

bool Field::rechercherVar(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0)	groupID = 0;

	int nbGroups = grpScripts.size();

	while(groupID < nbGroups)
	{
		if(grpScripts.at(groupID)->rechercherVar(bank, adress, value, scriptID, opcodeID))	return true;
		++groupID;
		scriptID = opcodeID = 0;
	}
	return false;
}

bool Field::rechercherExec(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0)	groupID = 0;

	int nbGroups = grpScripts.size();

	while(groupID < nbGroups)
	{
		if(grpScripts.at(groupID)->rechercherExec(group, script, scriptID, opcodeID))	return true;
		++groupID;
		scriptID = opcodeID = 0;
	}
	return false;
}

bool Field::rechercherTexte(const QRegExp &texte, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0)	groupID = 0;

	Data::currentTextes = &textes;

	int nbGroups = grpScripts.size();

	while(groupID < nbGroups)
	{
		if(grpScripts.at(groupID)->rechercherTexte(texte, scriptID, opcodeID))	return true;
		++groupID;
		scriptID = opcodeID = 0;
	}
	return false;
}

bool Field::rechercherOpcodeP(int opcode, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= grpScripts.size())	groupID = grpScripts.size()-1;

	while(groupID >= 0)
	{
		if(grpScripts.at(groupID)->rechercherOpcodeP(opcode, scriptID, opcodeID))	return true;
		--groupID;
		if(groupID >= 0) {
			scriptID = grpScripts.at(groupID)->size()-1;
			if(scriptID >= 0) {
				opcodeID = grpScripts.at(groupID)->getScript(scriptID)->size()-1;
			}
		}
	}
	return false;
}

bool Field::rechercherVarP(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= grpScripts.size())	groupID = grpScripts.size()-1;

	while(groupID >= 0)
	{
		if(grpScripts.at(groupID)->rechercherVarP(bank, adress, value, scriptID, opcodeID))	return true;
		--groupID;
		if(groupID >= 0) {
			scriptID = grpScripts.at(groupID)->size()-1;
			if(scriptID >= 0) {
				opcodeID = grpScripts.at(groupID)->getScript(scriptID)->size()-1;
			}
		}
	}
	return false;
}

bool Field::rechercherExecP(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= grpScripts.size())	groupID = grpScripts.size()-1;

	while(groupID >= 0)
	{
		if(grpScripts.at(groupID)->rechercherExecP(group, script, scriptID, opcodeID))	return true;
		--groupID;
		if(groupID >= 0) {
			scriptID = grpScripts.at(groupID)->size()-1;
			if(scriptID >= 0) {
				opcodeID = grpScripts.at(groupID)->getScript(scriptID)->size()-1;
			}
		}
	}
	return false;
}

bool Field::rechercherTexteP(const QRegExp &texte, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= grpScripts.size())	groupID = grpScripts.size()-1;

	Data::currentTextes = &textes;

	while(groupID >= 0)
	{
		if(grpScripts.at(groupID)->rechercherTexteP(texte, scriptID, opcodeID))	return true;
		--groupID;
		if(groupID >= 0) {
			scriptID = grpScripts.at(groupID)->size()-1;
			if(scriptID >= 0) {
				opcodeID = grpScripts.at(groupID)->getScript(scriptID)->size()-1;
			}
		}
	}
	return false;
}

void Field::insertText(int row)
{
	textes.insert(row, new FF7Text);
	foreach(GrpScript *grpScript, grpScripts)
		grpScript->shiftTextIds(row-1, +1);
	_isModified = true;
}

void Field::deleteText(int row)
{
	if(row < textes.size()) {
		delete textes.takeAt(row);
		foreach(GrpScript *grpScript, grpScripts)
			grpScript->shiftTextIds(row, -1);
		_isModified = true;
	}
}

QSet<quint8> Field::listUsedTexts() const
{
	QSet<quint8> usedTexts;
	foreach(GrpScript *grpScript, grpScripts)
		grpScript->listUsedTexts(usedTexts);
	return usedTexts;
}

void Field::shiftTutIds(int row, int shift)
{
	foreach(GrpScript *grpScript, grpScripts)
		grpScript->shiftTutIds(row, shift);
}

QSet<quint8> Field::listUsedTuts() const
{
	QSet<quint8> usedTuts;
	foreach(GrpScript *grpScript, grpScripts)
		grpScript->listUsedTuts(usedTuts);
	return usedTuts;
}

EncounterFile *Field::getEncounter()
{
	if(encounter)	return encounter;
	return encounter = new EncounterFile();
}

TutFile *Field::getTut()
{
	if(tut)		return tut;
	return tut = new TutFile();
}

IdFile *Field::getId()
{
	if(id)	return id;
	return id = new IdFile();
}

CaFile *Field::getCa()
{
	if(ca)	return ca;
	return ca = new CaFile();
}

InfFile *Field::getInf()
{
	if(inf)	return inf;
	return inf = new InfFile();
}

FieldModelLoader *Field::getFieldModelLoader()
{
	if(modelLoader)	return modelLoader;
	return modelLoader = new FieldModelLoader();
}

const QString &Field::getName() const
{
	return name;
}

void Field::setName(const QString &name)
{
	this->name = name;
	_isModified = true;
}

quint32 Field::getPosition() const
{
	return position;
}

void Field::setPosition(quint32 position)
{
	if(this->position != 0)
		this->position = position;
}

const QString &Field::getAuthor() const
{
	return author;
}

void Field::setAuthor(const QString &author)
{
	this->author = author;
	_isModified = true;
}

quint16 Field::getScale() const
{
	return scale;
}

void Field::setScale(quint16 scale)
{
	this->scale = scale;
	_isModified = true;
}

QList<FF7Text *> *Field::getTexts()
{
	return &textes;
}

int Field::getNbTexts() const
{
	return textes.size();
}

FF7Text *Field::getText(int textID) const
{
	return textes.at(textID);
}

void Field::setSaved()
{
	if(encounter)	encounter->setModified(false);
	if(tut)			tut->setModified(false);
	if(id)			id->setModified(false);
	if(ca)			ca->setModified(false);
	if(inf)			inf->setModified(false);
}

QByteArray Field::save(const QByteArray &fileData, bool compress)
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

QByteArray Field::saveDat(const QByteArray &fileData, bool compress)
{	
	QByteArray decompresse = LZS::decompress(fileData), toc, newData;
	const char *decompresseData = decompresse.constData();
	quint32 padd, pos, debutSections[9];

	for(quint8 i=0 ; i<7 ; ++i)
		memcpy(&debutSections[i], &decompresseData[4*i], 4);

	padd = debutSections[0] - 28;

	toc.append((char *)&debutSections[0], 4);

	// Section 1 (scripts + textes + akaos/tutos)
	newData.append(saveSection1(decompresse.mid(28, debutSections[1]-debutSections[0])));
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 2 (walkmesh)
	QByteArray section;
	if(id && id->isModified() && id->save(section)) {
		newData.append(section);
	} else {
		newData.append(decompresse.mid(debutSections[1]-padd, debutSections[2]-debutSections[1]));
	}
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 3 (background tileMap)
	newData.append(decompresse.mid(debutSections[2]-padd, debutSections[3]-debutSections[2]));
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 4 (camera)
	section = QByteArray();
	if(ca && ca->isModified() && ca->save(section)) {
		newData.append(section);
	} else {
		newData.append(decompresse.mid(debutSections[3]-padd, debutSections[4]-debutSections[3]));
	}
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 5 (trigger)
	if(inf && inf->isModified()) {
		newData.append(inf->save());
	} else {
		newData.append(decompresse.mid(debutSections[4]-padd, debutSections[5]-debutSections[4]));
	}
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 6 (encounter)
	if(encounter && encounter->isModified()) {
		newData.append(encounter->save());
	} else {
		newData.append(decompresse.mid(debutSections[5]-padd, debutSections[6]-debutSections[5]));
	}
	toc.append((char *)&(pos = 28 + newData.size() + padd), 4);

	// Section 7 (model loader PS)
	newData.append(decompresse.mid(debutSections[6]-padd));

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
//		newData = decompresse;
//	}

	if(compress)
	{
		QByteArray compresse = LZS::compress(newData);
		quint32 taille = compresse.size();
		return QByteArray((char *)&taille, 4).append(compresse);
	}

	return newData;
}

QByteArray Field::saveSection1(const QByteArray &contenu) const
{
	QByteArray grpScriptNames, positionsScripts, positionsAKAO, allScripts, realScript, positionsTexts, allTexts, allAKAOs;
	quint32 posAKAO, posFirstAKAO;
	quint16 posTextes, posScripts, newPosTextes, nbAKAO, pos;
	quint8 nbGrpScripts, newNbGrpScripts;
	const char *constData = contenu.constData();
	
	nbGrpScripts = (quint8)contenu.at(2);//nbGrpScripts
	newNbGrpScripts = grpScripts.size();
	memcpy(&posTextes, &constData[4], 2);//posTextes (et fin des scripts)
	memcpy(&nbAKAO, &constData[6], 2);//nbAKAO
	
	posScripts = 32 + newNbGrpScripts * 72 + nbAKAO * 4;
	pos = posScripts;
	
	//Création posScripts + scripts
	quint8 nbObjets3D = 0;
	foreach(GrpScript *grpScript, grpScripts)
	{
		grpScriptNames.append( grpScript->getRealName().leftJustified(8, QChar('\x00'), true) );
		for(quint8 j=0 ; j<32 ; ++j)
		{
			realScript = grpScript->getRealScript(j);
			if(!realScript.isEmpty())	pos = posScripts + allScripts.size();
			positionsScripts.append((char *)&pos, 2);
			allScripts.append(realScript);
		}
		if(grpScript->getTypeID() == 1)		++nbObjets3D;
	}

	//Création nouvelles positions Textes
	newPosTextes = posScripts + allScripts.size();

	quint16 newNbText = textes.size();

	foreach(FF7Text *text, textes)
	{
		pos = 2 + newNbText*2 + allTexts.size();
		positionsTexts.append((char *)&pos, 2);
		allTexts.append(text->getData());
		allTexts.append('\xff');// end of text
	}

	if(tut!=NULL && tut->isModified()) {
		allAKAOs = tut->save(positionsAKAO, newPosTextes + (2 + newNbText*2 + allTexts.size()));
	} else if(nbAKAO > 0) {
		memcpy(&posFirstAKAO, &constData[32+nbGrpScripts*8], 4);
		qint32 diff = (newPosTextes - posTextes) + ((2 + newNbText*2 + allTexts.size()) - (posFirstAKAO - posTextes));// (newSizeBeforeTexts - oldSizeBeforeTexts) + (newSizeTexts - olSizeTexts)

		//Création nouvelles positions AKAO
		for(quint32 i=0 ; i<nbAKAO ; ++i)
		{
			memcpy(&posAKAO, &constData[32+nbGrpScripts*8+i*4], 4);
			posAKAO += diff;
			positionsAKAO.append((char *)&posAKAO, 4);
		}

		allAKAOs = contenu.mid(posFirstAKAO);
	}

	QByteArray mapauthor = author.toLatin1().leftJustified(8, '\x00', true), mapname = name.toLower().toLatin1().leftJustified(8, '\x00', true);
	mapauthor[7] = '\x00';
	mapname[7] = '\x00';

	return contenu.left(2) //Début
			.append((char)newNbGrpScripts) //nbGrpScripts
			.append((char)nbObjets3D) //nbObjets3D
			.append((char *)&newPosTextes, 2) //PosTextes
			.append(&constData[6], 2) //AKAO count
			.append((char *)&scale, 2)
			.append(&constData[10], 6) //Empty
			.append(mapauthor).append(mapname) //Strings
			.append(grpScriptNames) //Noms des grpScripts
			.append(positionsAKAO).append(positionsScripts).append(allScripts) //PosAKAO + PosScripts + Scripts
			.append((char *)&newNbText, 2) // nbTexts
			.append(positionsTexts) // positionsTexts
			.append(allTexts) // Texts
			.append(allAKAOs); // AKAO / tutos
}

qint8 Field::exporter(const QString &path, const QByteArray &data, bool compress)
{
	if(data.isEmpty())	return 1;

	QFile fic(path);
	if(!fic.open(QIODevice::WriteOnly | QIODevice::Truncate))	return 3;
	fic.write(save(data.mid(4), compress));
	
	return 0;
}

qint8 Field::exporterDat(const QString &path, const QByteArray &datData)
{
	if(datData.isEmpty())	return 1;

	QFile fic(path);
	if(!fic.open(QIODevice::WriteOnly | QIODevice::Truncate))	return 3;
	fic.write(saveDat(datData.mid(4), true));

	return 0;
}

qint8 Field::importer(const QString &path, FieldParts part)
{
	QFile fic(path);
	if(!fic.open(QIODevice::ReadOnly))	return 1;
	if(fic.size() > 10000000)	return 1;

	QByteArray contenu;
	
	if(path.endsWith(".lzs", Qt::CaseInsensitive))
	{
		quint32 fileSize;
		fic.read((char *)&fileSize, 4);
		if(fileSize+4 != fic.size()) return 2;

		contenu = LZS::decompress(fic.readAll());
		if(part.testFlag(Scripts)) {
			close();
			if(openSection1(contenu, 46) == -1)	return 3;
		}
	}
	else if(path.endsWith(".dat", Qt::CaseInsensitive))
	{
		quint32 fileSize;
		fic.read((char *)&fileSize, 4);
		if(fileSize+4 != fic.size()) return 2;

		contenu = LZS::decompress(fic.readAll());
		if(part.testFlag(Scripts)) {
			close();
			if(openSection1(contenu, 28) == -1)	return 3;
		}
	}
	else
	{
		contenu = fic.readAll();
		if(part.testFlag(Scripts)) {
			close();
			if(openSection1(contenu, 46) == -1)	return 3;
		}
	}

	if(part.testFlag(Akaos)) {
		TutFile *tut = getTut();
		if(!tut->open(contenu))		return 3;
		tut->setModified(true);
	}
	if(part.testFlag(Encounter)) {
		EncounterFile *enc = getEncounter();
		if(!enc->open(contenu))		return 3;
		enc->setModified(true);
	}
	if(part.testFlag(Walkmesh)) {
		IdFile *walk = getId();
		if(!walk->open(contenu))	return 3;
		walk->setModified(true);
	}
	if(part.testFlag(Camera)) {
		CaFile *ca = getCa();
		if(!ca->open(contenu))		return 3;
		ca->setModified(true);
	}
	if(part.testFlag(Inf)) {
		InfFile *inf = getInf();
		if(!inf->open(contenu, true))		return 3;
		inf->setModified(true);
	}
	if(part.testFlag(HrcLoader)) {
		if(!path.endsWith(".dat", Qt::CaseInsensitive) && position != 0) {
			getFieldModelLoader()->load(contenu, this->name);
		}
	}
	
	return 0;
}
