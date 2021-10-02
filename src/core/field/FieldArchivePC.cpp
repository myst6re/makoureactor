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
#include "FieldArchivePC.h"
#include "BackgroundFilePC.h"

FieldArchivePC::FieldArchivePC() :
	FieldArchive()
{
}

FieldArchivePC::FieldArchivePC(const QString &path, FieldArchiveIO::Type type) :
	FieldArchive()
{
	switch (type) {
	case FieldArchiveIO::Lgp:
		setIO(new FieldArchiveIOPCLgp(path, this));
		break;
	case FieldArchiveIO::File:
		setIO(new FieldArchiveIOPCFile(path, this));
		break;
	case FieldArchiveIO::Dir:
		setIO(new FieldArchiveIOPCDir(path, this));
		break;
	default:
		break;
	}
}

FieldArchivePC::~FieldArchivePC()
{
	for (TutFile *tut : qAsConst(_tuts)) {
		if (tut != nullptr) {
			delete tut;
		}
	}
}

void FieldArchivePC::clear()
{
	for (TutFile *tut : qAsConst(_tuts)) {
		if (tut != nullptr) {
			delete tut;
		}
	}

	_tuts.clear();

	FieldArchive::clear();
}

const FieldPC *FieldArchivePC::field(quint32 id) const
{
	return static_cast<const FieldPC *>(FieldArchive::field(id));
}

FieldPC *FieldArchivePC::field(quint32 id, bool open, bool dontOptimize)
{
	return static_cast<FieldPC *>(FieldArchive::field(id, open, dontOptimize));
}

TutFilePC *FieldArchivePC::tut(const QString &name)
{
	QMapIterator<QString, TutFilePC *> it(_tuts);

	while (it.hasNext()) {
		it.next();

		const QString &tutName = it.key();

		if (name.startsWith(tutName, Qt::CaseInsensitive)) {
			TutFilePC *tutFile = it.value();
			if (tutFile == nullptr) {
				QByteArray data = io()->fileData(tutName + ".tut", false);
				if (!data.isEmpty()) {
					tutFile = new TutFilePC();
					if (!static_cast<TutFile *>(tutFile)->open(data)) {
						delete tutFile;
						return nullptr;
					}
					_tuts.insert(tutName, tutFile);
					return tutFile;
				} else {
					return nullptr;
				}
			} else {
				return tutFile;
			}
		}
	}

	return nullptr;
}

const QMap<QString, TutFilePC *> &FieldArchivePC::tuts() const
{
	return _tuts;
}

void FieldArchivePC::addTut(const QString &name)
{
	_tuts.insert(name, nullptr);
}

void FieldArchivePC::setSaved()
{
	for (TutFile *tut : qAsConst(_tuts)) {
		if (tut != nullptr) {
			tut->setModified(false);
		}
	}

	FieldArchive::setSaved();
}

FieldArchiveIOPC *FieldArchivePC::io() const
{
	return static_cast<FieldArchiveIOPC *>(FieldArchive::io());
}

void FieldArchivePC::cleanModelLoader()
{
	FieldArchiveIterator it(*this);
	int i = 0;

	observer()->setObserverMaximum(size());

	while (it.hasNext()) {
		if (observer()->observerWasCanceled()) {
			return;
		}
		FieldPC *field = static_cast<FieldPC *>(it.next());
		if (field != nullptr) {
			FieldModelLoaderPC *modelLoader = field->fieldModelLoader();
			if (modelLoader->isOpen()) {
				modelLoader->clean();
				if (modelLoader->isModified() && !field->isModified()) {
					field->setModified(true);
				}
			}
		}
		observer()->setObserverValue(i++);
	}
}

void FieldArchivePC::removeUnusedSections()
{
	FieldArchiveIterator it(*this);
	int i = 0;

	observer()->setObserverMaximum(size());

	while (it.hasNext()) {
		if (observer()->observerWasCanceled()) {
			return;
		}
		FieldPC *field = static_cast<FieldPC *>(it.next());
		if (field != nullptr) {
			field->setRemoveUnusedSection(true);
			field->setModified(true);
		}
		observer()->setObserverValue(i++);
	}
}

void FieldArchivePC::repairBackgroundsPC()
{
	FieldArchiveIterator it(*this);
	int i = 0;

	observer()->setObserverMaximum(size());

	while (it.hasNext()) {
		if (observer()->observerWasCanceled()) {
			return;
		}
		FieldPC *field = static_cast<FieldPC *>(it.next());
		if (field != nullptr && (field->name().toLower() == "lastmap" || field->name().toLower() == "fr_e")) {
			BackgroundFilePC *bg = static_cast<BackgroundFilePC *>(field->background());
			if (bg->isOpen() && bg->repair()) {
				field->setModified(true);
			}
		}
		observer()->setObserverValue(i++);
	}
}
