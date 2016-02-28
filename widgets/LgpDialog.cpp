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
#include "LgpDialog.h"
#include "core/Config.h"
#include "Parameters.h"

IconThread::IconThread(QObject *parent) :
	QThread(parent), abort(false)
{
}

IconThread::~IconThread()
{
	QMutexLocker locker(&mutexFiles);
	abort = true;
	hasFilesCondition.wakeOne();
	locker.unlock();
	wait();
}

void IconThread::clear()
{
	QMutexLocker locker(&mutexFiles);
	_files.clear();
}

void IconThread::addFile(const QString &filePath)
{
	QMutexLocker locker(&mutexFiles);
	_files.enqueue(filePath);
	hasFilesCondition.wakeAll();
}

void IconThread::run()
{
	forever {
		QMutexLocker locker(&mutexFiles);
		if(abort) {
			return;
		}
		qDebug() << "IconThread::run()";
		if(_files.isEmpty()) {
			hasFilesCondition.wait(&mutexFiles);
		}
		QString path = _files.dequeue();
		qDebug() << "IconThread::run() treat path" << path;
		locker.unlock();

		int index = path.lastIndexOf('.');
		if(index != -1) {
			QString type = path.mid(index + 1);

			if(cacheIcon.contains(type)) {
				emit iconLoaded(path, cacheIcon.value(type));
			} else {
				QFile tmp(QDir::tempPath() % "/" % PROG_NAME % "." % type);
				if(tmp.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
					QIcon icon = QFileIconProvider().icon(QFileInfo(tmp));
					tmp.remove();

					cacheIcon.insert(type, icon);

					emit iconLoaded(path, icon);
				} else {
					qWarning() << "IconThread::run() cannot create tmp file" << tmp.fileName() << tmp.errorString();
				}
			}
		}
	}
}

LgpDirectoryItem *LgpItem::root()
{
	LgpItem *current = this;
	while(current->parent()) {
		current = current->parent();
	}
	return static_cast<LgpDirectoryItem *>(current);
}

QString LgpItem::path() const
{
	QString dirPath = dirName();

	if(dirPath.isEmpty()) {
		return name();
	}
	return dirPath + "/" + name();
}

QString LgpItem::dirName() const
{
	return parent() ? parent()->path() : QString();
}

int LgpItem::id() const
{
	if(parent()) {
		return parent()->indexOfChild(this);
	}

	return -1;
}

qint64 LgpFileItem::fileSize()
{
	QIODevice *io = file();
	if(io->open(QIODevice::ReadOnly)) {
		return io->size();
	}
	return -1;
}

QIODevice *LgpFileItem::file()
{
	return _lgp->modifiedFile(path());
}

bool LgpFileItem::setFile(QIODevice *io)
{
	return _lgp->setFile(path(), io);
}

void LgpFileItem::setIcon(const QIcon &icon)
{
	_icon = icon;
}

const QIcon &LgpFileItem::icon() const
{
	return _icon;
}

bool LgpFileItem::move(const QString &destination) {
	bool ok, sameDir;
	QString source = path(), destFileName, destDirName;
	int index = destination.lastIndexOf('/');
	if(index < 0) {
		destFileName = destination;
	} else {
		destDirName = destination.left(index);
		destFileName = destination.mid(index + 1);
	}

	sameDir = dirName() == destDirName; // We don't need to move the item if the directory is the same

	ok = sameDir || parent()->unrefChild(this);
	if(ok) {
		ok = _lgp->renameFile(source, destination);
		if(ok) {
			if(!sameDir) {
				root()->addChild(destination, this);
			} else {
				parent()->renameChild(this, destFileName);
			}
		} else {
			if(!sameDir) {
				root()->addChild(source, this); // Cancel renaming
			}
			qWarning() << "cannot rename child!";
		}
	} else {
		qWarning() << "cannot unref child!";
	}

	return ok;
}

void LgpFileItem::debug() const
{
	qDebug() << "File" << path();
}

LgpDirectoryItem::~LgpDirectoryItem()
{
	qDeleteAll(_childs);
}

void LgpDirectoryItem::addChild(const QString &name, Lgp *lgp)
{
	addChild(name, new LgpFileItem(QString(), lgp));
}

