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
#include "FieldPS.h"
#include "FieldPC.h"
#include "GZIP.h"
#include "LZS.h"
#include "Data.h"
#include "Config.h"

FieldIO::FieldIO(Field *field, QObject *parent) :
	QIODevice(parent), _field(field)
{
}

void FieldIO::close()
{
	_cache.clear();
	QIODevice::close();
}

qint64 FieldIO::readData(char *data, qint64 maxSize)
{
	if(setCache()) {
		if(pos() < _cache.size()) {
			const char *constData = _cache.constData();
			qint64 r = qMin(maxSize, _cache.size() - pos());
			if(r > 0) {
				memcpy(data, constData + pos(), r);
				return r;
			} else if(r == 0) {
				return 0;
			} else {
				return -1;
			}
		} else {
			return 0;
		}
	}
	return -1;
}

bool FieldIO::setCache()
{
	if(_cache.isEmpty()) {
		if(!_field->save(_cache, true)) {
			_cache.clear();
			return false;
		}
	}
	return true;
}

FieldArchive::FieldArchive() :
	fic(NULL), lgp(NULL), dir(NULL), iso(NULL), isDat(false)
{
}

FieldArchive::FieldArchive(const QString &path, bool isDirectory) :
	fic(NULL), lgp(NULL), dir(NULL), iso(NULL), isDat(false)
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
			if(isDat) {
				fic = new QLockedFile(path);
			} else {
				lgp = new Lgp(path);
			}
		}
	}
	//	fileWatcher.addPath(path);
	//	connect(&fileWatcher, SIGNAL(fileChanged(QString)), this, SIGNAL(fileChanged(QString)));
	//	connect(&fileWatcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(directoryChanged(QString)));
}

FieldArchive::~FieldArchive()
{
	foreach(Field *field, fileList)	delete field;
	foreach(TutFile *tut, tuts)		if(tut != NULL)	delete tut;
	if(fic!=NULL)	delete fic;
	if(lgp!=NULL)	delete lgp;
	if(dir!=NULL)	delete dir;
	if(iso!=NULL)	delete iso;
	clearCachedData();
}

QString FieldArchive::path() const
{
	if(dir!=NULL)
		return dir->path();
	if(fic!=NULL)
		return fic->fileName();
	if(lgp!=NULL)
		return lgp->fileName();
	if(iso!=NULL)
		return iso->fileName();

	return QString();
}

QString FieldArchive::name() const
{
	if(dir!=NULL)
		return QString();

	QString filePath = path();
	if(filePath.isEmpty()) {
		return filePath;
	}

	return filePath.mid(filePath.lastIndexOf("/") + 1);
}

QString FieldArchive::chemin() const
{
	if(dir!=NULL)
		return dir->path() + "/";

	QString filePath = path();
	if(filePath.isEmpty()) {
		return filePath;
	}

	return filePath.left(filePath.lastIndexOf("/") + 1);
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
	return lgp != NULL;
}

bool FieldArchive::isIso() const
{
	return iso != NULL;
}

bool FieldArchive::openField(Field *field, bool dontOptimize)
{
	if(!field->isOpen()) {
		return field->open(dontOptimize);
	}
	return true;
}

Field *FieldArchive::field(quint32 id, bool open, bool dontOptimize)
{
	Field *field = fileList.value(id, NULL);
	if(field!=NULL && open && !openField(field, dontOptimize)) {
		return NULL;
	}
	return field;
}

QByteArray FieldArchive::fieldDataCache;
QByteArray FieldArchive::mimDataCache;
QByteArray FieldArchive::modelDataCache;
Field *FieldArchive::fieldCache=0;
Field *FieldArchive::mimCache=0;
Field *FieldArchive::modelCache=0;

QByteArray FieldArchive::getFieldData(Field *field, bool unlzs)
{
	QByteArray data;

	// use data from the cache
	if(unlzs && fieldDataIsCached(field)) {
//		qDebug() << "FieldArchive use field data from cache" << field->getName();
		return fieldDataCache;
	} /*else {
		qDebug() << "FieldArchive don't use field data from cache" << field->getName() << unlzs;
	}*/

	if(isDatFile() || isLgp()) {
		data = getFileData(field->getName(), unlzs);
	} else if(isIso() || isDirectory()) {
		data = getFileData(field->getName().toUpper()+".DAT", unlzs);
	}

	if(unlzs && !data.isEmpty()) { // put decompressed data in the cache
		fieldCache = field;
		fieldDataCache = data;
	}
	return data;
}

