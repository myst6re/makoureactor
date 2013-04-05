#include "FieldArchivePC.h"

FieldArchivePC::FieldArchivePC() :
	FieldArchive()
{
}

FieldArchivePC::FieldArchivePC(const QString &path, bool isDirectory) :
	FieldArchive(path, isDirectory)
{
}

FieldArchivePC::~FieldArchivePC()
{
	foreach(TutFile *tut, _tuts) {
		if(tut != NULL)	delete tut;
	}
}

void FieldArchivePC::clear()
{
	foreach(TutFile *tut, _tuts) {
		if(tut != NULL)	delete tut;
	}

	_tuts.clear();

	FieldArchive::clear();
}

TutFile *FieldArchivePC::tut(const QString &name)
{
	if(!io() || io()->type() != FieldArchiveIO::Lgp) {
		return NULL;
	}

	Lgp *lgp = (Lgp *)io()->device();

	QMapIterator<QString, TutFile *> it(_tuts);

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
					_tuts.insert(name, tutFile);
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

const QMap<QString, TutFile *> &FieldArchivePC::tuts() const
{
	return _tuts;
}

void FieldArchivePC::addTut(const QString &name)
{
	_tuts.insert(name, NULL);
}

void FieldArchivePC::setSaved()
{
	foreach(TutFile *tut, _tuts) {
		if(tut != NULL) {
			tut->setModified(false);
		}
	}

	FieldArchive::setSaved();
}

FieldArchiveIO *FieldArchivePC::io() const
{
	return FieldArchive::io();
}
