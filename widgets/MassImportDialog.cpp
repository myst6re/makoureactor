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
#include "MassImportDialog.h"
#include "core/Config.h"

MassImportDialog::MassImportDialog(QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint),
	_fieldArchive(0)
{
	setWindowTitle(tr("Importer en masse"));

	akaoImport = new FormatSelectionWidget(tr("Importer les sons"),
										   QStringList() <<
										   tr("Son AKAO"), this);
	textImport = new FormatSelectionWidget(tr("Importer les textes"),
										   QStringList() <<
										   tr("Texte simple TXT"), this);

	dirPath = new QLineEdit(this);
	dirPath->setText(Config::value("importDirectory").toString());
	changeDir = new QPushButton(tr("Choisir..."), this);

	fieldList = new QListWidget(this);
	fieldList->setUniformItemSizes(true);
	fieldList->setFixedWidth(200);
	fieldList->setSelectionMode(QAbstractItemView::MultiSelection);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
	buttonBox->addButton(tr("Importer"), QDialogButtonBox::AcceptRole);
	buttonBox->addButton(QDialogButtonBox::Cancel);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(fieldList, 0, 0, 6, 1);
	layout->addWidget(akaoImport, 0, 1, 1, 2);
	layout->addWidget(textImport, 1, 1, 1, 2);
	layout->addWidget(new QLabel(tr("Emplacement de la source :")), 2, 1, 1, 2);
	layout->addWidget(dirPath, 3, 1);
	layout->addWidget(changeDir, 3, 2);
	layout->addWidget(buttonBox, 5, 0, 1, 3, Qt::AlignRight);
	layout->setRowStretch(4, 1);
	layout->setColumnStretch(1, 1);

	connect(changeDir, SIGNAL(clicked()),  SLOT(chooseImportDirectory()));
	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
}

void MassImportDialog::fill(FieldArchive *fieldArchive)
{
	_fieldArchive = fieldArchive;

	fieldList->clear();
	for(int i=0 ; i<_fieldArchive->size() ; ++i) {
		Field *field = _fieldArchive->field(i, false);
		if(field) {
			QListWidgetItem *item = new QListWidgetItem(field->name());
			item->setData(Qt::UserRole, i);
			fieldList->addItem(item);
		}
	}
	fieldList->selectAll();
}

void MassImportDialog::chooseImportDirectory()
{
	QString dir = Config::value("importDirectory").toString();
	dir = QFileDialog::getExistingDirectory(this, tr("Choisir un dossier"), dir);
	if(dir.isNull())	return;

	Config::setValue("importDirectory", dir);
	dirPath->setText(dir);
}

QList<int> MassImportDialog::selectedFields() const
{
	QList<int> ids;

	QList<QListWidgetItem *> items = fieldList->selectedItems();

	foreach(QListWidgetItem *item, items) {
		ids.append(item->data(Qt::UserRole).toInt());
	}

	return ids;
}

bool MassImportDialog::importAkao() const
{
	return akaoImport->isChecked();
}

int MassImportDialog::importAkaoFormat() const
{
	return akaoImport->currentFormat();
}

bool MassImportDialog::importText() const
{
	return textImport->isChecked();
}

int MassImportDialog::importTextFormat() const
{
	return textImport->currentFormat();
}

QString MassImportDialog::directory() const
{
	return dirPath->text();
}

void MassImportDialog::accept()
{
	QDialog::accept();
}
