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

TutWidget::TutWidget(QWidget *parent) :
	QDialog(parent, Qt::Tool), copied(false)
{
	setWindowTitle(tr("Tutoriels/Musiques"));

	ListWidget *_list = new ListWidget(this);
	_list->setFixedWidth(70);
	_list->addAction(ListWidget::Add, tr("Ajouter"), this, SLOT(add()));
	_list->addAction(ListWidget::Rem, tr("Supprimer"), this, SLOT(del()));
//	_list->addSeparator(true);
//	_list->addAction(ListWidget::Cut, tr("Couper"), this, SLOT(cutCurrent()), true);
//	_list->addAction(ListWidget::Copy, tr("Copier"), this, SLOT(copyCurrent()), true);
//	_list->addAction(ListWidget::Paste, tr("Coller"), this, SLOT(pasteOnCurrent()), true);
	list = _list->listWidget();
//	list->setSelectionMode(QAbstractItemView::ExtendedSelection);

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

	versionPS = new QRadioButton(tr("PlayStation"), this);
	versionPC = new QRadioButton(tr("PC"), this);

	exportLayout->addWidget(versionPS);
	exportLayout->addWidget(versionPC);

	connect(versionPS, SIGNAL(toggled(bool)), SLOT(changeVersion(bool)));

	exportLayout->addStretch();
	exportLayout->addWidget(exportButton);
	exportLayout->addWidget(importButton);

	QGridLayout *layout = new QGridLayout(this);
	layout->addLayout(exportLayout, 0, 0, 1, 2);
	layout->addWidget(_list, 1, 0);
	layout->addWidget(stackedWidget, 1, 1);

	connect(list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(showText(QListWidgetItem*,QListWidgetItem*)));

	connect(exportButton, SIGNAL(released()), SLOT(exportation()));
	connect(importButton, SIGNAL(released()), SLOT(importation()));
}

void TutWidget::fill(Field *field, TutFilePC *tutPC, bool reload)
{
	if((!reload && this->tut == field->tutosAndSounds()) || !field)	return;
	clear();
	tut = field->tutosAndSounds();
	this->tutPC = tutPC;
	this->field = field;
	usedTuts = field->scriptsAndTexts()->listUsedTuts();
	currentTut = tut;

	if(tutPC != NULL && tut->hasTut()) {
		versionPS->setVisible(true);
		versionPC->setVisible(true);
		versionPS->setChecked(true);
		changeVersion(true);
	} else {
		versionPS->setVisible(false);
		versionPC->setVisible(false);
		fillList();
		list->setCurrentRow(0);
	}
}

void TutWidget::clear()
{
	tut = 0;
	tutPC = 0;
	currentTut = 0;
	usedTuts.clear();
	list->clear();
	textEdit->clear();
	akaoIDList->setCurrentIndex(-1);
	akaoIDList->setEnabled(false);
	akaoDesc->clear();
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
		akaoIDList->addItem(tr("%1 - %2 : %3")
							.arg(i, 2, 10, QChar('0'))
							.arg(musicName)
							.arg(Data::music_desc.at(i)), i);
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

	textEdit->blockSignals(true);
	textEdit->clear();
	textEdit->blockSignals(false);
	fillList();
	list->setCurrentRow(0);
}

void TutWidget::setTextChanged()
{
	saveText(list->currentItem());
}

void TutWidget::fillList()
{
	list->blockSignals(true);
	list->clear();
	int size = currentTut->size();

	for(int i=0 ; i<size ; ++i) {
		list->addItem(createListItem(i));
	}
	list->blockSignals(false);
}

QListWidgetItem *TutWidget::createListItem(int id) const
{
	return new QListWidgetItem((currentTut->isTut(id)
								? tr("Tuto %1")
								: tr("Musique %1"))
							   .arg(id));
}

int TutWidget::currentRow(QListWidgetItem *item) const
{
	if(!item) {
		item = list->currentItem();
	}
	if(item) {
		return list->row(item);
	}
	return -1;
}

