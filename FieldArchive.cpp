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
#include "FieldArchive.h"

FieldArchive::FieldArchive()
	: fic(NULL), dir(NULL), iso(NULL), isDat(false)
{
}

FieldArchive::FieldArchive(const QString &path, bool isDirectory)
	: fic(NULL), dir(NULL), iso(NULL), isDat(false)
{
	if(isDirectory) {
		dir = new QDir(path);
	}
	else {
		QString ext = path.mid(path.lastIndexOf('.')+1).toLower();

		if(ext == "iso" || ext == "bin") {
			iso = new IsoArchive(path);
			if(!iso->open(QIODevice::ReadOnly)) {
				delete iso;
				iso = NULL;
			}
		}
		else {
			isDat = ext == "dat";
			fic = new QFile(path);
		}
	}
	//	fileWatcher.addPath(path);
	//	connect(&fileWatcher, SIGNAL(fileChanged(QString)), this, SIGNAL(fileChanged(QString)));
	//	connect(&fileWatcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(directoryChanged(QString)));
}

FieldArchive::~FieldArchive()
{
	foreach(Field *field, fileList)	delete field;
	foreach(TutFile *tut, tuts)		delete tut;
	if(fic!=NULL)	delete fic;
	if(dir!=NULL)	delete dir;
	if(iso!=NULL)	delete iso;
}

QString FieldArchive::path() const
{
	if(dir!=NULL)
		return dir->path();
	if(fic!=NULL)
		return fic->fileName();
	if(iso!=NULL)
		return iso->fileName();

	return QString();
}

QString FieldArchive::name() const
{
	if(fic!=NULL)
		return fic->fileName().mid(fic->fileName().lastIndexOf("/")+1);
	if(iso!=NULL)
		return iso->fileName().mid(iso->fileName().lastIndexOf("/")+1);

	return QString();
}

QString FieldArchive::chemin() const
{
	if(dir!=NULL)
		return dir->path() + "/";
	if(fic!=NULL)
		return fic->fileName().left(fic->fileName().lastIndexOf("/")+1);
	if(iso!=NULL)
		return iso->fileName().left(iso->fileName().lastIndexOf("/")+1);

	return QString();
}

int FieldArchive::size() const
{
	return fileList.size();
}

bool FieldArchive::isDatFile() const
{
	return fic != NULL && isDat;
}

bool FieldArchive::isDirectory() const
{
	return dir != NULL;
}

bool FieldArchive::isLgp() const
{
	return fic != NULL && !isDat;
}

bool FieldArchive::isIso() const
{
	return iso != NULL;
}

qint8 FieldArchive::openField(Field *field)
{
	if(!field->isOpen()) {
		QByteArray fieldData = getFieldData(field, false);
		if(isLgp())		return field->open(fieldData);
		else			return field->open2(fieldData);
	}
	return 0;
}

Field *FieldArchive::field(quint32 id, bool open)
{
	Field *field = fileList.value(id, NULL);
	if(field!=NULL && open && openField(field) != 0) {
		return NULL;
	}
	return field;
}

Field *FieldArchive::field(const QString &name, bool open)
{
	int index;
	if((index = rechercherFichier(name)) != -1) {
		return field(index, open);
	}
	return NULL;
}

QByteArray FieldArchive::getLgpData(int position)
{
	if(fic==NULL)	return QByteArray();
	quint32 fileSize;

	if(!fic->open(QIODevice::ReadOnly))		return QByteArray();
	fic->seek(position+20);
	fic->read((char *)&fileSize, 4);

	QByteArray data = fic->read(fileSize);

	fic->close();

	return data;
}

