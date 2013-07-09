#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <QtCore>

struct ArchiveObserver
{
	ArchiveObserver() {}
	virtual bool observerWasCanceled() const=0;
	virtual void setObserverMaximum(unsigned int max)=0;
	virtual void setObserverValue(int value)=0;
};

class Archive
{
public:

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
	virtual bool pack(const QString &destination=QString(), ArchiveObserver *observer=NULL)=0;
	QString errorString() const;
protected:
	virtual bool openHeader()=0;
	inline void setErrorString(const QString &errorString) {
		_errorString = errorString;
	}
	inline QFile *archiveIO() const {
		return _archiveIO;
	}

private:
	QString _errorString;
	QFile *_archiveIO;
};

#endif // ARCHIVE_H
