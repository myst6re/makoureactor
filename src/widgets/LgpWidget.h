/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2022 Arzel Jérôme <myst6re@gmail.com>
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

#include <QtWidgets>
#include <Lgp>
#include "ArchivePreview.h"

class IconThread : public QThread
{
	Q_OBJECT
public:
	explicit IconThread(QObject *parent = nullptr);
	~IconThread() override;

	void clear();
	void addFile(const QString &filePath);
signals:
	void iconLoaded(const QString &filePath, const QIcon &icon) const;
protected:
	void run() override;
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
	explicit LgpItem(const QString &name, LgpDirectoryItem *parent = nullptr) :
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
	LgpFileItem(const QString &name, Lgp *lgp, LgpDirectoryItem *parent = nullptr) :
		LgpItem(name, parent), _lgp(lgp) {
	}
	virtual ~LgpFileItem() override {
	}

	inline bool isDirectory() const override {
		return false;
	}

	qint64 fileSize();
	QIODevice *file();
	bool setFile(QIODevice *io);
	void setIcon(const QIcon &icon);
	const QIcon &icon() const;
	bool move(const QString &destination);
	void debug() const override;

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

	explicit LgpDirectoryItem(const QString &name, LgpDirectoryItem *parent = nullptr) :
		LgpItem(name, parent) {
	}
	virtual ~LgpDirectoryItem() override;

	inline bool isDirectory() const override {
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
	LgpItem *find(const QString &path) override;
	void debug() const override;
private:
	QHash<QString, LgpFileItem *> _fileChilds;
	QHash<QString, LgpDirectoryItem *> _dirChilds;
	QList<LgpItem *> _childItems;
};

class LgpItemModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit LgpItemModel(Lgp *lgp, QObject *parent = nullptr);
	~LgpItemModel() override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
	bool insertRow(const QString &name, QIODevice *io, const QModelIndex &parent = QModelIndex());
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	bool removeRow(const QModelIndex &index);
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	void update(const QModelIndex &index);
	LgpItem *item(const QModelIndex &index) const;
private slots:
	void setIcon(const QString &path, const QIcon &icon);
private:
//	IconThread iconThread;
	QIcon fileIcon, directoryIcon, textureIcon;
	LgpDirectoryItem *root;
	Lgp *_lgp;
	QStringList _textureTypes { ".tex", ".png", ".tim", ".jpg", ".jpeg" };
};

class LgpWidget : public QWidget, ArchiveObserver
{
	Q_OBJECT
public:
	explicit LgpWidget(Lgp *lgp, QWidget *parent = nullptr);

	bool observerWasCanceled() const override;
	void setObserverMaximum(unsigned int max) override;
	void setObserverValue(int value) override;
	bool observerRetry(const QString &message) override;
signals:
	void modified();
private slots:
	void changePreview();
	void changeImageInPreview(int imageID);
	void changeImagePaletteInPreview(int palID);
	void renameCurrent();
	void replaceCurrent();
	void extractCurrent();
	void extractAll();
	void add();
	void removeCurrent();
	void setButtonsState();
private:
	void generatePreview();
	Lgp *lgp;
	QTreeView *treeView;
	QPushButton *extractButton, *extractAllButton, *renameButton,
	            *replaceButton, *addButton,
	            *removeButton;
	QProgressDialog *progressDialog;
	LgpItemModel *_model;
	ArchivePreview *preview;
	FieldModelFilePC fieldModelFile;
	int currentImage, currentPal;
};