QByteArray FieldArchive::getFieldData(Field *field, bool unlzs)
{
	quint32 lzsSize;
	QByteArray data;

	if(isDatFile()) {
		if(!fic->open(QIODevice::ReadOnly))		return QByteArray();
		data = fic->readAll();
		fic->close();

		memcpy(&lzsSize, data.constData(), 4);

		if((quint32)data.size() != lzsSize+4)	return QByteArray();

		return unlzs ? LZS::decompress(data.mid(4)) : data;
	} else if(isLgp()) {
		data = getLgpData(field->getPosition());
		memcpy(&lzsSize, data.constData(), 4);
		if((quint32)data.size() != lzsSize + 4)				return QByteArray();

		return unlzs ? LZS::decompress(data.mid(4)) : data;
	} else if(isIso()) {
		data = iso->file(isoFieldDirectory->file(field->getName().toUpper()+".DAT"));

		memcpy(&lzsSize, data.constData(), 4);

		if((quint32)data.size() != lzsSize+4)	return QByteArray();

		return unlzs ? LZS::decompress(data.mid(4)) : data;
	} else if(isDirectory()) {
		QFile fic(chemin()+field->getName().toUpper()+".DAT");
		if(!fic.open(QIODevice::ReadOnly))	return QByteArray();
		data = fic.readAll();
		fic.close();

		memcpy(&lzsSize, data.constData(), 4);

		if((quint32)data.size() != lzsSize+4)	return QByteArray();

		return unlzs ? LZS::decompress(data.mid(4)) : data;
	} else {
		return QByteArray();
	}
}

QByteArray FieldArchive::getMimData(Field *field, bool unlzs)
{
	quint32 lzsSize;
	QByteArray data;

	if(isLgp()) {
		return QByteArray();
	} else if(isIso()) {
		data = iso->file(isoFieldDirectory->file(field->getName().toUpper()+".MIM"));

		memcpy(&lzsSize, data.constData(), 4);

		if((quint32)data.size() != lzsSize+4)	return QByteArray();

		return unlzs ? LZS::decompress(data.mid(4)) : data;
	} else if(isDatFile() || isDirectory()) {
		QFile fic(chemin()+field->getName().toUpper()+".MIM");
		if(!fic.open(QIODevice::ReadOnly))	return QByteArray();
		data = fic.readAll();
		fic.close();

		memcpy(&lzsSize, data.constData(), 4);

		if((quint32)data.size() != lzsSize+4)	return QByteArray();

		return unlzs ? LZS::decompress(data.mid(4)) : data;
	} else {
		return QByteArray();
	}
}

QByteArray FieldArchive::getModelData(Field *field, bool unlzs)
{
	quint32 lzsSize;
	QByteArray data;

	if(isLgp()) {
		return QByteArray();
	} else if(isIso()) {
		data = iso->file(isoFieldDirectory->file(field->getName().toUpper()+".BSX"));

		memcpy(&lzsSize, data.constData(), 4);

		if((quint32)data.size() != lzsSize+4)	return QByteArray();

		return unlzs ? LZS::decompress(data.mid(4)) : data;
	} else if(isDatFile() || isDirectory()) {
		QFile fic(chemin()+field->getName().toUpper()+".BSX");
		if(!fic.open(QIODevice::ReadOnly))	return QByteArray();
		data = fic.readAll();
		fic.close();

		memcpy(&lzsSize, data.constData(), 4);

		if((quint32)data.size() != lzsSize+4)	return QByteArray();

		return unlzs ? LZS::decompress(data.mid(4)) : data;
	} else {
		return QByteArray();
	}
}

TutFile *FieldArchive::getTut(const QString &name)
{
	if(!isLgp()) return NULL;

	QMapIterator<QString, int> i(tutPos);
	while(i.hasNext()) {
		i.next();
		if(name.startsWith(i.key(), Qt::CaseInsensitive)) {
			int pos = i.value();
			if(tuts.contains(i.key())) {
				return tuts.value(i.key());
			} else {
				QByteArray data = getLgpData(pos);
				if(!data.isEmpty()) {
					TutFile *tutFile = new TutFile(data, true);
					tuts.insert(i.key(), tutFile);
					return tutFile;
				}
			}
		}
	}

	return NULL;
}

bool FieldArchive::isAllOpened()
{
	foreach(Field *f, fileList) {
		if(!f->isOpen())	return false;
	}
	return true;
}

QList<FF7Var> FieldArchive::searchAllVars()
{
	QList<FF7Var> vars;
	int size = fileList.size();

	for(int i=0 ; i<size ; ++i) {
		QCoreApplication::processEvents();
		Field *field = this->field(i);
		if(field != NULL) {
			vars.append(field->searchAllVars());
		}
	}

	return vars;
}

void FieldArchive::searchAll()
{
	int size = fileList.size();

	for(int i=0 ; i<size ; ++i) {
		Field *field = this->field(i);
		if(field != NULL) {
//			qDebug() << field->getName();
//			InfFile *inf = field->getInf();
//			if(!inf->isOpen()) {
//				if(inf->open(getFieldData(field))) {
//					inf->test();
//				}
//			}
			qDebug() << field->getName();
			WalkmeshFile *walkmesh = field->getWalkmesh();
			if(!walkmesh->isOpen()) {
				if(walkmesh->open(getFieldData(field))) {
//					if(walkmesh->test()) {
//						qDebug() << field->getName();
//					}
				}
			}
			if(!field->isModified()) {
				field->close();
			}
		}
	}
}

