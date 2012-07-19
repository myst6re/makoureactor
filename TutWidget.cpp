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
#include "TutWidget.h"

TutWidget::TutWidget(Field *field, TutFile *tut, TutFile *tutPC, QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint), field(field), tut(tut), tutPC(tutPC), tutCpy(*tut), textChanged(false)
{
	setWindowTitle(tr("Tutoriels/Sons"));

	QToolBar *_toolBar = new QToolBar();
	_toolBar->setIconSize(QSize(14,14));
	QAction *add_A = _toolBar->addAction(QIcon(":/images/plus.png"), tr("Ajouter tutoriel"), this, SLOT(add()));
	add_A->setShortcut(QKeySequence("Ctrl++"));
	QAction *del_A = _toolBar->addAction(QIcon(":/images/minus.png"), tr("Supprimer tutoriel"), this, SLOT(del()));
	del_A->setShortcut(Qt::Key_Delete);


	list = new QListWidget(this);
	list->setFixedWidth(70);
	textEdit = new QPlainTextEdit(this);
	textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
	new TextHighlighter(textEdit->document(), true);
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

	exportButton = new QPushButton(tr("Exporter..."));
	importButton = new QPushButton(tr("Importer..."));
	exportButton->setEnabled(false);
	importButton->setEnabled(false);

	QHBoxLayout *exportLayout = new QHBoxLayout();
	exportLayout->setContentsMargins(QMargins());

	if(tutPC != NULL)	tutPCCpy = *tutPC;

	if(tutPC != NULL && tut->hasTut()) {
		versionPS = new QRadioButton(tr("PlayStation"), this);

		exportLayout->addWidget(versionPS);
		exportLayout->addWidget(new QRadioButton(tr("PC"), this));

		connect(versionPS, SIGNAL(toggled(bool)), SLOT(changeVersion(bool)));
	}

	exportLayout->addStretch();
	exportLayout->addWidget(exportButton);
	exportLayout->addWidget(importButton);

	QGridLayout *layout = new QGridLayout(this);
	layout->addLayout(exportLayout, 0, 0, 1, 2);
	layout->addWidget(_toolBar, 1, 0);
	layout->addWidget(list, 2, 0);
	layout->addWidget(textEdit, 1, 1, 2, 1);
	layout->addWidget(buttonBox, 3, 0, 1, 2, Qt::AlignRight);

	currentTut = tut;

	connect(list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(showText(QListWidgetItem*,QListWidgetItem*)));
	connect(textEdit, SIGNAL(textChanged()), SLOT(setTextChanged()));

	if(tutPC != NULL && tut->hasTut()) {
		versionPS->setChecked(true);
	} else {
		fillList();
		list->setCurrentRow(0);
	}

	usedTuts = field->listUsedTuts();

	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

	connect(exportButton, SIGNAL(released()), SLOT(exportation()));
	connect(importButton, SIGNAL(released()), SLOT(importation()));
}

void TutWidget::changeVersion(bool isPS)
{
	saveText(list->currentItem());

	if(isPS) {
		currentTut = tut;
	} else {
		currentTut = tutPC;
	}

	textEdit->clear();
	textChanged = false;
	fillList();
	list->setCurrentRow(0);
}

void TutWidget::setTextChanged()
{
	textChanged = true;
}

void TutWidget::fillList()
{
	list->clear();
	int size = currentTut->size();

	for(int i=0 ; i<size ; ++i) {
		QListWidgetItem *item = new QListWidgetItem((currentTut->isTut(i) ? tr("Tuto %1") : tr("Son %1")).arg(i));
		item->setData(Qt::UserRole, i);
		list->addItem(item);
	}
}

void TutWidget::showText(QListWidgetItem *item, QListWidgetItem *lastItem)
{
	if(item == NULL)	return;

	if(lastItem!=NULL) {
		saveText(lastItem);
	}

	int id = item->data(Qt::UserRole).toInt();

	textEdit->setPlainText(currentTut->parseScripts(id));
	textEdit->setReadOnly(!currentTut->isTut(id));
	exportButton->setEnabled(!currentTut->isTut(id));
	importButton->setEnabled(!currentTut->isTut(id));

	textChanged = false;
}