QByteArray FieldArchive::getMimData(Field *field, bool unlzs)
{
	// use data from the cache
	if(unlzs && mimDataIsCached(field)) {
		return mimDataCache;
	}

	QByteArray data = getFileData(field->getName().toUpper()+".MIM", unlzs);

	if(unlzs && !data.isEmpty()) { // put decompressed data in the cache
		mimCache = field;
		mimDataCache = data;
	}
	return data;
}

QByteArray FieldArchive::getModelData(Field *field, bool unlzs)
{
	// use data from the cache
	if(unlzs && modelDataIsCached(field)) {
		return modelDataCache;
	}

	QByteArray data = getFileData(field->getName().toUpper()+".BSX", unlzs);

	if(unlzs && !data.isEmpty()) { // put decompressed data in the cache
		modelCache = field;
		modelDataCache = data;
	}
	return data;
}

QByteArray FieldArchive::getFileData(const QString &fileName, bool unlzs)
{
	QByteArray data;

	if(isLgp()) {
		if(!lgp->isOpen() && !lgp->open()) return QByteArray();
		data = lgp->fileData(fileName);
	} else if(isIso()) {
		data = iso->file(isoFieldDirectory->file(fileName.toUpper()));
	} else if(isDirectory()) {
		QFile f(chemin() + fileName.toUpper());
		if(!f.open(QIODevice::ReadOnly))	return QByteArray();
		data = f.readAll();
		f.close();
	} else if(isDatFile()) {
		if(!fic->isOpen() && !fic->open(QIODevice::ReadOnly))		return QByteArray();
		fic->reset();
		data = fic->readAll();
		fic->close();
	} else {
		return QByteArray();
	}

	if(data.size() < 4)		return QByteArray();

	const char *lzsDataConst = data.constData();
	quint32 lzsSize;
	memcpy(&lzsSize, lzsDataConst, 4);

	if(!Config::value("lzsNotCheck").toBool() && (quint32)data.size() != lzsSize + 4)
		return QByteArray();

	return unlzs ? LZS::decompressAll(lzsDataConst + 4, qMin(lzsSize, quint32(data.size() - 4))) : data;
}

TutFile *FieldArchive::getTut(const QString &name)
{
	if(!isLgp()) return NULL;

	QMapIterator<QString, TutFile *> it(tuts);

	while(it.hasNext()) {
		it.next();

		const QString &tutName = it.key();

		if(name.startsWith(tutName, Qt::CaseInsensitive)) {
			TutFile *tutFile = it.value();
			if(tutFile == NULL) {
				if(!lgp->isOpen() && !lgp->open())
					return NULL;
				QByteArray data = lgp->fileData(tutName + ".tut");
				if(!data.isEmpty()) {
					tutFile = new TutFile(data, true);
					tuts.insert(name, tutFile);
					return tutFile;
				} else {
					return NULL;
				}
			} else {
				return tutFile;
			}
		}
	}

	return NULL;
}

bool FieldArchive::fieldDataIsCached(Field *field) const
{
	return fieldCache && fieldCache == field;
}

bool FieldArchive::mimDataIsCached(Field *field) const
{
	return mimCache && mimCache == field;
}

bool FieldArchive::modelDataIsCached(Field *field) const
{
	return modelCache && modelCache == field;
}

void FieldArchive::clearCachedData()
{
//	qDebug() << "FieldArchive::clearCachedData()";
	fieldCache = 0;
	mimCache = 0;
	modelCache = 0;
	fieldDataCache.clear();
	mimDataCache.clear();
	modelDataCache.clear();
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
			field->scriptsAndTexts()->searchAllVars(vars);
		}
	}

	return vars;
}

