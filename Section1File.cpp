/****************************************************************************
 ** Deling Final Fantasy VIII Section1File Editor
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
#include "Section1File.h"

Section1File::Section1File() :
	modified(false), opened(false), _tut(0)
{
}

Section1File::~Section1File()
{
	foreach(GrpScript *grpScript, _grpScripts)	delete grpScript;
	foreach(FF7Text *texte, _texts)				delete texte;
}

void Section1File::clear()
{
	foreach(GrpScript *grpScript, _grpScripts)	delete grpScript;
	foreach(FF7Text *texte, _texts)				delete texte;
	_grpScripts.clear();
	_texts.clear();
	_author.clear();

	opened = false;
}

bool Section1File::open(const QByteArray &data)
{
	quint16 posTexts;
	int dataSize = data.size();
	const char *constData = data.constData();

	if(dataSize < 32)	return false;

	memcpy(&posTexts, &constData[4], 2);//posTexts (et fin des scripts)
	if((quint32)dataSize < posTexts || posTexts < 32)	return false;

	clear();

	/* ---------- SCRIPTS ---------- */

	quint32 posAKAO = 0;
	quint16 nbAKAO, posScripts, pos;
	quint8 j, k, grpVides=0, nbScripts = (quint8)data.at(2);

	GrpScript *grpScript;

	//this->nbObjets3D = (quint8)data.at(3);
	memcpy(&nbAKAO, &constData[6], 2);//nbAKAO
	posScripts = 32+8*nbScripts+4*nbAKAO;

	if(posTexts < posScripts+64*nbScripts)	return false;

	memcpy(&_scale, &constData[8], 2);
	_author = data.mid(16, 8);
	//QString name2 = data.mid(24, 8);

	quint16 positions[33];

	for(quint8 i=0 ; i<nbScripts ; ++i)
	{
		grpScript = new GrpScript(QString(data.mid(32+8*i,8)));
		if(grpVides > 1)
		{
			for(int j=0 ; j<32 ; ++j)	grpScript->addScript();
			_grpScripts.append(grpScript);
			grpVides--;
			continue;
		}

		//Listage des positions de départ
		memcpy(positions, &constData[posScripts+64*i], 64);

		//Ajout de la position de fin
		if(i==nbScripts-1)	positions[32] = posTexts;
		else
		{
			memcpy(&pos, &constData[posScripts+64*i+64], 2);

			if(pos > positions[31])	positions[32] = pos;
			else
			{
				grpVides = 1;
				while(pos <= positions[31] && i+grpVides<nbScripts-1)
				{
					memcpy(&pos, &constData[posScripts+64*(i+grpVides)+64], 2);
					grpVides++;
				}
				if(i+grpVides==nbScripts)	positions[32] = posTexts;
				else	positions[32] = pos;
			}
		}

		k=0;
		for(j=0 ; j<32 ; ++j)
		{
			if(positions[j+1] > positions[j])
			{
				if(!grpScript->addScript(data.mid(positions[j], positions[j+1]-positions[j]))) {
					return false;
				}
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
//		QString out;
//		bool pasok = false;
//		for(int i=0 ; i<nbAKAO ; ++i) {
//			memcpy(&posAKAO, &constData[32+8*nbScripts+i*4], 4);
//			out.append(QString("%1 %2 %3 (%4)\n").arg(i).arg(posAKAO).arg(QString(data.mid(posAKAO, 4))).arg(QString(data.mid(posAKAO-4, 8).toHex())));
//			if(data.mid(posAKAO, 4) != "AKAO" && data.at(posAKAO) != '\x12') {
//				pasok = true;
//			}
//		}
//		if(pasok) {
//			qDebug() << out;
//		}

		memcpy(&posAKAO, &constData[32+8*nbScripts], 4);//posAKAO
	}
	else
	{
		posAKAO = dataSize;
	}

	/* ---------- TEXTS ---------- */

	if((posAKAO -= posTexts) > 4)//If there are texts
	{
		quint16 posDeb, posFin, nbTextes;
		if(dataSize < posTexts+2)	return false;
		memcpy(&posDeb, &constData[posTexts+2], 2);
		nbTextes = posDeb/2 - 1;

		for(quint16 i=1 ; i<nbTextes ; ++i)
		{
			memcpy(&posFin, &constData[posTexts+2+i*2], 2);

			if(dataSize < posTexts+posFin)	return false;

			_texts.append(new FF7Text(data.mid(posTexts+posDeb, posFin-posDeb)));
			posDeb = posFin;
		}
		if((quint32)dataSize < posAKAO)	return false;
		_texts.append(new FF7Text(data.mid(posTexts+posDeb, posAKAO-posDeb)));
	}

	opened = true;

	return true;
}

QByteArray Section1File::save(const QByteArray &data) const
{
	QByteArray grpScriptNames, positionsScripts, positionsAKAO, allScripts, realScript, positionsTexts, allTexts, allAKAOs;
	quint32 posAKAO, posAKAOs, newPosAKAOs;
	quint16 posTocAKAOs, posTexts, newPosScripts, newPosTexts, newNbAKAO, pos;
	quint8 nbGrpScripts, newNbGrpScripts;
	const char *constData = data.constData();

	nbGrpScripts = (quint8)data.at(2);//nbGrpScripts
	newNbGrpScripts = _grpScripts.size();
	memcpy(&posTexts, &constData[4], 2);//posTexts (et fin des scripts)

	if(_tut && _tut->isModified()) {
		newNbAKAO = _tut->size();
	} else {
		memcpy(&newNbAKAO, &constData[6], 2);//nbAKAO
	}

	newPosScripts = 32 + newNbGrpScripts * 72 + newNbAKAO * 4;
	pos = newPosScripts;

	//Création newPosScripts + scripts
	quint8 nbObjets3D = 0;
	foreach(GrpScript *grpScript, _grpScripts)
	{
		grpScriptNames.append( grpScript->getRealName().leftJustified(8, QChar('\x00'), true) );
		for(quint8 j=0 ; j<32 ; ++j)
		{
			realScript = grpScript->toByteArray(j);
			if(!realScript.isEmpty())	pos = newPosScripts + allScripts.size();
			positionsScripts.append((char *)&pos, 2);
			allScripts.append(realScript);
		}
		if(grpScript->getTypeID() == GrpScript::Model)		++nbObjets3D;
	}

	//Création nouvelles positions Textes
	newPosTexts = newPosScripts + allScripts.size();

	quint16 newNbText = _texts.size();

	foreach(FF7Text *text, _texts)
	{
		pos = 2 + newNbText*2 + allTexts.size();
		positionsTexts.append((char *)&pos, 2);
		allTexts.append(text->getData());
		allTexts.append('\xff');// end of text
	}

	newPosAKAOs = newPosTexts + (2 + newNbText*2 + allTexts.size());

	if(_tut && _tut->isModified()) {
		allAKAOs = _tut->save(positionsAKAO, newPosAKAOs);
	} else if(newNbAKAO > 0) {
		posTocAKAOs = 32 + nbGrpScripts*8;
		memcpy(&posAKAOs, &constData[posTocAKAOs], 4);
		qint32 diff = newPosAKAOs - posAKAOs;

		//Création nouvelles positions AKAO
		for(quint32 i=0 ; i<newNbAKAO ; ++i)
		{
			memcpy(&posAKAO, &constData[posTocAKAOs + i*4], 4);
			posAKAO += diff;
			positionsAKAO.append((char *)&posAKAO, 4);
		}

		allAKAOs = data.mid(posAKAOs);
	}

	QByteArray mapauthor = _author.toLatin1().leftJustified(8, '\x00', true);
	mapauthor[7] = '\x00';

	return data.left(2) //Début
			.append((char)newNbGrpScripts) //nbGrpScripts
			.append((char)nbObjets3D) //nbObjets3D
			.append((char *)&newPosTexts, 2) //PosTextes
			.append((char *)&newNbAKAO, 2) //AKAO count
			.append((char *)&_scale, 2)
			.append(&constData[10], 6) //Empty
			.append(mapauthor) //mapAuthor
			.append(&constData[24], 8) //mapName
			.append(grpScriptNames) //Noms des grpScripts
			.append(positionsAKAO).append(positionsScripts).append(allScripts) //PosAKAO + PosScripts + Scripts
			.append((char *)&newNbText, 2) // nbTexts
			.append(positionsTexts) // positionsTexts
			.append(allTexts) // Texts
			.append(allAKAOs); // AKAO / tutos
}

bool Section1File::isOpen() const
{
	return opened;
}

bool Section1File::isModified() const
{
	return modified || (_tut && _tut->isModified());
}

void Section1File::setModified(bool modified)
{
	this->modified = modified;
}

int Section1File::getModelID(quint8 grpScriptID) const
{
	if(_grpScripts.at(grpScriptID)->getTypeID() != GrpScript::Model)	return -1;

	int ID=0;

	for(int i=0 ; i<grpScriptID ; ++i)
	{
		if(_grpScripts.at(i)->getTypeID()==GrpScript::Model)
			++ID;
	}
	return ID;
}

void Section1File::getBgParamAndBgMove(QHash<quint8, quint8> &paramActifs, qint16 *z, qint16 *x, qint16 *y) const
{
	foreach(GrpScript *grpScript, _grpScripts) {
		grpScript->getBgParams(paramActifs);
		if(z)	grpScript->getBgMove(z, x, y);
	}
}

const QList<GrpScript *> &Section1File::grpScripts() const
{
	return _grpScripts;
}

GrpScript *Section1File::grpScript(int groupID) const
{
	return _grpScripts.at(groupID);
}

int Section1File::grpScriptCount() const
{
	return _grpScripts.size();
}

void Section1File::insertGrpScript(int row)
{
	_grpScripts.insert(row, new GrpScript);
	modified = true;
}

void Section1File::insertGrpScript(int row, GrpScript *grpScript)
{
	_grpScripts.insert(row, grpScript);
	modified = true;
}

void Section1File::deleteGrpScript(int row)
{
	if(row < _grpScripts.size()) {
		delete _grpScripts.takeAt(row);
		modified = true;
	}
}

void Section1File::removeGrpScript(int row)
{
	if(row < _grpScripts.size()) {
		_grpScripts.removeAt(row);
		modified = true;
	}
}

bool Section1File::moveGrpScript(int row, bool direction)
{
	if(row >= _grpScripts.size())	return false;

	if(direction)
	{
		if(row == _grpScripts.size()-1)	return false;
		_grpScripts.swap(row, row+1);
		modified = true;
	}
	else
	{
		if(row == 0)	return false;
		_grpScripts.swap(row, row-1);
		modified = true;
	}
	return true;
}

void Section1File::searchAllVars(QList<FF7Var> &vars) const
{
	foreach(GrpScript *group, _grpScripts)
		group->searchAllVars(vars);
}

bool Section1File::searchOpcode(int opcode, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0)
		groupID = scriptID = opcodeID = 0;
	if(groupID >= _grpScripts.size())
		return false;
	if(_grpScripts.at(groupID)->searchOpcode(opcode, scriptID, opcodeID))
		return true;

	return searchOpcode(opcode, ++groupID, scriptID = 0, opcodeID = 0);
}

