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

FieldArchiveIterator::FieldArchiveIterator(const FieldArchive &archive) :
	QListIterator<Field *>(archive.fileList)
{
}

Field *FieldArchiveIterator::next(bool open, bool dontOptimize)
{
	return openField(QListIterator<Field *>::next(), open, dontOptimize);
}

Field *FieldArchiveIterator::peekNext(bool open, bool dontOptimize) const
{
	return openField(QListIterator<Field *>::peekNext(), open, dontOptimize);
}

Field *FieldArchiveIterator::peekPrevious(bool open, bool dontOptimize) const
{
	return openField(QListIterator<Field *>::peekPrevious(), open, dontOptimize);
}

Field *FieldArchiveIterator::previous(bool open, bool dontOptimize)
{
	return openField(QListIterator<Field *>::previous(), open, dontOptimize);
}

Field *FieldArchiveIterator::openField(Field *field, bool open, bool dontOptimize) const
{
	if(field != NULL && open &&
			!field->isOpen() && !field->open(dontOptimize)) {
		return NULL;
	}
	return field;
}

FieldArchive::FieldArchive() :
	_io(0), _observer(0)
{
}

FieldArchive::FieldArchive(FieldArchiveIO *io) :
	_io(io), _observer(0)
{
	//	fileWatcher.addPath(path);
	//	connect(&fileWatcher, SIGNAL(fileChanged(QString)), this, SIGNAL(fileChanged(QString)));
	//	connect(&fileWatcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(directoryChanged(QString)));
}

FieldArchive::~FieldArchive()
{
	qDeleteAll(fileList);
	if(_io)		delete _io;
}

