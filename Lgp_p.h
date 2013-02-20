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
/*
 * This file may contains some code (especially the conflict part)
 * inspired from LGP/UnLGP tool written by Aali.
 * http://forums.qhimm.com/index.php?topic=8641.0
 */
#ifndef LGP_P_H
#define LGP_P_H

#include <QtCore>

#define LOOKUP_VALUE_MAX 30
#define LOOKUP_TABLE_ENTRIES LOOKUP_VALUE_MAX * LOOKUP_VALUE_MAX

#define MAX_CONFLICTS 4096

struct LgpLookupTableEntry
{
	quint16 tocOffset;
	quint16 fileCount;
} Q_PACKED;

struct LgpConflictEntry
{
	LgpConflictEntry() : fileDir(QString()), tocIndex(0) {}
	LgpConflictEntry(const QString &fileDir, quint16 tocIndex=0) :
		fileDir(fileDir), tocIndex(tocIndex) {}
	QString fileDir;
	quint16 tocIndex;
};

struct LgpTocEntry
{
	LgpTocEntry() : conflict(0), tocIndex(0) {}
	LgpTocEntry(quint16 tocIndex, quint16 conflict=0) :
		conflict(conflict), tocIndex(tocIndex) {}
	quint16 conflict;
	quint16 tocIndex;
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
	QIODevice *file(QFile *lgp);
	QIODevice *modifiedFile(QFile *lgp);
	void setFile(QIODevice *io);
	void setModifiedFile(QIODevice *io);
private:
	QIODevice *createFile(QFile *lgp);
	QString _fileName;
	QString _fileDir;
	quint32 _filePosition;
	quint32 _fileSize;
	bool _hasFileSize;
	QIODevice *_io;
	QIODevice *_newIO;
};

class LgpIO : public QIODevice
{
public:
	LgpIO(QIODevice *lgp, const LgpHeaderEntry *header, QObject *parent=0);
	bool open(OpenMode mode);
	qint64 size() const;
	bool canReadLine() const;
protected:
	qint64 readData(char *data, qint64 maxSize);
	qint64 writeData(const char *data, qint64 maxSize);
private:
	QIODevice *_lgp;
	const LgpHeaderEntry *_header;
};

class LgpIterator;

class LgpToc
{
public:
	LgpToc();
	LgpToc(const LgpToc &other);
	virtual ~LgpToc();
	bool addEntry(LgpHeaderEntry *entry);
	LgpHeaderEntry *entry(const QString &filePath) const;
	QList<LgpHeaderEntry *> entries(quint16 id) const;
	const QMultiHash<quint16, LgpHeaderEntry *> &table() const;
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
	QMultiHash<quint16, LgpHeaderEntry *> _header;
};

#endif // LGP_P_H
