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
#include "LgpWidget.h"
#include "core/Config.h"
#include "core/TimFile.h"
#include "core/TexFile.h"
#include "core/field/CharArchive.h"

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
		if (abort) {
			return;
		}
		qDebug() << "IconThread::run()";
		if (_files.isEmpty()) {
			hasFilesCondition.wait(&mutexFiles);
		}
		QString path = _files.dequeue();
		qDebug() << "IconThread::run() treat path" << path;
		locker.unlock();

		int index = path.lastIndexOf('.');
		if (index != -1) {
			QString type = path.mid(index + 1);

			if (cacheIcon.contains(type)) {
				emit iconLoaded(path, cacheIcon.value(type));
			} else {
				QFile tmp(QDir::tempPath() % "/" % MAKOU_REACTOR_NAME % "." % type);
				if (tmp.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
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
	while (current->parent()) {
		current = current->parent();
	}
	return static_cast<LgpDirectoryItem *>(current);
}

QString LgpItem::path() const
{
	QString dirPath = dirName();

	if (dirPath.isEmpty()) {
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
	if (parent()) {
		return parent()->indexOfChild(this);
	}

	return -1;
}

qint64 LgpFileItem::fileSize()
{
	QIODevice *io = file();
	if (io->open(QIODevice::ReadOnly)) {
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
	if (index < 0) {
		destFileName = destination;
	} else {
		destDirName = destination.left(index);
		destFileName = destination.mid(index + 1);
	}

	sameDir = dirName() == destDirName; // We don't need to move the item if the directory is the same

	ok = sameDir || parent()->unrefChild(this);
	if (ok) {
		ok = _lgp->renameFile(source, destination);
		if (ok) {
			if (!sameDir) {
				root()->addChild(destination, this);
			} else {
				parent()->renameChild(this, destFileName);
			}
		} else {
			if (!sameDir) {
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
	qDeleteAll(_childItems);
}

void LgpDirectoryItem::addChild(const QString &name, Lgp *lgp)
{
	addChild(name, new LgpFileItem(QString(), lgp));
}

void LgpDirectoryItem::addChild(const QString &name, LgpFileItem *item)
{
	int index = name.indexOf('/');
	if (index < 0) {
		if (_fileChilds.contains(name)) {
			qWarning() << "LgpDirectoryItem::addChild Child already in the tree";
			delete item;
		} else {
			item->setParent(this);
			item->setName(name);
			_fileChilds.insert(name, item);
			_childItems.append(item);
		}
	} else {
		QString dirName = name.left(index);
		LgpDirectoryItem *dirItem;
		if (_dirChilds.contains(dirName)) {
			dirItem = _dirChilds.value(dirName);
		} else {
			dirItem = new LgpDirectoryItem(dirName, this);
			_dirChilds.insert(dirName, dirItem);
			_childItems.append(dirItem);
		}

		dirItem->addChild(name.mid(index + 1), item);
	}
}

void LgpDirectoryItem::sort(SortType type, Qt::SortOrder order)
{
	switch (type) {
	case ByName:
		switch (order) {
		case Qt::AscendingOrder:
			std::sort(_childItems.begin(), _childItems.end(), [](LgpItem *i1, LgpItem *i2) {
				if (i1->isDirectory()) {
					return true;
				}
				if (i2->isDirectory()) {
					return false;
				}
				return i1->name() > i2->name();
			});
			break;
		case Qt::DescendingOrder:
			std::sort(_childItems.begin(), _childItems.end(), [](LgpItem *i1, LgpItem *i2) {
				if (i1->isDirectory()) {
					return false;
				}
				if (i2->isDirectory()) {
					return true;
				}
				return i1->name() < i2->name();
			});
			break;
		}

		break;
	case BySize:
		switch (order) {
		case Qt::AscendingOrder:
			std::sort(_childItems.begin(), _childItems.end(), [](LgpItem *i1, LgpItem *i2) {
				if (i1->isDirectory() && i2->isDirectory()) {
					return i1->name() > i2->name();
				}
				if (i1->isDirectory()) {
					return true;
				}
				if (i2->isDirectory()) {
					return false;
				}
				LgpFileItem *i1f = static_cast<LgpFileItem *>(i1),
				            *i2f = static_cast<LgpFileItem *>(i2);
				return i1f->fileSize() > i2f->fileSize();
			});
			break;
		case Qt::DescendingOrder:
			std::sort(_childItems.begin(), _childItems.end(), [](LgpItem *i1, LgpItem *i2) {
				if (i1->isDirectory() && i2->isDirectory()) {
					return i1->name() < i2->name();
				}
				if (i1->isDirectory()) {
					return false;
				}
				if (i2->isDirectory()) {
					return true;
				}
				LgpFileItem *i1f = static_cast<LgpFileItem *>(i1),
				            *i2f = static_cast<LgpFileItem *>(i2);
				return i1f->fileSize() < i2f->fileSize();
			});
			break;
		}

		break;
	}
}

bool LgpDirectoryItem::unrefChild(LgpItem *child)
{
	bool ok;

	if (child->isDirectory()) {
		ok = _dirChilds.remove(child->name()) == 1;
	} else {
		ok = _fileChilds.remove(child->name()) == 1;
	}

	if (!ok) {
		qWarning() << "LgpDirectoryItem::unrefChild: child not found in hash tables!" << child->isDirectory() << child->name();
	}

	return ok && _childItems.removeOne(child);
}

void LgpDirectoryItem::renameChild(LgpItem *child, const QString &destination)
{
	if (child->isDirectory()) {
		LgpDirectoryItem *item = _dirChilds.take(child->name());
		if (item) {
			item->setName(destination);
			_dirChilds.insert(destination, item);
		}
	} else {
		LgpFileItem *item = _fileChilds.take(child->name());
		if (item) {
			item->setName(destination);
			_fileChilds.insert(destination, item);
		}
	}
}

bool LgpDirectoryItem::removeChild(LgpItem *child)
{
	return removeChildren(child->id(), 1);
}

bool LgpDirectoryItem::removeChildren(int position, int count)
{
	if (position < 0 || position + count > _childItems.size()) {
		return false;
	}

	for (int row = 0; row < count; ++row) {
		LgpItem *item = _childItems.takeAt(position);
		if (item->isDirectory()) {
			_dirChilds.remove(item->name());
		} else {
			_fileChilds.remove(item->name());
		}
		delete item;
	}

	return true;
}

LgpItem *LgpItem::find(const QString &path)
{
	Q_UNUSED(path) //TODO
	return nullptr;
}

LgpItem *LgpDirectoryItem::find(const QString &path)
{
	QString name, rest;
	int index = path.indexOf('/');

	if (index < 0) {
		name = path.left(index);
		rest = path.mid(index + 1);
	} else {
		name = path;
	}

	//TODO
	return nullptr;
}

void LgpDirectoryItem::debug() const
{
	qDebug() << "Directory" << path();
	for (LgpItem *item : _childItems) {
		item->debug();
	}
}

LgpItemModel::LgpItemModel(Lgp *lgp, QObject *parent) :
	QAbstractItemModel(parent), _lgp(lgp)/*, iconThread(this)*/
{
	root = new LgpDirectoryItem("");
	for (const QString &path : lgp->fileList()) {
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
	if (!item || item->isDirectory()) {
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

	LgpDirectoryItem *parentItem = static_cast<LgpDirectoryItem *>(item(parent));
	if (!parentItem) {
		parentItem = root;
	}

	return createIndex(row, column, parentItem->child(row));
}

LgpItem *LgpItemModel::item(const QModelIndex &index) const
{
	if (index.isValid()) {
		LgpItem *item = static_cast<LgpItem *>(index.internalPointer());
		if (item) {
			return item;
		}
	}
	return nullptr;
}

QModelIndex LgpItemModel::parent(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return QModelIndex();
	}

	LgpItem *childItem = item(index);
	if (!childItem) {
		return QModelIndex();
	}
	LgpItem *parentItem = childItem->parent();

	if (parentItem == root || !parentItem) {
		return QModelIndex();
	}

	int row = parentItem->id();
	if (row < 0) {
		return QModelIndex();
	}

	return createIndex(row, 0, parentItem);
}

int LgpItemModel::rowCount(const QModelIndex &parent) const
{
	LgpItem *parentItem = item(parent);
	if (!parentItem) {
		parentItem = root;
	}

	if (parentItem->isDirectory()) {
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
	if (!index.isValid()) {
		return QVariant();
	}

	LgpItem *lgpItem = item(index);
	if (!lgpItem) {
		return QVariant();
	}

	switch (role) {
	case Qt::EditRole:
	case Qt::DisplayRole:
		switch (index.column()) {
		case 0:		return lgpItem->name();
		case 1:
			if (lgpItem->isDirectory()) {
				return QString();
			} else {
				qint64 fileSize = static_cast<LgpFileItem *>(lgpItem)->fileSize();
				return fileSize < 0 ? tr("?") : QString::number(fileSize);
			}
		default:	break;
		}
		break;
	case Qt::DecorationRole:
		if (index.column() == 0) {
			if (lgpItem->isDirectory()) {
				return directoryIcon;
			} else {
				if (static_cast<LgpFileItem *>(lgpItem)->icon().isNull()) {
					return fileIcon;
				} else {
					return static_cast<LgpFileItem *>(lgpItem)->icon();
				}
			}
		}
		break;
	case Qt::TextAlignmentRole:
		if (index.column() == 1) {
			return Qt::AlignRight;
		}
		break;
	}

	return QVariant();
}

bool LgpItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()
			|| role != Qt::EditRole) {
		return false;
	}

	LgpItem *lgpItem = item(index);

	if (!lgpItem || lgpItem->isDirectory()) {
		return false;
	}
	QString newFilename = value.toString();

	if (lgpItem->path() == newFilename) {
		return false;
	}

	bool ok = static_cast<LgpFileItem *>(lgpItem)->move(newFilename);

	if (ok) {
		emit dataChanged(index, index);
		emit layoutChanged();
	}

	return ok;
}

void LgpItemModel::update(const QModelIndex &index)
{
	if (index.isValid()) {
		emit dataChanged(index, index);
	}
}

QVariant LgpItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (role) {
	case Qt::TextAlignmentRole:
		return Qt::AlignLeft;
	}

	if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
		return QAbstractItemModel::headerData(section, orientation, role);
	}

	switch (section) {
	case 0:		return tr("Name");
	case 1:		return tr("Size");
	default:	return QVariant();
	}
}

void LgpItemModel::sort(int column, Qt::SortOrder order)
{
	return QAbstractItemModel::sort(column, order);

	// FIXME: TODO
	/* switch (column) {
	case 0:
		root->sort(LgpDirectoryItem::ByName, order);
		break;
	case 1:
		root->sort(LgpDirectoryItem::BySize, order);
		break;
	default:
		break;
	} */
}

bool LgpItemModel::insertRow(const QString &name, QIODevice *io,
                             const QModelIndex &parent)
{
	LgpItem *lgpItem = item(parent);

	// Parent must be a directory
	if (lgpItem && !lgpItem->isDirectory()) {
		return false;
	}

	if (!lgpItem) {
		lgpItem = root;
	}

	LgpDirectoryItem *dir = static_cast<LgpDirectoryItem *>(lgpItem);
	QString path;

	if (dir == root) {
		path = name;
	} else {
		path = dir->path() + "/" + name;
	}

	if (_lgp->addFile(path, io)) {
		int row = rowCount();
		beginInsertRows(parent, row, row);
		dir->addChild(path, _lgp);
		endInsertRows();
		return true;
	}

	return false;
}

bool LgpItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
	beginInsertRows(parent, row, row + count - 1);
	endInsertRows();

	return true;
}

bool LgpItemModel::removeRow(const QModelIndex &index)
{
	LgpItem *item = this->item(index);
	if (item && !item->isDirectory()
	        && _lgp->removeFile(item->path())) {
		int row = index.row();
		beginRemoveRows(parent(index), row, row);
		item->parent()->removeChild(item);
		endRemoveRows();
		return true;
	}

	return false;
}

bool LgpItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
	beginRemoveRows(parent, row, row + count - 1);
	endRemoveRows();

	return true;
}

LgpWidget::LgpWidget(Lgp *lgp, QWidget *parent) :
    QWidget(parent),
    lgp(lgp), progressDialog(nullptr), currentImage(0), currentPal(0)
{
	setWindowTitle(tr("LGP archive manager"));
	resize(800, 600);

	_model = new LgpItemModel(lgp);
	treeView = new QTreeView(this);
	treeView->setModel(_model);
	treeView->setUniformRowHeights(true);
	treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	treeView->header()->setStretchLastSection(false);
	treeView->setSortingEnabled(false); // TODO

	renameButton = new QPushButton(tr("Rename"), this);
	renameButton->setShortcut(Qt::Key_F2);
	replaceButton = new QPushButton(tr("Replace"), this);
	replaceButton->setShortcut(QKeySequence("Ctrl+R"));
	extractButton = new QPushButton(tr("Extract"), this);
	extractButton->setShortcut(QKeySequence("Ctrl+E"));
	extractAllButton = new QPushButton(tr("Extract All"), this);
	extractAllButton->setShortcut(QKeySequence("Ctrl+A"));
	addButton = new QPushButton(QIcon(":/images/plus.png"), tr("Add"), this);
	addButton->setShortcut(QKeySequence::New);
	removeButton = new QPushButton(QIcon(":/images/minus.png"), tr("Delete"), this);
	removeButton->setShortcut(QKeySequence::Delete);

	preview = new ArchivePreview(this);

	QHBoxLayout *barLayout = new QHBoxLayout;
	barLayout->addWidget(renameButton);
	barLayout->addWidget(replaceButton);
	barLayout->addWidget(extractButton);
	barLayout->addWidget(extractAllButton);
	barLayout->addWidget(addButton);
	barLayout->addWidget(removeButton);
	barLayout->addStretch();

	QGridLayout *layout = new QGridLayout(this);
	layout->addLayout(barLayout, 0, 0, 1, 2);
	layout->addWidget(treeView, 1, 0);
	layout->addWidget(preview, 1, 1);

	connect(renameButton, SIGNAL(released()), SLOT(renameCurrent()));
	connect(replaceButton, SIGNAL(released()), SLOT(replaceCurrent()));
	connect(extractButton, SIGNAL(released()), SLOT(extractCurrent()));
	connect(extractAllButton, SIGNAL(released()), SLOT(extractAll()));
	connect(addButton, SIGNAL(released()), SLOT(add()));
	connect(removeButton, SIGNAL(released()), SLOT(removeCurrent()));
	connect(treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(setButtonsState()));
	connect(treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(changePreview()));
	connect(preview, SIGNAL(currentImageChanged(int)), SLOT(changeImageInPreview(int)));
	connect(preview, SIGNAL(currentPaletteChanged(int)), SLOT(changeImagePaletteInPreview(int)));

	setButtonsState();
}

void LgpWidget::changePreview()
{
	currentImage = 0;
	currentPal = 0;
	setButtonsState();
	generatePreview();
}

void LgpWidget::generatePreview()
{
	QModelIndex index = treeView->currentIndex();
	if (!index.isValid()) {
		preview->clearPreview();
		return;
	}

	LgpItem *item = _model->item(index);
	if (!item || item->isDirectory()) {
		preview->clearPreview();
		return;
	}

	QString fileName, fileType;

	fileName = item->path();
	fileType = fileName.mid(fileName.lastIndexOf('.')+1).toLower();

	if (fileType == "tex")
	{
		TexFile texFile(lgp->fileData(fileName));
		texFile.setCurrentColorTable(currentPal);
		preview->imagePreview(QPixmap::fromImage(texFile.image()), fileName, currentPal, texFile.colorTableCount());
	}
	else if (fileType == "tim")
	{
		TimFile timFile(lgp->fileData(fileName));
		timFile.setCurrentColorTable(currentPal);
		preview->imagePreview(QPixmap::fromImage(timFile.image()), fileName, currentPal, timFile.colorTableCount());
	}
	else if (fileType == "h" || fileType == "c"
	         || fileType == "sym" || fileType.isEmpty()
	         || fileType == "bak" || fileType == "dir"
	         || fileType == "fl" || fileType == "txt"
	         || fileType == "hrc")
	{
		QByteArray fileData = lgp->fileData(fileName);
		preview->textPreview(QString::fromLatin1(fileData.constData(), strlen(fileData)));
	}
	else if (fileType == "png" || fileType == "jpg" || fileType == "jpeg")
	{
		preview->imagePreview(QPixmap::fromImage(QImage::fromData(lgp->fileData(fileName))), fileName);
	}
	else if (fileType == "rsd")
	{
		CharArchive charArchive(lgp);
		fieldModelFile.clear();
		fieldModelFile.loadPart(&charArchive, fileName);
		preview->modelPreview(&fieldModelFile);
	}
	else
	{
		/* QList<int> indexes = FF8Image::findTims(data);
		if (!indexes.isEmpty())
		{
			TimFile timFile(data.mid(indexes.value(currentImage, indexes.first())));
			timFile.setCurrentColorTable(currentPal);
			preview->imagePreview(QPixmap::fromImage(timFile.image()), fileName,
			                      currentPal, timFile.colorTableCount(),
			                      currentImage, indexes.size());
		}
		else
		{*/
			preview->clearPreview();
		//}
	}
}

void LgpWidget::changeImageInPreview(int imageID)
{
	if (imageID < 0) {
		return;
	}

	currentImage = imageID;
	generatePreview();
}

void LgpWidget::changeImagePaletteInPreview(int palID)
{
	if (palID < 0) {
		return;
	}

	currentPal = palID;
	generatePreview();
}

void LgpWidget::renameCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if (index.isValid()) {
		LgpItem *item = _model->item(index);
		if (item && !item->isDirectory()) {
			bool ok;
			QString oldFilePath = item->path(),
					newFilePath = QInputDialog::getText(this, tr("Rename"), tr("New Name:"), QLineEdit::Normal,
														oldFilePath, &ok, Qt::Dialog | Qt::WindowCloseButtonHint);
			if (!ok || newFilePath == oldFilePath) {
				return;
			}

			if (!lgp->isNameValid(newFilePath)) {
				QMessageBox::warning(this, tr("Error"), tr("The name '%1' is invalid, don't put special characters.")
									 .arg(newFilePath));
			} else if (lgp->fileExists(newFilePath)) {
				QMessageBox::warning(this, tr("Error"), tr("A file named '%1' already exists, please choose another name.")
									 .arg(newFilePath));
			} else if (!_model->setData(index, newFilePath)) {
				QMessageBox::warning(this, tr("Error"), tr("Can not Rename the file"));
			} else {
				treeView->scrollTo(index);
				emit modified();
			}
		}
	}
}

void LgpWidget::replaceCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if (index.isValid()) {
		LgpItem *item = _model->item(index);
		if (item && !item->isDirectory()) {
			QString filename = item->name();
			int indexOf = filename.lastIndexOf('.');
			QString extension = indexOf==-1 ? QString() : filename.mid(indexOf + 1);
			QStringList filter;
			if (!extension.isEmpty()) {
				filter << tr("%1 file (*.%1)").arg(extension);
			}
			filter << tr("All files (*)");

			QString lastDir = Config::value("lgpDialogOpenDirectory").toString();
			if (!lastDir.isEmpty()) {
				filename = lastDir + "/" + filename;
			}

			QString path = QFileDialog::getOpenFileName(this, tr("New File"), filename, filter.join(";;"));
			if (path.isNull()) {
				return;
			}

			Config::setValue("lgpDialogOpenDirectory", path.left(path.lastIndexOf('/')));

			if (!static_cast<LgpFileItem *>(item)->setFile(new QFile(path))) {
				QMessageBox::warning(this, tr("Error"), tr("Can not modify the archive!"));
			} else {
				emit modified();
			}
		}
	}
}

void LgpWidget::extractCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if (index.isValid()) {
		LgpItem *item = _model->item(index);
		if (item && !item->isDirectory()) {
			QString filename = item->name();
			qsizetype index = filename.lastIndexOf('.');
			QString extension = index==-1 ? QString() : filename.mid(index + 1);
			QStringList filter;
			if (!extension.isEmpty()) {
				filter.append(tr("%1 file (*.%1)").arg(extension));
			}
			filter.append(tr("All files (*)"));

			QString lastDir = Config::value("lgpDialogSaveDirectory").toString();
			if (!lastDir.isEmpty()) {
				filename = lastDir + "/" + filename;
			}

			QString path = QFileDialog::getSaveFileName(this, tr("Extract file"), filename, filter.join(";;"));
			if (path.isNull()) {
				return;
			}

			Config::setValue("lgpDialogSaveDirectory", path.left(path.lastIndexOf('/')));

			QIODevice *io = static_cast<LgpFileItem *>(item)->file();
			if (io && io->open(QIODevice::ReadOnly)) {
				QFile file(path);
				if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
					while (io->bytesAvailable()) {
						file.write(io->read(4096));
					}
					if (file.error() != QFile::NoError) {
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

void LgpWidget::extractAll()
{
	QString lastDir = Config::value("lgpDialogSaveDirectory").toString();

	QString path = QFileDialog::getExistingDirectory(this, tr("Extract all"), lastDir);
	if (path.isNull()) {
		return;
	}

	Config::setValue("lgpDialogSaveDirectory", path);

	progressDialog = new QProgressDialog(tr("Extracting..."), tr("Cancel"), 0, 0, this,
	                                     Qt::Dialog | Qt::WindowCloseButtonHint);
	progressDialog->setWindowModality(Qt::WindowModal);
	progressDialog->setAutoClose(false);
	progressDialog->show();

	QDir dir(path);
	QString error;

	setObserverMaximum(static_cast<unsigned int>(lgp->fileCount()));
	int i = 0;

	for (const QString &fileName : lgp->fileList()) {
		if (observerWasCanceled()) {
			break;
		}

		QIODevice *io = lgp->file(fileName);
		if (io && io->open(QIODevice::ReadOnly)) {
			int index = fileName.lastIndexOf('/');
			if (index > 0) {
				dir.mkpath(fileName.left(index));
			}
			QFile file(dir.filePath(fileName));
			if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				while (io->bytesAvailable()) {
					file.write(io->read(4096));
				}
				if (file.error() != QFile::NoError) {
					error = file.errorString();
				}
				file.close();
			} else {
				error = file.errorString();
			}
			io->close();
		}

		setObserverValue(i++);
	}

	progressDialog->hide();
	progressDialog->deleteLater();
	progressDialog = nullptr;

	if (!error.isEmpty()) {
		QMessageBox::warning(this, tr("Error"), error);
	}
}

void LgpWidget::add()
{
	QString dirName;
	QModelIndex index = treeView->currentIndex();
	if (index.isValid()) {
		LgpItem *item = _model->item(index);
		if (item) {
			dirName = item->isDirectory() ? item->path() : item->dirName();
		}
	}

	QString lastDir = Config::value("lgpDialogOpenDirectory").toString();

	QStringList filter;
	filter.append(tr("All files (*)"));
	QStringList paths = QFileDialog::getOpenFileNames(this, tr("New File"), lastDir, filter.join(";;"));
	if (paths.isEmpty()) {
		return;
	}

	QString path = paths.first(),
	        directory = path.left(path.lastIndexOf('/'));
	Config::setValue("lgpDialogOpenDirectory", directory);

	for (const QString &path : qAsConst(paths)) {
		QString filePath = path.mid(path.lastIndexOf('/') + 1);

		if (!dirName.isEmpty()) {
			filePath = dirName + "/" + filePath;
		}

		bool rename = false;
		
		do {
			if (!lgp->isNameValid(filePath)) {
				QMessageBox::warning(this, tr("Error"),
									 tr("The name '%1' is invalid, don't put special characters.")
									 .arg(filePath));
				rename = true;
			} else if (lgp->fileExists(filePath)) {
				QMessageBox::warning(this, tr("Error"),
									 tr("A file named '%1' already exists, please choose another name.")
									 .arg(filePath));
				rename = true;
			} else {
				rename = false;
			}

			if (rename) {
				bool ok;
				filePath = QInputDialog::getText(this, tr("Rename"),
												 tr("New Name:"), QLineEdit::Normal,
												 filePath, &ok,
												 Qt::Dialog | Qt::WindowCloseButtonHint);
				if (!ok) {
					return;
				}
			}
		} while (rename);

		if (!_model->insertRow(filePath, new QFile(path))) {
			QMessageBox::critical(this, tr("Error"),
			                      tr("Can not add the file '%1'")
			                      .arg(path));
		} else {
			treeView->scrollTo(_model->index(_model->rowCount() - 1, 0));
			emit modified();
		}
	}
}

void LgpWidget::removeCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if (index.isValid()) {
		QMessageBox::StandardButton button = QMessageBox::question(this, tr("Delete?"),
							  tr("Are you sure you want to delete this file from the archive?"),
							  QMessageBox::Yes | QMessageBox::Cancel);
		if (button != QMessageBox::Yes) {
			return;
		}

		if (_model->removeRow(index)) {
			emit modified();
		} else {
			QMessageBox::warning(this, tr("Error"), tr("Cannot delete the file!"));
		}
	}
}

bool LgpWidget::observerWasCanceled() const
{
	return progressDialog && progressDialog->wasCanceled();
}

void LgpWidget::setObserverMaximum(unsigned int max)
{
	if (progressDialog) {
		progressDialog->setMaximum(int(max));
	}
}

void LgpWidget::setObserverValue(int value)
{
	QApplication::processEvents();

	if (progressDialog) {
		progressDialog->setValue(value);
	}
}

bool LgpWidget::observerRetry(const QString &message)
{
	return QMessageBox::Retry == QMessageBox::question(this, tr("Error"), message,
	                                                   QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Retry);
}

void LgpWidget::setButtonsState()
{
	QModelIndexList modelIndexList = treeView->selectionModel()->selectedRows();
	LgpItem *item;
	if (!modelIndexList.isEmpty()) {
		item = _model->item(modelIndexList.first());
	} else {
		item = nullptr;
	}
	bool enabled = !modelIndexList.isEmpty() &&
	               item &&
	               !item->isDirectory();

	renameButton->setEnabled(enabled);
	replaceButton->setEnabled(enabled);
	extractButton->setEnabled(enabled);
	removeButton->setEnabled(enabled);
}
