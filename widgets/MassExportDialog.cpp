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
#include "core/Config.h"

MassExportDialog::MassExportDialog(QWidget *parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint),
	_fieldArchive(0)
{
	setWindowTitle(tr("Exporter en masse"));

	fieldList = new QListWidget(this);
	fieldList->setUniformItemSizes(true);
	fieldList->setFixedWidth(200);
	fieldList->setSelectionMode(QAbstractItemView::MultiSelection);

	exports.insert(Fields,
				   new FormatSelectionWidget(tr("Exporter les écrans"),
											 QStringList(), this));
	exports.insert(Backgrounds,
				   new FormatSelectionWidget(tr("Exporter les décors"),
											 QStringList() <<
											 tr("Image PNG;;png") <<
											 tr("Image JPG;;jpg") <<
											 tr("Image BMP;;bmp"), this));
	exports.insert(Akaos,
				   new FormatSelectionWidget(tr("Exporter les sons"),
											 QStringList() <<
											 tr("Son AKAO;;akao"), this));
	exports.insert(Texts,
				   new FormatSelectionWidget(tr("Exporter les textes"),
											 QStringList() <<
											 tr("Texte simple TXT;;txt"), this));

	exports.value(Backgrounds)->setCurrentFormat(Config::value("exportBackgroundFormat").toString());

	dirPath = new QLineEdit(this);
	dirPath->setText(Config::value("exportDirectory").toString());
	changeDir = new QPushButton(tr("Choisir..."), this);

	overwriteIfExists = new QCheckBox(tr("Écraser les fichiers existants"), this);
	overwriteIfExists->setChecked(Config::value("overwriteOnExport", true).toBool());

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
	buttonBox->addButton(tr("Exporter"), QDialogButtonBox::AcceptRole);
	buttonBox->addButton(QDialogButtonBox::Cancel);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(fieldList, 0, 0, 3 + exports.size(), 1);
	int row = 0;
	foreach(FormatSelectionWidget *formatSelection, exports) {
		layout->addWidget(formatSelection, row++, 1, 1, 2);
	}
	layout->addWidget(new QLabel(tr("Emplacement de l'export :")), 4, 1, 1, 2);
	layout->addWidget(dirPath, 5, 1);
	layout->addWidget(changeDir, 5, 2);
	layout->addWidget(overwriteIfExists, 6, 1, 1, 2);
	layout->addWidget(buttonBox, 8, 0, 1, 3);
	layout->setRowStretch(7, 1);
	layout->setColumnStretch(1, 1);

	connect(changeDir, SIGNAL(clicked()),  SLOT(chooseExportDirectory()));
	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
}

void MassExportDialog::fill(FieldArchive *fieldArchive)
{
	_fieldArchive = fieldArchive;

	QString fieldType = _fieldArchive->isPC() ? tr("PC") : tr("PS");
	exports.value(Fields)->setFormats(QStringList()
									 << (tr("Fichier FIELD %1").arg(fieldType) + (_fieldArchive->isPC() ? "" : ";;dat"))
									 << (tr("Fichier décompressé FIELD %1").arg(fieldType) + ";;dec"));

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

bool MassExportDialog::exportModule(ExportType type) const
{
	return exports.value(type)->isChecked();
}

const QString &MassExportDialog::moduleFormat(ExportType type) const
{
	return exports.value(type)->currentFormat();
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
	Config::setValue("exportBackgroundFormat", moduleFormat(Backgrounds));

	QDialog::accept();
}