bool Section1File::searchVar(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0)
		groupID = scriptID = opcodeID = 0;
	if(groupID >= _grpScripts.size())
		return false;
	if(_grpScripts.at(groupID)->searchVar(bank, adress, value, scriptID, opcodeID))
		return true;

	return searchVar(bank, adress, value, ++groupID, scriptID = 0, opcodeID = 0);
}

bool Section1File::searchExec(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0)
		groupID = scriptID = opcodeID = 0;
	if(groupID >= _grpScripts.size())
		return false;
	if(_grpScripts.at(groupID)->searchExec(group, script, scriptID, opcodeID))
		return true;

	return searchExec(group, script, ++groupID, scriptID = 0, opcodeID = 0);
}

bool Section1File::searchMapJump(quint16 field, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0)
		groupID = scriptID = opcodeID = 0;
	if(groupID >= _grpScripts.size())
		return false;
	if(_grpScripts.at(groupID)->searchMapJump(field, scriptID, opcodeID))
		return true;

	return searchMapJump(field, ++groupID, scriptID = 0, opcodeID = 0);
}

bool Section1File::searchTextInScripts(const QRegExp &text, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID < 0)
		groupID = scriptID = opcodeID = 0;
	if(groupID >= _grpScripts.size())
		return false;

	if(_grpScripts.at(groupID)->searchTextInScripts(text, scriptID, opcodeID))
		return true;

	return searchTextInScripts(text, ++groupID, scriptID = 0, opcodeID = 0);
}