bool FieldArchive::rechercherOpcode(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	if(fieldID < 0)	fieldID = 0;

	int nbFields = fileList.size();

	while(fieldID < nbFields)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID);
		if(f!=NULL && f->rechercherOpcode(opcode, groupID, scriptID, opcodeID))
			return true;
		++fieldID;
		groupID = scriptID = opcodeID = 0;
	}
	return false;

	/*if(fieldID >= fileList.size())		return false;

	QMap<QString, int>::const_iterator i, end;

	switch(sorting) {
	case SortByName:
		i = fieldsSortByName.constFind(fieldsSortByName.key(fieldID), fieldID);
		end = fieldsSortByName.constEnd();
		if(i==end) {
			i = fieldsSortByName.constBegin();
		}
		break;
	case SortByMapId:
		i = fieldsSortByMapId.constFind(fieldsSortByMapId.key(fieldID), fieldID);
		end = fieldsSortByMapId.constEnd();
		if(i==end) {
			i = fieldsSortByMapId.constBegin();
		}
		break;
	}

	for( ; i != end ; ++i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->rechercherOpcode(opcode, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 0;
	}
	return false;*/
}

bool FieldArchive::rechercherVar(quint8 bank, quint8 adress, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	if(fieldID < 0)	fieldID = 0;

	int nbFields = fileList.size();

	while(fieldID < nbFields)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID);
		if(f!=NULL && f->rechercherVar(bank, adress, value, groupID, scriptID, opcodeID))
			return true;
		++fieldID;
		groupID = scriptID = opcodeID = 0;
	}
	return false;

	/*if(fieldID >= fileList.size())		return false;

	QMap<QString, int>::const_iterator i, end;

	switch(sorting) {
	case SortByName:
		i = fieldsSortByName.constFind(fieldsSortByName.key(fieldID), fieldID);
		end = fieldsSortByName.constEnd();
		if(i==end) {
			i = fieldsSortByName.constBegin();
		}
		break;
	case SortByMapId:
		i = fieldsSortByMapId.constFind(fieldsSortByMapId.key(fieldID), fieldID);
		end = fieldsSortByMapId.constEnd();
		if(i==end) {
			i = fieldsSortByMapId.constBegin();
		}
		break;
	}

	for( ; i != end ; ++i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->rechercherVar(bank, adress, value, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 0;
	}
	return false;*/
}

bool FieldArchive::rechercherExec(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	if(fieldID < 0)	fieldID = 0;

	int nbFields = fileList.size();

	while(fieldID < nbFields)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID);
		if(f!=NULL && f->rechercherExec(group, script, groupID, scriptID, opcodeID))
			return true;
		++fieldID;
		groupID = scriptID = opcodeID = 0;
	}
	return false;

	/*if(fieldID >= fileList.size())		return false;

	QMap<QString, int>::const_iterator i, end;

	switch(sorting) {
	case SortByName:
		i = fieldsSortByName.constFind(fieldsSortByName.key(fieldID), fieldID);
		end = fieldsSortByName.constEnd();
		if(i==end) {
			i = fieldsSortByName.constBegin();
		}
		break;
	case SortByMapId:
		i = fieldsSortByMapId.constFind(fieldsSortByMapId.key(fieldID), fieldID);
		end = fieldsSortByMapId.constEnd();
		if(i==end) {
			i = fieldsSortByMapId.constBegin();
		}
		break;
	}

	for( ; i != end ; ++i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->rechercherExec(group, script, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 0;
	}
	return false;*/
}

