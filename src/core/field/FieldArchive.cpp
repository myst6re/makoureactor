/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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
#include "Data.h"
#include <PsfFile.h>

SearchIn::~SearchIn()
{
}

void SearchInScript::reset()
{
	groupID = scriptID = opcodeID = 0;
}

void SearchInScript::toEnd()
{
	groupID = scriptID = opcodeID = 2147483647;
}

void SearchInText::reset()
{
	textID = 0;
	from = -1;
}

void SearchInText::toEnd()
{
	textID = 2147483647;
	from = -1;
}

FieldArchiveIterator::FieldArchiveIterator(const FieldArchive &archive) :
    QMapIterator<int, Field *>(archive.fileList), mapList(archive.fileList)
{
}

bool FieldArchiveIterator::seek(int mapId)
{
	toFront();

	if (mapId < 0) {
		return true;
	}

	Field *f = mapList.value(mapId);
	if (nullptr != f && findNext(f)) {
		previous();
		return true;
	}

	toFront();

	return false;
}

Field *FieldArchiveIterator::next(bool open, bool dontOptimize)
{
	return openField(QMapIterator<int, Field *>::next().value(), open, dontOptimize);
}

Field *FieldArchiveIterator::peekNext(bool open, bool dontOptimize) const
{
	return openField(QMapIterator<int, Field *>::peekNext().value(), open, dontOptimize);
}

Field *FieldArchiveIterator::peekPrevious(bool open, bool dontOptimize) const
{
	return openField(QMapIterator<int, Field *>::peekPrevious().value(), open, dontOptimize);
}

Field *FieldArchiveIterator::previous(bool open, bool dontOptimize)
{
	return openField(QMapIterator<int, Field *>::previous().value(), open, dontOptimize);
}

Field *FieldArchiveIterator::openField(Field *field, bool open, bool dontOptimize)
{
	if (field != nullptr && open && !field->isOpen()) {
		field->open(dontOptimize);
	}
	return field;
}

FieldArchive::FieldArchive() :
	_io(nullptr), _observer(nullptr)
{
}

FieldArchive::FieldArchive(FieldArchiveIO *io) :
	_io(io), _observer(nullptr)
{
	//	fileWatcher.addPath(path);
	//	connect(&fileWatcher, &QFileSystemWatcher::fileChanged, this, &FieldArchive::fileChanged);
	//	connect(&fileWatcher, &QFileSystemWatcher::directoryChanged, this, &FieldArchive::directoryChanged);
}

FieldArchive::~FieldArchive()
{
	qDeleteAll(fileList);
	if (_io) {
		delete _io;
	}
}

FieldArchiveIO::ErrorCode FieldArchive::open()
{
	if (!_io)	return FieldArchiveIO::Invalid;

	clear();

	FieldArchiveIO::ErrorCode error = _io->open(observer());
	if (error != FieldArchiveIO::Ok) {
		return error;
	}

	if (fileList.isEmpty()) {
		return FieldArchiveIO::FieldNotFound;
	}

	int fieldID = 0;
	for (Field *f : std::as_const(fileList)) {
		if (f != nullptr) {
			updateFieldLists(f, fieldID);
		}
		++fieldID;
	}

	return FieldArchiveIO::Ok;
}

FieldArchiveIO::ErrorCode FieldArchive::save(const QString &path)
{
	if (!_io)	return FieldArchiveIO::Invalid;

	FieldArchiveIO::ErrorCode error = _io->save(path, observer());
	if (error == FieldArchiveIO::Ok) {
		// Clear "isModified" state
		setSaved();
	}
	return error;
}

void FieldArchive::close()
{
	if (_io)	_io->close();
}

void FieldArchive::clear()
{
	qDeleteAll(fileList);
	fileList.clear();
	fieldsSortByName.clear();
	_mapList.clear();
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
	if (!field->isOpen()) {
		return field->open(dontOptimize);
	}
	return true;
}

int FieldArchive::indexOfField(const QString &name) const
{
	return fieldsSortByName.value(name.toLower(), -1);
}

const Field *FieldArchive::field(int mapId) const
{
	return fileList.value(mapId, nullptr);
}