bool Section1File::searchText(const QRegExp &text, int &textID, int &from, int &size) const
{
	if(textID < 0)
		textID = 0;
	if(textID >= _texts.size())
		return false;
	if((from = _texts.at(textID)->indexOf(text, from, size)) != -1)
		return true;

	return searchText(text, ++textID, from = 0, size);
}

bool Section1File::searchOpcodeP(int opcode, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size()-1;
		scriptID = opcodeID = 2147483647;
	}
	if(groupID < 0)
		return false;
	if(_grpScripts.at(groupID)->searchOpcodeP(opcode, scriptID, opcodeID))
		return true;

	return searchOpcodeP(opcode, --groupID, scriptID = 2147483647, opcodeID = 2147483647);
}

bool Section1File::searchVarP(quint8 bank, quint8 adress, int value, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size()-1;
		scriptID = opcodeID = 2147483647;
	}
	if(groupID < 0)
		return false;
	if(_grpScripts.at(groupID)->searchVarP(bank, adress, value, scriptID, opcodeID))
		return true;

	return searchVarP(bank, adress, value, --groupID, scriptID = 2147483647, opcodeID = 2147483647);
}

bool Section1File::searchExecP(quint8 group, quint8 script, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size()-1;
		scriptID = opcodeID = 2147483647;
	}
	if(groupID < 0)
		return false;
	if(_grpScripts.at(groupID)->searchExecP(group, script, scriptID, opcodeID))
		return true;

	return searchExecP(group, script, --groupID, scriptID = 2147483647, opcodeID = 2147483647);
}