bool FieldArchive::rechercherTexte(const QRegExp &texte, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	if(fieldID < 0)	fieldID = 0;

	int nbFields = fileList.size();

	while(fieldID < nbFields)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID);
		if(f!=NULL && f->rechercherTexte(texte, groupID, scriptID, opcodeID))
			return true;
		++fieldID;
		groupID = scriptID = opcodeID = 0;
	}
	return false;

	/*if(fieldID >= fileList.size())		return false;

	QMap<QString, int>::const_iterator i, end;

	switch(sorting) {
	case SortByName:
		i = fieldsSortByName.constFind(fieldsSortByName.key(fieldID), fieldID);
		end = fieldsSortByName.constEnd();
		if(i==end) {
			i = fieldsSortByName.constBegin();
		}
		break;
	case SortByMapId:
		i = fieldsSortByMapId.constFind(fieldsSortByMapId.key(fieldID), fieldID);
		end = fieldsSortByMapId.constEnd();
		if(i==end) {
			i = fieldsSortByMapId.constBegin();
		}
		break;
	}

	for( ; i != end ; ++i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->rechercherTexte(texte, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 0;
	}
	return false;*/
}

bool FieldArchive::rechercherOpcodeP(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	//	if(fieldID < 0)	return false;

	/*QMap<QString, int>::const_iterator i, begin;

	switch(sorting) {
	case SortByName:
		begin = fieldsSortByName.constBegin();
		i = fieldsSortByName.constFind(fieldsSortByName.key(fieldID), fieldID);
		if(i==fieldsSortByName.constEnd()) {
			--i;
		}
		break;
	case SortByMapId:
		begin = fieldsSortByMapId.constBegin();
		i = fieldsSortByMapId.constFind(fieldsSortByMapId.key(fieldID), fieldID);
		if(i==fieldsSortByMapId.constEnd()) {
			--i;
		}
		break;
	}

	for( ; i != begin-1 ; --i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->rechercherOpcodeP(opcode, groupID, scriptID, opcodeID))
			return true;
		if(i != begin) {
			fieldID = (i-1).value();
			if(fieldID >= 0) {
				groupID = field(fieldID)->grpScripts.size()-1;
				if(groupID >= 0) {
					scriptID = field(fieldID)->grpScripts.at(groupID)->size()-1;
					if(scriptID >= 0) {
						opcodeID = field(fieldID)->grpScripts.at(groupID)->getScript(scriptID)->size()-1;
					}
				}
			}
		}
	}
	fieldID = -1;
	return false;*/

	if(fieldID >= fileList.size())	fieldID = fileList.size()-1;

	while(fieldID >= 0)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID);
		if(f!=NULL && f->rechercherOpcodeP(opcode, groupID, scriptID, opcodeID))	return true;
		--fieldID;
		if(fieldID >= 0) {
			groupID = field(fieldID)->grpScripts.size()-1;
			if(groupID >= 0) {
				scriptID = field(fieldID)->grpScripts.at(groupID)->size()-1;
				if(scriptID >= 0) {
					opcodeID = field(fieldID)->grpScripts.at(groupID)->getScript(scriptID)->size()-1;
				}
			}
		}
	}
	return false;
}

bool FieldArchive::rechercherVarP(quint8 bank, quint8 adress, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	if(fieldID >= fileList.size())	fieldID = fileList.size()-1;

	while(fieldID >= 0)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID);
		if(f!=NULL && f->rechercherVarP(bank, adress, value, groupID, scriptID, opcodeID))	return true;
		--fieldID;
		if(fieldID >= 0) {
			groupID = field(fieldID)->grpScripts.size()-1;
			if(groupID >= 0) {
				scriptID = field(fieldID)->grpScripts.at(groupID)->size()-1;
				if(scriptID >= 0) {
					opcodeID = field(fieldID)->grpScripts.at(groupID)->getScript(scriptID)->size()-1;
				}
			}
		}
	}
	return false;
}

bool FieldArchive::rechercherExecP(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	if(fieldID >= fileList.size())	fieldID = fileList.size()-1;

	while(fieldID >= 0)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID);
		if(f!=NULL && f->rechercherExecP(group, script, groupID, scriptID, opcodeID))	return true;
		--fieldID;
		if(fieldID >= 0) {
			groupID = field(fieldID)->grpScripts.size()-1;
			if(groupID >= 0) {
				scriptID = field(fieldID)->grpScripts.at(groupID)->size()-1;
				if(scriptID >= 0) {
					opcodeID = field(fieldID)->grpScripts.at(groupID)->getScript(scriptID)->size()-1;
				}
			}
		}
	}
	return false;
}

