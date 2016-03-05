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
	_fieldArchive(0), _currentField(-1)
{
	setWindowTitle(tr("Mass Export"));

	fieldList = new QListWidget(this);
	fieldList->setUniformItemSizes(true);
	fieldList->setSelectionMode(QAbstractItemView::MultiSelection);

	QToolBar *toolBar = new QToolBar(this);
	toolBar->addAction(tr("Current"), this, SLOT(selectCurrentField()));
	toolBar->addAction(tr("+"), fieldList, SLOT(selectAll()));
	toolBar->addAction(tr("-"), fieldList, SLOT(clearSelection()));

	QVBoxLayout *listLayout = new QVBoxLayout;
	listLayout->addWidget(toolBar);
	listLayout->addWidget(fieldList, 1);

	exports.insert(Fields,
				   new FormatSelectionWidget(tr("Export fields"),
											 QStringList(), this));
	exports.insert(Backgrounds,
				   new FormatSelectionWidget(tr("Export backgrounds"),
											 QStringList() <<
											 tr("PNG image") + ";;png" <<
											 tr("JPG image") + ";;jpg" <<
											 tr("BMP image") + ";;bmp", this));
	exports.insert(Akaos,
				   new FormatSelectionWidget(tr("Export sounds"),
											 QStringList() <<
											 tr("AKAO sound") + ";;akao", this));
	exports.insert(Texts,
				   new FormatSelectionWidget(tr("Export texts"),
											 QStringList() <<
											 tr("XML Text") + ";;xml" <<
											 tr("Simple text TXT") + ";;txt", this));

	exports.value(Backgrounds)->setCurrentFormat(Config::value("exportBackgroundFormat").toString());

	dirPath = new QLineEdit(this);
	dirPath->setText(Config::value("exportDirectory").toString());
	changeDir = new QPushButton(tr("Choose..."), this);

	overwriteIfExists = new QCheckBox(tr("Overwrite existing files"), this);
	overwriteIfExists->setChecked(Config::value("overwriteOnExport", true).toBool());

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
	buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);
	buttonBox->addButton(QDialogButtonBox::Cancel);

	QGridLayout *layout = new QGridLayout(this);
	layout->addLayout(listLayout, 0, 0, 3 + exports.size(), 1);
	int row = 0;
	foreach(FormatSelectionWidget *formatSelection, exports) {
		layout->addWidget(formatSelection, row++, 1, 1, 2);
	}
	layout->addWidget(new QLabel(tr("Export directory:")), row, 1, 1, 2);
	layout->addWidget(dirPath, row + 1, 1);
	layout->addWidget(changeDir, row + 1, 2);
	layout->addWidget(overwriteIfExists, row + 2, 1, 1, 2);
	layout->addWidget(buttonBox, row + 4, 0, 1, 3);
	layout->setRowStretch(row + 3, 1);

	connect(changeDir, SIGNAL(clicked()),  SLOT(chooseExportDirectory()));
	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

	fieldList->setFocus();
}

void MassExportDialog::fill(const FieldArchive *fieldArchive, int currentField)
{
	_fieldArchive = fieldArchive;
	_currentField = currentField;

	QString fieldType = _fieldArchive->isPC() ? tr("PC") : tr("PS");
	QStringList formats;
	formats.append(tr("FIELD File %1").arg(fieldType) + (_fieldArchive->isPC() ? "" : ";;dat"));
	formats.append(tr("Uncompressed FIELD %1").arg(fieldType) + ";;dec");

	exports.value(Fields)->setFormats(formats);

	fieldList->clear();
	for(int i=0 ; i<_fieldArchive->size() ; ++i) {
		const Field *field = _fieldArchive->field(i);
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
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory"), directory());
	if(dir.isNull())	return;

	dirPath->setText(dir);
}

void MassExportDialog::selectCurrentField()
{
	if(_currentField < 0) {
		return;
	}

	fieldList->clearSelection();

	for(int row=0 ; row<fieldList->count() ; ++row) {
		QListWidgetItem *item = fieldList->item(row);
		if(item->data(Qt::UserRole).toInt() == _currentField) {
			fieldList->setItemSelected(item, true);
			fieldList->scrollToItem(item);
			return;
		}
	}
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
	return exports[type]->currentFormat();
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
	Config::setValue("exportDirectory", directory());

	QDialog::accept();
}
