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
#pragma once

#include <QtCore>

struct ArchiveObserver
{
	ArchiveObserver() {}
	virtual ~ArchiveObserver();
	virtual bool observerWasCanceled() const=0;
	virtual void setObserverMaximum(unsigned int max)=0;
	virtual void setObserverValue(int value)=0;
	virtual bool observerRetry(const QString &message)=0;
};

class Archive
{
public:
	enum ArchiveError {
		NoError,
		ReadError,
		WriteError,
		OpenError,
		AbortError,
		RemoveError,
		RenameError,
		PositionError,
		ResizeError,
		PermissionsError,
		CopyError,
		InvalidError,
		FileNotFoundError
	};

	Archive();
	explicit Archive(const QString &filename);
	explicit Archive(QFile *device);
	virtual ~Archive();
	virtual inline void clear() {}
	virtual QStringList fileList() const=0;
	virtual int fileCount() const=0;
	virtual bool fileExists(const QString &filePath) const=0;
	virtual QIODevice *file(const QString &filePath)=0;
	QByteArray fileData(const QString &filePath);
	virtual QIODevice *modifiedFile(const QString &filePath)=0;
	QByteArray modifiedFileData(const QString &filePath);
	virtual bool setFile(const QString &filePath, QIODevice *data)=0;
	bool setFileData(const QString &filePath, const QByteArray &data);
	virtual bool addFile(const QString &filePath, QIODevice *data)=0;
	bool addFileData(const QString &filePath, const QByteArray &data);
	virtual bool removeFile(const QString &filePath)=0;
	virtual bool isNameValid(const QString &filePath) const=0;
	virtual bool renameFile(const QString &filePath, const QString &newFilePath)=0;
	virtual bool open();
	virtual bool isOpen() const;
	virtual void close();
	QString fileName() const;
	void setFileName(const QString &fileName);
	virtual bool pack(const QString &destination=QString(), ArchiveObserver *observer=nullptr)=0;
	ArchiveError error() const;
	QString errorString() const;
protected:
	virtual bool openHeader()=0;
	inline void setErrorString(const QString &errorString) {
		_errorString = errorString;
	}
	void setError(ArchiveError error, const QString &errorString=QString());
	inline QFile *archiveIO() const {
		return _archiveIO;
	}

private:
	Q_DISABLE_COPY(Archive)
	QString _errorString;
	ArchiveError _error;
	QFile *_archiveIO;
};