bool FieldArchive::rechercherTexteP(const QRegExp &texte, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	if(fieldID >= fileList.size())	fieldID = fileList.size()-1;

	while(fieldID >= 0)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID);
		if(f!=NULL && f->rechercherTexteP(texte, groupID, scriptID, opcodeID))	return true;
		--fieldID;
		if(fieldID >= 0) {
			groupID = field(fieldID)->grpScripts.size()-1;
			if(groupID >= 0) {
				scriptID = field(fieldID)->grpScripts.at(groupID)->size()-1;
				if(scriptID >= 0) {
					opcodeID = field(fieldID)->grpScripts.at(groupID)->getScript(scriptID)->size()-1;
				}
			}
		}
	}
	return false;
}

void FieldArchive::close()
{
	if(fic!=NULL)	fic->close();
}

void FieldArchive::addDAT(const QString &name, QList<QTreeWidgetItem *> &items)
{
	Field *field = new Field(name);

	QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << name.toLower() << QString());
	item->setData(0, Qt::UserRole, fileList.size());
	items.append(item);
	fileList.append(field);
}

quint8 FieldArchive::open(QList<QTreeWidgetItem *> &items)
{
	foreach(Field *field, fileList)	delete field;
	foreach(TutFile *tut, tuts)		delete tut;
	fileList.clear();
	tuts.clear();
	tutPos.clear();
	fieldsSortByName.clear();
	fieldsSortByMapId.clear();
	Data::field_names.clear();

	if(isDirectory())
	{
		QStringList list;
		list.append("*.DAT");
		list = dir->entryList(list, QDir::Files | QDir::NoSymLinks);

		emit nbFilesChanged(list.size());

		//		QTime t;t.start();

		for(int i=0 ; i<list.size() ; ++i) {
			QCoreApplication::processEvents();
			emit progress(i);

			QString path = dir->filePath(list.at(i));
			QString name = path.mid(path.lastIndexOf('/')+1);
			addDAT(name.left(name.lastIndexOf('.')), items);
		}

		//		qDebug("Ouverture : %d ms", t.elapsed());
	} else if(isDatFile()) {
		QString path = fic->fileName();
		QString name = path.mid(path.lastIndexOf('/')+1);
		addDAT(name.left(name.lastIndexOf('.')), items);
	} else if(isIso()) {
		isoFieldDirectory = iso->rootDirectory()->directory("FIELD");
		if(isoFieldDirectory == NULL) {
			return 4;
		}

		emit nbFilesChanged(isoFieldDirectory->filesAndDirectories().size());

		//		QTime t;t.start();

		int i=0;
		foreach(IsoFile *file, isoFieldDirectory->files()) {
			QCoreApplication::processEvents();
			emit progress(i++);

			if(file->name().endsWith(".DAT") && !file->name().startsWith("WM")) {
				QString name = file->name().mid(file->name().lastIndexOf('/')+1);
				addDAT(name.left(name.lastIndexOf('.')), items);
			}
		}
		//		qDebug("Ouverture : %d ms", t.elapsed());
	} else if(isLgp()) {
		if(!fic->open(QIODevice::ReadOnly))	return 1;

		quint32 nbFiles;

		fic->seek(12);
		fic->read((char *)&nbFiles,4);

		if(nbFiles==0 || fic->size()<16+27*nbFiles)	return 2;

		quint32 filePos;
		QList<quint32> listPos;

		QByteArray toc = fic->read(27 * nbFiles);
		const char *tocData = toc.constData();

		quint32 i;
		for(i=0 ; i<nbFiles ; ++i)
		{
			QString name(QByteArray(&tocData[27 * i], 20));
			if(!name.contains(".")) {
				memcpy(&filePos, &tocData[27 * i + 20], 4);
				listPos.append(filePos);
			} else if(name.endsWith(".tut", Qt::CaseInsensitive)) {
				memcpy(&filePos, &tocData[27 * i + 20], 4);
				name.chop(4);// strip ".tut"
				tutPos.insert(name, filePos);
			}
		}
		if(listPos.isEmpty())	return 3;
		qSort(listPos);

		emit nbFilesChanged(listPos.last());

		quint32 fileSize, lzsSize, freq = listPos.size()>50 ? listPos.size()/50 : 1;

		//		QTime t;t.start();

		Field *field;
		i = 0;
		foreach(const quint32 &pos, listPos)
		{
			if(i%freq==0) {
				QCoreApplication::processEvents();
				emit progress(pos);
			}

			if(!fic->seek(pos))	break;
			QByteArray data = fic->read(24);
			QString name(data.left(20));
			memcpy(&fileSize, data.constData()+20, 4);

			if(name.compare("maplist", Qt::CaseInsensitive) == 0)	Data::openMaplist(fic->read(fileSize));
			else
			{
				fic->read((char *)&lzsSize, 4);
				if(fileSize != lzsSize+4)	continue;

				field = new Field(pos, name);

				QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << name << QString());
				item->setData(0, Qt::UserRole, fileList.size());
				fileList.append(field);
				items.append(item);
				++i;
			}
		}
		//		qDebug("Ouverture : %d ms", t.elapsed());
		// qDebug() << FICHIER::size << " o";
		// qDebug() << "nbGS : " << GrpScript::COUNT;
		// qDebug() << "nbS : " << Script::COUNT;
		// qDebug() << "nbC : " << Opcode::COUNT;
		// qDebug("-------------------------------------------------");
	}
	if(items.isEmpty())	return 3;

	if(Data::field_names.isEmpty()) {
		Data::openMaplist(isLgp());
	}

	int index;
	foreach(QTreeWidgetItem *item, items) {
		QString name = item->text(0);
		int id = item->data(0, Qt::UserRole).toInt();

		if((index = Data::field_names.indexOf(name)) != -1) {
			item->setText(1, QString("%1").arg(index,3));
		} else {
			item->setText(1, "~");
		}
		fieldsSortByName.insert(name, id);
		fieldsSortByMapId.insert(item->text(1), id);
	}

	return 0;
}

