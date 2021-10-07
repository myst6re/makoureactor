/****************************************************************************
 ** Makou Reactor Final Fantasy VII Field Script Editor
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
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
    _fieldArchive(nullptr), _currentMapId(-1)
{
	setWindowTitle(tr("Mass Import"));

	imports.insert(Fields,
				   new FormatSelectionWidget(tr("Import fields"),
											 QStringList(), this));
	imports.insert(Akaos,
				   new FormatSelectionWidget(tr("Import sounds"),
											 QStringList() <<
											 tr("AKAO sound") + ";;akao", this));
	imports.insert(Texts,
				   new FormatSelectionWidget(tr("Import text"),
											 QStringList() <<
											 tr("XML Text") + ";;xml" <<
											 tr("Simple text TXT") + ";;txt", this));

	dirPath = new QLineEdit(this);
	dirPath->setText(Config::value("importDirectory").toString());
	changeDir = new QPushButton(tr("Choose..."), this);

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

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
	buttonBox->addButton(tr("Import"), QDialogButtonBox::AcceptRole);
	buttonBox->addButton(QDialogButtonBox::Cancel);

	QGridLayout *layout = new QGridLayout(this);
	layout->addLayout(listLayout, 0, 0, 3 + imports.size(), 1);
	int row = 0;
	for (FormatSelectionWidget *formatSelection : qAsConst(imports)) {
		layout->addWidget(formatSelection, row++, 1, 1, 2);
	}
	layout->addWidget(new QLabel(tr("Source directory:")), row, 1, 1, 2);
	layout->addWidget(dirPath, row + 1, 1);
	layout->addWidget(changeDir, row + 1, 2);
	layout->addWidget(buttonBox, row + 3, 0, 1, 3);
	layout->setRowStretch(row + 2, 1);

	connect(changeDir, SIGNAL(clicked()),  SLOT(chooseImportDirectory()));
	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

	fieldList->setFocus();
}

void MassImportDialog::fill(const FieldArchive *fieldArchive, int currentMapId)
{
	_fieldArchive = fieldArchive;
	_currentMapId = currentMapId;

	QString fieldType = _fieldArchive->isPC() ? tr("PC") : tr("PS");
	QStringList formats;
	formats.append(tr("FIELD File %1").arg(fieldType) + (_fieldArchive->isPC() ? QString() : QLatin1String(";;dat")));
	formats.append(tr("Uncompressed FIELD %1").arg(fieldType) + QLatin1String(";;dec"));

	imports.value(Fields)->setFormats(formats);

	fieldList->clear();
	FieldArchiveIterator it(*_fieldArchive);
	while (it.hasNext()) {
		const Field *field = it.next(false);
		if (field) {
			QListWidgetItem *item = new QListWidgetItem(field->name());
			item->setData(Qt::UserRole, it.mapId());
			fieldList->addItem(item);
		}
	}
	fieldList->selectAll();
}

void MassImportDialog::chooseImportDirectory()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory"), directory());
	if (dir.isNull())	return;

	dirPath->setText(dir);
}

void MassImportDialog::selectCurrentField()
{
	if (_currentMapId < 0) {
		return;
	}

	fieldList->clearSelection();

	for (int row=0; row<fieldList->count(); ++row) {
		QListWidgetItem *item = fieldList->item(row);
		if (item->data(Qt::UserRole).toInt() == _currentMapId) {
			item->setSelected(true);
			fieldList->scrollToItem(item);
			return;
		}
	}
}

QList<int> MassImportDialog::selectedFields() const
{
	QList<int> ids;

	QList<QListWidgetItem *> items = fieldList->selectedItems();

	for (QListWidgetItem *item : qAsConst(items)) {
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
	return imports[type]->currentFormat();
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
