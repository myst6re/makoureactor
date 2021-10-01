#pragma once

#include "core/Lgp.h"

class CharArchive
{
public:
	CharArchive();
	explicit CharArchive(Lgp *io);
	explicit CharArchive(const QString &filename);
	virtual ~CharArchive();

	static CharArchive *instance();

	inline bool isOpen() const {
		return _io->isOpen();
	}
	inline bool open() {
		return _io->open();
	}
	void close();
	inline QString filename() const {
		return _io->fileName();
	}
	inline void setFilename(const QString &filename) {
		_io->setFileName(filename);
	}
	QStringList hrcFiles() const;
	QStringList aFiles(int boneCount = -1);
	QIODevice *fileIO(const QString &filename);

private:
	bool openAnimBoneCount();
	Lgp *_io;
	QMultiHash<int, QString> _animBoneCount;
	bool _delete;
	static CharArchive *_instance;
};