void FieldArchive::searchAll()
{
	int size = fileList.size();

	QTime t;t.start();
	bool iff = false, win = false;
	OpcodeIf *opcodeIf=0;

	for(int i=0 ; i<size ; ++i) {
		Field *field = this->field(i, true);
		if(field != NULL) {
//			qDebug() << field->getName();
			/*int scriptID=0, opcodeID=0;
			Section1File *scripts = field->scriptsAndTexts();
			foreach(GrpScript *group, scripts->grpScripts()) {
				scriptID=0;
				foreach(Script *script, group->getScripts()) {
					opcodeID = 0;
					opcodeIf = 0;
					iff = win = false;
					foreach(Opcode *opcode, script->getOpcodes()) {
						if(opcode->id() == Opcode::IFUB || opcode->id() == Opcode::IFUBL
								|| opcode->id() == Opcode::IFSW || opcode->id() == Opcode::IFSWL
								|| opcode->id() == Opcode::IFUW || opcode->id() == Opcode::IFUWL) {
							opcodeIf = (OpcodeIf *)opcode;
							iff = true;
							win = false;
						} else if(iff) {
							iff = false;
							win = opcode->id() == Opcode::WSIZW || opcode->id() == Opcode::WINDOW;
						} else if(win) {
							if(opcode->isJump()) {
								qDebug() << field->getName() << group->getName() << "script" << scriptID << "line" << opcodeID << opcodeIf->toString();
							}
							iff = false;
							win = false;
						} else {
							iff = false;
							win = false;
						}
						opcodeID++;
					}
					scriptID++;
				}
			}*/
			QString out;
			InfFile *inf = field->getInf();
			if(inf != NULL) {
				int curExit=0;
				QString curOut;
				foreach(const Exit &exit, inf->exitLines()) {
					if(exit.fieldID != 0x7FFF && !inf->arrowIsDisplayed(curExit)) {
						curOut.append(QString("Sortie vers %1 sans flêche\n").arg(Data::field_names.value(exit.fieldID)));
					}
					++curExit;
				}
				if(!curOut.isEmpty()) {
					out.append(QString("=== %1 ===\n").arg(field->getName()));
					out.append(curOut);

					int redArrowCount = 0;
					foreach(const Arrow &arrow, inf->arrows()) {
						if(arrow.type == 1) {
							redArrowCount++;
						}
					}
					if(redArrowCount > 0) {
						out.append(QString("Mais %1 flêche(s) rouge ont été trouvée(s)\n").arg(redArrowCount));
					}
				}

				if(!out.isEmpty())
					qDebug() << out.toLatin1().data();
			}

		}
	}
}

bool FieldArchive::searchIterators(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &end, int fieldID, Sorting sorting) const
{
	if(fieldID >= fileList.size())		return false;

	switch(sorting) {
	case SortByName:
		i = fieldsSortByName.constFind(fieldsSortByName.key(fieldID), fieldID);
		end = fieldsSortByName.constEnd();
		if(i==end) {
			i = fieldsSortByName.constBegin();
		}
		return true;
	case SortByMapId:
		i = fieldsSortByMapId.constFind(fieldsSortByMapId.key(fieldID), fieldID);
		end = fieldsSortByMapId.constEnd();
		if(i==end) {
			i = fieldsSortByMapId.constBegin();
		}
		return true;
	}
	return true;
}