Field *FieldArchive::field(int mapId, bool open, bool dontOptimize)
{
	Field *field = fileList.value(mapId, nullptr);
	if (field != nullptr && open && !openField(field, dontOptimize)) {
		return nullptr;
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

void FieldArchive::updateFieldLists(Field *field, int mapId)
{
	fieldsSortByName.insert(field->name().toLower(), mapId);
}

void FieldArchive::addNewField(Field *field, int &mapID)
{
	mapID = appendField(field);
	_mapList.addMap(field->name());
	updateFieldLists(field, mapID);
}

void FieldArchive::renameField(Field *field, const QString &newName)
{
	const QString oldName = field->name();
	field->setName(newName);
	InfFile *inf = field->inf();
	if (inf != nullptr && inf->isOpen()) {
		inf->setMapName(field->name());
	}
	int mapId = fieldsSortByName.take(oldName.toLower());
	updateFieldLists(field, mapId);
	if (!_mapList.renameMap(oldName, field->name())) {
		qWarning() << "FieldArchive::renameField map not found in maplist";
	}
}

void FieldArchive::delField(int mapId)
{
	Field *field = fileList.value(mapId);

	if (nullptr == field) {
		return;
	}

	_mapList.softDeleteMap(field->name());

	fileList.insert(mapId, nullptr);
	delete field;
}

int FieldArchive::appendField(Field *field)
{
	qsizetype mapId = _mapList.mapNames().indexOf(field->name());
	if (mapId < 0) {
		if (_mapList.mapNames().isEmpty()) {
			mapId = Data::maplist(field->isPC()).indexOf(field->name());
		}
		if (mapId < 0) {
			mapId = 1200 + (fileList.isEmpty() ? 0 : fileList.lastKey() + 1);
		}
	}
	fileList.insert(int(mapId), field);
	return int(mapId);
}

bool FieldArchive::isAllOpened() const
{
	for (Field *f : fileList) {
		if (f != nullptr && !f->isOpen()) {
			return false;
		}
	}

	return true;
}

bool FieldArchive::isModified() const
{
	for (Field *f : fileList) {
		if (f != nullptr && f->isModified()) {
			return true;
		}
	}

	return false;
}

QList<FF7Var> FieldArchive::searchAllVars(QMap<FF7Var, QSet<QString> > &fieldNames)
{
	QList<FF7Var> vars;
	FieldArchiveIterator it(*this);

	while (it.hasNext()) {
		Field *field = it.next();

		if (field != nullptr) {
			QList<FF7Var> fieldVars;
			field->scriptsAndTexts()->searchAllVars(fieldVars);

			for (const FF7Var &fieldVar : std::as_const(fieldVars)) {
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
#include "core/FF7Font.h"

void FieldArchive::validateAsk()
{
	FieldArchiveIterator it(*this);

	while (it.hasNext()) {
		Field *f = it.next();

		if (f == nullptr) {
			qWarning() << "FieldArchive::printAkaos: cannot open field" << it.mapId();
			continue;
		}

		QString name = f->name();

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if (scriptsAndTexts->isOpen()) {

			int grpScriptID = 0;
			for (const GrpScript &grp : scriptsAndTexts->grpScripts()) {
				int scriptID = 0;
				for (const Script &script : grp.scripts()) {
					int opcodeID = 0;
					for (const Opcode &opcode : script.opcodes()) {
						if (opcode.id() == OpcodeKey::ASK) {
							const OpcodeASK &opcodeASK = opcode.op().opcodeASK;
							quint8 textID = opcodeASK.textID,
									firstLine = opcodeASK.firstLine,
									lastLine = opcodeASK.lastLine;
							if (textID < scriptsAndTexts->textCount()) {
								QString text = scriptsAndTexts->text(textID).text(false);
								if (!text.isEmpty()) {
									int lineNum = 0, autoFirstLine = -1, autoLastLine = -1;
									bool hasChoice = false;
									for (const QString &line : text.split('\n')) {
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
													  .arg(name, grp.name(), grp.scriptName(scriptID))
													  .arg(opcodeID + 1)
													  .arg(textID).toLatin1().data();
									} else if (autoFirstLine != firstLine
											|| autoLastLine != lastLine) {
										qWarning() << QString("%1 > %2 > %3 > %4: %5 should be %6, %7 should be %8 (textID: %9)")
													  .arg(name, grp.name(), grp.scriptName(scriptID))
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
	FieldArchiveIterator it(*this);

	while (it.hasNext()) {
		Field *f = it.next();

		if (f == nullptr) {
			qWarning() << "FieldArchive::printAkaos: cannot open field" << it.mapId();
			continue;
		}

		QString name = f->name();

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if (scriptsAndTexts->isOpen()) {
			//qWarning() << f->name();

			int grpScriptID = 0;
			for (const GrpScript &grp : scriptsAndTexts->grpScripts()) {
				int scriptID = 0;
				for (const Script &script : grp.scripts()) {
					int opcodeID = 0;
					const OpcodeWindow *opcodeWindow = nullptr;
					for (const Opcode &opcode : script.opcodes()) {
						if (opcode.id() == OpcodeKey::WSIZW
								|| opcode.id() == OpcodeKey::WINDOW) {
							opcodeWindow = opcode.op().opcodeWindow;
						} else if (opcode.getTextID() >= 0 && opcodeWindow) {
							FF7Window window;
							opcodeWindow->getWindow(window);
							if (opcode.getTextID() < scriptsAndTexts->textCount()) {
								FF7String text = scriptsAndTexts->text(opcode.getTextID());
								QSize optimSize = FF7Font::calcSize(text.data());
								if (!text.data().isEmpty() && !text.contains(QRegularExpression("\n")) && (window.w != optimSize.width() || window.h != optimSize.height())) {
									qWarning() << name << grpScriptID << grp.name() << grp.scriptName(scriptID) << opcodeID << "width=" << window.w << "height=" << window.h << "better size=" << optimSize.width() << optimSize.height();
								}
							} else {
								qWarning() << name << grpScriptID << grp.name() << grp.scriptName(scriptID) << opcodeID << "text not found";
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

void FieldArchive::printAkaos(const QString &filename)
{
	QFile deb(filename);
	deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	FieldArchiveIterator it(*this);

	while (it.hasNext()) {
		Field *f = it.next();
		if (f == nullptr) {
			qWarning() << "FieldArchive::printAkaos: cannot open field" << it.mapId();
			continue;
		}
		QString name = f->inf()->mapName();

		TutFileStandard *tutosAndSounds = f->tutosAndSounds();
		if (!tutosAndSounds->isOpen()) {
			qWarning() << "FieldArchive::printAkaos: cannot open tutos and musics" << name;
		}

		for (int akaoID=0; akaoID < tutosAndSounds->size(); ++akaoID) {
			if (tutosAndSounds->isTut(akaoID)) {
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
	FieldArchiveIterator it(*this);

	while (it.hasNext()) {
		Field *f = it.next();
		if (f == nullptr) {
			qWarning() << "FieldArchive::printModelLoaders: cannot open field" << it.mapId();
			continue;
		}
		if (!f->inf()->isOpen()) {
			qWarning() << "FieldArchive::printModelLoaders: Cannot open inf file";
			continue;
		}
		QString name = f->inf()->mapName();

		FieldModelLoader *modelLoader = f->fieldModelLoader();
		if (!modelLoader->isOpen()) {
			qWarning() << "FieldArchive::printModelLoaders: cannot open tutos and musics" << name;
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

				/* for (const QRgb &color : colors) {
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
		} else if (f->isPC()) {
			FieldModelLoaderPC *modelLoaderPC = static_cast<FieldModelLoaderPC *>(modelLoader);

			int modelID = 0;
			for (const QString &hrc : modelLoaderPC->HRCNames()) {
				deb.write(QString("%1 > model %2: %3\n")
						  .arg(name)
						  .arg(modelID)
						  .arg(hrc.toLower().left(hrc.lastIndexOf('.')))
						  .toLatin1());

				int animID = 0;
				for (const QString &a : modelLoaderPC->ANames(modelID)) {
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
	FieldArchiveIterator it(*this);

	while (it.hasNext()) {
		Field *f = it.next();
		if (f == nullptr) {
			qWarning() << "FieldArchive::printTexts: cannot open field" << it.mapId();
			continue;
		}

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if (scriptsAndTexts->isOpen()) {
			qWarning() << f->inf()->mapName();
			QSet<quint8> listUsedTexts = usedTexts ? scriptsAndTexts->listUsedTexts() : QSet<quint8>();

			deb.write(QString("\n=== %1 ===\n\n")
			          .arg(f->inf()->mapName()).toUtf8());

			quint8 textID = 0;
			for (const FF7String &text : scriptsAndTexts->texts()) {
				if ((!usedTexts || listUsedTexts.contains(textID)) && !text.data().isEmpty()) {
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

	FieldArchiveIterator it(*this);

	while (it.hasNext()) {
		Field *f = it.next();

		if (f == nullptr) {
			qWarning() << "FieldArchive::printTexts: cannot open field" << it.mapId();
			continue;
		}

		QFile deb(dir.filePath(f->name() + ".txt"));
		deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if (scriptsAndTexts->isOpen()) {
			qWarning() << f->inf()->mapName();
			QSet<quint8> listUsedTexts = usedTexts ? scriptsAndTexts->listUsedTexts() : QSet<quint8>();

			quint8 textID = 0;
			for (const FF7String &text : scriptsAndTexts->texts()) {
				if (!usedTexts || listUsedTexts.contains(textID)) {
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
		if (f1 == nullptr) {
			qWarning() << "FieldArchive::compareTexts: cannot open field";
			continue;
		}
		Field *f2 = other->field(f1->name());
		if (f2 == nullptr) {
			qWarning() << "FieldArchive::compareTexts: cannot find field2" << f1->name();
			continue;
		}

		Section1File *scriptsAndTexts1 = f1->scriptsAndTexts(),
				*scriptsAndTexts2 = f2->scriptsAndTexts();
		if (scriptsAndTexts1->isOpen() && scriptsAndTexts2->isOpen()) {
			qWarning() << f1->name();

			if (scriptsAndTexts1->textCount() != scriptsAndTexts2->textCount()) {
				qWarning() << "FieldArchive::compareTexts: different text count" << f1->name();
				continue;
			}

			int textID = 0;
			for (const FF7String &text1 : scriptsAndTexts1->texts()) {
				const FF7String &text2 = scriptsAndTexts2->text(textID);
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

	FieldArchiveIterator it(*this);

	while (it.hasNext()) {
		Field *f = it.next();
		if (f == nullptr) {
			qWarning() << "FieldArchive::printAkaos: cannot open field" << it.mapId();
			continue;
		}

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if (scriptsAndTexts->isOpen()) {
			qWarning() << f->inf()->mapName();

			int grpScriptID = 0;
			for (const GrpScript &grp : scriptsAndTexts->grpScripts()) {
				int scriptID = 0;
				for (const Script &script : grp.scripts()) {
					int opcodeID = 0;
					for (const Opcode &opcode : script.opcodes()) {
						FF7Window win;
						/* if (opcode.getWindow(win)) {
							deb.write(QString("%1 > %2 > %3: %4 %5\n")
									  .arg(f->name(), grp->name())
									  .arg(scriptID)
									  .arg(win.x).arg(win.y).toLatin1());
						} else if (opcode.getTextID() < 0) { */
							deb.write(QString("%1 > %2 > %3: %4\n")
									  .arg(f->inf()->mapName(), grp.name())
									  .arg(scriptID)
									  .arg(opcode.toString(scriptsAndTexts)).toUtf8());
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
	if (!dir.exists()) {
		dir.mkdir(".");
	}

	FieldArchiveIterator it(*this);

	while (it.hasNext()) {
		Field *f = it.next();

		if (f == nullptr) {
			qWarning() << "FieldArchive::printScriptsDirs: cannot open field" << it.mapId();
			continue;
		}
		if (!f->name().startsWith("del")) {
			continue;
		}

		Section1File *scriptsAndTexts = f->scriptsAndTexts();
		if (scriptsAndTexts->isOpen()) {
			qWarning() << f->inf()->mapName();

			QString dirname = f->inf()->mapName();
			dir.mkdir(dirname);
			dir.cd(dirname);

			int grpScriptID = 0;
			for (const GrpScript &grp : scriptsAndTexts->grpScripts()) {

				dirname = QString("%1-%2")
				          .arg(grpScriptID, 2, 10, QChar('0'))
				          .arg(grp.name());
				dir.mkdir(dirname);
				dir.cd(dirname);

				int scriptID = 0;
				for (const Script &script : grp.scripts()) {

					QFile deb(dir.filePath(QString("%1-script").arg(scriptID, 2, 10, QChar('0'))));
					if (!deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
						qWarning() << "error opening debug file" << deb.fileName();
						return;
					}

					int opcodeID = 0;
					for (const Opcode &opcode : script.opcodes()) {
						// FF7Window win;
						/* if (opcode.getWindow(win)) {
							deb.write(QString("%1 > %2 > %3: %4 %5\n")
									  .arg(f->name(), grp->name())
									  .arg(scriptID)
									  .arg(win.x).arg(win.y).toLatin1());
						} else if (opcode.getTextID() < 0) { */
							deb.write(opcode.toString(scriptsAndTexts).toUtf8());
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
	if (original.open() != FieldArchiveIO::Ok) {
		qWarning() << "error opening pc lgp";
		return;
	}

	QSet<QString> screens;
	FieldArchiveIterator it(*this);

	while (it.hasNext()) {
		Field *field = it.next();
		Field *fieldOriginal = original.field(original.indexOfField(field->name()));
		if (fieldOriginal == nullptr) {
			qWarning() << "ALERT field not found" << field->name();
			break;
		}
		if (field != nullptr) {
			Section1File *scriptsAndTexts = field->scriptsAndTexts();
			Section1File *scriptsAndTextsOriginal = fieldOriginal->scriptsAndTexts();
			if (scriptsAndTexts->isOpen() && scriptsAndTextsOriginal->isOpen()) {
				qWarning() << field->name();

				if (scriptsAndTexts->grpScriptCount() < scriptsAndTextsOriginal->grpScriptCount()) {
					qWarning() << "ALERT much grpscript wow";
					break;
				}

				QFile deb(QString("scripts-window-coord-18-05-2014/%1.txt").arg(field->name()));
				deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

				int grpScriptID = 0, scriptID;
				for (const GrpScript &group2 : scriptsAndTextsOriginal->grpScripts()) {
					GrpScript grp = scriptsAndTexts->grpScript(grpScriptID);

					while (group2.name() != grp.name()) {
						grpScriptID++;
						if (grpScriptID >= scriptsAndTexts->grpScriptCount()) {
							qWarning() << "ALERT end grpScript reached";
							goto next_group;
						}
						grp = scriptsAndTexts->grpScript(grpScriptID);
					}

					if (grp.size() != group2.size()) {
						qWarning() << "ALERT wrong group size" << grp->size() << group2->size();
						break;
					}

					scriptID = 0;
					for (Script *script2 : group2->scripts()) {
						Script *script = grp->script(scriptID);

						int opcodeID = 0;
						for (Opcode *opcode2 : script2->opcodes()) {
							FF7Window win, win2;

							if (opcode2->getWindow(win2)) {

								Opcode *opcode;

								do {
									if (opcodeID >= script->size()) {
										qWarning() << "ALERT wrong script size";
										goto next_script;
									}

									opcode = script->opcode(opcodeID++);

								} while (!opcode.getWindow(win));

								const quint8 minDiff = 1, maxDiff = 5;
								int diffX = qAbs(win2.x - win.x), diffY = qAbs(win2.y - win.y);
								bool hasTinyDiffX = diffX >= minDiff && diffX <= maxDiff,
										hasTinyDiffY = diffY >= minDiff && diffY <= maxDiff,
										hasLargeDiffX = diffX > 15,//maxDiff,
										hasLargeDiffY = diffY > 15;//maxDiff;
								if (diffX > 0 || diffY > 0) { //(hasTinyDiffX && hasLargeDiffY) || (hasTinyDiffY && hasLargeDiffX)) {
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
				if (deb.size() == 0) {
					deb.remove();
				}
				qDebug() << screens.size() << "écrans";
			}
		}
	}
}

bool FieldArchive::printBackgroundTiles(bool uniformize, bool fromUnusedPCSection)
{
	FieldArchiveIterator it(*this);
	QDir dir(QString("background-tiles-%1").arg(isPC() ? "pc" : "ps"));
	dir.mkpath(".");

	while (it.hasNext()) {
		Field *field = it.next();
		if (!field || !field->isOpen()) {
			continue;
		}

		InfFile *inf = field->inf();
		if (!inf || !inf->isOpen()) {
			continue;
		}

		QFile f(dir.filePath("field-" + inf->mapName() + ".txt"));
		if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			qWarning() << "FieldArchive::printBackgroundTiles" << f.errorString() << f.fileName();
			return false;
		}

		BackgroundFile *bgFile = field->background();
		if (!bgFile || !bgFile->isOpen()) {
			qWarning() << "FieldArchive::printBackgroundTiles no bg for this field" << field->name();
			continue;
		}

		BackgroundTiles tiles;
		if (fromUnusedPCSection) {
			BackgroundTilesFile *tilesFile = field->tiles();
			tiles = tilesFile->tiles();
		}
		else if (uniformize && field->isPS()) { // Convert PS to PC to have the same output whenever the source
			BackgroundFilePS *bgFilePS = static_cast<BackgroundFilePS *>(bgFile);
			PalettesPC palettesPC = ((PalettesPS *)&(bgFilePS->palettes()))->toPC();
			static_cast<BackgroundTexturesPS *>(bgFilePS->textures())
					->toPC(bgFilePS->tiles(), tiles, palettesPC);
		} else {
			tiles = bgFile->tiles();
		}

		for (const Tile &tile : tiles.sortedTiles()) {
			f.write(QString("tileID=%1, layer=%2, param=%3, state=%4\n")
					.arg(tile.tileID)
					.arg(tile.layerID)
					.arg(tile.param)
					.arg(tile.state).toUtf8());
			/* f.write(QString("srcX=%1, srcY=%2, textureID1=%3, textureID2=%4\n")
					.arg(tile.srcX)
					.arg(tile.srcY)
					.arg(tile.textureID)
					.arg(tile.textureID2).toUtf8()); */
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
			//f.write(QString("dstZBig=%1\n")
			//        .arg(tile.IDBig).toUtf8());
			f.write("\n");
		}
	}

	return true;
}

void FieldArchive::printBackgroundZ()
{
	QMultiMap<int, quint32> ids;
	FieldArchiveIterator it(*this);

	QDir dir(QString("background-z-%1").arg(isPC() ? "pc" : "ps"));
	dir.mkpath(".");
	QFile deb2("background-z.txt");
	deb2.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

	while (it.hasNext()) {
		Field *field = it.next();

		if (field && field->isOpen()) {
			BackgroundFile *bg = field->background();
			InfFile *inf = field->inf();

			if (bg->isOpen()) {
				bool function1 = false;

				//QFile deb(dir.filePath("%1-background-id.txt").arg(inf->mapName()));
				//deb.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

				for (const Tile &tile : bg->tiles()) {

					//deb.write(QString("%1\n").arg(tile.ID).toLatin1());

					if (tile.layerID == 1)
						ids.insert(tile.ID, tile.IDBig);

					continue;

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
		break;
	}

	for (int ID : ids.uniqueKeys()) {
		QList<quint32> zBigs = ids.values(ID);
		QList<quint32> zBigsUnique = zBigs.toSet().toList();
		std::sort(zBigsUnique.begin(), zBigsUnique.end());
		for (quint32 zBig : zBigsUnique) {
			deb2.write(QString("%1,%2\n").arg(ID).arg(zBig).toLatin1());
		}
	}

	deb2.close();
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
				while (scripts->searchOpcode(int(OpcodeKey::BATTLE), groupID, scriptID, opcodeID)) {
					OpcodeBATTLE *op = (OpcodeBATTLE *)scripts->grpScript(groupID)->script(quint8(scriptID))->opcode(quint16(opcodeID));
					deb1.write((field->name() % "," % QString::number(op.battleID) % "," % QString::number(groupID) % "," % QString::number(scriptID) % "," % QString::number(opcodeID) % "\n").toLocal8Bit());
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
				for (EncounterFile::Table table :  QList<EncounterFile::Table>() << EncounterFile::Table1 << EncounterFile::Table2) {
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
				for (EncounterFile::Table table :  QList<EncounterFile::Table>() << EncounterFile::Table1 << EncounterFile::Table2) {
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
				for (GrpScript *grp : section1->grpScripts()) {
					if (grp->type() == GrpScript::Model || grp->type() == GrpScript::Location) {
						Script *talkScript = grp->script(2); // Talk
						bool disableMovability = false,
								disableMenus = false,
								saySomething = false;
						for (Opcode *op : talkScript->opcodes()) {
							if (op.id() == OpcodeKey::UC) {
								disableMovability = true;
							} else if (op.id() == OpcodeKey::MENU2) {
								disableMenus = true;
							} else if (op.getTextID() >= 0) {
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
	if (original.open() != FieldArchiveIO::Ok) {
		qWarning() << "error opening pc lgp";
		return;
	}

	/*FieldArchivePS ps("C:/Users/Jérôme/Games/Final Fantasy VII-PSX-PAL-FR-CD1.bin", FieldArchiveIO::Iso);
	if (ps.open() != FieldArchiveIO::Ok) {
		qWarning() << "error opening ps iso";
		return;
	}

	if (!isPC()) {
		return;
	}

	Field *f1 = this->field(fieldsSortByName.value("yougan"), true);
	if (f1) {
		Section1File *f1s1 = f1->scriptsAndTexts();
		if (f1s1->isOpen()) {
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
//	for (int i=0; i<size; ++i) {
	FieldArchiveIterator it2(*this);
	while (it2.hasNext()) {
		Field *field = it2.next();
		Field *fieldOriginal = original.field(original.indexOfField(field->name()));
		if (fieldOriginal == nullptr) {
			qWarning() << "ALERT field not found" << field->name();
			break;
		}
		if (field != nullptr) {
			Section1File *scriptsAndTexts = field->scriptsAndTexts();
			Section1File *scriptsAndTextsOriginal = fieldOriginal->scriptsAndTexts();
			if (scriptsAndTexts->isOpen() && scriptsAndTextsOriginal->isOpen()) {
				qWarning() << field->name();

				if (scriptsAndTexts->grpScriptCount() < scriptsAndTextsOriginal->grpScriptCount()) {
					qWarning() << "ALERT much grpscript wow";
					break;
				}

				int grpScriptID = 0, scriptID;
				for (GrpScript *group2 : scriptsAndTextsOriginal->grpScripts()) {
					GrpScript *grp = scriptsAndTexts->grpScript(grpScriptID);

					while (group2->name() != grp->name()) {
						grpScriptID++;
						if (grpScriptID >= scriptsAndTexts->grpScriptCount()) {
							qWarning() << "ALERT end grpScript reached";
							goto next_group;
						}
						grp = scriptsAndTexts->grpScript(grpScriptID);
					}

					if (grp->size() != group2->size()) {
						qWarning() << "ALERT wrong group size" << grp->size() << group2->size();
						break;
					}

					scriptID = 0;
					for (Script *script2 : group2->scripts()) {
						Script *script = grp->script(scriptID);

						int opcodeID = 0;
						for (Opcode *opcode2 : script2->opcodes()) {
							FF7Window win, win2;

							if (opcode2->getWindow(win2)) {

								Opcode *opcode;

								do {
									if (opcodeID >= script->size()) {
										qWarning() << "ALERT wrong script size";
										goto next_script;
									}

									opcode = script->opcode(opcodeID++);

								} while (!opcode.getWindow(win));

								const quint8 minDiff = 1, maxDiff = 5;
								int diffX = qAbs(win2.x - win.x), diffY = qAbs(win2.y - win.y);
								bool hasTinyDiffX = diffX >= minDiff && diffX <= maxDiff,
										hasTinyDiffY = diffY >= minDiff && diffY <= maxDiff,
										hasLargeDiffX = diffX > 15,//maxDiff,
										hasLargeDiffY = diffY > 15;//maxDiff;
								if ((hasTinyDiffX && hasLargeDiffY) || (hasTinyDiffY && hasLargeDiffX)) {
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
				if (!tmp.mkdir(fieldDir)) {
					qWarning() << "cannot create dir" << tmp.absoluteFilePath(fieldDir);
				}
				tmp.cd(fieldDir);

				scriptsAndTexts->clearTexts();
				int grpScriptID = 0;
				for (GrpScript *grp : scriptsAndTexts->grpScripts()) {
					QString grpFilename = QString("%1-%2").arg(grpScriptID).arg(grp->name());
					QFile out(tmp.absoluteFilePath(grpFilename));
					if (out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
						out.write(grp->toString(field).toLatin1());
						out.close();
					}
					grpScriptID++;
				}

				tmp.cdUp();*/
				/*QFile out(tmp.absoluteFilePath(fieldDir));
				if (out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {

					int textID = 0;
					for (const FF7String &text : scriptsAndTexts->texts()) {
						out.write(QString("=== Texte %1 ===\n").arg(textID++).toLatin1());
						out.write(text.text(false).toLatin1());
						out.write("\n");
					}

					out.close();
				}*/
			}
			/*FieldPC *fieldPC = (FieldPC *)field;
			FieldModelLoaderPC *modelLoader = fieldPC->fieldModelLoader();
			if (modelLoader->isOpen()) {
				int modelID = 0;
				for (const QString &HRC : modelLoader->HRCNames()) {
					for (const QString &a : modelLoader->ANames(modelID)) {
						FieldModelFilePC *fieldModel = fieldPC->fieldModel(HRC, a);
						if (fieldModel->isOpen()) {
							if (fieldModel->animBoneCount() > 1 && fieldModel->animBoneCount() != fieldModel->boneCount()) {
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

bool FieldArchive::find(bool (*predicate)(Field *, SearchQuery *, SearchIn *),
						SearchQuery *toSearch, int &mapID, SearchIn *searchIn,
						Sorting sorting, SearchScope scope)
{
	Q_UNUSED(sorting)
	FieldArchiveIterator it(*this);
	if (!it.seek(mapID)) {
		return false;
	}

	while (it.hasNext()) {
		Field *f = it.next();
		mapID = it.mapId();
		if (f != nullptr && (*predicate)(f, toSearch, searchIn)) {
			return true;
		}
		searchIn->reset();
		if (scope >= FieldScope) {
			break;
		}
	}
	return false;
}

bool FieldArchive::findLast(bool (*predicate)(Field *, SearchQuery *, SearchIn *),
						SearchQuery *toSearch, int &mapID, SearchIn *searchIn,
						Sorting sorting, SearchScope scope)
{
	Q_UNUSED(sorting)
	FieldArchiveIterator it(*this);
	if (!it.seek(mapID)) {
		return false;
	}

	while (it.hasPrevious()) {
		Field *f = it.previous();
		mapID = it.key();
		if (f != nullptr && (*predicate)(f, toSearch, searchIn)) {
			return true;
		}
		searchIn->toEnd();
		if (scope >= FieldScope) {
			break;
		}
	}

	return false;
}

bool FieldArchive::searchOpcode(int opcode, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchOpcodeQuery query(opcode);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchOpcodeQuery *query = static_cast<SearchOpcodeQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchOpcode(query->opcode, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::searchVar(quint8 bank, quint16 address, Opcode::Operation op, int value, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchVarQuery query(bank, address, op, value);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchVarQuery *query = static_cast<SearchVarQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchVar(query->bank, query->address, query->op, query->value, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::searchExec(quint8 group, quint8 script, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchExecQuery query(group, script);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchExecQuery *query = static_cast<SearchExecQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchExec(query->group, query->script, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::searchMapJump(int map, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchFieldQuery query(map);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchFieldQuery *query = static_cast<SearchFieldQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchMapJump(quint16(query->mapID), searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::searchTextInScripts(const QRegularExpression &text, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = static_cast<SearchTextQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchTextInScripts(query->text, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::searchText(const QRegularExpression &text, int &mapID, int &textID, qsizetype &from, qsizetype &size, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	qsizetype empty;
	SearchInText searchIn(textID, from, size, empty);

	return find([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = static_cast<SearchTextQuery *>(_query);
		SearchInText *searchIn = static_cast<SearchInText *>(_searchIn);
		return f->scriptsAndTexts()->searchText(query->text, searchIn->textID, searchIn->from, searchIn->size);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::searchOpcodeP(int opcode, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchOpcodeQuery query(opcode);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchOpcodeQuery *query = static_cast<SearchOpcodeQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchOpcodeP(query->opcode, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::searchVarP(quint8 bank, quint16 address, Opcode::Operation op, int value, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchVarQuery query(bank, address, op, value);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchVarQuery *query = static_cast<SearchVarQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchVarP(query->bank, query->address, query->op, query->value, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::searchExecP(quint8 group, quint8 script, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchExecQuery query(group, script);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchExecQuery *query = static_cast<SearchExecQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchExecP(query->group, query->script, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::searchMapJumpP(int map, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchFieldQuery query(map);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchFieldQuery *query = static_cast<SearchFieldQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchMapJumpP(quint16(query->mapID), searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::searchTextInScriptsP(const QRegularExpression &text, int &mapID, int &groupID, int &scriptID, int &opcodeID, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	SearchInScript searchIn(groupID, scriptID, opcodeID);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = static_cast<SearchTextQuery *>(_query);
		SearchInScript *searchIn = static_cast<SearchInScript *>(_searchIn);
		return f->scriptsAndTexts()->searchTextInScriptsP(query->text, searchIn->groupID, searchIn->scriptID, searchIn->opcodeID);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::searchTextP(const QRegularExpression &text, int &mapID, int &textID, qsizetype &from, qsizetype &index, qsizetype &size, Sorting sorting, SearchScope scope)
{
	SearchTextQuery query(text);
	SearchInText searchIn(textID, from, size, index);

	return findLast([](Field *f, SearchQuery *_query, SearchIn *_searchIn) {
		SearchTextQuery *query = static_cast<SearchTextQuery *>(_query);
		SearchInText *searchIn = static_cast<SearchInText *>(_searchIn);
		return f->scriptsAndTexts()->searchTextP(query->text, searchIn->textID, searchIn->from, searchIn->index, searchIn->size);
	}, &query, mapID, &searchIn, sorting, scope);
}

bool FieldArchive::replaceText(const QRegularExpression &search, const QString &after, int mapID, int textID, int from)
{
	if (textID > -1) {
		Field *field = this->field(mapID);
		if (field) {
			Section1File *texts = field->scriptsAndTexts();
			if (texts->isOpen() && textID < texts->textCount()) {
				if (texts->replaceText(search, after, textID, from)) {
					return true;
				}
			}
		}
	}
	return false;
}

bool FieldArchive::compileScripts(int &mapID, int &groupID, int &scriptID, int &opcodeID, QString &errorStr)
{
	FieldArchiveIterator it(*this);

	while (it.hasNext()) {
		Field *field = it.next(false);
		mapID = it.mapId();
		if (field != nullptr && field->isOpen()) {
			Section1File *section1 = field->scriptsAndTexts(false);
			if (section1->isOpen() && section1->isModified() && !section1->compileScripts(groupID, scriptID, opcodeID, errorStr)) {
				return false;
			}
		}
	}

	return true;
}

void FieldArchive::removeBattles()
{
	FieldArchiveIterator it(*this);
	int i = 0;

	observer()->setObserverMaximum(quint32(fileList.size()));

	while (it.hasNext()) {
		if (observer()->observerWasCanceled()) {
			return;
		}
		Field *field = it.next();
		if (field != nullptr && field->encounter()->isOpen()) {
			field->encounter()->setBattleEnabled(EncounterFile::Table1, false);
			field->encounter()->setBattleEnabled(EncounterFile::Table2, false);
			if (!field->isModified()) {
				field->setModified(true);
			}
		}
		observer()->setObserverValue(i++);
	}
}

void FieldArchive::removeTexts()
{
	FieldArchiveIterator it(*this);
	int i = 0;

	observer()->setObserverMaximum(quint32(fileList.size()));

	while (it.hasNext()) {
		if (observer()->observerWasCanceled()) {
			return;
		}
		Field *field = it.next();
		if (field != nullptr && field->scriptsAndTexts()->isOpen()) {
			field->scriptsAndTexts()->removeTexts();
			if (field->scriptsAndTexts()->isModified() && !field->isModified()) {
				field->setModified(true);
			}
		}
		observer()->setObserverValue(i++);
	}
}

void FieldArchive::cleanTexts()
{
	FieldArchiveIterator it(*this);
	int i = 0;

	observer()->setObserverMaximum(quint32(fileList.size()));

	while (it.hasNext()) {
		if (observer()->observerWasCanceled()) {
			return;
		}
		Field *field = it.next();
		if (field != nullptr && field->scriptsAndTexts()->isOpen()) {
			field->scriptsAndTexts()->cleanTexts();
			if (field->scriptsAndTexts()->isModified() && !field->isModified()) {
				field->setModified(true);
			}
		}
		observer()->setObserverValue(i++);
	}
}

void FieldArchive::autosizeTextWindows()
{
	FieldArchiveIterator it(*this);
	int i = 0;

	observer()->setObserverMaximum(quint32(fileList.size()));

	while (it.hasNext()) {
		if (observer()->observerWasCanceled()) {
			return;
		}
		Field *field = it.next();
		if (field != nullptr && field->scriptsAndTexts()->isOpen()) {
			field->scriptsAndTexts()->autosizeTextWindows();
			if (field->scriptsAndTexts()->isModified() && !field->isModified()) {
				field->setModified(true);
			}
		}
		observer()->setObserverValue(i++);
	}
}

bool FieldArchive::exportation(const QList<int> &selectedFields, const QString &directory,
							   bool overwrite, const QMap<ExportType, QString> &toExport,
                               PsfTags *tags)
{
	if (selectedFields.isEmpty() || toExport.isEmpty()) {
		return true;
	}

	QString path, extension;
	int currentField = 0;
	if (observer()) {
		observer()->setObserverMaximum(quint32(selectedFields.size() - 1));
	}

	for (const int &mapID : selectedFields) {
		if (observer() && observer()->observerWasCanceled()) {
			break;
		}
		Field *f = field(mapID);
		if (f) {
			if (toExport.contains(Fields)) {
				extension = toExport.value(Fields);
				path = QDir::cleanPath(extension.isEmpty()
									   ? QString("%1/%2")
										 .arg(directory, f->name())
									   : QString("%1/%2.%3")
										 .arg(directory, f->name(), extension));
				if (overwrite || !QFile::exists(path)) {
					QByteArray fieldData = io()->fieldData(f, io()->isPC() ? QString() : "DAT", extension.compare("dec", Qt::CaseInsensitive) == 0);
					if (!fieldData.isEmpty()) {
						QFile fieldExport(path);
						if (fieldExport.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
							fieldExport.write(fieldData);
							fieldExport.close();
						}
					}
				}
			}
			if (toExport.contains(Backgrounds)) {
				extension = toExport.value(Backgrounds);
				bool exportLayers = extension.endsWith('_');
				path = QDir::cleanPath(QString("%1/%2").arg(directory, f->name()));
				if (exportLayers) {
					extension.truncate(extension.size() - 1);
				} else {
					path.append(QString(".%3").arg(extension));
				}

				if (overwrite || !QFile::exists(path)) {
					BackgroundFile *bg = f->background();
					if (bg->isOpen()) {
						if (exportLayers) {
							bg->exportLayers(path, extension);
						} else {
							QImage background = bg->openBackground();
							if (!background.isNull()) {
								background.save(path);
							}
						}
					}
				}
			}
			if (toExport.contains(Akaos)) {
				TutFileStandard *akaoList = f->tutosAndSounds();
				if (akaoList->isOpen()) {
					int akaoCount = int(akaoList->size());
					for (int i=0; i<akaoCount; ++i) {
						if (!akaoList->isTut(i)) {
							extension = toExport.value(Akaos);
							path = QDir::cleanPath(QString("%1/%2.%3").arg(directory, Data::music_names.value(akaoList->akaoID(i), f->name()), extension));
							if (overwrite || !QFile::exists(path)) {
								QFile tutExport(path);
								if (tutExport.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
									if (extension == "minipsf" || extension == "psf") {
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
			if (toExport.contains(Texts)) {
				Section1File *section1 = f->scriptsAndTexts();
				if (section1->isOpen()) {
					extension = toExport.value(Texts);
					path = QDir::cleanPath(QString("%1/%2.%3").arg(directory, f->name(), extension));
					if (overwrite || !QFile::exists(path)) {
						QFile textExport(path);
						Section1File::ExportFormat format;
						if (extension == "txt") {
							format = Section1File::TXTText;
						} else if (extension == "xml") {
							format = Section1File::XMLText;
						} else {
							return false;
						}

						if (!section1->exporter(&textExport, format)) {
							return false;
						}
					}
				}
			}
			if (toExport.contains(Chunks)) {
				path = QDir::cleanPath(directory);
				extension = toExport.value(Chunks);
				Field::FieldSection section;
				if (extension == "chunk.1") {
					section = Field::Scripts;
				} else if (extension == "chunk.2") {
					section = Field::Camera;
				} else if (extension == "chunk.3") {
					section = Field::ModelLoader;
				} else if (extension == "chunk.4") {
					section = Field::PalettePC;
				} else if (extension == "chunk.5") {
					section = Field::Walkmesh;
				} else if (extension == "chunk.6") {
					section = Field::Tiles;
				} else if (extension == "chunk.7") {
					section = Field::Encounter;
				} else if (extension == "chunk.8") {
					section = Field::Inf;
				} else if (extension == "chunk.9") {
					section = Field::Background;
				} else {
					return false;
				}
				QDir dir(path);
				if (!dir.exists()) {
					dir.mkpath("./");
				}
				if (!f->exportToChunks(dir, section)) {
					return false;
				}
			}
		}
		if (observer()) {
			observer()->setObserverValue(currentField++);
		}
	}

	return true;
}

bool FieldArchive::importation(const QList<int> &selectedFields, const QString &directory,
							   const QMap<Field::FieldSection, QString> &toImport)
{
	Q_UNUSED(directory) //TODO

	if (selectedFields.isEmpty() || toImport.isEmpty()) {
		return true;
	}
	int currentField = 0;

	for (const int &mapID : selectedFields) {
		if (observer()->observerWasCanceled()) 	break;

		Field *f = field(mapID);
		if (f) {
			if (toImport.contains(Field::Scripts)) {
				Section1File *section1 = f->scriptsAndTexts();
				if (section1->isOpen()) {
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
	for (Field *field : std::as_const(fileList)) {
		if (field != nullptr) {
			field->setSaved();
		}
	}
	_mapList.setSaved();
}

QString FieldArchive::mapName(int mapID) const
{
	if (mapID < _mapList.mapNames().size()) {
		return _mapList.mapNames().at(mapID);
	}
	
	return Data::mapName(mapID, isPC());
}
