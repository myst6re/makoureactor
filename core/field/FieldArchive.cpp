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

FieldArchive::FieldArchive() :
	_io(0)
{
}

FieldArchive::FieldArchive(FieldArchiveIO *io) :
	_io(io)
{
	//	fileWatcher.addPath(path);
	//	connect(&fileWatcher, SIGNAL(fileChanged(QString)), this, SIGNAL(fileChanged(QString)));
	//	connect(&fileWatcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(directoryChanged(QString)));
}

FieldArchive::~FieldArchive()
{
	foreach(Field *field, fileList)	delete field;
	if(_io)		delete _io;
}

FieldArchiveIO::ErrorCode FieldArchive::open(FieldArchiveIOObserver *observer)
{
	if(!_io)	return FieldArchiveIO::Invalid;
//	qDebug() << "FieldArchive::open()";
	clear();

	FieldArchiveIO::ErrorCode error = _io->open(observer);
	if(error != FieldArchiveIO::Ok) {
		return error;
	}

	if(fileList.isEmpty())	return FieldArchiveIO::FieldNotFound;

	if(Data::field_names.isEmpty()) {
		Data::openMaplist(_io->isPC());
	}

	int fieldID=0;
	foreach(Field *f, fileList) {
		updateFieldLists(f, fieldID);
		++fieldID;
	}

//	qDebug() << "/FieldArchive::open()";

	return FieldArchiveIO::Ok;
}