QList<int> TutWidget::selectedRows() const
{
	QList<int> ret;
	QList<QListWidgetItem *> items = list->selectedItems();
	foreach(QListWidgetItem *item, items) {
		ret.append(list->row(item));
	}

	return ret;
}

void TutWidget::showText(QListWidgetItem *item, QListWidgetItem *lastItem)
{
	if(item == NULL)	return;

	if(lastItem != NULL) {
		saveText(lastItem);
	}

	int id = currentRow(item);
	bool isTut = currentTut->isTut(id);

	if(isTut) {
		stackedWidget->setCurrentIndex(0);
		textEdit->setPlainText(currentTut->parseScripts(id));
	} else {
		stackedWidget->setCurrentIndex(1);
		akaoDesc->setText(currentTut->parseScripts(id));
		quint16 akaoID = tut->akaoID(id);
		akaoIDList->setEnabled(true);
		int index = akaoIDList->findData(akaoID);
		if(index != -1) {
			akaoIDList->setCurrentIndex(index);
		} else {
			akaoIDList->setEditText(QString::number(akaoID));
		}
	}

	textEdit->setReadOnly(!isTut);
	exportButton->setEnabled(true);
	importButton->setEnabled(true);
}

void TutWidget::saveText(QListWidgetItem *item)
{
	if(item == NULL)	return;

	int id = currentRow(item);
	if(currentTut->isTut(id)) {
		currentTut->parseText(id, textEdit->toPlainText());
		if(currentTut->isModified()) {
			emit modified();
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
				emit modified();
			}
		}
	}
}

void TutWidget::add()
{
	int row = currentRow();
	if(row < 0) {
		row = currentTut->size();
	} else {
		row += 1;
	}

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
		chooseLayout->addWidget(buttonBox, 1, 0, 1, 2);
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

	if(addTut) {
		currentTut->insertTut(row);
		Section1File *scriptsAndTexts = field->scriptsAndTexts();
		if(tutPC == NULL)
			scriptsAndTexts->shiftTutIds(row-1, +1);
		usedTuts = scriptsAndTexts->listUsedTuts();
	} else {
		currentTut->insertData(row, akaoPath);
	}
	emit modified();
	fillList();

	list->setCurrentItem(list->item(row < list->count() ? row : list->count()-1));
}

