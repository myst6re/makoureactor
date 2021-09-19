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
#ifndef LGP_H
#define LGP_H

#include <QtCore>
#include "Archive.h"

class LgpHeaderEntry;
class LgpToc;
class Lgp;

class LgpIterator
{
	friend class Lgp;
public:
	explicit LgpIterator(const Lgp &lgp);
	bool hasNext() const;
	void next();
	void toBack();
	void toFront();
	QIODevice *file();
	QIODevice *modifiedFile();
	const QString &fileName() const;
	const QString &fileDir() const;
	QString filePath() const;
private:
	LgpIterator(LgpToc *toc, QFile *lgp);
	QHashIterator<quint16, LgpHeaderEntry *> it;
	QFile *_lgp;
};

class Lgp : public Archive
{
	friend class LgpIterator;
public:
	enum LgpError {
		NoError,
		ReadError,
		WriteError,
		OpenError,
		OpenTempError,
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

	Lgp();
	explicit Lgp(const QString &name);
	explicit Lgp(QFile *device);
	virtual ~Lgp();
	void clear();
	QStringList fileList() const;
	int fileCount() const;
	LgpIterator iterator();
	bool fileExists(const QString &filePath) const;
	QIODevice *file(const QString &filePath);
	QIODevice *modifiedFile(const QString &filePath);
	bool setFile(const QString &filePath, QIODevice *data);
	bool addFile(const QString &filePath, QIODevice *data);
	bool removeFile(const QString &filePath);
	bool isNameValid(const QString &filePath) const;
	bool renameFile(const QString &filePath, const QString &newFilePath);
	const QString &companyName();
	void setCompanyName(const QString &companyName);
	const QString &productName();
	void setProductName(const QString &productName);
	bool pack(const QString &destination=QString(), ArchiveObserver *observer=nullptr);
	LgpError error() const;
	void unsetError();
private:
	Q_DISABLE_COPY(Lgp)
	bool openHeader();
	bool openCompanyName();
	bool openProductName();
	LgpHeaderEntry *headerEntry(const QString &filePath) const;
	void setError(LgpError error, const QString &errorString=QString());
	static QByteArray readAll(QIODevice *d, bool *ok);

	QString _companyName;
	LgpToc *_files;
	QString _productName;
	LgpError _error;

};

#endif // LGP_H
