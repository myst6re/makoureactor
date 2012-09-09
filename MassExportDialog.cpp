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
#include "MassExportDialog.h"
#include "Config.h"

MassExportDialog::MassExportDialog(QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint),
	_fieldArchive(0), _fieldID(0)
{
	setWindowTitle(tr("Exporter en masse"));

	fieldList = new QListWidget(this);
	fieldList->setUniformItemSizes(true);
	fieldList->setFixedWidth(200);
	fieldList->setSelectionMode(QAbstractItemView::MultiSelection);

	buildBgExportWidget();
	buildAkaoExportWidget();
	buildTextExportWidget();

	dirPath = new QLineEdit(this);
	dirPath->setText(Config::value("exportDirectory").toString());
	changeDir = new QPushButton(tr("Choisir..."), this);

	overwriteIfExists = new QCheckBox(tr("Écraser les fichiers existants"), this);
	overwriteIfExists->setChecked(Config::value("overwriteOnExport", true).toBool());

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
	buttonBox->addButton(tr("Exporter"), QDialogButtonBox::AcceptRole);
	buttonBox->addButton(QDialogButtonBox::Cancel);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(fieldList, 0, 0, 6, 1);
	layout->addWidget(bgExport, 0, 1, 1, 2);
	layout->addWidget(akaoExport, 1, 1, 1, 2);
	layout->addWidget(textExport, 2, 1, 1, 2);
	layout->addWidget(new QLabel(tr("Emplacement de l'export :")), 3, 1, 1, 2);
	layout->addWidget(dirPath, 4, 1);
	layout->addWidget(changeDir, 4, 2);
	layout->addWidget(overwriteIfExists, 5, 1, 1, 2);
	layout->addWidget(buttonBox, 7, 0, 1, 3, Qt::AlignRight);
	layout->setRowStretch(6, 1);
	layout->setColumnStretch(1, 1);

	connect(changeDir, SIGNAL(clicked()),  SLOT(chooseExportDirectory()));
	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
}

void MassExportDialog::buildBgExportWidget()
{
	bgExport = new QGroupBox(tr("Exporter les décors"), this);
	bgExport->setCheckable(true);
	bgExport->setChecked(true);

	bgFormat = new QComboBox(bgExport);
	bgFormat->addItem(tr("Image PNG"));
	bgFormat->addItem(tr("Image JPG"));
	bgFormat->addItem(tr("Image BMP"));
	bgFormat->setCurrentIndex(Config::value("exportBackgroundFormat").toInt());

	QVBoxLayout *layout = new QVBoxLayout(bgExport);
	layout->addWidget(bgFormat);
}

void MassExportDialog::buildAkaoExportWidget()
{
	akaoExport = new QGroupBox(tr("Exporter les sons"), this);
	akaoExport->setCheckable(true);
	akaoExport->setChecked(false);

	akaoFormat = new QComboBox(akaoExport);
	akaoFormat->addItem(tr("Son AKAO"));
	akaoFormat->setEnabled(false);

	QVBoxLayout *layout = new QVBoxLayout(akaoExport);
	layout->addWidget(akaoFormat);
}

void MassExportDialog::buildTextExportWidget()
{
	textExport = new QGroupBox(tr("Exporter les textes"), this);
	textExport->setCheckable(true);
	textExport->setChecked(false);

	textFormat = new QComboBox(textExport);
	textFormat->addItem(tr("Texte simple TXT"));
	textFormat->setEnabled(false);

	QVBoxLayout *layout = new QVBoxLayout(textExport);
	layout->addWidget(textFormat);
}

void MassExportDialog::fill(FieldArchive *fieldArchive, int fieldID)
{
	_fieldArchive = fieldArchive;
	_fieldID = fieldID;

	fieldList->clear();
	for(int i=0 ; i<_fieldArchive->size() ; ++i) {
		Field *field = _fieldArchive->field(i, false);
		if(field) {
			QListWidgetItem *item = new QListWidgetItem(field->getName());
			item->setData(Qt::UserRole, i);
			fieldList->addItem(item);
		}
	}
	fieldList->selectAll();
}

void MassExportDialog::chooseExportDirectory()
{
	QString dir = Config::value("exportDirectory").toString();
	dir = QFileDialog::getExistingDirectory(this, tr("Choisir un dossier"), dir);
	if(dir.isNull())	return;

	Config::setValue("exportDirectory", dir);
	dirPath->setText(dir);
}

QList<int> MassExportDialog::selectedFields() const
{
	QList<int> ids;

	QList<QListWidgetItem *> items = fieldList->selectedItems();

	foreach(QListWidgetItem *item, items) {
		ids.append(item->data(Qt::UserRole).toInt());
	}

	return ids;
}

bool MassExportDialog::exportBackground() const
{
	return bgExport->isChecked();
}

int MassExportDialog::exportBackgroundFormat() const
{
	return bgFormat->currentIndex();
}

bool MassExportDialog::exportAkao() const
{
	return akaoExport->isChecked();
}

int MassExportDialog::exportAkaoFormat() const
{
	return akaoFormat->currentIndex();
}

bool MassExportDialog::exportText() const
{
	return textExport->isChecked();
}

int MassExportDialog::exportTextFormat() const
{
	return textFormat->currentIndex();
}

QString MassExportDialog::directory() const
{
	return dirPath->text();
}

bool MassExportDialog::overwrite() const
{
	return overwriteIfExists->isChecked();
}

void MassExportDialog::accept()
{
	Config::setValue("overwriteOnExport", overwrite());
	Config::setValue("exportBackgroundFormat", exportBackgroundFormat());

	QDialog::accept();
}
