/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2013 Arzel Jérôme <myst6re@gmail.com>
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
#ifndef LGPDIALOG_H
#define LGPDIALOG_H

#include <QtWidgets>
#include "core/Lgp.h"
#include "ArchivePreview.h"

class IconThread : public QThread
{
	Q_OBJECT
public:
	explicit IconThread(QObject *parent = 0);
	~IconThread();

	void clear();
	void addFile(const QString &filePath);
signals:
	void iconLoaded(const QString &filePath, const QIcon &icon) const;
protected:
	void run();
private:
	QMutex mutexFiles;
	QWaitCondition hasFilesCondition;
	bool abort;

	QQueue<QString> _files;
	QMap<QString, QIcon> cacheIcon;
};

class LgpDirectoryItem;

class LgpItem
{
public:
	explicit LgpItem(const QString &name, LgpDirectoryItem *parent = 0) :
		_name(name), _parent(parent) {
	}
	virtual ~LgpItem() {
	}

	inline LgpDirectoryItem *parent() const {
		return _parent;
	}
	inline void setParent(LgpDirectoryItem *parent) {
		_parent = parent;
	}

	LgpDirectoryItem *root();

	virtual bool isDirectory() const=0;
	inline bool isFile() const {
		return !isDirectory();
	}
	inline const QString &name() const {
		return _name;
	}
	inline void setName(const QString &name) {
		_name = name;
	}
	QString path() const;
	QString dirName() const;
	int id() const;

	virtual LgpItem *find(const QString &path);

	virtual void debug() const=0;
private:
	QString _name;
	LgpDirectoryItem *_parent;
};

class LgpFileItem : public LgpItem
{
public:
	LgpFileItem(const QString &name, Lgp *lgp, LgpDirectoryItem *parent = 0) :
		LgpItem(name, parent), _lgp(lgp) {
	}
	virtual ~LgpFileItem() {
	}

	inline bool isDirectory() const {
		return false;
	}

	qint64 fileSize();
	QIODevice *file();
	bool setFile(QIODevice *io);
	void setIcon(const QIcon &icon);
	const QIcon &icon() const;
	bool move(const QString &destination);
	void debug() const;

private:
	QIcon _icon;
	Lgp *_lgp;
};

class LgpDirectoryItem : public LgpItem
{
public:
	enum SortType {
		ByName,
		BySize
	};

	explicit LgpDirectoryItem(const QString &name, LgpDirectoryItem *parent = 0) :
		LgpItem(name, parent) {
	}
	virtual ~LgpDirectoryItem();

	inline bool isDirectory() const {
		return true;
	}

	inline LgpItem *child(int row) {
		return _childItems.at(row);
	}

	inline int indexOfChild(const LgpItem *child) const {
		return _childItems.indexOf(const_cast<LgpItem *>(child));
	}

	inline int childCount() const {
		return _childItems.size();
	}

	void sort(SortType type, Qt::SortOrder order);
	bool unrefChild(LgpItem *child);
	void renameChild(LgpItem *child, const QString &destination);
	bool removeChild(LgpItem *child);
	bool removeChildren(int position, int count);
	void addChild(const QString &name, Lgp *lgp);
	void addChild(const QString &name, LgpFileItem *item);
	LgpItem *find(const QString &path);
	void debug() const;
private:
	QHash<QString, LgpFileItem *> _fileChilds;
	QHash<QString, LgpDirectoryItem *> _dirChilds;
	QList<LgpItem *> _childItems;
};

class LgpItemModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit LgpItemModel(Lgp *lgp, QObject *parent=0);
	~LgpItemModel();
	QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent=QModelIndex()) const;
	int columnCount(const QModelIndex &parent=QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
	QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
	bool insertRow(const QString &name, QIODevice *io, const QModelIndex &parent=QModelIndex());
	bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex());
	bool removeRow(const QModelIndex &index);
	bool removeRows(int row, int count, const QModelIndex &parent=QModelIndex());
	void update(const QModelIndex &index);
	LgpItem *item(const QModelIndex &index) const;
private slots:
	void setIcon(const QString &path, const QIcon &icon);
private:
//	IconThread iconThread;
	QIcon fileIcon, directoryIcon;
	LgpDirectoryItem *root;
	Lgp *_lgp;
};

class LgpDialog : public QDialog, ArchiveObserver
{
	Q_OBJECT
public:
	explicit LgpDialog(Lgp *lgp, QWidget *parent=0);

	bool observerWasCanceled() const;
	void setObserverMaximum(unsigned int max);
	void setObserverValue(int value);
	bool observerRetry(const QString &message);
signals:
	void modified();
private slots:
	void changePreview();
	void changeImageInPreview(int imageID);
	void changeImagePaletteInPreview(int palID);
	void renameCurrent();
	void replaceCurrent();
	void extractCurrent();
	void add();
	void removeCurrent();
	void setButtonsState();
	void pack();
private:
	void generatePreview();
	Lgp *lgp;
	QTreeView *treeView;
	QPushButton *extractButton, *renameButton,
	            *replaceButton, *addButton,
	            *removeButton, *packButton;
	QProgressDialog *progressDialog;
	LgpItemModel *_model;
	ArchivePreview *preview;
	FieldModelFilePC fieldModelFile;
	int currentImage, currentPal;
};

#endif // LGPDIALOG_H
