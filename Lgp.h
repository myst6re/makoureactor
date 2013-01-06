#ifndef LGP_H
#define LGP_H

#include <QtCore>

#define LOOKUP_VALUE_MAX 30
#define LOOKUP_TABLE_ENTRIES LOOKUP_VALUE_MAX * LOOKUP_VALUE_MAX

#define MAX_CONFLICTS 4096

class LgpObserver
{
public:
	LgpObserver() {}
	virtual bool observerWasCanceled()=0;
	virtual unsigned int observerMaximum()=0;
	virtual void setObserverMaximum(unsigned int max)=0;
	virtual void setObserverValue(int value)=0;
};

class LgpHeaderEntry;

class LgpIO : public QIODevice
{
public:
	LgpIO(QIODevice *lgp, LgpHeaderEntry *header, QObject *parent=0);
	virtual bool open(OpenMode mode);
	virtual qint64 size() const;
protected:
	virtual qint64 readData(char *data, qint64 maxSize);
private:
	qint64 writeData(const char *, qint64) { return -1; }
	QIODevice *_lgp;
	LgpHeaderEntry *_header;
};

class LgpHeaderEntry
{
public:
	LgpHeaderEntry(const QString &fileName, quint32 filePosition);
	virtual ~LgpHeaderEntry();
	const QString &fileName() const;
	const QString &fileDir() const;
	QString filePath() const;
	quint32 filePosition() const;
	qint64 fileSize() const;
	void setFileName(const QString &fileName);
	void setFileDir(const QString &fileDir);
	void setFilePosition(quint32 filePosition);
	void setFileSize(quint32 fileSize);
	QIODevice *file() const;
	void setFile(QIODevice *io);
private:
	QString _fileName;
	QString _fileDir;
	quint32 _filePosition;
	quint32 _fileSize;
	bool _hasFileSize;
	QIODevice *_io;
};

class LgpToc
{
public:
	static unsigned int count;
	unsigned int self;
	LgpToc();
	LgpToc(const LgpToc &other);
	virtual ~LgpToc();
	bool addEntry(LgpHeaderEntry *entry);
	LgpHeaderEntry *entry(const QString &filePath) const;
	QList<LgpHeaderEntry *> entries(quint16 id) const;
	bool hasEntries(quint16 id) const;
	bool removeEntry(const QString &filePath);
	bool contains(const QString &filePath) const;
	void clear();
	bool isEmpty() const;
	int size() const;
	QList<const LgpHeaderEntry *> filesSortedByPosition() const;
	LgpToc &operator=(const LgpToc &other);
private:
	LgpHeaderEntry *entry(const QString &filePath, quint16 id) const;
	static qint32 lookupValue(const QString &filePath);
	static quint8 lookupValue(const QChar &qc);
	QMultiMap<quint16, LgpHeaderEntry *> _header;
};

class Lgp
{
public:
	enum LgpError {
		NoError = 0,
		ReadError = 1,
		WriteError = 2,
		FatalError = 3,
		ResourceError = 4,
		OpenError = 5,
		AbortError = 6,
		TimeOutError = 7,
		UnspecifiedError = 8,
		RemoveError = 9,
		RenameError = 10,
		PositionError = 11,
		ResizeError = 12,
		PermissionsError = 13,
		CopyError = 14,
		InvalidError = 15,
		FileNotFoundError = 16
	};

	Lgp(const QString &name);
	virtual ~Lgp();
	QStringList fileList();
	QIODevice *file(const QString &filePath);
	QByteArray fileData(const QString &filePath);
	bool setFile(const QString &filePath, QIODevice *data);
	bool setFileData(const QString &filePath, const QByteArray &data);
	bool addFile(const QString &filePath, QIODevice *data);
	bool addFileData(const QString &filePath, const QByteArray &data);
	bool removeFile(const QString &filePath);
	const QString &companyName();
	const QString &productName();
	bool open(QIODevice::OpenMode mode);
	bool isOpen() const;
	void close();
	QString fileName() const;
	bool pack(const QString &destination=QString(), LgpObserver *observer=NULL);
	LgpError error() const;
	void unsetError();
	QString errorString() const;
private:
	bool openHeader();
	void resolveConflicts();
	LgpHeaderEntry *headerEntry(const QString &filePath);
	void setError(LgpError error);

	QString _companyName;
	LgpToc _files;
	QString _productName;
	QFile _file;
	LgpError _error;

};

#endif // LGP_H
