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
#include "FieldArchive.h"
#include "Palette.h"
#include "LZS.h"

FieldPC::FieldPC(quint32 position, const QString &name, FieldArchive *fieldArchive) :
	Field(name, fieldArchive), position(position)
{
}

FieldPC::FieldPC(const Field &field) :
	Field(field), position(0)
{
}

FieldPC::~FieldPC()
{
}

bool FieldPC::open(bool dontOptimize)
{
	QByteArray fileData;

//	qDebug() << "FieldPC::open" << dontOptimize << name;

	if(!dontOptimize && !fieldArchive->fieldDataIsCached(this)) {
		QByteArray lzsData = fieldArchive->getFieldData(this, false);
		quint32 size;
		if(lzsData.size() < 4)		return false;
		memcpy(&size, lzsData.constData(), 4);
		if(size+4 != (quint32)lzsData.size()) 	return false;

		fileData = LZS::decompress(lzsData.mid(4), 42);//partial decompression
	} else {
		fileData = fieldArchive->getFieldData(this);
	}

	if(fileData.size() < 42)	return false;

	memcpy(sectionPositions, &(fileData.constData()[6]), 9 * 4); // header

	_isOpen = true;

	return true;
}

QByteArray FieldPC::sectionData(FieldPart part)
{
	if(!_isOpen) {
		open();
	}
	if(!_isOpen)	return QByteArray();

	switch(part) {
	case Scripts:
		return sectionData(0);
	case Akaos:
		return sectionData(0);
	case Camera:
		return sectionData(1);
	case ModelLoader:
		return sectionData(2);
	case Walkmesh:
		return sectionData(4);
	case Encounter:
		return sectionData(6);
	case Inf:
		return sectionData(7);
	case Background:
		return sectionData(8);
	}

	return QByteArray();
}

QByteArray FieldPC::sectionData(int idPart)
{
	int position = sectionPositions[idPart] + 4;
	int size;

	if(idPart < 8) {
		size = sectionPositions[idPart+1] - position;
	} else {
		size = -1;
	}

	if(size == -1 || fieldArchive->fieldDataIsCached(this)) {
		return fieldArchive->getFieldData(this).mid(position, size);
	} else {
		QByteArray lzsData = fieldArchive->getFieldData(this, false);
		quint32 lzsSize;
		const char *lzsDataConst = lzsData.constData();
		memcpy(&lzsSize, lzsDataConst, 4);
		if(lzsSize+4 != (quint32)lzsData.size()) 	return QByteArray();

		return LZS::decompress(lzsData.mid(4), sectionPositions[idPart+1]).mid(position, size);
	}
}

QPixmap FieldPC::openBackground()
{
	// Search default background params
	QHash<quint8, quint8> paramActifs;
	qint16 z[] = {-1, -1};
	scriptsAndTexts()->getBgParamAndBgMove(paramActifs, z);

	return openBackground(paramActifs, z);
}