void TutWidget::del()
{
	int row = currentRow();
	if(row < 0) {
		return;
	}

	if((tutPC == NULL || currentTut == tutPC) && usedTuts.contains(row)) {
		QMessageBox::StandardButton rep = QMessageBox::warning(this, tr("Tutoriel utilisé dans les script"), currentTut == tutPC ? tr("Ce tutoriel est peut-être utilisé par un ou plusieurs scripts de cet écran.\nLe supprimer peut provoquer des erreurs.\nÊtes-vous sûr de vouloir continuer ?") : tr("Ce tutoriel est utilisé par un ou plusieurs scripts de cet écran.\nLe supprimer remplacera les appels à ce tutoriel par des appels au tutoriel qui suit.\nÊtes-vous sûr de vouloir continuer ?"), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
		if(rep == QMessageBox::Cancel) 	return;
	}

	currentTut->removeTut(row);
	emit modified();
	Section1File *scriptsAndTexts = field->scriptsAndTexts();
	if(tutPC == NULL)
		scriptsAndTexts->shiftTutIds(row, -1);
	usedTuts = scriptsAndTexts->listUsedTuts();
	fillList();

	list->setCurrentRow(row < list->count() ? row : list->count()-1);
}

void TutWidget::cutCurrent()
{
	cut(selectedRows());
}

void TutWidget::copyCurrent()
{
	copy(selectedRows());
}

void TutWidget::pasteOnCurrent()
{
	int row = currentRow();
	if(row < 0) {
		row = list->count();
	} else {
		row += 1;
	}
	paste(row);
}

void TutWidget::cut(const QList<int> &rows)
{
	if(rows.isEmpty()) {
		return;
	}

	copy(rows);

	Section1File *scriptsAndTexts = field->scriptsAndTexts();

	QList<int> sortedRows = rows;
	qSort(sortedRows);

	for(int i=sortedRows.size()-1 ; i>=0 ; --i) {
		int row = sortedRows.at(i);
		currentTut->removeTut(row);
		if(tutPC == NULL)
			scriptsAndTexts->shiftTutIds(row, -1);
	}

	usedTuts = scriptsAndTexts->listUsedTuts();
	fillList();
	list->setCurrentRow(sortedRows.first());

	emit modified();
}

void TutWidget::copy(const QList<int> &rows)
{
	if(rows.isEmpty()) {
		return;
	}

	_copiedData.clear();
	foreach(int row, rows) {
		_copiedData.append(currentTut->data(row));
	}
	copied = true;
}

void TutWidget::paste(int row)
{
	if(!copied) {
		return;
	}

	for(int i=_copiedData.size()-1 ; i>=0 ; --i) {
		currentTut->insertData(row, _copiedData.at(i));
	}

	fillList();
	list->selectionModel()->select(
				QItemSelection(list->model()->index(row, 0),
							   list->model()->index(row + _copiedData.size()-1, 0)),
				QItemSelectionModel::ClearAndSelect);

	emit modified();
}

void TutWidget::exportation()
{
	int row = currentRow();
	if(row < 0) {
		return;
	}

	bool isTut = currentTut->isTut(row);
	QString path = QDir::fromNativeSeparators(QDir::cleanPath(Config::value("akaoImportExportPath").toString()));
	QString filename, filter;

	if(isTut) {
		filename = tr("tuto_%1.tutps").arg(row);
		filter = tr("Tuto Final Fantasy VII PS (*.tutps)");
	} else {
		filename = tr("son_%1.akao").arg(row);
		filter = tr("Son Final Fantasy (*.akao)");
	}

	path = QFileDialog::getSaveFileName(this, tr("Exporter"), path + "/" + filename, filter);

	if(path.isNull()) {
		return;
	}

	Config::setValue("akaoImportExportPath", path.left(path.lastIndexOf('/')));

	QFile akao(path);
	if(!akao.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QMessageBox::warning(this, tr("Erreur"), tr("Erreur d'ouverture du fichier"));
		return;
	}

	akao.write(currentTut->data(row));
	akao.close();
}

void TutWidget::importation()
{
	int row = currentRow();
	if(row < 0) {
		return;
	}

	bool isTut = currentTut->isTut(row);

	QString path = QDir::fromNativeSeparators(QDir::cleanPath(Config::value("akaoImportExportPath").toString()));
	QString filename, filter;

	if(isTut) {
		filename = tr("tuto_%1.tutps").arg(row);
		filter = tr("Tuto Final Fantasy VII PS (*.tutps)");
	} else {
		filename = tr("son_%1.akao").arg(row);
		filter = tr("Son Final Fantasy (*.akao)");
	}

	path = QFileDialog::getOpenFileName(this, tr("Importer"), path + "/" + filename, filter);

	if(path.isNull()) {
		return;
	}

	Config::setValue("akaoImportExportPath", path.left(path.lastIndexOf('/')));

	QFile akao(path);
	if(!akao.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(this, tr("Erreur"), tr("Erreur d'ouverture du fichier"));
		return;
	}

	if(akao.size() > 1000000) { // FIXME: remove fixed limitation
		QMessageBox::warning(this, tr("Erreur"), tr("Fichier trop gros"));
		return;
	}

	currentTut->setData(row, akao.readAll());
	akao.close();
	emit modified();

	isTut = currentTut->isTut(row);

	textEdit->setPlainText(currentTut->parseScripts(row));
	textEdit->setReadOnly(!isTut);
}
