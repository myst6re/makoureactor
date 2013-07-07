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
	return (FieldArchivePC *)FieldArchiveIO::fieldArchive();
}

FieldArchiveIOPCLgp::FieldArchiveIOPCLgp(const QString &path, FieldArchivePC *fieldArchive) :
	FieldArchiveIOPC(fieldArchive), _lgp(path)
{
}

QString FieldArchiveIOPCLgp::path() const
{
	return _lgp.fileName();
}

void *FieldArchiveIOPCLgp::device()
{
	return &_lgp;
}

QByteArray FieldArchiveIOPCLgp::fieldData2(Field *field, bool unlzs)
{
	return fileData(field->name(), unlzs);
}

QByteArray FieldArchiveIOPCLgp::fileData2(const QString &fileName)
{
	if(!_lgp.isOpen() && !_lgp.open()) return QByteArray();
	return _lgp.modifiedFileData(fileName);
}

void FieldArchiveIOPCLgp::close()
{
	_lgp.close();
	FieldArchiveIO::close();
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCLgp::open2(FieldArchiveIOObserver *observer)
{
	if(!_lgp.isOpen() && !_lgp.open()) {
		return ErrorOpening;
	}

	QStringList archiveList = _lgp.fileList();

	if(archiveList.isEmpty()) {
		return Invalid;
	}

	if(observer)	observer->setObserverMaximum(archiveList.size());

	quint32 i, freq = archiveList.size()>50 ? archiveList.size()/50 : 1;

//	QTime t;t.start();

	i = 0;
	foreach(const QString &name, archiveList) {
		if(i % freq == 0) {
			if(observer) {
				if(observer->observerWasCanceled()) {
					return Aborted;
				}
				observer->setObserverValue(i);
			}
		}

		if(name.compare("maplist", Qt::CaseInsensitive) == 0) {
			if(!Data::openMaplist(_lgp.fileData(name))) {
				qWarning() << "Cannot open maplist!";
			}
		} else if(name.endsWith(".tut", Qt::CaseInsensitive)) {
			fieldArchive()->addTut(name.toLower().left(name.size()-4));
		} else if(!name.contains('.')) {
			fieldArchive()->appendField(new FieldPC(name, this));
			++i;
		}
	}
//	qDebug("Ouverture : %d ms", t.elapsed());
	// qDebug() << FICHIER::size << " o";
	// qDebug() << "nbGS : " << GrpScript::COUNT;
	// qDebug() << "nbS : " << Script::COUNT;
	// qDebug() << "nbC : " << Opcode::COUNT;
	// qDebug("-------------------------------------------------");

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCLgp::save2(const QString &path, FieldArchiveIOObserver *observer)
{
	if(!_lgp.isOpen() && !_lgp.open()) {
		return ErrorOpening;
	}

	for(int fieldID=0 ; fieldID<fieldArchive()->size() ; ++fieldID) {
		if(observer && observer->observerWasCanceled()) {
			return Aborted;
		}
		Field *field = fieldArchive()->field(fieldID, false);
		if(field && field->isOpen() && field->isModified()) {
			if(!_lgp.setFile(field->name(), new FieldSaveIO(field))) {
				return FieldNotFound;
			}
		}
	}

	QMapIterator<QString, TutFilePC *> itTut(fieldArchive()->tuts());

	while(itTut.hasNext()) {
		itTut.next();
		TutFile *tut = itTut.value();

		if(tut != NULL && tut->isModified()) {
			if(!_lgp.setFileData(itTut.key() + ".tut", tut->save())) {
				return FieldNotFound;
			}
		}
	}

	this->observer = observer;

	if(!_lgp.pack(path, this)) {
		this->observer = 0;

		switch(_lgp.error()) {
		case Lgp::OpenError:
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

	this->observer = 0;

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

void *FieldArchiveIOPCFile::device()
{
	return &fic;
}

QByteArray FieldArchiveIOPCFile::fieldData2(Field *field, bool unlzs)
{
	Q_UNUSED(field)
	return fileData(QString(), unlzs);
}

QByteArray FieldArchiveIOPCFile::fileData2(const QString &fileName)
{
	Q_UNUSED(fileName)
	if(!fic.isOpen() && !fic.open(QIODevice::ReadOnly))		return QByteArray();
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

FieldArchiveIO::ErrorCode FieldArchiveIOPCFile::open2(FieldArchiveIOObserver *observer)
{
	Q_UNUSED(observer)

	QString name = this->name();
	fieldArchive()->appendField(new FieldPC(name.left(name.lastIndexOf('.')), this));

	return fieldArchive()->field(0)->isOpen() ? Ok : Invalid;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCFile::save2(const QString &path0, FieldArchiveIOObserver *observer)
{
	Q_UNUSED(observer)
	QString path = path0.isNull() ? fic.fileName() : path0;

	Field *field = fieldArchive()->field(0, false);
	if(field && field->isOpen() && field->isModified()) {
		qint8 err = field->save(path, true);
		if(err == 2)	return ErrorOpening;
		if(err == 1)	return Invalid;
		if(err != 0)	return NotImplemented;
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

void *FieldArchiveIOPCDir::device()
{
	return &dir;
}

QByteArray FieldArchiveIOPCDir::fieldData2(Field *field, bool unlzs)
{
	return fileData(field->name(), unlzs);
}

QByteArray FieldArchiveIOPCDir::fileData2(const QString &fileName)
{
	QByteArray data;

	QFile f(dir.filePath(fileName));
	if(!f.open(QIODevice::ReadOnly))	return QByteArray();
	data = f.readAll();
	f.close();

	return data;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCDir::open2(FieldArchiveIOObserver *observer)
{
	QStringList list;
	list.append("*");
	list = dir.entryList(list, QDir::Files | QDir::NoSymLinks);

	if(observer)	observer->setObserverMaximum(list.size());

	// QTime t;t.start();

	int i=0;
	foreach(const QString &name, list) {
		if(observer) {
			if(observer->observerWasCanceled()) {
				return Aborted;
			}
			observer->setObserverValue(i++);
		}

		if(name.compare("maplist", Qt::CaseInsensitive) == 0) {
			if(!Data::openMaplist(fileData2(name))) {
				qWarning() << "Cannot open maplist!";
			}
		} else if(name.endsWith(".tut", Qt::CaseInsensitive)) {
			fieldArchive()->addTut(name.toLower().left(name.size()-4));
		} else if(!name.contains(".")) {
			fieldArchive()->appendField(new FieldPC(name, this));
		}
	}

	// qDebug("Opening: %d ms", t.elapsed());

	return Ok;
}

FieldArchiveIO::ErrorCode FieldArchiveIOPCDir::save2(const QString &path, FieldArchiveIOObserver *observer)
{
	bool saveAs;

	if(!path.isEmpty()) {
		saveAs = QDir::cleanPath(path) != QDir::cleanPath(dir.path());
	} else {
		saveAs = false;
	}

	quint32 nbFiles = fieldArchive()->size();
	if(observer)	observer->setObserverMaximum(nbFiles);

	for(quint32 fieldID=0 ; fieldID<nbFiles ; ++fieldID) {
		if(observer && observer->observerWasCanceled()) {
			return Aborted;
		}
		Field *field = fieldArchive()->field(fieldID, false);
		if(field) {
			QString fileName = field->name();
			QString filePath = dir.filePath(fileName);
			if(field && field->isOpen() && field->isModified()) {
				qint8 err = field->save(filePath, true);
				if(err == 2)	return ErrorOpening;
				if(err == 1)	return Invalid;
				if(err != 0)	return NotImplemented;
			} else if(saveAs) {
				QString dstPath = path + "/" + fileName;
				if(!QFile::copy(filePath, dstPath)) {
					return ErrorCopying;
				}
			}
		}
		if(observer)	observer->setObserverValue(fieldID);
	}

	QMapIterator<QString, TutFilePC *> itTut(fieldArchive()->tuts());

	while(itTut.hasNext()) {
		itTut.next();
		TutFile *tut = itTut.value();

		if(tut != NULL && tut->isModified()) {
			QFile tutFile(dir.filePath(itTut.key() + ".tut"));
			if(tutFile.open(QIODevice::WriteOnly)) {
				tutFile.write(tut->save());
				tutFile.close();
			} else {
				return ErrorOpening;
			}
		}
	}

	return Ok;
}
