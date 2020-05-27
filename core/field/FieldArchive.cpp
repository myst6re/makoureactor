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
#include "core/PsfFile.h"
#include "Parameters.h"

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

Field *FieldArchiveIterator::openField(Field *field, bool open, bool dontOptimize)
{
	if(field != NULL && open && !field->isOpen()) {
		field->open(dontOptimize);
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
	return fieldsSortByName.value(name.toLower(), -1);
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

const Field *FieldArchive::field(const QString &name) const
{
	return field(indexOfField(name));
}

Field *FieldArchive::field(const QString &name, bool open, bool dontOptimize)
{
	return field(indexOfField(name), open, dontOptimize);
}

void FieldArchive::updateFieldLists(Field *field, int fieldID)
{
	const QString &name = field->name();

	int index = Data::field_names.indexOf(name);
	QString mapId = index != -1 ?
				QString("%1").arg(index, 3) :
				"~";

	fieldsSortByName.insert(name.toLower(), fieldID);
	fieldsSortByMapId.insert(mapId, fieldID);
}

void FieldArchive::addNewField(Field *field, int &fieldId)
{
	fieldId = fileList.size();
	appendField(field);
	Data::field_names.append(field->name());
	updateFieldLists(field, fieldId);
}

void FieldArchive::delField(quint32 id)
{
	Field *field = fileList.at(id);

	for (int i = 0; i < Data::field_names.size(); ++i) {
		if (Data::field_names.at(i) == field->name()) {
			Data::field_names[i] = "";
		}
	}

	fileList[id] = nullptr;
	delete field;
}

void FieldArchive::appendField(Field *field)
{
	fileList.append(field);
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

QList<FF7Var> FieldArchive::searchAllVars(QMap<FF7Var, QSet<QString> > &fieldNames)
{
	Q_UNUSED(fieldNames)
	QList<FF7Var> vars;
	int size = fileList.size();

	for(int i=0 ; i<size ; ++i) {
		QCoreApplication::processEvents();
		Field *field = this->field(i);
		if(field != NULL) {
			QList<FF7Var> fieldVars;
			field->scriptsAndTexts()->searchAllVars(fieldVars);
			foreach (const FF7Var &fieldVar, fieldVars) {
				QSet<QString> names = fieldNames.value(fieldVar);
				names.insert(field->scriptsAndTexts()->author());
				fieldNames.insert(fieldVar, names);
			}

			vars.append(fieldVars);
		}
	}

	return vars;
}

#ifdef DEBUG_FUNCTIONS

#include "BackgroundFilePC.h"
#include "BackgroundFilePS.h"
#include "FieldArchivePC.h"
#include "widgets/TextPreview.h"

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
							OpcodeASK *opcodeASK = static_cast<OpcodeASK *>(opcode);
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

void FieldArchive::validateOneLineSize()
{
	TextPreview tp; // Init textpreview to use autodim algo

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
					OpcodeWindow *opcodeWindow = 0;
					foreach(Opcode *opcode, script->opcodes()) {
						if(opcode->id() == Opcode::WSIZW
								|| opcode->id() == Opcode::WINDOW) {
							opcodeWindow = static_cast<OpcodeWindow *>(opcode);
						} else if(opcode->getTextID() >= 0 && opcodeWindow) {
							FF7Window window;
							opcodeWindow->getWindow(window);
							if (opcode->getTextID() < scriptsAndTexts->textCount()) {
								FF7Text text = scriptsAndTexts->text(opcode->getTextID());
								QSize optimSize = TextPreview::calcSize(text.data());
								if (!text.data().isEmpty() && !text.contains(QRegExp("\n")) && (window.w != optimSize.width() || window.h != optimSize.height())) {
									qWarning() << name << grpScriptID << grp->name() << grp->scriptName(scriptID) << opcodeID << "width=" << window.w << "height=" << window.h << "better size=" << optimSize.width() << optimSize.height();
								}
							} else {
								qWarning() << name << grpScriptID << grp->name() << grp->scriptName(scriptID) << opcodeID << "text not found";
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

void FieldArchive::extractAkaos(const QString &dirname)
{
	QDir d(dirname);

	if (!d.exists()) {
		d.mkpath(d.absolutePath());
	}

	foreach(int i, fieldsSortByMapId) {
		Field *f = field(i, true);
		if(f == NULL) {
			qWarning() << "FieldArchive::printAkaos: cannot open field" << i;
			continue;
		}

		QString name = f->inf()->mapName();

		TutFileStandard *tutosAndSounds = f->tutosAndSounds();
		if(!tutosAndSounds->isOpen()) {
			qWarning() << "FieldArchive::printAkaos: cannot open tutos and sounds" << name;
		}

		for(int akaoID=0; akaoID < tutosAndSounds->size(); ++akaoID) {
			if(!tutosAndSounds->isTut(akaoID)) {
				QString filename = d.filePath(QString("%1-%2.akao")
				                                  .arg(tutosAndSounds->akaoID(akaoID))
				                                  .arg(Data::music_names.at(tutosAndSounds->akaoID(akaoID))));
				if (QFile::exists(filename)) {
					QFile r(filename);
					r.open(QIODevice::ReadOnly);
					if (tutosAndSounds->data(akaoID) == r.readAll()) {
						qDebug() << "Already saved";
						r.close();
						continue;
					} else {
						qDebug() << "variation" << name;
						filename = QString("%1-%2-%3.akao")
						               .arg(tutosAndSounds->akaoID(akaoID))
						               .arg(Data::music_names.at(tutosAndSounds->akaoID(akaoID)))
						               .arg(name);
					}
					r.close();
				}
				QFile deb(filename);
				deb.open(QIODevice::WriteOnly | QIODevice::Truncate);
				deb.write(tutosAndSounds->data(akaoID));
				deb.close();
			}
		}
	}
}

void FieldArchive::printAkaos(const QString &filename)
{
	QFile deb(filename);
	deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	foreach(int i, fieldsSortByMapId) {
		Field *f = field(i, true);
		if(f == NULL) {
			qWarning() << "FieldArchive::printAkaos: cannot open field" << i;
			continue;
		}
		QString name = f->inf()->mapName();

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

void FieldArchive::printModelLoaders(const QString &filename, bool generic)
{
	QFile deb(filename);
	deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	foreach(int i, fieldsSortByMapId) {
		Field *f = field(i, true);
		if(f == NULL) {
			qWarning() << "FieldArchive::printModelLoaders: cannot open field" << i;
			continue;
		}
		if (!f->inf()->isOpen()) {
			qWarning() << "FieldArchive::printModelLoaders: Cannot open inf file";
			continue;
		}
		QString name = f->inf()->mapName();

		FieldModelLoader *modelLoader = f->fieldModelLoader();
		if(!modelLoader->isOpen()) {
			qWarning() << "FieldArchive::printModelLoaders: cannot open tutos and sounds" << name;
			continue;
		}

		if (generic) {
			for (int modelId = 0; modelId < modelLoader->modelCount(); ++modelId) {
				FieldModelFile *fieldModel = f->fieldModel(modelId);
				quint16 scale;
				QList<QRgb> colors;
				int boneCount = fieldModel->boneCount();
				if (f->isPC()) {
					FieldModelLoaderPC *modelLoaderPC = static_cast<FieldModelLoaderPC *>(modelLoader);
					scale = modelLoaderPC->scale(modelId);
					// colors = modelLoaderPC->lightColors(modelId);
				} else {
					FieldModelFilePS *fieldModelPS = static_cast<FieldModelFilePS *>(fieldModel);
					scale = fieldModelPS->scale();
					// colors = fieldModelPS->lightColors();
					boneCount -= 1;
					if (boneCount <= 0) {
						boneCount = 1;
					}
				}
				deb.write(QString("%1 > model %2: bones %3\nscale %4\nunknown %5\n")
						  .arg(name)
						  .arg(modelId)
						  .arg(boneCount)
						  .arg(scale)
						  .arg(modelLoader->unknown(modelId))
						  .toLatin1());

				/* foreach (const QRgb &color, colors) {
					deb.write(QString("%1, ")
							  .arg(color).toLatin1());
				}
				deb.write("\n"); */

				/*for (int animId = 0; animId < modelLoader->animCount(modelId); ++animId) {
					deb.write(QString("\tanim %1\n")
							  .arg(animId)
							  .toLatin1());
				}*/
			}
		} else if(f->isPC()) {
			FieldModelLoaderPC *modelLoaderPC = static_cast<FieldModelLoaderPC *>(modelLoader);

			int modelID = 0;
			foreach(const QString &hrc, modelLoaderPC->HRCNames()) {
				deb.write(QString("%1 > model %2: %3\n")
						  .arg(name)
						  .arg(modelID)
						  .arg(hrc.toLower().left(hrc.lastIndexOf('.')))
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

void FieldArchive::printTexts(const QString &filename, bool usedTexts)
{
	QFile deb(filename);
	deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	foreach(int i, fieldsSortByMapId) {
		Field *f = field(quint32(i), true);
		if(f == nullptr) {
			qWarning() << "FieldArchive::printTexts: cannot open field" << i;
			continue;
		}

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if(scriptsAndTexts->isOpen()) {
			qWarning() << f->inf()->mapName();
			QSet<quint8> listUsedTexts = usedTexts ? scriptsAndTexts->listUsedTexts() : QSet<quint8>();

			deb.write(QString("\n=== %1 ===\n\n")
			          .arg(f->inf()->mapName()).toUtf8());

			quint8 textID = 0;
			foreach(const FF7Text &text, scriptsAndTexts->texts()) {
				if((!usedTexts || listUsedTexts.contains(textID)) && !text.data().isEmpty()) {
					deb.write(QString("%1\n\n")
					          .arg(text.text(false)).toUtf8());
				}
				textID++;
			}
		}
	}
}

void FieldArchive::printTextsDir(const QString &dirname, bool usedTexts)
{
	QDir dir(dirname);

	if (!dir.exists()) {
		dir.mkpath(dirname);
	}

	foreach(int i, fieldsSortByMapId) {
		Field *f = field(quint32(i), true);
		if(f == nullptr) {
			qWarning() << "FieldArchive::printTexts: cannot open field" << i;
			continue;
		}

		QFile deb(dir.filePath(f->name() + ".txt"));
		deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if(scriptsAndTexts->isOpen()) {
			qWarning() << f->inf()->mapName();
			QSet<quint8> listUsedTexts = usedTexts ? scriptsAndTexts->listUsedTexts() : QSet<quint8>();

			quint8 textID = 0;
			foreach(const FF7Text &text, scriptsAndTexts->texts()) {
				if(!usedTexts || listUsedTexts.contains(textID)) {
					deb.write(QString("%1\n\n")
					          .arg(text.text(false)).toUtf8());
				}
				textID++;
			}
		}
	}
}

void FieldArchive::compareTexts(FieldArchive *other)
{
	FieldArchiveIO::ErrorCode err = other->open();
	if (err != FieldArchiveIO::Ok) {
		qWarning() << "Error opening other field archive";
		return;
	}
	qDebug() << other->size();

	QFile deb(QString("texts-compare-P%1.txt").arg(isPC() ? "C" : "S"));
	deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	const int dialogWidth = 80; // characters

	FieldArchiveIterator it(*this);
	while (it.hasNext()) {
		Field *f1 = it.next();
		if(f1 == NULL) {
			qWarning() << "FieldArchive::compareTexts: cannot open field";
			continue;
		}
		Field *f2 = other->field(f1->name());
		if(f2 == NULL) {
			qWarning() << "FieldArchive::compareTexts: cannot find field2" << f1->name();
			continue;
		}

		Section1File *scriptsAndTexts1 = f1->scriptsAndTexts(),
				*scriptsAndTexts2 = f2->scriptsAndTexts();
		if(scriptsAndTexts1->isOpen() && scriptsAndTexts2->isOpen()) {
			qWarning() << f1->name();

			if (scriptsAndTexts1->textCount() != scriptsAndTexts2->textCount()) {
				qWarning() << "FieldArchive::compareTexts: different text count" << f1->name();
				continue;
			}

			int textID = 0;
			foreach(const FF7Text &text1, scriptsAndTexts1->texts()) {
				const FF7Text &text2 = scriptsAndTexts2->text(textID);
				if (text1 != text2) {
					QStringList lines1 = text1.text(false).split("\n"),
								lines2 = text2.text(false).split("\n");
					QListIterator<QString> it1(lines1), it2(lines2);
					while (it1.hasNext() || it2.hasNext()) {
						QString line1, line2;
						if (it1.hasNext()) {
							line1 = it1.next();
						}
						if (it2.hasNext()) {
							line2 = it2.next();
						}
						deb.write(QString("%1\t%2\n")
								  .arg(line1.leftJustified(dialogWidth),
									   line2.leftJustified(dialogWidth)).toUtf8());
					}
					deb.write(QString("\n").toUtf8());
				}
				textID++;
			}
		} else {
			qWarning() << "FieldArchive::compareTexts: Cannot open texts" << f1->name();
		}
	}
}

void FieldArchive::printScripts(const QString &filename)
{
	QFile deb(filename);
	deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	foreach(int i, fieldsSortByMapId) {
		Field *f = field(i, true);
		if(f == NULL) {
			qWarning() << "FieldArchive::printAkaos: cannot open field" << i;
			continue;
		}

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if(scriptsAndTexts->isOpen()) {
			qWarning() << f->inf()->mapName();

			int grpScriptID = 0;
			foreach(GrpScript *grp, scriptsAndTexts->grpScripts()) {
				int scriptID = 0;
				foreach(Script *script, grp->scripts()) {
					int opcodeID = 0;
					foreach(Opcode *opcode, script->opcodes()) {
						FF7Window win;
						/* if(opcode->getWindow(win)) {
							deb.write(QString("%1 > %2 > %3: %4 %5\n")
									  .arg(f->name(), grp->name())
									  .arg(scriptID)
									  .arg(win.x).arg(win.y).toLatin1());
						} else if (opcode->getTextID() < 0) { */
							deb.write(QString("%1 > %2 > %3: %4\n")
									  .arg(f->inf()->mapName(), grp->name())
									  .arg(scriptID)
									  .arg(opcode->toString(f)).toUtf8());
						// }
						opcodeID++;
					}
					scriptID++;
				}
				grpScriptID++;
			}
		}
	}
}

void FieldArchive::printScriptsDirs(const QString &filename)
{
	QDir dir(filename);
	if(!dir.exists()) {
		dir.mkdir(".");
	}

	foreach(int i, fieldsSortByMapId) {
		Field *f = field(i, true);
		if(f == NULL) {
			qWarning() << "FieldArchive::printScriptsDirs: cannot open field" << i;
			continue;
		}
		if(!f->name().startsWith("del")) {
			continue;
		}

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if(scriptsAndTexts->isOpen()) {
			qWarning() << f->inf()->mapName();

			QString dirname = f->inf()->mapName();
			dir.mkdir(dirname);
			dir.cd(dirname);

			int grpScriptID = 0;
			foreach(GrpScript *grp, scriptsAndTexts->grpScripts()) {

				dirname = QString("%1-%2")
				          .arg(grpScriptID, 2, 10, QChar('0'))
				          .arg(grp->name());
				dir.mkdir(dirname);
				dir.cd(dirname);

				int scriptID = 0;
				foreach(Script *script, grp->scripts()) {

					QFile deb(dir.filePath(QString("%1-script").arg(scriptID, 2, 10, QChar('0'))));
					if (!deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
						qWarning() << "error opening debug file" << deb.fileName();
						return;
					}

					int opcodeID = 0;
					foreach(Opcode *opcode, script->opcodes()) {
						// FF7Window win;
						/* if(opcode->getWindow(win)) {
							deb.write(QString("%1 > %2 > %3: %4 %5\n")
									  .arg(f->name(), grp->name())
									  .arg(scriptID)
									  .arg(win.x).arg(win.y).toLatin1());
						} else if (opcode->getTextID() < 0) { */
							deb.write(opcode->toString(f).toUtf8());
							deb.write("\n");
						// }
						opcodeID++;
					}
					scriptID++;
					deb.close();
				}
				dir.cdUp();
				grpScriptID++;
			}

			dir.cdUp();
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
				foreach(GrpScript *group2, scriptsAndTextsOriginal->grpScripts()) {
					GrpScript *grp = scriptsAndTexts->grpScript(grpScriptID);

					while(group2->name() != grp->name()) {
						grpScriptID++;
						if(grpScriptID >= scriptsAndTexts->grpScriptCount()) {
							qWarning() << "ALERT end grpScript reached";
							goto next_group;
						}
						grp = scriptsAndTexts->grpScript(grpScriptID);
					}

					if(grp->size() != group2->size()) {
						qWarning() << "ALERT wrong group size" << grp->size() << group2->size();
						break;
					}

					scriptID = 0;
					foreach(Script *script2, group2->scripts()) {
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

bool FieldArchive::printBackgroundTiles(Field *field, const QString &filename, bool uniformize)
{
	QFile f(filename);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		qWarning() << "FieldArchive::printBackgroundTiles" << f.errorString();
		return false;
	}

	BackgroundFile *bgFile = field->background();
	if (!bgFile || !bgFile->isOpen()) {
		qWarning() << "FieldArchive::printBackgroundTiles no bg for this field" << field->name();
		return false;
	}

	BackgroundTiles tiles;
	if (uniformize && field->isPS()) { // Convert PS to PC to have the same output whenever the source
		BackgroundFilePS *bgFilePS = static_cast<BackgroundFilePS *>(bgFile);
		PalettesPC palettesPC = ((PalettesPS *)&(bgFilePS->palettes()))->toPC();
		static_cast<BackgroundTexturesPS *>(bgFilePS->textures())
		        ->toPC(bgFilePS->tiles(), tiles, palettesPC);
	} else {
		tiles = bgFile->tiles();
	}

	foreach (const Tile &tile, tiles.sortedTiles()) {
		f.write(QString("tileID=%1, layer=%2, param=%3, state=%4\n")
		        .arg(tile.tileID)
		        .arg(tile.layerID)
		        .arg(tile.param)
		        .arg(tile.state).toUtf8());
		f.write(QString("srcX=%1, srcY=%2, textureID1=%3, textureID2=%4\n")
		        .arg(tile.srcX)
		        .arg(tile.srcY)
		        .arg(tile.textureID)
		        .arg(tile.textureID2).toUtf8());
		f.write(QString("dstX=%1, dstY=%2, dstZ=%3, size=%4\n")
		        .arg(tile.dstX)
		        .arg(tile.dstY)
		        .arg(tile.ID)
		        .arg(tile.size).toUtf8());
		f.write(QString("palID=%1, depth=%2, blending=%3, typeTrans=%4\n")
		        .arg(tile.paletteID)
		        .arg(tile.depth)
		        .arg(tile.blending)
		        .arg(tile.typeTrans).toUtf8());
		f.write(QString("dstZBig=%1\n")
		        .arg(tile.IDBig).toUtf8());
		f.write("\n");
	}

	return true;
}

void FieldArchive::searchBackgroundZ()
{
	QMultiMap<int, quint32> ids;
	FieldArchiveIterator it(*this);
	while (it.hasNext()) {
		Field *field = it.next();
		if (field && field->isOpen()) {
			BackgroundFile *bg = field->background();
			if (bg->isOpen()) {
				bool function1 = false;
				foreach (const Tile &tile, bg->tiles()) {
					float ratio = float(tile.ID) / float(tile.IDBig);
					if (tile.layerID == 1 && tile.ID * 10000 != tile.IDBig) {
						if (ratio > 4.5e-5 && tile.IDBig < 9000000) {
							ids.insert(tile.ID, tile.IDBig);
							function1 = true;
						} else {
							if (function1) {
								qDebug() << "Function2 in function 1 ?";
							}
						}
					}
					if (tile.layerID == 1 && tile.ID == 324 && tile.IDBig < 9000000) {
						qDebug() << field->name();
						qDebug() << QString("tileID=%1, layer=%2, param=%3, state=%4\n")
						            .arg(tile.tileID)
						            .arg(tile.layerID)
						            .arg(tile.param)
						            .arg(tile.state).toUtf8();
						qDebug() << QString("srcX=%1, srcY=%2, textureID1=%3, textureID2=%4\n")
						            .arg(tile.srcX)
						            .arg(tile.srcY)
						            .arg(tile.textureID)
						            .arg(tile.textureID2).toUtf8();
						qDebug() << QString("dstX=%1, dstY=%2, dstZ=%3, size=%4\n")
						            .arg(tile.dstX)
						            .arg(tile.dstY)
						            .arg(tile.ID)
						            .arg(tile.size).toUtf8();
						qDebug() << QString("palID=%1, depth=%2, blending=%3, typeTrans=%4\n")
						            .arg(tile.paletteID)
						            .arg(tile.depth)
						            .arg(tile.blending)
						            .arg(tile.typeTrans).toUtf8();
						qDebug() << QString("dstZBig=%1\n")
						            .arg(tile.IDBig).toUtf8();
					}
				}

//				qDebug() << field->name() << field->scriptsAndTexts()->scale();
			}
		}
	}

	foreach (int ID, ids.uniqueKeys()) {
		QList<quint32> zBigs = ids.values(ID);
		QList<quint32> zBigsUnique = zBigs.toSet().toList();
		qSort(zBigsUnique);
		foreach (quint32 zBig, zBigsUnique) {
			qDebug() << qPrintable(QString("%1\t%2\t%3\t%4").arg(ID).arg(zBig).arg(zBigs.count(zBig)).arg(float(ID) / float(zBig)));
		}
	}
}

void FieldArchive::searchAll()
{
	QTime t;t.start();

	/* QFile deb1("encounters-scripts.txt");
	deb1.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	deb1.write("field name,encounter,group ID,script ID,opcode ID\n"); */

	FieldArchiveIterator it1(*this);
	while (it1.hasNext()) {
		Field *field = it1.next(true, true);
		if (field && field->isOpen()) {
			/* Section1File *scripts = field->scriptsAndTexts();
			if (scripts->isOpen()) {
				int groupID=0, scriptID=0, opcodeID=0;
				while (scripts->searchOpcode(int(Opcode::BATTLE), groupID, scriptID, opcodeID)) {
					OpcodeBATTLE *op = (OpcodeBATTLE *)scripts->grpScript(groupID)->script(quint8(scriptID))->opcode(quint16(opcodeID));
					deb1.write((field->name() % "," % QString::number(op->battleID) % "," % QString::number(groupID) % "," % QString::number(scriptID) % "," % QString::number(opcodeID) % "\n").toLocal8Bit());
					opcodeID += 1;
				}
			} */
			FieldModelLoader *modelLoader = field->fieldModelLoader();
			if (modelLoader && modelLoader->isOpen() && field->isPC()) {
				FieldModelLoaderPC *modelLoaderPc = (FieldModelLoaderPC *)modelLoader;
				for (int i = 0; i < modelLoaderPc->modelCount(); i++) {
					FieldModelInfosPC infos = modelLoaderPc->modelInfos(i);
					if (infos.nameHRC == "AAAA.HRC" && infos.typeHRC >= 1024) {
						qDebug() << qPrintable(field->name()) << infos.typeHRC;
					}
				}
			}
			/* EncounterFile *enc = field->encounter();
			if (enc && enc->isOpen()) {
				foreach (EncounterFile::Table table, QList<EncounterFile::Table>() << EncounterFile::Table1 << EncounterFile::Table2) {
					const EncounterTable &ta = enc->encounterTable(table);
					if (ta.enabled & 1) {
						for (int i = 0; i < 6; i++) {
							if (PROBABILITY(ta.enc_standard[i]) >= 32) {
								qDebug() << qPrintable(field->name()) << PROBABILITY(ta.enc_standard[i]);
							}
						}
						for (int i = 0; i < 4; i++) {
							if (PROBABILITY(ta.enc_special[i]) >= 32) {
								qDebug() << qPrintable(field->name()) << PROBABILITY(ta.enc_special[i]);
							}
						}
					}
				}
			} */
		}
	}

	// deb1.close();

	return;

	/* QFile deb1("encounters.txt");
	deb1.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	deb1.write("field name,encounter\n");

	FieldArchiveIterator it1(*this);
	while (it1.hasNext()) {
		Field *field = it1.next(true, true);
		if (field && field->isOpen()) {
			EncounterFile *enc = field->encounter();
			if (enc->isOpen()) {
				foreach (EncounterFile::Table table, QList<EncounterFile::Table>() << EncounterFile::Table1 << EncounterFile::Table2) {
					const EncounterTable &ta = enc->encounterTable(table);
					if (ta.enabled & 1) {
						for (int i = 0; i < 6; i++) {
							if (BATTLE_ID(ta.enc_standard[i]) > 0 && PROBABILITY(ta.enc_standard[i]) > 0) {
								deb1.write((field->name() % "," % QString::number(BATTLE_ID(ta.enc_standard[i])) % "\n").toLocal8Bit());
							}
						}
						for (int i = 0; i < 4; i++) {
							if (BATTLE_ID(ta.enc_special[i]) > 0 && PROBABILITY(ta.enc_special[i]) > 0) {
								deb1.write((field->name() % "," % QString::number(BATTLE_ID(ta.enc_special[i])) % "\n").toLocal8Bit());
							}
						}

					}
				}

			}
		}
	}

	deb1.close();

	return; */

	QFile deb(QString("scripts-unlocked-talk-P%1.txt").arg(isPC() ? "C" : "S"));
	deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	FieldArchiveIterator it(*this);
	while (it.hasNext()) {
		Field *field = it.next();
		if (field && field->isOpen()) {
			TutFileStandard *tut = field->tutosAndSounds();
			if (tut->isOpen()) {
				for (int tutID = 0; tutID < tut->size(); ++tutID) {
					if (tut->isAkao(tutID)) {
						if (tut->akaoID(tutID) == 99) {
							qDebug() << "akao 99 is here:" << field->name();
						}
					}
				}
			}
			/*Section1File *section1 = field->scriptsAndTexts();
			if (section1->isOpen()) {
				int groupID = 0;
				foreach (GrpScript *grp, section1->grpScripts()) {
					if (grp->typeID() == GrpScript::Model || grp->typeID() == GrpScript::Location) {
						Script *talkScript = grp->script(2); // Talk
						bool disableMovability = false,
								disableMenus = false,
								saySomething = false;
						foreach (Opcode *op, talkScript->opcodes()) {
							if (op->id() == Opcode::UC) {
								disableMovability = true;
							} else if (op->id() == Opcode::MENU2) {
								disableMenus = true;
							} else if (op->getTextID() >= 0) {
								saySomething = true;
							}
						}

						if (saySomething && (!disableMovability || !disableMenus)) {
							deb.write(QString("%1 -> %2: %3 (blame: %4)\n")
									  .arg(field->name())
									  .arg(groupID)
									  .arg(grp->name())
									  .arg(section1->author())
									  .toLatin1());
						}
					}
					groupID++;
				}
			}*/
//			BackgroundFile *bg = field->background();
//			if (bg->isOpen()) {
//				bg->openBackground();
//				qDebug() << field->name() << field->scriptsAndTexts()->scale();
//			}
		}
	}

	return;

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
				foreach(GrpScript *group2, scriptsAndTextsOriginal->grpScripts()) {
					GrpScript *grp = scriptsAndTexts->grpScript(grpScriptID);

					while(group2->name() != grp->name()) {
						grpScriptID++;
						if(grpScriptID >= scriptsAndTexts->grpScriptCount()) {
							qWarning() << "ALERT end grpScript reached";
							goto next_group;
						}
						grp = scriptsAndTexts->grpScript(grpScriptID);
					}

					if(grp->size() != group2->size()) {
						qWarning() << "ALERT wrong group size" << grp->size() << group2->size();
						break;
					}

					scriptID = 0;
					foreach(Script *script2, group2->scripts()) {
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
		SearchOpcodeQuery *query = static_cast<SearchOpcodeQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchOpcode(query->opcode, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchVarQuery query(bank, address, op, value);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchVarQuery *query = static_cast<SearchVarQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchVar(query->bank, query->address, query->op, query->value, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchExec(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchExecQuery query(group, script);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchExecQuery *query = static_cast<SearchExecQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchExec(query->group, query->script, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchMapJump(quint16 _field, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchFieldQuery query(_field);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchFieldQuery *query = static_cast<SearchFieldQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchMapJump(query->fieldID, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchTextInScripts(const QRegExp &text, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = static_cast<SearchTextQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchTextInScripts(query->text, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchText(const QRegExp &text, int &fieldID, int &textID, int &from, int &size, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	int empty;
	SearchInText searchIn(textID, from, size, empty);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = static_cast<SearchTextQuery *>(_query);
		SearchInText *searchIn = static_cast<SearchInText *>(_searchIn);
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
		SearchOpcodeQuery *query = static_cast<SearchOpcodeQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchOpcodeP(query->opcode, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchVarQuery query(bank, address, op, value);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchVarQuery *query = static_cast<SearchVarQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchVarP(query->bank, query->address, query->op, query->value, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchExecP(quint8 group, quint8 script, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchExecQuery query(group, script);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchExecQuery *query = static_cast<SearchExecQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchExecP(query->group, query->script, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchMapJumpP(quint16 _field, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchFieldQuery query(_field);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchFieldQuery *query = static_cast<SearchFieldQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchMapJumpP(query->fieldID, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchTextInScriptsP(const QRegExp &text, int &fieldID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = static_cast<SearchTextQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchTextInScriptsP(query->text, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, fieldID, &searchIn, sorting, scope);
}

bool FieldArchive::searchTextP(const QRegExp &text, int &fieldID, int &textID, int &from, int &index, int &size, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	SearchInText searchIn(textID, from, size, index);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = static_cast<SearchTextQuery *>(_query);
		SearchInText *searchIn = static_cast<SearchInText *>(_searchIn);
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
							   bool overwrite, const QMap<ExportType, QString> &toExport,
                               PsfTags *tags)
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
					QByteArray fieldData = io()->fieldData(f, io()->isPC() ? QString() : "DAT", extension.compare("dec", Qt::CaseInsensitive) == 0);
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
							path = QDir::cleanPath(QString("%1/%2.%3").arg(directory, Data::music_names.value(akaoList->akaoID(i), f->name()), extension));
							if(overwrite || !QFile::exists(path)) {
								QFile tutExport(path);
								if(tutExport.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
									if (extension == "minipsf") {
										tags->setTitle(Data::music_desc.value(akaoList->akaoID(i), f->name()));
										tutExport.write(PsfFile::fromAkao(akaoList->data(i), *tags).save());
									} else {
										tutExport.write(akaoList->data(i));
									}
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