void LgpDirectoryItem::addChild(const QString &name, LgpFileItem *item)
{
	int index = name.indexOf('/');
	if(index < 0) {
		if(_fileChilds.contains(name)) {
			qWarning() << "LgpDirectoryItem::addChild Child already in the tree";
			delete item;
		} else {
			item->setParent(this);
			item->setName(name);
			_fileChilds.insert(name, item);
			_childs.append(item);
		}
	} else {
		QString dirName = name.left(index);
		LgpDirectoryItem *dirItem;
		if(_dirChilds.contains(dirName)) {
			dirItem = _dirChilds.value(dirName);
		} else {
			dirItem = new LgpDirectoryItem(dirName, this);
			_dirChilds.insert(dirName, dirItem);
			_childs.append(dirItem);
		}

		dirItem->addChild(name.mid(index + 1), item);
	}
}

bool LgpDirectoryItem::unrefChild(LgpItem *child)
{
	bool ok;

	if(child->isDirectory()) {
		ok = _dirChilds.remove(child->name()) == 1;
	} else {
		ok = _fileChilds.remove(child->name()) == 1;
	}

	if(!ok) {
		qWarning() << "LgpDirectoryItem::unrefChild: child not found in hash tables!" << child->isDirectory() << child->name();
	}

	return ok && _childs.removeOne(child);
}

void LgpDirectoryItem::renameChild(LgpItem *child, const QString &destination)
{
	if(child->isDirectory()) {
		LgpDirectoryItem *item = _dirChilds.take(child->name());
		if(item) {
			item->setName(destination);
			_dirChilds.insert(destination, item);
		}
	} else {
		LgpFileItem *item = _fileChilds.take(child->name());
		if(item) {
			item->setName(destination);
			_fileChilds.insert(destination, item);
		}
	}
}

void LgpDirectoryItem::removeChild(LgpItem *child)
{
	if(child->isDirectory()) {
		_dirChilds.remove(child->name());
	} else {
		_fileChilds.remove(child->name());
	}

	delete _childs.takeAt(child->id());
}

LgpItem *LgpItem::find(const QString &path)
{
	Q_UNUSED(path) //TODO
	return NULL;
}

LgpItem *LgpDirectoryItem::find(const QString &path)
{
	QString name, rest;
	int index = path.indexOf('/');

	if(index < 0) {
		name = path.left(index);
		rest = path.mid(index + 1);
	} else {
		name = path;
	}

	if(_fileChilds.contains(name)) {
		return _fileChilds.value(name);
	}
	if(_dirChilds.contains(name)) {
		if(!rest.isEmpty()) {
			return _dirChilds.value(name)->find(rest);
		}

	}
	return NULL;
}

void LgpDirectoryItem::debug() const
{
	qDebug() << "Directory" << path();
	foreach(LgpItem *item, _childs) {
		item->debug();
	}
}

LgpItemModel::LgpItemModel(Lgp *lgp, QObject *parent) :
	QAbstractItemModel(parent)/*, iconThread(this)*/
{
	root = new LgpDirectoryItem("");
	foreach(const QString &path, lgp->fileList()) {
		root->addChild(path, lgp);
//		iconThread.addFile(path);
	}
//	root->debug();

//	connect(&iconThread, SIGNAL(iconLoaded(QString,QIcon)), SLOT(setIcon(QString,QIcon)));

	QFileIconProvider iconProvider;
	fileIcon = iconProvider.icon(QFileIconProvider::File);
	directoryIcon = iconProvider.icon(QFileIconProvider::Folder);

//	iconThread.start(QThread::LowPriority);
}

LgpItemModel::~LgpItemModel()
{
	delete root;
}

void LgpItemModel::setIcon(const QString &path, const QIcon &icon)
{
	LgpItem *item = root->find(path);
	if(!item || item->isDirectory()) {
		return;
	}

	static_cast<LgpFileItem *>(item)->setIcon(icon);

	emit layoutChanged();
}

QModelIndex LgpItemModel::index(int row, int column, const QModelIndex &parent) const
{
	if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent)) {
		return QModelIndex();
	}

	LgpDirectoryItem *parentItem = static_cast<LgpDirectoryItem *>(getItem(parent));
	if(!parentItem) {
		parentItem = root;
	}

	return createIndex(row, column, parentItem->child(row));
}

LgpItem *LgpItemModel::getItem(const QModelIndex &index) const
{
	if(index.isValid()) {
		LgpItem *item = static_cast<LgpItem *>(index.internalPointer());
		if(item) {
			return item;
		}
	}
	return 0;
}

QModelIndex LgpItemModel::parent(const QModelIndex &index) const
{
	if(!index.isValid()) {
		return QModelIndex();
	}

	LgpItem *childItem = getItem(index);
	LgpItem *parentItem = childItem->parent();

	if(parentItem == root || !parentItem) {
		return QModelIndex();
	}

	int row = parentItem->id();
	if(row < 0) {
		return QModelIndex();
	}

	return createIndex(row, 0, parentItem);
}