FieldArchiveIO::ErrorCode FieldArchive::save(const QString &path, FieldArchiveIOObserver *observer)
{
	if(!_io)	return FieldArchiveIO::Invalid;

	FieldArchiveIO::ErrorCode error = _io->save(path, observer);
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

void FieldArchive::clear()
{
	foreach(Field *field, fileList)	delete field;
	fileList.clear();
	fieldsSortByName.clear();
	fieldsSortByMapId.clear();
	Data::field_names.clear();
}

int FieldArchive::size() const
{
	return fileList.size();
}

FieldArchiveIO *FieldArchive::io() const
{
	return _io;
}

void FieldArchive::setIO(FieldArchiveIO *io)
{
	_io = io;
}

bool FieldArchive::openField(Field *field, bool dontOptimize)
{
	if(!field->isOpen()) {
		return field->open(dontOptimize);
	}
	return true;
}

int FieldArchive::indexOfField(const QString &name) const
{
	return fieldsSortByName.value(name, -1);
}

const Field *FieldArchive::field(quint32 id) const
{
	return fileList.value(id, NULL);
}

Field *FieldArchive::field(quint32 id, bool open, bool dontOptimize)
{
	Field *field = fileList.value(id, NULL);
	if(field!=NULL && open && !openField(field, dontOptimize)) {
		return NULL;
	}
	return field;
}

void FieldArchive::updateFieldLists(Field *field, int fieldID)
{
	const QString &name = field->name();

	int index;
	QString mapId;
	if((index = Data::field_names.indexOf(name)) != -1) {
		mapId = QString("%1").arg(index, 3);
	} else {
		mapId = "~";
	}
	fieldsSortByName.insert(name, fieldID);
	fieldsSortByMapId.insert(mapId, fieldID);
}

void FieldArchive::addField(Field *field/*, bool referenceToMaplist*/)
{
	int fieldId = fileList.size();
	appendField(field);
	updateFieldLists(field, fieldId);
//	if(referenceToMaplist) {
		//TODO
//	}
}

void FieldArchive::appendField(Field *field)
{
	fileList.append(field);
}

void FieldArchive::removeField(quint32 id)
{
	fileList.removeAt(id);
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

	QFile deb(QString("savesP%1.txt").arg(isPC() ? "C" : "S"));
	deb.open(QIODevice::WriteOnly | QIODevice::Text/* | QIODevice::Truncate*/);


//	for(int i=0 ; i<size ; ++i) {
	foreach(int i, fieldsSortByMapId) {
		Field *field = this->field(i, true);
		if(field != NULL) {
			bool modelFound = false;
			int modelFoundID = 0;
			FieldModelLoaderPC *modelLoader = (FieldModelLoaderPC *)field->fieldModelLoader();
			if(modelLoader->isOpen()) {
				foreach(const QString &HRC, modelLoader->HRCNames()) {
					if(HRC == "AVFE.HRC") {
						modelFound = true;
						break;
					}
					++modelFoundID;
				}
			}

			if(!modelFound) continue;

//			if(field->name() != "convil_2") continue;
			Section1File *s1 = field->scriptsAndTexts();
			int modelID = 0;
			foreach(GrpScript *group, s1->grpScripts()) {
				if(group->typeID() == GrpScript::Model) {
					if(modelID == modelFoundID) {
						if(group->size() > 0) {
							Script *script = group->script(0);
							QList<FF7Position> positions;
							foreach(Opcode *opcode, script->getOpcodes()) {
								opcode->listModelPositions(positions);
							}

							if(!positions.isEmpty()) {
								const FF7Position &pos = positions.first();
								deb.write(QString("%1,%2,%3,%4,%5,%6\n")
										  .arg(Data::field_names.indexOf(field->name())).arg(field->name())
										  .arg(pos.x).arg(pos.y).arg(pos.hasZ ? QString::number(pos.z) : "?").arg(pos.hasId ? QString::number(pos.id) : "?")
										  .toLatin1());
							}
						}
						break;
					}
					modelID++;
				}
			}
//			CaFile *ca = field->camera();
//			if(ca->isOpen()) {
//				if(field->isPS() && ca->cameraCount() == 1) {
//					deb.write(QString("%1\n").arg(ca->camera(0).unknown).toLatin1());
//				}
				/*if(ca->cameraCount() == 2) {
					Camera cam1 = ca->camera(0), cam2 = ca->camera(1);
					if(isPC()) {
						if(memcmp(&cam1, &cam2, sizeof(cam1)) != 0) {
							qWarning() << "different cam! " << field->name();
						}
					} else {
						if(memcmp(&cam1, &cam2, 18) != 0) {
							qWarning() << "different cam! " << field->name();
						}
					}
				}*/
//				int c=0;
//				for(; c<ca->cameraCount(); ++c) {
//					Camera cam = ca->camera(c);
//					deb.write(QString("%1: %2 -> (%3, %4, %5), (%6, %7, %8), (%9, %10, %11), %12, %13, %14, %15, %16\n")
//							  .arg(field->name()).arg(c)
//							  .arg(cam.camera_axis[0].x).arg(cam.camera_axis[0].y).arg(cam.camera_axis[0].z)
//							.arg(cam.camera_axis[1].x).arg(cam.camera_axis[1].y).arg(cam.camera_axis[1].z)
//							.arg(cam.camera_axis[2].x).arg(cam.camera_axis[2].y).arg(cam.camera_axis[2].z)
//							.arg(cam.camera_position[0]).arg(cam.camera_position[1]).arg(cam.camera_position[2])
//							.arg(cam.camera_zoom)
//							.arg(field->sectionData(Field::Camera).size()).toLatin1());
//				}
//				QFile deb2(QString("camera/%1-cameraP%2.bin")
//						   .arg(field->name())
//						   .arg(isPC() ? "C" : "S"));
//				deb2.open(QIODevice::WriteOnly | QIODevice::Truncate);
//				deb2.write(field->sectionData(Field::Camera));
//				deb2.close();
//			}

			/*IdFile *id = field->walkmesh();
			if(id->isOpen()) {
				deb.write(QString("%1:\n").arg(field->name()).toLatin1());
				int triangleID = 0;
				foreach(const Triangle &triangle, id->triangles()) {
					const Access &a = id->access(triangleID++);
					for(int jj=0; jj<3; ++jj) {
						deb.write(QString("(x=%1, y=%2, z=%3)+a=%4 ")
								.arg(triangle.vertices[jj].x)
								.arg(triangle.vertices[jj].y)
								.arg(triangle.vertices[jj].z)
								  .arg(a.a[jj]).toLatin1());
					}
					deb.write("\n");
				}
			}*/


			/*EncounterFile *e = field->encounter();
			EncounterTable tables[2];
			tables[0] = e->encounterTable(EncounterFile::Table1);
			tables[1] = e->encounterTable(EncounterFile::Table2);
//			for(int tableId=0; tableId<2; tableId++) {
//				EncounterTable t = tables[tableId];
				deb.write(QString("%1\t%2\t%3\n")//, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, (%15)
						  .arg(field->name())
						  .arg(255 - tables[0].rate)
						  //.arg(t.enabled)
						  .arg(255 - tables[1].rate)
						.arg(t.enc_standard[0])
						.arg(t.enc_standard[1])
						.arg(t.enc_standard[2])
						.arg(t.enc_standard[3])
						.arg(t.enc_standard[4])
						.arg(t.enc_standard[5])
						.arg(t.enc_special[0])
						.arg(t.enc_special[1])
						.arg(t.enc_special[2])
						.arg(t.enc_special[3])
						.arg(t._pad).toLatin1());
//			}*/

			/*
			 *for(int i=0; i<tut->size(); ++i) {
					const QByteArray &data = tut->data(i);
					quint8 id;
					if(data.size() < 6) {
						id = -1;
					} else {
						memcpy(&id, data.constData() + 4, 1);
					}
					deb.write(QString("%1: %2 -> %3 | %4\n").arg(field->name())
							  .arg(i).arg(id).arg(QString(tut->data(i).toHex())).toLatin1());
				}*/

//			Data::charlgp_loadAnimBoneCount();
//			FieldModelLoader *modelLoader = (FieldModelLoader *)field->fieldModelLoader();
//			if(modelLoader->isOpen()) {
//				for(int i=0; i<modelLoader->modelCount(); ++i) {
//					field->fieldModel(i, 0, false);
//					deb.write(QString("%1: %2 -> %3\n").arg(field->name())
//							  .arg(i).arg(modelLoader->unknown(i)).toLatin1());

					/*int boneCount = field->fieldModel(i)->boneCount();
					foreach(const QString &animation, modelLoader->ANames(i)) {
						QString animName = animation.left(animation.lastIndexOf('.')).toLower() + ".a";
						if(boneCount != Data::charlgp_animBoneCount.value(animName) &&
								!(boneCount == 1 && Data::charlgp_animBoneCount.value(animName) == 0)) {
							qDebug() << boneCount << Data::charlgp_animBoneCount.value(animName) << field->name() << modelLoader->HRCName(i) << animation;
						}
					}*/
//				}
//			}
			/*TutFileStandard *tut = field->tutosAndSounds();
			if(tut->isOpen()) {
				deb.write(QString("=== %1 ===\n").arg(field->name()).toLatin1());
				for(int j=0; j<tut->size(); ++j) {
					if(!tut->isTut(j)) {
						deb.write(QString("id= %1\n").arg(tut->akaoID(j)).toLatin1());
					}
				}
			}*/
//			qDebug() << field->name();
//			int scriptID=0, opcodeID=0;
//			Section1File *scripts = field->scriptsAndTexts();
//			if(scripts) {
//				scripts->setModified(true);
//			}
//			field->setModified(true);
			/*foreach(GrpScript *group, scripts->grpScripts()) {
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

bool FieldArchive::compileScripts(int &fieldID, int &groupID, int &scriptID, int &opcodeID, QString &errorStr)
{
	int size = fileList.size();

	for(fieldID=0 ; fieldID<size ; ++fieldID) {
		QCoreApplication::processEvents();
		Field *field = this->field(fieldID, false);
		if(field != NULL && field->isOpen()) {
			Section1File *section1 = field->scriptsAndTexts();
			if(section1->isOpen() && !section1->compileScripts(groupID, scriptID, opcodeID, errorStr)) {
				return false;
			}
		}
	}

	return true;
}

bool FieldArchive::exportation(const QList<int> &selectedFields, const QString &directory,
							   bool overwrite, const QMap<ExportType, QString> &toExport,
							   FieldArchiveIOObserver *observer)
{
	if(selectedFields.isEmpty() || toExport.isEmpty()) {
		return true;
	}

	QString path, extension;
	int currentField=0;
	observer->setObserverMaximum(selectedFields.size()-1);

	foreach(const int &fieldID, selectedFields) {
		if(observer->observerWasCanceled()) 	break;
		Field *f = field(fieldID);
		if(f) {
			if(toExport.contains(Fields)) {
				extension = toExport.value(Fields);
				path = QDir::cleanPath(extension.isEmpty()
									   ? QString("%1/%2")
										 .arg(directory, f->name())
									   : QString("%1/%2.%3")
										 .arg(directory, f->name(), extension));
				if(overwrite || !QFile::exists(path)) {
					QByteArray fieldData = io()->fieldData(f, extension.compare("dec", Qt::CaseInsensitive) == 0);
					if(!fieldData.isEmpty()) {
						QFile fieldExport(path);
						if(fieldExport.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
							fieldExport.write(fieldData);
							fieldExport.close();
						}
					}
				}
			}
			if(toExport.contains(Backgrounds)) {
				extension = toExport.value(Backgrounds);
				path = QDir::cleanPath(QString("%1/%2.%3").arg(directory, f->name(), extension));

				if(overwrite || !QFile::exists(path)) {
					QImage background = f->background()->openBackground();
					if(!background.isNull())
						background.save(path);
				}
			}
			if(toExport.contains(Akaos)) {
				TutFileStandard *akaoList = f->tutosAndSounds();
				if(akaoList->isOpen()) {
					int akaoCount = akaoList->size();
					for(int i=0 ; i<akaoCount ; ++i) {
						if(!akaoList->isTut(i)) {
							extension = toExport.value(Akaos);
							path = QDir::cleanPath(QString("%1/%2-%3.%4").arg(directory, f->name()).arg(i).arg(extension));
							if(overwrite || !QFile::exists(path)) {
								QFile tutExport(path);
								if(tutExport.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
									tutExport.write(akaoList->data(i));
									tutExport.close();
								}
							}
						}
					}
				}
			}
			if(toExport.contains(Texts)) {
				qDebug() << f->name();
				Section1File *section1 = f->scriptsAndTexts();
				if(section1->isOpen()) {
					extension = toExport.value(Texts);
					path = QDir::cleanPath(QString("%1/%2.%3").arg(directory, f->name(), extension));
					if(overwrite || !QFile::exists(path)) {
						QFile textExport(path);
						Section1File::ExportFormat format;
						if(extension == "txt") {
							format = Section1File::TXTText;
						} else if(extension == "xml") {
							format = Section1File::XMLText;
						} else {
							return false;
						}

						if(!section1->exporter(&textExport, format)) {
							return false;
						}
					}
				}
			}
		}
		observer->setObserverValue(currentField++);
	}

	return true;
}

bool FieldArchive::importation(const QList<int> &selectedFields, const QString &directory,
							   const QMap<Field::FieldSection, QString> &toImport,
							   FieldArchiveIOObserver *observer)
{
	if(selectedFields.isEmpty() || toImport.isEmpty()) {
		return true;
	}
	int currentField=0;

	foreach(const int &fieldID, selectedFields) {
		if(observer->observerWasCanceled()) 	break;

		Field *f = field(fieldID);
		if(f) {
			if(toImport.contains(Field::Scripts)) {
				Section1File *section1 = f->scriptsAndTexts();
				if(section1->isOpen()) {
					//TODO
				}
			}
		}
		observer->setObserverValue(currentField++);
	}

	return true;
}

void FieldArchive::setSaved()
{
	foreach(Field *field, fileList) {
		field->setSaved();
	}
}
