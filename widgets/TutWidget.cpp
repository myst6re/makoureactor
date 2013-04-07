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
#include "TextHighlighter.h"
#include "Data.h"
#include "core/Config.h"

TutWidget::TutWidget(Field *field, TutFileStandard *tut, TutFilePC *tutPC, QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint),
	field(field), tut(tut), tutCpy(*tut), tutPC(tutPC), textChanged(false)
{
	setWindowTitle(tr("Tutoriels/Musiques"));

	QToolBar *_toolBar = new QToolBar();
	_toolBar->setIconSize(QSize(14,14));
	QAction *add_A = _toolBar->addAction(QIcon(":/images/plus.png"), tr("Ajouter tutoriel"), this, SLOT(add()));
	add_A->setShortcut(QKeySequence("Ctrl++"));
	QAction *del_A = _toolBar->addAction(QIcon(":/images/minus.png"), tr("Supprimer tutoriel"), this, SLOT(del()));
	del_A->setShortcut(Qt::Key_Delete);

	list = new QListWidget(this);
	list->setFixedWidth(70);

	stackedWidget = new QStackedWidget();
	stackedWidget->addWidget(buildTutPage());
	stackedWidget->addWidget(buildSoundPage());
	stackedWidget->setCurrentIndex(0);

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

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

	QGridLayout *layout = new QGridLayout(this);
	layout->addLayout(exportLayout, 0, 0, 1, 2);
	layout->addWidget(_toolBar, 1, 0);
	layout->addWidget(list, 2, 0);
	layout->addWidget(stackedWidget, 1, 1, 2, 1);
	layout->addWidget(buttonBox, 3, 0, 1, 2, Qt::AlignRight);

	currentTut = tut;

	connect(list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(showText(QListWidgetItem*,QListWidgetItem*)));

	if(tutPC != NULL && tut->hasTut()) {
		versionPS->setChecked(true);
	} else {
		fillList();
		list->setCurrentRow(0);
	}

	usedTuts = field->scriptsAndTexts()->listUsedTuts();

	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

	connect(exportButton, SIGNAL(released()), SLOT(exportation()));
	connect(importButton, SIGNAL(released()), SLOT(importation()));
}

QWidget *TutWidget::buildTutPage()
{
	textEdit = new QPlainTextEdit(this);
	textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
	new TextHighlighter(textEdit->document(), true);

	connect(textEdit, SIGNAL(textChanged()), SLOT(setTextChanged()));

	return textEdit;
}

QWidget *TutWidget::buildSoundPage()
{
	QWidget *ret = new QWidget(this);

	akaoIDList = new QComboBox(ret);
	akaoIDList->setEditable(true);
	int i=0;
	foreach(const QString &musicName, Data::music_names) {
		akaoIDList->addItem(QString("%1 - %2").arg(i, 2, 10, QChar('0')).arg(musicName), i);
		++i;
	}

	akaoDesc = new QLabel(ret);
	akaoDesc->setTextFormat(Qt::PlainText);
	akaoDesc->setWordWrap(true);

	QGridLayout *layout = new QGridLayout(ret);
	layout->addWidget(new QLabel(tr("ID musique :")), 0, 0);
	layout->addWidget(akaoIDList, 0, 1);
	layout->addWidget(akaoDesc, 1, 0, 1, 2);
	layout->setRowStretch(2, 1);
	layout->setContentsMargins(QMargins());

	return ret;
}

void TutWidget::changeVersion(bool isPS)
{
	saveText(list->currentItem());

	stackedWidget->setCurrentIndex(0);

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
		QListWidgetItem *item = new QListWidgetItem((currentTut->isTut(i) ? tr("Tuto %1") : tr("Musique %1")).arg(i));
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

	if(currentTut->isTut(id)) {
		stackedWidget->setCurrentIndex(0);
		textEdit->setPlainText(currentTut->parseScripts(id));
	} else {
		stackedWidget->setCurrentIndex(1);
		akaoDesc->setText(currentTut->parseScripts(id));
		quint16 akaoID = tut->akaoID(id);
		int index = akaoIDList->findData(akaoID);
		if(index != -1) {
			akaoIDList->setCurrentIndex(index);
		} else {
			akaoIDList->setEditText(QString::number(akaoID));
		}
	}

	textEdit->setReadOnly(!currentTut->isTut(id));
	exportButton->setEnabled(!currentTut->isTut(id));
	importButton->setEnabled(!currentTut->isTut(id));

	textChanged = false;
}

