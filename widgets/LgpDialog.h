#ifndef LGPDIALOG_H
#define LGPDIALOG_H

#include <QtGui>
#include "core/Lgp.h"

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
	inline const QString &name() const {
		return _name;
	}
	inline void setName(const QString &name) {
		_name = name;
	}
	QString path() const;
	QString dirName() const;
	int id() const;

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
	~LgpFileItem() {
	}

	inline bool isDirectory() const {
		return false;
	}

	qint64 fileSize();
	QIODevice *file();
	bool setFile(QIODevice *io);
	bool move(const QString &destination);
	void debug() const;

private:
	Lgp *_lgp;
};

class LgpDirectoryItem : public LgpItem
{
public:
	LgpDirectoryItem(const QString &name, LgpDirectoryItem *parent = 0) :
		LgpItem(name, parent) {
	}
	~LgpDirectoryItem();

	inline bool isDirectory() const {
		return true;
	}

	inline LgpItem *child(int row) {
		return _childs.at(row);
	}

	inline int indexOfChild(const LgpItem *child) const {
		return _childs.indexOf(const_cast<LgpItem *>(child));
	}

	inline int childCount() const {
		return _childs.size();
	}

	bool unrefChild(LgpItem *child);
	void renameChild(LgpItem *child, const QString &destination);
	void removeChild(LgpItem *child);
	void addChild(const QString &name, Lgp *lgp);
	void addChild(const QString &name, LgpFileItem *item);
	void debug() const;
private:
	QHash<QString, LgpFileItem *> _fileChilds;
	QHash<QString, LgpDirectoryItem *> _dirChilds;
	QList<LgpItem *> _childs;
};

class LgpItemModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	LgpItemModel(Lgp *lgp, QObject *parent=0);
	~LgpItemModel();
	QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent=QModelIndex()) const;
	int columnCount(const QModelIndex &parent=QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
	QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex());
	bool removeRows(int row, int count, const QModelIndex &parent=QModelIndex());
	void update(const QModelIndex &index);
	LgpItem *getItem(const QModelIndex &index) const;
private:
	QIcon fileIcon, directoryIcon;
	LgpDirectoryItem *root;
};

class LgpDialog : public QDialog, ArchiveObserver
{
	Q_OBJECT
public:
	LgpDialog(Lgp *lgp, QWidget *parent=0);

	bool observerWasCanceled() const;
	void setObserverMaximum(unsigned int max);
	void setObserverValue(int value);
signals:
	void modified();
private slots:
	void renameCurrent();
	void replaceCurrent();
	void extractCurrent();
	void add();
	void removeCurrent();
	void setButtonsState();
	void pack();
private:
	Lgp *lgp;
	QTreeView *treeView;
	QPushButton *extractButton, *renameButton,
	*replaceButton, *addButton,
	*removeButton, *packButton;
	QProgressDialog *progressDialog;
};

#endif // LGPDIALOG_H
