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
#include "FieldPC.h"
#include "FieldPS.h"

Field::Field() :
	_isOpen(false), _isModified(false), name(QString()),
	encounter(0), tut(0), id(0), ca(0), inf(0), modelLoader(0)
{
}

Field::Field(const QString &name) :
	_isOpen(false), _isModified(false), name(name),
	encounter(0), tut(0), id(0), ca(0), inf(0), modelLoader(0)
{
}

Field::~Field()
{
	foreach(GrpScript *grpScript, _grpScripts)	delete grpScript;
	foreach(FF7Text *texte, textes)				delete texte;
	if(encounter)		delete encounter;
	if(tut)				delete tut;
	if(id)				delete id;
	if(ca)				delete ca;
	if(inf)				delete inf;
	if(modelLoader)		delete modelLoader;
}

void Field::close()
{
	foreach(GrpScript *grpScript, _grpScripts)	delete grpScript;
	foreach(FF7Text *texte, textes)				delete texte;
	_grpScripts.clear();
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
			_grpScripts.append(grpScript);
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
		_grpScripts.append(grpScript);
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
	if(_grpScripts.at(grpScriptID)->getTypeID()!=1)	return -1;

	int ID=0;

	for(int i=0 ; i<grpScriptID ; ++i)
	{
		if(_grpScripts.at(i)->getTypeID()==1)
			++ID;
	}
	return ID;
}

void Field::getBgParamAndBgMove(QHash<quint8, quint8> &paramActifs, qint16 *z, qint16 *x, qint16 *y) const
{
	foreach(GrpScript *grpScript, _grpScripts) {
		grpScript->getBgParams(paramActifs);
		if(z)	grpScript->getBgMove(z, x, y);
	}
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

const QList<GrpScript *> &Field::grpScripts() const
{
	return _grpScripts;
}

GrpScript *Field::grpScript(int groupID) const
{
	return _grpScripts.at(groupID);
}

int Field::grpScriptCount() const
{
	return _grpScripts.size();
}

void Field::insertGrpScript(int row)
{
	_grpScripts.insert(row, new GrpScript);
	_isModified = true;
}

void Field::insertGrpScript(int row, GrpScript *grpScript)
{
	GrpScript *newGrpScript = new GrpScript(grpScript->getName());
	for(int i=0 ; i<grpScript->size() ; i++)
		newGrpScript->addScript(grpScript->getScript(i)->toByteArray(), false);
	_grpScripts.insert(row, newGrpScript);
	_isModified = true;
}

void Field::deleteGrpScript(int row)
{
	if(row < _grpScripts.size()) {
		delete _grpScripts.takeAt(row);
		_isModified = true;
	}
}

void Field::removeGrpScript(int row)
{
	if(row < _grpScripts.size()) {
		_grpScripts.removeAt(row);
		_isModified = true;
	}
}

bool Field::moveGrpScript(int row, bool direction)
{
	if(row >= _grpScripts.size())	return false;
	
	if(direction)
	{
		if(row == _grpScripts.size()-1)	return false;
		_grpScripts.swap(row, row+1);
		_isModified = true;
	}
	else
	{
		if(row == 0)	return false;
		_grpScripts.swap(row, row-1);
		_isModified = true;
	}
	return true;
}

QList<FF7Var> Field::searchAllVars() const
{
	QList<FF7Var> vars;

	foreach(GrpScript *group, _grpScripts)
		vars.append(group->searchAllVars());

	return vars;
}

bool Field::searchOpcode(int opcode, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0) {
		groupID = scriptID = opcodeID = 0;
	}

	int nbGroups = _grpScripts.size();

	while(groupID < nbGroups)
	{
		if(_grpScripts.at(groupID)->searchOpcode(opcode, scriptID, opcodeID))	return true;
		++groupID;
		scriptID = opcodeID = 0;
	}
	return false;
}

bool Field::searchVar(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0) {
		groupID = scriptID = opcodeID = 0;
	}

	int nbGroups = _grpScripts.size();

	while(groupID < nbGroups)
	{
		if(_grpScripts.at(groupID)->searchVar(bank, adress, value, scriptID, opcodeID))	return true;
		++groupID;
		scriptID = opcodeID = 0;
	}
	return false;
}

bool Field::searchExec(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0) {
		groupID = scriptID = opcodeID = 0;
	}

	qDebug() << "Field::searchExec" << name << groupID << scriptID << opcodeID;

	int nbGroups = _grpScripts.size();

	while(groupID < nbGroups)
	{
		if(_grpScripts.at(groupID)->searchExec(group, script, scriptID, opcodeID))	return true;
		++groupID;
		scriptID = opcodeID = 0;
	}
	return false;
}

bool Field::searchText(const QRegExp &texte, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0) {
		groupID = scriptID = opcodeID = 0;
	}

	Data::currentTextes = &textes;

	int nbGroups = _grpScripts.size();

	while(groupID < nbGroups)
	{
		if(_grpScripts.at(groupID)->searchText(texte, scriptID, opcodeID))	return true;
		++groupID;
		scriptID = opcodeID = 0;
	}
	return false;
}

bool Field::searchOpcodeP(int opcode, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size()-1;
		scriptID = opcodeID = 2147483647;
	}

	while(groupID >= 0)
	{
		if(_grpScripts.at(groupID)->searchOpcodeP(opcode, scriptID, opcodeID))	return true;
		--groupID;
		scriptID = opcodeID = 2147483647;
	}
	return false;
}

bool Field::searchVarP(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size()-1;
		scriptID = opcodeID = 2147483647;
	}

	while(groupID >= 0)
	{
		if(_grpScripts.at(groupID)->searchVarP(bank, adress, value, scriptID, opcodeID))	return true;
		--groupID;
		scriptID = opcodeID = 2147483647;
	}
	return false;
}