FieldArchiveIO::ErrorCode FieldArchive::open()
{
	if(!_io)	return FieldArchiveIO::Invalid;
//	qDebug() << "FieldArchive::open()";
	clear();

	FieldArchiveIO::ErrorCode error = _io->open(observer());
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

FieldArchiveIO::ErrorCode FieldArchive::save(const QString &path)
{
	if(!_io)	return FieldArchiveIO::Invalid;

	FieldArchiveIO::ErrorCode error = _io->save(path, observer());
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
	qDeleteAll(fileList);
	fileList.clear();
	fieldsSortByName.clear();
	fieldsSortByMapId.clear();
	Data::field_names.clear();
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

	int index = Data::field_names.indexOf(name);
	QString mapId = index != -1 ?
				QString("%1").arg(index, 3) :
				"~";

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

bool FieldArchive::isAllOpened() const
{
	foreach(Field *f, fileList) {
		if(!f->isOpen())	return false;
	}
	return true;
}

bool FieldArchive::isModified() const
{
	foreach(Field *f, fileList) {
		if(f->isModified())	return true;
	}
	return false;
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

#ifdef DEBUG_FUNCTIONS

#include "BackgroundFilePC.h"
#include "BackgroundFilePS.h"
#include "FieldArchivePC.h"

void FieldArchive::validateAsk()
{
	foreach(int i, fieldsSortByMapId) {
		Field *f = field(i, true);
		QString name = Data::field_names.value(fieldsSortByMapId.key(i).toInt());
		if(f == NULL) {
			qWarning() << "FieldArchive::printAkaos: cannot open field" << i << name;
			continue;
		}

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if(scriptsAndTexts->isOpen()) {
			//qWarning() << f->name();

			int grpScriptID = 0;
			foreach(GrpScript *grp, scriptsAndTexts->grpScripts()) {
				int scriptID = 0;
				foreach(Script *script, grp->scripts()) {
					int opcodeID = 0;
					foreach(Opcode *opcode, script->opcodes()) {
						if(opcode->id() == Opcode::ASK) {
							OpcodeASK *opcodeASK = (OpcodeASK *)opcode;
							quint8 textID = opcodeASK->textID,
									firstLine = opcodeASK->firstLine,
									lastLine = opcodeASK->lastLine;
							if (textID < scriptsAndTexts->textCount()) {
								QString text = scriptsAndTexts->text(textID).text(false);
								if (!text.isEmpty()) {
									int lineNum = 0, autoFirstLine = -1, autoLastLine = -1;
									bool hasChoice = false;
									foreach (const QString &line, text.split('\n')) {
										if (line.startsWith("{CHOICE}")) {
											if (!hasChoice) {
												if (autoFirstLine == -1) {
													autoFirstLine = lineNum;
												} else {
													qWarning() << "Multi choices!?";
												}
											}
											autoLastLine = lineNum;
											hasChoice = true;
										} else if (line.startsWith("{NEW PAGE")) {
											lineNum = -1;
											if (autoFirstLine != -1) {
												qWarning() << "Choice before new page!?";
											}
											hasChoice = false;
										} else {
											hasChoice = false;
										}
										lineNum++;
									}

									if (autoFirstLine == -1) {
										qWarning() << QString("%1 > %2 > %3 > %4: No {CHOICE} here (textID: %9)")
													  .arg(f->name(), grp->name(), grp->scriptName(scriptID))
													  .arg(opcodeID + 1)
													  .arg(textID).toLatin1().data();
									} else if (autoFirstLine != firstLine
											|| autoLastLine != lastLine) {
										qWarning() << QString("%1 > %2 > %3 > %4: %5 should be %6, %7 should be %8 (textID: %9)")
													  .arg(f->name(), grp->name(), grp->scriptName(scriptID))
													  .arg(opcodeID + 1)
													  .arg(firstLine).arg(autoFirstLine)
													  .arg(lastLine).arg(autoLastLine)
													  .arg(textID).toLatin1().data();
									}
								}
							} else {
								qWarning() << "FATAL: textID overflow";
							}
						}
						opcodeID++;
					}
					scriptID++;
				}
				grpScriptID++;
			}
		}
	}
}

void FieldArchive::printAkaos()
{
	QFile deb(QString("akaos-18-04-2014-P%1.txt").arg(isPC() ? "C" : "S"));
	deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	foreach(int i, fieldsSortByMapId) {
		Field *f = field(i, true);
		QString name = Data::field_names.value(fieldsSortByMapId.key(i).toInt());
		if(f == NULL) {
			qWarning() << "FieldArchive::printAkaos: cannot open field" << i << name;
			continue;
		}

		TutFileStandard *tutosAndSounds = f->tutosAndSounds();
		if(!tutosAndSounds->isOpen()) {
			qWarning() << "FieldArchive::printAkaos: cannot open tutos and sounds" << name;
		}

		for(int akaoID=0; akaoID < tutosAndSounds->size(); ++akaoID) {
			if(tutosAndSounds->isTut(akaoID)) {
				deb.write(QString("%1 > tuto %2\n")
						  .arg(name)
						  .arg(akaoID)
						  .toLatin1());
			} else {
				deb.write(QString("%1 > akao %2: %3\n")
						  .arg(name)
						  .arg(akaoID)
						  .arg(tutosAndSounds->akaoID(akaoID))
						  .toLatin1());
			}
		}
	}
}

void FieldArchive::printModelLoaders()
{
	QFile deb(QString("model-loader-18-04-2014-P%1.txt").arg(isPC() ? "C" : "S"));
	deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	foreach(int i, fieldsSortByMapId) {
		Field *f = field(i, true);
		QString name = Data::field_names.value(fieldsSortByMapId.key(i).toInt());
		if(f == NULL) {
			qWarning() << "FieldArchive::printAkaos: cannot open field" << i << name;
			continue;
		}

		FieldModelLoader *modelLoader = f->fieldModelLoader();
		if(!modelLoader->isOpen()) {
			qWarning() << "FieldArchive::printAkaos: cannot open tutos and sounds" << name;
		}

		if(f->isPC()) {
			FieldModelLoaderPC *modelLoaderPC = (FieldModelLoaderPC *)modelLoader;

			int modelID = 0;
			foreach(const QString &hrc, modelLoaderPC->HRCNames()) {
				deb.write(QString("%1 > model %2: %3\n")
						  .arg(name)
						  .arg(modelID)
						  .arg(hrc.toLower())
						  .toLatin1());

				int animID = 0;
				foreach(const QString &a, modelLoaderPC->ANames(modelID)) {
					deb.write(QString("\tanim %1: %2\n")
							  .arg(animID)
							  .arg(a.toLower().left(a.lastIndexOf('.')))
							  .toLatin1());

					++animID;
				}
				++modelID;
			}
		}
	}
}

void FieldArchive::printScripts()
{
	QFile deb(QString("scripts-window-coord-17-05-2014-P%1.txt").arg(isPC() ? "C" : "S"));
	deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	foreach(int i, fieldsSortByMapId) {
		Field *f = field(i, true);
		QString name = Data::field_names.value(fieldsSortByMapId.key(i).toInt());
		if(f == NULL) {
			qWarning() << "FieldArchive::printAkaos: cannot open field" << i << name;
			continue;
		}

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if(scriptsAndTexts->isOpen()) {
			qWarning() << f->name();

			int grpScriptID = 0;
			foreach(GrpScript *grp, scriptsAndTexts->grpScripts()) {
				int scriptID = 0;
				foreach(Script *script, grp->scripts()) {
					int opcodeID = 0;
					foreach(Opcode *opcode, script->opcodes()) {
						FF7Window win;
						if(opcode->getWindow(win)) {
							deb.write(QString("%1 > %2 > %3: %4 %5\n")
									  .arg(f->name(), grp->name())
									  .arg(scriptID)
									  .arg(win.x).arg(win.y).toLatin1());
						}
						opcodeID++;
					}
					scriptID++;
				}
				grpScriptID++;
			}
		}
	}
}

void FieldArchive::diffScripts()
{
	FieldArchivePC original("C:/Program Files/Square Soft, Inc/Final Fantasy VII/data/field/fflevel - original.lgp", FieldArchiveIO::Lgp);
	if(original.open() != FieldArchiveIO::Ok) {
		qWarning() << "error opening pc lgp";
		return;
	}

	QSet<QString> screens;

	foreach(int i, fieldsSortByMapId) {
		Field *field = this->field(i, true);
		Field *fieldOriginal = original.field(original.indexOfField(field->name()));
		if(fieldOriginal == NULL) {
			qWarning() << "ALERT field not found" << field->name();
			break;
		}
		if(field != NULL) {
			Section1File *scriptsAndTexts = field->scriptsAndTexts();
			Section1File *scriptsAndTextsOriginal = fieldOriginal->scriptsAndTexts();
			if(scriptsAndTexts->isOpen() && scriptsAndTextsOriginal->isOpen()) {
				qWarning() << field->name();

				if(scriptsAndTexts->grpScriptCount() < scriptsAndTextsOriginal->grpScriptCount()) {
					qWarning() << "ALERT much grpscript wow";
					break;
				}

				QFile deb(QString("scripts-window-coord-18-05-2014/%1.txt").arg(field->name()));
				deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

				int grpScriptID = 0, scriptID;
				foreach(GrpScript *grp2, scriptsAndTextsOriginal->grpScripts()) {
					GrpScript *grp = scriptsAndTexts->grpScript(grpScriptID);

					while(grp2->name() != grp->name()) {
						grpScriptID++;
						if(grpScriptID >= scriptsAndTexts->grpScriptCount()) {
							qWarning() << "ALERT end grpScript reached";
							goto next_group;
						}
						grp = scriptsAndTexts->grpScript(grpScriptID);
					}

					if(grp->size() != grp2->size()) {
						qWarning() << "ALERT wrong group size" << grp->size() << grp2->size();
						break;
					}

					scriptID = 0;
					foreach(Script *script2, grp2->scripts()) {
						Script *script = grp->script(scriptID);

						int opcodeID = 0;
						foreach(Opcode *opcode2, script2->opcodes()) {
							FF7Window win, win2;

							if(opcode2->getWindow(win2)) {

								Opcode *opcode;

								do {
									if(opcodeID >= script->size()) {
										qWarning() << "ALERT wrong script size";
										goto next_script;
									}

									opcode = script->opcode(opcodeID++);

								} while (!opcode->getWindow(win));

								const quint8 minDiff = 1, maxDiff = 5;
								int diffX = qAbs(win2.x - win.x), diffY = qAbs(win2.y - win.y);
								bool hasTinyDiffX = diffX >= minDiff && diffX <= maxDiff,
										hasTinyDiffY = diffY >= minDiff && diffY <= maxDiff,
										hasLargeDiffX = diffX > 15,//maxDiff,
										hasLargeDiffY = diffY > 15;//maxDiff;
								if(diffX > 0 || diffY > 0) { //(hasTinyDiffX && hasLargeDiffY) || (hasTinyDiffY && hasLargeDiffX)) {
									deb.write(QString("%1 > %2 > %3 > %4: window %5 (retraduit: %6, %7) (original: %8, %9)\n")
											  .arg(field->name())
											  .arg(grp->name())
											  .arg(grp->scriptName(scriptID))
											  .arg(opcodeID + 1)
											  .arg(opcode2->getWindowID())
											  .arg(win.x)
											  .arg(win.y)
											  .arg(win2.x)
											  .arg(win2.y).toLatin1());
									screens.insert(field->name());
								}
							}
						}
						next_script:;
						scriptID++;
					}
					next_group:;
					grpScriptID++;
				}
				if(deb.size() == 0) {
					deb.remove();
				}
				qDebug() << screens.size() << "écrans";
			}
		}
	}
}

void FieldArchive::searchAll()
{
	QTime t;t.start();

	QFile deb(QString("scripts-window-coord-18-05-2014-P%1.txt").arg(isPC() ? "C" : "S"));
	deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	FieldArchivePC original("C:/Program Files/Square Soft, Inc/Final Fantasy VII/data/field/fflevel - original.lgp", FieldArchiveIO::Lgp);
	if(original.open() != FieldArchiveIO::Ok) {
		qWarning() << "error opening pc lgp";
		return;
	}

	/*FieldArchivePS ps("C:/Users/Jérôme/Games/Final Fantasy VII-PSX-PAL-FR-CD1.bin", FieldArchiveIO::Iso);
	if(ps.open() != FieldArchiveIO::Ok) {
		qWarning() << "error opening ps iso";
		return;
	}

	if(!isPC()) {
		return;
	}

	Field *f1 = this->field(fieldsSortByName.value("yougan"), true);
	if(f1) {
		Section1File *f1s1 = f1->scriptsAndTexts();
		if(f1s1->isOpen()) {
			int groupScriptCount = f1s1->grpScriptCount();

			for (int groupScriptID=0; groupScriptID < groupScriptCount; groupScriptID++) {
				GrpScript *group1 = f1s1->grpScript(groupScriptID);
				int scriptCount = group1->size();

				deb.write(QString("--- groupe '%1' (%2) ---\n").arg(group1->name()).arg(groupScriptID).toLatin1());

				for (int scriptID=0; scriptID < scriptCount; scriptID++) {
					Script *script1 = group1->script(scriptID);
					int opcodeCount = script1->size();

					deb.write(QString("--- script %1 ---\n").arg(scriptID).toLatin1());

					for (int opcodeID=0; opcodeID < opcodeCount; opcodeID++) {
						Opcode *opcode1 = script1->opcode(opcodeID);

						deb.write(opcode1->toString(f1).toLatin1() + "\n");
					}
				}
			}
		}
	}*/

	/*QString dest = "scripts-PC-acro-v09-03-2014";
	QDir tmp;
	if(!QFile::exists(tmp.absoluteFilePath(dest))) {
		tmp.mkdir(dest);
	}
	tmp.cd(dest);*/
	QSet<QString> screens;
//	for(int i=0 ; i<size ; ++i) {
	foreach(int i, fieldsSortByMapId) {
		Field *field = this->field(i, true);
		Field *fieldOriginal = original.field(original.indexOfField(field->name()));
		if(fieldOriginal == NULL) {
			qWarning() << "ALERT field not found" << field->name();
			break;
		}
		if(field != NULL) {
			Section1File *scriptsAndTexts = field->scriptsAndTexts();
			Section1File *scriptsAndTextsOriginal = fieldOriginal->scriptsAndTexts();
			if(scriptsAndTexts->isOpen() && scriptsAndTextsOriginal->isOpen()) {
				qWarning() << field->name();

				if(scriptsAndTexts->grpScriptCount() < scriptsAndTextsOriginal->grpScriptCount()) {
					qWarning() << "ALERT much grpscript wow";
					break;
				}

				int grpScriptID = 0, scriptID;
				foreach(GrpScript *grp2, scriptsAndTextsOriginal->grpScripts()) {
					GrpScript *grp = scriptsAndTexts->grpScript(grpScriptID);

					while(grp2->name() != grp->name()) {
						grpScriptID++;
						if(grpScriptID >= scriptsAndTexts->grpScriptCount()) {
							qWarning() << "ALERT end grpScript reached";
							goto next_group;
						}
						grp = scriptsAndTexts->grpScript(grpScriptID);
					}

					if(grp->size() != grp2->size()) {
						qWarning() << "ALERT wrong group size" << grp->size() << grp2->size();
						break;
					}

					scriptID = 0;
					foreach(Script *script2, grp2->scripts()) {
						Script *script = grp->script(scriptID);

						int opcodeID = 0;
						foreach(Opcode *opcode2, script2->opcodes()) {
							FF7Window win, win2;

							if(opcode2->getWindow(win2)) {

								Opcode *opcode;

								do {
									if(opcodeID >= script->size()) {
										qWarning() << "ALERT wrong script size";
										goto next_script;
									}

									opcode = script->opcode(opcodeID++);

								} while (!opcode->getWindow(win));

								const quint8 minDiff = 1, maxDiff = 5;
								int diffX = qAbs(win2.x - win.x), diffY = qAbs(win2.y - win.y);
								bool hasTinyDiffX = diffX >= minDiff && diffX <= maxDiff,
										hasTinyDiffY = diffY >= minDiff && diffY <= maxDiff,
										hasLargeDiffX = diffX > 15,//maxDiff,
										hasLargeDiffY = diffY > 15;//maxDiff;
								if((hasTinyDiffX && hasLargeDiffY) || (hasTinyDiffY && hasLargeDiffX)) {
									deb.write(QString("%1 > %2 > %3: window %4 (retraduit: %5, %6) (original: %7, %8)\n")
											  .arg(field->name())
											  .arg(grp->name())
											  .arg(scriptID)
											  .arg(opcode2->getWindowID())
											  .arg(win.x)
											  .arg(win.y)
											  .arg(win2.x)
											  .arg(win2.y).toLatin1());
									screens.insert(field->name());
								}
							}
						}
						next_script:;
						scriptID++;
					}
					next_group:;
					grpScriptID++;
				}
				qDebug() << screens.size() << "écrans";
				/*QString fieldDir = QString("%1-%2").arg(Data::field_names.indexOf(field->name()), 3, 10, QChar('0')).arg(field->name());
				if(!tmp.mkdir(fieldDir)) {
					qWarning() << "cannot create dir" << tmp.absoluteFilePath(fieldDir);
				}
				tmp.cd(fieldDir);

				scriptsAndTexts->clearTexts();
				int grpScriptID = 0;
				foreach(GrpScript *grp, scriptsAndTexts->grpScripts()) {
					QString grpFilename = QString("%1-%2").arg(grpScriptID).arg(grp->name());
					QFile out(tmp.absoluteFilePath(grpFilename));
					if(out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
						out.write(grp->toString(field).toLatin1());
						out.close();
					}
					grpScriptID++;
				}

				tmp.cdUp();*/
				/*QFile out(tmp.absoluteFilePath(fieldDir));
				if(out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {

					int textID = 0;
					foreach(const FF7Text &text, scriptsAndTexts->texts()) {
						out.write(QString("=== Texte %1 ===\n").arg(textID++).toLatin1());
						out.write(text.text(false).toLatin1());
						out.write("\n");
					}

					out.close();
				}*/
			}
			/*FieldPC *fieldPC = (FieldPC *)field;
			FieldModelLoaderPC *modelLoader = fieldPC->fieldModelLoader();
			if(modelLoader->isOpen()) {
				int modelID = 0;
				foreach(const QString &HRC, modelLoader->HRCNames()) {
					foreach(const QString &a, modelLoader->ANames(modelID)) {
						FieldModelFilePC *fieldModel = fieldPC->fieldModel(HRC, a);
						if(fieldModel->isOpen()) {
							if(fieldModel->animBoneCount() > 1 && fieldModel->animBoneCount() != fieldModel->boneCount()) {
								qDebug() << field->name() << modelID << HRC << a << fieldModel->animBoneCount() << fieldModel->boneCount();
							}
						} else {
							qWarning() << "error" << field->name() << modelID << HRC << a;
						}
					}

					modelID++;
				}
			} else {
				qWarning() << "Error opening modelLoader" << field->name();
			}*/

		}
	}
}

#endif

bool FieldArchive::searchIterators(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &end, int fieldID, Sorting sorting, SearchScope scope) const
{
	if(fieldID >= fileList.size())		return false;

	switch(sorting) {
	case SortByName:
		i = fieldsSortByName.constFind(fieldsSortByName.key(fieldID), fieldID);
		end = fieldsSortByName.constEnd();
		if(i == end) {
			i = fieldsSortByName.constBegin();
		}
		break;
	case SortByMapId:
		i = fieldsSortByMapId.constFind(fieldsSortByMapId.key(fieldID), fieldID);
		end = fieldsSortByMapId.constEnd();
		if(i == end) {
			i = fieldsSortByMapId.constBegin();
		}
		break;
	}

	if(scope >= FieldScope && fieldID != i.value()) {
		return false;
	}

	return true;
}

bool FieldArchive::find(bool (*predicate)(Field *, SearchQuery *, SearchIn *),
						SearchQuery *toSearch, int &fieldID, SearchIn *searchIn,
						Sorting sorting, SearchScope scope)
{
	QMap<QString, int>::const_iterator i, end;
	if(!searchIterators(i, end, fieldID, sorting, scope))	return false;

	for( ; i != end ; ++i) {
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && (*predicate)(f, toSearch, searchIn))
			return true;
		searchIn->reset();
		if(scope >= FieldScope)		break;
	}
	return false;
}

bool FieldArchive::findLast(bool (*predicate)(Field *, SearchQuery *, SearchIn *),
						SearchQuery *toSearch, int &fieldID, SearchIn *searchIn,
						Sorting sorting, SearchScope scope)
{
	QMap<QString, int>::const_iterator i, begin;
	if(!searchIteratorsP(i, begin, fieldID, sorting, scope))	return false;

	for( ; i != begin-1 ; --i)
	{
		QCoreApplication::processEvents();
		Field *f = field(fieldID = i.value());
		if(f!=NULL && (*predicate)(f, toSearch, searchIn))
			return true;
		searchIn->toEnd();
		if(scope >= FieldScope)		break;
	}
	return false;
}

bool FieldArchive::searchOpcode(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchOpcodeQuery query(opcode);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchOpcodeQuery *query = (SearchOpcodeQuery *)_query;
		SearchInScript *searchIn = (SearchInScript *)_searchIn;
		return f->scriptsAndTexts()->searchOpcode(query->opcode, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchVar(quint8 bank, quint8 adress, Opcode::Operator op, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchVarQuery query(bank, adress, op, value);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchVarQuery *query = (SearchVarQuery *)_query;
		SearchInScript *searchIn = (SearchInScript *)_searchIn;
		return f->scriptsAndTexts()->searchVar(query->bank, query->adress, query->op, query->value, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchExec(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchExecQuery query(group, script);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchExecQuery *query = (SearchExecQuery *)_query;
		SearchInScript *searchIn = (SearchInScript *)_searchIn;
		return f->scriptsAndTexts()->searchExec(query->group, query->script, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchMapJump(quint16 _field, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchFieldQuery query(_field);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchFieldQuery *query = (SearchFieldQuery *)_query;
		SearchInScript *searchIn = (SearchInScript *)_searchIn;
		return f->scriptsAndTexts()->searchMapJump(query->fieldID, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchTextInScripts(const QRegExp &text, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = (SearchTextQuery *)_query;
		SearchInScript *searchIn = (SearchInScript *)_searchIn;
		return f->scriptsAndTexts()->searchTextInScripts(query->text, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchText(const QRegExp &text, int &fieldID, int &textID, int &from, int &size, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	int empty;
	SearchInText searchIn(textID, from, size, empty);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = (SearchTextQuery *)_query;
		SearchInText *searchIn = (SearchInText *)_searchIn;
		return f->scriptsAndTexts()->searchText(query->text, searchIn->textID, searchIn->from, searchIn->size);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchIteratorsP(QMap<QString, int>::const_iterator &i, QMap<QString, int>::const_iterator &begin, int fieldID, Sorting sorting, SearchScope scope) const
{
	if(fieldID < 0)		return false;

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

	if(scope >= FieldScope && fieldID != i.value()) {
		return false;
	}

	return true;
}

bool FieldArchive::searchOpcodeP(int opcode, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchOpcodeQuery query(opcode);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchOpcodeQuery *query = (SearchOpcodeQuery *)_query;
		SearchInScript *searchIn = (SearchInScript *)_searchIn;
		return f->scriptsAndTexts()->searchOpcodeP(query->opcode, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchVarP(quint8 bank, quint8 adress, Opcode::Operator op, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchVarQuery query(bank, adress, op, value);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchVarQuery *query = (SearchVarQuery *)_query;
		SearchInScript *searchIn = (SearchInScript *)_searchIn;
		return f->scriptsAndTexts()->searchVarP(query->bank, query->adress, query->op, query->value, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchExecP(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchExecQuery query(group, script);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchExecQuery *query = (SearchExecQuery *)_query;
		SearchInScript *searchIn = (SearchInScript *)_searchIn;
		return f->scriptsAndTexts()->searchExecP(query->group, query->script, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchMapJumpP(quint16 _field, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchFieldQuery query(_field);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchFieldQuery *query = (SearchFieldQuery *)_query;
		SearchInScript *searchIn = (SearchInScript *)_searchIn;
		return f->scriptsAndTexts()->searchMapJumpP(query->fieldID, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchTextInScriptsP(const QRegExp &text, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = (SearchTextQuery *)_query;
		SearchInScript *searchIn = (SearchInScript *)_searchIn;
		return f->scriptsAndTexts()->searchTextInScriptsP(query->text, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchTextP(const QRegExp &text, int &fieldID, int &textID, int &from, int &index, int &size, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	SearchInText searchIn(textID, from, size, index);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = (SearchTextQuery *)_query;
		SearchInText *searchIn = (SearchInText *)_searchIn;
		return f->scriptsAndTexts()->searchTextP(query->text, searchIn->textID, searchIn->from, searchIn->index, searchIn->size);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::replaceText(const QRegExp &search, const QString &after, int fieldID, int textID, int from)
{
	if(textID > -1) {
		Field *field = this->field(fieldID);
		if(field) {
			Section1File *texts = field->scriptsAndTexts();
			if(texts->isOpen() && textID < texts->textCount()) {
				if(texts->replaceText(search, after, textID, from)) {
					return true;
				}
			}
		}
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

void FieldArchive::removeBattles()
{
	observer()->setObserverMaximum(fileList.size());

	for(int fieldID=0 ; fieldID<fileList.size() ; ++fieldID) {
		if(observer()->observerWasCanceled()) {
			return;
		}
		Field *field = this->field(fieldID, true);
		if(field != NULL && field->scriptsAndTexts()->isOpen()) {
			field->encounter()->setBattleEnabled(EncounterFile::Table1, false);
			field->encounter()->setBattleEnabled(EncounterFile::Table2, false);
			if(!field->isModified()) {
				field->setModified(true);
			}
		}
		observer()->setObserverValue(fieldID);
	}
}

void FieldArchive::removeTexts()
{
	observer()->setObserverMaximum(fileList.size());

	for(int fieldID=0 ; fieldID<fileList.size() ; ++fieldID) {
		if(observer()->observerWasCanceled()) {
			return;
		}
		Field *field = this->field(fieldID, true);
		if(field != NULL && field->scriptsAndTexts()->isOpen()) {
			field->scriptsAndTexts()->removeTexts();
			if(field->scriptsAndTexts()->isModified() && !field->isModified()) {
				field->setModified(true);
			}
		}
		observer()->setObserverValue(fieldID);
	}
}

void FieldArchive::cleanTexts()
{
	observer()->setObserverMaximum(fileList.size());

	for(int fieldID=0 ; fieldID<fileList.size() ; ++fieldID) {
		if(observer()->observerWasCanceled()) {
			return;
		}
		Field *field = this->field(fieldID, true);
		if(field != NULL && field->scriptsAndTexts()->isOpen()) {
			field->scriptsAndTexts()->cleanTexts();
			if(field->scriptsAndTexts()->isModified() && !field->isModified()) {
				field->setModified(true);
			}
		}
		observer()->setObserverValue(fieldID);
	}
}

bool FieldArchive::exportation(const QList<int> &selectedFields, const QString &directory,
							   bool overwrite, const QMap<ExportType, QString> &toExport)
{
	if(selectedFields.isEmpty() || toExport.isEmpty()) {
		return true;
	}

	QString path, extension;
	int currentField=0;
	observer()->setObserverMaximum(selectedFields.size()-1);

	foreach(const int &fieldID, selectedFields) {
		if(observer()->observerWasCanceled()) 	break;
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
		observer()->setObserverValue(currentField++);
	}

	return true;
}

bool FieldArchive::importation(const QList<int> &selectedFields, const QString &directory,
							   const QMap<Field::FieldSection, QString> &toImport)
{
	Q_UNUSED(directory) //TODO

	if(selectedFields.isEmpty() || toImport.isEmpty()) {
		return true;
	}
	int currentField=0;

	foreach(const int &fieldID, selectedFields) {
		if(observer()->observerWasCanceled()) 	break;

		Field *f = field(fieldID);
		if(f) {
			if(toImport.contains(Field::Scripts)) {
				Section1File *section1 = f->scriptsAndTexts();
				if(section1->isOpen()) {
					//TODO
				}
			}
		}
		observer()->setObserverValue(currentField++);
	}

	return true;
}

void FieldArchive::setSaved()
{
	foreach(Field *field, fileList) {
		field->setSaved();
	}
}
