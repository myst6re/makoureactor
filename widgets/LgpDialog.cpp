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

LgpItemModel::LgpItemModel(Lgp *lgp, QObject *parent) :
	QAbstractItemModel(parent), lgp(lgp)
{
	fileList = lgp->fileList();
	QFileIconProvider iconProvider;
	fileIcon = iconProvider.icon(QFileIconProvider::File);
}

QModelIndex LgpItemModel::index(int row, int column, const QModelIndex &parent) const
{
	if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
		return QModelIndex();

	return createIndex(row, column, (QString *)&fileList[row]);
}

QModelIndex LgpItemModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index)
	return QModelIndex();
}

int LgpItemModel::rowCount(const QModelIndex &parent) const
{
	if(!parent.isValid()) {
		return fileList.size();
	} else {
		return 0;
	}
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

	QString *filePath = (QString *)index.internalPointer();
	if(!filePath) {
		return QVariant();
	}

	switch(role) {
	case Qt::EditRole:
	case Qt::DisplayRole:
		QIODevice *io;
		switch(index.column()) {
		case 0:		return *filePath;
		case 1:
			io = lgp->file(*filePath);
			if(io && io->open(QIODevice::ReadOnly)) {
				return io->size();
			} else {
				return tr("?");
			}
		default:	break;
		}
		break;
	case Qt::DecorationRole:
		if(index.column() == 0) {
			return fileIcon;
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

QVariant LgpItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch(role) {
	case Qt::TextAlignmentRole:
		return Qt::AlignLeft;
	}

	if(orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return QAbstractItemModel::headerData(section, orientation, role);

	switch(section) {
	case 0:		return tr("Nom");
	case 1:		return tr("Taille");
	default:	return QVariant();
	}
}

LgpDialog::LgpDialog(Lgp *lgp, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint), lgp(lgp)
{
	setWindowTitle(tr("Gestionnaire d'archive LGP"));
	resize(800, 600);

	LgpItemModel *model = new LgpItemModel(lgp);
	treeView = new QTreeView(this);
	treeView->setModel(model);
	treeView->setUniformRowHeights(true);
	treeView->header()->setResizeMode(0, QHeaderView::Stretch);
	treeView->header()->setStretchLastSection(false);

	renameButton = new QPushButton(tr("Renommer"), this);
	replaceButton = new QPushButton(tr("Remplacer"), this);
	extractButton = new QPushButton(tr("Extraire"), this);
	packButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton),
								 tr("Sauvegarder"), this);
	packButton->setEnabled(false);

	QHBoxLayout *barLayout = new QHBoxLayout;
	barLayout->addWidget(renameButton);
	barLayout->addWidget(replaceButton);
	barLayout->addWidget(extractButton);
	barLayout->addWidget(packButton);
	barLayout->addStretch();

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addLayout(barLayout);
	layout->addWidget(treeView, 1);

	connect(renameButton, SIGNAL(released()), SLOT(renameCurrent()));
	connect(replaceButton, SIGNAL(released()), SLOT(replaceCurrent()));
	connect(extractButton, SIGNAL(released()), SLOT(extractCurrent()));
	connect(packButton, SIGNAL(released()), SLOT(pack()));
	connect(treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(setButtonsState()));

	setButtonsState();
}

void LgpDialog::renameCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if(index.isValid()) {
		QString *filePath = (QString *)index.internalPointer();
		if(filePath) {
			bool ok;
			QString newFilePath = QInputDialog::getText(this, tr("Renommer"), tr("Nouveau nom :"), QLineEdit::Normal,
														*filePath, &ok, Qt::Dialog | Qt::WindowCloseButtonHint);
			if(!ok) {
				return;
			}

			if(!lgp->isNameValid(newFilePath)) {
				QMessageBox::warning(this, tr("Erreur"), tr("Le nom '%1' est invalide, ne mettez pas de caractères spéciaux.")
									 .arg(newFilePath));
			} else if(lgp->fileExists(newFilePath)) {
				QMessageBox::warning(this, tr("Erreur"), tr("Un fichier nommé '%1' existe déjà, veuillez choisir un autre nom.")
									 .arg(newFilePath));
			} else if(!lgp->renameFile(*filePath, newFilePath)) {
				QMessageBox::warning(this, tr("Erreur"), tr("Impossible de renommer le fichier"));
			} else {
				//TODO: update view
				emit modified();
				packButton->setEnabled(true);
			}
		}
	}
}

