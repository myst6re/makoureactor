/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
#include "FieldArchiveIOPC.h"
#include "FieldPC.h"
#include "FieldArchivePC.h"
#include "FieldIO.h"
#include "Data.h"

FieldArchiveIOPC::FieldArchiveIOPC(FieldArchivePC *fieldArchive) :
	FieldArchiveIO(fieldArchive)
{
}

FieldArchivePC *FieldArchiveIOPC::fieldArchive()
{
	return static_cast<FieldArchivePC *>(FieldArchiveIO::fieldArchive());
}

FieldArchiveIOPCLgp::FieldArchiveIOPCLgp(const QString &path, FieldArchivePC *fieldArchive) :
	FieldArchiveIOPC(fieldArchive), _lgp(path), observer(nullptr)
{
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCLgp::addField(const QString &fileName,
                                                        const QString &name)
{
	if (_lgp.addFile(name, new QFile(fileName))) {
		return FieldArchiveIO::Ok;
	}
	return FieldArchiveIO::FieldExists;
}

QString FieldArchiveIOPCLgp::path() const
{
	return _lgp.fileName();
}

Archive *FieldArchiveIOPCLgp::device()
{
	return &_lgp;
}

QByteArray FieldArchiveIOPCLgp::fieldData2(Field *field, const QString &extension, bool unlzs)
{
	Q_UNUSED(extension);
	return fileData(field->name(), unlzs);
}

QByteArray FieldArchiveIOPCLgp::fileData2(const QString &fileName)
{
	if (!_lgp.isOpen() && !_lgp.open()) return QByteArray();
	QByteArray data = _lgp.fileData(fileName);
	if (data.isEmpty()) {
		return _lgp.modifiedFileData(fileName);
	}
	return data;
}

void FieldArchiveIOPCLgp::close()
{
	_lgp.close();
	FieldArchiveIO::close();
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCLgp::open2(ArchiveObserver *observer)
{
	if (!_lgp.isOpen() && !_lgp.open()) {
		return ErrorOpening;
	}

	QStringList archiveList = _lgp.fileList();

	if (archiveList.isEmpty()) {
		return Invalid;
	}

	if (observer) {
		observer->setObserverMaximum(uint(archiveList.size()));
	}

	quint32 i, freq = archiveList.size() > 50 ? quint32(archiveList.size() / 50) : 1;
	QString maplistName = "maplist";

	if (!_lgp.fileExists(maplistName)) {
		qWarning() << "cannot find" << maplistName << "file";
		return FieldNotFound;
	}

	if (!Data::openMaplist(_lgp.fileData(maplistName))) {
		qWarning() << "Cannot open" << maplistName;
		return FieldNotFound;
	}

	QStringList maplist = Data::maplist();

//	QTime t;t.start();

	i = 0;
	for (const QString &name : qAsConst(archiveList)) {
		if (i % freq == 0) {
			if (observer) {
				if (observer->observerWasCanceled()) {
					return Aborted;
				}
				observer->setObserverValue(int(i));
			}
		}

		if (name.compare(maplistName, Qt::CaseInsensitive) == 0) {
		} else if (name.endsWith(".tut", Qt::CaseInsensitive)) {
			fieldArchive()->addTut(name.toLower().left(name.size()-4));
		} else if (!name.contains('.')) {
			fieldArchive()->appendField(new FieldPC(name, this));
			maplist.removeOne(name);
			++i;
		}
	}

	// Adding missing fields
	QStringList ml = maplist.mid(65);
	for (const QString &name : ml) {
		fieldArchive()->appendField(new FieldPC(name));
	}

//	qDebug("Ouverture : %d ms", t.elapsed());
	// qDebug() << FICHIER::size << " o";
	// qDebug() << "nbGS : " << GrpScript::COUNT;
	// qDebug() << "nbS : " << Script::COUNT;
	// qDebug() << "nbC : " << Opcode::COUNT;
	// qDebug("-------------------------------------------------");

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCLgp::save2(const QString &path, ArchiveObserver *observer)
{
	if (!_lgp.isOpen() && !_lgp.open()) {
		return ErrorOpening;
	}
	FieldArchiveIterator it(*(fieldArchive()));
	bool oneFieldAdded = false;

	while (it.hasNext()) {
		if (observer && observer->observerWasCanceled()) {
			return Aborted;
		}
		Field *field = it.next(false);
		if (field && field->isOpen() && field->isModified()) {
			if (_lgp.fileExists(field->name())) {
				if (!_lgp.setFile(field->name(), new FieldSaveIO(field))) {
					return ErrorOpening;
				}
			} else if (!_lgp.addFile(field->name(), new FieldSaveIO(field))) {
				return ErrorOpening;
			} else {
				oneFieldAdded = true;
			}
		}
	}

	if (oneFieldAdded && _lgp.fileExists("maplist")) {
		QStringList maplistCopy = Data::maplist();
		// Check if the list was correctly opened before // FIXME
		if (Data::openMaplist(_lgp.fileData("maplist"))) {
			Data::setMaplist(maplistCopy);
			QByteArray mapListData;
			if (Data::saveMaplist(mapListData)){
				_lgp.setFileData("maplist", mapListData);
			} else {
				return Invalid;
			}
		}
	}

	QMapIterator<QString, TutFilePC *> itTut(fieldArchive()->tuts());

	while (itTut.hasNext()) {
		itTut.next();
		TutFile *tut = itTut.value();

		if (tut != nullptr && tut->isModified()) {
			if (!_lgp.setFileData(itTut.key() + ".tut", tut->save())) {
				return FieldNotFound;
			}
		}
	}

	this->observer = observer;

	if (!_lgp.pack(path, this)) {
		this->observer = nullptr;

		switch (_lgp.error()) {
		case Lgp::OpenError:
			return ErrorOpening;
		case Lgp::OpenTempError:
			return ErrorOpeningTemp;
		case Lgp::InvalidError:
			return Invalid;
		case Lgp::CopyError:
			return ErrorCopying;
		case Lgp::RemoveError:
			return ErrorRemoving;
		case Lgp::RenameError:
			return ErrorRenaming;
		case Lgp::AbortError:
			return Aborted;
		default:
			return Invalid;
		}
	}

	this->observer = nullptr;

	return Ok;
}

FieldArchiveIOPCFile::FieldArchiveIOPCFile(const QString &path, FieldArchivePC *fieldArchive) :
	FieldArchiveIOPC(fieldArchive), fic(path)
{
}

QString FieldArchiveIOPCFile::path() const
{
	return fic.fileName();
}

Archive *FieldArchiveIOPCFile::device()
{
	return nullptr;
}

QByteArray FieldArchiveIOPCFile::fieldData2(Field *field, const QString &extension, bool unlzs)
{
	Q_UNUSED(field)
	Q_UNUSED(extension)
	return fileData(QString(), unlzs);
}

QByteArray FieldArchiveIOPCFile::fileData2(const QString &fileName)
{
	Q_UNUSED(fileName)
	if (!fic.isOpen() && !fic.open(QIODevice::ReadOnly))		return QByteArray();
	fic.reset();
	QByteArray data = fic.readAll();
	fic.close();

	return data;
}

void FieldArchiveIOPCFile::close()
{
	fic.close();
	FieldArchiveIO::close();
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCFile::open2(ArchiveObserver *observer)
{
	Q_UNUSED(observer)

	QString name = this->name();
	fieldArchive()->appendField(new FieldPC(name.left(name.lastIndexOf('.')), this));

	// Open field
	FieldArchiveIterator it(*(fieldArchive()));
	Field *field = it.next();

	return field && field->isOpen() ? Ok : Invalid;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCFile::save2(const QString &path0, ArchiveObserver *observer)
{
	Q_UNUSED(observer)
	QString path = path0.isNull() ? fic.fileName() : path0;
	bool saveAs = !path0.isNull() && QFileInfo(path0) != QFileInfo(fic);

	FieldArchiveIterator it(*(fieldArchive()));
	Field *field = it.next(false);

	if (field && field->isOpen() && field->isModified()) {
		qint8 err = field->save(path, true);
		if (err == 2)	return ErrorOpening;
		if (err == 1)	return Invalid;
		if (err != 0)	return NotImplemented;
	} else if (saveAs) {
		if (QFile::exists(path0) && !QFile::remove(path0)) {
			return ErrorRemoving;
		}
		if (!fic.copy(path0)) {
			return ErrorCopying;
		}
	}

	if (saveAs) {
		fic.close();
		fic.setFileName(path0);
	}

	return Ok;
}

FieldArchiveIOPCDir::FieldArchiveIOPCDir(const QString &path, FieldArchivePC *fieldArchive) :
	FieldArchiveIOPC(fieldArchive), dir(path)
{
}

QString FieldArchiveIOPCDir::path() const
{
	return dir.path();
}

Archive *FieldArchiveIOPCDir::device()
{
	return nullptr;
}

QByteArray FieldArchiveIOPCDir::fieldData2(Field *field, const QString &extension, bool unlzs)
{
	Q_UNUSED(extension)
	return fileData(field->name(), unlzs);
}

QByteArray FieldArchiveIOPCDir::fileData2(const QString &fileName)
{
	QByteArray data;

	QFile f(dir.filePath(fileName));
	if (!f.open(QIODevice::ReadOnly))	return QByteArray();
	data = f.readAll();
	f.close();

	return data;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCDir::open2(ArchiveObserver *observer)
{
	QStringList list;
	list.append("*");
	list = dir.entryList(list, QDir::Files | QDir::NoSymLinks);

	if (observer) {
		observer->setObserverMaximum(uint(list.size()));
	}

	// QTime t;t.start();

	int i = 0;
	for (const QString &name : qAsConst(list)) {
		if (observer) {
			if (observer->observerWasCanceled()) {
				return Aborted;
			}
			observer->setObserverValue(i++);
		}

		if (name.compare("maplist", Qt::CaseInsensitive) == 0) {
			if (!Data::openMaplist(fileData2(name))) {
				qWarning() << "Cannot open maplist!";
			}
		} else if (name.endsWith(".tut", Qt::CaseInsensitive)) {
			fieldArchive()->addTut(name.toLower().left(name.size()-4));
		} else if (!name.contains(".")) {
			fieldArchive()->appendField(new FieldPC(name, this));
		}
	}

	// qDebug("Opening: %d ms", t.elapsed());

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCDir::save2(const QString &path, ArchiveObserver *observer)
{
	bool saveAs;

	if (!path.isEmpty()) {
		saveAs = QDir::cleanPath(path) != QDir::cleanPath(dir.path());
	} else {
		saveAs = false;
	}

	int i = 0;
	FieldArchiveIterator it(*(fieldArchive()));

	if (observer) {
		observer->setObserverMaximum(uint(fieldArchive()->size()));
	}

	while (it.hasNext()) {
		if (observer && observer->observerWasCanceled()) {
			return Aborted;
		}
		Field *field = it.next(false);
		if (field) {
			QString fileName = field->name(),
			        filePath = dir.filePath(fileName);
			if (field->isOpen() && field->isModified()) {
				qint8 err = field->save(filePath, true);
				if (err == 2)	return ErrorOpening;
				if (err == 1)	return Invalid;
				if (err != 0)	return NotImplemented;
			} else if (saveAs) {
				QString dstPath = path + "/" + fileName;
				if (!QFile::copy(filePath, dstPath)) {
					return ErrorCopying;
				}
			}
		}
		if (observer) {
			observer->setObserverValue(i++);
		}
	}

	QMapIterator<QString, TutFilePC *> itTut(fieldArchive()->tuts());

	while (itTut.hasNext()) {
		itTut.next();
		TutFile *tut = itTut.value();

		if (tut != nullptr && tut->isModified()) {
			QFile tutFile(dir.filePath(itTut.key() + ".tut"));
			if (tutFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				tutFile.write(tut->save());
				tutFile.close();
			} else {
				return ErrorOpening;
			}
		}
	}

	if (saveAs) {
		dir.setPath(path);
	}

	return Ok;
}
