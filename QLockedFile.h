#ifndef QLOCKEDFILE_H
#define QLOCKEDFILE_H

#include <QtCore>
#ifdef Q_WS_WIN
#include <windef.h>
#include <winbase.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class QLockedFile : public QFile
{
	Q_OBJECT
public:
	explicit QLockedFile(const QString &name);
	explicit QLockedFile(QObject *parent);
	explicit QLockedFile(const QString &name, QObject *parent);
	virtual void close();
	virtual bool open(OpenMode mode);
signals:

public slots:
private:
#ifdef Q_WS_WIN
	HANDLE handle;
#else
	int handle;
#endif
};

#endif // QLOCKEDFILE_H