void TutWidget::saveText(QListWidgetItem *item)
{
	if(item != NULL && textChanged) {
		int id = item->data(Qt::UserRole).toInt();
		if(currentTut->isTut(id)) {
			currentTut->parseText(id, textEdit->toPlainText());
			textChanged = false;
		}
	}
}

void TutWidget::add()
{
	QListWidgetItem *item = list->currentItem();
	int row = !item ? currentTut->size() : item->data(Qt::UserRole).toInt()+1;

	if(row < currentTut->size() && currentTut == tutPC) {
		QMessageBox::StandardButton rep = QMessageBox::warning(this, tr("Tutoriel utilisé dans les script"), tr("Insérer un tutoriel ici va décaler les identifiants des tutoriels qui suit, cela risque de poser problème.\nÊtes-vous sûr de vouloir continuer ?"), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
		if(rep == QMessageBox::Cancel) 	return;
	}

	list->blockSignals(true);

	currentTut->insertTut(row);
	if(tutPC == NULL)
		field->shiftTutIds(row-1, +1);
	usedTuts = field->listUsedTuts();
	fillList();

	list->blockSignals(false);
	list->setCurrentItem(list->item(row < list->count() ? row : list->count()-1));
}

void TutWidget::del()
{
	QListWidgetItem *item = list->currentItem();
	if(!item) return;

	int row = item->data(Qt::UserRole).toInt();

	if((tutPC == NULL || currentTut == tutPC) && usedTuts.contains(row)) {
		QMessageBox::StandardButton rep = QMessageBox::warning(this, tr("Tutoriel utilisé dans les script"), currentTut == tutPC ? tr("Ce tutoriel est peut-être utilisé par un ou plusieurs scripts de cet écran.\nLe supprimer peut provoquer des erreurs.\nÊtes-vous sûr de vouloir continuer ?") : tr("Ce tutoriel est utilisé par un ou plusieurs scripts de cet écran.\nLe supprimer remplacera les appels à ce tutoriel par des appels au tutoriel qui suit.\nÊtes-vous sûr de vouloir continuer ?"), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
		if(rep == QMessageBox::Cancel) 	return;
	}
	list->blockSignals(true);

	currentTut->removeTut(row);
	if(tutPC == NULL)
		field->shiftTutIds(row, -1);
	usedTuts = field->listUsedTuts();
	fillList();

	list->blockSignals(false);
	list->setCurrentRow(row < list->count() ? row : list->count()-1);
}

void TutWidget::exportation()
{
	QListWidgetItem *item = list->currentItem();
	if(!item) return;

	int row = item->data(Qt::UserRole).toInt();
	if(currentTut->isTut(row))	return;

	QString path = QFileDialog::getSaveFileName(this, tr("Exporter"), tr("son_%1.akao").arg(row), "Final Fantasy Sound (*.akao)");
	if(path.isNull())	return;

	QFile akao(path);
	if(!akao.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this, tr("Erreur"), tr("Erreur d'ouverture du fichier"));
		return;
	}

	akao.write(currentTut->data(row));
	akao.close();
}

void TutWidget::importation()
{
	QListWidgetItem *item = list->currentItem();
	if(!item) return;

	int row = item->data(Qt::UserRole).toInt();
	if(currentTut->isTut(row))	return;

	QString path = QFileDialog::getOpenFileName(this, tr("Importer"), tr("son_%1.akao").arg(row), "Final Fantasy Sound (*.akao)");
	if(path.isNull())	return;

	QFile akao(path);
	if(!akao.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(this, tr("Erreur"), tr("Erreur d'ouverture du fichier"));
		return;
	}
	if(akao.size() > 1000000) {
		QMessageBox::warning(this, tr("Erreur"), tr("Fichier trop gros"));
		return;
	}

	currentTut->setData(row, akao.readAll());
	akao.close();

	textEdit->setPlainText(currentTut->parseScripts(row));
	textChanged = false;
}

void TutWidget::accept()
{
	saveText(list->currentItem());

	QDialog::accept();
}

void TutWidget::reject()
{
	*tut = tutCpy;
	if(tutPC != NULL)	*tutPC = tutPCCpy;

	QDialog::reject();
}
