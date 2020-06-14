#include "CharArchive.h"
#include "AFile.h"
#include "Data.h"

CharArchive::CharArchive() :
    _io(new Lgp()), _delete(true)
{
}

CharArchive::CharArchive(Lgp *io) :
      _io(io), _delete(false)
{
}

CharArchive::CharArchive(const QString &filename) :
      _io(new Lgp(filename)), _delete(true)
{
}

CharArchive::~CharArchive()
{
	if (_delete) {
		delete _io;
	}
}

CharArchive *CharArchive::_instance = 0;

CharArchive *CharArchive::instance()
{
	if (!_instance) {
		_instance = new CharArchive(Data::charlgp_path());
	}
	if (!_instance->isOpen()) {
		_instance->open();
	}
	return _instance;
}

void CharArchive::close()
{
	_io->clear();
	_io->close();
	_animBoneCount.clear();
}

QStringList CharArchive::hrcFiles() const
{
	QStringList files;

	foreach(const QString &file, _io->fileList()) {
		if(file.endsWith(".hrc", Qt::CaseInsensitive)) {
			files.append(file.toUpper());
		}
	}

	return files;
}

QStringList CharArchive::aFiles(int boneCount)
{
	if(boneCount >= 0 && openAnimBoneCount()) {
		return _animBoneCount.values(boneCount);
	}

	QStringList files;

	foreach(const QString &file, _io->fileList()) {
		if(file.endsWith(".a", Qt::CaseInsensitive)) {
			files.append(file.left(file.size()-2).toUpper());
		}
	}

	return files;
}

QIODevice *CharArchive::fileIO(const QString &filename)
{
	return _io->file(filename.toLower());
}

bool CharArchive::openAnimBoneCount()
{
	if (!isOpen()) {
		qWarning() << "CharArchive::openAnimBoneCount" << "archive not opened";
		return false;
	}

	_animBoneCount.clear();

	LgpIterator it = _io->iterator();
	while(it.hasNext()) {
		it.next();
		const QString &fileName = it.fileName();
		if(fileName.endsWith(".a", Qt::CaseInsensitive)) {
			QCoreApplication::processEvents();
			QIODevice *aFile = it.file();
			if(aFile && aFile->open(QIODevice::ReadOnly)) {
				AFile a(aFile);
				AHeader header;

				if(!a.readHeader(header)) {
					qWarning() << "CharArchive::openAnimBoneCount" << "animation error" << fileName;
					continue;
				}

				_animBoneCount.insert(header.boneCount, fileName.left(fileName.size()-2).toUpper());
			} else {
				return false;
			}
		}
	}

	return true;
}