int LgpItemModel::rowCount(const QModelIndex &parent) const
{
	LgpItem *parentItem = getItem(parent);
	if(!parentItem) {
		parentItem = root;
	}

	if(parentItem->isDirectory()) {
		return static_cast<LgpDirectoryItem *>(parentItem)->childCount();
	}
	return 0;
}

int LgpItemModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return 2;
}

QVariant LgpItemModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid()) {
		return QVariant();
	}

	LgpItem *lgpItem = getItem(index);
	if(!lgpItem) {
		return QVariant();
	}

	switch(role) {
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch(index.column()) {
		case 0:		return lgpItem->name();
		case 1:
			if(lgpItem->isDirectory()) {
				return QString();
			} else {
				qint64 fileSize = static_cast<LgpFileItem *>(lgpItem)->fileSize();
				return fileSize < 0 ? tr("?") : QString::number(fileSize);
			}
		default:	break;
		}
		break;
	case Qt::DecorationRole:
		if(index.column() == 0) {
			if(lgpItem->isDirectory()) {
				return directoryIcon;
			} else {
				if(static_cast<LgpFileItem *>(lgpItem)->icon().isNull()) {
					return fileIcon;
				} else {
					return static_cast<LgpFileItem *>(lgpItem)->icon();
				}
			}
		}
		break;
	case Qt::TextAlignmentRole:
		if(index.column() == 1) {
			return Qt::AlignRight;
		}
		break;
	}

	return QVariant();
}

bool LgpItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!index.isValid()
			|| role != Qt::EditRole) {
		return false;
	}

	LgpItem *lgpItem = getItem(index);

	if(!lgpItem || lgpItem->isDirectory()) {
		return false;
	}
	QString newFilename = value.toString();

	if(lgpItem->path() == newFilename) {
		return false;
	}

	bool ok = static_cast<LgpFileItem *>(lgpItem)->move(newFilename);

	if(ok) {
		emit dataChanged(index, index);
		emit layoutChanged();
	}

	return ok;
}

void LgpItemModel::update(const QModelIndex &index)
{
	if(index.isValid()) {
		emit dataChanged(index, index);
	}
}

QVariant LgpItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch(role) {
	case Qt::TextAlignmentRole:
		return Qt::AlignLeft;
	}

	if(orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return QAbstractItemModel::headerData(section, orientation, role);

	switch(section) {
	case 0:		return tr("Name");
	case 1:		return tr("Size");
	default:	return QVariant();
	}
}

bool LgpItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
	beginInsertRows(parent, row, row + count - 1);
	endInsertRows();

	return true;
}

bool LgpItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
	beginRemoveRows(parent, row, row + count - 1);
	endRemoveRows();

	return true;
}

LgpDialog::LgpDialog(Lgp *lgp, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint), lgp(lgp)
{
	setWindowTitle(tr("LGP archive manager"));
	resize(800, 600);

	LgpItemModel *model = new LgpItemModel(lgp);
	treeView = new QTreeView(this);
	treeView->setModel(model);
	treeView->setUniformRowHeights(true);
	treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	treeView->header()->setStretchLastSection(false);

	renameButton = new QPushButton(tr("Rename"), this);
	renameButton->setShortcut(Qt::Key_F2);
	replaceButton = new QPushButton(tr("Replace"), this);
	replaceButton->setShortcut(QKeySequence("Ctrl+R"));
	extractButton = new QPushButton(tr("Extract"), this);
	extractButton->setShortcut(QKeySequence("Ctrl+E"));
	addButton = new QPushButton(QIcon(":/images/plus.png"), tr("Add"), this);
	addButton->setShortcut(QKeySequence::New);
	removeButton = new QPushButton(QIcon(":/images/minus.png"), tr("Delete"), this);
	removeButton->setShortcut(QKeySequence::Delete);
	packButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton),
								 tr("Save"), this);
	packButton->setEnabled(false);

	QHBoxLayout *barLayout = new QHBoxLayout;
	barLayout->addWidget(renameButton);
	barLayout->addWidget(replaceButton);
	barLayout->addWidget(extractButton);
	barLayout->addWidget(addButton);
	barLayout->addWidget(removeButton);
	barLayout->addWidget(packButton);
	barLayout->addStretch();

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addLayout(barLayout);
	layout->addWidget(treeView, 1);

	connect(renameButton, SIGNAL(released()), SLOT(renameCurrent()));
	connect(replaceButton, SIGNAL(released()), SLOT(replaceCurrent()));
	connect(extractButton, SIGNAL(released()), SLOT(extractCurrent()));
	connect(addButton, SIGNAL(released()), SLOT(add()));
	connect(removeButton, SIGNAL(released()), SLOT(removeCurrent()));
	connect(packButton, SIGNAL(released()), SLOT(pack()));
	connect(treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(setButtonsState()));

	setButtonsState();
}