qint32 FieldArchive::rechercherFichier(const QString &name) const
{
	qint32 i=0;
	foreach(Field *field, fileList) {
		if(field->getName().compare(name, Qt::CaseInsensitive) == 0)	return i;
		++i;
	}
	return -1;
}

void FieldArchive::setSaved()
{
	foreach(Field *field, fileList) {
		field->setSaved();
	}
	foreach(TutFile *tut, tuts) {
		tut->setModified(false);
	}
}

quint8 FieldArchive::save(QString path)
{
	quint32 nbFiles, pos, taille, oldtaille, fileSize;
	qint32 fileID;
	bool saveAs;

	if(isDirectory() || isDatFile())
	{
		nbFiles = fileList.size();
		emit nbFilesChanged(nbFiles);

		for(quint32 fileID=0 ; fileID<nbFiles ; ++fileID)
		{
			Field *field = fileList.at(fileID);
			if(field->isOpen() && field->isModified())
			{
				QTemporaryFile tempFic;
				if(!tempFic.open())		return 2;
				QFile DATfic(isDatFile() ? fic->fileName() : dir->filePath(field->getName()+".DAT"));
				if(!DATfic.open(QIODevice::ReadOnly)) return 1;
				DATfic.read((char *)&fileSize,4);
				tempFic.write(field->saveDat(DATfic.read(fileSize), true));//nouveau fichier
				if(!DATfic.remove()) return 4;
				tempFic.copy(DATfic.fileName());
			}
			emit progress(fileID);
		}

		setSaved();

		return 0;
	}
	else if(isLgp())
	{
		QMap<quint32, quint32> positions1, positions2;
		QMap<Field *, quint32> newPositions;
		QMap<QString, quint32> newPositionsTut;
		QByteArray chaine, nouveauFichier;
		QString tutName;

		if(path.isNull())
			path = fic->fileName();

		saveAs = path != fic->fileName();

		// QFile fic(this->path());
		if(!fic->open(QIODevice::ReadOnly))	return 1;
		QTemporaryFile tempFic(path%".makoutemp");
		if(!tempFic.open())		return 2;

		fic->seek(12);
		fic->read((char *)&nbFiles, 4);

		if(nbFiles>1000 || nbFiles==0) 	return 5;

		emit nbFilesChanged(nbFiles+2);

		//	QTime t;t.start();
		//Parcourir la table des matière
		QByteArray toc = fic->read(27 * nbFiles);
		const char *tocData = toc.constData();

		for(quint32 i=0 ; i<nbFiles ; ++i)
		{
			memcpy(&pos, &tocData[27 * i + 20], 4);
			positions1.insert(pos, i);
		}
		// qDebug("Lister les positions des fichiers : %d ms", t.elapsed());
		// t.restart();

		QMap<quint32, quint32>::const_iterator i = positions1.constBegin();
		const quint32 positionPremierFichier = i.key();

		//	qDebug() << i.key() << (27*nbFiles + 16);//CRC size

		if(!tempFic.resize(positionPremierFichier))		return 2;
		if(!tempFic.seek(positionPremierFichier))		return 2;
		quint16 avancement = 0;
		while(i != positions1.constEnd())
		{
			QCoreApplication::processEvents();

			if(!fic->seek(i.key()))	return 3;

			//Listage positions 2
			pos = tempFic.pos();
			positions2.insert(i.value(), pos);

			fileID = rechercherFichier(QString(chaine = fic->read(20)));
			if(chaine.size() != 20 || fic->read((char *)&oldtaille, 4) != 4) 	return 3;
			tempFic.write(chaine);//Nom du fichier
			//			qDebug() << QString(chaine);

			if(fileID != -1)
			{
				Field *field = fileList.at(fileID);
				qDebug() << "[FIELD]" << fileID << field->getName();

				if(!saveAs)
					newPositions.insert(field, pos);

				//vérifier si on a pas une nouvelle version du fichier
				if(field->isOpen() && field->isModified())
				{
					qDebug() << "======== modified";
					//Récupérer l'ancien fichier
					if(fic->read((char *)&fileSize, 4) != 4)	return 3;
					if(oldtaille != fileSize+4)					return 3;
					//Créer le nouveau fichier à partir de l'ancien
					nouveauFichier = field->save(fic->read(fileSize), true);

					//Refaire les tailles
					taille = nouveauFichier.size();

					//Écrire le nouveau fichier dans le fichier temporaire
					tempFic.write((char *)&taille, 4);//nouvelle taille
					tempFic.write(nouveauFichier);//nouveau fichier
				}
				else
				{
					qDebug() << "unmodified";
					tempFic.write((char *)&oldtaille, 4);//Taille
					tempFic.write(fic->read(oldtaille));//Fichier
				}
			}
			else if(QString(chaine).endsWith(".tut", Qt::CaseInsensitive) && tuts.contains(tutName = chaine.left(chaine.lastIndexOf('.'))))
			{
				TutFile *tut = tuts.value(tutName);

				qDebug() << "[TUT]" << tutName;

				if(!saveAs)
					newPositionsTut.insert(tutName, pos);

				if(tut->isModified())
				{
					qDebug() << "======== modified";
					QByteArray toc, tutData;
					tutData = tut->save(toc);
					toc.append(tutData);
					taille = toc.size();
					tempFic.write((char *)&taille, 4);//Taille
					tempFic.write(toc);//Fichier
				}
				else
				{
					qDebug() << "unmodified";
					tempFic.write((char *)&oldtaille, 4);//Taille
					tempFic.write(fic->read(oldtaille));//Fichier
				}
			}
			else
			{
				qDebug() << "[NOTHING] unmodified" << QString(chaine);
				tempFic.write((char *)&oldtaille, 4);//Taille
				tempFic.write(fic->read(oldtaille));//Fichier
			}
			emit progress(avancement++);
			++i;
		}

		emit progress(nbFiles);
		tempFic.write(fic->readAll());

		// qDebug("Ecrire les fichiers : %d ms", t.elapsed());
		// t.restart();

		fic->reset();
		tempFic.reset();
		tempFic.write(fic->read(16));

		//fabrication de la nouvelle table des matières
		for(quint32 i=0 ; i<nbFiles ; ++i)
		{
			tempFic.write(fic->read(20));
			pos = positions2.value(i);
			tempFic.write((char *)&pos, 4);
			fic->seek(fic->pos() + 4);
			tempFic.write(fic->read(3));
		}
		tempFic.write(fic->read(positionPremierFichier-fic->pos()));
		// qDebug("Ecrire la table des matières : %d ms", t.elapsed());
		// t.restart();

		emit progress(nbFiles+2);

		if(saveAs)
		{
			fic->close();
			//		fic->setFileName(path);
		}
		else
		{
			//création nouveau lgp
			if(!fic->remove())	return 4;
			// qDebug("Supprimer l'ancien Lgp : %d ms", t.elapsed());
			// t.restart();
			QMapIterator<Field *, quint32> fieldIt(newPositions);
			while(fieldIt.hasNext()) {
				fieldIt.next();
				fieldIt.key()->setPosition(fieldIt.value());
			}
			//			qDebug() << tutPos << newPositionsTut;
			QMapIterator<QString, quint32> tutIt(newPositionsTut);
			while(tutIt.hasNext()) {
				tutIt.next();
				tutPos.insert(tutIt.key(), tutIt.value());
			}
			//			qDebug() << tutPos;
		}
		tempFic.copy(path);

		setSaved();

		//	qDebug("Ecrire le nouvel Lgp : %d ms", t.elapsed());
		return 0;
	}
	else if(isIso())
	{
		if(path.isNull())
			path = iso->fileName();

		saveAs = path != iso->fileName();

		IsoArchive isoTemp(path%".makoutemp");
		if(!isoTemp.open(QIODevice::ReadWrite | QIODevice::Truncate))		return 2;

		emit nbFilesChanged(100);

		// FIELD/*.DAT

		foreach(Field *field, fileList) {
			if(field->isOpen() && field->isModified()) {
				IsoFile *isoField = isoFieldDirectory->file(field->getName() + ".DAT");
				if(isoField == NULL) {
					continue;
				}

				isoField->setData(field->saveDat(iso->file(isoField).mid(4), true));
			}
		}

		// FIELD/FIELD.BIN

		IsoFile *isoFieldBin = isoFieldDirectory->file("FIELD.BIN");
		if(isoFieldBin == NULL) {
			return 3;
		}

		QByteArray data = updateFieldBin(iso->file(isoFieldBin), isoFieldDirectory);
		if(data.isEmpty()) {
			return 3;
		}
		isoFieldBin->setData(data);

		qDebug() << "start";

		if(!iso->pack(&isoTemp, this))
			return 3;

		// Fin

		if(saveAs)
		{
			iso->close();
		}
		else
		{
			//création nouveau iso
			if(!iso->remove())	return 4;
		}
		isoTemp.rename(path);

		if(!saveAs) {
			if(!iso->open(QIODevice::ReadOnly))		return 4;
		}

		setSaved();

		return 0;
	}

	return 6;
}