QPixmap FieldPC::openBackground(const QHash<quint8, quint8> &paramActifs, const qint16 *z, const bool *layers)
{
//	qDebug() << "FieldPC::openBackground";
	QByteArray data = fieldArchive->getFieldData(this);
	const char *constData = data.constData();
	quint32 dataSize = data.size();
	QList<Palette> palettes;
	quint32 debutSection4, debutSection9, i, j;
	quint16 nb;

	if(dataSize < 42)	return QPixmap();

	memcpy(&debutSection4, &constData[18], 4);//Adresse Section 4
	memcpy(&debutSection9, &constData[38], 4);//Adresse Section 9

	if(dataSize < debutSection4+16)	return QPixmap();

	memcpy(&nb, &constData[debutSection4+14], 2);//nbPalettes

	if(dataSize < debutSection4+16+nb*512)	return QPixmap();
	
	for(i=0 ; i<nb ; ++i)
		palettes.append(Palette(&constData[debutSection4+16+i*512], data.at(debutSection9+16+i)));

	quint32 aTex = debutSection9+48;
	quint16 nbTiles1, nbTiles2=0, nbTiles3=0, nbTiles4=0;
	bool exist2, exist3, exist4;

	if(dataSize < aTex+2)	return QPixmap();

	memcpy(&nbTiles1, &constData[aTex], 2);//nbTiles1
	aTex += 8+nbTiles1*52;
	if(dataSize < aTex+1)	return QPixmap();
	if((exist2 = (bool)data.at(aTex)))
	{
		if(dataSize < aTex+7)	return QPixmap();
		memcpy(&nbTiles2, &constData[aTex+5], 2);//nbTiles2
		aTex += 26+nbTiles2*52;
	}
	aTex++;
	if(dataSize < aTex+1)	return QPixmap();
	if((exist3 = (bool)data.at(aTex)))
	{
		if(dataSize < aTex+7)	return QPixmap();
		memcpy(&nbTiles3, &constData[aTex+5], 2);//nbTiles3
		aTex += 20+nbTiles3*52;
	}
	aTex++;
	if(dataSize < aTex+1)	return QPixmap();
	if((exist4 = (bool)data.at(aTex)))
	{
		if(dataSize < aTex+7)	return QPixmap();
		memcpy(&nbTiles4, &constData[aTex+5], 2);//nbTiles4
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

	aTex = debutSection9+56;
	QMultiMap<qint16, Tile> tiles;
	Tile tile;
	quint16 largeurMax=0, largeurMin=0, hauteurMax=0, hauteurMin=0;

	//BG 0
	for(i=0 ; i<nbTiles1 ; ++i) {
		memcpy(&tile, &constData[aTex+i*52], 34);

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
			memcpy(&tile, &constData[aTex+i*52], 34);
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
			memcpy(&tile, &constData[aTex+i*52], 34);
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
			memcpy(&tile, &constData[aTex+i*52], 34);
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
				memcpy(&deuxOctets, &constData[origin+j], 2);
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

bool FieldPC::getUsedParams(QHash<quint8, quint8> &usedParams, bool *layerExists)
{
	QByteArray data = fieldArchive->getFieldData(this);
	const char *constData = data.constData();
	quint32 debutSection9, i;

	if(data.isEmpty())	return false;

	memcpy(&debutSection9, &constData[38], 4);//Adresse Section 9

	if((quint32)data.size() <= debutSection9+153)	return false;

	quint32 aTex = debutSection9+48;
	quint16 nbTiles1, nbTiles2=0, nbTiles3=0, nbTiles4=0;
	bool exist2, exist3, exist4;

	memcpy(&nbTiles1, &constData[aTex], 2);//nbTiles1
	aTex += 8+nbTiles1*52;
	if((exist2 = (bool)data.at(aTex)))
	{
		memcpy(&nbTiles2, &constData[aTex+5], 2);//nbTiles2
		aTex += 26+nbTiles2*52;
	}
	aTex++;
	if((exist3 = (bool)data.at(aTex)))
	{
		memcpy(&nbTiles3, &constData[aTex+5], 2);//nbTiles3
		aTex += 20+nbTiles3*52;
	}
	aTex++;
	if((exist4 = (bool)data.at(aTex)))
	{
		memcpy(&nbTiles4, &constData[aTex+5], 2);//nbTiles4
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
			memcpy(&tile, &constData[aTex+i*52], 34);
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
			memcpy(&tile, &constData[aTex+i*52], 34);
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
			memcpy(&tile, &constData[aTex+i*52], 34);
			if(tile.param && qAbs(tile.cibleX) < 1000 && qAbs(tile.cibleY) < 1000)
			{
				usedParams.insert(tile.param, usedParams.value(tile.param) | tile.state);
			}
		}
	}
	return true;
}

FieldModelLoaderPC *FieldPC::getFieldModelLoader(bool open)
{
	FieldModelLoaderPC *modelLoader = (FieldModelLoaderPC *)this->modelLoader;
	if(!modelLoader)	modelLoader = new FieldModelLoaderPC();
	if(open && !modelLoader->isLoaded()) {
		modelLoader->load(sectionData(ModelLoader));
		//	Data::currentCharNames = model_nameChar;
		//	Data::currentHrcNames = &fieldModelLoader->model_nameHRC;
		//	Data::currentAnimNames = &fieldModelLoader->model_anims;
	}
	this->modelLoader = modelLoader;

	return modelLoader;
}

FieldModelFilePC *FieldPC::getFieldModel(int modelID, int animationID, bool animate)
{
	FieldModelLoaderPC *modelLoader = getFieldModelLoader();
	QString hrc = modelLoader->HRCName(modelID);
	QString a = modelLoader->AName(modelID, animationID);

	return getFieldModel(hrc, a, animate);
}

FieldModelFilePC *FieldPC::getFieldModel(const QString &hrc, const QString &a, bool animate)
{
	if(!fieldModel)		fieldModel = new FieldModelFilePC();
	((FieldModelFilePC *)fieldModel)->load(hrc, a, animate);
	return (FieldModelFilePC *)fieldModel;
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

bool FieldPC::save(QByteArray &newData, bool compress)
{
	newData = QByteArray();
	qDebug() << "FieldPC::save" << compress << name;

	if(!isOpen())	return false;

	QByteArray decompresse = fieldArchive->getFieldData(this), section, toc;
	const char *decompresseData = decompresse.constData();
	quint32 sectionPositions[9], size, section_size;

	if(decompresse.isEmpty())	return false;

	sectionPositions[0] = 42;
	for(quint8 i=1 ; i<9 ; ++i)
		memcpy(&sectionPositions[i], &decompresseData[6+4*i], 4);

	// Header + pos section 1
	toc.append("\x00\x00", 2);
	toc.append("\x09\x00\x00\x00", 4); // section count
	toc.append("\x2A\x00\x00\x00", 4); // pos section 1

	// Section 1 (scripts + textes + akaos/tutos)
	if(section1 && section1->isModified()) {
		section = section1->save(decompresse.mid(sectionPositions[0]+4, sectionPositions[1]-sectionPositions[0]-4));
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(&decompresseData[sectionPositions[0]], sectionPositions[1]-sectionPositions[0]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 2

	// Section 2 (camera)
	section = QByteArray();
	if(ca && ca->isModified() && ca->save(section)) {
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(&decompresseData[sectionPositions[1]], sectionPositions[2]-sectionPositions[1]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 3

	// Section 3 (model loader PC)
	if(modelLoader && modelLoader->isLoaded() && modelLoader->isModified()) {
		section = getFieldModelLoader()->save();
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(&decompresseData[sectionPositions[2]], sectionPositions[3]-sectionPositions[2]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 4
	
	// Section 4 (background palette PC)
	newData.append(&decompresseData[sectionPositions[3]], sectionPositions[4]-sectionPositions[3]);

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 5

	// Section 5 (walkmesh)
	section = QByteArray();
	if(id && id->isModified() && id->save(section)) {
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(&decompresseData[sectionPositions[4]], sectionPositions[5]-sectionPositions[4]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 6

	// Section 6 (background tileMap -unused-)
	newData.append(&decompresseData[sectionPositions[5]], sectionPositions[6]-sectionPositions[5]);

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 7

	// Section 7 (encounter)
	if(_encounter && _encounter->isModified()) {
		section = _encounter->save();
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(&decompresseData[sectionPositions[6]], sectionPositions[7]-sectionPositions[6]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 8

	// Section 8 (trigger)
	if(inf && inf->isModified()) {
		section = inf->save();
		section_size = section.size();
		newData.append((char *)&section_size, 4).append(section);
	} else {
		newData.append(&decompresseData[sectionPositions[7]], sectionPositions[8]-sectionPositions[7]);
	}

	size = 42 + newData.size();
	toc.append((char *)&size, 4); // pos section 9

	// Section 9 (background PC)
	newData.append(decompresse.mid(sectionPositions[8]));

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
		newData = QByteArray((char *)&taille, 4).append(compresse);
		return true;
	}

	return true;
}

qint8 FieldPC::importer(const QByteArray &data, bool isPSField, FieldParts part)
{
	if(!isPSField) {
		quint32 sectionPositions[9];

		if(data.size() < 6 + 9 * 4)	return 3;
		memcpy(sectionPositions, &(data.constData()[6]), 9 * 4); // header

		if(part.testFlag(ModelLoader)) {
			FieldModelLoaderPC *modelLoader = getFieldModelLoader(false);
			if(!modelLoader->load(data.mid(sectionPositions[2]+4, sectionPositions[3]-sectionPositions[2]-4))) {
				return 2;
			}
			modelLoader->setModified(true);
		}
	}

	return Field::importer(data, isPSField, part);
}