void LgpDialog::renameCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if(index.isValid()) {
		LgpItem *item = static_cast<LgpItemModel *>(treeView->model())->getItem(index);
		if(item && !item->isDirectory()) {
			bool ok;
			QString oldFilePath = item->path(),
					newFilePath = QInputDialog::getText(this, tr("Rename"), tr("New Name:"), QLineEdit::Normal,
														oldFilePath, &ok, Qt::Dialog | Qt::WindowCloseButtonHint);
			if(!ok || newFilePath == oldFilePath) {
				return;
			}

			if(!lgp->isNameValid(newFilePath)) {
				QMessageBox::warning(this, tr("Error"), tr("The name '%1' is invalid, don't put special characters.")
									 .arg(newFilePath));
			} else if(lgp->fileExists(newFilePath)) {
				QMessageBox::warning(this, tr("Error"), tr("A file named '%1' already exists, please choose another name.")
									 .arg(newFilePath));
			} else if(!treeView->model()->setData(index, newFilePath)) {
				QMessageBox::warning(this, tr("Error"), tr("Can not Rename the file"));
			} else {
				treeView->scrollTo(index);
//				emit modified();
				packButton->setEnabled(true);
			}
		}
	}
}

void LgpDialog::replaceCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if(index.isValid()) {
		LgpItem *item = static_cast<LgpItemModel *>(treeView->model())->getItem(index);
		if(item && !item->isDirectory()) {
			QString filename = item->name();
			int indexOf = filename.lastIndexOf('.');
			QString extension = indexOf==-1 ? QString() : filename.mid(indexOf + 1);
			QStringList filter;
			if(!extension.isEmpty()) {
				filter << tr("%1 file (*.%1)").arg(extension);
			}
			filter << tr("All files (*)");

			QString lastDir = Config::value("lgpDialogOpenDirectory").toString();
			if(!lastDir.isEmpty()) {
				filename = lastDir + "/" + filename;
			}

			QString path = QFileDialog::getOpenFileName(this, tr("New File"), filename, filter.join(";;"));
			if(path.isNull()) {
				return;
			}

			Config::setValue("lgpDialogOpenDirectory", path.left(path.lastIndexOf('/')));

			if(!static_cast<LgpFileItem *>(item)->setFile(new QFile(path))) {
				QMessageBox::warning(this, tr("Error"), tr("Can not modify the archive!"));
			} else {
//				emit modified();
				packButton->setEnabled(true);
			}
		}
	}
}

void LgpDialog::extractCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if(index.isValid()) {
		LgpItem *item = static_cast<LgpItemModel *>(treeView->model())->getItem(index);
		if(item && !item->isDirectory()) {
			QString filename = item->name();
			int index = filename.lastIndexOf('.');
			QString extension = index==-1 ? QString() : filename.mid(index + 1);
			QStringList filter;
			if(!extension.isEmpty()) {
				filter << tr("%1 file (*.%1)").arg(extension);
			}
			filter << tr("All files (*)");

			QString lastDir = Config::value("lgpDialogSaveDirectory").toString();
			if(!lastDir.isEmpty()) {
				filename = lastDir + "/" + filename;
			}

			QString path = QFileDialog::getSaveFileName(this, tr("New File"), filename, filter.join(";;"));
			if(path.isNull()) {
				return;
			}

			Config::setValue("lgpDialogSaveDirectory", path.left(path.lastIndexOf('/')));

			QIODevice *io = static_cast<LgpFileItem *>(item)->file();
			if(io && io->open(QIODevice::ReadOnly)) {
				QFile file(path);
				if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
					while(io->bytesAvailable()) {
						file.write(io->read(4096));
					}
					if(file.error() != QFile::NoError) {
						QMessageBox::warning(this, tr("Write error"), tr("Can not write to file (message: %1).")
											 .arg(file.errorString()));
					}
					file.close();
				} else {
					QMessageBox::warning(this, tr("Opening error"), tr("Can not open the file (message: %1).")
										 .arg(file.errorString()));
				}
				io->close();
			}
		}
	}
}