bool Section1File::searchMapJumpP(quint16 field, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size()-1;
		scriptID = opcodeID = 2147483647;
	}
	if(groupID < 0)
		return false;
	if(_grpScripts.at(groupID)->searchMapJumpP(field, scriptID, opcodeID))
		return true;

	return searchMapJumpP(field, --groupID, scriptID = 2147483647, opcodeID = 2147483647);
}

bool Section1File::searchTextInScriptsP(const QRegExp &text, int &groupID, int &scriptID, int &opcodeID) const
{
	if(groupID >= _grpScripts.size()) {
		groupID = _grpScripts.size()-1;
		scriptID = opcodeID = 2147483647;
	}
	if(groupID < 0)
		return false;
	if(_grpScripts.at(groupID)->searchTextInScriptsP(text, scriptID, opcodeID))
		return true;

	return searchTextInScriptsP(text, --groupID, scriptID = 2147483647, opcodeID = 2147483647);
}

bool Section1File::searchTextP(const QRegExp &text, int &textID, int &from, int &index, int &size) const
{
	if(textID >= _texts.size()) {
		textID = _texts.size()-1;
		from = -1;
	}
	if(textID < 0)
		return false;
	if((index = _texts.at(textID)->lastIndexOf(text, from, size)) != -1)
		return true;

	return searchTextP(text, --textID, from = -1, index, size);
}

