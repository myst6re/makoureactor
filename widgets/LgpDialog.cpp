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
	resize(800, 600);

	LgpItemModel *model = new LgpItemModel(lgp);
	treeView = new QTreeView(this);
	treeView->setModel(model);

	replaceButton = new QPushButton(tr("Remplacer"), this);
	extractButton = new QPushButton(tr("Extraire"), this);
	packButton = new QPushButton(tr("Créer l'image disque modifiée"), this);

	QHBoxLayout *barLayout = new QHBoxLayout;
	barLayout->addWidget(replaceButton);
	barLayout->addWidget(extractButton);
	barLayout->addWidget(packButton);
	barLayout->addStretch();

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addLayout(barLayout);
	layout->addWidget(treeView, 1);

	connect(replaceButton, SIGNAL(released()), SLOT(replaceCurrent()));
	connect(extractButton, SIGNAL(released()), SLOT(extractCurrent()));
	connect(packButton, SIGNAL(released()), SLOT(pack()));
	connect(treeView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(setButtonsState()));
}

void LgpDialog::replaceCurrent()
{
	QModelIndex index = treeView->currentIndex();
	if(index.isValid()) {
		QString *filePath = (QString *)index.internalPointer();
		if(filePath) {
			QString extension = filePath->mid(filePath->lastIndexOf('.') + 1);
			QString path = QFileDialog::getOpenFileName(this, tr("Nouveau fichier"), "", tr("Fichier %1 (*.%1);;Tous les fichiers (*)").arg(extension));
			if(path.isNull()) {
				return;
			}

			if(!lgp->setFile(*filePath, new QFile(path))) {
				QMessageBox::warning(this, tr("Erreur"), tr("Impossible de modifier l'archive !"));
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
			QString extension = filePath->mid(filePath->lastIndexOf('.') + 1);
			QString path = QFileDialog::getOpenFileName(this, tr("Nouveau fichier"), "", tr("Fichier %1 (*.%1);;Tous les fichiers (*)").arg(extension));
			if(path.isNull()) {
				return;
			}

			QIODevice *io = lgp->modifiedFile(*filePath);
			if(io && io->open(QIODevice::ReadOnly)) {
				QFile file(path);
				if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
					//TODO
				}
//TODO:error
			}
		}
	}
}

void LgpDialog::pack()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Enregistrer sous"), "", tr("Fichier Lgp (*.lgp)"));
	if(path.isNull()) {
		return;
	}

	QFileInfo info1(path), info2(lgp->fileName());
	if(info1 == info2) {
		QMessageBox::warning(this, tr("Action impossible"), tr("Merci de sélectionner un autre fichier que celui actuellement ouvert par le logiciel."));
		return;
	}

	if(!lgp->pack(path)) {//TODO: observer
		if(lgp->error() != Lgp::AbortError) {
			QMessageBox::warning(this, tr("Erreur"), tr("Impossible de créer l'archive (message : %1).").arg(lgp->errorString()));
		} else {
			return;
		}
	}
}

void LgpDialog::setButtonsState()
{
	/*QModelIndexList modelIndexList = treeView->selectionModel()->selectedRows();
	if(!modelIndexList.isEmpty()) {
		QModelIndex index = modelIndexList.first();
		replaceButton->setEnabled(((IsoFileOrDirectory *)index.internalPointer())->isFile());
	} else {
		replaceButton->setEnabled(false);
	}*/
}
