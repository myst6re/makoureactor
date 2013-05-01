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

	imports.insert(Fields,
				   new FormatSelectionWidget(tr("Importer des écrans"),
											 QStringList(), this));
	imports.insert(Akaos,
				   new FormatSelectionWidget(tr("Importer les sons"),
											 QStringList() <<
											 tr("Son AKAO") + ";;akao", this));
	imports.insert(Texts,
				   new FormatSelectionWidget(tr("Importer les textes"),
											 QStringList() <<
											 tr("Texte XML") + ";;xml" <<
											 tr("Texte simple TXT") + ";;txt", this));

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
	layout->addWidget(fieldList, 0, 0, 3 + imports.size(), 1);
	int row = 0;
	foreach(FormatSelectionWidget *formatSelection, imports) {
		layout->addWidget(formatSelection, row++, 1, 1, 2);
	}
	layout->addWidget(new QLabel(tr("Emplacement de la source :")), row, 1, 1, 2);
	layout->addWidget(dirPath, row + 1, 1);
	layout->addWidget(changeDir, row + 1, 2);
	layout->addWidget(buttonBox, row + 3, 0, 1, 3);
	layout->setRowStretch(row + 2, 1);
	layout->setColumnStretch(1, 1);

	connect(changeDir, SIGNAL(clicked()),  SLOT(chooseImportDirectory()));
	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
}

void MassImportDialog::fill(FieldArchive *fieldArchive)
{
	_fieldArchive = fieldArchive;

	QString fieldType = _fieldArchive->isPC() ? tr("PC") : tr("PS");
	QStringList formats;
	formats.append(tr("Fichier FIELD %1").arg(fieldType) + (_fieldArchive->isPC() ? "" : ";;dat"));
	formats.append(tr("Fichier décompressé FIELD %1").arg(fieldType) + ";;dec");

	imports.value(Fields)->setFormats(formats);

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
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choisir un dossier"), directory());
	if(dir.isNull())	return;

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

bool MassImportDialog::importModule(ImportType type) const
{
	return imports.value(type)->isChecked();
}

const QString &MassImportDialog::moduleFormat(ImportType type) const
{
	return imports.value(type)->currentFormat();
}

QString MassImportDialog::directory() const
{
	return dirPath->text();
}

void MassImportDialog::accept()
{
	Config::setValue("importDirectory", directory());

	QDialog::accept();
}