void TutWidget::saveText(QListWidgetItem *item)
{
	if(item == NULL)	return;

	int id = item->data(Qt::UserRole).toInt();
	if(currentTut->isTut(id)) {
		if(textChanged) {
			currentTut->parseText(id, textEdit->toPlainText());
			textChanged = false;
		}
	} else {
		quint16 akaoID;

		QString userText = akaoIDList->lineEdit()->text();
		QRegExp regExp("\\d+");
		if(regExp.indexIn(userText) != -1) {
			userText = regExp.capturedTexts().first();
			akaoID = userText.toInt();

			if(tut->akaoID(id) != akaoID) {
				tut->setAkaoID(id, akaoID);
			}
		}
	}
}

void TutWidget::add()
{
	QListWidgetItem *item = list->currentItem();
	int row = !item ? currentTut->size() : item->data(Qt::UserRole).toInt()+1;

	QString akaoPath;
	bool addTut = true;
	if(currentTut == tut) {
		QDialog chooseType(this, Qt::Dialog | Qt::WindowCloseButtonHint);
		QRadioButton *tutType = new QRadioButton(tr("Tutoriel"), &chooseType);
		QRadioButton *akaoType = new QRadioButton(tr("Musique"), &chooseType);
		tutType->setChecked(true);
		QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &chooseType);
		QGridLayout *chooseLayout = new QGridLayout(&chooseType);
		chooseLayout->addWidget(tutType, 0, 0);
		chooseLayout->addWidget(akaoType, 0, 1);
		chooseLayout->addWidget(buttonBox, 1, 0, 1, 2, Qt::AlignRight);
		connect(buttonBox, SIGNAL(accepted()), &chooseType, SLOT(accept()));
		connect(buttonBox, SIGNAL(rejected()), &chooseType, SLOT(reject()));
		if(chooseType.exec() != QDialog::Accepted) {
			return;
		}

		addTut = tutType->isChecked();

		if(!addTut) {
			akaoPath = QDir::fromNativeSeparators(QDir::cleanPath(Config::value("akaoImportExportPath").toString()));
			akaoPath = QFileDialog::getOpenFileName(this, tr("Importer"), akaoPath + "/" + tr("son_%1.akao").arg(row), tr("Son Final Fantasy (*.akao)"));
			if(akaoPath.isNull())	return;
			Config::setValue("akaoImportExportPath", akaoPath.left(akaoPath.lastIndexOf('/')));
		}
	}

	if(addTut && row < currentTut->size() && currentTut == tutPC) {
		QMessageBox::StandardButton rep = QMessageBox::warning(this, tr("Tutoriel utilisé dans les script"), tr("Insérer un tutoriel ici va décaler les identifiants des tutoriels qui suit, cela risque de poser problème.\nÊtes-vous sûr de vouloir continuer ?"), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
		if(rep == QMessageBox::Cancel) 	return;
	}

	list->blockSignals(true);

	if(addTut) {
		currentTut->insertTut(row);
		Section1File *scriptsAndTexts = field->scriptsAndTexts();
		if(tutPC == NULL)
			scriptsAndTexts->shiftTutIds(row-1, +1);
		usedTuts = scriptsAndTexts->listUsedTuts();
	} else {
		currentTut->insertData(row, akaoPath);
	}
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
	Section1File *scriptsAndTexts = field->scriptsAndTexts();
	if(tutPC == NULL)
		scriptsAndTexts->shiftTutIds(row, -1);
	usedTuts = scriptsAndTexts->listUsedTuts();
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

	QString path = QDir::fromNativeSeparators(QDir::cleanPath(Config::value("akaoImportExportPath").toString()));
	path = QFileDialog::getSaveFileName(this, tr("Exporter"), path + "/" + tr("son_%1.akao").arg(row), tr("Son Final Fantasy (*.akao)"));
	if(path.isNull())	return;
	Config::setValue("akaoImportExportPath", path.left(path.lastIndexOf('/')));

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

	QString path = QDir::fromNativeSeparators(QDir::cleanPath(Config::value("akaoImportExportPath").toString()));
	path = QFileDialog::getOpenFileName(this, tr("Importer"), path + "/" + tr("son_%1.akao").arg(row), tr("Son Final Fantasy (*.akao)"));
	if(path.isNull())	return;
	Config::setValue("akaoImportExportPath", path.left(path.lastIndexOf('/')));

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