void LgpDialog::add()
{
	QString dirName;
	QModelIndex index = treeView->currentIndex();
	if(index.isValid()) {
		LgpItem *item = static_cast<LgpItemModel *>(treeView->model())->getItem(index);
		if(item) {
			dirName = item->isDirectory() ? item->path() : item->dirName();
		}
	}

	QString lastDir = Config::value("lgpDialogOpenDirectory").toString();

	QStringList filter;
	filter << tr("All files (*)");
	QString path = QFileDialog::getOpenFileName(this, tr("New File"), lastDir, filter.join(";;"));
	if(path.isNull()) {
		return;
	}

	Config::setValue("lgpDialogOpenDirectory", path.left(path.lastIndexOf('/')));

	QString name = path.mid(path.lastIndexOf('/') + 1);

	if(!dirName.isEmpty()) {
		name = dirName + "/" + name;
	}

	bool ok;
	QString filePath = QInputDialog::getText(this, tr("Rename"), tr("New Name:"), QLineEdit::Normal,
												name, &ok, Qt::Dialog | Qt::WindowCloseButtonHint);
	if(!ok) {
		return;
	}

	if(!lgp->isNameValid(filePath)) {
		QMessageBox::warning(this, tr("Error"), tr("The name '%1' is invalid, don't put special characters.")
							 .arg(filePath));
	} else if(lgp->fileExists(filePath)) {
		QMessageBox::warning(this, tr("Error"), tr("A file named '%1' already exists, please choose another name.")
							 .arg(filePath));
	} else if(!lgp->addFile(filePath, new QFile(path))) {
		QMessageBox::warning(this, tr("Error"), tr("Can not add the file"));
	} else {
		treeView->model()->insertRow(treeView->model()->rowCount());
		treeView->scrollTo(treeView->model()->index(treeView->model()->rowCount() - 1, 0));
//		emit modified();
		packButton->setEnabled(true);
	}
}

void LgpDialog::removeCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if(index.isValid()) {
		QMessageBox::StandardButton button = QMessageBox::question(this, tr("Delete ?"),
							  tr("Are you sure you want to delete this file from the archive?"),
							  QMessageBox::Yes | QMessageBox::Cancel);
		if(button != QMessageBox::Yes) {
			return;
		}

		LgpItem *item = static_cast<LgpItemModel *>(treeView->model())->getItem(index);
		if(item && !item->isDirectory()) {
			if(!lgp->removeFile(item->path())) {
				QMessageBox::warning(this, tr("Error"), tr("Cannot delete the file!"));
			} else {
				treeView->model()->removeRow(index.row());
//				emit modified();
				packButton->setEnabled(true);
			}
		}
	}
}

void LgpDialog::pack()
{
	QString path = QFileDialog::getSaveFileName(this, tr("Save as"), lgp->fileName(), tr("Lgp File (*.lgp)"));
	if(path.isNull()) {
		return;
	}

//	QFileInfo info1(path), info2(lgp->fileName());
//	if(info1 == info2) {
//		QMessageBox::warning(this, tr("Action impossible"), tr("Merci de sélectionner un autre fichier que celui actuellement ouvert par le logiciel."));
//		return;
//	}

	progressDialog = new QProgressDialog(tr("Saving..."), tr("Cancel"), 0, 0, this, Qt::Dialog | Qt::WindowCloseButtonHint);
	progressDialog->setWindowModality(Qt::WindowModal);
	progressDialog->setAutoClose(false);
	progressDialog->show();

	bool ok = lgp->pack(path, this);

	progressDialog->hide();
	progressDialog->deleteLater();

	if(!ok) {
		if(lgp->error() != Lgp::AbortError) {
			QMessageBox::warning(this, tr("Error"), tr("Cannot create the archive (message: %1).")
								 .arg(lgp->errorString()));
		}
	} else {
		lgp->open();
//		emit modified();
		packButton->setEnabled(false);
	}
}

bool LgpDialog::observerWasCanceled() const
{
	return progressDialog->wasCanceled();
}

void LgpDialog::setObserverMaximum(unsigned int max)
{
	progressDialog->setMaximum(max);
}

void LgpDialog::setObserverValue(int value)
{
	QApplication::processEvents();

	progressDialog->setValue(value);
}

void LgpDialog::setButtonsState()
{
	QModelIndexList modelIndexList = treeView->selectionModel()->selectedRows();
	bool enabled = !modelIndexList.isEmpty() &&
			!static_cast<LgpItemModel *>(treeView->model())->getItem(modelIndexList.first())->isDirectory();

	renameButton->setEnabled(enabled);
	replaceButton->setEnabled(enabled);
	extractButton->setEnabled(enabled);
	removeButton->setEnabled(enabled);
}