bool FieldArchive::searchOpcode(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, end;
	if(!searchIterators(i, end, fieldID, sorting))	return false;

	for( ; i != end ; ++i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchOpcode(opcode, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 0;
	}
	return false;
}

bool FieldArchive::searchVar(quint8 bank, quint8 adress, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, end;
	if(!searchIterators(i, end, fieldID, sorting))	return false;

	for( ; i != end ; ++i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchVar(bank, adress, value, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 0;
	}
	return false;
}

bool FieldArchive::searchExec(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, end;
	if(!searchIterators(i, end, fieldID, sorting))	return false;

	for( ; i != end ; ++i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchExec(group, script, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 0;
	}
	return false;
}

bool FieldArchive::searchMapJump(quint16 _field, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, end;
	if(!searchIterators(i, end, fieldID, sorting))	return false;

	for( ; i != end ; ++i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchMapJump(_field, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 0;
	}
	return false;
}

bool FieldArchive::searchTextInScripts(const QRegExp &text, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, end;
	if(!searchIterators(i, end, fieldID, sorting))	return false;

	for( ; i != end ; ++i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchTextInScripts(text, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 0;
	}
	return false;
}

bool FieldArchive::searchText(const QRegExp &text, int &fieldID, int &textID, int &from, int &size, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, end;
	if(!searchIterators(i, end, fieldID, sorting))	return false;

	for( ; i != end ; ++i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchText(text, textID, from, size))
			return true;
		textID = from = 0;
	}
	return false;
}

bool FieldArchive::searchIteratorsP(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &begin, int fieldID, Sorting sorting) const
{
	if(fieldID < 0)		return false;

	switch(sorting) {
	case SortByName:
		begin = fieldsSortByName.constBegin();
		i = fieldsSortByName.constFind(fieldsSortByName.key(fieldID), fieldID);
		if(i==fieldsSortByName.constEnd()) {
			--i;
		}
		return true;
	case SortByMapId:
		begin = fieldsSortByMapId.constBegin();
		i = fieldsSortByMapId.constFind(fieldsSortByMapId.key(fieldID), fieldID);
		if(i==fieldsSortByMapId.constEnd()) {
			--i;
		}
		return true;
	}
	return true;
}

bool FieldArchive::searchOpcodeP(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, begin;
	if(!searchIteratorsP(i, begin, fieldID, sorting))	return false;

	for( ; i != begin-1 ; --i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchOpcodeP(opcode, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 2147483647;
	}

	return false;
}

bool FieldArchive::searchVarP(quint8 bank, quint8 adress, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, begin;
	if(!searchIteratorsP(i, begin, fieldID, sorting))	return false;

	for( ; i != begin-1 ; --i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchVarP(bank, adress, value, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 2147483647;
	}
	return false;
}

bool FieldArchive::searchExecP(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, begin;
	if(!searchIteratorsP(i, begin, fieldID, sorting))	return false;

	for( ; i != begin-1 ; --i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchExecP(group, script, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 2147483647;
	}
	return false;
}

bool FieldArchive::searchMapJumpP(quint16 _field, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, begin;
	if(!searchIteratorsP(i, begin, fieldID, sorting))	return false;

	for( ; i != begin-1 ; --i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchMapJumpP(_field, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 2147483647;
	}
	return false;
}

bool FieldArchive::searchTextInScriptsP(const QRegExp &text, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, begin;
	if(!searchIteratorsP(i, begin, fieldID, sorting))	return false;

	for( ; i != begin-1 ; --i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchTextInScriptsP(text, groupID, scriptID, opcodeID))
			return true;
		groupID = scriptID = opcodeID = 2147483647;
	}
	return false;
}

bool FieldArchive::searchTextP(const QRegExp &text, int &fieldID, int &textID, int &from, int &index, int &size, Sorting sorting)
{
	QMap<QString, int>::const_iterator i, begin;
	if(!searchIteratorsP(i, begin, fieldID, sorting))	return false;

	for( ; i != begin-1 ; --i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && f->scriptsAndTexts()->searchTextP(text, textID, from, index, size))
			return true;
		textID = 2147483647;
		from = -1;
	}
	return false;
}

void FieldArchive::close()
{
//	qDebug() << "FieldArchive::close()";
	if(fic!=NULL)	fic->close();
	if(lgp!=NULL)	lgp->close();
	clearCachedData();
//	qDebug() << "/FieldArchive::close()";
}

FieldArchive::ErrorCode FieldArchive::open()
{
//	qDebug() << "FieldArchive::open()";
	foreach(Field *field, fileList)	delete field;
	foreach(TutFile *tut, tuts) if(tut != NULL)	delete tut;
	fileList.clear();
	tuts.clear();
	fieldsSortByName.clear();
	fieldsSortByMapId.clear();
	Data::field_names.clear();
	clearCachedData();

	if(isDirectory())
	{
		QStringList list;
		list.append("*.DAT");
		list = dir->entryList(list, QDir::Files | QDir::NoSymLinks);

		emit nbFilesChanged(list.size());

		// QTime t;t.start();

		for(int i=0 ; i<list.size() ; ++i) {
			QCoreApplication::processEvents();
			emit progress(i);

			QString path = dir->filePath(list.at(i));
			QString name = path.mid(path.lastIndexOf('/')+1);
			fileList.append(new FieldPS(name.left(name.lastIndexOf('.')), this));
		}

		// qDebug("Ouverture : %d ms", t.elapsed());
	} else if(isDatFile()) {
		QString path = fic->fileName();
		QString name = path.mid(path.lastIndexOf('/')+1);
		fileList.append(new FieldPS(name.left(name.lastIndexOf('.')), this));
	} else if(isIso()) {
		isoFieldDirectory = iso->rootDirectory()->directory("FIELD");
		if(isoFieldDirectory == NULL)	return FieldNotFound;

		QList<IsoFile *> files = isoFieldDirectory->files();

		emit nbFilesChanged(files.size());

		// QTime t;t.start();

		int i=0;
		foreach(IsoFile *file, files) {
			QCoreApplication::processEvents();
			emit progress(i++);

			if(file->name().endsWith(".DAT") && !file->name().startsWith("WM")) {
				QString name = file->name().mid(file->name().lastIndexOf('/')+1);
				fileList.append(new FieldPS(name.left(name.lastIndexOf('.')), this));
			}
		}
		// qDebug("Ouverture : %d ms", t.elapsed());
	} else if(isLgp()) {
		if(!lgp->isOpen() && !lgp->open())	return ErrorOpening;

		QStringList archiveList = lgp->fileList();

		if(archiveList.isEmpty()) {
			return Invalid;
		}

		emit nbFilesChanged(archiveList.size());

		quint32 i, freq = archiveList.size()>50 ? archiveList.size()/50 : 1;

		QTime t;t.start();

		i = 0;
		foreach(const QString &name, archiveList) {
			if(i % freq == 0) {
				QCoreApplication::processEvents();
				emit progress(i);
			}

			if(name.compare("maplist", Qt::CaseInsensitive) == 0) {
				if(!Data::openMaplist(lgp->fileData(name))) {
					qWarning() << "Cannot open maplist!";
				}
			} else if(name.endsWith(".tut", Qt::CaseInsensitive)) {
				tuts.insert(name.toLower().left(name.size()-4), NULL);
			} else if(!name.contains('.')) {
				fileList.append(new FieldPC(name, this));
				++i;
			}
		}
		qDebug("Ouverture : %d ms", t.elapsed());
		// qDebug() << FICHIER::size << " o";
		// qDebug() << "nbGS : " << GrpScript::COUNT;
		// qDebug() << "nbS : " << Script::COUNT;
		// qDebug() << "nbC : " << Opcode::COUNT;
		// qDebug("-------------------------------------------------");
	}
	if(fileList.isEmpty())	return FieldNotFound;

	if(Data::field_names.isEmpty()) {
		Data::openMaplist(isLgp());
	}

	int fieldID=0;
	foreach(Field *f, fileList) {
		const QString &name = f->getName();

		int index;
		QString mapId;
		if((index = Data::field_names.indexOf(name)) != -1) {
			mapId = QString("%1").arg(index, 3);
		} else {
			mapId = "~";
		}
		fieldsSortByName.insert(name, fieldID);
		fieldsSortByMapId.insert(mapId, fieldID);
		++fieldID;
	}

//	qDebug() << "/FieldArchive::open()";

	return Ok;
}

void FieldArchive::setSaved()
{
	foreach(Field *field, fileList) {
		field->setSaved();
	}
	foreach(TutFile *tut, tuts) {
		if(tut != NULL) {
			tut->setModified(false);
		}
	}
}

FieldArchive::ErrorCode FieldArchive::save(QString path)
{
//	qDebug() << "FieldArchive::save()" << path;
	quint32 nbFiles;
	bool saveAs;

	if(isDirectory() || isDatFile())
	{
		nbFiles = fileList.size();
		emit nbFilesChanged(nbFiles);

		for(quint32 fieldID=0 ; fieldID<nbFiles ; ++fieldID)
		{
			Field *field = fileList.at(fieldID);
			if(field->isOpen() && field->isModified())
			{
				QString datPath = isDatFile() ? fic->fileName() : dir->filePath(field->getName()+".DAT");
				qint8 err = field->save(datPath, true);
				if(err == 2)	return ErrorOpening;
				if(err == 1)	return Invalid;
				if(err != 0)	return NotImplemented;
			}
			emit progress(fieldID);
		}

		setSaved();
		clearCachedData(); // Important: the file data will change

		return Ok;
	}
	else if(isLgp())
	{
		if(!lgp->isOpen() && !lgp->open())	return ErrorOpening;

		foreach(Field *field, fileList) {
			if(field->isOpen() && field->isModified()) {
				if(!lgp->setFile(field->getName(), new FieldIO(field))) {
					return FieldNotFound;
				}
			}
		}

		QMapIterator<QString, TutFile *> itTut(tuts);

		while(itTut.hasNext()) {
			itTut.next();
			TutFile *tut = itTut.value();

			if(tut != NULL && tut->isModified()) {
//				qDebug() << "[TUT]" << tutName << "======== modified";
				QByteArray tocTut, tutData;
				tutData = tut->save(tocTut);
				tocTut.append(tutData);
				if(!lgp->setFile(itTut.key() + ".tut", tocTut)) {
					return FieldNotFound;
				}
			}
		}

		if(!lgp->pack(path, this)) {
			switch(lgp->error()) {
			case Lgp::OpenError:
				return ErrorOpeningTemp;
			case Lgp::InvalidError:
				return Invalid;
			case Lgp::CopyError:
			case Lgp::RemoveError:
			case Lgp::RenameError:
				return ErrorRemoving;
			default:
				return Invalid;
			}
		}

		// Clear "isModified" state
		setSaved();
		clearCachedData(); // Important: the file data will change

		// qDebug("Ecrire le nouvel Lgp : %d ms", t.elapsed());
		return Ok;
	}
	else if(isIso())
	{
		if(path.isNull())
			path = iso->fileName();

		saveAs = QFileInfo(path) != QFileInfo(*iso);

		IsoArchive isoTemp(path%".makoutemp");
		if(!isoTemp.open(QIODevice::ReadWrite | QIODevice::Truncate))		return ErrorOpening;

		emit nbFilesChanged(100);
		// Reset IsoControl
		baseEstimation = 0;
		estimation = 100;

		// FIELD/*.DAT

		bool archiveModified = false;

		foreach(Field *field, fileList) {
			if(field->isOpen() && field->isModified()) {
				IsoFile *isoField = isoFieldDirectory->file(field->getName().toUpper() + ".DAT");
				if(isoField == NULL) {
					continue;
				}

				QByteArray newData;

				if(field->save(newData, true)) {
					isoField->setData(newData);
					archiveModified = true;
				} else {
					return Invalid;
				}
			}
		}

		if(!archiveModified) {
			return Invalid;
		}

		// FIELD/FIELD.BIN

		IsoFile *isoFieldBin = isoFieldDirectory->file("FIELD.BIN");
		if(isoFieldBin == NULL) {
			return Invalid;
		}

		QByteArray data = updateFieldBin(iso->file(isoFieldBin), isoFieldDirectory);
		if(data.isEmpty()) {
			return Invalid;
		}
		isoFieldBin->setData(data);

//		qDebug() << "start";

		if(!iso->pack(&isoTemp, this, isoFieldDirectory))
			return Invalid;

		// End

		// Remove or close the old file
		if(!saveAs) {
			if(!iso->remove())	return ErrorRemoving;
		} else {
			iso->close();
			iso->setFileName(path);
		}
		// Move the temporary file
		if(QFile::exists(path) && !QFile::remove(path))		return ErrorRemoving;
		isoTemp.rename(path);

		iso->open(QIODevice::ReadOnly);

		// Clear "isModified" state
		iso->applyModifications(isoFieldDirectory);
		setSaved();
		clearCachedData(); // Important: the file data will change

		return Ok;
	}

	return NotImplemented;
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

//	qDebug() << "header field.bin" << header.toHex() << QString::number(data.size()-8,16) << decSize << ungzip.size();

	// mise à jour

	int indicativePosition=0x30000, count;
	QByteArray copy = ungzip;
	QMapIterator<QByteArray, QByteArray> i(changementsFieldBin);
	while(i.hasNext()) {
		i.next();
		count = copy.count(i.key());
		if(count == 0) {
			qWarning() << "Error not found!" << fichiers.value(i.key());
			return QByteArray();
		} else if(count == 1) {
			indicativePosition = copy.indexOf(i.key());
			ungzip.replace(i.key(), i.value());
		} else {
			qWarning() << "error multiple occurrences 1" << fichiers.value(i.key());
			if(copy.mid(indicativePosition).count(i.key())==1) {
				ungzip.replace(copy.indexOf(i.key(), indicativePosition), 8, i.value());
			} else {
				qWarning() << "error multiple occurrences 2" << fichiers.value(i.key());
				return QByteArray();
			}
		}
	}

	copy = GZIP::compress(ungzip);

	if(copy.isEmpty())	return QByteArray();

	return header.append(copy);
}