void LgpDialog::replaceCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if(index.isValid()) {
		QString *filePath = (QString *)index.internalPointer();
		if(filePath) {
			QString filename = filePath->mid(filePath->lastIndexOf('/') + 1);
			int index = filename.lastIndexOf('.');
			QString extension = index==-1 ? QString() : filename.mid(index + 1);
			QStringList filter;
			if(!extension.isEmpty()) {
				filter << tr("Fichier %1 (*.%1)").arg(extension);
			}
			filter << tr("Tous les fichiers (*)");
			QString path = QFileDialog::getOpenFileName(this, tr("Nouveau fichier"), filename, filter.join(";;"));
			if(path.isNull()) {
				return;
			}

			if(!lgp->setFile(*filePath, new QFile(path))) {
				QMessageBox::warning(this, tr("Erreur"), tr("Impossible de modifier l'archive !"));
			} else {
				//TODO: update view (file size)
				emit modified();
				packButton->setEnabled(true);
			}
		}
	}
}

void LgpDialog::extractCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if(index.isValid()) {
		QString *filePath = (QString *)index.internalPointer();
		if(filePath) {
			QString filename = filePath->mid(filePath->lastIndexOf('/') + 1);
			int index = filename.lastIndexOf('.');
			QString extension = index==-1 ? QString() : filename.mid(index + 1);
			QStringList filter;
			if(!extension.isEmpty()) {
				filter << tr("Fichier %1 (*.%1)").arg(extension);
			}
			filter << tr("Tous les fichiers (*)");
			QString path = QFileDialog::getSaveFileName(this, tr("Nouveau fichier"), filename, filter.join(";;"));
			if(path.isNull()) {
				return;
			}

			QIODevice *io = lgp->modifiedFile(*filePath);
			if(io && io->open(QIODevice::ReadOnly)) {
				QFile file(path);
				if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
					while(io->bytesAvailable()) {
						file.write(io->read(4096));
					}
					if(file.error() != QFile::NoError) {
						QMessageBox::warning(this, tr("Erreur d'écriture"), tr("Impossible d'écrire dans le fichier (message : %1).")
											 .arg(file.errorString()));
					}
					file.close();
				} else {
					QMessageBox::warning(this, tr("Erreur d'ouverture"), tr("Impossible d'ouvrir le fichier (message : %1).")
										 .arg(file.errorString()));
				}
				io->close();
			}
		}
	}
}

void LgpDialog::pack()
{
	QString path = QFileDialog::getSaveFileName(this, tr("Enregistrer sous"), lgp->fileName(), tr("Fichier Lgp (*.lgp)"));
	if(path.isNull()) {
		return;
	}

//	QFileInfo info1(path), info2(lgp->fileName());
//	if(info1 == info2) {
//		QMessageBox::warning(this, tr("Action impossible"), tr("Merci de sélectionner un autre fichier que celui actuellement ouvert par le logiciel."));
//		return;
//	}

	progressDialog = new QProgressDialog(tr("Sauvegarde..."), tr("Annuler"), 0, 0, this, Qt::Dialog | Qt::WindowCloseButtonHint);
	progressDialog->setWindowModality(Qt::WindowModal);
	progressDialog->setAutoClose(false);
	progressDialog->show();

	bool ok = lgp->pack(path, this);

	progressDialog->hide();
	progressDialog->deleteLater();

	if(!ok) {
		if(lgp->error() != Lgp::AbortError) {
			QMessageBox::warning(this, tr("Erreur"), tr("Impossible de créer l'archive (message : %1).")
								 .arg(lgp->errorString()));
		}
	} else {
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
	bool enabled = !modelIndexList.isEmpty();

	renameButton->setEnabled(enabled);
	replaceButton->setEnabled(enabled);
	extractButton->setEnabled(enabled);
}