void Section1File::setWindow(const FF7Window &win)
{
	if(win.groupID < _grpScripts.size()) {
		_grpScripts.at(win.groupID)->setWindow(win);
		modified = true;
	}
}

void Section1File::listWindows(QMultiMap<quint64, FF7Window> &windows, QMultiMap<quint8, quint64> &text2win) const
{
	int groupID=0;
	foreach(GrpScript *group, _grpScripts)
		group->listWindows(groupID++, windows, text2win);
}

//void Section1File::searchWindows() const
//{
//	int groupID=0;
//	foreach(GrpScript *group, _grpScripts) {
//		const QList<Script *> &scripts = group->getScripts();
//		if(!scripts.isEmpty()) {
//			scripts.at(0)->searchWindows();
//			if(scripts.size() > 0) {
//				scripts.at(1)->searchWindows();

//				if(group->getTypeID() == GrpScript::Model) {
//					if(scripts.size() > 1) {
//						scripts.at(2)->searchWindows(); // talk
//					}
//					if(scripts.size() > 2) {
//						scripts.at(3)->searchWindows(); // touch
//					}
//				} else if(group->getTypeID() == GrpScript::Location) {
//					if(scripts.size() > 1) {
//						scripts.at(2)->searchWindows(); // talk
//					}
//					if(scripts.size() > 2) {
//						scripts.at(3)->searchWindows(); // touch
//					}
//					if(scripts.size() > 3) {
//						scripts.at(4)->searchWindows(); // move
//					}
//					if(scripts.size() > 4) {
//						scripts.at(5)->searchWindows(); // go
//					}
//					if(scripts.size() > 5) {
//						scripts.at(6)->searchWindows(); // go1
//					}
//					if(scripts.size() > 6) {
//						scripts.at(7)->searchWindows(); // leave
//					}
//				}

//			}
//		}
//	}
//		group->listWindows(groupID++, windows, text2win);
//}

QList<FF7Text *> *Section1File::texts()
{
	return &_texts;
}

int Section1File::textCount() const
{
	return _texts.size();
}

FF7Text *Section1File::text(int textID) const
{
	return _texts.at(textID);
}

void Section1File::insertText(int row)
{
	_texts.insert(row, new FF7Text);
	foreach(GrpScript *grpScript, _grpScripts)
		grpScript->shiftTextIds(row-1, +1);
	modified = true;
}

void Section1File::deleteText(int row)
{
	if(row < _texts.size()) {
		delete _texts.takeAt(row);
		foreach(GrpScript *grpScript, _grpScripts)
			grpScript->shiftTextIds(row, -1);
		modified = true;
	}
}

QSet<quint8> Section1File::listUsedTexts() const
{
	QSet<quint8> usedTexts;
	foreach(GrpScript *grpScript, _grpScripts)
		grpScript->listUsedTexts(usedTexts);
	return usedTexts;
}

void Section1File::shiftTutIds(int row, int shift)
{
	foreach(GrpScript *grpScript, _grpScripts)
		grpScript->shiftTutIds(row, shift);
	modified = true;
}

QSet<quint8> Section1File::listUsedTuts() const
{
	QSet<quint8> usedTuts;
	foreach(GrpScript *grpScript, _grpScripts)
		grpScript->listUsedTuts(usedTuts);
	return usedTuts;
}

const QString &Section1File::author() const
{
	return _author;
}

void Section1File::setAuthor(const QString &author)
{
	_author = author;
	modified = true;
}

quint16 Section1File::scale() const
{
	return _scale;
}

void Section1File::setScale(quint16 scale)
{
	_scale = scale;
	modified = true;
}

TutFile *Section1File::tut() const
{
	return _tut;
}

void Section1File::setTut(TutFile *tut)
{
	_tut = tut;
}