bool Field::searchExecP(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size()-1;
		scriptID = opcodeID = 2147483647;
	}

	while(groupID >= 0)
	{
		if(_grpScripts.at(groupID)->searchExecP(group, script, scriptID, opcodeID))	return true;
		--groupID;
		scriptID = opcodeID = 2147483647;
	}
	return false;
}

bool Field::searchTextP(const QRegExp &texte, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size()-1;
		scriptID = opcodeID = 2147483647;
	}

	Data::currentTextes = &textes;

	while(groupID >= 0)
	{
		if(_grpScripts.at(groupID)->searchTextP(texte, scriptID, opcodeID))	return true;
		--groupID;
		scriptID = opcodeID = 2147483647;
	}
	return false;
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

void Field::insertText(int row)
{
	textes.insert(row, new FF7Text);
	foreach(GrpScript *grpScript, _grpScripts)
		grpScript->shiftTextIds(row-1, +1);
	_isModified = true;
}

void Field::deleteText(int row)
{
	if(row < textes.size()) {
		delete textes.takeAt(row);
		foreach(GrpScript *grpScript, _grpScripts)
			grpScript->shiftTextIds(row, -1);
		_isModified = true;
	}
}

QSet<quint8> Field::listUsedTexts() const
{
	QSet<quint8> usedTexts;
	foreach(GrpScript *grpScript, _grpScripts)
		grpScript->listUsedTexts(usedTexts);
	return usedTexts;
}

void Field::shiftTutIds(int row, int shift)
{
	foreach(GrpScript *grpScript, _grpScripts)
		grpScript->shiftTutIds(row, shift);
}

QSet<quint8> Field::listUsedTuts() const
{
	QSet<quint8> usedTuts;
	foreach(GrpScript *grpScript, _grpScripts)
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

const QString &Field::getName() const
{
	return name;
}

void Field::setName(const QString &name)
{
	this->name = name;
	_isModified = true;
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

void Field::setSaved()
{
	if(encounter)	encounter->setModified(false);
	if(tut)			tut->setModified(false);
	if(id)			id->setModified(false);
	if(ca)			ca->setModified(false);
	if(inf)			inf->setModified(false);
}

QByteArray Field::saveSection1(const QByteArray &contenu) const
{
	QByteArray grpScriptNames, positionsScripts, positionsAKAO, allScripts, realScript, positionsTexts, allTexts, allAKAOs;
	quint32 posAKAO, posFirstAKAO;
	quint16 posTextes, posScripts, newPosTextes, nbAKAO, pos;
	quint8 nbGrpScripts, newNbGrpScripts;
	const char *constData = contenu.constData();
	
	nbGrpScripts = (quint8)contenu.at(2);//nbGrpScripts
	newNbGrpScripts = _grpScripts.size();
	memcpy(&posTextes, &constData[4], 2);//posTextes (et fin des scripts)
	memcpy(&nbAKAO, &constData[6], 2);//nbAKAO
	
	posScripts = 32 + newNbGrpScripts * 72 + nbAKAO * 4;
	pos = posScripts;
	
	//Création posScripts + scripts
	quint8 nbObjets3D = 0;
	foreach(GrpScript *grpScript, _grpScripts)
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

	Field *field = new FieldPC(*this);

	QFile fic(path);
	if(!fic.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		delete field;
		return 3;
	}
	fic.write(field->save(data.mid(4), compress));

	delete field;

	return 0;
}

qint8 Field::exporterDat(const QString &path, const QByteArray &data)
{
	if(data.isEmpty())	return 1;

	Field *field = new FieldPS(*this);

	QFile fic(path);
	if(!fic.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		delete field;
		return 3;
	}
	fic.write(field->save(data.mid(4), true));

	delete field;

	return 0;
}

qint8 Field::importer(const QString &path, FieldParts part)
{
	QFile fic(path);
	if(!fic.open(QIODevice::ReadOnly))	return 1;
	if(fic.size() > 10000000)	return 1;

	QByteArray data;
	bool isDat = false;

	if(path.endsWith(".lzs", Qt::CaseInsensitive))
	{
		quint32 fileSize;
		fic.read((char *)&fileSize, 4);
		if(fileSize+4 != fic.size()) return 2;

		data = LZS::decompress(fic.readAll());
	}
	else if(path.endsWith(".dat", Qt::CaseInsensitive))
	{
		quint32 fileSize;
		fic.read((char *)&fileSize, 4);
		if(fileSize+4 != fic.size()) return 2;

		data = LZS::decompress(fic.readAll());
		isDat = true;
	}
	else
	{
		data = fic.readAll();
	}
	
	return importer(data, isDat, part);
}

qint8 Field::importer(const QByteArray &data, bool isDat, FieldParts part)
{
	if(part.testFlag(Scripts)) {
		close();
		if(openSection1(data, isDat? 28 : 46) == -1)	return 3;
	}

	if(part.testFlag(Akaos)) {
		TutFile *tut = getTut();
		if(!tut->open(data))		return 3;
		tut->setModified(true);
	}
	if(part.testFlag(Encounter)) {
		EncounterFile *enc = getEncounter();
		if(!enc->open(data))		return 3;
		enc->setModified(true);
	}
	if(part.testFlag(Walkmesh)) {
		IdFile *walk = getId();
		if(!walk->open(data))	return 3;
		walk->setModified(true);
	}
	if(part.testFlag(Camera)) {
		CaFile *ca = getCa();
		if(!ca->open(data))		return 3;
		ca->setModified(true);
	}
	if(part.testFlag(Inf)) {
		InfFile *inf = getInf();
		if(!inf->open(data, true))		return 3;
		inf->setModified(true);
	}

	return 0;
}