QByteArray FieldArchive::updateFieldBin(const QByteArray &data, IsoDirectory *fieldDirectory)
{
	QByteArray header = data.left(8), ungzip;
	quint32 oldSectorStart, newSectorStart, oldSize, newSize;
	QMap<QByteArray, QByteArray> changementsFieldBin;
	QMap<QByteArray, QString> fichiers;//debug

	foreach(IsoDirectory *fileOrDir, fieldDirectory->directories()) {
		if(fileOrDir->isModified() && !fileOrDir->name().endsWith(".X") && fileOrDir->name()!="FIELD.BIN") {
			oldSectorStart = fileOrDir->location();
			oldSize = fileOrDir->size();
			newSectorStart = fileOrDir->newLocation();
			newSize = fileOrDir->newSize();

			QByteArray tempBA = QByteArray((char *)&oldSectorStart, 4).append((char *)&oldSize, 4);
			changementsFieldBin.insert(tempBA, QByteArray((char *)&newSectorStart, 4).append((char *)&newSize, 4));
			fichiers.insert(tempBA, fileOrDir->name());
		}
	}

	/**
	 *	HEADER :
	 *	 4 : ungzipped size
	 *	 4 : ungzipped size - 51588
	 */
	// décompression gzip
	quint32 decSize;
	memcpy(&decSize, data.constData(), 4);
	ungzip = GZIP::decompress(data.mid(8), decSize);
	if(ungzip.isEmpty())	return QByteArray();

	qDebug() << "header field.bin" << header.toHex() << QString::number(data.size()-8,16) << decSize << ungzip.size();

	// mise à jour

	int indicativePosition=0x30000, count;
	QByteArray copy = ungzip;
	QMapIterator<QByteArray, QByteArray> i(changementsFieldBin);
	while(i.hasNext()) {
		i.next();
		count = copy.count(i.key());
		if(count == 0) {
			qDebug() << "Erreur introuvable !" << fichiers.value(i.key());
			return QByteArray();
		} else if(count == 1) {
			indicativePosition = copy.indexOf(i.key());
			ungzip.replace(i.key(), i.value());
		} else {
			qDebug() << "error multiple occurrences 1" << fichiers.value(i.key());
			if(copy.mid(indicativePosition).count(i.key())==1) {
				ungzip.replace(copy.indexOf(i.key(), indicativePosition), 8, i.value());
			} else {
				qDebug() << "error multiple occurrences 2" << fichiers.value(i.key());
				return QByteArray();
			}
		}
	}

	copy = GZIP::compress(ungzip);

	if(copy.isEmpty())	return QByteArray();

	return header.append(copy);
}
