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
#include "Data.h"
#include "Config.h"

FieldArchive::FieldArchive() :
	_io(0)
{
}

FieldArchive::FieldArchive(const QString &path, bool isDirectory) :
	_io(0)
{
	if(isDirectory) {
		_io = new FieldArchiveIODir(path);
	}
	else {
		QString ext = path.mid(path.lastIndexOf('.') + 1).toLower();

		if(ext == "iso" || ext == "bin") {
			_io = new FieldArchiveIOIso(path);
		}
		else {
			if(ext == "dat") {
				_io = new FieldArchiveIOFile(path);
			} else if(ext == "lgp") {
				_io = new FieldArchiveIOLgp(path);
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
	if(_io)		delete _io;
}

FieldArchiveIO::ErrorCode FieldArchive::open()
{
	if(!_io)	return FieldArchiveIO::Invalid;
//	qDebug() << "FieldArchive::open()";
	foreach(Field *field, fileList)	delete field;
	foreach(TutFile *tut, tuts) if(tut != NULL)	delete tut;
	fileList.clear();
	tuts.clear();
	fieldsSortByName.clear();
	fieldsSortByMapId.clear();
	Data::field_names.clear();

	FieldArchiveIO::ErrorCode error = _io->open(fileList, tuts, this);
	if(error != FieldArchiveIO::Ok) {
		return error;
	}

	if(fileList.isEmpty())	return FieldArchiveIO::FieldNotFound;

	if(Data::field_names.isEmpty()) {
		Data::openMaplist(_io->type() == FieldArchiveIO::Lgp);
	}

	int fieldID=0;
	foreach(Field *f, fileList) {
		const QString &name = f->name();

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

	return FieldArchiveIO::Ok;
}

FieldArchiveIO::ErrorCode FieldArchive::save(const QString &path)
{
	if(!_io)	return FieldArchiveIO::Invalid;

	FieldArchiveIO::ErrorCode error = _io->save(fileList, tuts, path, this);
	if(error == FieldArchiveIO::Ok) {
		// Clear "isModified" state
		setSaved();
	}
	return error;
}

void FieldArchive::close()
{
	if(_io)	_io->close();
}

int FieldArchive::size() const
{
	return fileList.size();
}

FieldArchiveIO *FieldArchive::io() const
{
	return _io;
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

TutFile *FieldArchive::tut(const QString &name)
{
	if(!_io || _io->type() != FieldArchiveIO::Lgp) {
		return NULL;
	}

	Lgp *lgp = (Lgp *)_io->device();

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

	const QList<FF7Text *> *currentTextesSav = Data::currentTextes;

	for(int i=0 ; i<size ; ++i) {
		QCoreApplication::processEvents();
		Field *field = this->field(i);
		if(field != NULL) {
			field->scriptsAndTexts()->searchAllVars(vars);
		}
	}

	Data::currentTextes = currentTextesSav;

	return vars;
}

void FieldArchive::searchAll()
{
	int size = fileList.size();

	QTime t;t.start();
	bool iff = false, win = false;
	OpcodeIf *opcodeIf=0;

	QFile deb("akao_comparison.txt");
	deb.open(QIODevice::WriteOnly | QIODevice::Text);


//	for(int i=0 ; i<size ; ++i) {
	foreach(int i, fieldsSortByMapId) {
		Field *field = this->field(i, true);
		if(field != NULL) {
			TutFile *tut = field->tutosAndSounds();
			if(tut->isOpen()) {
				deb.write(QString("=== %1 ===\n").arg(field->name()).toLatin1());
				for(int j=0; j<tut->size(); ++j) {
					if(!tut->isTut(j)) {
						deb.write(QString("id= %1\n").arg(tut->akaoID(j)).toLatin1());
					}
				}
			}
//			qDebug() << field->name();
			/*int scriptID=0, opcodeID=0;
			Section1File *scripts = field->scriptsAndTexts();
			foreach(GrpScript *group, scripts->grpScripts()) {
				scriptID=0;
				foreach(Script *script, group->scripts()) {
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
								qDebug() << field->name() << group->name() << "script" << scriptID << "line" << opcodeID << opcodeIf->toString();
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
			/*QString out;
			InfFile *inf = field->inf();
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
					out.append(QString("=== %1 ===\n").arg(field->name()));
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
			}*/

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
